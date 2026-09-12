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

namespace {

/* Bridge: the scanner keeps one adapter DiscoveryListener registration
 * (bluetoothle.h cannot include bluetoothadapter.h — the adapter header
 * includes this module's types indirectly). */
class ScannerBridge : public BluetoothAdapter::DiscoveryListener {
public:
    explicit ScannerBridge(BluetoothLeScanner* owner) : mOwner(owner) {}
    void onDeviceFound(const BluetoothDevice& device) override {
        mOwner->onDeviceFound(device);
    }
private:
    BluetoothLeScanner* mOwner;
};

} // namespace

BluetoothLeScanner::BluetoothLeScanner(BluetoothAdapter& adapter)
    : mAdapter(adapter), mBridge(new ScannerBridge(this)) {
    mAdapter.addDiscoveryListener(static_cast<ScannerBridge*>(mBridge));
}

bool BluetoothLeScanner::startScan(const std::vector<ScanFilter>& filters,
                                   const ScanSettings& settings,
                                   ScanCallback* callback) {
    if (callback == nullptr) return false;
    if (mCallback != nullptr) {
        callback->onScanFailed(ScanCallback::SCAN_FAILED_ALREADY_STARTED);
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
        callback->onScanFailed(ScanCallback::SCAN_FAILED_INTERNAL_ERROR);
        return false;
    }
    mFilters = filters;
    mCallback = callback;
    return true;
}

bool BluetoothLeScanner::stopScan(ScanCallback* /*callback*/) {
    if (mCallback == nullptr) return false;
    mCallback = nullptr;
    mFilters.clear();
    return mAdapter.cancelDiscovery();
}

void BluetoothLeScanner::onDeviceFound(const BluetoothDevice& device) {
    if (mCallback == nullptr) return;
    /* Empty filter list accepts everything (AOSP semantics). */
    if (mFilters.empty()) {
        BluezDevice snapshot;
        int rssi = 0;
        if (mAdapter.client().findDevice(device.getAddress(), snapshot))
            rssi = snapshot.rssi;
        const int64_t now = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
        mCallback->onScanResult(1, ScanResult(device, rssi, now));
        return;
    }
    for (const ScanFilter& f : mFilters) {
        if (!f.getDeviceAddress().empty()
                && f.getDeviceAddress() != device.getAddress()) continue;
        if (!f.getDeviceName().empty()
                && f.getDeviceName() != device.getName()) continue;
        if (f.hasServiceUuid()) {
            /* advertisement service list = the device's cached UUIDs
             * (simplified ScanRecord); match on string form */
            BluezDevice snapshot;
            bool matched = false;
            if (mAdapter.client().findDevice(device.getAddress(), snapshot)) {
                for (const std::string& u : snapshot.uuids) {
                    if (BluetoothUuid::fromString(u) == f.getServiceUuid()) {
                        matched = true;
                        break;
                    }
                }
            }
            if (!matched) continue;
        }
        BluezDevice snapshot;
        int rssi = 0;
        if (mAdapter.client().findDevice(device.getAddress(), snapshot))
            rssi = snapshot.rssi;
        const int64_t now = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
        mCallback->onScanResult(1 /* CALLBACK_TYPE_ALL_MATCHES */,
                                ScanResult(device, rssi, now));
        return;   /* first matching filter wins (AOSP semantics) */
    }
}

void BluetoothLeScanner::onDiscoveryFinished() {
    /* LE scans run until stopped; nothing to finish. */
}

BluetoothLeScanner::~BluetoothLeScanner() {
    delete static_cast<ScannerBridge*>(mBridge);
}

} // namespace cdroid
