#ifndef __WIFI_MANAGER_H__
#define __WIFI_MANAGER_H__

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <core/callbackbase.h>   /* EventSet listener base (header-only) */

#include <networkeventmonitor.h>
#include <wifi/wifiradio.h>
#include <dhcpclient.h>
#include <wifi/supplicantclient.h>
#include <wifi/hostapdclient.h>
#include <wifi/scanresult.h>
#include <wifi/softapcapability.h>
#include <wifi/softapconfiguration.h>
#include <wifi/wificlient.h>
#include <wifi/wificonfiguration.h>
#include <wifi/wifiinfo.h>

namespace cdroid {

class DhcpServer;

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
class WifiManager : private NetworkEventMonitor::Events {
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
    /* Interim surface for WIFI_AP_STATE_CHANGED_ACTION (broadcast system is
     * future work, same decision as the STA listeners above). */
    class WifiApStateListener : public EventSet {
    public:
        virtual ~WifiApStateListener() = default;
        virtual void onWifiApStateChanged(int wifiApState) = 0;
    };
    /**
     * Port of WifiManager.SoftApCallback (@SystemApi, android-36). The
     * Executor marshaling folds into the shared dispatcher seam: callbacks
     * arrive on the hostapd monitor thread unless a dispatcher is installed.
     */
    class SoftApCallback : public EventSet {
    public:
        /* SAP_STATE_* (SoftApCallback): numeric mirrors of the
         * WIFI_AP_STATE_* constants declared below (a nested class cannot
         * reference them before their declaration). */
        static constexpr int SAP_STATE_DISABLING = 10;
        static constexpr int SAP_STATE_ENABLED   = 13;
        static constexpr int SAP_STATE_FAILED    = 14;
        /* SAP_CLIENT_BLOCK_REASON_CODE_* (also surfaced through
         * onBlockedClientConnecting). */
        static constexpr int SAP_CLIENT_BLOCK_REASON_CODE_BLOCKED_BY_USER = 0;

        virtual ~SoftApCallback() = default;
        /** AP state change; failureCode is a SAP_START_FAILURE_* when state
         * is SAP_STATE_FAILED, 0 otherwise. */
        virtual void onStateChanged(int state, int failureCode) = 0;
        /** Connected-client list changed (full list, like the binder array). */
        virtual void onConnectedClientsChanged(
                const std::vector<WifiClient>& clients, int reasonCode) {}
        /** Operating info changed (frequency/bandwidth of the AP). */
        virtual void onInfoChanged(const std::vector<SoftApInfo>& infoList) {}
        /** Backend capabilities (static for the hostapd backend). */
        virtual void onCapabilityChanged(const SoftApCapability& capability) {}
        /** A blocked client attempted to associate. */
        virtual void onBlockedClientConnecting(
                const WifiClient& client, int blockedReason) {}
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

    /* --- Soft AP (hotspot) ----------------------------------------------------- */

    /* Broadcast intent constants (kept for the future broadcast system). */
    static constexpr const char* WIFI_AP_STATE_CHANGED_ACTION =
            "android.net.wifi.WIFI_AP_STATE_CHANGED";
    static constexpr const char* EXTRA_WIFI_AP_STATE = "wifi_state";
    static constexpr const char* EXTRA_PREVIOUS_WIFI_AP_STATE = "previous_wifi_state";
    static constexpr const char* EXTRA_WIFI_AP_FAILURE_REASON =
            "android.net.wifi.extra.WIFI_AP_FAILURE_REASON";
    static constexpr const char* EXTRA_WIFI_AP_INTERFACE_NAME =
            "android.net.wifi.extra.WIFI_AP_INTERFACE_NAME";
    static constexpr const char* EXTRA_WIFI_AP_MODE =
            "android.net.wifi.extra.WIFI_AP_MODE";

    /* Wi-Fi AP states (getWifiApState). */
    static constexpr int WIFI_AP_STATE_DISABLING = 10;
    static constexpr int WIFI_AP_STATE_DISABLED  = 11;
    static constexpr int WIFI_AP_STATE_ENABLING  = 12;
    static constexpr int WIFI_AP_STATE_ENABLED   = 13;
    static constexpr int WIFI_AP_STATE_FAILED    = 14;

    /* AP start failure reasons (EXTRA_WIFI_AP_FAILURE_REASON). */
    static constexpr int SAP_START_FAILURE_GENERAL = 0;
    static constexpr int SAP_START_FAILURE_NO_CHANNEL = 1;
    static constexpr int SAP_START_FAILURE_UNSUPPORTED_CONFIGURATION = 2;
    static constexpr int SAP_START_FAILURE_USER_REJECTED = 3;

