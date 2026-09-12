#ifndef __BLUEZ_CLIENT_H__
#define __BLUEZ_CLIENT_H__

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <utility>
#include <string>
#include <thread>
#include <vector>

#include <bluetoothdevice.h>

struct sd_bus;
struct sd_bus_message;
struct sd_bus_slot;
struct sd_bus_error;

namespace cdroid {

/**
 * Snapshot of the BlueZ org.bluez.Device1 properties CDROID tracks per
 * remote device. Property names are the D-Bus ones (BlueZ doc), values
 * already converted to the android.bluetooth shapes (short RSSI, CoD
 * int, address upper-cased like the Java side expects).
 */
/* org.bluez.GattService1 / GattCharacteristic1 snapshot records. */
struct BluezGattService {
    std::string objectPath;
    std::string devicePath;
    std::string uuid;
};
struct BluezGattCharacteristic {
    std::string objectPath;
    std::string servicePath;
    std::string uuid;
    std::vector<std::string> flags;   /* "read","write","notify",... */
    std::vector<uint8_t> value;
    bool notifying = false;
};

struct BluezDevice {
    std::string objectPath;   /* "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF" */
    std::string address;      /* "AA:BB:CC:DD:EE:FF" */
    std::string name;         /* remote name (may be empty until resolved) */
    std::string alias;        /* user-writable alias (defaults to name) */
    std::string addressType;  /* "public"/"random" — present on LE devices */
    bool paired = false;
    bool connected = false;
    bool trusted = false;
    int16_t rssi = 0;         /* 0 = unknown; valid values negative */
    uint32_t cod = 0;         /* Class of Device ("Class" property) */
    std::vector<std::string> uuids;
};

/**
 * Blocking request/reply + signal transport to a running bluetoothd over
 * the system D-Bus (sd-bus from libsystemd). AOSP counterpart: the
 * framework talks to com.android.bluetooth over binder (IBluetooth.*);
 * in CDROID's single-process world this class is that transport, with
 * BlueZ playing the role of the Bluetooth stack process.
 *
 * Structure mirrors SupplicantClient (src/net/wifi):
 *  - synchronous request methods on the caller thread (serialized by
 *    mBusMutex — one sd_bus object is not safe for concurrent calls)
 *  - a dedicated monitor thread pumping the bus (sd_bus_process) which
 *    dispatches PropertiesChanged / InterfacesAdded / InterfacesRemoved
 *    to the Events callback — the WifiMonitor analog
 *  - no bluetoothd / no system bus: connect() fails, isConnected() stays
 *    false and the monitor keeps retrying in the background so the
 *    adapter appears when a dongle is plugged in
 *
 * The object cache (devices per adapter) is built from
 * ObjectManager.GetManagedObjects at connect and maintained from the
 * signals, so BluetoothAdapter's synchronous getters never touch D-Bus.
 */
class BluezClient {
public:
    class Events {
    public:
        virtual ~Events() = default;
        /* Adapter-level property flips with their VALUES. Handlers run on
         * the monitor thread inside the bus lock — they must never issue
         * a synchronous bus call back into this client (self-deadlock);
         * that is why the values ride along instead of being re-read. */
        virtual void onAdapterBoolChanged(const std::string& name, bool value) {}
        virtual void onAdapterStringChanged(const std::string& name,
                                            const std::string& value) {}
        /* A Device1 object appeared (discovery result or paired device
         * coming back into the cache). */
        virtual void onDeviceAdded(const BluezDevice& device) {}
        /* A Device1 property changed (Name/Alias/Paired/RSSI/Connected...). */
        virtual void onDevicePropertyChanged(const BluezDevice& device,
                                             const std::string& name) {}
        /* A Device1 object disappeared (unpaired/removed). */
        virtual void onDeviceRemoved(const std::string& objectPath) {}
        /* A GATT characteristic's Value/Notifying property changed. */
        virtual void onGattCharacteristicChanged(const BluezGattCharacteristic& ch) {}
        /* D-Bus/bluez service lost and re-established (daemon restart). */
        virtual void onBluezDisconnected() {}
        virtual void onBluezReconnected() {}
        /* Pairing agent requests (org.bluez.Agent1). The pending daemon
         * request is held until replyPairing*() answers it. */
        virtual void onPairingPinRequested(const std::string& address) {}
        virtual void onPairingPasskeyRequested(const std::string& address) {}
        virtual void onPairingConfirmationRequested(const std::string& address,
                                                   uint32_t passkey) {}
        /* BlueZ consent-only methods (RequestAuthorization /
         * AuthorizeService): no passkey exists — AOSP surfaces these
         * as PAIRING_VARIANT_CONSENT. */
        virtual void onPairingConsentRequested(const std::string& address) {}
        virtual void onDisplayPasskey(const std::string& address, uint32_t passkey) {}
        virtual void onPairingCancelled() {}
    };

