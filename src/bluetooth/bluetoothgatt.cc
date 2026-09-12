/**
 * BluetoothGatt family — GATT client over BlueZ (GattService1 /
 * GattCharacteristic1 objects). Device1.Connect carries the bearer;
 * characteristics read/write/notify map to the D-Bus methods of the
 * same names.
 */
#include <algorithm>

#include <bluetoothadapter.h>
#include <bluetoothgatt.h>
#include <bluezclient.h>

namespace cdroid {

/* ------------------------------------------------------------------ */
/* characteristic / service                                            */
/* ------------------------------------------------------------------ */

BluetoothGattCharacteristic::BluetoothGattCharacteristic(
        const BluetoothUuid& uuid, int instanceId, int properties,
        const std::string& objectPath)
    : mUuid(uuid), mInstanceId(instanceId), mProperties(properties),
      mObjectPath(objectPath) {}

BluetoothGattService::BluetoothGattService(const BluetoothUuid& uuid,
                                           int instanceId, int type,
                                           const std::string& objectPath)
    : mUuid(uuid), mInstanceId(instanceId), mType(type),
      mObjectPath(objectPath) {}

BluetoothGattService::~BluetoothGattService() {
    for (BluetoothGattCharacteristic* c : mCharacteristics) delete c;
}

BluetoothGattCharacteristic* BluetoothGattService::getCharacteristic(
        const BluetoothUuid& uuid) const {
    for (BluetoothGattCharacteristic* c : mCharacteristics) {
        if (c->getUuid() == uuid) return c;
    }
    return nullptr;
}

/* ------------------------------------------------------------------ */
/* BluetoothGatt                                                       */
/* ------------------------------------------------------------------ */

BluetoothGatt::BluetoothGatt(const BluetoothDevice& device, bool autoConnect,
                             BluetoothGattCallback* callback)
    : mDevice(device), mAutoConnect(autoConnect), mCallback(callback),
      mClient(BluetoothAdapter::getDefaultAdapter().client()) {}

BluetoothGatt::~BluetoothGatt() {
    close();
}

void BluetoothGatt::close() {
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        if (mClosed) return;
        mClosed = true;
    }
    /* unregister first so no NEW fan-out starts, then take the state
     * lock (a monitor already past the registry may still be inside
     * onCharacteristicChangedInternal — the lock serializes it) */
    BluetoothAdapter::getDefaultAdapter().unregisterGattSession(this);
    std::lock_guard<std::mutex> lock(mStateMutex);
    for (BluetoothGattService* s : mServices) delete s;
    mServices.clear();
}

bool BluetoothGatt::connect() {
    if (mClosed) return false;
    {std::lock_guard<std::mutex> lock(mStateMutex);
     mConnectionState = STATE_CONNECTING;}
    if (mCallback)
        mCallback->onConnectionStateChange(this, GATT_SUCCESS, STATE_CONNECTING);
    if (!mClient.connectDevice(mDevice.getAddress())) {
        {std::lock_guard<std::mutex> lock(mStateMutex);
         mConnectionState = STATE_DISCONNECTED;}
        if (mCallback)
            mCallback->onConnectionStateChange(this, GATT_FAILURE,
                                               STATE_DISCONNECTED);
        return false;
    }
    {std::lock_guard<std::mutex> lock(mStateMutex);
     mConnectionState = STATE_CONNECTED;}
    BluetoothAdapter::getDefaultAdapter().registerGattSession(this);
    if (mCallback)
        mCallback->onConnectionStateChange(this, GATT_SUCCESS,
                                           STATE_CONNECTED);
    return true;
}

void BluetoothGatt::disconnect() {
    if (mClosed) return;
    mClient.disconnectDevice(mDevice.getAddress());
    {std::lock_guard<std::mutex> lock(mStateMutex);
     mConnectionState = STATE_DISCONNECTED;}
    if (mCallback)
        mCallback->onConnectionStateChange(this, GATT_SUCCESS,
                                           STATE_DISCONNECTED);
}

