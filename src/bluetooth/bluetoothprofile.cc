/**
 * BluetoothA2dp / BluetoothHeadset — faithful stubs (android-36 shape,
 * disconnected defaults). The BlueZ side of these profiles (Media1
 * endpoints, SCO routing) plugs into an audio stack CDROID does not
 * feed yet; bodies note the dependency per the house stub convention.
 */
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

} // namespace cdroid
