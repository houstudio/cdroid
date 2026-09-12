#ifndef __CDROID_BLUETOOTH_ADAPTER_H__
#define __CDROID_BLUETOOTH_ADAPTER_H__

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include <bluetoothpairing.h>
#include <bluetoothprofile.h>
#include <bluetoothdevice.h>
#include <bluetoothsocket.h>
#include <bluetoothuuid.h>
#include <bluezclient.h>

namespace cdroid {

class BluetoothLeScanner;

/**
 * Port of android.bluetooth.BluetoothAdapter (android-36), module phase.
 * Backed directly by BlueZ over the system D-Bus (BluezClient) instead of
 * the IBluetooth binder service. Module phase = process singleton via
 * getDefaultAdapter() (the AOSP factory method name); Context#
 * getSystemService(BLUETOOTH_SERVICE) lands when this library graduates
 * into src/gui/bluetooth — the same graduation path cdnet documents.
 *
 * Threading contract is cdnet's: synchronous methods issue D-Bus calls on
 * the caller thread; listener callbacks arrive on the BluezClient monitor
 * thread unless the app marshals them (View::post / a main-looper
 * Handler, the pattern every demo uses).
 *
 * State fidelity note: BlueZ exposes a boolean Powered property, so the
 * STATE_TURNING_* intermediate states are synthesized locally — enable()
 * reports TURNING_ON and disable() TURNING_OFF until the Powered
 * PropertiesChanged lands. The BLE_* adapter states (14/15/16) are a
 * single-controller nicety of AOSP's dual-stack; they are never reported
 * here.
 */
class BluetoothAdapter : private BluezClient::Events {
public:
    /* --- adapter state (getState) ------------------------------------- */
    static constexpr int STATE_OFF = 10;
    static constexpr int STATE_TURNING_ON = 11;
    static constexpr int STATE_ON = 12;
    static constexpr int STATE_TURNING_OFF = 13;
    static constexpr int STATE_BLE_TURNING_ON = 14;
    static constexpr int STATE_BLE_ON = 15;
    static constexpr int STATE_BLE_TURNING_OFF = 16;

    /* Broadcast intents, kept as constants for the future broadcast
     * system (same strings the Java side broadcasts). */
    static constexpr const char* ACTION_STATE_CHANGED =
            "android.bluetooth.adapter.action.STATE_CHANGED";
    static constexpr const char* ACTION_DISCOVERY_STARTED =
            "android.bluetooth.adapter.action.DISCOVERY_STARTED";
    static constexpr const char* ACTION_DISCOVERY_FINISHED =
            "android.bluetooth.adapter.action.DISCOVERY_FINISHED";
    static constexpr const char* ACTION_LOCAL_NAME_CHANGED =
            "android.bluetooth.adapter.action.LOCAL_NAME_CHANGED";
    static constexpr const char* ACTION_REQUEST_ENABLE =
            "android.bluetooth.adapter.action.REQUEST_ENABLE";
    static constexpr const char* ACTION_REQUEST_DISCOVERABLE =
            "android.bluetooth.adapter.action.REQUEST_DISCOVERABLE";

    /* Extra keys (state changes, discovery results). */
    static constexpr const char* EXTRA_STATE = "android.bluetooth.adapter.extra.STATE";
    static constexpr const char* EXTRA_PREVIOUS_STATE =
            "android.bluetooth.adapter.extra.PREVIOUS_STATE";
    static constexpr const char* EXTRA_DEVICE = "android.bluetooth.device.extra.DEVICE";
    static constexpr const char* EXTRA_RSSI = "android.bluetooth.device.extra.RSSI";
    static constexpr const char* EXTRA_CLASS = "android.bluetooth.device.extra.CLASS";

    /* Get a handle to the default local Bluetooth adapter (AOSP name).
     * The first call binds the BlueZ transport and starts the event
     * pump; with no bluetoothd on the bus the adapter reports STATE_OFF
     * and keeps retrying in the background. */
    static BluetoothAdapter& getDefaultAdapter();

    /* --- radio power --------------------------------------------------- */
    bool enable();
    bool disable();
    bool isEnabled();
    int getState();

    /* --- identity ------------------------------------------------------ */
    /* Hardware address of the adapter ("AA:BB:CC:DD:EE:FF"); empty when
     * no adapter is present. AOSP returns a null-analog here. */
    std::string getAddress();
    /* Friendly local name / set it (Adapter1 Alias property). */
    std::string getName();
    bool setName(const std::string& name);

    /* --- discovery ------------------------------------------------------ */
    bool startDiscovery();
    bool cancelDiscovery();
    bool isDiscovering();

    /* --- RFCOMM listeners ------------------------------------------------- */
    /* Listen for RFCOMM connections on the given channel (AOSP
     * listenUsingRfcommOn — no SDP registration, clients connect by the
     * agreed channel). */
    BluetoothServerSocket* listenUsingRfcommOn(int channel);
    BluetoothServerSocket* listenUsingInsecureRfcommOn(int channel);
    /* listenUsingRfcommWithServiceRecord(name, uuid): the SDP record
     * registration lands with the SDP resolver; the listener itself is
     * fully live (bound to the RFCOMM channel SPP convention, 1, when
     * uuid is SerialPort(), else nullptr). */
    BluetoothServerSocket* listenUsingRfcommWithServiceRecord(
            const std::string& name, const BluetoothUuid& uuid);
    BluetoothServerSocket* listenUsingInsecureRfcommWithServiceRecord(
            const std::string& name, const BluetoothUuid& uuid);

