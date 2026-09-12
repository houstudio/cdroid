#ifndef __CDROID_BLUETOOTH_GATT_H__
#define __CDROID_BLUETOOTH_GATT_H__

#include <cstdint>
#include <string>
#include <vector>

#include <bluetoothdevice.h>
#include <bluetoothuuid.h>

namespace cdroid {

class BluetoothGatt;
class BluetoothAdapter;

/**
 * Port of android.bluetooth.BluetoothGattCharacteristic (android-36):
 * a GATT characteristic handle. Instances are minted by
 * BluetoothGatt::discoverServices and owned by it; getValue/setValue
 * buffer locally, read()/write() round-trip the transport.
 */
class BluetoothGattCharacteristic {
public:
    static constexpr int PROPERTY_BROADCAST = 0x01;
    static constexpr int PROPERTY_READ = 0x02;
    static constexpr int PROPERTY_WRITE_NO_RESPONSE = 0x04;
    static constexpr int PROPERTY_WRITE = 0x08;
    static constexpr int PROPERTY_NOTIFY = 0x10;
    static constexpr int PROPERTY_INDICATE = 0x20;

    BluetoothUuid getUuid() const { return mUuid; }
    /* Instance id maps to the BlueZ object path tail; kept for parity. */
    int getInstanceId() const { return mInstanceId; }
    int getProperties() const { return mProperties; }

    /* Local value buffer (AOSP semantics: last read/written value).
     * setValue mirrors the Java boolean return (always true — no
     * length-limit validation exists on this port). */
    std::vector<uint8_t> getValue() const { return mValue; }
    bool setValue(const std::vector<uint8_t>& value) { mValue = value; return true; }

private:
    friend class BluetoothGatt;
    BluetoothGattCharacteristic(const BluetoothUuid& uuid, int instanceId,
                                int properties, const std::string& objectPath);
    BluetoothUuid mUuid;
    int mInstanceId;
    int mProperties;
    std::string mObjectPath;   /* BlueZ GattCharacteristic1 path */
    std::vector<uint8_t> mValue;
};

/**
 * Port of android.bluetooth.BluetoothGattService (android-36): a GATT
 * service with its characteristics.
 */
class BluetoothGattService {
public:
    static constexpr int SERVICE_TYPE_PRIMARY = 0;
    static constexpr int SERVICE_TYPE_SECONDARY = 1;

    BluetoothUuid getUuid() const { return mUuid; }
    int getInstanceId() const { return mInstanceId; }
    int getType() const { return mType; }

    std::vector<BluetoothGattCharacteristic*> getCharacteristics() const {
        return mCharacteristics;
    }
    BluetoothGattCharacteristic* getCharacteristic(const BluetoothUuid& uuid) const;

private:
    friend class BluetoothGatt;
    BluetoothGattService(const BluetoothUuid& uuid, int instanceId, int type,
                         const std::string& objectPath);
    ~BluetoothGattService();
    BluetoothUuid mUuid;
    int mInstanceId;
    int mType;
    std::string mObjectPath;
    std::vector<BluetoothGattCharacteristic*> mCharacteristics;
};

/**
 * Port of android.bluetooth.BluetoothGattCallback (android-36). All
 * callbacks arrive on the monitor thread (the cdnet convention — marshal
 * through the app's Handler/View::post for UI work).
 */
class BluetoothGattCallback {
public:
    virtual ~BluetoothGattCallback() = default;
    virtual void onConnectionStateChange(BluetoothGatt* gatt, int status,
                                         int newState) {}
    virtual void onServicesDiscovered(BluetoothGatt* gatt, int status) {}
    virtual void onCharacteristicRead(BluetoothGatt* gatt,
                                      BluetoothGattCharacteristic* characteristic,
                                      int status) {}
    virtual void onCharacteristicWrite(BluetoothGatt* gatt,
                                       BluetoothGattCharacteristic* characteristic,
                                       int status) {}
    /* Value arrived via notification/indication. */
    virtual void onCharacteristicChanged(BluetoothGatt* gatt,
                                         BluetoothGattCharacteristic* characteristic) {}
};

/**
 * Port of android.bluetooth.BluetoothGatt (android-36): the GATT client.
 * connect() on the AOSP side is asynchronous and the object must be
 * closed after use — same contract here: construct via
 * BluetoothDevice.connectGatt(context, autoConnect, callback), call
 * discoverServices() once onConnectionStateChange reports STATE_CONNECTED,
 * and close() when done (the destructor closes too).
 *
 * discoverServices() populates from BlueZ's GATT object tree (bluez
 * resolves services during Device1.Connect — the AOSP equivalent of the
 * service discovery cache).
 */
class BluetoothGatt {
public:
    static constexpr int STATE_DISCONNECTED = 0;
    static constexpr int STATE_CONNECTING = 1;
    static constexpr int STATE_CONNECTED = 2;

    /* GATT_SUCCESS and friends (BluetoothGatt/GattCallback codes). */
    static constexpr int GATT_SUCCESS = 0;
    static constexpr int GATT_FAILURE = 257;
    static constexpr int GATT_ERROR = 133;

    ~BluetoothGatt();

    bool connect();
    void disconnect();
    void close();

    bool discoverServices();
    std::vector<BluetoothGattService*> getServices() const { return mServices; }
    BluetoothGattService* getService(const BluetoothUuid& uuid) const;

    bool readCharacteristic(BluetoothGattCharacteristic* characteristic);
    bool writeCharacteristic(BluetoothGattCharacteristic* characteristic);
    bool setCharacteristicNotification(BluetoothGattCharacteristic* characteristic,
                                      bool enable);

private:
    friend class BluetoothDevice;
    friend class BluetoothAdapter;   /* characteristic-changed fan-out */
    BluetoothGatt(const BluetoothDevice& device, bool autoConnect,
                  BluetoothGattCallback* callback);
    /* BlueZ characteristic Value/Notifying flip (monitor thread). */
    void onCharacteristicChangedInternal(const std::string& objectPath,
                                         const std::vector<uint8_t>& value);

    BluetoothDevice mDevice;
    bool mAutoConnect;
    BluetoothGattCallback* mCallback;
    class BluezClient& mClient;      /* shared transport (via the adapter) */
    int mConnectionState = STATE_DISCONNECTED;
    std::vector<BluetoothGattService*> mServices;
    bool mClosed = false;
};

} // namespace cdroid

#endif /* __CDROID_BLUETOOTH_GATT_H__ */
