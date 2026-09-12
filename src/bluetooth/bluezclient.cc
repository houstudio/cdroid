/**
 * BluezClient — sd-bus transport to bluetoothd (org.bluez), the transport
 * half of the android.bluetooth port. Property/method mapping (BlueZ D-Bus
 * API == the role AOSP's IBluetooth binder plays):
 *
 *   Adapter1.Powered            enable()/disable()/isEnabled()
 *   Adapter1.Alias              getName()/setName()
 *   Adapter1.StartDiscovery     startDiscovery()   (powers the adapter too)
 *   Adapter1.Discovering        isDiscovering() + DISCOVERY_* events
 *   Device1.Pair                createBond()
 *   Adapter1.RemoveDevice       removeBond()
 *   ObjectManager signals       ACTION_FOUND / state changes
 *
 * Include order note: sd-bus.h must come first inside its own guard block
 * because it pulls <errno-style> system macros; keeping it isolated avoids
 * polluting the port headers.
 */
#include <errno.h>
#include <poll.h>
#include <sys/prctl.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

#include <systemd/sd-bus.h>

/* The module stays self-contained (no cdroid dependency), so logging is a
 * plain fprintf shim with the same severity names as cdlog (the cdnet
 * precedent). */
#define LOGI(...) do { fprintf(stdout, "BluezClient: " __VA_ARGS__); fprintf(stdout, "\n"); } while (0)
#define LOGD(...) do { fprintf(stdout, "BluezClient: " __VA_ARGS__); fprintf(stdout, "\n"); } while (0)
#define LOGE(...) do { fprintf(stderr, "BluezClient E: " __VA_ARGS__); fprintf(stderr, "\n"); } while (0)
#define LOGV(...) do { fprintf(stdout, "BluezClient V: " __VA_ARGS__); fprintf(stdout, "\n"); } while (0)

#include <bluezclient.h>

namespace cdroid {

static const char* kBluezService = "org.bluez";
static const char* kPropIface = "org.freedesktop.DBus.Properties";
static const char* kObjMgrIface = "org.freedesktop.DBus.ObjectManager";
static const char* kAdapterIface = "org.bluez.Adapter1";
static const char* kDeviceIface = "org.bluez.Device1";

/* ------------------------------------------------------------------ */
/* construction / teardown                                             */
/* ------------------------------------------------------------------ */

BluezClient::BluezClient(Events* events) : mEvents(events) {}

BluezClient::~BluezClient() {
    stopMonitor();
}

void BluezClient::stopMonitor() {
    mRunning.store(false);
    if (mBus) {
        /* Wake a monitor blocked in sd_bus_wait with a synthetic write to
         * the bus fd — sd_bus_process returning 0 with no match fired is
         * what blocks; sd_bus_wait honors a flush timeout, so a short one
         * is enough for a prompt exit. */
        std::lock_guard<std::mutex> lock(mBusMutex);
        if (mBus) sd_bus_flush(mBus);
    }
    if (mMonitorThread.joinable()) mMonitorThread.join();
    std::lock_guard<std::mutex> lock(mBusMutex);
    mSlotProperties = mSlotIfAdded = mSlotIfRemoved = mSlotNameOwner = nullptr;
    if (mBus) {
        sd_bus_flush_close_unref(mBus);
        mBus = nullptr;
    }
    mConnected.store(false);
    clearCache();
}

/* ------------------------------------------------------------------ */
/* connection                                                          */
/* ------------------------------------------------------------------ */

bool BluezClient::connect() {
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        if (mConnected.load() && mBus) return true;

    sd_bus* bus = nullptr;
    int rc = sd_bus_open_system(&bus);
    if (rc < 0) {
        LOGV("no system bus (%s)", strerror(-rc));
        return false;
    }
    /* Bound every synchronous call (default is 25s — an unresponsive
     * bluetoothd must not hang a caller for that long). */
    sd_bus_set_method_call_timeout(bus, 5 * 1000ULL * 1000ULL);

    /* Verify org.bluez owns something before declaring connected; use a
     * cheap ObjectManager call — unknown method/failure both mean "no
     * bluetoothd on this bus". */
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    rc = sd_bus_call_method(bus, kBluezService, "/org/bluez", kObjMgrIface,
                            "GetManagedObjects", &err, &reply, "");
    if (rc < 0) {
        LOGD("org.bluez not available (%s)",
             err.message ? err.message : strerror(-rc));
        sd_bus_error_free(&err);
        sd_bus_unref(bus);
        return false;
    }
    sd_bus_message_unref(reply);
    sd_bus_error_free(&err);

    /* Signal matches attach HERE (and only here) — the same setup path
     * serves the first connect and every reconnect, so a first connect
     * can never end up match-less (properties would go silent). */
    if (mSlotProperties == nullptr) {
        sd_bus_add_match(bus, &mSlotProperties,
                "type='signal',sender='org.bluez',"
                "interface='org.freedesktop.DBus.Properties',"
                "member='PropertiesChanged'", onPropertiesChangedStatic, this);
        sd_bus_add_match(bus, &mSlotIfAdded,
                "type='signal',sender='org.bluez',"
                "interface='org.freedesktop.DBus.ObjectManager',"
                "member='InterfacesAdded'", onInterfacesAddedStatic, this);
        sd_bus_add_match(bus, &mSlotIfRemoved,
                "type='signal',sender='org.bluez',"
                "interface='org.freedesktop.DBus.ObjectManager',"
                "member='InterfacesRemoved'", onInterfacesRemovedStatic, this);
        sd_bus_add_match(bus, &mSlotNameOwner,
                "type='signal',sender='org.freedesktop.DBus',"
                "interface='org.freedesktop.DBus',"
                "member='NameOwnerChanged',arg0='org.bluez'",
                onNameOwnerChangedStatic, this);
    }

        mBus = bus;
        mConnected.store(true);
        /* monitor thread starts once per client lifetime */
        if (!mRunning.load()) {
            mRunning.store(true);
            mMonitorThread = std::thread(&BluezClient::monitorLoop, this);
        }
    }   /* mBusMutex released — refresh takes it itself */

