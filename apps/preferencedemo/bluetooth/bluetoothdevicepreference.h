/*********************************************************************************
 * Port of AOSP 12 Settings BluetoothDevicePreference.java:
 *   packages/apps/Settings/src/com/android/settings/bluetooth/
 *   BluetoothDevicePreference.java
 *
 * A device row: class-of-device icon, name title, connection summary,
 * busy-dimmed, hidden when nameless (unless the developer flag allows).
 * Click semantics (AOSP onClicked): connected -> ask disconnect dialog;
 * bonded -> connect; unbonded -> pair (with the pairing-error toast).
 *
 * GearPreference's second target (device details gear) is dropped: the
 * details screen depends on profile controllers that are stubbed in cdblue.
 *********************************************************************************/
#ifndef PREFERENCEDRMO_BLUETOOTH_DEVICE_PREFERENCE_H
#define PREFERENCEDRMO_BLUETOOTH_DEVICE_PREFERENCE_H

#include <preference/preference.h>
#include <app/alertdialog.h>

#include "cachedbluetoothdevice.h"

namespace preferencedemo {

class BluetoothDevicePreference : public cdroid::Preference {
public:
    /** AOSP SortType (TYPE_FIFO is what the list fragments use). */
    enum SortType {
        TYPE_DEFAULT = 1,
        TYPE_FIFO = 2,
        TYPE_NO_SORT = 3,
    };

    BluetoothDevicePreference(cdroid::Context* context,
                              CachedBluetoothDevice* cachedDevice,
                              bool showDeviceWithoutNames, int type);

    CachedBluetoothDevice* getCachedDevice() const { return mCachedDevice; }

    /* AOSP onPreferenceAttributesChanged: icon/title/summary/enabled/visible. */
    void onPreferenceAttributesChanged();

    /** AOSP onClicked, invoked by the list fragment on a row tap. */
    void onClicked();

    int compareTo(const cdroid::Preference& another) const override;

protected:
    void onPrepareForRemoval() override;

private:
    void askDisconnect();

    CachedBluetoothDevice* mCachedDevice;
    const bool mShowDevicesWithoutNames;
    const int64_t mCurrentTime;
    const int mType;
    /* The row's class-icon drawable: Preference::setIcon overwrites the raw
     * pointer without freeing, so the row owns and recycles this clone. */
    cdroid::Drawable* mClassIcon = nullptr;
    cdroid::AlertDialog* mDisconnectDialog = nullptr;
    bool mIsCallbackRemoved = false;
};

} // namespace preferencedemo

#endif // PREFERENCEDRMO_BLUETOOTH_DEVICE_PREFERENCE_H
