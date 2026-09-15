#ifndef __CONNECTIVITY_MANAGER_H__
#define __CONNECTIVITY_MANAGER_H__

#include <mutex>
#include <string>
#include <vector>

#include <ethernet/ethernetmanager.h>
#include <networkcapabilities.h>
#include <networkinfo.h>
#include <wifi/supplicantstate.h>
#include <wifi/wifimanager.h>

namespace cdroid {

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
class ConnectivityManager : private WifiManager::NetworkStateListener,
                            private EthernetManager::Listener,
                            private WifiManager::WifiApStateListener {
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

    /* Interim stand-in for the CONNECTIVITY_ACTION broadcast. */
    class NetworkStateListener {
    public:
        virtual ~NetworkStateListener() = default;
        virtual void onNetworkStateChanged(const NetworkInfo& networkInfo) = 0;
    };

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

    void addNetworkStateListener(NetworkStateListener* listener);
    void removeNetworkStateListener(NetworkStateListener* listener);

    /*
     * Tethered hotspot (TETHERING_WIFI): starts the Soft AP with the stored
     * SoftApConfiguration (setSoftApConfiguration), then brings up NAT
     * toward the default-route interface (netd enableNat semantics). The
     * AOSP binder path is async with callbacks — this port is synchronous,
     * like the WifiManager Soft AP entries. USB/Bluetooth tethering need
     * their own interface owners and stay TODO (faithful-stub rule).
     */
    bool startTethering(int type);
    bool stopTethering(int type);

private:
    ConnectivityManager();
    ~ConnectivityManager() override;
    ConnectivityManager(const ConnectivityManager&) = delete;
    ConnectivityManager& operator=(const ConnectivityManager&) = delete;

    /* WifiManager::NetworkStateListener / EthernetManager::Listener —
     * delivered on the supplicant monitor / ethernet poll threads. */
    void onNetworkStateChanged(const WifiInfo& info) override;
    void onAvailabilityChanged(const std::string& iface, bool isAvailable) override;
    /* WifiManager::WifiApStateListener — delivered on whichever thread
     * brings the AP down (stopSoftAp main call, hostapd monitor, the
     * Soft-Ap idle-shutdown worker). */
    void onWifiApStateChanged(int wifiApState) override;

    NetworkInfo buildWifiNetworkInfo();
    NetworkInfo buildEthernetNetworkInfo();
    void dispatch(const NetworkInfo& info);
    /* Default-route interface, else first link-up ethernet port. */
    std::string tetheringUpstreamIface();
    /* Removes the recorded NAT pair (if any) — the "enabled iface pair"
     * ledger netd keeps; called by stopTethering and by the AP-down
     * callback. */
    void teardownRecordedNat();

    std::mutex mListenersMutex;
    std::vector<NetworkStateListener*> mListeners;
    /* The (internal, external) pair enableNat actually programmed. stop
     * must remove what was installed, not whatever the default route
     * points at by stop time — it may have moved or vanished mid-session,
     * which stranded the MASQUERADE/FORWARD rules in the kernel while
     * ip_forward still got cleared for everyone else. */
    std::mutex mNatMutex;
    std::string mNatInternal;
    std::string mNatExternal;
};

} // namespace cdroid

#endif /* __CONNECTIVITY_MANAGER_H__ */
