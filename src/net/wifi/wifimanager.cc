/* Port of android.net.wifi.WifiManager (android-36), wpa_ctrl backed. */
#include <wifi/wifimanager.h>

#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>   // inet_ntop (wpa_control sockaddr dump)
#include <sys/stat.h>
#include <unistd.h>

#include <ipapplicator.h>
#include <dhcpserver.h>
#include <wifi/softapconfigstore.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <random>

#include <wifi/wparesponseparser.h>
#include <linkaddress.h>
#include <wpa_ctrl.h>   /* WPA_EVENT_* (canonical event spellings) */

/* wpa_ctrl.h also defines INTERFACE_ENABLED/INTERFACE_DISABLED *event*
 * macros that collide with the SupplicantState enumerators of the same
 * name; the macros are unused here (the vendored header stays untouched). */
#undef INTERFACE_ENABLED
#undef INTERFACE_DISABLED

namespace cdroid {

/* android-36 keeps these private inside WifiManager. */
static constexpr int MIN_RSSI = -100;
static constexpr int MAX_RSSI = -55;
/* Framework signal bins (WifiService RSSI_LEVELS). */
static constexpr int RSSI_LEVELS = 5;
static constexpr int POLL_RSSI_INTERVAL_MS = 3000;

/* GET_NETWORK variables mirrored into a WifiConfiguration. */
static const char* const kNetworkVariables[] = {
    "ssid", "bssid", "psk", "key_mgmt", "proto", "pairwise", "group",
    "auth_alg", "priority", "scan_ssid", "disabled",
};

/* The WPA_EVENT_* macros carry a trailing space (they prefix full wpa
 * messages); the parsed event name is the bare token — match by length. */
static bool eventIs(const SupplicantEvent& event, const char* wpaEvent) {
    const size_t len = strlen(wpaEvent) - 1;
    return event.name.size() == len && memcmp(event.name.data(), wpaEvent, len) == 0;
}

WifiManager& WifiManager::getInstance() {
    static WifiManager instance;
    return instance;
}

WifiManager::WifiManager() {
    /* Transport event slots (EventSet + lambdas — value semantics, no
     * listener objects to own). */
    SupplicantClient::EventCallback supplicantEvents;
    supplicantEvents.onSupplicantEvent =
            [this](const SupplicantEvent& event) { onSupplicantEvent(event); };
    supplicantEvents.onSupplicantDisconnected =
            [this]() { onSupplicantDisconnected(); };
    supplicantEvents.onSupplicantReconnected =
            [this]() { onSupplicantReconnected(); };
    mClient.setEventCallback(supplicantEvents);

    HostapdClient::EventCallback hostapdEvents;
    hostapdEvents.onHostapdEvent = [this](const HostapdClient::HostapdEvent& event) {
        onHostapdEvent(event);
    };
    hostapdEvents.onHostapdDisconnected = [this]() { onHostapdDisconnected(); };
    hostapdEvents.onHostapdReconnected = [this]() { onHostapdReconnected(); };
    mHostapd.setEventCallback(hostapdEvents);
    /* Address events drive the COMPLETED -> (has IP) CONNECTED promotion
     * the way AOSP's IpClient callback does. */
    mAddressMonitor = NetworkEventMonitor::create();
    if (mAddressMonitor && !mAddressMonitor->start(this)) {
        delete mAddressMonitor;
        mAddressMonitor = nullptr;
    }
    /* radio data plane (nl80211 dump): scan source of record, like the
     * AOSP wifi HAL; on-demand, no thread. */
    mRadioData = WifiRadioData::create();
}

WifiManager::~WifiManager() {
    /* The process owns the Soft AP it started (module phase: WifiManager IS
     * the WifiService) — teardown order: AP daemons (hostapd + dnsmasq +
     * address), then the STA side. Without this hostapd outlives the
     * process while dnsmasq dies with it — the 14:50 E2E orphan.
     * Join the idle worker FIRST: stopSoftAp() early-returns at DISABLED
     * (so an AP already down never reaches its cancel), and the unlocked
     * mApDhcpServer delete below raced the worker's locked teardown when
     * the idle timer happened to fire near shutdown. */
    cancelSoftApIdleShutdown();
    stopSoftAp();
    stopDhcpAndRelease();
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        delete mApDhcpServer;   /* stops dnsmasq + clears the AP address */
        mApDhcpServer = nullptr;
    }
    if (mAddressMonitor) {
        mAddressMonitor->stop();
        delete mAddressMonitor;
        mAddressMonitor = nullptr;
    }
    delete mRadioData;
    mRadioData = nullptr;
    stopRssiPolling();
    /* Retire the transport event slots before the members go — their
     * lambdas capture this, and the monitors are only joined inside
     * ~SupplicantClient / ~HostapdClient, i.e. after this body. */
    mClient.setEventCallback(SupplicantClient::EventCallback());
    mHostapd.setEventCallback(HostapdClient::EventCallback());
    mClient.close();
    mHostapd.close();
}

bool WifiManager::initialize(const std::string& ctrlPath) {
    if (mClient.isConnected()) {
        /* Re-bind path (fragment recreation): the cached state is only as
         * fresh as the last event — re-seed once so the caller's first
         * getWifiState()/isWifiEnabled() is not a stale UNKNOWN. */
        refreshWifiStateFromSupplicant();
        return true;
    }
    mClient.setCtrlPath(ctrlPath);
    const bool ok = mClient.connect();
    if (ok) {
        /* "/var/run/wpa_supplicant/wlan0" -> "wlan0" normally, but the
         * global socket ("/var/run/wpa_supplicant") would yield the daemon
         * directory name: ask INTERFACES what the daemon actually serves
         * and fall back to the path basename. */
        const size_t slash = ctrlPath.find_last_of('/');
        const std::string base = (slash == std::string::npos)
                ? ctrlPath : ctrlPath.substr(slash + 1);
        std::string iface = base;
        const std::string listed = mClient.request("INTERFACES");
        std::vector<std::string> names;
        size_t pos = 0;
        while (pos < listed.size()) {
            const size_t nl = listed.find('\n', pos);
            std::string name = listed.substr(pos, nl == std::string::npos
                    ? std::string::npos : nl - pos);
            while (!name.empty() && (name.back() == '\r' || name.back() == ' '))
                name.pop_back();
            if (!name.empty()) names.push_back(name);
            if (nl == std::string::npos) break;
            pos = nl + 1;
        }
        if (!names.empty()) {
            iface = names[0];
            for (const std::string& n : names)   /* a per-if socket keeps its own name */
                if (n == base) { iface = base; break; }
        }
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            mIfaceName = iface;
        }
        refreshWifiStateFromSupplicant();
        seedDhcpFromCurrentState();
    }
    return ok;
}

std::string WifiManager::interfaceName() const {
    std::lock_guard<std::mutex> lock(mStateMutex);
    return mIfaceName;
}

/* --- Wi-Fi state ----------------------------------------------------------- */

int WifiManager::getWifiState() {
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        if (mWifiState != WIFI_STATE_UNKNOWN) return mWifiState;
    }
    /* UNKNOWN means no writer has resolved the state yet — pages that
     * never call initialize() (the settings summary) ask all the same.
     * One synchronous STATUS resolves it, but only when the transport is
     * already connected: a getter must never trigger socket re-opens (a
     * down daemon would otherwise turn every UI refresh into a reconnect
     * storm). The event stream owns the value from the first resolve on. */
    if (mClient.isConnected()) refreshWifiStateFromSupplicant();
    std::lock_guard<std::mutex> lock(mStateMutex);
    return mWifiState;
}

