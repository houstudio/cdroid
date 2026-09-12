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

        /* RETIRE the previous bus before adopting the new one (the
         * reconnect path): dropping it releases its match slots and the
         * Agent1 vtable with it, and skipping this leaked one sd_bus+fd
         * per daemon restart while leaving every slot pointing at a dead
         * bus (matches would never re-attach — the review's "permanently
         * deaf after restart" finding). */
        if (mBus) {
            sd_bus_flush_close_unref(mBus);
            mBus = nullptr;
        }
        mSlotProperties = mSlotIfAdded = mSlotIfRemoved = mSlotNameOwner = nullptr;

        /* Signal matches attach to THIS bus on every (re)connect. */
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

        mBus = bus;
        mConnected.store(true);
        /* monitor thread starts once per client lifetime */
        if (!mRunning.load()) {
            mRunning.store(true);
            mMonitorThread = std::thread(&BluezClient::monitorLoop, this);
        }
    }   /* mBusMutex released — refresh takes it itself */

    /* Initial snapshot (also the reconnect resync): without this the
     * FIRST connect leaves the cache empty until some getter trips the
     * lazy refresh — and getBondedDevices() never does. */
    refreshManagedObjects();
    /* Re-register the pairing agent on the fresh bus if one was active. */
    if (!mAgentCapability.empty()) registerAgent(mAgentCapability);
    return true;
}


/* ------------------------------------------------------------------ */
/* pairing agent (org.bluez.Agent1)                                    */
/* ------------------------------------------------------------------ */

/* Runs on the monitor thread with NO bus lock held. */
void BluezClient::flushDeferredPairing() {
    std::vector<std::pair<std::string, int>> pending;
    pending.swap(mDeferredPairing);
    for (const auto& req : pending) {
        if (mEvents == nullptr) continue;
        switch (req.second) {
        case 0: mEvents->onPairingPinRequested(req.first); break;
        case 1: mEvents->onPairingPasskeyRequested(req.first); break;
        case 2: mEvents->onPairingConfirmationRequested(req.first); break;
        case -1: mEvents->onPairingCancelled(); break;
        default: break;
        }
    }
}

static const char* kAgentPath = "/org/cdroid/agent";
static const char* kAgentIface = "org.bluez.Agent1";
static const char* kAgentMgrIface = "org.bluez.AgentManager1";

std::string BluezClient::addressForDevicePath(const std::string& objectPath) const {
    /* ".../dev_AA_BB_CC_DD_EE_FF..." -> "AA:BB:CC:DD:EE:FF" */
    const size_t pos = objectPath.find("/dev_");
    if (pos == std::string::npos) return std::string();
    std::string addr = objectPath.substr(pos + 5);
    const size_t end = addr.find('/');
    if (end != std::string::npos) addr = addr.substr(0, end);
    std::replace(addr.begin(), addr.end(), '_', ':');
    return addr;
}

void BluezClient::holdPendingPairing(sd_bus_message* m,
                                     const std::string& address, int kind) {
    sd_bus_message_ref(m);
    std::lock_guard<std::mutex> lock(mPairingMutex);
    if (mPendingPairing.message) {
        /* superseded: refuse the old one so the daemon is not stuck */
        sd_bus_error err = SD_BUS_ERROR_NULL;
        sd_bus_error_set(&err, "org.bluez.Error.Rejected", "superseded");
        sd_bus_reply_method_error(m, &err);
        sd_bus_error_free(&err);
        sd_bus_message_unref(m);
        return;
    }
    mPendingPairing.message = m;
    mPendingPairing.address = address;
    mPendingPairing.kind = kind;
}

int BluezClient::agentRequestPinCode(sd_bus_message* m, void* userdata,
                                     sd_bus_error*) {
    auto* self = static_cast<BluezClient*>(userdata);
    const char* dev = nullptr;
    sd_bus_message_read(m, "o", &dev);
    const std::string addr = self->addressForDevicePath(dev ? dev : "");
    self->holdPendingPairing(m, addr, 0);
    LOGD("agent: RequestPinCode for %s (held)", addr.c_str());
    self->mDeferredPairing.push_back({addr, 0});   /* fired outside the lock */
    /* Return 1: the reply is DEFERRED (setPin answers later). A vtable
     * method handler returning 0 tells sd-bus "done, nothing owed" and
     * 260 auto-replies UnknownMethod to the caller — the deferred-reply
     * contract is a positive return (message ownership taken). */
    return 1;
}

