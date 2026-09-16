#ifndef __CDROID_BLUETOOTH_LE_H__
#define __CDROID_BLUETOOTH_LE_H__

#include <functional>
#include <mutex>
#include <string>
#include <vector>

#include <core/callbackbase.h>   /* EventSet listener base (header-only) */

/* BluetoothAdapter only forward-declares this class, so including its
 * header here forms no cycle — needed for the DiscoveryListener member. */
#include <bluetoothadapter.h>

#include <bluetoothdevice.h>
#include <bluetoothuuid.h>

namespace cdroid {

/**
 * Port of android.bluetooth.le.ScanFilter (android-36): a filter on LE
 * scan results. The AOSP builder becomes plain setters on the aggregate
 * value type (the CDROID convention for parameter objects).
 */
class ScanFilter {
public:
    ScanFilter& setDeviceAddress(const std::string& address) {
        mDeviceAddress = address;
        return *this;
    }
    ScanFilter& setDeviceName(const std::string& name) {
        mDeviceName = name;
        return *this;
    }
    ScanFilter& setServiceUuid(const BluetoothUuid& uuid) {
        mServiceUuid = uuid;
        mHasServiceUuid = true;
        return *this;
    }
    const std::string& getDeviceAddress() const { return mDeviceAddress; }
    const std::string& getDeviceName() const { return mDeviceName; }
    bool hasServiceUuid() const { return mHasServiceUuid; }
    const BluetoothUuid& getServiceUuid() const { return mServiceUuid; }
private:
    std::string mDeviceAddress;
    std::string mDeviceName;
    BluetoothUuid mServiceUuid;
    bool mHasServiceUuid = false;
};

/**
 * Port of android.bluetooth.le.ScanSettings (android-36), the subset the
 * BlueZ discovery filter can express.
 */
class ScanSettings {
public:
    /* Scan modes (SCAN_MODE_*). */
    static constexpr int SCAN_MODE_LOW_POWER = 0;
    static constexpr int SCAN_MODE_BALANCED = 1;
    static constexpr int SCAN_MODE_LOW_LATENCY = 2;

    ScanSettings& setScanMode(int mode) { mScanMode = mode; return *this; }
    ScanSettings& setCallbackType(int type) { mCallbackType = type; return *this; }
    int getScanMode() const { return mScanMode; }
private:
    int mScanMode = SCAN_MODE_LOW_POWER;
    int mCallbackType = 1 /* CALLBACK_TYPE_ALL_MATCHES */;
};

/**
 * Port of android.bluetooth.le.ScanResult (android-36): one LE
 * advertisement observation.
 */
class ScanResult {
public:
    ScanResult(const BluetoothDevice& device, int rssi, int64_t timestampNanos)
        : mDevice(device), mRssi(rssi), mTimestampNanos(timestampNanos) {}
    BluetoothDevice getDevice() const { return mDevice; }
    /* SecondaryPhy/AdvertisingSid/TxPower/PeriodicAdvertisingInterval are
     * extended-frame fields BlueZ does not surface; ScanRecord's service
     * UUIDs ride on BluetoothDevice's discovered UUIDs instead. */
    int getRssi() const { return mRssi; }
    int64_t getTimestampNanos() const { return mTimestampNanos; }
private:
    BluetoothDevice mDevice;
    int mRssi;
    int64_t mTimestampNanos;
};

/**
 * Port of android.bluetooth.le.ScanCallback (android-36): the result
 * surface for BluetoothLeScanner. EventSet + std::function slots —
 * identity (==) is what stopScan(callback) matches on; unset slots are
 * no-ops. Callbacks arrive on the monitor thread (cdnet marshaling
 * convention).
 */
class ScanCallback : public EventSet {
public:
    static constexpr int SCAN_FAILED_ALREADY_STARTED = 1;
    static constexpr int SCAN_FAILED_INTERNAL_ERROR = 3;
    std::function<void(int callbackType, const ScanResult&)> onScanResult;
    std::function<void(const std::vector<ScanResult>&)> onBatchScanResults;
    std::function<void(int errorCode)> onScanFailed;
};

/**
 * Port of android.bluetooth.le.BluetoothLeScanner (android-36), obtained
 * from BluetoothAdapter.getBluetoothLeScanner(). Backed by the BlueZ
 * adapter discovery with an LE transport filter; per-filter matching
 * happens client-side on the discovery results.
 */
class BluetoothLeScanner {
public:
    /* startScan with filters/settings; an empty filter list accepts
     * everything (AOSP semantics). The callback is stored as a value
     * copy; stopScan matches it by EventSet identity. */
    bool startScan(const std::vector<ScanFilter>& filters,
                   const ScanSettings& settings, const ScanCallback& callback);
    bool stopScan(const ScanCallback& callback);

    /* adapter-found devices flow through the filter set into the scan
     * callback (wired to the adapter's discovery stream through
     * mDiscoveryBridge) */
    void onDeviceFound(const BluetoothDevice& device);
    void onDiscoveryFinished();

private:
    friend class BluetoothAdapter;
    explicit BluetoothLeScanner(BluetoothAdapter& adapter);

    BluetoothAdapter& mAdapter;
    /* adapter discovery registration (was a heap ScannerBridge subclass
     * + a void* mBridge pair) */
    BluetoothAdapter::DiscoveryListener mDiscoveryBridge;
    /* Written by startScan/stopScan (caller thread), read by
     * onDeviceFound (monitor thread) — guarded (review's UAF). */
    mutable std::mutex mScanMutex;
    std::vector<ScanFilter> mFilters;
    bool mScanActive = false;   /* an active registration exists (mScanMutex) */
    ScanCallback mCallback;     /* the active registration (mScanMutex) */
public:
    ~BluetoothLeScanner();
};

} // namespace cdroid

#endif /* __CDROID_BLUETOOTH_LE_H__ */