void WifiManager::refreshWifiStateFromSupplicant() {
    /* A user-initiated disable owns the state (WifiSettingsStore
     * semantics): daemon-reachable must not resurrect it. */
    bool userDisabled;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        userDisabled = mUserDisabled;
    }
    if (userDisabled) {
        setWifiStateAndNotify(WIFI_STATE_DISABLED);
        return;
    }
    /* The ctrl iface has no radio state: map the supplicant's view —
     * through the shared STATUS parser (a raw substring probe would fire
     * on an SSID that happens to contain the state text). */
    const std::string status = mClient.request("STATUS");
    if (status.empty()) return;
    WifiInfo probe;
    WpaResponseParser::applyStatus(probe, status);
    setWifiStateAndNotify(probe.getSupplicantState() == SupplicantState::INTERFACE_DISABLED
            ? WIFI_STATE_DISABLED : WIFI_STATE_ENABLED);
}

bool WifiManager::isWifiEnabled() {
    return getWifiState() == WIFI_STATE_ENABLED;
}

bool WifiManager::setWifiEnabled(bool enabled) {
    /* TODO(porting): full enable/disable drives the interface and the
     * supplicant lifecycle (platform-specific); the module phase only owns
     * the supplicant client side: "enabled" means reaching the daemon,
     * "disabled" detaches from the current network. */
    if (enabled) {
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            mUserDisabled = false;
        }
        const bool ok = initialize();
        /* AOSP re-associates the saved networks on enable; a supplicant
         * told to DISCONNECT stays idle until RECONNECT. */
        if (ok) requestOk("RECONNECT");
        return ok;
    }
    const bool ok = requestOk("DISCONNECT");
    if (ok) {
        stopRssiPolling();
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            mUserDisabled = true;
        }
        /* The detach is synchronous here (no platform radio path yet):
         * notify only the completed state, no phantom DISABLING. */
        setWifiStateAndNotify(WIFI_STATE_DISABLED);
    }
    return ok;
}

/* --- scan -------------------------------------------------------------------- */

bool WifiManager::startScan() {
    /* wpa replies "OK", "FAIL", or "FAIL-BUSY" (throttled in-flight). */
    return requestOk("SCAN");
}

DhcpInfo WifiManager::getDhcpInfo() {
    std::lock_guard<std::mutex> lock(mStateMutex);
    return mLease.bound ? mLease.toDhcpInfo() : DhcpInfo();
}

std::vector<ScanResult> WifiManager::getScanResults() {
    /* Cached per scan generation: WPA_EVENT_SCAN_RESULTS invalidates. The
     * dump is the most expensive parse in the library and UI refreshes run
     * several times per scan. */
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        if (!mScanResultsCache.empty()) return mScanResultsCache;
    }
    /* AOSP pulls scans from the radio HAL (wificond), not the supplicant:
     * the nl80211 dump is that source — IEs / TSF / channel width — with
     * the ctrl_iface SCAN_RESULTS list as fallback. */
    std::vector<ScanResult> results;
    if (mRadioData)
        results = mRadioData->getScanResults(interfaceName());
    if (results.empty())
        results = WpaResponseParser::parseScanResults(mClient.request("SCAN_RESULTS"));
    std::lock_guard<std::mutex> lock(mStateMutex);
    mScanResultsCache = std::move(results);
    return mScanResultsCache;
}

/* --- connection info ------------------------------------------------------------ */

WifiInfo WifiManager::getConnectionInfo() {
    /* Cached copy: updateConnectionInfoFromStatus (event stream) and the
     * RSSI poller are the writers — a UI refresh costs no RPC. */
    std::lock_guard<std::mutex> lock(mStateMutex);
    return mConnectionInfo;
}

/* --- configured networks --------------------------------------------------------- */

std::vector<WifiConfiguration> WifiManager::getConfiguredNetworks() {
    /* Cached: 1 + 11N synchronous GET_NETWORK round trips per call would
     * hit every settings-screen refresh. Mutations through this manager
     * (add/update/remove/enable/disable/save) invalidate. */
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        if (!mConfigsDirty) return mConfiguredNetworksCache;
    }
    std::vector<WifiConfiguration> configs;
    const auto entries = WpaResponseParser::parseListNetworks(mClient.request("LIST_NETWORKS"));
    for (const auto& entry : entries) {
        WifiConfiguration config;
        config.networkId = entry.networkId;
        if (entry.flags.find("CURRENT") != std::string::npos)
            config.status = WifiConfiguration::Status::CURRENT;
        else if (entry.flags.find("DISABLED") != std::string::npos)
            config.status = WifiConfiguration::Status::DISABLED;
        else
            config.status = WifiConfiguration::Status::ENABLED;
        for (const char* variable : kNetworkVariables) {
            const std::string cmd = "GET_NETWORK " + std::to_string(entry.networkId)
                    + " " + variable;
            const std::string value = mClient.request(cmd);
            if (value.empty() || value.compare(0, 4, "FAIL") == 0) continue;
            /* ssid keeps its stored raw (quoted/hex) form; applyNetworkVariable
             * unquotes per variable. */
            WpaResponseParser::applyNetworkVariable(config, variable, value);
        }
        configs.push_back(config);
    }
    std::lock_guard<std::mutex> lock(mStateMutex);
    mConfiguredNetworksCache = std::move(configs);
    mConfigsDirty = false;
    return mConfiguredNetworksCache;
}

int WifiManager::addOrUpdateNetwork(const WifiConfiguration& config) {
    bool adding = (config.networkId == WifiConfiguration::INVALID_NETWORK_ID);
    int networkId = config.networkId;
    if (adding) {
        /* AOSP's connect reuses the existing block for the same SSID;
         * ADD_NETWORK on every tap left a trail of disabled duplicates on
         * the supplicant (SELECT_NETWORK disables all others). */
        for (const WifiConfiguration& existing : getConfiguredNetworks()) {
            if (existing.SSID == config.SSID) {
                adding = false;
                networkId = existing.networkId;
                break;
            }
        }
    }
    if (adding) {
        const std::string reply = mClient.request("ADD_NETWORK");
        if (reply.empty() || reply.compare(0, 4, "FAIL") == 0) return -1;
        networkId = atoi(reply.c_str());
    }
    for (const auto& var : WpaResponseParser::networkVariables(config)) {
        const std::string cmd = "SET_NETWORK " + std::to_string(networkId)
                + " " + var.first + " " + var.second;
        if (!requestOk(cmd)) {
            if (adding) mClient.request("REMOVE_NETWORK " + std::to_string(networkId));
            return -1;
        }
    }
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        mConfigsDirty = true;
    }
    return networkId;
}

int WifiManager::addNetwork(const WifiConfiguration& config) {
    return addOrUpdateNetwork(config);
}

bool WifiManager::updateNetwork(const WifiConfiguration& config) {
    if (config.networkId == WifiConfiguration::INVALID_NETWORK_ID) return false;
    return addOrUpdateNetwork(config) != -1;
}