int BluezClient::agentRequestPasskey(sd_bus_message* m, void* userdata,
                                     sd_bus_error*) {
    auto* self = static_cast<BluezClient*>(userdata);
    const char* dev = nullptr;
    sd_bus_message_read(m, "o", &dev);
    const std::string addr = self->addressForDevicePath(dev ? dev : "");
    self->holdPendingPairing(m, addr, 1);
    self->mDeferredPairing.push_back({addr, 1});
    return 1;   /* deferred reply — see RequestPinCode */
}

int BluezClient::agentRequestConfirmation(sd_bus_message* m, void* userdata,
                                         sd_bus_error*) {
    /* THE DisplayYesNo method: numeric comparison. Surfaces as the
     * PASSKEY_CONFIRMATION variant; replyPairingConfirmation answers. */
    auto* self = static_cast<BluezClient*>(userdata);
    const char* dev = nullptr;
    uint32_t passkey = 0;
    sd_bus_message_read(m, "ou", &dev, &passkey);
    const std::string addr = self->addressForDevicePath(dev ? dev : "");
    self->holdPendingPairing(m, addr, 2);
    self->mDeferredPairing.push_back({addr, 2});
    return 1;   /* deferred reply */
}

int BluezClient::agentRequestAuthorization(sd_bus_message* m, void* userdata,
                                           sd_bus_error*) {
    auto* self = static_cast<BluezClient*>(userdata);
    const char* dev = nullptr;
    sd_bus_message_read(m, "o", &dev);
    const std::string addr = self->addressForDevicePath(dev ? dev : "");
    self->holdPendingPairing(m, addr, 2);
    self->mDeferredPairing.push_back({addr, 2});
    return 1;   /* deferred reply */
}

int BluezClient::agentAuthorizeService(sd_bus_message* m, void* userdata,
                                       sd_bus_error*) {
    auto* self = static_cast<BluezClient*>(userdata);
    const char* dev = nullptr;
    sd_bus_message_read(m, "os", &dev, nullptr);
    const std::string addr = self->addressForDevicePath(dev ? dev : "");
    self->holdPendingPairing(m, addr, 2);
    self->mDeferredPairing.push_back({addr, 2});
    return 1;   /* deferred reply */
}

int BluezClient::agentDisplayPasskey(sd_bus_message* m, void* userdata,
                                     sd_bus_error*) {
    auto* self = static_cast<BluezClient*>(userdata);
    const char* dev = nullptr;
    uint32_t passkey = 0;
    uint16_t entered = 0;
    sd_bus_message_read(m, "ouq", &dev, &passkey, &entered);
    const std::string addr = self->addressForDevicePath(dev ? dev : "");
    if (self->mEvents) self->mEvents->onDisplayPasskey(addr, passkey);
    return sd_bus_reply_method_return(m, "");
}

int BluezClient::agentDisplayPinCode(sd_bus_message* m, void* userdata,
                                     sd_bus_error*) {
    auto* self = static_cast<BluezClient*>(userdata);
    const char* dev = nullptr;
    sd_bus_message_read(m, "os", &dev, nullptr);
    const std::string addr = self->addressForDevicePath(dev ? dev : "");
    if (self->mEvents) self->mEvents->onDisplayPasskey(addr, 0);
    return sd_bus_reply_method_return(m, "");
}

int BluezClient::agentCancel(sd_bus_message* m, void* userdata, sd_bus_error*) {
    auto* self = static_cast<BluezClient*>(userdata);
    {
        std::lock_guard<std::mutex> lock(self->mPairingMutex);
        if (self->mPendingPairing.message) {
            sd_bus_message_unref(self->mPendingPairing.message);
            self->mPendingPairing.message = nullptr;
            self->mPendingPairing.kind = -1;
        }
    }
    self->mDeferredPairing.push_back({"", -1});   /* cancelled notice */
    return sd_bus_reply_method_return(m, "");
}

int BluezClient::agentRelease(sd_bus_message* m, void*, sd_bus_error*) {
    return sd_bus_reply_method_return(m, "");
}

