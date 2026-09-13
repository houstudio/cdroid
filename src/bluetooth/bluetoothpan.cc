#include <bluetoothpan.h>

#include <dirent.h>
#include <string.h>
#include <unistd.h>

#include <bluetoothadapter.h>

/* Module convention: fprintf-based logging (cdblue links no cdroid). */
#define LOGD(...) do { fprintf(stdout, "BluetoothPan: " __VA_ARGS__); fprintf(stdout, "\n"); } while (0)

namespace cdroid {

BluetoothPan::BluetoothPan(BluetoothAdapter& adapter)
    : mAdapter(adapter) {
}

/* --- NAP (this device shares its network) ------------------------------------ */

bool BluetoothPan::setBluetoothTethering(bool enabled) {
    if (enabled) {
        /* BlueZ accepts Register with a dangling bridge name and only fails
         * when a peer's bnep enslavement misses it; fail fast instead. */
        if (access((std::string("/sys/class/net/") + TETHERING_BRIDGE).c_str(),
                   F_OK) != 0) {
            LOGD("tethering bridge %s does not exist", TETHERING_BRIDGE);
            return false;   /* PAN_OPERATION_GENERIC_FAILURE */
        }
        if (!mAdapter.client().networkServerRegister("nap", TETHERING_BRIDGE)) {
            return false;   /* PAN_OPERATION_GENERIC_FAILURE */
        }
    } else {
        /* Unregister drops the server; bluetoothd tears the peer links. */
        if (!mAdapter.client().networkServerUnregister("nap")) {
            return false;
        }
    }
    return true;
}

bool BluetoothPan::isTetheringOn() const {
    return mAdapter.client().isNapServerRegistered();
}

std::vector<std::string> BluetoothPan::getTetheredIfaces() const {
    /* bluetoothd names its BNEP interfaces bnepX; every tethered peer
     * (bonded + BNEP connected) shows up as one. */
    std::vector<std::string> ifaces;
    DIR* dir = opendir("/sys/class/net");
    if (dir == nullptr) return ifaces;
    while (dirent* ent = readdir(dir)) {
        if (strncmp(ent->d_name, "bnep", 4) == 0) {
            ifaces.push_back(ent->d_name);
        }
    }
    closedir(dir);
    return ifaces;
}

/* --- PANU (this device uses the remote's network) ----------------------------- */

bool BluetoothPan::connect(const BluetoothDevice& device, std::string& ifaceOut) {
    const std::string address = device.getAddress();
    if (mPanuIfaces.count(address)) {
        ifaceOut = mPanuIfaces[address];
        return false;   /* PAN_CONNECT_FAILED_ALREADY_CONNECTED */
    }
    if (!mAdapter.client().networkConnect(address, "panu", ifaceOut)) {
        return false;   /* PAN_CONNECT_FAILED_ATTEMPT_FAILED */
    }
    mPanuIfaces[address] = ifaceOut;
    return true;
}

bool BluetoothPan::disconnect(const BluetoothDevice& device) {
    const std::string address = device.getAddress();
    if (!mPanuIfaces.count(address)) {
        return false;   /* PAN_DISCONNECT_FAILED_NOT_CONNECTED */
    }
    if (!mAdapter.client().networkDisconnect(address)) {
        return false;
    }
    mPanuIfaces.erase(address);
    return true;
}

/* --- BluetoothProfile --------------------------------------------------------- */

int BluetoothPan::getConnectionState(const BluetoothDevice& device) {
    if (mPanuIfaces.count(device.getAddress())) {
        return BluetoothProfile::STATE_CONNECTED;
    }
    return BluetoothProfile::STATE_DISCONNECTED;
}

std::vector<BluetoothDevice> BluetoothPan::getConnectedDevices() {
    std::vector<BluetoothDevice> devices;
    for (const auto& kv : mPanuIfaces) {
        devices.push_back(mAdapter.getRemoteDevice(kv.first));
    }
    return devices;
}

} // namespace cdroid