bool WifiManager::removeNetwork(int netId) {
    const bool ok = requestOk("REMOVE_NETWORK " + std::to_string(netId));
    if (ok) {
        std::lock_guard<std::mutex> lock(mStateMutex);
        mConfigsDirty = true;
    }
    return ok;
}

bool WifiManager::enableNetwork(int netId, bool attemptConnect) {
    /* attemptConnect maps to SELECT_NETWORK: it re-enables the network and
     * disables all others (wpa's counterpart of the framework connect flow). */
    const bool ok = attemptConnect
            ? requestOk("SELECT_NETWORK " + std::to_string(netId))
            : requestOk("ENABLE_NETWORK " + std::to_string(netId));
    if (ok) {
        std::lock_guard<std::mutex> lock(mStateMutex);
        mConfigsDirty = true;
    }
    return ok;
}

bool WifiManager::disableNetwork(int netId) {
    const bool ok = requestOk("DISABLE_NETWORK " + std::to_string(netId));
    if (ok) {
        std::lock_guard<std::mutex> lock(mStateMutex);
        mConfigsDirty = true;
    }
    return ok;
}

bool WifiManager::saveConfiguration() {
    return requestOk("SAVE_CONFIG");
}

/* --- async operations ----------------------------------------------------------- */

void WifiManager::connect(int networkId, const ActionListener& listener) {
    if (!enableNetwork(networkId, true)) {
        if (listener.onFailure)
        listener.onFailure(ActionListener::FAILURE_INTERNAL_ERROR);
        return;
    }
    if (listener.onSuccess) listener.onSuccess();
}

void WifiManager::connect(const WifiConfiguration& config, const ActionListener& listener) {
    const int networkId = addOrUpdateNetwork(config);
    if (networkId == -1) {
        if (listener.onFailure)
        listener.onFailure(ActionListener::FAILURE_INTERNAL_ERROR);
        return;
    }
    if (!requestOk("SELECT_NETWORK " + std::to_string(networkId))) {
        if (listener.onFailure)
        listener.onFailure(ActionListener::FAILURE_INTERNAL_ERROR);
        return;
    }
    saveConfiguration();
    if (listener.onSuccess) listener.onSuccess();
}

void WifiManager::forget(int networkId, const ActionListener& listener) {
    if (!removeNetwork(networkId)) {
        if (listener.onFailure)
        listener.onFailure(ActionListener::FAILURE_INTERNAL_ERROR);
        return;
    }
    saveConfiguration();
    if (listener.onSuccess) listener.onSuccess();
}

void WifiManager::disable(int networkId, const ActionListener& listener) {
    if (!disableNetwork(networkId)) {
        if (listener.onFailure)
        listener.onFailure(ActionListener::FAILURE_INTERNAL_ERROR);
        return;
    }
    if (listener.onSuccess) listener.onSuccess();
}

void WifiManager::save(const ActionListener& listener) {
    if (!saveConfiguration()) {
        if (listener.onFailure)
        listener.onFailure(ActionListener::FAILURE_INTERNAL_ERROR);
        return;
    }
    if (listener.onSuccess) listener.onSuccess();
}

bool WifiManager::disconnect() {
    return requestOk("DISCONNECT");
}

bool WifiManager::reconnect() {
    return requestOk("RECONNECT");
}

bool WifiManager::reassociate() {
    return requestOk("REASSOCIATE");
}

bool WifiManager::pingSupplicant() {
    /* wpa_supplicant answers PING with "PONG" — AOSP WifiNative matches the
     * exact string, not the generic OK convention. */
    const std::string reply = mClient.request("PING");
    return reply.compare(0, 4, "PONG") == 0;
}

/* --- signal levels -------------------------------------------------------------- */

int WifiManager::calculateSignalLevel(int rssi, int numLevels) {
    if (rssi <= MIN_RSSI) {
        return 0;
    } else if (rssi >= MAX_RSSI) {
        return numLevels - 1;
    } else {
        const float inputRange = (MAX_RSSI - MIN_RSSI);
        const float outputRange = (numLevels - 1);
        return (int)((float)(rssi - MIN_RSSI) * outputRange / inputRange);
    }
}

int WifiManager::getMaxSignalLevel() {
    return RSSI_LEVELS - 1;
}

int WifiManager::compareSignalLevel(int rssiA, int rssiB) {
    return rssiA - rssiB;
}

/* --- listeners ---------------------------------------------------------------- */

void WifiManager::addWifiStateListener(const WifiStateListener& listener) {
    /* Resolve BEFORE enlisting: the resolve's notify chain must not
     * already contain this listener, or it receives the sticky state
     * twice (one delivery, like AOSP's sticky broadcast). */
    const int state = getWifiState();
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        mWifiStateListeners.push_back(listener);
    }
    if (state != WIFI_STATE_UNKNOWN) {
        WifiStateListener replay = listener;   /* operator() is non-const */
        replay(state);
    }
}

void WifiManager::removeWifiStateListener(const WifiStateListener& listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mWifiStateListeners.erase(std::remove(mWifiStateListeners.begin(),
            mWifiStateListeners.end(), listener), mWifiStateListeners.end());
}

void WifiManager::addScanResultsListener(const ScanResultsListener& listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mScanResultsListeners.push_back(listener);
}

void WifiManager::removeScanResultsListener(const ScanResultsListener& listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mScanResultsListeners.erase(std::remove(mScanResultsListeners.begin(),
            mScanResultsListeners.end(), listener), mScanResultsListeners.end());
}

void WifiManager::addNetworkStateListener(const NetworkStateListener& listener) {
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        mNetworkStateListeners.push_back(listener);
    }
    /* Sticky semantics, like NetworkCallback.onAvailable firing at
     * registration — but only once something real was reported: replaying
     * a pristine default WifiInfo is a spurious "not connected" event. */
    const WifiInfo info = getConnectionInfo();
    if (info.getSupplicantState() != SupplicantState::UNINITIALIZED) {
        NetworkStateListener replay = listener;   /* operator() is non-const */
        replay(info);
    }
}

void WifiManager::removeNetworkStateListener(const NetworkStateListener& listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mNetworkStateListeners.erase(std::remove(mNetworkStateListeners.begin(),
            mNetworkStateListeners.end(), listener), mNetworkStateListeners.end());
}

void WifiManager::addRssiListener(const RssiListener& listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mRssiListeners.push_back(listener);
}

void WifiManager::removeRssiListener(const RssiListener& listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mRssiListeners.erase(std::remove(mRssiListeners.begin(),
            mRssiListeners.end(), listener), mRssiListeners.end());
}

/* --- supplicant events (monitor thread / dispatcher) ------------------------------ */

void WifiManager::onLinkStateChanged(const std::string&, bool, bool) {
    refreshAndDispatchNetworkState();
}

void WifiManager::onAddressChanged(const std::string&, bool, const std::string&) {
    refreshAndDispatchNetworkState();
}

void WifiManager::refreshAndDispatchNetworkState() {
    /* Any interface address/link change can flip our IP presence; refresh
     * and re-dispatch (delivered on the monitor's event thread). */
    updateConnectionInfoFromStatus();
    notifyNetworkStateListeners();
}

void WifiManager::onSupplicantDisconnected() {
    setWifiStateAndNotify(WIFI_STATE_UNKNOWN);
}

