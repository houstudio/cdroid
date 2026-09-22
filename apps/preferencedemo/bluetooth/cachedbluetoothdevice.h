/*********************************************************************************
 * Port of AOSP 12 SettingsLib CachedBluetoothDevice.java:
 *   frameworks/base/packages/SettingsLib/src/com/android/settingslib/
 *   bluetooth/CachedBluetoothDevice.java
 *
 * One cached entry per remote address: display name, bond state, connection
 * summary, pairing entry points. The profile set is the HID host (the one
 * profile cdblue serves end to end); A2DP/HFP/HearingAid, battery metadata
 * and TWS stay stubbed, so the connection summary keeps the bond-state
 * branch plus the HID-connected branch of AOSP's getConnectionSummary.
 *********************************************************************************/
#ifndef PREFERENCEDRMO_CACHED_BLUETOOTH_DEVICE_H
#define PREFERENCEDRMO_CACHED_BLUETOOTH_DEVICE_H

#include <functional>
#include <string>
#include <vector>

#include <bluetoothdevice.h>

#include "localbluetoothmanager.h"

namespace preferencedemo {

class CachedBluetoothDeviceManager;
class LocalBluetoothProfileManager;

class CachedBluetoothDevice {
public:
    /** AOSP CachedBluetoothDevice.Callback (single-method register list). */
    using Callback = std::function<void()>;

    /* AOSP ctor shape (context dropped): profileManager + localAdapter. */
    CachedBluetoothDevice(LocalBluetoothProfileManager* profileManager,
                          LocalBluetoothAdapter* localAdapter,
                          cdroid::BluetoothDevice device);

    cdroid::BluetoothDevice getDevice() const { return mDevice; }
    std::string getAddress() const { return mDevice.getAddress(); }

    /* AOSP getName: alias if present, else the address. */
    std::string getName() const;
    void setName(const std::string& name);

    int getBondState() const { return mDevice.getBondState(); }
    /* AOSP hasHumanReadableName: alias non-empty. */
    bool hasHumanReadableName() const;

    /* AOSP isBusy: profile busy states stub away; bonding remains. */
    bool isBusy() const { return getBondState() == cdroid::BluetoothDevice::BOND_BONDING; }
    /* AOSP isConnected: any profile connected — here the HID host. */
    bool isConnected() const;

    bool startPairing();
    void unpair();
    /* AOSP connect(): ensurePaired (pairs first when unpaired) then
     * connectAllEnabledProfiles — the HID host when the device is one. */
    void connect();
    void disconnect();

    /* AOSP getConnectionSummary(boolean): the bond-state half; profile and
     * battery branches collapse (no profiles, battery unknown). Returns ""
     * for the AOSP null (not pairing, nothing connected). */
    std::string getConnectionSummary() const;

    /** AOSP compareTo: bonded first, then by name. */
    int compareTo(const CachedBluetoothDevice& another) const;

    void registerCallback(Callback callback);
    void unregisterCallbacks();
    void dispatchAttributesChanged();

    bool operator==(const CachedBluetoothDevice& other) const {
        return mDevice.getAddress() == other.mDevice.getAddress();
    }

private:
    friend class CachedBluetoothDeviceManager;
    void refresh();   // re-read properties after an event
    /* The HID proxy via the profile manager (nullptr for non-HID sets). */
    cdroid::BluetoothHidHost* hidProfile() const;

    LocalBluetoothProfileManager* mProfileManager;
    LocalBluetoothAdapter* mLocalAdapter;
    cdroid::BluetoothDevice mDevice;
    std::vector<Callback> mCallbacks;   // main thread only
};

} // namespace preferencedemo

#endif // PREFERENCEDRMO_CACHED_BLUETOOTH_DEVICE_H
