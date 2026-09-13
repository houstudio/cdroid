#include "cachedbluetoothdevicemanager.h"

#include <bluetoothadapter.h>

namespace preferencedemo {

CachedBluetoothDeviceManager::CachedBluetoothDeviceManager(
        LocalBluetoothAdapter* localAdapter)
    : mLocalAdapter(localAdapter) {
}

std::vector<CachedBluetoothDevice*> CachedBluetoothDeviceManager::getCachedDevicesCopy() const {
    std::vector<CachedBluetoothDevice*> copy;
    copy.reserve(mCachedDevices.size());
    for (auto& kv : mCachedDevices) copy.push_back(kv.second);
    return copy;
}

CachedBluetoothDevice* CachedBluetoothDeviceManager::findDevice(const std::string& address) {
    auto it = mCachedDevices.find(address);
    return it != mCachedDevices.end() ? it->second : nullptr;
}

CachedBluetoothDevice* CachedBluetoothDeviceManager::onDeviceAdded(
        const cdroid::BluetoothDevice& device) {
    // AOSP drops device events while the radio is off.
    if (mLocalAdapter->getState() != cdroid::BluetoothAdapter::STATE_ON) return nullptr;

    const std::string address = device.getAddress();
    CachedBluetoothDevice* cachedDevice = findDevice(address);
    if (cachedDevice == nullptr) {
        cachedDevice = new CachedBluetoothDevice(mLocalAdapter, this, device);
        mCachedDevices[address] = cachedDevice;
    }
    return cachedDevice;
}

void CachedBluetoothDeviceManager::onDeviceDeleted(CachedBluetoothDevice* cachedDevice) {
    const std::string address = cachedDevice->getAddress();
    if (findDevice(address) == nullptr) return;
    if (mOnDeviceDeletedHook) mOnDeviceDeletedHook(cachedDevice);
    cachedDevice->unregisterCallbacks();
    mCachedDevices.erase(address);
    delete cachedDevice;
}

void CachedBluetoothDeviceManager::onBluetoothStateChanged(int state) {
    if (state == cdroid::BluetoothAdapter::STATE_TURNING_OFF
            || state == cdroid::BluetoothAdapter::STATE_OFF) {
        // AOSP clears the cache when the radio goes down.
        auto cached = getCachedDevicesCopy();
        for (CachedBluetoothDevice* d : cached) onDeviceDeleted(d);
        mCachedDevices.clear();
    }
}

} // namespace preferencedemo