void WifiManager::onSupplicantReconnected() {
    updateConnectionInfoFromStatus();
    /* Daemon-reachable does not mean enabled: a user-disabled wifi stays
     * disabled across daemon restarts (WifiSettingsStore semantics). */
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        if (mUserDisabled) return;
    }
    setWifiStateAndNotify(WIFI_STATE_ENABLED);
    seedDhcpFromCurrentState();
}

void WifiManager::onSupplicantEvent(const SupplicantEvent& event) {
    if (eventIs(event, WPA_EVENT_SCAN_RESULTS)) {
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            mScanResultsCache.clear();   /* new scan generation */
        }
        notifyScanResultsListeners();
    } else if (eventIs(event, WPA_EVENT_STATE_CHANGE)) {
        WifiInfo info;
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            info = mConnectionInfo;
        }
        std::string value;
        if (event.getArg("id", value)) info.setNetworkId(atoi(value.c_str()));
        if (event.getArg("state", value)) {
            try {
                info.setSupplicantState(SupplicantState::fromString(
                        WpaResponseParser::unquote(value)));
            } catch (const std::invalid_argument&e) {
            }
        }
        if (event.getArg("BSSID", value)) info.setBSSID(WpaResponseParser::unquote(value));
        if (event.getArg("SSID", value)) {
            /* The event prints the SSID via wpa_ssid_txt: unquoted printable
             * text. WifiSsid::fromString only accepts the quoted/hex forms
             * (AOSP contract), so a bare non-hex token falls back to raw
             * text like AOSP's WifiMonitor (createFromAsciiEncoded). */
            try {
                info.setSSID(WifiSsid::fromString(value));
            } catch (const std::invalid_argument&e) {
                info.setSSID(WifiSsid::fromUtf8Text(value));
            }
        }
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            mConnectionInfo = info;
        }
        notifyNetworkStateListeners();   /* re-snapshot: includes our write-back */
    } else if (eventIs(event, WPA_EVENT_CONNECTED)) {
        updateConnectionInfoFromStatus();
        startRssiPolling();
        /* IP provisioning begins with L2 completion (AOSP IpClient) */
        startDhcpIfNeeded();
        notifyNetworkStateListeners();
    } else if (eventIs(event, WPA_EVENT_DISCONNECTED)) {
        stopRssiPolling();
        stopDhcpAndRelease();
        /* WifiInfo goes stale the moment L2 drops (AOSP clears SSID/BSSID
         * on disconnect): without this the card keeps showing the old
         * "connected <ssid>" with a residual lease IP while the supplicant
         * is actually SCANNING. */
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            mConnectionInfo = WifiInfo();
            mLastRssi.store(WifiInfo::INVALID_RSSI);
        }
        updateConnectionInfoFromStatus();
        notifyNetworkStateListeners();
    }
    /* TODO(porting): WPA: 4-Way Handshake failed → ERROR_AUTHENTICATING
     * surfaced through the connect ActionListener once the async connect
     * flow is wired (needs the broadcast-era state machine). */
}

/* --- internal ------------------------------------------------------------------- */

bool WifiManager::requestOk(const std::string& cmd) {
    const std::string reply = mClient.request(cmd);
    return reply.compare(0, 2, "OK") == 0;
}

void WifiManager::setWifiStateAndNotify(int newState) {
    int previous;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        previous = mWifiState;
        if (previous == newState) return;
        mWifiState = newState;
    }
    notifyWifiStateListeners(newState);
}

void WifiManager::notifyWifiStateListeners(int state) {
    std::vector<WifiStateListener> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mWifiStateListeners;
    }
    for (WifiStateListener listener : listeners) listener(state);
}

void WifiManager::notifyScanResultsListeners() {
    std::vector<ScanResultsListener> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mScanResultsListeners;
    }
    for (ScanResultsListener listener : listeners) listener();
}

void WifiManager::notifyNetworkStateListeners() {
    WifiInfo info;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        info = mConnectionInfo;
    }
    std::vector<NetworkStateListener> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mNetworkStateListeners;
    }
    for (NetworkStateListener listener : listeners) listener(info);
}

void WifiManager::notifyRssiListeners(int rssi) {
    std::vector<RssiListener> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mRssiListeners;
    }
    for (RssiListener listener : listeners) listener(rssi);
}

void WifiManager::updateConnectionInfoFromStatus() {
    WifiInfo info;
    const std::string status = mClient.request("STATUS");
    if (status.empty()) return;
    WpaResponseParser::applyStatus(info, status);
    WpaResponseParser::applySignalPoll(info, mClient.request("SIGNAL_POLL"));
    /* AOSP takes the address from IpClient/LinkProperties, never from the
     * supplicant; the supplicant STATUS only knows about its own (unused)
     * DHCP. Read the interface when STATUS did not provide one. */
    if (info.getIpAddress() == 0) {
        /* AOSP takes the address from IpClient/LinkProperties, never from
         * the supplicant; the shared single-interface probe is that read. */
        std::string dotted;
        if (interfaceHasIpv4Address(interfaceName(), &dotted))
            info.setInetAddress(dotted);
    }
    std::lock_guard<std::mutex> lock(mStateMutex);
    mConnectionInfo = info;
    mLastRssi.store(info.getRssi());
}

void WifiManager::startRssiPolling() {
    bool expected = false;
    if (!mRssiPolling.compare_exchange_strong(expected, true)) return;
    mRssiPollThread = std::thread(&WifiManager::rssiPollLoop, this);
}

void WifiManager::stopRssiPolling() {
    if (mRssiPolling.exchange(false)) {
        if (mRssiPollThread.joinable()) mRssiPollThread.join();
    }
}

void WifiManager::rssiPollLoop() {
    /* AOSP polls RSSI every 3s while connected (WifiStateMachine) and fires
     * RSSI_CHANGED on change; ctrl_iface exposes it via SIGNAL_POLL. */
    while (mRssiPolling.load()) {
        for (int waited = 0; waited < POLL_RSSI_INTERVAL_MS && mRssiPolling.load(); waited += 100)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!mRssiPolling.load()) break;
        const std::string poll = mClient.request("SIGNAL_POLL");
        if (poll.empty()) continue;
        WifiInfo probe;
        WpaResponseParser::applySignalPoll(probe, poll);
        const int rssi = probe.getRssi();
        if (rssi == WifiInfo::INVALID_RSSI || rssi == mLastRssi.exchange(rssi)) continue;
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            mConnectionInfo.setRssi(rssi);
        }
        notifyRssiListeners(rssi);
    }
}

void WifiManager::seedDhcpFromCurrentState() {
    /* The DHCP lifecycle is event-driven (CTRL-EVENT-CONNECTED), but the
     * supplicant often auto-reconnects BEFORE the app starts — the event
     * is long gone while the state says COMPLETED. Seed from the live
     * state like the ethernet snapshot does. */
    updateConnectionInfoFromStatus();   /* mConnectionInfo is pristine at initialize() */
    WifiInfo info;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        info = mConnectionInfo;
    }
    if (info.getSupplicantState() == SupplicantState::COMPLETED && info.getIpAddress() == 0)
        startDhcpIfNeeded();
}