bool BluezClient::registerAgent(const std::string& capability) {
    if (!connect()) return false;
    mAgentCapability = capability;   /* re-registered on reconnect */
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        if (!mBus) return false;
        /* vtable with the full method set (member fns are statics) */
        static const sd_bus_vtable agent[] = {
            SD_BUS_VTABLE_START(0),
            SD_BUS_METHOD("RequestPinCode", "o", "s",
                          BluezClient::agentRequestPinCode, 0),
            SD_BUS_METHOD("RequestPasskey", "o", "u",
                          BluezClient::agentRequestPasskey, 0),
            SD_BUS_METHOD("RequestConfirmation", "ou", NULL,
                          BluezClient::agentRequestConfirmation, 0),
            SD_BUS_METHOD("RequestAuthorization", "o", NULL,
                          BluezClient::agentRequestAuthorization, 0),
            SD_BUS_METHOD("AuthorizeService", "os", NULL,
                          BluezClient::agentAuthorizeService, 0),
            SD_BUS_METHOD("DisplayPasskey", "ouq", NULL,
                          BluezClient::agentDisplayPasskey, 0),
            SD_BUS_METHOD("DisplayPinCode", "os", NULL,
                          BluezClient::agentDisplayPinCode, 0),
            SD_BUS_METHOD("Cancel", NULL, NULL, BluezClient::agentCancel, 0),
            SD_BUS_METHOD("Release", NULL, NULL, BluezClient::agentRelease, 0),
            SD_BUS_VTABLE_END,
        };
        sd_bus_slot* slot = nullptr;
        if (sd_bus_add_object_vtable(mBus, &slot, kAgentPath, kAgentIface,
                                      agent, this) < 0)
            return false;
        sd_bus_error err = SD_BUS_ERROR_NULL;
        sd_bus_message* reply = nullptr;
        const bool ok = sd_bus_call_method(mBus, kBluezService,
                "/org/bluez", kAgentMgrIface, "RegisterAgent", &err, &reply,
                "os", kAgentPath, capability.c_str()) >= 0
         && sd_bus_call_method(mBus, kBluezService, "/org/bluez",
                kAgentMgrIface, "RequestDefaultAgent", &err, &reply,
                "o", kAgentPath) >= 0;
        if (!ok && err.message) LOGD("RegisterAgent failed: %s", err.message);
        sd_bus_message_unrefp(&reply);
        sd_bus_error_free(&err);
        return ok;
    }
}

bool BluezClient::replyPairingPin(const std::string& pin) {
    sd_bus_message* m = nullptr;
    {
        std::lock_guard<std::mutex> lock(mPairingMutex);
        if (mPendingPairing.message == nullptr) return false;
        if (mPendingPairing.kind != 0) return false;   /* not a PIN request */
        m = mPendingPairing.message;
        mPendingPairing.message = nullptr;
        mPendingPairing.kind = -1;
    }
    /* Bus send outside mPairingMutex: the monitor takes mBusMutex first
     * and then mPairingMutex inside agent handlers — holding pairing
     * while waiting for the bus would invert that order. */
    std::lock_guard<std::mutex> lock(mBusMutex);
    const int rc = mBus ? sd_bus_reply_method_return(m, "s", pin.c_str()) : -ENOTCONN;
    sd_bus_message_unref(m);
    return rc >= 0;
}

bool BluezClient::replyPairingPasskey(uint32_t passkey) {
    sd_bus_message* m = nullptr;
    {
        std::lock_guard<std::mutex> lock(mPairingMutex);
        if (mPendingPairing.message == nullptr) return false;
        if (mPendingPairing.kind != 1) return false;   /* not a passkey req */
        m = mPendingPairing.message;
        mPendingPairing.message = nullptr;
        mPendingPairing.kind = -1;
    }
    std::lock_guard<std::mutex> lock(mBusMutex);
    const int rc = mBus ? sd_bus_reply_method_return(m, "u", passkey) : -ENOTCONN;
    sd_bus_message_unref(m);
    return rc >= 0;
}

bool BluezClient::replyPairingConfirmation(bool confirm) {
    sd_bus_message* m = nullptr;
    {
        std::lock_guard<std::mutex> lock(mPairingMutex);
        if (mPendingPairing.message == nullptr) return false;
        if (mPendingPairing.kind != 2) return false;   /* not a confirm req */
        m = mPendingPairing.message;
        mPendingPairing.message = nullptr;
        mPendingPairing.kind = -1;
    }
    std::lock_guard<std::mutex> lock(mBusMutex);
    int rc;
    if (confirm) {
        rc = sd_bus_reply_method_return(m, "");
    } else {
        sd_bus_error err = SD_BUS_ERROR_NULL;
        sd_bus_error_set(&err, "org.bluez.Error.Rejected", "rejected by user");
        rc = sd_bus_reply_method_error(m, &err);
        sd_bus_error_free(&err);
    }
    sd_bus_message_unref(m);
    return rc >= 0;
}

