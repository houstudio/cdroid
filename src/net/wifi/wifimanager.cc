/* Port of android.net.wifi.WifiManager (android-36), wpa_ctrl backed. */
#include <wifi/wifimanager.h>

#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>   // inet_ntop (wpa_control sockaddr dump)

#include <ipapplicator.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>

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
    mClient.setEventCallback(this);
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
    stopDhcpAndRelease();
    if (mAddressMonitor) {
        mAddressMonitor->stop();
        delete mAddressMonitor;
        mAddressMonitor = nullptr;
    }
    delete mRadioData;
    mRadioData = nullptr;
    stopRssiPolling();
    mClient.close();
}

bool WifiManager::initialize(const std::string& ctrlPath) {
    if (mClient.isConnected()) return true;
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
    /* Cached: the event stream (refreshWifiStateFromSupplicant /
     * onSupplicantReconnected / setWifiEnabled) is the writer, so UI
     * refreshes pay no synchronous RPC. Radio control (rfkill / interface
     * up-down) needs a platform hook: TODO(porting). */
    std::lock_guard<std::mutex> lock(mStateMutex);
    return mWifiState;
}

void WifiManager::refreshWifiStateFromSupplicant() {
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
    if (enabled) return initialize();
    const bool ok = requestOk("DISCONNECT");
    if (ok) {
        stopRssiPolling();
        setWifiStateAndNotify(WIFI_STATE_DISABLING);
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

void WifiManager::connect(int networkId, ActionListener* listener) {
    if (!enableNetwork(networkId, true)) {
        if (listener) listener->onFailure(ActionListener::FAILURE_INTERNAL_ERROR);
        return;
    }
    if (listener) listener->onSuccess();
}

void WifiManager::connect(const WifiConfiguration& config, ActionListener* listener) {
    const int networkId = addOrUpdateNetwork(config);
    if (networkId == -1) {
        if (listener) listener->onFailure(ActionListener::FAILURE_INTERNAL_ERROR);
        return;
    }
    if (!requestOk("SELECT_NETWORK " + std::to_string(networkId))) {
        if (listener) listener->onFailure(ActionListener::FAILURE_INTERNAL_ERROR);
        return;
    }
    saveConfiguration();
    if (listener) listener->onSuccess();
}

void WifiManager::forget(int networkId, ActionListener* listener) {
    if (!removeNetwork(networkId)) {
        if (listener) listener->onFailure(ActionListener::FAILURE_INTERNAL_ERROR);
        return;
    }
    saveConfiguration();
    if (listener) listener->onSuccess();
}

void WifiManager::disable(int networkId, ActionListener* listener) {
    if (!disableNetwork(networkId)) {
        if (listener) listener->onFailure(ActionListener::FAILURE_INTERNAL_ERROR);
        return;
    }
    if (listener) listener->onSuccess();
}

void WifiManager::save(ActionListener* listener) {
    if (!saveConfiguration()) {
        if (listener) listener->onFailure(ActionListener::FAILURE_INTERNAL_ERROR);
        return;
    }
    if (listener) listener->onSuccess();
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

void WifiManager::addWifiStateListener(WifiStateListener* listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mWifiStateListeners.push_back(listener);
}

void WifiManager::removeWifiStateListener(WifiStateListener* listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mWifiStateListeners.erase(std::remove(mWifiStateListeners.begin(),
            mWifiStateListeners.end(), listener), mWifiStateListeners.end());
}

void WifiManager::addScanResultsListener(ScanResultsListener* listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mScanResultsListeners.push_back(listener);
}

void WifiManager::removeScanResultsListener(ScanResultsListener* listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mScanResultsListeners.erase(std::remove(mScanResultsListeners.begin(),
            mScanResultsListeners.end(), listener), mScanResultsListeners.end());
}

void WifiManager::addNetworkStateListener(NetworkStateListener* listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mNetworkStateListeners.push_back(listener);
}

void WifiManager::removeNetworkStateListener(NetworkStateListener* listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mNetworkStateListeners.erase(std::remove(mNetworkStateListeners.begin(),
            mNetworkStateListeners.end(), listener), mNetworkStateListeners.end());
}

void WifiManager::addRssiListener(RssiListener* listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mRssiListeners.push_back(listener);
}

void WifiManager::removeRssiListener(RssiListener* listener) {
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
            } catch (const std::invalid_argument&) {
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
            } catch (const std::invalid_argument&) {
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
    std::vector<WifiStateListener*> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mWifiStateListeners;
    }
    for (WifiStateListener* listener : listeners) listener->onWifiStateChanged(state);
}

void WifiManager::notifyScanResultsListeners() {
    std::vector<ScanResultsListener*> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mScanResultsListeners;
    }
    for (ScanResultsListener* listener : listeners) listener->onScanResultsAvailable();
}

void WifiManager::notifyNetworkStateListeners() {
    WifiInfo info;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        info = mConnectionInfo;
    }
    std::vector<NetworkStateListener*> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mNetworkStateListeners;
    }
    for (NetworkStateListener* listener : listeners) listener->onNetworkStateChanged(info);
}

void WifiManager::notifyRssiListeners(int rssi) {
    std::vector<RssiListener*> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mRssiListeners;
    }
    for (RssiListener* listener : listeners) listener->onRssiChanged(rssi);
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

} // namespace cdroid
