#include "devicelistpreferencefragment.h"

#include <bluetoothadapter.h>
#include <core/context.h>

#include "cachedbluetoothdevicemanager.h"

namespace preferencedemo {

DeviceListPreferenceFragment::DeviceListPreferenceFragment(Host* host)
    : mHost(host) {
    mLocalManager = LocalBluetoothManager::getInstance();
    mLocalAdapter = mLocalManager->getBluetoothAdapter();
}

void DeviceListPreferenceFragment::onStart() {
    mLocalManager->getEventManager()->registerCallback(this);
}

void DeviceListPreferenceFragment::onStop() {
    // AOSP DevicePickerFragment/PairingDetail onStop: stop scanning (the
    // base class alone does not, but only scanning screens ever enable it).
    disableScanning();
    // AOSP onStop: remove every device row, then unregister. Rows go through
    // removePreference (not a bare delete): the group owns its children
    // (~PreferenceGroup would double-free otherwise) and removePreferenceInt
    // is what invokes onPrepareForRemoval, which unregisters the row's
    // cached-device callback (a bare delete leaves it dangling in the
    // process-singleton cache).
    for (const auto& kv : mDevicePreferenceMap) {
        if (mDeviceListGroup != nullptr) {
            mDeviceListGroup->removePreference(kv.second);
        }
        delete kv.second;
    }
    mDevicePreferenceMap.clear();
    mLocalManager->getEventManager()->unregisterCallback(this);
}

bool DeviceListPreferenceFragment::onPreferenceClick(cdroid::Preference* preference) {
    auto* btPreference = dynamic_cast<BluetoothDevicePreference*>(preference);
    if (btPreference == nullptr) return false;
    onDevicePreferenceClick(btPreference);
    return true;
}

void DeviceListPreferenceFragment::onDevicePreferenceClick(
        BluetoothDevicePreference* btPreference) {
    btPreference->onClicked();
}

void DeviceListPreferenceFragment::addDeviceCategory(
        cdroid::PreferenceGroup* preferenceGroup, const std::string& title,
        Filter filter, bool addCachedDevices) {
    // AOSP addDeviceCategory: title, group swap, optional cached fill (with
    // the unbonded filter so stale bonded devices stay out of the picker).
    preferenceGroup->setTitle(title);
    mDeviceListGroup = preferenceGroup;
    if (addCachedDevices) {
        // AOSP fills under the UNBONDED filter ("Don't show bonded devices
        // when screen turned back on") because its bonded rows arrive via
        // readPairedDevices' onDeviceAdded dispatch instead. This port
        // fills under the FINAL filter so the paired category (BONDED)
        // populates from the seeded cache directly — documented divergence;
        // the picker screens pass ALL and behave identically either way.
        mFilter = filter;
        for (CachedBluetoothDevice* cachedDevice :
                mLocalManager->getCachedDeviceManager()->getCachedDevicesCopy()) {
            onDeviceAdded(cachedDevice);
        }
    }
    mFilter = filter;
    preferenceGroup->setEnabled(true);
}

void DeviceListPreferenceFragment::onDeviceAdded(CachedBluetoothDevice* cachedDevice) {
    if (mDevicePreferenceMap.count(cachedDevice->getAddress())) return;

    // Prevent updates while the list shows one of the state messages.
    if (mLocalAdapter->getState() != cdroid::BluetoothAdapter::STATE_ON) return;

    // AOSP BluetoothDeviceFilter.matches: BONDED keeps bonded only, UNBONDED
    // drops them, ALL keeps everything; nameless devices hide themselves in
    // the row (BluetoothDevicePreference visibility).
    const bool bonded = cachedDevice->getBondState() == cdroid::BluetoothDevice::BOND_BONDED;
    if (mFilter == FILTER_BONDED && !bonded) return;
    if (mFilter == FILTER_UNBONDED && bonded) return;
    createDevicePreference(cachedDevice);
}

void DeviceListPreferenceFragment::createDevicePreference(
        CachedBluetoothDevice* cachedDevice) {
    if (mDeviceListGroup == nullptr) return;

    const std::string key = cachedDevice->getAddress();
    auto* preference = new BluetoothDevicePreference(
            mHost->prefContext(), cachedDevice, /*showDeviceWithoutNames=*/false,
            BluetoothDevicePreference::TYPE_FIFO);
    preference->setKey(key);
    mDeviceListGroup->addPreference(preference);
    mDevicePreferenceMap[key] = preference;
}

void DeviceListPreferenceFragment::onDeviceDeleted(CachedBluetoothDevice* cachedDevice) {
    // Note: called from the deletion hook BEFORE the cache entry dies.
    const auto it = mDevicePreferenceMap.find(cachedDevice->getAddress());
    if (it != mDevicePreferenceMap.end()) {
        BluetoothDevicePreference* preference = it->second;
        if (mDeviceListGroup != nullptr) {
            mDeviceListGroup->removePreference(preference);
        }
        mDevicePreferenceMap.erase(it);
        delete preference;
    }
}

void DeviceListPreferenceFragment::onDeviceBondStateChanged(
        CachedBluetoothDevice* cachedDevice, int bondState) {
    if (bondState == cdroid::BluetoothDevice::BOND_BONDED) {
        onDeviceAdded(cachedDevice);
    }
}

void DeviceListPreferenceFragment::onBluetoothStateChanged(int) {
    // The screens refresh their rows on state flips (see the hosts).
}

void DeviceListPreferenceFragment::onScanningStateChanged(bool started) {
    if (!started && mScanEnabled) {
        startScanning();   // AOSP: keep scanning while this screen wants it.
    }
}

void DeviceListPreferenceFragment::enableScanning() {
    // BluetoothAdapter already handles repeated scan requests.
    if (!mScanEnabled) {
        startScanning();
        mScanEnabled = true;
    }
}

void DeviceListPreferenceFragment::disableScanning() {
    if (mScanEnabled) {
        stopScanning();
        mScanEnabled = false;
    }
}

void DeviceListPreferenceFragment::startScanning() {
    if (!mLocalAdapter->isDiscovering()) {
        mLocalAdapter->startDiscovery();
    }
}

void DeviceListPreferenceFragment::stopScanning() {
    if (mLocalAdapter->isDiscovering()) {
        mLocalAdapter->cancelDiscovery();
    }
}

} // namespace preferencedemo