void BluezClient::cancelPairingReply() {
    sd_bus_message* m = nullptr;
    {
        std::lock_guard<std::mutex> lock(mPairingMutex);
        if (mPendingPairing.message == nullptr) return;
        m = mPendingPairing.message;
        mPendingPairing.message = nullptr;
        mPendingPairing.kind = -1;
    }
    std::lock_guard<std::mutex> lock(mBusMutex);
    if (mBus) {
        sd_bus_error err = SD_BUS_ERROR_NULL;
        sd_bus_error_set(&err, "org.bluez.Error.Canceled", "canceled by user");
        sd_bus_reply_method_error(m, &err);
        sd_bus_error_free(&err);
    }
    sd_bus_message_unref(m);
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
        /* Pairing-agent notifications fire HERE, off the bus lock: a
         * listener may answer synchronously (setPin -> reply), and the
         * reply takes mBusMutex — dispatching under processBus's lock
         * would self-deadlock the monitor. */
        flushDeferredPairing();

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
    out.clear();   /* replace semantics — signal values are whole values */
    uint8_t v = 0;
    if (sd_bus_message_enter_container(m, 'a', "y") < 0) return false;
    while (sd_bus_message_read_basic(m, 'y', &v) > 0) out.push_back(v);
    sd_bus_message_exit_container(m);
    return true;
}
bool readStringArrayProp(sd_bus_message* m, std::vector<std::string>& out) {
    out.clear();
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
    std::string adapterAlias, adapterAddress;
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
                    } else if (propName == "Address") {
                        readStringProp(reply, adapterAddress);
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
        mPowered = adapterPowered;
        mDiscovering = adapterDiscovering;
        mAlias = adapterAlias;
        mAdapterAddress = adapterAddress;
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
                if (sd_bus_message_read_basic(m, 'b', &v) >= 0) {
                    {   /* cache the value the getters serve */
                        std::lock_guard<std::mutex> lock(mCacheMutex);
                        if (name == "Powered") mPowered = v != 0;
                        else if (name == "Discovering") mDiscovering = v != 0;
                    }
                    if (mEvents) mEvents->onAdapterBoolChanged(name, v != 0);
                }
                sd_bus_message_exit_container(m);
            } else if (sd_bus_message_enter_container(m, 'v', "s") >= 0) {
                const char* s = nullptr;
                if (sd_bus_message_read_basic(m, 's', &s) >= 0 && s) {
                    {
                        std::lock_guard<std::mutex> lock(mCacheMutex);
                        if (name == "Alias") mAlias = s;
                        else if (name == "Address") mAdapterAddress = s;
                    }
                    if (mEvents) mEvents->onAdapterStringChanged(name, s);
                }
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
    std::vector<std::string> changedNames;
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
            changedNames.push_back(name);
        }
        sd_bus_message_exit_container(m);
        it->second = snapshot;   /* publish under the same lock */
    }
    /* Dispatch per changed property NAME — consumers branch on real
     * names ("Paired" drives the bond-state machine); the old "*"
     * wildcard left those branches unreachable. Fired OUTSIDE the
     * cache lock: listeners re-read the cache (getBondState ->
     * findDevice) and would self-deadlock inside it. */
    if (mEvents) {
        for (const std::string& name : changedNames)
            mEvents->onDevicePropertyChanged(snapshot, name);
    }
}

int BluezClient::onInterfacesAddedStatic(sd_bus_message* m, void* userdata,
                                         sd_bus_error*) {
    static_cast<BluezClient*>(userdata)->handleInterfacesAdded(m);
    return 0;
}

