/**
 * BluetoothA2dp / BluetoothHeadset — faithful stubs (android-36 shape,
 * disconnected defaults). The BlueZ side of these profiles (Media1
 * endpoints, SCO routing) plugs into an audio stack CDROID does not
 * feed yet; bodies note the dependency per the house stub convention.
 *
 * BluetoothHidHost is LIVE: connect/disconnect/state ride the BlueZ
 * device cache and Device1.ConnectProfile; see the header for the
 * control-channel stubs.
 */
#include <bluetoothadapter.h>
#include <bluetoothprofile.h>

namespace cdroid {

/* --- BluetoothA2dp --------------------------------------------------- */

std::vector<BluetoothDevice> BluetoothA2dp::getConnectedDevices() {
    return {};   /* TODO(porting): BlueZ Media1 endpoint query */
}

int BluetoothA2dp::getConnectionState(const BluetoothDevice&) {
    return PROFILE_DISCONNECTED;
}

bool BluetoothA2dp::isA2dpPlaying(const BluetoothDevice&) {
    return false;
}

bool BluetoothA2dp::connect(const BluetoothDevice&) {
    return false;   /* TODO(porting): MediaEndpoint connect */
}

bool BluetoothA2dp::disconnect(const BluetoothDevice&) {
    return false;
}

/* --- BluetoothHeadset ------------------------------------------------ */

std::vector<BluetoothDevice> BluetoothHeadset::getConnectedDevices() {
    return {};   /* TODO(porting): HFP profile objects */
}

int BluetoothHeadset::getConnectionState(const BluetoothDevice&) {
    return PROFILE_DISCONNECTED;
}

int BluetoothHeadset::getAudioState(const BluetoothDevice&) {
    return STATE_AUDIO_DISCONNECTED;
}

bool BluetoothHeadset::connect(const BluetoothDevice&) {
    return false;
}

bool BluetoothHeadset::disconnect(const BluetoothDevice&) {
    return false;
}

bool BluetoothHeadset::startVoiceRecognition(const BluetoothDevice&) {
    return false;   /* TODO(porting): SCO audio routing */
}

bool BluetoothHeadset::stopVoiceRecognition(const BluetoothDevice&) {
    return false;
}

/* --- BluetoothHidHost ------------------------------------------------ */

namespace {

/* BlueZ advertises service UUIDs as 128-bit strings (lowercase); the
 * parse-and-compare path is case-proof either way. */
bool advertisesHid(const BluezDevice& d) {
    const BluetoothUuid hid = BluetoothUuid::HID();
    for (const std::string& u : d.uuids) {
        if (BluetoothUuid::fromString(u) == hid) return true;
    }
    return false;
}

/* BluetoothAdapter.priorityToConnectionPolicy / connectionPolicyToPriority
 * (the Java source's deprecated-pair mapping, android-36 dropped both
 * helpers from the adapter — kept file-local here). */
int priorityToConnectionPolicy(int priority) {
    if (priority == BluetoothProfile::PRIORITY_AUTO_CONNECT
            || priority == BluetoothProfile::PRIORITY_ON) {
        return BluetoothProfile::CONNECTION_POLICY_ALLOWED;
    }
    if (priority == BluetoothProfile::PRIORITY_OFF) {
        return BluetoothProfile::CONNECTION_POLICY_FORBIDDEN;
    }
    return BluetoothProfile::CONNECTION_POLICY_UNKNOWN;
}

int connectionPolicyToPriority(int connectionPolicy) {
    if (connectionPolicy == BluetoothProfile::CONNECTION_POLICY_ALLOWED) {
        return BluetoothProfile::PRIORITY_ON;
    }
    if (connectionPolicy == BluetoothProfile::CONNECTION_POLICY_FORBIDDEN) {
        return BluetoothProfile::PRIORITY_OFF;
    }
    return BluetoothProfile::PRIORITY_UNDEFINED;
}

}   // namespace

BluetoothHidHost::BluetoothHidHost(BluetoothAdapter& adapter)
    : mAdapter(adapter) {
}

std::vector<BluetoothDevice> BluetoothHidHost::getConnectedDevices() {
    std::vector<BluetoothDevice> out;
    for (const BluezDevice& d : mAdapter.client().getDevices()) {
        if (d.connected && advertisesHid(d))
            out.push_back(BluetoothDevice(d.address));
    }
    return out;
}

int BluetoothHidHost::getConnectionState(const BluetoothDevice& device) {
    /* One Connected flag per device in BlueZ: connected + HID UUID is
     * HID-connected (exact for the HID-only devices this serves). */
    BluezDevice d;
    if (!mAdapter.client().findDevice(device.getAddress(), d)
            || !d.connected || !advertisesHid(d)) {
        return PROFILE_DISCONNECTED;
    }
    return PROFILE_CONNECTED;
}

bool BluetoothHidHost::connect(const BluetoothDevice& device) {
    return mAdapter.client().connectDeviceProfile(
            device.getAddress(), BluetoothUuid::HID().toString());
}

bool BluetoothHidHost::disconnect(const BluetoothDevice& device) {
    return mAdapter.client().disconnectDeviceProfile(
            device.getAddress(), BluetoothUuid::HID().toString());
}

std::vector<BluetoothDevice> BluetoothHidHost::getDevicesMatchingConnectionStates(
        const std::vector<int>& states) {
    /* Stack-side query analog: HID devices in the cache whose (bool
     * Connected-derived) state is in the requested set. */
    std::vector<BluetoothDevice> out;
    for (const BluezDevice& d : mAdapter.client().getDevices()) {
        if (!advertisesHid(d)) continue;
        const int state = d.connected ? PROFILE_CONNECTED : PROFILE_DISCONNECTED;
        for (int want : states) {
            if (want == state) {
                out.push_back(BluetoothDevice(d.address));
                break;
            }
        }
    }
    return out;
}

bool BluetoothHidHost::setPriority(const BluetoothDevice& device, int priority) {
    /* AOSP: pure delegation over setConnectionPolicy. */
    return setConnectionPolicy(device, priorityToConnectionPolicy(priority));
}

int BluetoothHidHost::getPriority(const BluetoothDevice& device) {
    /* AOSP: pure delegation over getConnectionPolicy. */
    return connectionPolicyToPriority(getConnectionPolicy(device));
}

bool BluetoothHidHost::setConnectionPolicy(const BluetoothDevice& device,
                                           int connectionPolicy) {
    /* AOSP validates the value before touching the stack. */
    if (connectionPolicy != BluetoothProfile::CONNECTION_POLICY_FORBIDDEN
            && connectionPolicy != BluetoothProfile::CONNECTION_POLICY_ALLOWED) {
        return false;
    }
    /* BlueZ keeps no per-profile policy: the device-wide Trusted flag
     * (set automatically at bond) is the auto-connect gate. ALLOWED maps
     * to trusting, FORBIDDEN to untrusting — exact for the HID-only
     * devices this profile serves; on a multi-profile peer it widens to
     * the whole device (TODO(porting): per-profile policy store). */
    return mAdapter.client().setDeviceTrusted(
            device.getAddress(),
            connectionPolicy == BluetoothProfile::CONNECTION_POLICY_ALLOWED);
}

int BluetoothHidHost::getConnectionPolicy(const BluetoothDevice& device) {
    /* Mirror of setConnectionPolicy's mapping: bonded+trusted devices
     * auto-connect (ALLOWED), anything else is UNKNOWN. */
    BluezDevice d;
    if (!mAdapter.client().findDevice(device.getAddress(), d)) {
        return BluetoothProfile::CONNECTION_POLICY_UNKNOWN;
    }
    if (d.paired && d.trusted) return BluetoothProfile::CONNECTION_POLICY_ALLOWED;
    return BluetoothProfile::CONNECTION_POLICY_UNKNOWN;
}

/* HID control-channel operations: no BlueZ D-Bus surface exists (they
 * live on the L2CAP control channel, PSM 0x11) — faithful signatures,
 * TODO(porting) bodies until a raw control path is needed. */
bool BluetoothHidHost::virtualUnplug(const BluetoothDevice&) {
    return false;   /* TODO(porting): HID control channel */
}

bool BluetoothHidHost::getProtocolMode(const BluetoothDevice&) {
    return false;   /* TODO(porting): HID control channel */
}

bool BluetoothHidHost::setProtocolMode(const BluetoothDevice&, int) {
    return false;   /* TODO(porting): HID control channel */
}

bool BluetoothHidHost::getReport(const BluetoothDevice&, uint8_t,
                                 uint8_t, int) {
    return false;   /* TODO(porting): HID control channel */
}

bool BluetoothHidHost::setReport(const BluetoothDevice&, uint8_t,
                                 const std::string&) {
    return false;   /* TODO(porting): HID control channel */
}

bool BluetoothHidHost::sendData(const BluetoothDevice&, const std::string&) {
    return false;   /* TODO(porting): HID control channel */
}

bool BluetoothHidHost::getIdleTime(const BluetoothDevice&) {
    return false;   /* TODO(porting): HID control channel */
}

bool BluetoothHidHost::setIdleTime(const BluetoothDevice&, uint8_t) {
    return false;   /* TODO(porting): HID control channel */
}

} // namespace cdroid
