#ifndef __CDROID_BLUETOOTH_PROFILE_H__
#define __CDROID_BLUETOOTH_PROFILE_H__

#include <cstdint>
#include <functional>
#include <vector>

#include <core/callbackbase.h>   /* EventSet listener base (header-only) */

#include <bluetoothdevice.h>

namespace cdroid {

class BluetoothA2dp;
class BluetoothHeadset;
class BluetoothAdapter;

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
    /* Profile connection states (AOSP BluetoothProfile.*). */
    static constexpr int STATE_DISCONNECTED = 0;
    static constexpr int STATE_CONNECTING = 1;
    static constexpr int STATE_CONNECTED = 2;
    static constexpr int STATE_DISCONNECTING = 3;

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

    /* Connection policy constants — the modern names of the same values
     * (CONNECTION_POLICY_* maps 1:1 against PRIORITY_*). */
    static constexpr int CONNECTION_POLICY_FORBIDDEN = 0;
    static constexpr int CONNECTION_POLICY_ALLOWED = 100;
    static constexpr int CONNECTION_POLICY_UNKNOWN = -1;

    /**
     * Port of BluetoothProfile.ServiceListener: proxy attach/detach
     * notification. EventSet + std::function slots (identity in EventSet,
     * unset slots are no-ops); the proxy stays a raw pointer because its
     * identity is what closeProfileProxy() deletes. With in-process
     * profiles the callback fires synchronously from getProfileProxy(),
     * the AOSP timing contract (async delivery) is kept for parity.
     */
    class ServiceListener : public EventSet {
    public:
        std::function<void(int profile,
                          BluetoothProfile* proxy)> onServiceConnected;
        std::function<void(int profile)> onServiceDisconnected;
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

/**
 * Port of android.bluetooth.BluetoothHidHost (android-36) — HID host role
 * (keyboards, mice). LIVE, unlike the audio stubs above: BlueZ's input
 * plugin owns the HID channels (L2CAP PSM 0x11/0x13) and injects the
 * device into the kernel via uhid, where CDROID's evdev input path picks
 * it up — no CDROID-side pipeline is missing.
 *
 *   - connect/disconnect -> Device1.ConnectProfile/DisconnectProfile with
 *     the HID UUID (BlueZ connects exactly that profile)
 *   - connection state -> the device cache: BlueZ exposes one Connected
 *     flag per DEVICE, so a connected device advertising the HID UUID is
 *     reported as HID-connected (exact for the HID-only devices this
 *     profile serves)
 *   - connection policy -> the device-wide BlueZ Trusted flag (bond sets
 *     it automatically): ALLOWED/FORBIDDEN map to trusting/untrusting —
 *     exact for HID-only peers, device-wide on multi-profile ones
 *
 * The HID control-channel operations (protocol mode, reports, idle) have
 * no BlueZ D-Bus surface — stubs until a raw L2CAP control path exists.
 */
class BluetoothHidHost : public BluetoothProfile {
public:
    /* Action/extra string values verbatim from android-36
     * BluetoothHidHost (kept for the future broadcast system). */
    static constexpr const char* ACTION_CONNECTION_STATE_CHANGED =
            "android.bluetooth.input.profile.action.CONNECTION_STATE_CHANGED";
    static constexpr const char* ACTION_PROTOCOL_MODE_CHANGED =
            "android.bluetooth.input.profile.action.PROTOCOL_MODE_CHANGED";
    static constexpr const char* ACTION_HANDSHAKE =
            "android.bluetooth.input.profile.action.HANDSHAKE";
    static constexpr const char* ACTION_REPORT =
            "android.bluetooth.input.profile.action.REPORT";
    static constexpr const char* ACTION_VIRTUAL_UNPLUG_STATUS =
            "android.bluetooth.input.profile.action.VIRTUAL_UNPLUG_STATUS";
    static constexpr const char* ACTION_IDLE_TIME_CHANGED =
            "android.bluetooth.input.profile.action.IDLE_TIME_CHANGED";
    static constexpr const char* EXTRA_PROTOCOL_MODE =
            "android.bluetooth.BluetoothHidHost.extra.PROTOCOL_MODE";
    static constexpr const char* EXTRA_REPORT_TYPE =
            "android.bluetooth.BluetoothHidHost.extra.REPORT_TYPE";
    static constexpr const char* EXTRA_REPORT_ID =
            "android.bluetooth.BluetoothHidHost.extra.REPORT_ID";
    static constexpr const char* EXTRA_REPORT =
            "android.bluetooth.BluetoothHidHost.extra.REPORT";
    static constexpr const char* EXTRA_STATUS =
            "android.bluetooth.BluetoothHidHost.extra.STATUS";
    static constexpr const char* EXTRA_VIRTUAL_UNPLUG_STATUS =
            "android.bluetooth.BluetoothHidHost.extra.VIRTUAL_UNPLUG_STATUS";
    static constexpr const char* EXTRA_IDLE_TIME =
            "android.bluetooth.BluetoothHidHost.extra.IDLE_TIME";

