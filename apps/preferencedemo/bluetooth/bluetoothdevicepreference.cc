#include "bluetoothdevicepreference.h"

#include <core/context.h>
#include <widget/toast.h>

#include <R.h>

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
    // The icon clone is row-owned: Preference::setIcon overwrites the raw
    // pointer without freeing (and ~Preference is default) — recycle the
    // previous one or every refresh leaks it.
    cdroid::Drawable* nextIcon = getBtClassDrawable(getContext(), *mCachedDevice);
    setIcon(nextIcon);
    delete mClassIcon;
    mClassIcon = nextIcon;

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
            const std::string name = mCachedDevice->getName();
            if (context != nullptr) {
                cdroid::Toast::makeText(context,
                        context->getString(
                                (int)preferencedemo::R::string::bluetooth_pairing_error_message,
                                {name}),
                        cdroid::Toast::LENGTH_SHORT)->show();
            }
        }
    }
}

void BluetoothDevicePreference::askDisconnect() {
    cdroid::Context* context = &getContext();
    if (context == nullptr) return;
    const std::string name = mCachedDevice->getName();   // alias-or-address
    CachedBluetoothDevice* device = mCachedDevice;
    // cdroid dialogs are owner-managed (dismiss does not delete).
    if (mDisconnectDialog != nullptr) {
        mDisconnectDialog->dismiss();
        delete mDisconnectDialog;
    }
    mDisconnectDialog = cdroid::AlertDialog::Builder(context)
            .setTitle(context->getString(
                    (int)preferencedemo::R::string::bluetooth_disconnect_title))
            .setMessage(context->getString(
                    (int)preferencedemo::R::string::bluetooth_disconnect_all_profiles,
                    {name}))
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
        delete mDisconnectDialog;   // owner-managed: dismiss does not delete
        mDisconnectDialog = nullptr;
    }
    // Preference does not free the icon it displays.
    setIcon(nullptr);
    delete mClassIcon;
    mClassIcon = nullptr;
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
        // AOSP returns (int)(mCurrentTime - another.mCurrentTime); a 1/-1
        // pair for equal timestamps violates strict weak ordering, which
        // PreferenceGroup's lower_bound/std::sort depend on.
        return static_cast<int>(mCurrentTime - other->mCurrentTime);
    default:
        return Preference::compareTo(another);
    }
}

} // namespace preferencedemo