    /* --- remote devices -------------------------------------------------- */
    /* Get a BluetoothDevice object for the given hardware address.
     * Valid regardless of whether the address has been seen. */
    BluetoothDevice getRemoteDevice(const std::string& address);
    /* Currently paired/bonded remote devices (BlueZ device cache with
     * Paired=true; AOSP returns the same information as a Set). */
    std::vector<BluetoothDevice> getBondedDevices();
    /* The BLE scanner (AOSP entry point); null-analog when no adapter. */
    BluetoothLeScanner* getBluetoothLeScanner();

    /* --- listeners (not owned; add/remove pairs, thread-safe) ------------
     * Interim listener surfaces — one per broadcast action — until the
     * broadcast/receiver system exists (the cdnet module-phase decision). */
    class AdapterStateListener {
    public:
        virtual ~AdapterStateListener() = default;
        virtual void onAdapterStateChanged(int newState, int prevState) = 0;
    };
    class DiscoveryListener {
    public:
        virtual ~DiscoveryListener() = default;
        /* ACTION_DISCOVERY_STARTED. */
        virtual void onDiscoveryStarted() {}
        /* ACTION_FOUND — also fired on Name/RSSI updates of an already
         * known device while discovering, like the Java sticky updates. */
        virtual void onDeviceFound(const BluetoothDevice& device) {}
        /* ACTION_DISCOVERY_FINISHED. */
        virtual void onDiscoveryFinished() {}
    };
    class BondStateListener {
    public:
        virtual ~BondStateListener() = default;
        virtual void onBondStateChanged(const BluetoothDevice& device,
                                        int bondState, int prevState) = 0;
    };

    void addAdapterStateListener(AdapterStateListener* listener);
    void removeAdapterStateListener(AdapterStateListener* listener);
    void addDiscoveryListener(DiscoveryListener* listener);
    void removeDiscoveryListener(DiscoveryListener* listener);
    void addBondStateListener(BondStateListener* listener);
    void removeBondStateListener(BondStateListener* listener);

    /* --- profile proxies ----------------------------------------------------- */
    /* AOSP getProfileProxy: hands the caller the profile proxy through
     * the ServiceListener (synchronously here — in-process profiles).
     * A2DP/HEADSET are faithful stubs until the audio pipeline lands. */
    bool getProfileProxy(BluetoothProfile::ServiceListener* listener,
                         int profile);
    void closeProfileProxy(int profile, BluetoothProfile* proxy);

    /* --- pairing agent ----------------------------------------------------- */
    /* Register the pairing agent. capability: "DisplayYesNo" (a UI will
     * answer pairing) or "NoInputNoOutput" (just-works). Must be called
     * before createBond() when interactive pairing is wanted. */
    bool registerPairingAgent(const std::string& capability);
    void addPairingListener(BluetoothPairingListener* listener);
    void removePairingListener(BluetoothPairingListener* listener);
    /* BluetoothDevice.setPin / setPasskey / setPairingConfirmation land
     * here (the ACTION_PAIRING_REQUEST answer API). */
    bool replyPairingPin(const std::string& pin);
    bool replyPairingPasskey(uint32_t passkey);
    bool replyPairingConfirmation(bool confirm);
    void cancelPairingUserInput();

    /* --- internal (BluetoothDevice resolve path; do not use) ------------- */
    BluezClient& client() { return mClient; }
    /* GATT session fan-out (characteristic Value changes). */
    void registerGattSession(BluetoothGatt* session);
    void unregisterGattSession(BluetoothGatt* session);
    bool resolveDeviceName(const std::string& address, std::string& out) const;
    bool resolveDeviceAlias(const std::string& address, std::string& out) const;
    bool setDeviceAlias(const std::string& address, const std::string& alias);
    int resolveBondState(const std::string& address) const;
    int resolveDeviceType(const std::string& address) const;
    int resolveDeviceClass(const std::string& address) const;
    bool bondDevice(const std::string& address);
    bool unbondDevice(const std::string& address);

private:
    BluetoothAdapter();
    ~BluetoothAdapter() override;
    BluetoothAdapter(const BluetoothAdapter&) = delete;
    BluetoothAdapter& operator=(const BluetoothAdapter&) = delete;

    /* BluezClient::Events — monitor thread. */
    void onAdapterBoolChanged(const std::string& name, bool value) override;
    void onAdapterStringChanged(const std::string& name,
                                const std::string& value) override;
    void onDeviceAdded(const BluezDevice& device) override;
    void onDevicePropertyChanged(const BluezDevice& device,
                                 const std::string& name) override;
    void onDeviceRemoved(const std::string& objectPath) override;
    void onBluezDisconnected() override;
    void onBluezReconnected() override;
    void onGattCharacteristicChanged(const BluezGattCharacteristic& ch) override;
    void onPairingPinRequested(const std::string& address) override;
    void onPairingPasskeyRequested(const std::string& address) override;
    void onPairingConfirmationRequested(const std::string& address) override;
    void onDisplayPasskey(const std::string& address, uint32_t passkey) override;
    void onPairingCancelled() override;

    void setStateAndNotify(int newState);
    void dispatchFound(const BluezDevice& device);

    BluezClient& mClient;

    std::mutex mStateMutex;
    int mAdapterState = STATE_OFF;
    bool mDiscovering = false;

    /* objectPath -> bond state snapshot for listener prev-values */
    std::map<std::string, int> mBondStates;

    std::mutex mListenersMutex;
    std::vector<AdapterStateListener*> mStateListeners;
    std::vector<DiscoveryListener*> mDiscoveryListeners;
    std::vector<BondStateListener*> mBondListeners;
    std::vector<BluetoothGatt*> mGattSessions;   /* guarded by mStateMutex */
    std::vector<BluetoothPairingListener*> mPairingListeners;
    BluetoothLeScanner* mLeScanner = nullptr;
};

} // namespace cdroid

#endif /* __CDROID_BLUETOOTH_ADAPTER_H__ */
