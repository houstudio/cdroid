#ifndef __WIFI_MANAGER_H__
#define __WIFI_MANAGER_H__

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <networkeventmonitor.h>
#include <wifi/wifiradio.h>
#include <dhcpclient.h>
#include <wifi/supplicantclient.h>
#include <wifi/scanresult.h>
#include <wifi/wificonfiguration.h>
#include <wifi/wifiinfo.h>

namespace cdroid {

/**
 * Port of android.net.wifi.WifiManager (android-36), backed directly by
 * wpa_supplicant's control interface (SupplicantClient) instead of the
 * WifiService binder. Module phase: a process singleton (service registry
 * integration — Context#getSystemService(WIFI_SERVICE) — lands when this
 * library graduates into src/gui/net/wifi).
 *
 * Synchronous methods issue ctrl_iface requests on the caller thread;
 * events arrive on the SupplicantClient monitor thread unless a dispatcher
 * was installed (see SupplicantClient::setDispatcher).
 *
 * Radio enable/disable (rfkill / interface control) needs a platform hook
 * that does not exist yet: setWifiEnabled is supplicant-client scoped, see
 * the TODO inside.
 */
class WifiManager : private SupplicantClient::EventCallback,
                    private NetworkEventMonitor::Events {
public:
    /* Broadcast intents, kept as constants for the future broadcast system. */
    static constexpr const char* WIFI_STATE_CHANGED_ACTION =
            "android.net.wifi.WIFI_STATE_CHANGED";
    static constexpr const char* EXTRA_WIFI_STATE = "wifi_state";
    static constexpr const char* EXTRA_PREVIOUS_WIFI_STATE = "previous_wifi_state";
    static constexpr const char* SCAN_RESULTS_AVAILABLE_ACTION =
            "android.net.wifi.SCAN_RESULTS";
    static constexpr const char* NETWORK_STATE_CHANGED_ACTION =
            "android.net.wifi.STATE_CHANGE";
    static constexpr const char* RSSI_CHANGED_ACTION =
            "android.net.wifi.RSSI_CHANGE";
    static constexpr const char* EXTRA_NEW_RSSI = "newRssi";

    /* Wi-Fi states (getWifiState/isWifiEnabled). */
    static constexpr int WIFI_STATE_DISABLING = 0;
    static constexpr int WIFI_STATE_DISABLED  = 1;
    static constexpr int WIFI_STATE_ENABLING  = 2;
    static constexpr int WIFI_STATE_ENABLED   = 3;
    static constexpr int WIFI_STATE_UNKNOWN   = 4;

    static constexpr const char* UNKNOWN_SSID = "<unknown ssid>";
    /* @deprecated passed with ActionListener#onFailure during connect. */
    static constexpr int ERROR_AUTHENTICATING = 1;

    /** Interface for answer-bearing async operations (connect/forget/save). */
    class ActionListener {
    public:
        static constexpr int FAILURE_INTERNAL_ERROR = 0;
        static constexpr int FAILURE_IN_PROGRESS    = 1;
        static constexpr int FAILURE_BUSY           = 2;
        static constexpr int FAILURE_INVALID_ARGS   = 3;
        static constexpr int FAILURE_NOT_AUTHORIZED = 4;
        virtual ~ActionListener() = default;
        /** The operation succeeded. */
        virtual void onSuccess() = 0;
        /** The operation failed. @param reason one of the FAILURE_* above. */
        virtual void onFailure(int reason) = 0;
    };

    /*
     * Interim listener surfaces — one per broadcast action — until the
     * broadcast/receiver system exists (per-module-phase decision).
     */
    class WifiStateListener {
    public:
        virtual ~WifiStateListener() = default;
        virtual void onWifiStateChanged(int wifiState) = 0;
    };
    class ScanResultsListener {
    public:
        virtual ~ScanResultsListener() = default;
        virtual void onScanResultsAvailable() = 0;
    };
    class NetworkStateListener {
    public:
        virtual ~NetworkStateListener() = default;
        virtual void onNetworkStateChanged(const WifiInfo& info) = 0;
    };
    class RssiListener {
    public:
        virtual ~RssiListener() = default;
        virtual void onRssiChanged(int newRssi) = 0;
    };

    /* Process singleton (Context#getSystemService lands at graduation). */
    static WifiManager& getInstance();

    /* Bind the transport to a supplicant control socket and start the
     * event pump. Idempotent; returns false when the daemon is unreachable
     * (monitor keeps retrying in the background). */
    bool initialize(const std::string& ctrlPath = SupplicantClient::defaultCtrlPath());
    /* Transport seam: the cdroid integration installs a main-looper
     * dispatcher here before any listener is registered. */
    SupplicantClient& getSupplicantClient() { return mClient; }

    /* --- Wi-Fi state ------------------------------------------------------ */
    int getWifiState();
    bool isWifiEnabled();
    bool setWifiEnabled(bool enabled);

    /* --- scan ------------------------------------------------------------- */
    bool startScan();
    std::vector<ScanResult> getScanResults();

    /* --- connection info --------------------------------------------------- */
    WifiInfo getConnectionInfo();
    /* @deprecated last lease obtained by the built-in DHCP client (zeroed
     * when none). AOSP surface; filled since the wifi IP provisioning
     * path (the IpClient counterpart) landed. */
    DhcpInfo getDhcpInfo();

    /* --- configured networks ------------------------------------------------ */
    std::vector<WifiConfiguration> getConfiguredNetworks();
    /* @deprecated returns the new network id, -1 on failure. */
    int addNetwork(const WifiConfiguration& config);
    /* @deprecated returns true on success; config.networkId selects the
     * network to update. */
    bool updateNetwork(const WifiConfiguration& config);
    /* @deprecated */
    bool removeNetwork(int netId);
    /* @deprecated */
    bool enableNetwork(int netId, bool attemptConnect);
    /* @deprecated */
    bool disableNetwork(int netId);
    /* @deprecated writes the wpa_supplicant config (SAVE_CONFIG — requires
     * update_config=1 on the daemon side). */
    bool saveConfiguration();

    /* --- async operations ---------------------------------------------------- */
    /* Connect to a configured network. */
    void connect(int networkId, ActionListener* listener);
    /* Add or update config, then connect to it. */
    void connect(const WifiConfiguration& config, ActionListener* listener);
    /* Remove and forget networkId. */
    void forget(int networkId, ActionListener* listener);
    /* Disable networkId. */
    void disable(int networkId, ActionListener* listener);
    /* @deprecated persist the running configuration. */
    void save(ActionListener* listener);

    bool disconnect();
    bool reconnect();
    bool reassociate();
    bool pingSupplicant();

    /* --- signal levels ------------------------------------------------------- */
    /* numLevels bins between MIN_RSSI=-100dBm and MAX_RSSI=-55dBm. */
    static int calculateSignalLevel(int rssi, int numLevels);
    int getMaxSignalLevel();
    static int compareSignalLevel(int rssiA, int rssiB);

    /* --- listeners (not owned; add/remove pairs, thread-safe) ---------------- */
    void addWifiStateListener(WifiStateListener* listener);
    void removeWifiStateListener(WifiStateListener* listener);
    void addScanResultsListener(ScanResultsListener* listener);
    void removeScanResultsListener(ScanResultsListener* listener);
    void addNetworkStateListener(NetworkStateListener* listener);
    void removeNetworkStateListener(NetworkStateListener* listener);
    void addRssiListener(RssiListener* listener);
    void removeRssiListener(RssiListener* listener);

private:
    WifiManager();
    ~WifiManager() override;
    WifiManager(const WifiManager&) = delete;
    WifiManager& operator=(const WifiManager&) = delete;

    /* SupplicantClient::EventCallback — monitor thread (or dispatcher). */
    void onSupplicantEvent(const SupplicantEvent& event) override;
    void onSupplicantDisconnected() override;
    void onSupplicantReconnected() override;

    /* NetworkEventMonitor::Events — IP acquisition has no supplicant event
     * (AOSP: IpClient callback); the platform address push plays that role.
     * Link flips (rfkill/netdev down) matter too: both re-dispatch
     * NetworkState with the refreshed WifiInfo. */
    void onLinkStateChanged(const std::string& iface, bool up, bool lowerUp) override;
    void onAddressChanged(const std::string& iface, bool added,
                          const std::string& address) override;
    void refreshAndDispatchNetworkState();

    bool requestOk(const std::string& cmd);
    std::string interfaceName() const;
    int addOrUpdateNetwork(const WifiConfiguration& config);
    void setWifiStateAndNotify(int newState);
    /* Copy-then-dispatch tails shared by the event branches: snapshot the
     * state under its lock, snapshot the listeners, then call out. */
    void notifyWifiStateListeners(int state);
    void notifyScanResultsListeners();
    void notifyNetworkStateListeners();
    void notifyRssiListeners(int rssi);
    /* One STATUS probe mapped through the shared parser; seeds mWifiState
     * at initialize() (afterwards the event stream keeps it current). */
    void refreshWifiStateFromSupplicant();
    void updateConnectionInfoFromStatus();
    void startRssiPolling();
    void stopRssiPolling();
    void rssiPollLoop();

    /* wifi IP provisioning (AOSP IpClient counterpart): on supplicant
     * COMPLETED acquire a lease, apply it (shared ipapplicator), renew at
     * T1; release on disconnect. */
    void startDhcpIfNeeded();
    void stopDhcpAndRelease();
    /* start DHCP when the live state is already COMPLETED (the connect
     * event predates the process — auto-reconnected supplicant) */
    void seedDhcpFromCurrentState();
    struct WifiDhcpSession {
        std::thread thread;
        std::atomic<bool> stop{false};
    };
    WifiDhcpSession* mDhcpSession = nullptr;
    DhcpClient::Lease mLease;

    SupplicantClient mClient;
    NetworkEventMonitor* mAddressMonitor = nullptr;
    WifiRadioData* mRadioData = nullptr;
    /* Written once by initialize() (INTERFACES-resolved), read by the
     * netlink monitor thread, the DHCP session thread and app threads —
     * every access goes through mStateMutex / interfaceName(). */
    std::string mIfaceName;
    mutable std::mutex mStateMutex;
    int mWifiState = WIFI_STATE_UNKNOWN;
    WifiInfo mConnectionInfo;
    std::atomic<int> mLastRssi { WifiInfo::INVALID_RSSI };
    /* Getter caches (mStateMutex): the UI polls the getters per refresh,
     * so they must not fan out synchronous RPCs — the event stream is the
     * writer, SCAN-RESULTS / network mutations are the invalidations. */
    std::vector<ScanResult> mScanResultsCache;
    std::vector<WifiConfiguration> mConfiguredNetworksCache;
    bool mConfigsDirty = true;

    std::mutex mListenersMutex;
    std::vector<WifiStateListener*> mWifiStateListeners;
    std::vector<ScanResultsListener*> mScanResultsListeners;
    std::vector<NetworkStateListener*> mNetworkStateListeners;
    std::vector<RssiListener*> mRssiListeners;

    std::thread mRssiPollThread;
    std::atomic<bool> mRssiPolling { false };
};

} // namespace cdroid

#endif /* __WIFI_MANAGER_H__ */
