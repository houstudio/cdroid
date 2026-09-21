/*********************************************************************************
 * Port of AOSP 12 SettingsLib LocalBluetoothProfileManager.java (reduced to
 * the HID host profile — the one profile cdblue serves end to end; A2DP/HFP
 * stay framework stubs with nothing to manage):
 *   frameworks/base/packages/SettingsLib/src/com/android/settingslib/
 *     bluetooth/LocalBluetoothProfileManager.java
 *
 * Owns the process-shared profile proxy (AOSP: one profile object shared by
 * every CachedBluetoothDevice) and assigns profiles to devices from their
 * service UUIDs — the updateProfiles half of the original.
 *********************************************************************************/
#ifndef PREFERENCEDRMO_LOCAL_BLUETOOTH_PROFILE_MANAGER_H
#define PREFERENCEDRMO_LOCAL_BLUETOOTH_PROFILE_MANAGER_H

#include <memory>
#include <vector>

#include <bluetoothdevice.h>
#include <bluetoothprofile.h>

#include "localbluetoothmanager.h"

namespace preferencedemo {

class LocalBluetoothProfileManager {
public:
    explicit LocalBluetoothProfileManager(LocalBluetoothAdapter* localAdapter);

    /** The HID host proxy, minted on first use (AOSP: profile objects are
     *  process singletons; nullptr while the adapter has no proxy yet). */
    cdroid::BluetoothHidHost* getHidHostProfile() const;

    /** updateProfiles(uuids): does the UUID set name the HID profile?
     *  (AOSP HidProfile.matches — UUID 0x1124.) */
    static bool isHidDevice(const std::vector<cdroid::BluetoothUuid>& uuids);

private:
    LocalBluetoothAdapter* mLocalAdapter;
    /* Lazily minted via getProfileProxy; mutable so the lazy mint works
     * from const getters (isConnected). */
    mutable std::unique_ptr<cdroid::BluetoothHidHost> mHidHost;
};

} // namespace preferencedemo

#endif // PREFERENCEDRMO_LOCAL_BLUETOOTH_PROFILE_MANAGER_H
