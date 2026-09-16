/**
 * BluetoothGatt family — GATT client over BlueZ (GattService1 /
 * GattCharacteristic1 objects). Device1.Connect carries the bearer;
 * characteristics read/write/notify map to the D-Bus methods of the
 * same names.
 */
#include <algorithm>
#include <memory>

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

std::shared_ptr<BluetoothGatt> BluetoothGatt::create(
        const BluetoothDevice& device, bool autoConnect,
        const BluetoothGattCallback& callback) {
    /* shared_ptr from the start: enable_shared_from_this needs the
     * object to be owned by one at construction. autoConnect is
     * accepted for API parity (background reconnect is not wired). */
    (void)autoConnect;
    return std::shared_ptr<BluetoothGatt>(
            new BluetoothGatt(device, callback));
}

BluetoothGatt::BluetoothGatt(const BluetoothDevice& device,
                             const BluetoothGattCallback& callback)
    : mDevice(device), mCallback(callback),
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
    if (mCallback.onConnectionStateChange)
        mCallback.onConnectionStateChange(*this, GATT_SUCCESS, STATE_CONNECTING);
    if (!mClient.connectDevice(mDevice.getAddress())) {
        {std::lock_guard<std::mutex> lock(mStateMutex);
         mConnectionState = STATE_DISCONNECTED;}
        if (mCallback.onConnectionStateChange)
            mCallback.onConnectionStateChange(*this, GATT_FAILURE,
                                               STATE_DISCONNECTED);
        return false;
    }
    {std::lock_guard<std::mutex> lock(mStateMutex);
     mConnectionState = STATE_CONNECTED;}
    auto self = shared_from_this();
    BluetoothAdapter::getDefaultAdapter().registerGattSession(self);
    if (mCallback.onConnectionStateChange)
        mCallback.onConnectionStateChange(*this, GATT_SUCCESS,
                                           STATE_CONNECTED);
    return true;
}

void BluetoothGatt::disconnect() {
    if (mClosed) return;
    mClient.disconnectDevice(mDevice.getAddress());
    {std::lock_guard<std::mutex> lock(mStateMutex);
     mConnectionState = STATE_DISCONNECTED;}
    if (mCallback.onConnectionStateChange)
        mCallback.onConnectionStateChange(*this, GATT_SUCCESS,
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
    if (mCallback.onServicesDiscovered) mCallback.onServicesDiscovered(*this, GATT_SUCCESS);
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
        if (mCallback.onCharacteristicRead)
            mCallback.onCharacteristicRead(*this, *characteristic, GATT_FAILURE);
        return false;
    }
    characteristic->setValue(value);
    if (mCallback.onCharacteristicRead)
        mCallback.onCharacteristicRead(*this, *characteristic, GATT_SUCCESS);
    return true;
}

bool BluetoothGatt::writeCharacteristic(
        BluetoothGattCharacteristic* characteristic) {
    if (mClosed || characteristic == nullptr) return false;
    const bool withoutResponse = (characteristic->getProperties()
            & BluetoothGattCharacteristic::PROPERTY_WRITE_NO_RESPONSE) != 0;
    if (!mClient.gattWrite(characteristic->mObjectPath,
                           characteristic->getValue(), withoutResponse)) {
        if (mCallback.onCharacteristicWrite)
            mCallback.onCharacteristicWrite(*this, *characteristic,
                                             GATT_FAILURE);
        return false;
    }
    if (mCallback.onCharacteristicWrite)
        mCallback.onCharacteristicWrite(*this, *characteristic, GATT_SUCCESS);
    return true;
}

bool BluetoothGatt::setCharacteristicNotification(
        BluetoothGattCharacteristic* characteristic, bool enable) {
    if (mClosed || characteristic == nullptr) return false;
    return mClient.gattSetNotify(characteristic->mObjectPath, enable);
}

void BluetoothGatt::onCharacteristicChangedInternal(
        const std::string& objectPath, const std::vector<uint8_t>& value) {
    BluetoothGattCharacteristic* hit = nullptr;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        if (mClosed) return;
        for (BluetoothGattService* s : mServices) {
            for (BluetoothGattCharacteristic* c : s->mCharacteristics) {
                if (c->mObjectPath == objectPath) { hit = c; break; }
            }
            if (hit) break;
        }
        if (hit) hit->setValue(value);
    }
    /* Callback OUTSIDE mStateMutex (same rule as onServicesDiscovered):
     * AOSP-idiomatic listeners call getService()/close() from
     * onCharacteristicChanged and self-deadlock on a held lock. The
     * characteristic pointer stays valid — close()/discoverServices()
     * are the only paths that free it, and both run on the app thread
     * which is not this (monitor) thread; the adapter's session
     * registry plus mClosed gate the teardown ordering. */
    if (hit && mCallback.onCharacteristicChanged)
        mCallback.onCharacteristicChanged(*this, *hit);
}

} // namespace cdroid
