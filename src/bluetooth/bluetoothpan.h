/*********************************************************************************
 * Port of android.bluetooth.BluetoothPan (android-12/36).
 *
 * Backed by BlueZ over D-Bus:
 *   - setBluetoothTethering(true) -> Adapter NetworkServer1.Register("nap",
 *     "bt-pan"): bluetoothd accepts BNEP connections and enslaves each peer
 *     bnepX to the named bridge. The bridge (address, DHCP server, NAT) is
 *     the tethering layer's job — AOSP splits the same way (PanService vs
 *     Tethering/netd); cdblue only owns the BlueZ half, so the demo/bench
 *     provisions the data plane and calls in here.
 *   - connect(device) -> Device Network1.Connect("panu") as the PANU client
 *     (this machine uses the remote's network); returns the bnep interface.
 *
 * BlueZ exposes no NetworkServer1 registered-state query, so isTetheringOn
 * reflects the last successful call (AOSP's PanService tracks the same way).
 *********************************************************************************/
#ifndef __CDROID_BLUETOOTH_PAN_H__
#define __CDROID_BLUETOOTH_PAN_H__

#include <map>
#include <string>
#include <vector>

#include <bluetoothprofile.h>
#include <bluetoothdevice.h>

namespace cdroid {

class BluetoothAdapter;

class BluetoothPan : public BluetoothProfile {
public:
    /* Local role extras (ACTION_CONNECTION_STATE_CHANGED). */
    static constexpr int PAN_ROLE_NONE = 0;
    static constexpr int LOCAL_NAP_ROLE = 1;
    static constexpr int LOCAL_PANU_ROLE = 2;
    static constexpr int REMOTE_NAP_ROLE = 1;
    static constexpr int REMOTE_PANU_ROLE = 2;

    /* ACTION_TETHERING_STATE_CHANGED extra values. */
    static constexpr int TETHERING_STATE_OFF = 1;
    static constexpr int TETHERING_STATE_ON = 2;

    /* Result codes (AOSP BluetoothPan). */
    static constexpr int PAN_DISCONNECT_FAILED_NOT_CONNECTED = 1000;
    static constexpr int PAN_CONNECT_FAILED_ALREADY_CONNECTED = 1001;
    static constexpr int PAN_CONNECT_FAILED_ATTEMPT_FAILED = 1002;
    static constexpr int PAN_OPERATION_GENERIC_FAILURE = 1003;
    static constexpr int PAN_OPERATION_SUCCESS = 1004;

    static constexpr const char* ACTION_CONNECTION_STATE_CHANGED =
            "android.bluetooth.pan.action.CONNECTION_STATE_CHANGED";
    static constexpr const char* ACTION_TETHERING_STATE_CHANGED =
            "android.bluetooth.pan.action.TETHERING_STATE_CHANGED";
    static constexpr const char* EXTRA_LOCAL_ROLE =
            "android.bluetooth.pan.extra.LOCAL_ROLE";
    static constexpr const char* EXTRA_TETHERING_STATE =
            "android.bluetooth.pan.extra.TETHERING_STATE";

    /* The bridge bluetoothd enslaves peer bnepX into (AOSP netd's name). */
    static constexpr const char* TETHERING_BRIDGE = "bt-pan";

    BluetoothPan(BluetoothAdapter& adapter);

    /* --- NAP (this device shares its network) --------------------------- */

    /* Register/unregister the local NAP server on TETHERING_BRIDGE. The
     * bridge must already exist (see the header comment); the caller runs
     * the DHCP/NAT data plane on it. */
    bool setBluetoothTethering(bool enabled);
    bool isTetheringOn() const;
    /* Interfaces of the currently tethered peers (bnepX present in the
     * system; BlueZ names its BNEP interfaces that way). */
    std::vector<std::string> getTetheredIfaces() const;

    /* --- PANU (this device uses the remote's network) -------------------- */

    /* Network1.Connect("panu"): returns the local bnep interface name via
     * ifaceOut (run a DHCP client on it afterwards). */
    bool connect(const BluetoothDevice& device, std::string& ifaceOut);
    bool disconnect(const BluetoothDevice& device);

    /* --- BluetoothProfile -------------------------------------------------- */

    int getConnectionState(const BluetoothDevice& device) override;
    std::vector<BluetoothDevice> getConnectedDevices() override;

private:
    BluetoothAdapter& mAdapter;
    /* address -> bnep interface of our PANU sessions. */
    std::map<std::string, std::string> mPanuIfaces;
};

} // namespace cdroid

#endif /* __CDROID_BLUETOOTH_PAN_H__ */