    explicit BluezClient(Events* events);
    ~BluezClient();

    BluezClient(const BluezClient&) = delete;
    BluezClient& operator=(const BluezClient&) = delete;

    /* Connect to the system bus and locate org.bluez. Never blocks longer
     * than one round trip; safe to call repeatedly. */
    bool connect();
    bool isConnected() const { return mConnected.load(); }
    /* true when org.bluez owns objects on the bus (bluetoothd running);
     * both read through the cache lock (mAdapterPath is written there). */
    bool hasAdapter() const { return !adapterPathLocked().empty(); }
    std::string adapterPath() const { return adapterPathLocked(); }

    /* --- synchronous requests (caller thread) --------------------------- */

    /* Adapter1 property access. Cached reads (the snapshot + signals
     * maintain Powered/Discovering/Alias) never touch the bus; the
     * request methods below do. Returns false on type mismatch,
     * unknown property, or transport failure. */
    bool getAdapterBool(const std::string& name, bool& value) const;
    bool getAdapterString(const std::string& name, std::string& value) const;
    bool setAdapterBool(const std::string& name, bool value);
    bool setAdapterString(const std::string& name, const std::string& value);

    bool startDiscovery();
    bool cancelDiscovery();
    /* Device1.Pair — object path resolved from the device cache. */
    bool pairDevice(const std::string& address);
    /* Adapter1.RemoveDevice (forget). */
    bool removeDevice(const std::string& address);
    bool setDeviceAlias(const std::string& address, const std::string& alias);

    /* --- BLE -------------------------------------------------------------- */

    /* Adapter1.SetDiscoveryFilter + StartDiscovery in one shot:
     * transport "le"/"auto", service-uuid filter list (may be empty),
     * rssi threshold (INT16_MIN = unset). */
    bool startLeDiscovery(const std::vector<std::string>& uuidFilter,
                          int16_t rssiThreshold);
    /* Device1.Connect / Disconnect (the GATT bearer). */
    bool connectDevice(const std::string& address);
    bool disconnectDevice(const std::string& address);

    /* --- GATT cache (from GetManagedObjects, refreshed by signals) ------- */
    std::vector<BluezGattService> getGattServices(const std::string& deviceAddress) const;
    std::vector<BluezGattCharacteristic> getGattCharacteristics(
            const std::string& servicePath) const;
    /* ReadValue/WriteValue on the characteristic object path; write
     * without response uses the type flag. Returns the byte vector
     * (read) / success (write). */
    bool gattRead(const std::string& characteristicPath,
                  std::vector<uint8_t>& out);
    bool gattWrite(const std::string& characteristicPath,
                   const std::vector<uint8_t>& value, bool withoutResponse);
    bool gattSetNotify(const std::string& characteristicPath, bool enable);

    /* --- pairing agent ----------------------------------------------------- */
    /* Register an org.bluez.Agent1 on the shared connection and make it
     * the default. Capability: "DisplayYesNo" (interactive UI) or
     * "NoInputNoOutput" (just-works). Requests surface through Events
     * and stay pending until the reply calls below. */
    bool registerAgent(const std::string& capability);
    /* Answer the pending request (from BluetoothDevice.setPin /
     * setPairingConfirmation / cancelPairingUserInput). */
    bool replyPairingPin(const std::string& pin);
    bool replyPairingPasskey(uint32_t passkey);
    bool replyPairingConfirmation(bool confirm);
    void cancelPairingReply();

    /* --- device cache (maintained by the monitor; copy under lock) ------ */
    std::vector<BluezDevice> getDevices() const;
    bool findDevice(const std::string& address, BluezDevice& out) const;

