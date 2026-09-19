/*********************************************************************************
 * Port of AOSP 12 Settings DeviceListPreferenceFragment.java:
 *   packages/apps/Settings/src/com/android/settings/bluetooth/
 *   DeviceListPreferenceFragment.java
 *
 * AOSP derives from RestrictedDashboardFragment; preferencedemo drives all
 * screens from its single SettingsFragment, so this is the same logic bound
 * to a Host surface instead of a Fragment base class (the two Bluetooth
 * screens instantiate it; onStart/onStop map to host attach/detach).
 *
 * BluetoothDeviceFilter is folded in (SettingsLib) reduced to the two
 * filters the screens use: ALL and UNBONDED (paired rows read from the
 * cache, discovery rows must be unbonded).
 *********************************************************************************/
#ifndef PREFERENCEDRMO_DEVICE_LIST_PREFERENCE_FRAGMENT_H
#define PREFERENCEDRMO_DEVICE_LIST_PREFERENCE_FRAGMENT_H

#include <map>
#include <string>

#include <preference/preference.h>
#include <preference/preferencecategory.h>

#include "bluetoothdevicepreference.h"
#include "bluetoothprogresscategory.h"
#include "localbluetoothmanager.h"

namespace preferencedemo {

class DeviceListPreferenceFragment : public BluetoothCallback {
public:
    /** The SettingsFragment surface this logic needs (AOSP: the Fragment). */
    class Host {
    public:
        virtual ~Host() = default;
        virtual cdroid::Preference* findPreference(const std::string& key) = 0;
        virtual cdroid::Context* prefContext() = 0;
    };

    explicit DeviceListPreferenceFragment(Host* host);

    /** AOSP onStart: register the callback. */
    void onStart();
    /** AOSP onStop: unregister + drop every device row. */
    void onStop();

    /** AOSP onPreferenceTreeClick's bluetooth rows: returns true when the
     *  preference was a device row handled here. */
    bool onPreferenceClick(cdroid::Preference* preference);

    // BluetoothCallback (main thread, dispatched by BluetoothEventManager).
    void onBluetoothStateChanged(int bluetoothState) override;
    void onScanningStateChanged(bool started) override;
    void onDeviceAdded(CachedBluetoothDevice* cachedDevice) override;
    void onDeviceDeleted(CachedBluetoothDevice* cachedDevice) override;
    void onDeviceBondStateChanged(CachedBluetoothDevice* cachedDevice,
                                  int bondState) override;

    /** SettingsLib BluetoothDeviceFilter, reduced to the filters used:
     *  ALL (pairing screen), BONDED (paired category), UNBONDED. */
    enum Filter { FILTER_ALL, FILTER_BONDED, FILTER_UNBONDED };

public:
    /** AOSP addDeviceCategory: set the group a screen fills with devices.
     *  The cached fill runs under the UNBONDED filter (AOSP: "Don't show
     *  bonded devices when screen turned back on"), then the final filter
     *  takes over. */
    void addDeviceCategory(cdroid::PreferenceGroup* preferenceGroup,
                           const std::string& title, Filter filter,
                           bool addCachedDevices);

public:
    /** AOSP enableScanning/disableScanning + the scanning state drive. */
    void enableScanning();
    void disableScanning();

protected:

    /** AOSP KEY_BT_SCAN handling: rows of BluetoothDevicePreference. */
    virtual void onDevicePreferenceClick(BluetoothDevicePreference* btPreference);

    LocalBluetoothAdapter* mLocalAdapter = nullptr;
    LocalBluetoothManager* mLocalManager = nullptr;
    cdroid::PreferenceGroup* mDeviceListGroup = nullptr;
    bool mScanEnabled = false;

private:
    void createDevicePreference(CachedBluetoothDevice* cachedDevice);
    void startScanning();
    void stopScanning();

    Host* mHost;
    /* AOSP mDevicePreferenceMap, keyed by address (the CachedBluetoothDevice
     * pointer identity is unstable across cache rebuilds). */
    std::map<std::string, BluetoothDevicePreference*> mDevicePreferenceMap;
    Filter mFilter = FILTER_ALL;   // AOSP mFilter
};

} // namespace preferencedemo

#endif // PREFERENCEDRMO_DEVICE_LIST_PREFERENCE_FRAGMENT_H