    /* Reason codes reported when a client is blocked from the Soft AP. */
    static constexpr int SAP_CLIENT_BLOCK_REASON_CODE_BLOCKED_BY_USER = 0;

    /* Intended IP mode of a Soft AP interface (EXTRA_WIFI_AP_MODE). */
    static constexpr int IFACE_IP_MODE_UNSPECIFIED = -1;
    static constexpr int IFACE_IP_MODE_CONFIGURATION_ERROR = 0;
    static constexpr int IFACE_IP_MODE_TETHERED = 1;
    static constexpr int IFACE_IP_MODE_LOCAL_ONLY = 2;

    static const MacAddress ALL_ZEROS_MAC_ADDRESS;

    int getWifiApState();
    bool isWifiApEnabled();
    /* Stored configuration, or the framework default (random SSID +
     * WPA2_PSK passphrase, WifiApConfigStore shape) when none was set.
     * Persistence across restarts is TODO(porting). */
    SoftApConfiguration getSoftApConfiguration();
    bool setSoftApConfiguration(const SoftApConfiguration& config);
    /* True when the config can be rendered by the hostapd backend (the
     * module counterpart of WifiService#validateSoftApConfiguration). */
    bool validateSoftApConfiguration(const SoftApConfiguration& config);
    /**
     * Start the tethered hotspot. @param softApConfig nullptr uses the
     * persisted configuration (Java @Nullable). Unlike AOSP's async binder
     * path this is synchronous: ENABLING/ENABLED listener dispatches happen
     * during the call. Starting may disable station operation — not
     * enforced here (bench runs STA+AP on separate radios), callers on
     * single-radio targets should stop wifi first.
     * @deprecated in android-36 in favor of the TetheringManager
     * TetheringRequest overload (TetheringManager is not ported yet).
     */
    bool startTetheredHotspot(const SoftApConfiguration* softApConfig);
    bool stopSoftAp();
    /* @deprecated legacy entry: converts and delegates to
     * startTetheredHotspot (Java @Nullable param → pointer). */
    bool startSoftAp(const WifiConfiguration* wifiConfig);

    /* cdroid seam: which interface the Soft AP runs on. AOSP resolves the
     * AP interface through the wifi HAL; here it defaults to the STA
     * interface (single-radio products) and can be pointed at a second
     * radio (e.g. wlan1 on the hwsim bench). */
    void configureSoftApInterface(const std::string& iface);
    /* The resolved AP interface name (tethering NAT setup asks for it —
     * AOSP gets it from ConnectivityService's interface snapshot). */
    std::string getSoftApInterfaceName() { return softApInterface(); }
    /* Transport seam, mirroring getSupplicantClient(). */
    HostapdClient& getHostapdClient() { return mHostapd; }
    /* RegisterSoftApCallback: registration immediately delivers the current
     * state, info and capabilities (the binder AOSP path does the same).
     * Not owned; pair with unregisterSoftApCallback. */
    void registerSoftApCallback(SoftApCallback* callback);
    void unregisterSoftApCallback(SoftApCallback* callback);

    /* --- listeners (not owned; add/remove pairs, thread-safe) ---------------- */
    void addWifiStateListener(WifiStateListener* listener);
    void removeWifiStateListener(WifiStateListener* listener);
    void addScanResultsListener(ScanResultsListener* listener);
    void removeScanResultsListener(ScanResultsListener* listener);
    void addNetworkStateListener(NetworkStateListener* listener);
    void removeNetworkStateListener(NetworkStateListener* listener);
    void addRssiListener(RssiListener* listener);
    void removeRssiListener(RssiListener* listener);
    void addWifiApStateListener(WifiApStateListener* listener);
    void removeWifiApStateListener(WifiApStateListener* listener);

private:
    WifiManager();
    ~WifiManager() override;
    WifiManager(const WifiManager&) = delete;
    WifiManager& operator=(const WifiManager&) = delete;

    /* SupplicantClient event slot — monitor thread (or dispatcher). */
    void onSupplicantEvent(const SupplicantEvent& event);
    void onSupplicantDisconnected();
    void onSupplicantReconnected();