    /* Re-enumerate ObjectManager state into the caches (also public
     * for BluetoothGatt::discoverServices, which refreshes after the
     * connect so late-arriving GATT objects are visible). */
    bool refreshManagedObjects();

private:
    /* Deferred Events dispatch: every callback is queued while the bus
     * lock is held (processBus) and run after it releases, so listener
     * code may make synchronous client calls freely. */
    std::vector<std::function<void(Events*)>> mDeferredEvents;
    void queueEvent(std::function<void(Events*)> fn);
    void flushDeferredEvents();
    /* shared adapter-request preamble (connect + non-empty path) */
    bool ensureAdapter(std::string& path);
    std::string adapterPathLocked() const;
    /* agent vtable callbacks */
    static int agentRequestPinCode(sd_bus_message* m, void* userdata, sd_bus_error* e);
    static int agentRequestPasskey(sd_bus_message* m, void* userdata, sd_bus_error* e);
    static int agentRequestConfirmation(sd_bus_message* m, void* userdata, sd_bus_error* e);
    static int agentRequestAuthorization(sd_bus_message* m, void* userdata, sd_bus_error* e);
    static int agentAuthorizeService(sd_bus_message* m, void* userdata, sd_bus_error* e);
    static int agentDisplayPasskey(sd_bus_message* m, void* userdata, sd_bus_error* e);
    static int agentDisplayPinCode(sd_bus_message* m, void* userdata, sd_bus_error* e);
    static int agentCancel(sd_bus_message* m, void* userdata, sd_bus_error* e);
    static int agentRelease(sd_bus_message* m, void* userdata, sd_bus_error* e);
    /* hold/answer the pending agent request */
    void holdPendingPairing(sd_bus_message* m, const std::string& address, int kind);
    std::string addressForDevicePath(const std::string& objectPath) const;

    /* monitor thread */
    void monitorLoop();
    void stopMonitor();
    /* one pass over pending bus messages; true when something was handled */
    bool processBus();
    void clearCache();
    void resetAdapterPropsLocked();
    void onDaemonLost();

    /* signal handlers (raw sd_bus callbacks forward into these) */
    void handlePropertiesChanged(sd_bus_message* m);
    void handleInterfacesAdded(sd_bus_message* m);
    void handleInterfacesRemoved(sd_bus_message* m);

    /* call a void method on the adapter object */
    bool adapterCall(const char* method);
    /* call a void method on a device object */
    bool deviceCall(const std::string& address, const char* method);
    std::string pathForAddressLocked(const std::string& address) const;

    static int onPropertiesChangedStatic(sd_bus_message* m, void* userdata,
                                         sd_bus_error* retError);
    static int onInterfacesAddedStatic(sd_bus_message* m, void* userdata,
                                       sd_bus_error* retError);
    static int onInterfacesRemovedStatic(sd_bus_message* m, void* userdata,
                                         sd_bus_error* retError);
    static int onNameOwnerChangedStatic(sd_bus_message* m, void* userdata,
                                        sd_bus_error* retError);

    Events* mEvents;
    sd_bus* mBus = nullptr;
    sd_bus_slot* mSlotProperties = nullptr;
    sd_bus_slot* mSlotIfAdded = nullptr;
    sd_bus_slot* mSlotIfRemoved = nullptr;
    sd_bus_slot* mSlotNameOwner = nullptr;

    std::string mAdapterPath;          /* "/org/bluez/hci0" */

    mutable std::mutex mCacheMutex;
    std::map<std::string, BluezDevice> mDevices;   /* objectPath -> device */
    std::map<std::string, BluezGattService> mGattServices;
    std::map<std::string, BluezGattCharacteristic> mGattCharacteristics;

    /* Request kinds aligned to the AOSP PAIRING_VARIANT_* they
     * surface as (PIN/PASSKEY/PASSKEY_CONFIRMATION/CONSENT); replies
     * are kind-checked so a passkey request is never answered with a
     * PIN signature. */
    struct PendingPairing {
        enum Kind { NONE = -1, PIN = 0, PASSKEY = 1,
                    CONFIRMATION = 2, CONSENT = 3 };
        sd_bus_message* message = nullptr;   /* ref-held, replied later */
        std::string address;
        Kind kind = NONE;
    };
    std::mutex mPairingMutex;
    PendingPairing mPendingPairing;
    sd_bus_slot* mPairSlot = nullptr;   /* async Pair reply slot */
    sd_bus_slot* mAgentSlot = nullptr;  /* Agent1 vtable slot */

    std::string mAgentCapability;      /* re-registered on every reconnect */

    /* Adapter property cache — written from the snapshot and the
     * PropertiesChanged signals (under mCacheMutex), read by the
     * value-returning getters below so an app-thread getter never pays
     * a bus round trip (and monitor-thread listeners can never
     * self-deadlock on mBusMutex). */
    bool mPowered = false;
    bool mDiscovering = false;
    std::string mAlias;
    std::string mAdapterAddress;

    std::mutex mBusMutex;              /* serializes sd_bus request calls */
    std::thread mMonitorThread;
    std::atomic<bool> mRunning { false };
    std::atomic<bool> mConnected { false };
};

} // namespace cdroid

#endif /* __BLUEZ_CLIENT_H__ */
