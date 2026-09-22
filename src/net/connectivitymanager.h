#ifndef __CONNECTIVITY_MANAGER_H__
#define __CONNECTIVITY_MANAGER_H__

#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <core/callbackbase.h>   /* EventSet listener base (header-only) */

#include <ethernet/ethernetmanager.h>
#include <networkcapabilities.h>
#include <networkinfo.h>
#include <wifi/supplicantstate.h>
#include <wifi/wifimanager.h>

namespace cdroid {

class DhcpServer;

/**
 * Port of android.net.ConnectivityManager (android-36), the aggregation
 * layer over the per-network managers. AOSP queries ConnectivityService
 * (network scores, netd, validation); in CDROID's single-process world the
 * answers are derived live from WifiManager (supplicant state) and
 * EthernetManager (link + address state), with a simple
 * ETHERNET > WIFI preference for the active network (AOSP uses per-network
 * scores — simplified, documented divergence).
 *
 * NetworkCallback / registerNetworkCallback / requestNetwork need the
 * full NetworkAgent machinery and stay TODO (faithful-stub rule); the
 * interim NetworkStateListener mirrors CONNECTIVITY_ACTION delivery.
 */
class ConnectivityManager {
public:
    static constexpr int TYPE_NONE          = -1;
    static constexpr int TYPE_MOBILE        = 0;
    static constexpr int TYPE_WIFI          = 1;
    static constexpr int TYPE_MOBILE_MMS    = 2;
    static constexpr int TYPE_MOBILE_SUPL   = 3;
    static constexpr int TYPE_MOBILE_DUN    = 4;
    static constexpr int TYPE_MOBILE_HIPRI  = 5;
    static constexpr int TYPE_BLUETOOTH     = 7;
    static constexpr int TYPE_ETHERNET      = 9;
    static constexpr int TYPE_PROXY         = 16;
    static constexpr int TYPE_VPN           = 17;

    /* Broadcast intents, kept as constants for the future broadcast system. */
    static constexpr const char* CONNECTIVITY_ACTION =
            "android.net.conn.CONNECTIVITY_CHANGE";
    static constexpr const char* EXTRA_NETWORK_INFO = "networkInfo";
    static constexpr const char* EXTRA_NO_CONNECTIVITY = "noConnectivity";

    /* Interim stand-in for the CONNECTIVITY_ACTION broadcast —
     * single-callback surface, so a comparable CallbackBase typedef. */
    using NetworkStateListener = CallbackBase<void,const NetworkInfo&>;

    /* Tethering types (TetheringManager#TETHERING_*); ConnectivityManager
     * carries the start/stop entry points that AOSP routes through
     * TetheringManager.startTethering(TetheringRequest). */
    static constexpr int TETHERING_WIFI      = 0;
    static constexpr int TETHERING_USB       = 1;
    static constexpr int TETHERING_BLUETOOTH = 2;
    static constexpr int TETHERING_WIFI_P2P  = 3;
    static constexpr int TETHERING_NCM       = 4;
    static constexpr int TETHERING_ETHERNET  = 5;

    static ConnectivityManager& getInstance();

    /* The connected network with the highest preference, or an offline
     * NetworkInfo of TYPE_NONE when nothing is connected (AOSP null). */
    NetworkInfo getActiveNetworkInfo();
    NetworkInfo getNetworkInfo(int networkType);
    std::vector<NetworkInfo> getAllNetworkInfo();
    /* No metered policy exists in the module phase: never metered. */
    bool isActiveNetworkMetered() { return false; }

    void addNetworkStateListener(const NetworkStateListener& listener);
    void removeNetworkStateListener(const NetworkStateListener& listener);

    /* Tethering bridge bluetoothd enslaves peer bnepX into — the netd name,
     * mirrored from cdblue's BluetoothPan::TETHERING_BRIDGE (cdnet must not
     * link cdblue; AOSP splits the same way over binder). */
    static constexpr const char* BT_TETHERING_BRIDGE = "bt-pan";

