/**
 * BluetoothDevice — port of android.bluetooth.BluetoothDevice
 * (android-36), module phase. Property getters resolve through the
 * adapter's BlueZ cache at call time (see header).
 */
#include <algorithm>
#include <cctype>
#include <cstdlib>

#include <bluetoothadapter.h>
#include <bluetoothdevice.h>
#include <bluetoothgatt.h>
#include <bluetoothsocket.h>
#include "internal/sdpclient.h"

namespace cdroid {

BluetoothDevice::BluetoothDevice(const std::string& address) : mAddress(address) {
    /* The Java helper validates and upper-cases; keep the same shape so
     * cache lookups (BlueZ reports upper-case) always match. */
    std::transform(mAddress.begin(), mAddress.end(), mAddress.begin(),
                   [](unsigned char c) { return std::toupper(c); });
}

std::string BluetoothDevice::getName() const {
    std::string out;
    BluetoothAdapter::getDefaultAdapter().resolveDeviceName(mAddress, out);
    return out;
}

std::string BluetoothDevice::getAlias() const {
    std::string out;
    BluetoothAdapter::getDefaultAdapter().resolveDeviceAlias(mAddress, out);
    return out;
}

bool BluetoothDevice::setAlias(const std::string& alias) {
    return BluetoothAdapter::getDefaultAdapter().setDeviceAlias(mAddress, alias);
}

int BluetoothDevice::getBondState() const {
    return BluetoothAdapter::getDefaultAdapter().resolveBondState(mAddress);
}

BluetoothClass BluetoothDevice::getBluetoothClass() const {
    return BluetoothClass(
            BluetoothAdapter::getDefaultAdapter().resolveDeviceClass(mAddress));
}

int BluetoothDevice::getType() const {
    return BluetoothAdapter::getDefaultAdapter().resolveDeviceType(mAddress);
}

bool BluetoothDevice::createBond() {
    return BluetoothAdapter::getDefaultAdapter().bondDevice(mAddress);
}

bool BluetoothDevice::removeBond() {
    return BluetoothAdapter::getDefaultAdapter().unbondDevice(mAddress);
}

bool BluetoothDevice::setPin(const std::string& pin) {
    return BluetoothAdapter::getDefaultAdapter().replyPairingPin(pin);
}

bool BluetoothDevice::setPairingConfirmation(bool confirm) {
    return BluetoothAdapter::getDefaultAdapter().replyPairingConfirmation(confirm);
}

bool BluetoothDevice::cancelPairingUserInput() {
    BluetoothAdapter::getDefaultAdapter().cancelPairingUserInput();
    return true;
}

std::shared_ptr<BluetoothGatt> BluetoothDevice::connectGatt(
        bool autoConnect, const BluetoothGattCallback& callback) const {
    return BluetoothGatt::create(*this, autoConnect, callback);
}

BluetoothSocket* BluetoothDevice::createRfcommSocket(int channel) const {
    return new BluetoothSocket(*this, channel, true);
}

BluetoothSocket* BluetoothDevice::createRfcommSocketToServiceRecord(
        const BluetoothUuid& uuid) const {
    /* AOSP: no I/O in the factory — the channel stays unresolved (-1)
     * and connect() runs the SDP lookup, where blocking is the
     * documented contract (apps keep connect() off the UI thread). */
    return new BluetoothSocket(*this, -1, true, uuid);
}

BluetoothSocket* BluetoothDevice::createInsecureRfcommSocketToServiceRecord(
        const BluetoothUuid& uuid) const {
    return new BluetoothSocket(*this, -1, false, uuid);
}

std::string BluetoothDevice::toString() const {
    return mAddress.empty() ? std::string() : "<" + mAddress + ">";
}

} // namespace cdroid