    /* Initial snapshot: without this the FIRST connect leaves the cache
     * empty until some getter trips the lazy refresh — and
     * getBondedDevices() never does. */
    refreshManagedObjects();
    return true;
}

/* ------------------------------------------------------------------ */
/* monitor thread                                                      */
/* ------------------------------------------------------------------ */

void BluezClient::monitorLoop() {
    prctl(PR_SET_NAME, "BluezMonitor", 0, 0, 0);
    int backoffMs = 500;
    while (mRunning.load()) {
        if (!mConnected.load()) {
            if (!connect()) {
                for (int i = 0; i < backoffMs / 50 && mRunning.load(); i++)
                    usleep(50 * 1000);
                backoffMs = std::min(backoffMs * 2, 5000);
                continue;
            }
            backoffMs = 500;
            /* connect() attached the matches; rebuild the cache and let
             * the adapter resync its state from the fresh snapshot. */
            refreshManagedObjects();
            if (mEvents) mEvents->onBluezReconnected();
        }

        while (mRunning.load() && mConnected.load() && processBus()) {
            /* drain everything pending */
        }
        if (!mRunning.load()) break;

        /* Wait for the next message WITHOUT holding the bus lock — a
         * sleeping monitor must never block a synchronous caller: poll
         * the bus fd here (it is stable for the bus lifetime), and only
         * re-take the lock inside processBus. */
        int fd = -1;
        {
            std::lock_guard<std::mutex> lock(mBusMutex);
            if (!mBus) { mConnected.store(false); continue; }
            fd = sd_bus_get_fd(mBus);
        }
        if (fd < 0) { mConnected.store(false); continue; }
        struct pollfd pfd = { fd, POLLIN, 0 };
        const int rc = poll(&pfd, 1, 300);
        if (rc < 0 && errno != EINTR) {
            LOGD("bus poll failed (%s) — will retry", strerror(errno));
            mConnected.store(false);
            if (mEvents) mEvents->onBluezDisconnected();
        }
        /* readable (or the periodic tick for call timeouts): drain */
        if (pfd.revents & (POLLIN | POLLERR | POLLHUP)) processBus();
        else {
            std::lock_guard<std::mutex> lock(mBusMutex);
            if (mBus) sd_bus_process(mBus, nullptr);   /* fire timeouts */
        }
    }
}

bool BluezClient::processBus() {
    std::lock_guard<std::mutex> lock(mBusMutex);
    if (!mBus) return false;
    int processed = 0;
    while (true) {
        sd_bus_message* m = nullptr;
        int rc = sd_bus_process(mBus, &m);
        if (rc < 0) {
            LOGD("process error %s", strerror(-rc));
            mConnected.store(false);
            if (mEvents) mEvents->onBluezDisconnected();
            return false;
        }
        if (m) sd_bus_message_unref(m);
        if (rc == 0) break;
        processed += rc;
        if (rc > 0 && !m) break;   /* 0 remaining */
    }
    return processed > 0;
}

/* ------------------------------------------------------------------ */
/* cache maintenance                                                   */
/* ------------------------------------------------------------------ */

void BluezClient::clearCache() {
    std::lock_guard<std::mutex> lock(mCacheMutex);
    mDevices.clear();
    mAdapterPath.clear();
}

