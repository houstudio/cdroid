/*********************************************************************************
 * Port of AOSP 12 SettingsLib bluetooth facade:
 *   frameworks/base/packages/SettingsLib/src/com/android/settingslib/bluetooth/
 *     LocalBluetoothAdapter.java  — radio state/discovery/identity over the
 *                                  framework BluetoothAdapter (here: cdblue's)
 *     BluetoothEventManager.java  — broadcast -> BluetoothCallback fan-out
 *     LocalBluetoothManager.java  — the process singleton tying adapter +
 *                                  CachedBluetoothDeviceManager + events
 *
 * Threading contract: cdblue listener callbacks arrive on the BlueZ monitor
 * thread; BluetoothEventManager marshals every dispatch onto the main looper
 * (AOSP delivers on the main thread via the broadcast system, which does not
 * exist yet — the cdnet module-phase decision).
 *********************************************************************************/
#ifndef PREFERENCEDRMO_LOCAL_BLUETOOTH_MANAGER_H
#define PREFERENCEDRMO_LOCAL_BLUETOOTH_MANAGER_H

#include <string>
#include <vector>

#include <bluetoothadapter.h>
#include <bluetoothdevice.h>

namespace preferencedemo {

class CachedBluetoothDevice;
class CachedBluetoothDeviceManager;
class LocalBluetoothManager;

/** SettingsLib BluetoothCallback: default-empty methods, AOSP shape. */
class BluetoothCallback {
public:
    virtual ~BluetoothCallback() = default;
    virtual void onBluetoothStateChanged(int bluetoothState) {}
    virtual void onScanningStateChanged(bool started) {}
    virtual void onDeviceAdded(CachedBluetoothDevice* cachedDevice) {}
    virtual void onDeviceDeleted(CachedBluetoothDevice* cachedDevice) {}
    virtual void onDeviceBondStateChanged(CachedBluetoothDevice* cachedDevice,
                                          int bondState) {}
    virtual void onDeviceAttributesChanged(CachedBluetoothDevice* cachedDevice) {}
};

/** SettingsLib LocalBluetoothAdapter: the adapter surface the Settings UI
 *  drives. All methods are synchronous (cdblue D-Bus on the caller thread). */
class LocalBluetoothAdapter {
public:
    int getState() const { return mAdapter.getState(); }
    bool isEnabled() const { return mAdapter.isEnabled(); }
    bool enable() const { return mAdapter.enable(); }
    bool disable() const { return mAdapter.disable(); }

    std::string getAddress() const { return mAdapter.getAddress(); }
    std::string getName() const { return mAdapter.getName(); }
    bool setName(const std::string& name) { return mAdapter.setName(name); }

    bool startDiscovery() const { return mAdapter.startDiscovery(); }
    bool cancelDiscovery() const { return mAdapter.cancelDiscovery(); }
    bool isDiscovering() const { return mAdapter.isDiscovering(); }

    cdroid::BluetoothAdapter& raw() { return mAdapter; }

private:
    friend class LocalBluetoothManager;
    explicit LocalBluetoothAdapter(cdroid::BluetoothAdapter& adapter) : mAdapter(adapter) {}
    cdroid::BluetoothAdapter& mAdapter;
};

/** SettingsLib BluetoothEventManager: cdblue listeners -> BluetoothCallback
 *  fan-out on the main looper + CachedBluetoothDeviceManager bookkeeping. */
class BluetoothEventManager
        : private cdroid::BluetoothAdapter::AdapterStateListener,
          private cdroid::BluetoothAdapter::DiscoveryListener,
          private cdroid::BluetoothAdapter::BondStateListener {
public:
    void registerCallback(BluetoothCallback* callback);
    void unregisterCallback(BluetoothCallback* callback);

private:
    friend class LocalBluetoothManager;
    BluetoothEventManager(CachedBluetoothDeviceManager* deviceManager);
    void post(std::function<void()> fn);   // monitor thread -> main looper

    // cdroid::BluetoothAdapter listeners (BlueZ monitor thread).
    void onAdapterStateChanged(int newState, int prevState) override;
    void onDiscoveryStarted() override;
    void onDeviceFound(const cdroid::BluetoothDevice& device) override;
    void onDiscoveryFinished() override;
    void onBondStateChanged(const cdroid::BluetoothDevice& device,
                            int bondState, int prevState) override;

    CachedBluetoothDeviceManager* mDeviceManager;
    std::vector<BluetoothCallback*> mCallbacks;   // main thread only
};

/** SettingsLib LocalBluetoothManager: process singleton. */
class LocalBluetoothManager {
public:
    static LocalBluetoothManager* getInstance();

    LocalBluetoothAdapter* getBluetoothAdapter() { return &mLocalAdapter; }
    CachedBluetoothDeviceManager* getCachedDeviceManager() { return mDeviceManager.get(); }
    BluetoothEventManager* getEventManager() { return &mEventManager; }

private:
    LocalBluetoothManager();

    LocalBluetoothAdapter mLocalAdapter;
    std::unique_ptr<CachedBluetoothDeviceManager> mDeviceManager;
    BluetoothEventManager mEventManager;
};

} // namespace preferencedemo

#endif // PREFERENCEDRMO_LOCAL_BLUETOOTH_MANAGER_H