void WifiManager::startDhcpIfNeeded() {
    const std::string iface = interfaceName();
    if (iface.empty()) return;
    WifiDhcpSession* session = new WifiDhcpSession();
    {
        /* Check-then-set under one lock: the monitor thread's
         * CTRL-EVENT-CONNECTED races initialize()'s seed here, and the
         * loser must not orphan a session (its stop flag would never be
         * set). The thread is created inside the same critical section so
         * stopDhcpAndRelease never sees a half-constructed session. */
        std::lock_guard<std::mutex> lock(mStateMutex);
        if (mDhcpSession) {   /* already provisioning/renewing */
            delete session;
            return;
        }
        mDhcpSession = session;
        session->thread = std::thread([this, session, iface]() {
            DhcpClient* client = DhcpClient::create(iface);
            DhcpClient::Lease lease;
            fprintf(stdout, "WifiManager: dhcp request on %s\n", iface.c_str());
            fflush(stdout);
            if (client->requestLease(lease)) {
                fprintf(stdout, "WifiManager: lease on %s: %s gw %s (%us)\n", iface.c_str(),
                        lease.ipAddress.c_str(), lease.gateway.c_str(), lease.leaseDurationSec);
                fflush(stdout);
                const auto applyLease = [this, iface](const DhcpClient::Lease& l) {
                    applyIpConfiguration(iface, toStaticIpConfiguration(l));
                    std::lock_guard<std::mutex> lock(mStateMutex);
                    mLease = l;
                };
                applyLease(lease);
                /* renewal loop shared with the ethernet session (T1 or half
                 * the lease, renew, fresh-DISCOVER fallback). */
                runLeaseRenewalLoop(client, lease, applyLease, session->stop);
            } else {
                fprintf(stderr, "WifiManager E: no lease on %s (timeout/NAK)\n",
                        iface.c_str());
            }
            delete client;
        });
    }
}

void WifiManager::stopDhcpAndRelease() {
    /* Take sole ownership under the lock (initializer swap): concurrent
     * stops (monitor DISCONNECTED vs destructor) must not double-join or
     * double-delete the same session. */
    WifiDhcpSession* session = nullptr;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        session = mDhcpSession;
        mDhcpSession = nullptr;
    }
    if (!session) return;
    session->stop.store(true);
    if (session->thread.joinable()) session->thread.join();
    delete session;
    DhcpClient::Lease lease;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        lease = mLease;
        mLease = DhcpClient::Lease();
    }
    const std::string iface = interfaceName();
    if (lease.bound && !iface.empty()) {
        DhcpClient* client = DhcpClient::create(iface);
        client->releaseLease(lease);   /* best effort */
        delete client;
    }
}

/* --- Soft AP (hostapd-backed SoftApManager path) ----------------------------- */

const MacAddress WifiManager::ALL_ZEROS_MAC_ADDRESS = MacAddress::ALL_ZEROS_MAC_ADDRESS;

std::string WifiManager::softApInterface() {
    std::lock_guard<std::mutex> lock(mStateMutex);
    if (!mSoftApIface.empty()) return mSoftApIface;
    ensureSoftApStoreLoaded();
    if (!mSoftApIface.empty()) return mSoftApIface;
    return mIfaceName;
}

void WifiManager::configureSoftApInterface(const std::string& iface) {
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        mSoftApIface = iface;
    }
    /* the choice must survive the process (wpatest apiface | apstart are
     * separate invocations) — persist alongside the stored configuration */
    persistSoftApConfiguration(getSoftApConfiguration());
}

void WifiManager::ensureSoftApStoreLoaded() {
    if (mSoftApStoreLoaded) return;
    mSoftApStoreLoaded = true;
    SoftApConfigStore::Record record;
    if (!SoftApConfigStore::load(&record)) return;
    mSoftApIface = record.iface;
    if (record.hasConfig && !mSoftApConfigSet) {
        mSoftApConfig = record.config;
        mSoftApConfigSet = true;
    }
}

int WifiManager::getWifiApState() {
    std::lock_guard<std::mutex> lock(mStateMutex);
    return mWifiApState;
}

bool WifiManager::isWifiApEnabled() {
    return getWifiApState() == WIFI_AP_STATE_ENABLED;
}

SoftApConfiguration WifiManager::getSoftApConfiguration() {
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        ensureSoftApStoreLoaded();
        if (mSoftApConfigSet) return mSoftApConfig;
    }
    /* Framework default, stored like WifiApConfigStore stores its generated
     * default so a rebooted process serves the same SSID. WifiApConfigStore
     * generates the default ONCE and serves it forever; caching it in
     * mSoftApConfig makes this idempotent in-process too — the old path
     * generated a NEW random default (and rewrote the store) on EVERY call
     * while nothing was user-configured, so the SSID shown to the user never
     * matched the one a subsequent startTetheredHotspot(nullptr) ran. */
    const SoftApConfiguration config = defaultSoftApConfiguration();
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        mSoftApConfig = config;
        mSoftApConfigSet = true;
    }
    persistSoftApConfiguration(config);
    return config;
}

bool WifiManager::setSoftApConfiguration(const SoftApConfiguration& config) {
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        mSoftApConfig = config;
        mSoftApConfigSet = true;
    }
    persistSoftApConfiguration(config);
    return true;
}

void WifiManager::persistSoftApConfiguration(const SoftApConfiguration& config) {
    SoftApConfigStore::Record record;
    record.hasConfig = true;
    record.config = config;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        record.iface = !mSoftApIface.empty() ? mSoftApIface : std::string();
        if (record.iface.empty()) {
            /* keep whatever the store already carries */
            SoftApConfigStore::Record existing;
            if (SoftApConfigStore::load(&existing)) record.iface = existing.iface;
        }
    }
    SoftApConfigStore::save(record);
}

/* AOSP draws the default hotspot SSID suffix and WPA2 passphrase from
 * SecureRandom. Plain rand() here was never seeded on the Soft AP path
 * (the only srand in src/net is the STA-side DHCP client's), so every
 * process on every device produced the SAME "random" default SSID and
 * passphrase — glibc's fixed seed-1 sequence, a universally known default
 * hotspot password. This engine is seeded from /dev/urandom instead
 * (std::random_device is permitted to be deterministic on some targets;
 * the explicit read is not). */
static int softApRandom() {
    static thread_local std::mt19937 engine = [] {
        std::random_device::result_type seed = 0;
        const int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
        if (fd >= 0) {
            const ssize_t n = read(fd, &seed, sizeof(seed));
            (void)n;
            close(fd);
        }
        return std::mt19937(seed);
    }();
    return static_cast<int>(engine());
}

SoftApConfiguration WifiManager::defaultSoftApConfiguration() {
    /* WifiApConfigStore default shape: "AndroidAP_" + 4 random digits with
     * a random WPA2 passphrase. */
    char ssid[16];
    snprintf(ssid, sizeof(ssid), "AndroidAP_%04d", 1000 + softApRandom() % 9000);
    static const char kPassChars[] =
            "abcdefghjkmnpqrstuvwxyzABCDEFGHJKMNPQRSTUVWXYZ23456789";
    char pass[11];
    for (int i = 0; i < 10; ++i)
        pass[i] = kPassChars[softApRandom() % (sizeof(kPassChars) - 1)];
    pass[10] = '\0';
    return SoftApConfiguration::Builder()
            .setSsid(ssid)
            .setPassphrase(pass, SoftApConfiguration::SECURITY_TYPE_WPA2_PSK)
            .setBand(SoftApConfiguration::BAND_2GHZ)
            .build();
}