namespace {

/* Read one a{sv} interface dictionary into a device record. Interface
 * unknown entries are skipped. Basic-property reader helpers keep the
 * variant traversal in one place. */
bool readBoolProp(sd_bus_message* m, bool& out) {
    int v = 0;
    if (sd_bus_message_read_basic(m, 'b', &v) < 0) return false;
    out = v != 0;
    return true;
}
bool readInt16Prop(sd_bus_message* m, int16_t& out) {
    return sd_bus_message_read_basic(m, 'n', &out) >= 0;
}
bool readUint32Prop(sd_bus_message* m, uint32_t& out) {
    return sd_bus_message_read_basic(m, 'u', &out) >= 0;
}
bool readObjectPathProp(sd_bus_message* m, std::string& out) {
    const char* s = nullptr;
    if (sd_bus_message_read_basic(m, 'o', &s) < 0 || !s) return false;
    out = s;
    return true;
}
bool readStringProp(sd_bus_message* m, std::string& out) {
    const char* s = nullptr;
    if (sd_bus_message_read_basic(m, 's', &s) < 0 || !s) return false;
    out = s;
    return true;
}
bool readByteArrayProp(sd_bus_message* m, std::vector<uint8_t>& out) {
    uint8_t v = 0;
    if (sd_bus_message_enter_container(m, 'a', "y") < 0) return false;
    while (sd_bus_message_read_basic(m, 'y', &v) > 0) out.push_back(v);
    sd_bus_message_exit_container(m);
    return true;
}
bool readStringArrayProp(sd_bus_message* m, std::vector<std::string>& out) {
    const char* s = nullptr;
    if (sd_bus_message_enter_container(m, 'a', "s") < 0) return false;
    while (sd_bus_message_read_basic(m, 's', &s) > 0) out.push_back(s);
    sd_bus_message_exit_container(m);
    return true;
}

/* Apply the property the message cursor sits IN (variant already
 * entered). Returns true when the name was consumed. */
bool applyDeviceProp(const std::string& name, sd_bus_message* m, BluezDevice& d) {
    if (name == "Address") return readStringProp(m, d.address);
    if (name == "Name") return readStringProp(m, d.name);
    if (name == "Alias") return readStringProp(m, d.alias);
    if (name == "AddressType") return readStringProp(m, d.addressType);
    if (name == "Paired") return readBoolProp(m, d.paired);
    if (name == "Connected") return readBoolProp(m, d.connected);
    if (name == "Trusted") return readBoolProp(m, d.trusted);
    if (name == "RSSI") return readInt16Prop(m, d.rssi);
    if (name == "Class") return readUint32Prop(m, d.cod);
    if (name == "UUIDs") return readStringArrayProp(m, d.uuids);
    /* skip unknown */
    sd_bus_message_skip(m, nullptr);
    return false;
}

} // namespace

namespace {

bool applyGattCharProp(const std::string& name, sd_bus_message* m,
                       BluezGattCharacteristic& c) {
    if (name == "UUID") return readStringProp(m, c.uuid);
    if (name == "Service") return readObjectPathProp(m, c.servicePath);
    if (name == "Flags") return readStringArrayProp(m, c.flags);
    if (name == "Value") return readByteArrayProp(m, c.value);
    if (name == "Notifying") return readBoolProp(m, c.notifying);
    sd_bus_message_skip(m, nullptr);
    return false;
}

} // namespace