    /*
     * Tethered hotspot (TETHERING_WIFI): starts the Soft AP with the stored
     * SoftApConfiguration (setSoftApConfiguration), then brings up NAT
     * toward the default-route interface (netd enableNat semantics). The
     * AOSP binder path is async with callbacks — this port is synchronous,
     * like the WifiManager Soft AP entries.
     *
     * TETHERING_BLUETOOTH: provisions the data plane on BT_TETHERING_BRIDGE
     * exactly as AOSP's Tethering/netd half (create bridge, address it from
     * config_tether_bluetooth_ranges 192.168.44.0/24, DHCP server, NAT
     * toward the upstream), then enables the BNEP server through the
     * enabler registered below (cdblue's BluetoothPan::setBluetoothTethering
     * — the PanService half). Without an enabler the data plane still comes
     * up (bench parity) with a warning. USB/WIFI_P2P tethering need their
     * own interface owners and stay TODO (faithful-stub rule).
     */
    bool startTethering(int type);
    bool stopTethering(int type);

    /* The Tethering<->PanService binder seam, in-process: the app layer
     * (which links both cdnet and cdblue) registers the bridge between
     * startTethering(TETHERING_BLUETOOTH) and
     * BluetoothPan::setBluetoothTethering. nullptr unregisters. */
    static void setBluetoothPanEnabler(const std::function<bool(bool enabled)>& enabler);

private:
    ConnectivityManager();
    ~ConnectivityManager();
    ConnectivityManager(const ConnectivityManager&) = delete;
    ConnectivityManager& operator=(const ConnectivityManager&) = delete;

    /* Upstream listener slots (value semantics — the members hold the
     * lambdas, registered as copies). WifiManager::NetworkStateListener
     * / EthernetManager::Listener arrive on the supplicant monitor /
     * ethernet poll threads; WifiApStateListener on whichever thread
     * brings the AP down (stopSoftAp main call, hostapd monitor, the
     * Soft-Ap idle-shutdown worker). */
    void onNetworkStateChanged(const WifiInfo& info);
    void onAvailabilityChanged(const std::string& iface, bool isAvailable);
    void onWifiApStateChanged(int wifiApState);
    WifiManager::NetworkStateListener mWifiNetworkListener;
    EthernetManager::Listener mEthernetListener;
    WifiManager::WifiApStateListener mWifiApListener;

    NetworkInfo buildWifiNetworkInfo();
    NetworkInfo buildEthernetNetworkInfo();
    void dispatch(const NetworkInfo& info);
    /* Default-route interface, else first link-up ethernet port. */
    std::string tetheringUpstreamIface();
    /* Removes the recorded NAT pair of one tethering type (if any) — the
     * "enabled iface pair" ledger netd keeps; called by stopTethering and
     * (for WIFI) by the AP-down callback. */
    void teardownRecordedNat(int type);

    std::mutex mListenersMutex;
    std::vector<NetworkStateListener> mListeners;
    /* The (internal, external) pair enableNat actually programmed, per
     * tethering type. stop must remove what was installed, not whatever
     * the default route points at by stop time — it may have moved or
     * vanished mid-session, which stranded the MASQUERADE/FORWARD rules in
     * the kernel while ip_forward still got cleared for everyone else. */
    std::mutex mNatMutex;
    std::map<int, std::pair<std::string, std::string>> mNatPairs;
    /* TETHERING_BLUETOOTH DHCP server on BT_TETHERING_BRIDGE; started/
     * stopped on the start/stopTethering path only (single-threaded, like
     * WifiManager's mApDhcpServer). */
    DhcpServer* mBtDhcpServer = nullptr;
    static std::function<bool(bool)> sBluetoothPanEnabler;
};

} // namespace cdroid

#endif /* __CONNECTIVITY_MANAGER_H__ */