bool WifiManager::validateSoftApConfiguration(const SoftApConfiguration& config) {
    /* The module's validity test = renderability by the hostapd backend
     * (AOSP validates against HAL features + country channels; the nl80211
     * country channel list is TODO along with the interface work). */
    return HostapdClient::writeConfigFile("/tmp/cdroid-softap-validate.conf",
            "wlan0", "/tmp/cdroid-softap-validate", config, nullptr);
}

bool WifiManager::startTetheredHotspot(const SoftApConfiguration* softApConfig) {
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        if (mWifiApState == WIFI_AP_STATE_ENABLING
                || mWifiApState == WIFI_AP_STATE_ENABLED)
            return false;   /* only from down, like SoftApManager */
    }
    const SoftApConfiguration config = withRandomizedBssid(softApConfig
            ? *softApConfig : getSoftApConfiguration());
    const std::string iface = softApInterface();
    if (iface.empty()) {
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            mWifiApFailureReason = SAP_START_FAILURE_GENERAL;
        }
        setWifiApStateAndNotify(WIFI_AP_STATE_FAILED);
        return false;
    }
    const std::string dir = HostapdClient::configDirectory(iface);
    const std::string confPath = dir + "/hostapd.conf";
    const std::string pidFile = dir + "/hostapd.pid";
    const std::string logFile = dir + "/hostapd.log";
    const std::string ctrlDir = dir + "/ctrl";
    /* 0700: this tree carries hostapd.conf with the WPA passphrase (AOSP
     * keeps WifiApConfigStore data under /data/misc with restrictive
     * permissions) — /tmp's 1777 must not make it world-readable. chmod
     * after each mkdir so a directory left 0755 by an older build is
     * tightened too (mkdir alone never rewrites an existing dir). */
    mkdir("/tmp/cdroid-softap", 0700);
    chmod("/tmp/cdroid-softap", 0700);
    mkdir(dir.c_str(), 0700);
    chmod(dir.c_str(), 0700);
    mkdir(ctrlDir.c_str(), 0700);
    chmod(ctrlDir.c_str(), 0700);

    std::string error;
    if (!HostapdClient::writeConfigFile(confPath, iface, ctrlDir, config, &error)) {
        fprintf(stderr, "WifiManager E: SoftApConfiguration rejected: %s\n",
                error.c_str());
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            mWifiApFailureReason = SAP_START_FAILURE_UNSUPPORTED_CONFIGURATION;
        }
        setWifiApStateAndNotify(WIFI_AP_STATE_FAILED);
        return false;
    }

    setWifiApStateAndNotify(WIFI_AP_STATE_ENABLING);
    HostapdClient::stopDaemon(pidFile);   /* stale daemon from an earlier run */
    const pid_t pid = HostapdClient::startDaemon(iface, confPath, pidFile, logFile);
    if (pid < 0) {
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            mWifiApFailureReason = SAP_START_FAILURE_GENERAL;
        }
        setWifiApStateAndNotify(WIFI_AP_STATE_FAILED);
        return false;
    }
    /* Readiness = ctrl socket answering PING; -B daemonizes before the
     * interface is fully up, so the socket appearing IS the ready signal.
     * Radio bring-up on slow radios (hwsim: nl80211 init + channel config)
     * takes multiple seconds — AOSP's SoftApManager start timeout is the
     * 30 s class; anything shorter fails healthy setups. */
    mHostapd.close();
    mHostapd.setCtrlPath(ctrlDir + "/" + iface);
    bool ready = false;
    for (int waited = 0; waited < 30000 && !ready; waited += 100) {
        if (!mHostapd.isConnected()) mHostapd.connect();
        std::string reply;
        /* compare(pos, len, s) matches the len-char substring: "PONG" is
         * four characters (a 3 here compares "PON" vs "PONG" — never equal). */
        if (mHostapd.request("PING", reply)
                && reply.compare(0, 4, "PONG") == 0)
            ready = true;
        else
            usleep(100 * 1000);
    }
    if (!ready) {
        fprintf(stderr,
                "WifiManager E: hostapd ctrl socket never came up (log: %s)\n",
                logFile.c_str());
        mHostapd.close();
        HostapdClient::stopDaemon(pidFile);
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            mWifiApFailureReason = SAP_START_FAILURE_GENERAL;
        }
        setWifiApStateAndNotify(WIFI_AP_STATE_FAILED);
        return false;
    }
    /* Operating info + client seed (AOSP reports these as the AP comes up). */
    refreshSoftApInfoFromHostapd();
    refreshSoftApClientsFromHostapd();
    setWifiApStateAndNotify(WIFI_AP_STATE_ENABLED);
    notifySoftApCallbacksInfo();
    bool startIdleTimer = false;
    int64_t idleTimeout = 0;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        mActiveSoftApConfig = config;
        mActiveSoftApConfigValid = true;
        mSoftApAutoShutdownEnabled = config.isAutoShutdownEnabled();
        /* DEFAULT_TIMEOUT resolves to the framework's soft-ap idle delay
         * (config_wifi_framework_soft_ap_timeout_delay; TODO(porting):
         * verify against frameworks/base res values — 10 min used here). */
        mSoftApShutdownTimeoutMillis = config.getShutdownTimeoutMillis()
                == SoftApConfiguration::DEFAULT_TIMEOUT
                ? 600000 : config.getShutdownTimeoutMillis();
        startIdleTimer = mSoftApAutoShutdownEnabled && mSoftApClients.empty();
        idleTimeout = mSoftApShutdownTimeoutMillis;
    }
    /* outside the lock: cancel joins the timer thread, which itself takes
     * mStateMutex inside its idle poll */
    if (startIdleTimer) {
        cancelSoftApIdleShutdown();
        scheduleSoftApIdleShutdown(idleTimeout);
    }
    /* AP-side DHCP (AOSP IpServer starts the tethering DHCP server once the
     * interface is up). Failure is logged, not fatal for the AP itself —
     * the tethering integration (P2) owns the teardown policy. */
    {
        DhcpServer::Config dhcp;
        dhcp.iface = iface;
        dhcp.workDir = dir;
        DhcpServer* server = DhcpServer::create(dhcp);
        if (server->start()) {
            std::lock_guard<std::mutex> lock(mStateMutex);
            mApDhcpServer = server;
        } else {
            fprintf(stderr, "WifiManager E: AP dhcp server failed to start\n");
            delete server;
        }
    }
    return true;
}

bool WifiManager::stopSoftAp() {
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        if (mWifiApState != WIFI_AP_STATE_ENABLED
                && mWifiApState != WIFI_AP_STATE_ENABLING
                && mWifiApState != WIFI_AP_STATE_FAILED)
            return false;   /* not up */
    }
    cancelSoftApIdleShutdown();
    setWifiApStateAndNotify(WIFI_AP_STATE_DISABLING);
    mHostapd.close();
    const std::string iface = softApInterface();
    const bool stopped = HostapdClient::stopDaemon(
            HostapdClient::configDirectory(iface) + "/hostapd.pid");
    /* DHCP teardown + client/info reset (IpServer teardown order). */
    {
        DhcpServer* server = nullptr;
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            server = mApDhcpServer;
            mApDhcpServer = nullptr;
            mSoftApClients.clear();
            mSoftApInfo = SoftApInfo();
        }
        delete server;
    }
    if (!iface.empty()) clearIpConfiguration(iface);
    setWifiApStateAndNotify(WIFI_AP_STATE_DISABLED);
    return stopped;
}