    /* Result codes (AOSP BluetoothHidHost.INPUT_*). */
    static constexpr int INPUT_DISCONNECT_FAILED_NOT_CONNECTED = 5000;
    static constexpr int INPUT_CONNECT_FAILED_ALREADY_CONNECTED = 5001;
    static constexpr int INPUT_CONNECT_FAILED_ATTEMPT_FAILED = 5002;
    static constexpr int INPUT_OPERATION_GENERIC_FAILURE = 5003;
    static constexpr int INPUT_OPERATION_SUCCESS = 5004;

    /* Protocol modes (PROTOCOL_*). */
    static constexpr int PROTOCOL_REPORT_MODE = 0;
    static constexpr int PROTOCOL_BOOT_MODE = 1;
    static constexpr int PROTOCOL_UNSUPPORTED_MODE = 255;

    /* Report types (REPORT_TYPE_*) — HID report-kind bytes. */
    static constexpr uint8_t REPORT_TYPE_INPUT = 1;
    static constexpr uint8_t REPORT_TYPE_OUTPUT = 2;
    static constexpr uint8_t REPORT_TYPE_FEATURE = 3;

    /* Virtual unplug statuses (VIRTUAL_UNPLUG_STATUS_*). */
    static constexpr int VIRTUAL_UNPLUG_STATUS_SUCCESS = 0;
    static constexpr int VIRTUAL_UNPLUG_STATUS_FAIL = 1;

    explicit BluetoothHidHost(BluetoothAdapter& adapter);

    std::vector<BluetoothDevice> getConnectedDevices() override;
    std::vector<BluetoothDevice> getDevicesMatchingConnectionStates(
            const std::vector<int>& states);
    int getConnectionState(const BluetoothDevice& device) override;
    bool connect(const BluetoothDevice& device);
    bool disconnect(const BluetoothDevice& device);
    /* Deprecated priority pair — pure delegations over the connection
     * policy methods, exactly like the Java source. */
    bool setPriority(const BluetoothDevice& device, int priority);
    int getPriority(const BluetoothDevice& device);
    bool setConnectionPolicy(const BluetoothDevice& device, int connectionPolicy);
    int getConnectionPolicy(const BluetoothDevice& device);

    /* --- HID control channel (no BlueZ surface; TODO(porting)) -------- */
    bool virtualUnplug(const BluetoothDevice& device);
    bool getProtocolMode(const BluetoothDevice& device);
    bool setProtocolMode(const BluetoothDevice& device, int protocolMode);
    bool getReport(const BluetoothDevice& device, uint8_t reportType,
                   uint8_t reportId, int bufferSize);
    bool setReport(const BluetoothDevice& device, uint8_t reportType,
                   const std::string& report);
    bool sendData(const BluetoothDevice& device, const std::string& report);
    bool getIdleTime(const BluetoothDevice& device);
    bool setIdleTime(const BluetoothDevice& device, uint8_t idleTime);

private:
    BluetoothAdapter& mAdapter;
};

} // namespace cdroid

#endif /* __CDROID_BLUETOOTH_PROFILE_H__ */