bool BluezClient::refreshManagedObjects() {
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        if (!mBus) return false;
        int rc = sd_bus_call_method(mBus, kBluezService, "/org/bluez", kObjMgrIface,
                                    "GetManagedObjects", &err, &reply, "");
        if (rc < 0) {
            sd_bus_error_free(&err);
            return false;
        }
    }

    /* reply: a{oa{sa{sv}}} */
    std::map<std::string, BluezDevice> next;
    std::map<std::string, BluezGattService> nextServices;
    std::map<std::string, BluezGattCharacteristic> nextChars;
    std::string adapterPath;
    bool adapterPowered = false, adapterDiscovering = false;
    std::string adapterAlias;
    sd_bus_message_enter_container(reply, 'a', "{oa{sa{sv}}}");
    while (sd_bus_message_enter_container(reply, 'e', "oa{sa{sv}}") > 0) {
        const char* path = nullptr;
        sd_bus_message_read_basic(reply, 'o', &path);
        sd_bus_message_enter_container(reply, 'a', "{sa{sv}}");
        BluezDevice dev;
        dev.objectPath = path ? path : "";
        bool isAdapter = false, isDevice = false;
        std::string svcUuid, svcDevice;
        BluezGattCharacteristic gattChar;
        bool isService = false, isCharacteristic = false;
        while (sd_bus_message_enter_container(reply, 'e', "sa{sv}") > 0) {
            const char* iface = nullptr;
            sd_bus_message_read_basic(reply, 's', &iface);
            sd_bus_message_enter_container(reply, 'a', "{sv}");
            const std::string ifaceName = iface ? iface : "";
            while (sd_bus_message_enter_container(reply, 'e', "sv") > 0) {
                const char* key = nullptr;
                sd_bus_message_read_basic(reply, 's', &key);
                sd_bus_message_enter_container(reply, 'v', nullptr);
                const std::string propName = key ? key : "";
                if (ifaceName == kAdapterIface) {
                    isAdapter = true;   /* first adapter wins (hci0) */
                    if (propName == "Powered") {
                        readBoolProp(reply, adapterPowered);
                    } else if (propName == "Discovering") {
                        readBoolProp(reply, adapterDiscovering);
                    } else if (propName == "Alias") {
                        readStringProp(reply, adapterAlias);
                    } else {
                        sd_bus_message_skip(reply, nullptr);
                    }
                } else if (ifaceName == kDeviceIface) {
                    isDevice = true;
                    applyDeviceProp(propName, reply, dev);
                } else if (ifaceName == "org.bluez.GattService1") {
                    isService = true;
                    if (propName == "UUID") readStringProp(reply, svcUuid);
                    else sd_bus_message_skip(reply, nullptr);
                } else if (ifaceName == "org.bluez.GattCharacteristic1") {
                    isCharacteristic = true;
                    gattChar.objectPath = dev.objectPath;
                    applyGattCharProp(propName, reply, gattChar);
                } else {
                    sd_bus_message_skip(reply, nullptr);
                }
                sd_bus_message_exit_container(reply);   // v
                sd_bus_message_exit_container(reply);   // e sv
            }
            sd_bus_message_exit_container(reply);       // a sv
            sd_bus_message_exit_container(reply);       // e sa{sv}
        }
        sd_bus_message_exit_container(reply);           // a sa{sv}
        sd_bus_message_exit_container(reply);           // e oa{...}
        if (isAdapter && adapterPath.empty()) adapterPath = dev.objectPath;
        if (isDevice && !dev.address.empty()) {
            std::transform(dev.address.begin(), dev.address.end(),
                           dev.address.begin(), ::toupper);
            if (dev.alias.empty()) dev.alias = dev.name;
            next[dev.objectPath] = dev;
        }
        if (isService) {
            BluezGattService s;
            s.objectPath = dev.objectPath;
            s.uuid = svcUuid;
            /* the service's device is the path prefix (.../dev_XX/serviceYY) */
            const size_t pos = s.objectPath.find("/service");
            s.devicePath = pos == std::string::npos ? std::string()
                        : s.objectPath.substr(0, pos);
            nextServices[s.objectPath] = s;
        }
        if (isCharacteristic) nextChars[gattChar.objectPath] = gattChar;
    }
    sd_bus_message_exit_container(reply);
    sd_bus_message_unref(reply);
    sd_bus_error_free(&err);

    {
        std::lock_guard<std::mutex> lock(mCacheMutex);
        mDevices = std::move(next);
        mGattServices = std::move(nextServices);
        mGattCharacteristics = std::move(nextChars);
        if (!adapterPath.empty()) mAdapterPath = adapterPath;
    }
    /* Resync the adapter state from the snapshot — values ride along so
     * handlers never need a synchronous bus call (self-deadlock guard). */
    if (mEvents && !adapterPath.empty()) {
        mEvents->onAdapterBoolChanged("Powered", adapterPowered);
        mEvents->onAdapterBoolChanged("Discovering", adapterDiscovering);
        if (!adapterAlias.empty())
            mEvents->onAdapterStringChanged("Alias", adapterAlias);
    }
    return true;
}

/* ------------------------------------------------------------------ */
/* signal handlers                                                     */
/* ------------------------------------------------------------------ */

int BluezClient::onPropertiesChangedStatic(sd_bus_message* m, void* userdata,
                                           sd_bus_error*) {
    static_cast<BluezClient*>(userdata)->handlePropertiesChanged(m);
    return 0;
}

