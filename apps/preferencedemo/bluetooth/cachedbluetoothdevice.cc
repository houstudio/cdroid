#include "cachedbluetoothdevice.h"

#include "cachedbluetoothdevicemanager.h"

namespace preferencedemo {

CachedBluetoothDevice::CachedBluetoothDevice(
        LocalBluetoothAdapter* localAdapter,
        cdroid::BluetoothDevice device)
    : mLocalAdapter(localAdapter),
      mDevice(device) {
}

std::string CachedBluetoothDevice::getName() const {
    // AOSP: alias if non-empty, else the address.
    const std::string aliasName = mDevice.getAlias();
    return aliasName.empty() ? getAddress() : aliasName;
}

void CachedBluetoothDevice::setName(const std::string& name) {
    if (name.empty() || name == getName()) return;
    mDevice.setAlias(name);
    dispatchAttributesChanged();
}

bool CachedBluetoothDevice::hasHumanReadableName() const {
    return !mDevice.getAlias().empty();
}

bool CachedBluetoothDevice::startPairing() {
    // AOSP: pairing is unreliable while scanning, so cancel discovery first.
    if (mLocalAdapter->isDiscovering()) {
        mLocalAdapter->cancelDiscovery();
    }
    return mDevice.createBond();
}

void CachedBluetoothDevice::unpair() {
    const int state = getBondState();
    if (state == cdroid::BluetoothDevice::BOND_BONDING) {
        mDevice.cancelPairingUserInput();
    }
    mDevice.removeBond();
}

std::string CachedBluetoothDevice::getConnectionSummary() const {
    // AOSP getConnectionSummary: the profile loop is empty (cdblue profile
    // stubs) and battery is unknown, so only the bond branch remains —
    // BOND_BONDING shows "正在配对…", otherwise the AOSP null (empty here).
    if (getBondState() == cdroid::BluetoothDevice::BOND_BONDING) {
        return "正在配对…";   // R.string.bluetooth_pairing
    }
    return std::string();
}

int CachedBluetoothDevice::compareTo(const CachedBluetoothDevice& another) const {
    // AOSP: a connected/bonded device sorts before a connecting one, then by
    // name. Profiles stub away; the bonded-first tier stays.
    const bool thisBonded = getBondState() == cdroid::BluetoothDevice::BOND_BONDED;
    const bool anotherBonded = another.getBondState() == cdroid::BluetoothDevice::BOND_BONDED;
    if (thisBonded != anotherBonded) return thisBonded ? -1 : 1;
    return getName().compare(another.getName());
}

void CachedBluetoothDevice::registerCallback(Callback callback) {
    mCallbacks.push_back(std::move(callback));
}

void CachedBluetoothDevice::unregisterCallbacks() {
    mCallbacks.clear();
}

void CachedBluetoothDevice::dispatchAttributesChanged() {
    for (auto& cb : mCallbacks) cb();
}

void CachedBluetoothDevice::refresh() {
    // BluetoothDevice getters resolve live through the adapter; the cache
    // only needs to re-notify its rows.
    dispatchAttributesChanged();
}

} // namespace preferencedemo
