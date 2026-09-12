/* Port of android.net.wifi.WifiManager (android-36), wpa_ctrl backed. */
#include <wifi/wifimanager.h>

#include <ifaddrs.h>
#include <netinet/in.h>

#include <ipapplicator.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>

#include <wifi/wparesponseparser.h>
#include <linkaddress.h>

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
    /* "/var/run/wpa_supplicant/wlan0" -> "wlan0" for the radio dump. */
    const size_t slash = ctrlPath.find_last_of('/');
    mIfaceName = (slash == std::string::npos) ? ctrlPath : ctrlPath.substr(slash + 1);
    const bool ok = mClient.connect();
    if (ok) seedDhcpFromCurrentState();
    return ok;
}

std::string WifiManager::interfaceName() const {
    return mIfaceName;
}

/* --- Wi-Fi state ----------------------------------------------------------- */

int WifiManager::getWifiState() {
    if (!mClient.isConnected()) return WIFI_STATE_UNKNOWN;
    /* The ctrl iface has no radio state: map the supplicant's view. Radio
     * control (rfkill / interface up-down) needs a platform hook:
     * TODO(porting). */
    const std::string status = mClient.request("STATUS");
    if (status.empty()) return WIFI_STATE_UNKNOWN;
    if (status.find("wpa_state=INTERFACE_DISABLED") != std::string::npos)
        return WIFI_STATE_DISABLED;
    return WIFI_STATE_ENABLED;
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
    /* AOSP pulls scans from the radio HAL (wificond), not the supplicant:
     * the nl80211 dump is that source — IEs / TSF / channel width — with
     * the ctrl_iface SCAN_RESULTS list as fallback. */
    if (mRadioData) {
        const std::vector<ScanResult> results = mRadioData->getScanResults(interfaceName());
        if (!results.empty()) return results;
    }
    return WpaResponseParser::parseScanResults(mClient.request("SCAN_RESULTS"));
}

/* --- connection info ------------------------------------------------------------ */

WifiInfo WifiManager::getConnectionInfo() {
    WifiInfo info;
    const std::string status = mClient.request("STATUS");
    if (status.empty()) return info;
    WpaResponseParser::applyStatus(info, status);
    WpaResponseParser::applySignalPoll(info, mClient.request("SIGNAL_POLL"));
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        mConnectionInfo = info;
        mLastRssi.store(info.getRssi());
    }
    return info;
}

/* --- configured networks --------------------------------------------------------- */

std::vector<WifiConfiguration> WifiManager::getConfiguredNetworks() {
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
    return configs;
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
    return requestOk("REMOVE_NETWORK " + std::to_string(netId));
}

bool WifiManager::enableNetwork(int netId, bool attemptConnect) {
    /* attemptConnect maps to SELECT_NETWORK: it re-enables the network and
     * disables all others (wpa's counterpart of the framework connect flow). */
    if (attemptConnect)
        return requestOk("SELECT_NETWORK " + std::to_string(netId));
    return requestOk("ENABLE_NETWORK " + std::to_string(netId));
}