void BluezClient::handlePropertiesChanged(sd_bus_message* m) {
    /* s(interface) a{sv} */
    const char* iface = nullptr;
    if (sd_bus_message_read_basic(m, 's', &iface) < 0 || !iface) return;
    const char* path = sd_bus_message_get_path(m);
    const std::string objPath = path ? path : "";

    if (std::string(iface) == kAdapterIface) {
        if (sd_bus_message_enter_container(m, 'a', "{sv}") < 0) return;
        while (sd_bus_message_enter_container(m, 'e', "sv") > 0) {
            const char* key = nullptr;
            sd_bus_message_read_basic(m, 's', &key);
            const std::string name = key ? key : "";
            if (sd_bus_message_enter_container(m, 'v', "b") >= 0) {
                int v = 0;
                if (sd_bus_message_read_basic(m, 'b', &v) >= 0 && mEvents)
                    mEvents->onAdapterBoolChanged(name, v != 0);
                sd_bus_message_exit_container(m);
            } else if (sd_bus_message_enter_container(m, 'v', "s") >= 0) {
                const char* s = nullptr;
                if (sd_bus_message_read_basic(m, 's', &s) >= 0 && mEvents && s)
                    mEvents->onAdapterStringChanged(name, s);
                sd_bus_message_exit_container(m);
            } else {
                sd_bus_message_skip(m, nullptr);
            }
            sd_bus_message_exit_container(m);
        }
        sd_bus_message_exit_container(m);
        return;
    }
    if (std::string(iface) == "org.bluez.GattCharacteristic1") {
        BluezGattCharacteristic snapshot;
        {
            std::lock_guard<std::mutex> lock(mCacheMutex);
            auto it = mGattCharacteristics.find(objPath);
            if (it == mGattCharacteristics.end()) return;
            snapshot = it->second;
            if (sd_bus_message_enter_container(m, 'a', "{sv}") < 0) return;
            while (sd_bus_message_enter_container(m, 'e', "sv") > 0) {
                const char* key = nullptr;
                sd_bus_message_read_basic(m, 's', &key);
                sd_bus_message_enter_container(m, 'v', nullptr);
                applyGattCharProp(key ? key : "", m, snapshot);
                sd_bus_message_exit_container(m);
                sd_bus_message_exit_container(m);
            }
            sd_bus_message_exit_container(m);
            it->second = snapshot;
        }
        if (mEvents) mEvents->onGattCharacteristicChanged(snapshot);
        return;
    }
    if (std::string(iface) != kDeviceIface) return;

    BluezDevice snapshot;
    {
        std::lock_guard<std::mutex> lock(mCacheMutex);
        auto it = mDevices.find(objPath);
        if (it == mDevices.end()) return;   /* stale/uninteresting */
        snapshot = it->second;
        if (sd_bus_message_enter_container(m, 'a', "{sv}") < 0) return;
        while (sd_bus_message_enter_container(m, 'e', "sv") > 0) {
            const char* key = nullptr;
            sd_bus_message_read_basic(m, 's', &key);
            sd_bus_message_enter_container(m, 'v', nullptr);
            const std::string name = key ? key : "";
            applyDeviceProp(name, m, snapshot);
            sd_bus_message_exit_container(m);
            sd_bus_message_exit_container(m);
        }
        sd_bus_message_exit_container(m);
        it->second = snapshot;   /* publish under the same lock */
    }
    if (mEvents) {
        /* One notification per PropertiesChanged carrying the refreshed
         * snapshot — the listener re-reads via the adapter anyway. */
        mEvents->onDevicePropertyChanged(snapshot, "*");
    }
}

int BluezClient::onInterfacesAddedStatic(sd_bus_message* m, void* userdata,
                                         sd_bus_error*) {
    static_cast<BluezClient*>(userdata)->handleInterfacesAdded(m);
    return 0;
}

void BluezClient::handleInterfacesAdded(sd_bus_message* m) {
    /* oa{sa{sv}} */
    const char* path = nullptr;
    if (sd_bus_message_read_basic(m, 'o', &path) < 0 || !path) return;
    const std::string objPath = path;

    if (sd_bus_message_enter_container(m, 'a', "{sa{sv}}") < 0) return;
    BluezDevice dev;
    dev.objectPath = objPath;
    bool isDevice = false;
    while (sd_bus_message_enter_container(m, 'e', "sa{sv}") > 0) {
        const char* iface = nullptr;
        sd_bus_message_read_basic(m, 's', &iface);
        sd_bus_message_enter_container(m, 'a', "{sv}");
        const std::string ifaceName = iface ? iface : "";
        while (sd_bus_message_enter_container(m, 'e', "sv") > 0) {
            const char* key = nullptr;
            sd_bus_message_read_basic(m, 's', &key);
            sd_bus_message_enter_container(m, 'v', nullptr);
            const std::string propName = key ? key : "";
            if (ifaceName == kDeviceIface) {
                isDevice = true;
                applyDeviceProp(propName, m, dev);
            } else {
                sd_bus_message_skip(m, nullptr);
            }
            sd_bus_message_exit_container(m);
            sd_bus_message_exit_container(m);
        }
        sd_bus_message_exit_container(m);
        sd_bus_message_exit_container(m);
    }
    sd_bus_message_exit_container(m);
    if (!isDevice || dev.address.empty()) return;

    std::transform(dev.address.begin(), dev.address.end(),
                   dev.address.begin(), ::toupper);
    if (dev.alias.empty()) dev.alias = dev.name;
    {
        std::lock_guard<std::mutex> lock(mCacheMutex);
        mDevices[objPath] = dev;
    }
    if (mEvents) mEvents->onDeviceAdded(dev);
}

