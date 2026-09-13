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
    // AOSP seeds the bonded cache at manager creation.
    mEventManager.readPairedDevices();
}

LocalBluetoothManager::~LocalBluetoothManager() {
    // The manager is a function-local static destroyed before the adapter
    // static (reverse construction order) while the BlueZ monitor thread may
    // still be dispatching — detach the listeners first or their next fan-out
    // calls into the dying BluetoothEventManager.
    mLocalAdapter.raw().removeAdapterStateListener(&mEventManager);
    mLocalAdapter.raw().removeDiscoveryListener(&mEventManager);
    mLocalAdapter.raw().removeBondStateListener(&mEventManager);
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

void BluetoothEventManager::readPairedDevices() {
    for (CachedBluetoothDevice* cachedDevice : mDeviceManager->readPairedDevices()) {
        for (auto* cb : mCallbacks) cb->onDeviceAdded(cachedDevice);
    }
}

void BluetoothEventManager::onAdapterStateChanged(int newState, int) {
    post([this, newState]{
        mDeviceManager->onBluetoothStateChanged(newState);
        for (auto* cb : mCallbacks) cb->onBluetoothStateChanged(newState);
        if (newState == cdroid::BluetoothAdapter::STATE_ON) {
            // AOSP: bonded devices re-enter the cache (and the screens) on
            // every radio-on transition.
            readPairedDevices();
        }
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
    const bool wasCached = mDeviceManager->findDevice(address) != nullptr;
    post([this, address, wasCached]{
        CachedBluetoothDevice* cached =
                mDeviceManager->onDeviceAdded(mDeviceManager->getLocalAdapter()
                        ->raw().getRemoteDevice(address));
        if (cached == nullptr) return;
        if (wasCached) {
            // The adapter re-fires Found on every Name/RSSI update: refresh
            // the row (name may have just resolved — it starts hidden).
            cached->dispatchAttributesChanged();
        } else {
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
        if (cached == nullptr) return;
        for (auto* cb : mCallbacks) cb->onDeviceBondStateChanged(cached, bondState);
        // AOSP keeps the (now unbonded) entry cached — deletion is reserved
        // for the explicit forget/unpair cascade — so a failed pairing can
        // be retried from the picker without a fresh discovery sweep.
        cached->dispatchAttributesChanged();
    });
}

} // namespace preferencedemo
