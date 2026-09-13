#include "bluetoothdevicepreference.h"

#include <core/context.h>
#include <widget/toast.h>

#include "bluetoothutils.h"

#include <core/systemclock.h>

namespace preferencedemo {

BluetoothDevicePreference::BluetoothDevicePreference(
        cdroid::Context* context, CachedBluetoothDevice* cachedDevice,
        bool showDeviceWithoutNames, int type)
    : Preference(*context),
      mCachedDevice(cachedDevice),
      mShowDevicesWithoutNames(showDeviceWithoutNames),
      mCurrentTime(cdroid::SystemClock::currentTimeMillis()),
      mType(type) {
    // AOSP: the row refreshes on every cached-device attribute change.
    mCachedDevice->registerCallback([this] { onPreferenceAttributesChanged(); });
    onPreferenceAttributesChanged();
}

void BluetoothDevicePreference::onPreferenceAttributesChanged() {
    setIcon(getBtClassDrawable(getContext(), *mCachedDevice));

    setTitle(mCachedDevice->getName());
    setSummary(mCachedDevice->getConnectionSummary());

    // Used to gray out the item.
    setEnabled(!mCachedDevice->isBusy());

    // Only visible when it has a real name (or the developer flag allows).
    setVisible(mShowDevicesWithoutNames || mCachedDevice->hasHumanReadableName());
}

void BluetoothDevicePreference::onClicked() {
    cdroid::Context* context = &getContext();
    const int bondState = mCachedDevice->getBondState();

    if (mCachedDevice->isConnected()) {
        askDisconnect();
    } else if (bondState == cdroid::BluetoothDevice::BOND_BONDED) {
        mCachedDevice->connect();
    } else if (bondState == cdroid::BluetoothDevice::BOND_NONE) {
        // pair()
        if (!mCachedDevice->startPairing()) {
            // Utils.showError: pairing failed toast.
            std::string name = mCachedDevice->getName();
            if (name.empty()) name = "未命名的蓝牙设备";
            if (context != nullptr) {
                cdroid::Toast::makeText(context,
                        "无法与" + name + "配对。",
                        cdroid::Toast::LENGTH_SHORT)->show();
            }
        }
    }
}

void BluetoothDevicePreference::askDisconnect() {
    cdroid::Context* context = &getContext();
    if (context == nullptr) return;
    std::string name = mCachedDevice->getName();
    if (name.empty()) name = "未命名的蓝牙设备";
    CachedBluetoothDevice* device = mCachedDevice;
    if (mDisconnectDialog != nullptr) mDisconnectDialog->dismiss();
    mDisconnectDialog = cdroid::AlertDialog::Builder(context)
            .setTitle("要断开与该设备的连接吗？")
            .setMessage("您的设备将断开与" + name + "的连接。")
            .setPositiveButton("断开", [device](cdroid::DialogInterface&, int) {
                device->disconnect();
            })
            .setNegativeButton("取消", [](cdroid::DialogInterface&, int) {})
            .create();
    mDisconnectDialog->show();
}

void BluetoothDevicePreference::onPrepareForRemoval() {
    Preference::onPrepareForRemoval();
    if (!mIsCallbackRemoved) {
        mCachedDevice->unregisterCallbacks();
        mIsCallbackRemoved = true;
    }
    if (mDisconnectDialog != nullptr) {
        mDisconnectDialog->dismiss();
        mDisconnectDialog = nullptr;
    }
}

int BluetoothDevicePreference::compareTo(const cdroid::Preference& another) const {
    const auto* other = dynamic_cast<const BluetoothDevicePreference*>(&another);
    if (other == nullptr) {
        return Preference::compareTo(another);   // Rely on default sort.
    }
    switch (mType) {
    case TYPE_DEFAULT:
        return mCachedDevice->compareTo(*other->mCachedDevice);
    case TYPE_FIFO:
        return mCurrentTime > other->mCurrentTime ? 1 : -1;
    default:
        return Preference::compareTo(another);
    }
}

} // namespace preferencedemo
