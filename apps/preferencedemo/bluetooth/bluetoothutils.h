/*********************************************************************************
 * Port of the icon half of AOSP 12 SettingsLib BluetoothUtils.java:
 *   frameworks/base/packages/SettingsLib/src/com/android/settingslib/
 *   bluetooth/BluetoothUtils.java — getBtClassDrawableWithDescription()
 *   (class-of-device -> framework ic_bt_* drawable).
 *
 * The profile-driven branch collapses (cdblue profiles are stubs); the
 * PROFILE_HEADSET / PROFILE_A2DP class matches remain, as they are pure
 * BluetoothClass checks.
 *********************************************************************************/
#ifndef PREFERENCEDRMO_BLUETOOTH_UTILS_H
#define PREFERENCEDRMO_BLUETOOTH_UTILS_H

#include <drawable/drawable.h>
#include <bluetoothclass.h>

namespace preferencedemo {

class CachedBluetoothDevice;

/** AOSP BluetoothUtils.getBtClassDrawableWithDescription: the device-class
 *  icon for a row. Returns the ic_settings_bluetooth fallback when the class
 *  is unknown. Caller owns the drawable (context.getDrawable clone). */
cdroid::Drawable* getBtClassDrawable(cdroid::Context& context,
                                     const CachedBluetoothDevice& cachedDevice);

} // namespace preferencedemo

#endif // PREFERENCEDRMO_BLUETOOTH_UTILS_H