int BluezClient::onInterfacesRemovedStatic(sd_bus_message* m, void* userdata,
                                           sd_bus_error*) {
    const char* path = nullptr;
    if (sd_bus_message_read_basic(m, 'o', &path) < 0 || !path) return 0;
    const std::string objPath = path;
    auto* self = static_cast<BluezClient*>(userdata);
    {
        std::lock_guard<std::mutex> lock(self->mCacheMutex);
        self->mDevices.erase(objPath);
    }
    if (self->mEvents) self->mEvents->onDeviceRemoved(objPath);
    return 0;
}

int BluezClient::onNameOwnerChangedStatic(sd_bus_message* m, void* userdata,
                                          sd_bus_error*) {
    auto* self = static_cast<BluezClient*>(userdata);
    /* s name, s old, s new */
    const char *name = nullptr, *oldOwner = nullptr, *newOwner = nullptr;
    if (sd_bus_message_read(m, "sss", &name, &oldOwner, &newOwner) < 0) return 0;
    if (!name || std::string(name) != kBluezService) return 0;
    const bool had = oldOwner && *oldOwner;
    const bool has = newOwner && *newOwner;
    if (had && !has) {
        self->mConnected.store(false);
        if (self->mEvents) self->mEvents->onBluezDisconnected();
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* synchronous requests                                                */
/* ------------------------------------------------------------------ */

std::string BluezClient::pathForAddressLocked(const std::string& address) const {
    for (const auto& kv : mDevices) {
        if (kv.second.address == address) return kv.first;
    }
    return std::string();
}

bool BluezClient::getAdapterBool(const std::string& name, bool& value) {
    if (!connect() || mAdapterPath.empty()) {
        /* try a fresh enumeration — the cache may predate a hotplug */
        if (!refreshManagedObjects() || mAdapterPath.empty()) return false;
    }
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    int v = 0;
    bool ok = false;
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        ok = sd_bus_call_method(mBus, kBluezService, mAdapterPath.c_str(), kPropIface,
                                "Get", &err, &reply, "ss", kAdapterIface,
                                name.c_str()) >= 0
             && sd_bus_message_enter_container(reply, 'v', "b") >= 0
             && sd_bus_message_read_basic(reply, 'b', &v) >= 0;
    }
    if (ok) value = v != 0;
    sd_bus_message_unrefp(&reply);
    sd_bus_error_free(&err);
    return ok;
}

bool BluezClient::getAdapterString(const std::string& name, std::string& value) {
    if (!connect() || mAdapterPath.empty()) {
        if (!refreshManagedObjects() || mAdapterPath.empty()) return false;
    }
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    const char* s = nullptr;
    bool ok = false;
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        ok = sd_bus_call_method(mBus, kBluezService, mAdapterPath.c_str(), kPropIface,
                                "Get", &err, &reply, "ss", kAdapterIface,
                                name.c_str()) >= 0
             && sd_bus_message_enter_container(reply, 'v', "s") >= 0
             && sd_bus_message_read_basic(reply, 's', &s) >= 0
             && s != nullptr;
    }
    if (ok) value = s;
    sd_bus_message_unrefp(&reply);
    sd_bus_error_free(&err);
    return ok;
}

bool BluezClient::setAdapterBool(const std::string& name, bool value) {
    if (!connect() || mAdapterPath.empty()) {
        if (!refreshManagedObjects() || mAdapterPath.empty()) return false;
    }
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    bool ok;
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        ok = sd_bus_call_method(mBus, kBluezService, mAdapterPath.c_str(), kPropIface,
                                "Set", &err, &reply, "ssv", kAdapterIface,
                                name.c_str(), "b", value ? 1 : 0) >= 0;
    }
    sd_bus_message_unrefp(&reply);
    sd_bus_error_free(&err);
    return ok;
}

bool BluezClient::setAdapterString(const std::string& name, const std::string& value) {
    if (!connect() || mAdapterPath.empty()) {
        if (!refreshManagedObjects() || mAdapterPath.empty()) return false;
    }
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    bool ok;
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        ok = sd_bus_call_method(mBus, kBluezService, mAdapterPath.c_str(), kPropIface,
                                "Set", &err, &reply, "ssv", kAdapterIface,
                                name.c_str(), "s", value.c_str()) >= 0;
    }
    sd_bus_message_unrefp(&reply);
    sd_bus_error_free(&err);
    return ok;
}

