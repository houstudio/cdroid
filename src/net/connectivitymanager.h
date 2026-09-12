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
                            private EthernetManager::Listener {
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

private:
    ConnectivityManager();
    ~ConnectivityManager() override;
    ConnectivityManager(const ConnectivityManager&) = delete;
    ConnectivityManager& operator=(const ConnectivityManager&) = delete;

    /* WifiManager::NetworkStateListener / EthernetManager::Listener —
     * delivered on the supplicant monitor / ethernet poll threads. */
    void onNetworkStateChanged(const WifiInfo& info) override;
    void onAvailabilityChanged(const std::string& iface, bool isAvailable) override;

    NetworkInfo buildWifiNetworkInfo();
    NetworkInfo buildEthernetNetworkInfo();
    void dispatch(const NetworkInfo& info);

    std::mutex mListenersMutex;
    std::vector<NetworkStateListener*> mListeners;
};

} // namespace cdroid

#endif /* __CONNECTIVITY_MANAGER_H__ */
