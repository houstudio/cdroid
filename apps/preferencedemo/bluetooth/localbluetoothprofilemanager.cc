#include "localbluetoothprofilemanager.h"

namespace preferencedemo {

LocalBluetoothProfileManager::LocalBluetoothProfileManager(
        LocalBluetoothAdapter* localAdapter)
    : mLocalAdapter(localAdapter) {
}

cdroid::BluetoothHidHost* LocalBluetoothProfileManager::getHidHostProfile() const {
    if (!mHidHost) {
        /* getProfileProxy hands the proxy over through the ServiceListener
         * synchronously (in-process profiles) — capture it and go. */
        cdroid::BluetoothProfile::ServiceListener listener;
        listener.onServiceConnected =
                [this](int, cdroid::BluetoothProfile* proxy) {
                    mHidHost.reset(
                            static_cast<cdroid::BluetoothHidHost*>(proxy));
                };
        mLocalAdapter->raw().getProfileProxy(
                listener, cdroid::BluetoothProfile::HID_HOST);
    }
    return mHidHost.get();
}

bool LocalBluetoothProfileManager::isHidDevice(
        const std::vector<cdroid::BluetoothUuid>& uuids) {
    const cdroid::BluetoothUuid hid = cdroid::BluetoothUuid::HID();
    for (const cdroid::BluetoothUuid& u : uuids) {
        if (u == hid) return true;
    }
    return false;
}

} // namespace preferencedemo
