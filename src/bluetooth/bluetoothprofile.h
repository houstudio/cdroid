#ifndef __CDROID_BLUETOOTH_PROFILE_H__
#define __CDROID_BLUETOOTH_PROFILE_H__

#include <vector>

#include <bluetoothdevice.h>

namespace cdroid {

class BluetoothA2dp;
class BluetoothHeadset;

/**
 * Port of android.bluetooth.BluetoothProfile (android-36), module phase.
 * The profile-proxy plumbing (getProfileProxy/ServiceListener) keeps its
 * AOSP shape; the concrete profiles are faithful stubs — the audio and
 * input pipelines they drive (AudioTrack routing, HAL input) are not
 * ported yet, exactly like InsertModeTransformationMethod sits behind
 * its @hide frameworks dependency. Every stub body notes what the real
 * implementation needs.
 */
class BluetoothProfile {
public:
    /* Profile id constants (BluetoothProfile.*). */
    static constexpr int HEADSET = 1;
    static constexpr int A2DP = 2;
    static constexpr int HID_HOST = 4;
    static constexpr int PAN = 5;
    static constexpr int PBAP = 6;
    static constexpr int HEALTH = 3;
    static constexpr int MAP = 9;
    static constexpr int A2DP_SINK = 11;
    static constexpr int HEADSET_CLIENT = 16;
    static constexpr int PBAP_CLIENT = 17;
    static constexpr int MAP_CLIENT = 18;
    static constexpr int HID_DEVICE = 19;
    static constexpr int HEARING_AID = 21;
    static constexpr int LE_AUDIO = 22;   /* android-36: 26 is LE_AUDIO_BROADCAST */

    /* Connection states (PROFILE_*). */
    static constexpr int PROFILE_DISCONNECTED = 0;
    static constexpr int PROFILE_CONNECTING = 1;
    static constexpr int PROFILE_CONNECTED = 2;
    static constexpr int PROFILE_DISCONNECTING = 3;

    /* Priority constants (PRIORITY_* / CONNECTION_PRIORITY_*). */
    static constexpr int PRIORITY_AUTO_CONNECT = 1000;
    static constexpr int PRIORITY_ON = 100;
    static constexpr int PRIORITY_OFF = 0;
    static constexpr int PRIORITY_UNDEFINED = -1;

    /**
     * Port of BluetoothProfile.ServiceListener: proxy attach/detach
     * notification. With in-process profiles the callback fires
     * synchronously from getProfileProxy(), the AOSP timing contract
     * (async delivery) is kept for parity.
     */
    class ServiceListener {
    public:
        virtual ~ServiceListener() = default;
        virtual void onServiceConnected(int profile,
                                        BluetoothProfile* proxy) = 0;
        virtual void onServiceDisconnected(int profile) = 0;
    };

    /* Interface methods (AOSP BluetoothProfile is an interface). */
    virtual std::vector<BluetoothDevice> getConnectedDevices() = 0;
    virtual int getConnectionState(const BluetoothDevice& device) = 0;
    virtual ~BluetoothProfile() = default;
};

/**
 * Port of android.bluetooth.BluetoothA2dp (android-36) — Advanced Audio
 * Distribution Profile source. STUB: BlueZ serves A2DP through its Media
 * endpoints against an audio stack (PipeWire/PulseAudio), and CDROID's
 * audio pipeline does not feed it yet. Signatures are the public surface;
 * bodies return the disconnected/empty defaults until that lands.
 */
class BluetoothA2dp : public BluetoothProfile {
public:
    /* Optional codecs (AOSP BluetoothA2dp.*). */
    static constexpr int OPTIONAL_CODECS_SUPPORT_UNKNOWN = 0;
    static constexpr int OPTIONAL_CODECS_NOT_SUPPORTED = 1;
    static constexpr int OPTIONAL_CODECS_SUPPORTED = 2;
    static constexpr int OPTIONAL_CODECS_PREF_UNKNOWN = 0;
    static constexpr int OPTIONAL_CODECS_PREF_DISABLE = 1;
    static constexpr int OPTIONAL_CODECS_PREF_ENABLE = 2;

    std::vector<BluetoothDevice> getConnectedDevices() override;
    int getConnectionState(const BluetoothDevice& device) override;
    /* TODO(porting): real routing needs the audio pipeline + BlueZ
     * Media1 endpoints; until then playback state is always false. */
    bool isA2dpPlaying(const BluetoothDevice& device);
    bool connect(const BluetoothDevice& device);
    bool disconnect(const BluetoothDevice& device);
};

/**
 * Port of android.bluetooth.BluetoothHeadset (android-36) — HFP.
 * STUB for the same reason as A2dp: the SCO audio path is not ported.
 */
class BluetoothHeadset : public BluetoothProfile {
public:
    /* Audio state constants (BluetoothHeadset.*). */
    static constexpr int STATE_AUDIO_DISCONNECTED = 10;
    static constexpr int STATE_AUDIO_CONNECTING = 11;
    static constexpr int STATE_AUDIO_CONNECTED = 12;

    std::vector<BluetoothDevice> getConnectedDevices() override;
    int getConnectionState(const BluetoothDevice& device) override;
    int getAudioState(const BluetoothDevice& device);
    bool connect(const BluetoothDevice& device);
    bool disconnect(const BluetoothDevice& device);
    /* SCO audio start/stop — TODO(porting) with the audio pipeline. */
    bool startVoiceRecognition(const BluetoothDevice& device);
    bool stopVoiceRecognition(const BluetoothDevice& device);
};

} // namespace cdroid

#endif /* __CDROID_BLUETOOTH_PROFILE_H__ */