bool BluezClient::adapterCall(const char* method) {
    if (!connect() || mAdapterPath.empty()) {
        if (!refreshManagedObjects() || mAdapterPath.empty()) return false;
    }
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    bool ok;
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        ok = sd_bus_call_method(mBus, kBluezService, mAdapterPath.c_str(),
                                kAdapterIface, method, &err, &reply, "") >= 0;
    }
    if (!ok && err.message) LOGD("%s failed: %s", method, err.message);
    sd_bus_message_unrefp(&reply);
    sd_bus_error_free(&err);
    return ok;
}

bool BluezClient::startDiscovery() {
    return adapterCall("StartDiscovery");
}

bool BluezClient::cancelDiscovery() {
    return adapterCall("StopDiscovery");
}

bool BluezClient::deviceCall(const std::string& address, const char* method) {
    std::string path;
    {
        std::lock_guard<std::mutex> lock(mCacheMutex);
        path = pathForAddressLocked(address);
    }
    if (path.empty()) return false;   /* unknown remote — discover it first */
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    bool ok;
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        if (!mBus) return false;
        ok = sd_bus_call_method(mBus, kBluezService, path.c_str(), kDeviceIface,
                                method, &err, &reply, "") >= 0;
    }
    if (!ok && err.message) LOGD("%s failed: %s", method, err.message);
    sd_bus_message_unrefp(&reply);
    sd_bus_error_free(&err);
    return ok;
}

bool BluezClient::pairDevice(const std::string& address) {
    return deviceCall(address, "Pair");
}

bool BluezClient::removeDevice(const std::string& address) {
    std::string path;
    {
        std::lock_guard<std::mutex> lock(mCacheMutex);
        path = pathForAddressLocked(address);
    }
    if (path.empty()) return false;
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    bool ok;
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        if (!mBus) return false;
        ok = sd_bus_call_method(mBus, kBluezService, mAdapterPath.c_str(),
                                kAdapterIface, "RemoveDevice", &err, &reply,
                                "o", path.c_str()) >= 0;
    }
    if (!ok && err.message) LOGD("RemoveDevice failed: %s", err.message);
    sd_bus_message_unrefp(&reply);
    sd_bus_error_free(&err);
    return ok;
}

bool BluezClient::setDeviceAlias(const std::string& address, const std::string& alias) {
    std::string path;
    {
        std::lock_guard<std::mutex> lock(mCacheMutex);
        path = pathForAddressLocked(address);
    }
    if (path.empty()) return false;
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    bool ok;
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        if (!mBus) return false;
        ok = sd_bus_call_method(mBus, kBluezService, path.c_str(), kPropIface,
                                "Set", &err, &reply, "ssv", kDeviceIface,
                                "Alias", "s", alias.c_str()) >= 0;
    }
    sd_bus_message_unrefp(&reply);
    sd_bus_error_free(&err);
    return ok;
}

/* ------------------------------------------------------------------ */
/* BLE / GATT                                                          */
/* ------------------------------------------------------------------ */

bool BluezClient::startLeDiscovery(const std::vector<std::string>& uuidFilter,
                                   int16_t rssiThreshold) {
    if (!connect() || mAdapterPath.empty()) {
        if (!refreshManagedObjects() || mAdapterPath.empty()) return false;
    }
    /* std::string -> char* vector for sd_bus_message_append_strv */
    std::vector<std::string> owned(uuidFilter.begin(), uuidFilter.end());
    std::vector<char*> strv;
    for (const std::string& u : owned) strv.push_back(const_cast<char*>(u.c_str()));
    strv.push_back(nullptr);

    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* msg = nullptr;
    sd_bus_message* reply = nullptr;
    bool ok = false;
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        if (!mBus) return false;
        ok = sd_bus_message_new_method_call(mBus, &msg, kBluezService,
                mAdapterPath.c_str(), kAdapterIface, "SetDiscoveryFilter") >= 0
          && sd_bus_message_open_container(msg, 'a', "{sv}") >= 0;
        if (ok) {   /* Transport: "le" */
            ok = sd_bus_message_open_container(msg, 'e', "sv") >= 0
              && sd_bus_message_append(msg, "s", "Transport") >= 0
              && sd_bus_message_append(msg, "v", "s", "le") >= 0
              && sd_bus_message_close_container(msg) >= 0;
        }
        if (ok && !uuidFilter.empty()) {
            ok = sd_bus_message_open_container(msg, 'e', "sv") >= 0
              && sd_bus_message_append(msg, "s", "UUIDs") >= 0
              && sd_bus_message_open_container(msg, 'v', "as") >= 0
              && sd_bus_message_append_strv(msg, strv.data()) >= 0
              && sd_bus_message_close_container(msg) >= 0
              && sd_bus_message_close_container(msg) >= 0;   /* v, then e */
            /* fix ordering: close variant then the dict entry — the two
             * closes above already did both */
            ok = ok && true;
        }
        if (ok && rssiThreshold > INT16_MIN) {
            ok = sd_bus_message_open_container(msg, 'e', "sv") >= 0
              && sd_bus_message_append(msg, "s", "RSSI") >= 0
              && sd_bus_message_append(msg, "v", "n", rssiThreshold) >= 0
              && sd_bus_message_close_container(msg) >= 0;
        }
        if (ok) {
            ok = sd_bus_message_close_container(msg) >= 0   /* a{sv} */
              && sd_bus_call(mBus, msg, 0, &err, &reply) >= 0;
        }
        if (!ok) LOGD("SetDiscoveryFilter failed (%s)",
                     err.message ? err.message : "build/transport");
    }
    sd_bus_message_unref(msg);
    sd_bus_message_unrefp(&reply);
    sd_bus_error_free(&err);
    return ok ? startDiscovery() : false;
}