void BluezClient::handleInterfacesAdded(sd_bus_message* m) {
    /* oa{sa{sv}} — one pass collects every interface we track:
     * Device1 (discovery results), GattService1 and
     * GattCharacteristic1 (created by Device1.Connect on a real
     * bluetoothd — dropping them left discoverServices empty, review
     * finding #2). */
    const char* path = nullptr;
    if (sd_bus_message_read_basic(m, 'o', &path) < 0 || !path) return;
    const std::string objPath = path;

    if (sd_bus_message_enter_container(m, 'a', "{sa{sv}}") < 0) return;
    BluezDevice dev;
    dev.objectPath = objPath;
    bool isDevice = false;
    std::string svcUuid;
    bool isService = false;
    BluezGattCharacteristic gattChar;
    bool isCharacteristic = false;
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
            } else if (ifaceName == "org.bluez.GattService1") {
                isService = true;
                if (propName == "UUID") readStringProp(m, svcUuid);
                else sd_bus_message_skip(m, nullptr);
            } else if (ifaceName == "org.bluez.GattCharacteristic1") {
                isCharacteristic = true;
                gattChar.objectPath = objPath;
                applyGattCharProp(propName, m, gattChar);
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

    if (isDevice && !dev.address.empty()) {
        std::transform(dev.address.begin(), dev.address.end(),
                       dev.address.begin(), ::toupper);
        if (dev.alias.empty()) dev.alias = dev.name;
        {
            std::lock_guard<std::mutex> lock(mCacheMutex);
            mDevices[objPath] = dev;
        }
        if (mEvents) mEvents->onDeviceAdded(dev);
    }
    {
        std::lock_guard<std::mutex> lock(mCacheMutex);
        if (isService) {
            BluezGattService s;
            s.objectPath = objPath;
            s.uuid = svcUuid;
            const size_t pos = objPath.find("/service");
            s.devicePath = pos == std::string::npos ? std::string()
                        : objPath.substr(0, pos);
            mGattServices[objPath] = s;
        }
        if (isCharacteristic) {
            mGattCharacteristics[objPath] = gattChar;
        }
    }
}

int BluezClient::onInterfacesRemovedStatic(sd_bus_message* m, void* userdata,
                                           sd_bus_error*) {
    const char* path = nullptr;
    if (sd_bus_message_read_basic(m, 'o', &path) < 0 || !path) return 0;
    const std::string objPath = path;
    auto* self = static_cast<BluezClient*>(userdata);
    bool wasDevice = false;
    {
        std::lock_guard<std::mutex> lock(self->mCacheMutex);
        wasDevice = self->mDevices.erase(objPath) > 0;
        /* GATT objects go with their device — never left stale */
        self->mGattServices.erase(objPath);
        self->mGattCharacteristics.erase(objPath);
        for (auto it = self->mGattServices.begin();
                it != self->mGattServices.end();) {
            if (objPath == it->second.devicePath
                    || it->second.objectPath.rfind(objPath + "/", 0) == 0)
                it = self->mGattServices.erase(it);
            else
                ++it;
        }
        for (auto it = self->mGattCharacteristics.begin();
                it != self->mGattCharacteristics.end();) {
            if (it->second.objectPath.rfind(objPath + "/", 0) == 0
                    || it->second.servicePath.rfind(objPath + "/", 0) == 0)
                it = self->mGattCharacteristics.erase(it);
            else
                ++it;
        }
    }
    if (wasDevice && self->mEvents) self->mEvents->onDeviceRemoved(objPath);
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

std::string BluezClient::adapterPathLocked() const {
    std::lock_guard<std::mutex> lock(mCacheMutex);
    return mAdapterPath;   /* copy while the writer's lock is held */
}

/* Shared preamble for every adapter-scoped request: a connected
 * transport plus a non-empty adapter path (refreshing once when the
 * cache predates a hotplug). */
bool BluezClient::ensureAdapter(std::string& path) {
    path = adapterPathLocked();
    if (!path.empty() && connect()) return true;
    if (!connect()) return false;
    if (!refreshManagedObjects()) return false;
    path = adapterPathLocked();
    return !path.empty();
}

std::string BluezClient::pathForAddressLocked(const std::string& address) const {
    for (const auto& kv : mDevices) {
        if (kv.second.address == address) return kv.first;
    }
    return std::string();
}

/* Cache-first property getters: the snapshot + PropertiesChanged
 * signals maintain Powered/Discovering/Alias/Address, so an app-thread
 * getter never pays a bus round trip and a monitor-thread listener can
 * never self-deadlock on mBusMutex (the review's design finding). */
bool BluezClient::getAdapterBool(const std::string& name, bool& value) const {
    std::lock_guard<std::mutex> lock(mCacheMutex);
    if (name == "Powered") { value = mPowered; return true; }
    if (name == "Discovering") { value = mDiscovering; return true; }
    return false;
}

bool BluezClient::getAdapterString(const std::string& name, std::string& value) const {
    std::lock_guard<std::mutex> lock(mCacheMutex);
    if (name == "Alias") { value = mAlias; return !mAlias.empty(); }
    if (name == "Address") { value = mAdapterAddress; return !mAdapterAddress.empty(); }
    return false;
}

bool BluezClient::setAdapterBool(const std::string& name, bool value) {
    std::string adapterPath;
    if (!ensureAdapter(adapterPath)) return false;
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    bool ok;
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        ok = sd_bus_call_method(mBus, kBluezService, adapterPath.c_str(), kPropIface,
                                "Set", &err, &reply, "ssv", kAdapterIface,
                                name.c_str(), "b", value ? 1 : 0) >= 0;
    }
    if (!ok && err.message) LOGD("set %s failed: %s", name.c_str(), err.message);
    sd_bus_message_unrefp(&reply);
    sd_bus_error_free(&err);
    return ok;
}

