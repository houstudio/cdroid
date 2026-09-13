#include "localbluetoothmanager.h"

#include <core/handler.h>
#include <core/looper.h>

#include "cachedbluetoothdevicemanager.h"

namespace preferencedemo {

// --- LocalBluetoothManager ----------------------------------------------------

LocalBluetoothManager* LocalBluetoothManager::getInstance() {
    static LocalBluetoothManager sInstance;
    return &sInstance;
}

LocalBluetoothManager::LocalBluetoothManager()
    : mLocalAdapter(cdroid::BluetoothAdapter::getDefaultAdapter()),
      mDeviceManager(std::make_unique<CachedBluetoothDeviceManager>(&mLocalAdapter)),
      mEventManager(mDeviceManager.get()) {
    mLocalAdapter.raw().addAdapterStateListener(&mEventManager);
    mLocalAdapter.raw().addDiscoveryListener(&mEventManager);
    mLocalAdapter.raw().addBondStateListener(&mEventManager);
}

// --- BluetoothEventManager ----------------------------------------------------

BluetoothEventManager::BluetoothEventManager(CachedBluetoothDeviceManager* deviceManager)
    : mDeviceManager(deviceManager) {
    // No-GC seam: entry deletion fans out onDeviceDeleted first (see
    // CachedBluetoothDeviceManager::onDeviceDeleted).
    deviceManager->setOnDeviceDeletedHook([this](CachedBluetoothDevice* cachedDevice) {
        for (auto* cb : mCallbacks) cb->onDeviceDeleted(cachedDevice);
    });
}

void BluetoothEventManager::registerCallback(BluetoothCallback* callback) {
    mCallbacks.push_back(callback);
}

void BluetoothEventManager::unregisterCallback(BluetoothCallback* callback) {
    for (size_t i = 0; i < mCallbacks.size(); i++) {
        if (mCallbacks[i] == callback) { mCallbacks.erase(mCallbacks.begin() + i); return; }
    }
}

void BluetoothEventManager::post(std::function<void()> fn) {
    // The manager is a process singleton; posted lambdas may only touch the
    // manager graph, which outlives every screen.
    static cdroid::Handler sHandler(cdroid::Looper::getMainLooper());
    sHandler.post(std::move(fn));
}

void BluetoothEventManager::onAdapterStateChanged(int newState, int) {
    post([this, newState]{
        mDeviceManager->onBluetoothStateChanged(newState);
        for (auto* cb : mCallbacks) cb->onBluetoothStateChanged(newState);
    });
}

void BluetoothEventManager::onDiscoveryStarted() {
    post([this]{
        for (auto* cb : mCallbacks) cb->onScanningStateChanged(true);
    });
}

void BluetoothEventManager::onDeviceFound(const cdroid::BluetoothDevice& device) {
    // Copy the address; mint/cache resolution happens on the main side.
    const std::string address = device.getAddress();
    post([this, address]{
        CachedBluetoothDevice* cached =
                mDeviceManager->onDeviceAdded(mDeviceManager->getLocalAdapter()
                        ->raw().getRemoteDevice(address));
        if (cached != nullptr) {
            for (auto* cb : mCallbacks) cb->onDeviceAdded(cached);
        }
    });
}

void BluetoothEventManager::onDiscoveryFinished() {
    post([this]{
        for (auto* cb : mCallbacks) cb->onScanningStateChanged(false);
    });
}

void BluetoothEventManager::onBondStateChanged(const cdroid::BluetoothDevice& device,
                                               int bondState, int) {
    const std::string address = device.getAddress();
    post([this, address, bondState]{
        cdroid::BluetoothDevice remote =
                mDeviceManager->getLocalAdapter()->raw().getRemoteDevice(address);
        CachedBluetoothDevice* cached = mDeviceManager->onDeviceAdded(remote);
        if (cached != nullptr) {
            for (auto* cb : mCallbacks) cb->onDeviceBondStateChanged(cached, bondState);
        }
        if (bondState == cdroid::BluetoothDevice::BOND_NONE) {
            if (cached != nullptr) {
                mDeviceManager->onDeviceDeleted(cached);
            }
        } else {
            if (cached != nullptr) cached->dispatchAttributesChanged();
        }
    });
}

} // namespace preferencedemo
