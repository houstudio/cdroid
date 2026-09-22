#include "cachedbluetoothdevice.h"

#include <bluetoothprofile.h>

#include "cachedbluetoothdevicemanager.h"
#include "localbluetoothprofilemanager.h"

namespace preferencedemo {

CachedBluetoothDevice::CachedBluetoothDevice(
        LocalBluetoothProfileManager* profileManager,
        LocalBluetoothAdapter* localAdapter,
        cdroid::BluetoothDevice device)
    : mProfileManager(profileManager),
      mLocalAdapter(localAdapter),
      mDevice(device) {
}

cdroid::BluetoothHidHost* CachedBluetoothDevice::hidProfile() const {
    // AOSP: the profile set comes from updateProfiles(uuids); the HID
    // membership check is that half, evaluated live (UUIDs resolve late).
    if (mProfileManager == nullptr
            || !LocalBluetoothProfileManager::isHidDevice(mDevice.getUuids())) {
        return nullptr;
    }
    return mProfileManager->getHidHostProfile();
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

bool CachedBluetoothDevice::isConnected() const {
    // AOSP isConnected: any profile connected — the HID host here.
    cdroid::BluetoothHidHost* hid = hidProfile();
    return hid != nullptr
            && hid->getConnectionState(mDevice)
                       == cdroid::BluetoothProfile::STATE_CONNECTED;
}

void CachedBluetoothDevice::connect() {
    // AOSP connect(): ensurePaired() — pairing starts when unpaired — then
    // connectAllEnabledProfiles(); the enabled set is the HID host.
    if (getBondState() == cdroid::BluetoothDevice::BOND_NONE) {
        startPairing();
        return;
    }
    if (cdroid::BluetoothHidHost* hid = hidProfile()) hid->connect(mDevice);
}

void CachedBluetoothDevice::disconnect() {
    // AOSP disconnect(): every connected profile — the HID host here.
    if (cdroid::BluetoothHidHost* hid = hidProfile()) hid->disconnect(mDevice);
}

std::string CachedBluetoothDevice::getConnectionSummary() const {
    // AOSP getConnectionSummary: pairing first, then the profile loop —
    // a connected profile (HID here) reports bluetooth_connected; the
    // battery branches stay stubbed away.
    if (getBondState() == cdroid::BluetoothDevice::BOND_BONDING) {
        return "正在配对…";   // R.string.bluetooth_pairing
    }
    if (isConnected()) {
        return "已连接";      // R.string.bluetooth_connected
    }
    return std::string();
}

int CachedBluetoothDevice::compareTo(const CachedBluetoothDevice& another) const {
    // AOSP: a connected device sorts first, then bonded before the rest,
    // then by name.
    const bool thisConnected = isConnected();
    const bool anotherConnected = another.isConnected();
    if (thisConnected != anotherConnected) return thisConnected ? -1 : 1;
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