bool BluezClient::setAdapterString(const std::string& name, const std::string& value) {
    std::string adapterPath;
    if (!ensureAdapter(adapterPath)) return false;
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    bool ok;
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        ok = sd_bus_call_method(mBus, kBluezService, adapterPath.c_str(), kPropIface,
                                "Set", &err, &reply, "ssv", kAdapterIface,
                                name.c_str(), "s", value.c_str()) >= 0;
    }
    sd_bus_message_unrefp(&reply);
    sd_bus_error_free(&err);
    return ok;
}

bool BluezClient::adapterCall(const char* method) {
    std::string adapterPath;
    if (!ensureAdapter(adapterPath)) return false;
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    bool ok;
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        ok = sd_bus_call_method(mBus, kBluezService, adapterPath.c_str(),
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

static int pairNoReply(sd_bus_message* /*reply*/, void*, sd_bus_error*) {
    return 0;   /* outcome arrives as the Paired property signal */
}

bool BluezClient::pairDevice(const std::string& address) {
    /* AOSP createBond() is fire-and-forget: the Pair round trip spans
     * the whole agent negotiation (minutes with a human at the other
     * end), so a synchronous call would sit on the bus lock and time
     * out. Send it async; the bond result lands via PropertiesChanged
     * (Paired) like ACTION_BOND_STATE_CHANGED. */
    std::string path;
    {
        std::lock_guard<std::mutex> lock(mCacheMutex);
        path = pathForAddressLocked(address);
    }
    if (path.empty()) return false;
    std::lock_guard<std::mutex> lock(mBusMutex);
    if (!mBus) return false;
    sd_bus_message* msg = nullptr;
    if (sd_bus_message_new_method_call(mBus, &msg, kBluezService,
            path.c_str(), kDeviceIface, "Pair") < 0) return false;
    /* The reply-slot must be owned: a NULL slot with a callback is not a
     * legal sd-bus call-async form (260 asserts ownership; a stack-dead
     * slot corrupts the pending-reply bookkeeping — seen as spurious
     * UnknownMethod replies for unrelated in-flight calls). */
    if (mPairSlot) sd_bus_slot_unref(mPairSlot);
    const int rc = sd_bus_call_async(mBus, &mPairSlot, msg, pairNoReply, this, 0);
    sd_bus_message_unref(msg);
    return rc >= 0;
}

bool BluezClient::removeDevice(const std::string& address) {
    std::string path;
    {
        std::lock_guard<std::mutex> lock(mCacheMutex);
        path = pathForAddressLocked(address);
    }
    if (path.empty()) return false;
    std::string adapterPath;
    if (!ensureAdapter(adapterPath)) return false;
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    bool ok;
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        if (!mBus) return false;
        ok = sd_bus_call_method(mBus, kBluezService, adapterPath.c_str(),
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
    std::string adapterPath;
    if (!ensureAdapter(adapterPath)) return false;
    /* std::string -> char* vector for sd_bus_message_append_strv */
    std::vector<char*> strv;
    for (const std::string& u : uuidFilter)
        strv.push_back(const_cast<char*>(u.c_str()));
    strv.push_back(nullptr);

    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* msg = nullptr;
    sd_bus_message* reply = nullptr;
    bool ok = false;
    {
        std::lock_guard<std::mutex> lock(mBusMutex);
        if (!mBus) return false;
        ok = sd_bus_message_new_method_call(mBus, &msg, kBluezService,
                adapterPath.c_str(), kAdapterIface, "SetDiscoveryFilter") >= 0
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
                "WriteValue") >= 0
          && sd_bus_message_append_array(msg, 'y', value.data(),
                                         value.size()) >= 0
          && sd_bus_message_open_container(msg, 'a', "{sv}") >= 0
          && sd_bus_message_open_container(msg, 'e', "sv") >= 0
          && sd_bus_message_append(msg, "s", "type") >= 0
          && sd_bus_message_append(msg, "v", "s",
                  withoutResponse ? "command" : "request") >= 0
          && sd_bus_message_close_container(msg) >= 0
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
