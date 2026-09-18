#include "cachedbluetoothdevicemanager.h"

#include <algorithm>

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
        cachedDevice = new CachedBluetoothDevice(mLocalAdapter, device);
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

std::vector<CachedBluetoothDevice*>
CachedBluetoothDeviceManager::readPairedDevices() {
    std::vector<CachedBluetoothDevice*> added;
    if (mLocalAdapter->getState() != cdroid::BluetoothAdapter::STATE_ON) {
        return added;
    }
    for (cdroid::BluetoothDevice device : mLocalAdapter->raw().getBondedDevices()) {
        CachedBluetoothDevice* cached = onDeviceAdded(device);
        if (cached != nullptr && cached->getBondState()
                == cdroid::BluetoothDevice::BOND_BONDED) {
            // Only genuinely-new entries dispatch (AOSP dispatches a
            // DEVICE_FOUND-style callback per bonded device it cached).
            if (std::find(added.begin(), added.end(), cached) == added.end()
                    && !mKnownFromRead.count(cached->getAddress())) {
                mKnownFromRead.insert(cached->getAddress());
                added.push_back(cached);
            }
        }
    }
    return added;
}

void CachedBluetoothDeviceManager::onBluetoothStateChanged(int state) {
    if (state == cdroid::BluetoothAdapter::STATE_TURNING_OFF) {
        // AOSP drops only the NON-bonded entries when the radio turns off;
        // bonded devices stay cached (and readPairedDevices re-seeds on
        // STATE_ON after a full clear anyway).
        auto cached = getCachedDevicesCopy();
        for (CachedBluetoothDevice* d : cached) {
            if (d->getBondState() != cdroid::BluetoothDevice::BOND_BONDED) {
                onDeviceDeleted(d);
            }
        }
    } else if (state == cdroid::BluetoothAdapter::STATE_OFF) {
        auto cached = getCachedDevicesCopy();
        for (CachedBluetoothDevice* d : cached) onDeviceDeleted(d);
        mKnownFromRead.clear();
    }
}

} // namespace preferencedemo