bool WifiManager::startSoftAp(const WifiConfiguration* wifiConfig) {
    if (!wifiConfig) return startTetheredHotspot(nullptr);
    SoftApConfiguration::Builder builder;
    /* WifiConfiguration.SSID is the quoted toString form — WifiSsid::fromString
     * is the parser for exactly that shape.
     * Every Builder setter validates and THROWS std::invalid_argument on a
     * bad value (setChannel rejects channel/band pairs like 165@2GHz, exactly
     * the IllegalArgumentException AOSP surfaces to the caller). This is a
     * public API on the process's main path, so the whole build is guarded:
     * a rejected caller value returns false instead of terminating the
     * process through an escaping exception. */
    try {
        builder.setWifiSsid(WifiSsid::fromString(wifiConfig->SSID));
        int band = SoftApConfiguration::BAND_2GHZ;
        if (wifiConfig->apBand == WifiConfiguration::AP_BAND_5GHZ)
            band = SoftApConfiguration::BAND_5GHZ;
        /* AP_BAND_ANY / AP_BAND_60GHZ collapse to 2GHz: the v1 backend is a
         * single-interface AP (bridged bands rejected at render). */
        if (wifiConfig->apChannel > 0)
            builder.setChannel(wifiConfig->apChannel, band);
        else
            builder.setBand(band);
        if (wifiConfig->allowedKeyManagement.test(WifiConfiguration::KeyMgmt::WPA2_PSK)) {
            if (wifiConfig->preSharedKey.empty()) return false;
            builder.setPassphrase(wifiConfig->preSharedKey,
                    SoftApConfiguration::SECURITY_TYPE_WPA2_PSK);
        } else if (!wifiConfig->allowedKeyManagement.test(WifiConfiguration::KeyMgmt::NONE)) {
            return false;   /* key management the backend cannot render */
        }
        builder.setHiddenSsid(wifiConfig->hiddenSSID);
    } catch (const std::invalid_argument& e) {
        fprintf(stderr, "WifiManager E: startSoftAp config rejected: %s\n", e.what());
        return false;
    }
    const SoftApConfiguration config = builder.build();
    return startTetheredHotspot(&config);
}

void WifiManager::addWifiApStateListener(const WifiApStateListener& listener) {
    /* Sticky dispatch like addWifiStateListener (AOSP sticky broadcast). */
    const int state = getWifiApState();
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        mWifiApListeners.push_back(listener);
    }
    WifiApStateListener replay = listener;   /* operator() is non-const */
    replay(state);
}

void WifiManager::removeWifiApStateListener(const WifiApStateListener& listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mWifiApListeners.erase(std::remove(mWifiApListeners.begin(),
            mWifiApListeners.end(), listener), mWifiApListeners.end());
}

void WifiManager::setWifiApStateAndNotify(int newState) {
    int previous;
    int failureCode = 0;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        previous = mWifiApState;
        if (previous == newState) return;
        mWifiApState = newState;
        if (newState == WIFI_AP_STATE_FAILED)
            failureCode = mWifiApFailureReason;
    }
    notifyWifiApStateListeners(newState);
    /* SoftApCallback#onStateChanged rides the same transitions (AOSP fans
     * the single SoftApManager state into both surfaces). */
    std::vector<SoftApCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        callbacks = mSoftApCallbacks;
    }
    for (const SoftApCallback& callback : callbacks)
        callback.onStateChanged(newState, failureCode);
}

void WifiManager::notifyWifiApStateListeners(int state) {
    std::vector<WifiApStateListener> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mWifiApListeners;
    }
    for (WifiApStateListener listener : listeners) listener(state);
}

/* --- SoftApCallback plumbing --------------------------------------------------- */

void WifiManager::registerSoftApCallback(const SoftApCallback& callback) {
    /* sticky: current state, info and capability arrive at registration
     * (the binder path replays them from SoftApManager). */
    int state;
    int failureCode = 0;
    SoftApInfo info;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        mSoftApCallbacks.push_back(callback);
    }
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        state = mWifiApState;
        if (state == WIFI_AP_STATE_FAILED) failureCode = mWifiApFailureReason;
        info = mSoftApInfo;
    }
    callback.onStateChanged(state, failureCode);
    callback.onInfoChanged({info});
    callback.onCapabilityChanged(SoftApCapability());
}

void WifiManager::unregisterSoftApCallback(const SoftApCallback& callback) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mSoftApCallbacks.erase(std::remove(mSoftApCallbacks.begin(),
            mSoftApCallbacks.end(), callback), mSoftApCallbacks.end());
}

void WifiManager::notifySoftApClientsChanged(int reasonCode) {
    std::vector<WifiClient> clients;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        clients = mSoftApClients;
    }
    std::vector<SoftApCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        callbacks = mSoftApCallbacks;
    }
    for (const SoftApCallback& callback : callbacks)
        callback.onConnectedClientsChanged(clients, reasonCode);
}

void WifiManager::notifySoftApCallbacksInfo() {
    SoftApInfo info;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        info = mSoftApInfo;
    }
    std::vector<SoftApCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        callbacks = mSoftApCallbacks;
    }
    for (const SoftApCallback& callback : callbacks)
        callback.onInfoChanged({info});
}

void WifiManager::notifySoftApCallbacksCapability() {
    std::vector<SoftApCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        callbacks = mSoftApCallbacks;
    }
    for (const SoftApCallback& callback : callbacks)
        callback.onCapabilityChanged(SoftApCapability());
}

/* --- Soft AP P3: enforcement, randomization, idle shutdown ------------------- */

void WifiManager::enforceBlockedClient(const MacAddress& mac) {
    std::vector<MacAddress> blocked;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        if (!mActiveSoftApConfigValid) return;
        blocked = mActiveSoftApConfig.getBlockedClientList();
    }
    if (std::find(blocked.begin(), blocked.end(), mac) == blocked.end()) return;
    /* deny_mac_file normally rejects these at association already — this is
     * the runtime half (list changes without restart) + the callback. */
    mHostapd.request("DISASSOCIATE " + mac.toString());
    std::vector<SoftApCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        callbacks = mSoftApCallbacks;
    }
    for (const SoftApCallback& callback : callbacks)
        callback.onBlockedClientConnecting(
                WifiClient(mac, softApInterface()),
                SoftApCallback::SAP_CLIENT_BLOCK_REASON_CODE_BLOCKED_BY_USER);
}

SoftApConfiguration WifiManager::withRandomizedBssid(
        const SoftApConfiguration& config) {
    if (config.getBssid().getBytes().size() == 6) return config;   /* explicit */
    if (config.getMacRandomizationSetting()
            == SoftApConfiguration::RANDOMIZATION_NONE)
        return config;
    /* RANDOMIZATION_PERSISTENT needs the per-SSID persisted address —
     * TODO(porting); a fresh random address each start is the
     * NON_PERSISTENT behavior applied in its place. */
    char text[18];
    snprintf(text, sizeof(text), "02:%02x:%02x:%02x:%02x:%02x",
             rand() & 0xff, rand() & 0xff, rand() & 0xff,
             rand() & 0xff, rand() & 0xff);
    return SoftApConfiguration::Builder(config)
            .setBssid(MacAddress::fromString(text))
            /* an explicit BSSID voids randomization (build() enforces the
             * exclusivity; Builder(other)'s auto-fix only fires when the
             * SOURCE already carries a bssid, not on a later setBssid) */
            .setMacRandomizationSetting(SoftApConfiguration::RANDOMIZATION_NONE)
            .build();
}