    /* HostapdClient event slot — monitor thread (or dispatcher). */
    void onHostapdEvent(const HostapdClient::HostapdEvent& event);
    void onHostapdDisconnected();
    void onHostapdReconnected();

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
    /* --- Soft AP internals ------------------------------------------------- */
    /* AP iface resolution: in-process choice, else the persisted store
     * entry, else the STA interface. */
    std::string softApInterface();
    /* One-shot load of SoftApConfigStore (mStateMutex held). */
    void ensureSoftApStoreLoaded();
    void setWifiApStateAndNotify(int newState);
    void notifyWifiApStateListeners(int state);
    /* WifiApConfigStore-style default: "AndroidAP_" + 4 random digits,
     * random WPA2 passphrase (stored on first use). */
    SoftApConfiguration defaultSoftApConfiguration();
    /* Persist config (and the current AP interface choice) to the store. */
    void persistSoftApConfiguration(const SoftApConfiguration& config);
    /* Connected-client bookkeeping: seed from hostapd LIST_CLIENTS, track
     * AP-STA-* events, notify SoftApCallbacks. */
    void refreshSoftApClientsFromHostapd();
    void notifySoftApClientsChanged(int reasonCode);
    /* Blocked-list enforcement: hostapd DISASSOCIATE + the
     * onBlockedClientConnecting callback (AOSP SoftApManager). */
    void enforceBlockedClient(const MacAddress& mac);
    /* SoftApConfiguration with a randomized BSSID when the setting asks
     * for it (RANDOMIZATION_NON_PERSISTENT default). */
    SoftApConfiguration withRandomizedBssid(const SoftApConfiguration& config);
    /* Auto-shutdown timer (config.isAutoShutdownEnabled + timeout): a
     * client-less AP tears itself down after the idle window. */
    void scheduleSoftApIdleShutdown(int64_t timeoutMillis);
    void cancelSoftApIdleShutdown();
    /* Timer-thread teardown: everything stopSoftAp does EXCEPT joining the
     * hostapd monitor (the monitor may be joining this timer — ABBA). The
     * transport socket dies with the daemon; the dtor closes it later. */
    void softApIdleShutdown();
    /* Fill mSoftApInfo from the hostapd STATUS reply (freq= line). */
    void refreshSoftApInfoFromHostapd();
    void notifySoftApCallbacksInfo();
    void notifySoftApCallbacksCapability();
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
    HostapdClient mHostapd;
    NetworkEventMonitor* mAddressMonitor = nullptr;
    WifiRadioData* mRadioData = nullptr;
    /* Written once by initialize() (INTERFACES-resolved), read by the
     * netlink monitor thread, the DHCP session thread and app threads —
     * every access goes through mStateMutex / interfaceName(). */
    std::string mIfaceName;
    mutable std::mutex mStateMutex;
    int mWifiState = WIFI_STATE_UNKNOWN;
    /* Client-side desired state (AOSP WifiSettingsStore semantics): a
     * user-initiated disable owns mWifiState — "supplicant reachable"
     * must not resurrect it. */
    bool mUserDisabled = false;
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
    std::vector<WifiApStateListener*> mWifiApListeners;
    std::vector<SoftApCallback*> mSoftApCallbacks;

    /* --- Soft AP state (mStateMutex) ----------------------------------------- */
    int mWifiApState = WIFI_AP_STATE_DISABLED;
    int mWifiApFailureReason = SAP_START_FAILURE_GENERAL;
    bool mSoftApConfigSet = false;   /* stored config vs framework default */
    bool mSoftApStoreLoaded = false; /* SoftApConfigStore read once */
    SoftApConfiguration mSoftApConfig;
    std::string mSoftApIface;        /* empty = persisted store / STA iface */
    std::vector<WifiClient> mSoftApClients;
    SoftApInfo mSoftApInfo;
    DhcpServer* mApDhcpServer = nullptr;   /* owned; AP-side DHCP (dnsmasq) */
    std::thread mSoftApIdleThread;
    std::atomic<bool> mSoftApIdleTimerActive { false };
    /* The configuration the RUNNING AP was started with (blocked-list
     * enforcement + idle-shutdown settings); mSoftApConfig is the stored
     * one and may diverge mid-run. */
    SoftApConfiguration mActiveSoftApConfig;
    bool mActiveSoftApConfigValid = false;
    bool mSoftApAutoShutdownEnabled = false;
    int64_t mSoftApShutdownTimeoutMillis = 0;

    std::thread mRssiPollThread;
    std::atomic<bool> mRssiPolling { false };
};

} // namespace cdroid

#endif /* __WIFI_MANAGER_H__ */
