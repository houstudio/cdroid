/**
 * BluetoothDevice — port of android.bluetooth.BluetoothDevice
 * (android-36), module phase. Property getters resolve through the
 * adapter's BlueZ cache at call time (see header).
 */
#include <algorithm>
#include <cctype>

#include <bluetoothadapter.h>
#include <bluetoothdevice.h>
#include <bluetoothgatt.h>
#include <bluetoothsocket.h>

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

BluetoothGatt* BluetoothDevice::connectGatt(bool autoConnect,
                                            BluetoothGattCallback* callback) const {
    return new BluetoothGatt(*this, autoConnect, callback);
}

BluetoothSocket* BluetoothDevice::createRfcommSocket(int channel) const {
    return new BluetoothSocket(*this, channel, true);
}

BluetoothSocket* BluetoothDevice::createRfcommSocketToServiceRecord(
        const BluetoothUuid& uuid) const {
    /* SDP resolution (L2CAP PSM 1 query) needs a live controller — the
     * resolver ships with the hardware bench. Interim: SPP maps to
     * channel 1, the de-facto SPP convention; anything else fails. */
    if (uuid == BluetoothUuid::SerialPort())
        return new BluetoothSocket(*this, 1, true);
    return nullptr;
}

BluetoothSocket* BluetoothDevice::createInsecureRfcommSocketToServiceRecord(
        const BluetoothUuid& uuid) const {
    if (uuid == BluetoothUuid::SerialPort())
        return new BluetoothSocket(*this, 1, false);
    return nullptr;
}

std::string BluetoothDevice::toString() const {
    return mAddress.empty() ? std::string() : "<" + mAddress + ">";
}

} // namespace cdroid