bool BluezClient::connectDevice(const std::string& address) {
    return deviceCall(address, "Connect");
}

bool BluezClient::disconnectDevice(const std::string& address) {
    return deviceCall(address, "Disconnect");
}

std::vector<BluezGattService> BluezClient::getGattServices(
        const std::string& deviceAddress) const {
    std::vector<BluezGattService> out;
    std::lock_guard<std::mutex> lock(mCacheMutex);
    for (const auto& kv : mGattServices) {
        /* device match by path prefix dev_XX */
        const std::string& p = kv.second.devicePath;
        const size_t pos = p.find("/dev_");
        if (pos != std::string::npos) {
            std::string addr = p.substr(pos + 5);
            std::replace(addr.begin(), addr.end(), '_', ':');
            if (addr == deviceAddress) out.push_back(kv.second);
        }
    }
    return out;
}

std::vector<BluezGattCharacteristic> BluezClient::getGattCharacteristics(
        const std::string& servicePath) const {
    std::vector<BluezGattCharacteristic> out;
    std::lock_guard<std::mutex> lock(mCacheMutex);
    for (const auto& kv : mGattCharacteristics) {
        if (kv.second.servicePath == servicePath) out.push_back(kv.second);
    }
    return out;
}

bool BluezClient::gattRead(const std::string& characteristicPath,
                           std::vector<uint8_t>& out) {
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    bool ok;
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        if (!mBus) return false;
        ok = sd_bus_call_method(mBus, kBluezService, characteristicPath.c_str(),
                "org.bluez.GattCharacteristic1", "ReadValue", &err, &reply,
                "a{sv}", 0) >= 0;
    }
    if (ok) ok = readByteArrayProp(reply, out);
    sd_bus_message_unrefp(&reply);
    sd_bus_error_free(&err);
    return ok;
}

bool BluezClient::gattWrite(const std::string& characteristicPath,
                            const std::vector<uint8_t>& value,
                            bool withoutResponse) {
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    sd_bus_message* msg = nullptr;
    bool ok;
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        if (!mBus) return false;
        ok = sd_bus_message_new_method_call(mBus, &msg, kBluezService,
                characteristicPath.c_str(), "org.bluez.GattCharacteristic1",
                withoutResponse ? "WriteValue" : "WriteValue") >= 0
          && sd_bus_message_append_array(msg, 'y', value.data(),
                                         value.size()) >= 0
          && sd_bus_message_open_container(msg, 'a', "{sv}") >= 0
          && sd_bus_message_close_container(msg) >= 0
          && sd_bus_call(mBus, msg, 0, &err, &reply) >= 0;
    }
    sd_bus_message_unref(msg);
    sd_bus_message_unrefp(&reply);
    sd_bus_error_free(&err);
    return ok;
}

bool BluezClient::gattSetNotify(const std::string& characteristicPath,
                                bool enable) {
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    bool ok;
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        if (!mBus) return false;
        ok = sd_bus_call_method(mBus, kBluezService, characteristicPath.c_str(),
                "org.bluez.GattCharacteristic1",
                enable ? "StartNotify" : "StopNotify", &err, &reply, "") >= 0;
    }
    sd_bus_message_unrefp(&reply);
    sd_bus_error_free(&err);
    return ok;
}

std::vector<BluezDevice> BluezClient::getDevices() const {
    std::lock_guard<std::mutex> lock(mCacheMutex);
    std::vector<BluezDevice> out;
    out.reserve(mDevices.size());
    for (const auto& kv : mDevices) out.push_back(kv.second);
    return out;
}

bool BluezClient::findDevice(const std::string& address, BluezDevice& out) const {
    std::lock_guard<std::mutex> lock(mCacheMutex);
    for (const auto& kv : mDevices) {
        if (kv.second.address == address) {
            out = kv.second;
            return true;
        }
    }
    return false;
}

} // namespace cdroid