void WifiManager::scheduleSoftApIdleShutdown(int64_t timeoutMillis) {
    bool expected = false;
    if (!mSoftApIdleTimerActive.compare_exchange_strong(expected, true)) return;
    mSoftApIdleThread = std::thread([this, timeoutMillis]() {
        const auto deadline = std::chrono::steady_clock::now()
                + std::chrono::milliseconds(timeoutMillis);
        while (mSoftApIdleTimerActive.load()
                && std::chrono::steady_clock::now() < deadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        if (!mSoftApIdleTimerActive.load()) return;   /* cancelled */
        mSoftApIdleTimerActive.store(false);
        fprintf(stdout, "WifiManager: soft ap idle timeout, shutting down\n");
        softApIdleShutdown();
    });
}

void WifiManager::cancelSoftApIdleShutdown() {
    mSoftApIdleTimerActive.exchange(false);
    /* Join whenever the worker is joinable — not only right after arming it.
     * A NATURAL timer expiry finishes the thread while mSoftApIdleTimerActive
     * is already false (the worker stores false itself before tearing down),
     * so the old exchange-gated join never ran: the finished thread stayed
     * joinable forever, and the next scheduleSoftApIdleShutdown move-assign
     * (or ~WifiManager's member destruction) aborted the process on it.
     * The flag still makes the poll loop exit promptly, so the wait is
     * bounded by one teardown. The self-guard keeps a callback running ON
     * the idle thread (via softApIdleShutdown's notifications) from joining
     * itself — in that case the next cancel joins it. */
    if (mSoftApIdleThread.joinable()
            && mSoftApIdleThread.get_id() != std::this_thread::get_id())
        mSoftApIdleThread.join();
}

void WifiManager::softApIdleShutdown() {
    setWifiApStateAndNotify(WIFI_AP_STATE_DISABLING);
    const std::string iface = softApInterface();
    HostapdClient::stopDaemon(
            HostapdClient::configDirectory(iface) + "/hostapd.pid");
    DhcpServer* server = nullptr;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        server = mApDhcpServer;
        mApDhcpServer = nullptr;
        mSoftApClients.clear();
        mSoftApInfo = SoftApInfo();
        mActiveSoftApConfigValid = false;
    }
    delete server;   /* dnsmasq SIGTERM — no thread joins inside */
    if (!iface.empty()) clearIpConfiguration(iface);
    setWifiApStateAndNotify(WIFI_AP_STATE_DISABLED);
}

void WifiManager::refreshSoftApClientsFromHostapd() {
    /* Seed from LIST_CLIENTS (a "MAC ..." line per associated STA); the
     * AP-STA-* event stream keeps it current afterwards. */
    const std::string reply = mHostapd.request("LIST_CLIENTS");
    if (reply.empty() || reply.compare(0, 4, "FAIL") == 0) return;
    const std::string iface = softApInterface();
    std::vector<WifiClient> clients;
    size_t pos = 0;
    while (pos < reply.size()) {
        const size_t nl = reply.find('\n', pos);
        std::string line = reply.substr(pos, nl == std::string::npos
                ? std::string::npos : nl - pos);
        const size_t blank = line.find(' ');
        const std::string first = line.substr(0, blank == std::string::npos
                ? std::string::npos : blank);
        const MacAddress mac = MacAddress::fromString(first);
        if (mac.getBytes().size() == 6)
            clients.push_back(WifiClient(mac, iface));
        if (nl == std::string::npos) break;
        pos = nl + 1;
    }
    std::lock_guard<std::mutex> lock(mStateMutex);
    mSoftApClients = std::move(clients);
}

void WifiManager::refreshSoftApInfoFromHostapd() {
    /* hostapd STATUS carries state/freq/ssid/bssid[channel]; the frequency
     * is what SoftApInfo reports (bandwidth stays AUTO until the nl80211
     * operating-channel query lands). */
    const std::string status = mHostapd.request("STATUS");
    if (status.empty()) return;
    SoftApInfo info;
    size_t pos = 0;
    while (pos < status.size()) {
        const size_t nl = status.find('\n', pos);
        const std::string line = status.substr(pos, nl == std::string::npos
                ? std::string::npos : nl - pos);
        if (line.compare(0, 5, "freq=") == 0)
            info.frequency = atoi(line.c_str() + 5);
        pos = (nl == std::string::npos) ? status.size() : nl + 1;
    }
    std::lock_guard<std::mutex> lock(mStateMutex);
    mSoftApInfo = info;
}

void WifiManager::onHostapdEvent(const HostapdClient::HostapdEvent& event) {
    if (eventIs(event, AP_STA_CONNECTED) || eventIs(event, AP_STA_DISCONNECTED)) {
        /* Positional argument: "<event> <mac>". */
        std::string macText = event.raw;
        if (macText.size() > event.name.size() + 1)
            macText = macText.substr(event.name.size() + 1);
        else
            macText.clear();
        const MacAddress mac = MacAddress::fromString(macText);
        const bool connected = eventIs(event, AP_STA_CONNECTED);
        const std::string iface = softApInterface();
        if (connected) enforceBlockedClient(mac);
        bool becameIdle = false;
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            const auto it = std::find_if(mSoftApClients.begin(),
                    mSoftApClients.end(),
                    [&mac](const WifiClient& c) { return c.getMacAddress() == mac; });
            if (connected) {
                if (it == mSoftApClients.end())
                    mSoftApClients.push_back(WifiClient(mac, iface));
            } else if (it != mSoftApClients.end()) {
                mSoftApClients.erase(it);
            }
            becameIdle = mSoftApClients.empty();
        }
        /* Idle-shutdown bookkeeping (outside mStateMutex: joining). */
        if (connected) {
            cancelSoftApIdleShutdown();
        } else if (becameIdle) {
            bool autoShutdown;
            int64_t timeout;
            {
                std::lock_guard<std::mutex> lock(mStateMutex);
                autoShutdown = mSoftApAutoShutdownEnabled;
                timeout = mSoftApShutdownTimeoutMillis;
            }
            if (autoShutdown) {
                cancelSoftApIdleShutdown();
                scheduleSoftApIdleShutdown(timeout);
            }
        }
        notifySoftApClientsChanged(0);
    }
}

void WifiManager::onHostapdDisconnected() {
    /* Daemon loss while up is the SoftApManager failure path (AOSP reports
     * WIFI_AP_STATE_FAILED with the last failure reason). */
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        if (mWifiApState != WIFI_AP_STATE_ENABLED) return;
    }
    setWifiApStateAndNotify(WIFI_AP_STATE_FAILED);
}

void WifiManager::onHostapdReconnected() {
    /* ctrl socket back after a loss: the state machine stays where it was;
     * without a live daemon query we do not resurrect ENABLED here. */
}

} // namespace cdroid
