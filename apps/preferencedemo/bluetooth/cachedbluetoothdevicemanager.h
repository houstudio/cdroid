/*********************************************************************************
 * Port of AOSP 12 SettingsLib CachedBluetoothDeviceManager.java:
 *   frameworks/base/packages/SettingsLib/src/com/android/settingslib/
 *   bluetooth/CachedBluetoothDeviceManager.java
 *
 * Address -> CachedBluetoothDevice map; devices enter on ACTION_FOUND /
 * bonded-list reads and leave on unbond. Lives for the process (the
 * BluetoothEventManager outlives every screen), so raw pointers handed to
 * the UI stay valid; rows unregister callbacks on removal.
 *********************************************************************************/
#ifndef PREFERENCEDRMO_CACHED_BLUETOOTH_DEVICE_MANAGER_H
#define PREFERENCEDRMO_CACHED_BLUETOOTH_DEVICE_MANAGER_H

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "cachedbluetoothdevice.h"

namespace preferencedemo {

class CachedBluetoothDeviceManager {
public:
    explicit CachedBluetoothDeviceManager(LocalBluetoothAdapter* localAdapter);

    std::vector<CachedBluetoothDevice*> getCachedDevicesCopy() const;

    /** AOSP findDevice: cache lookup by address, nullptr when unseen. */
    CachedBluetoothDevice* findDevice(const std::string& address);

    /** AOSP onDeviceAdded: mint (or return) the cache entry for an address.
     *  Returns nullptr while the adapter is off (AOSP drops the event). */
    CachedBluetoothDevice* onDeviceAdded(const cdroid::BluetoothDevice& device);

    /** AOSP onDeviceDeleted: drop the entry (after unbond). */
    void onDeviceDeleted(CachedBluetoothDevice* cachedDevice);

    LocalBluetoothAdapter* getLocalAdapter() const { return mLocalAdapter; }

private:
    friend class BluetoothEventManager;
    void onBluetoothStateChanged(int state);
    /* No-GC seam: deleting an entry must first let every BluetoothCallback
     * drop its rows (AOSP leans on GC here); the event manager installs the
     * fan-out. */
    void setOnDeviceDeletedHook(std::function<void(CachedBluetoothDevice*)> hook) {
        mOnDeviceDeletedHook = std::move(hook);
    }

    LocalBluetoothAdapter* mLocalAdapter;
    std::function<void(CachedBluetoothDevice*)> mOnDeviceDeletedHook;
    std::map<std::string, CachedBluetoothDevice*> mCachedDevices;
};

} // namespace preferencedemo

#endif // PREFERENCEDRMO_CACHED_BLUETOOTH_DEVICE_MANAGER_H