bool BluetoothGatt::discoverServices() {
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        if (mClosed || mConnectionState != STATE_CONNECTED) return false;
        for (BluetoothGattService* s : mServices) delete s;
        mServices.clear();
    }
    /* BlueZ resolves the GATT tree during Connect; enumerate what the
     * cache holds for this device now. */
    /* A snapshot refresh first: GATT objects created by Device1.Connect
     * arrive as InterfacesAdded — the cache keeps up now, and an explicit
     * refresh covers objects that predate the signal wiring (review #2). */
    mClient.refreshManagedObjects();
    {
    std::lock_guard<std::mutex> lock(mStateMutex);
    for (const BluezGattService& svc : mClient.getGattServices(
            mDevice.getAddress())) {
        auto* service = new BluetoothGattService(
                BluetoothUuid::fromString(svc.uuid), 0,
                BluetoothGattService::SERVICE_TYPE_PRIMARY, svc.objectPath);
        int instanceId = 0;
        for (const BluezGattCharacteristic& ch :
                mClient.getGattCharacteristics(svc.objectPath)) {
            int properties = 0;
            for (const std::string& flag : ch.flags) {
                if (flag == "read") properties |= BluetoothGattCharacteristic::PROPERTY_READ;
                else if (flag == "write") properties |= BluetoothGattCharacteristic::PROPERTY_WRITE;
                else if (flag == "write-without-response")
                    properties |= BluetoothGattCharacteristic::PROPERTY_WRITE_NO_RESPONSE;
                else if (flag == "notify") properties |= BluetoothGattCharacteristic::PROPERTY_NOTIFY;
                else if (flag == "indicate") properties |= BluetoothGattCharacteristic::PROPERTY_INDICATE;
                else if (flag == "broadcast") properties |= BluetoothGattCharacteristic::PROPERTY_BROADCAST;
            }
            auto* characteristic = new BluetoothGattCharacteristic(
                    BluetoothUuid::fromString(ch.uuid), instanceId++,
                    properties, ch.objectPath);
            characteristic->setValue(ch.value);
            /* service owns its characteristics (AOSP ownership shape) */
            service->mCharacteristics.push_back(characteristic);
        }
        mServices.push_back(service);
    }
    }   /* mStateMutex released before the callback: listeners call
        * getServices()/getCharacteristics() and would self-deadlock */
    if (mCallback) mCallback->onServicesDiscovered(this, GATT_SUCCESS);
    return true;
}

BluetoothGattService* BluetoothGatt::getService(const BluetoothUuid& uuid) const {
    std::lock_guard<std::mutex> lock(mStateMutex);
    for (BluetoothGattService* s : mServices) {
        if (s->getUuid() == uuid) return s;
    }
    return nullptr;
}

bool BluetoothGatt::readCharacteristic(
        BluetoothGattCharacteristic* characteristic) {
    if (mClosed || characteristic == nullptr) return false;
    std::vector<uint8_t> value;
    if (!mClient.gattRead(characteristic->mObjectPath, value)) {
        if (mCallback)
            mCallback->onCharacteristicRead(this, characteristic, GATT_FAILURE);
        return false;
    }
    characteristic->setValue(value);
    if (mCallback)
        mCallback->onCharacteristicRead(this, characteristic, GATT_SUCCESS);
    return true;
}

bool BluetoothGatt::writeCharacteristic(
        BluetoothGattCharacteristic* characteristic) {
    if (mClosed || characteristic == nullptr) return false;
    const bool withoutResponse = (characteristic->getProperties()
            & BluetoothGattCharacteristic::PROPERTY_WRITE_NO_RESPONSE) != 0;
    if (!mClient.gattWrite(characteristic->mObjectPath,
                           characteristic->getValue(), withoutResponse)) {
        if (mCallback)
            mCallback->onCharacteristicWrite(this, characteristic,
                                             GATT_FAILURE);
        return false;
    }
    if (mCallback)
        mCallback->onCharacteristicWrite(this, characteristic, GATT_SUCCESS);
    return true;
}

bool BluetoothGatt::setCharacteristicNotification(
        BluetoothGattCharacteristic* characteristic, bool enable) {
    if (mClosed || characteristic == nullptr) return false;
    return mClient.gattSetNotify(characteristic->mObjectPath, enable);
}

void BluetoothGatt::onCharacteristicChangedInternal(
        const std::string& objectPath, const std::vector<uint8_t>& value) {
    std::lock_guard<std::mutex> lock(mStateMutex);
    if (mClosed) return;
    for (BluetoothGattService* s : mServices) {
        for (BluetoothGattCharacteristic* c : s->mCharacteristics) {
            if (c->mObjectPath == objectPath) {
                c->setValue(value);
                if (mCallback) mCallback->onCharacteristicChanged(this, c);
                return;
            }
        }
    }
}

} // namespace cdroid
