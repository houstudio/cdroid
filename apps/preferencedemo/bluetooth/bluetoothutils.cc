#include "bluetoothutils.h"

#include <core/context.h>
#include <widget/internal_R.h>

#include "cachedbluetoothdevice.h"

namespace preferencedemo {

cdroid::Drawable* getBtClassDrawable(cdroid::Context& context,
                                     const CachedBluetoothDevice& cachedDevice) {
    using cdroid::BluetoothClass;
    const BluetoothClass btClass = cachedDevice.getDevice().getBluetoothClass();
    const int major = btClass.getMajorDeviceClass();
    int internalIcon = 0;
    switch (major) {
    case BluetoothClass::Device::Major::COMPUTER:
        internalIcon = (int)cdroid::internal::R::drawable::ic_bt_laptop;
        break;
    case BluetoothClass::Device::Major::PHONE:
        internalIcon = (int)cdroid::internal::R::drawable::ic_phone;
        break;
    case BluetoothClass::Device::Major::PERIPHERAL:
        // HidProfile.getHidClassDrawable, verbatim (ic_lockscreen_ime has no
        // CDROID twin — the generic HID icon stands in for keyboards).
        switch (btClass.getDeviceClass()) {
        case BluetoothClass::Device::PERIPHERAL_POINTING:
            internalIcon = (int)cdroid::internal::R::drawable::ic_bt_pointing_hid;
            break;
        default:
            internalIcon = (int)cdroid::internal::R::drawable::ic_bt_misc_hid;
            break;
        }
        break;
    case BluetoothClass::Device::Major::IMAGING:
        internalIcon = (int)cdroid::internal::R::drawable::ic_settings_print;
        break;
    default:
        break;
    }
    if (internalIcon != 0) return context.getDrawable(internalIcon);

    // The profile loop is empty (cdblue profile stubs) — straight to the
    // class-matching fallbacks.
    if (btClass.doesClassMatch(BluetoothClass::PROFILE_HEADSET)) {
        return context.getDrawable(
                (int)cdroid::internal::R::drawable::ic_bt_headset_hfp);
    }
    if (btClass.doesClassMatch(BluetoothClass::PROFILE_A2DP)) {
        return context.getDrawable(
                (int)cdroid::internal::R::drawable::ic_bt_headphones_a2dp);
    }
    return context.getDrawable(
            (int)cdroid::internal::R::drawable::ic_settings_bluetooth);
}

} // namespace preferencedemo