bool WifiManager::disableNetwork(int netId) {
    return requestOk("DISABLE_NETWORK " + std::to_string(netId));
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

void WifiManager::onSupplicantDisconnected() {
    setWifiStateAndNotify(WIFI_STATE_UNKNOWN);
}

void WifiManager::onSupplicantReconnected() {
    updateConnectionInfoFromStatus();
    setWifiStateAndNotify(WIFI_STATE_ENABLED);
    seedDhcpFromCurrentState();
}

void WifiManager::onSupplicantEvent(const SupplicantEvent& event) {
    if (event.name == "CTRL-EVENT-SCAN-RESULTS") {
        std::vector<ScanResultsListener*> listeners;
        {
            std::lock_guard<std::mutex> lock(mListenersMutex);
            listeners = mScanResultsListeners;
        }
        for (ScanResultsListener* listener : listeners) listener->onScanResultsAvailable();
    } else if (event.name == "CTRL-EVENT-STATE-CHANGE") {
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
        if (event.getArg("SSID", value)) info.setSSID(WifiSsid::fromString(value));
        {
            std::lock_guard<std::mutex> lock(mStateMutex);
            mConnectionInfo = info;
        }
        std::vector<NetworkStateListener*> listeners;
        {
            std::lock_guard<std::mutex> lock(mListenersMutex);
            listeners = mNetworkStateListeners;
        }
        for (NetworkStateListener* listener : listeners) listener->onNetworkStateChanged(info);
    } else if (event.name == "CTRL-EVENT-CONNECTED") {
        updateConnectionInfoFromStatus();
        startRssiPolling();
        /* IP provisioning begins with L2 completion (AOSP IpClient) */
        startDhcpIfNeeded();
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
    } else if (event.name == "CTRL-EVENT-DISCONNECTED") {
        stopRssiPolling();
        stopDhcpAndRelease();
        updateConnectionInfoFromStatus();
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
    std::vector<WifiStateListener*> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mWifiStateListeners;
    }
    for (WifiStateListener* listener : listeners) listener->onWifiStateChanged(newState);
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
    if (info.getIpAddress() == 0 && !mIfaceName.empty()) {
        struct ifaddrs* ifap = nullptr;
        if (getifaddrs(&ifap) == 0) {
            for (struct ifaddrs* ifa = ifap; ifa; ifa = ifa->ifa_next) {
                if (mIfaceName != ifa->ifa_name || !ifa->ifa_addr) continue;
                if (ifa->ifa_addr->sa_family != AF_INET) continue;
                char buf[INET_ADDRSTRLEN] = {0};
                const auto* sin = reinterpret_cast<const struct sockaddr_in*>(ifa->ifa_addr);
                if (inet_ntop(AF_INET, &sin->sin_addr, buf, sizeof(buf)))
                    info.setInetAddress(buf);
            }
            freeifaddrs(ifap);
        }
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
        std::vector<RssiListener*> listeners;
        {
            std::lock_guard<std::mutex> lock(mListenersMutex);
            listeners = mRssiListeners;
        }
        for (RssiListener* listener : listeners) listener->onRssiChanged(rssi);
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
    if (mIfaceName.empty()) return;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        if (mDhcpSession) return;   /* already provisioning/renewing */
    }
    WifiDhcpSession* session = new WifiDhcpSession();
    mDhcpSession = session;
    session->thread = std::thread([this, session]() {
        DhcpClient* client = DhcpClient::create(mIfaceName);
        DhcpClient::Lease lease;
        fprintf(stdout, "WifiManager: dhcp request on %s\n", mIfaceName.c_str());
        fflush(stdout);
        if (client->requestLease(lease)) {
            fprintf(stdout, "WifiManager: lease on %s: %s gw %s (%us)\n", mIfaceName.c_str(),
                    lease.ipAddress.c_str(), lease.gateway.c_str(), lease.leaseDurationSec);
            fflush(stdout);
            StaticIpConfiguration staticIp;
            staticIp.setIpAddress(LinkAddress(lease.ipAddress,
                    LinkAddress::netmaskToPrefixLengthV4(lease.netmask)))
                    .setGateway(lease.gateway)
                    .setDnsServers(lease.dnsServers);
            applyIpConfiguration(mIfaceName, staticIp);
            {
                std::lock_guard<std::mutex> lock(mStateMutex);
                mLease = lease;
            }
            /* renewal loop mirrors the ethernet session: T1 (or half the
             * lease), renew, fall back to a fresh DISCOVER on failure. */
            while (!session->stop.load()) {
                uint32_t waitSec = lease.t1Sec ? lease.t1Sec : lease.leaseDurationSec / 2;
                if (!waitSec) break;
                for (uint32_t waited = 0; waited < waitSec && !session->stop.load(); waited++)
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                if (session->stop.load()) break;
                DhcpClient::Lease renewed;
                if (client->renewLease(lease, renewed)) lease = renewed;
                else if (!client->requestLease(lease)) continue;
                StaticIpConfiguration renewedIp;
                renewedIp.setIpAddress(LinkAddress(lease.ipAddress,
                        LinkAddress::netmaskToPrefixLengthV4(lease.netmask)))
                        .setGateway(lease.gateway)
                        .setDnsServers(lease.dnsServers);
                applyIpConfiguration(mIfaceName, renewedIp);
                {
                    std::lock_guard<std::mutex> lock(mStateMutex);
                    mLease = lease;
                }
            }
        }
        fprintf(stderr, "WifiManager E: no lease on %s (timeout/NAK)\n", mIfaceName.c_str());
        delete client;
    });
}

void WifiManager::stopDhcpAndRelease() {
    WifiDhcpSession* session = mDhcpSession;
    if (!session) return;
    mDhcpSession = nullptr;
    session->stop.store(true);
    if (session->thread.joinable()) session->thread.join();
    delete session;
    DhcpClient::Lease lease;
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        lease = mLease;
        mLease = DhcpClient::Lease();
    }
    if (lease.bound && !mIfaceName.empty()) {
        DhcpClient* client = DhcpClient::create(mIfaceName);
        client->releaseLease(lease);   /* best effort */
        delete client;
    }
}

} // namespace cdroid
