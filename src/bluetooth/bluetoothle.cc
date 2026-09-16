/**
 * BluetoothLeScanner — android.bluetooth.le port over the BlueZ
 * discovery (Transport=le filter + client-side ScanFilter matching).
 */
#include <chrono>
#include <cstdint>

#include <bluetoothadapter.h>
#include <bluetoothle.h>
#include <bluezclient.h>

namespace cdroid {

BluetoothLeScanner::BluetoothLeScanner(BluetoothAdapter& adapter)
    : mAdapter(adapter) {
    /* Bridge into the adapter's discovery stream (was a heap
     * ScannerBridge subclass + void* pair). */
    mDiscoveryBridge.onDeviceFound = [this](const BluetoothDevice& device) {
        onDeviceFound(device);
    };
    mAdapter.addDiscoveryListener(mDiscoveryBridge);
}

bool BluetoothLeScanner::startScan(const std::vector<ScanFilter>& filters,
                                   const ScanSettings& settings,
                                   const ScanCallback& callback) {
    bool already = false;
    {
        std::lock_guard<std::mutex> lock(mScanMutex);
        already = mScanActive;
    }
    if (already) {
        if (callback.onScanFailed)
            callback.onScanFailed(ScanCallback::SCAN_FAILED_ALREADY_STARTED);
        return false;
    }
    /* Push the service-UUID filter list down to BlueZ when the filters
     * agree on one (the D-Bus filter takes a list); everything else —
     * address/name — matches client-side per result. */
    std::vector<std::string> uuidFilter;
    for (const ScanFilter& f : filters) {
        if (f.hasServiceUuid()) uuidFilter.push_back(f.getServiceUuid().toString());
    }
    if (!mAdapter.client().startLeDiscovery(uuidFilter, INT16_MIN)) {
        if (callback.onScanFailed)
            callback.onScanFailed(ScanCallback::SCAN_FAILED_INTERNAL_ERROR);
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(mScanMutex);
        mFilters = filters;
        mCallback = callback;
        mScanActive = true;
    }
    return true;
}

bool BluetoothLeScanner::stopScan(const ScanCallback& callback) {
    {
        std::lock_guard<std::mutex> lock(mScanMutex);
        if (!mScanActive) return false;
        /* Remove by handle: only the registration that started the scan
         * stops it (EventSet identity — copies compare equal). */
        if (!(mCallback == callback)) return false;
        mScanActive = false;
        mCallback = ScanCallback();
        mFilters.clear();
    }
    return mAdapter.cancelDiscovery();
}

void BluetoothLeScanner::onDeviceFound(const BluetoothDevice& device) {
    /* Snapshot the scan config under the lock; the callback copy runs
     * outside it. One findDevice serves name/uuids/rssi — the review's
     * triple-lookup note fixed in passing. */
    ScanCallback callback;
    std::vector<ScanFilter> filters;
    {
        std::lock_guard<std::mutex> lock(mScanMutex);
        if (!mScanActive) return;
        callback = mCallback;
        filters = mFilters;
    }

    BluezDevice snapshot;
    mAdapter.client().findDevice(device.getAddress(), snapshot);
    const std::string name = snapshot.alias.empty() ? snapshot.name : snapshot.alias;
    const int rssi = snapshot.rssi;
    const int64_t now = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();

    /* Empty filter list accepts everything (AOSP semantics). */
    bool matched = filters.empty();
    for (const ScanFilter& f : filters) {
        if (!f.getDeviceAddress().empty()
                && f.getDeviceAddress() != device.getAddress()) continue;
        if (!f.getDeviceName().empty() && f.getDeviceName() != name) continue;
        if (f.hasServiceUuid()) {
            bool uuidMatched = false;
            for (const std::string& u : snapshot.uuids) {
                if (BluetoothUuid::fromString(u) == f.getServiceUuid()) {
                    uuidMatched = true;
                    break;
                }
            }
            if (!uuidMatched) continue;
        }
        matched = true;   /* first matching filter wins (AOSP semantics) */
        break;
    }
    if (matched && callback.onScanResult)
        callback.onScanResult(1 /* CALLBACK_TYPE_ALL_MATCHES */,
                              ScanResult(device, rssi, now));
}

void BluetoothLeScanner::onDiscoveryFinished() {
    /* LE scans run until stopped; nothing to finish. */
}

BluetoothLeScanner::~BluetoothLeScanner() {
    /* Detach: the adapter holds a copy of the bridge whose lambda
     * captures this (the old code deleted the bridge WITHOUT removing
     * the registration — a dangling entry). */
    mAdapter.removeDiscoveryListener(mDiscoveryBridge);
}

} // namespace cdroid
