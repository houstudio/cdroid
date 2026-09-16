/* Port of android.net.ConnectivityManager (android-36), aggregation form. */
#include <connectivitymanager.h>

#include <ifaddrs.h>
#include <linux/if.h>   /* IFF_LOWER_UP */
#include <regex>

#include <natcontroller.h>

#include <algorithm>
#include <cstdio>

namespace cdroid {

ConnectivityManager& ConnectivityManager::getInstance() {
    static ConnectivityManager instance;
    return instance;
}

ConnectivityManager::ConnectivityManager() {
    /* Bridge both sources into the interim NetworkStateListener surface
     * (value-semantics slots: lambdas registered as copies). Callbacks
     * arrive on the supplicant monitor / ethernet poll threads — same
     * delivery contract as the WifiManager listeners. */
    mWifiNetworkListener = [this](const WifiInfo& info) {
        onNetworkStateChanged(info);
    };
    mEthernetListener = [this](const std::string& iface, bool isAvailable) {
        onAvailabilityChanged(iface, isAvailable);
    };
    /* AP state: any path that brings the Soft AP down (stopSoftAp, hostapd
     * death, the idle-shutdown worker) must also drop the NAT — netd's
     * IpServer does this on its tethering teardown. Sticky registration
     * immediately reports the current state; the DISABLED callback is a
     * no-op while no NAT pair is recorded. */
    mWifiApListener = [this](int wifiApState) {
        onWifiApStateChanged(wifiApState);
    };
    WifiManager::getInstance().addNetworkStateListener(mWifiNetworkListener);
    EthernetManager::getInstance().addListener(mEthernetListener);
    WifiManager::getInstance().addWifiApStateListener(mWifiApListener);
}

ConnectivityManager::~ConnectivityManager() {
    /* Tethering ownership mirrors the Soft AP's (module phase: this process
     * is the service): leaving without stopTethering would strand kernel NAT
     * rules past the AP they served. Reverse-construction order guarantees
     * WifiManager outlives this teardown. Detach the AP listener FIRST so
     * the teardown below cannot re-enter the callback. */
    WifiManager::getInstance().removeWifiApStateListener(mWifiApListener);
    if (WifiManager::getInstance().getWifiApState()
            != WifiManager::WIFI_AP_STATE_DISABLED)
        stopTethering(TETHERING_WIFI);
    EthernetManager::getInstance().removeListener(mEthernetListener);
    WifiManager::getInstance().removeNetworkStateListener(mWifiNetworkListener);
}

void ConnectivityManager::onNetworkStateChanged(const WifiInfo&) {
    dispatch(buildWifiNetworkInfo());
}

void ConnectivityManager::onAvailabilityChanged(const std::string&, bool) {
    dispatch(buildEthernetNetworkInfo());
}

void ConnectivityManager::dispatch(const NetworkInfo& info) {
    std::vector<NetworkStateListener> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mListeners;
    }
    /* Writable copies: CallbackBase::operator() is non-const. */
    for (NetworkStateListener listener : listeners) listener(info);
}

NetworkInfo ConnectivityManager::buildWifiNetworkInfo() {
    NetworkInfo info(TYPE_WIFI);
    const WifiInfo wifiInfo = WifiManager::getInstance().getConnectionInfo();
    const SupplicantState::State suppState = wifiInfo.getSupplicantState();
    if (!SupplicantState::isValidState(suppState)) {
        info.setIsAvailable(false);
        info.setDetailedState(NetworkInfo::DetailedState::IDLE, std::string(), std::string());
        return info;
    }
    info.setIsAvailable(true);
    /* AOSP maps COMPLETED to OBTAINING_IPADDR here and lets the IpClient
     * callback promote it to CONNECTED; we promote locally when an address
     * is present (no IpClient in the module phase). */
    NetworkInfo::DetailedState::Type detailed = WifiInfo::getDetailedStateOf(suppState);
    if (detailed == NetworkInfo::DetailedState::OBTAINING_IPADDR && wifiInfo.getIpAddress() != 0)
        detailed = NetworkInfo::DetailedState::CONNECTED;
    info.setDetailedState(detailed, std::string(), wifiInfo.getSSID());
    return info;
}

NetworkInfo ConnectivityManager::buildEthernetNetworkInfo() {
    NetworkInfo info(TYPE_ETHERNET);
    EthernetManager& ethernet = EthernetManager::getInstance();
    /* One getifaddrs sweep answers both questions (pattern-match available
     * + IPv4 presence) — the old shape ran one sweep per available
     * interface and discarded all but the first. */
    const std::string pattern = ethernet.getInterfacePattern();
    const std::regex patternRegex(pattern);
    struct ifaddrs* ifap = nullptr;
    std::string firstIface;
    bool firstHasAddress = false;
    if (getifaddrs(&ifap) == 0) {
        for (struct ifaddrs* ifa = ifap; ifa; ifa = ifa->ifa_next) {
            if (!std::regex_search(std::string(ifa->ifa_name), patternRegex)) continue;
            if ((ifa->ifa_flags & (IFF_UP | IFF_LOWER_UP)) != (IFF_UP | IFF_LOWER_UP)) continue;
            if (firstIface.empty()) firstIface = ifa->ifa_name;
            if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET
                    && ifa->ifa_name == firstIface)
                firstHasAddress = true;
        }
        freeifaddrs(ifap);
    }
    if (!firstIface.empty()) {
        info.setIsAvailable(true);
        /* AOSP promotes LINK_UP to CONNECTED once IpClient finishes; we use
         * the presence of an IPv4 address as the same signal. */
        info.setDetailedState(firstHasAddress
                ? NetworkInfo::DetailedState::CONNECTED
                : NetworkInfo::DetailedState::OBTAINING_IPADDR, std::string(), firstIface);
        return info;
    }
    info.setIsAvailable(false);
    info.setDetailedState(NetworkInfo::DetailedState::IDLE, std::string(), std::string());
    return info;
}

NetworkInfo ConnectivityManager::getActiveNetworkInfo() {
    /* preference: ETHERNET > WIFI (AOSP: per-network scores). */
    NetworkInfo ethernet = buildEthernetNetworkInfo();
    if (ethernet.isConnected()) return ethernet;
    NetworkInfo wifi = buildWifiNetworkInfo();
    if (wifi.isConnected()) return wifi;
    /* AOSP returns null; the no-optional convention yields a typed offline
     * NetworkInfo (TYPE_NONE, DISCONNECTED, unavailable). */
    NetworkInfo none(TYPE_NONE);
    none.setIsAvailable(false);
    none.setDetailedState(NetworkInfo::DetailedState::DISCONNECTED, std::string(), std::string());
    return none;
}

NetworkInfo ConnectivityManager::getNetworkInfo(int networkType) {
    switch (networkType) {
    case TYPE_WIFI:     return buildWifiNetworkInfo();
    case TYPE_ETHERNET: return buildEthernetNetworkInfo();
    default: {
        NetworkInfo info(networkType);
        info.setIsAvailable(false);
        info.setDetailedState(NetworkInfo::DetailedState::IDLE, std::string(), std::string());
        return info;
    }
    }
}

std::vector<NetworkInfo> ConnectivityManager::getAllNetworkInfo() {
    std::vector<NetworkInfo> all;
    all.push_back(buildWifiNetworkInfo());
    all.push_back(buildEthernetNetworkInfo());
    return all;
}

void ConnectivityManager::addNetworkStateListener(const NetworkStateListener& listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mListeners.push_back(listener);
}

void ConnectivityManager::removeNetworkStateListener(const NetworkStateListener& listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mListeners.erase(std::remove(mListeners.begin(), mListeners.end(), listener),
                     mListeners.end());
}

/* --- tethering --------------------------------------------------------------- */

std::string ConnectivityManager::tetheringUpstreamIface() {
    /* AOSP tethers toward the default network: the default-route owner. */
    const std::string route = NatController::defaultRouteInterface();
    if (!route.empty()) return route;
    /* No default route yet: first link-up ethernet port (the aggregation
     * preference order — ETHERNET > WIFI — applied to tethering). */
    for (const std::string& iface : EthernetManager::getInstance().getAvailableInterfaces())
        return iface;
    return std::string();
}

bool ConnectivityManager::startTethering(int type) {
    if (type != TETHERING_WIFI) {
        fprintf(stderr, "ConnectivityManager E: tethering type %d has no "
                "interface owner yet (wifi only)\n", type);
        return false;
    }
    WifiManager& wifi = WifiManager::getInstance();
    if (!wifi.startTetheredHotspot(nullptr)) return false;
    const std::string internal = wifi.getSoftApInterfaceName();
    const std::string external = tetheringUpstreamIface();
    if (external.empty()) {
        fprintf(stderr, "ConnectivityManager E: no upstream interface for "
                "tethering (AP stays up, no NAT)\n");
        return true;
    }
    if (!NatController::enableNat(internal, external)) {
        wifi.stopSoftAp();
        return false;
    }
    /* Record the pair that was actually programmed — netd's enabled-iface
     * pair ledger: stop must remove exactly these rules, not whatever the
     * default route resolves to at stop time. */
    {
        std::lock_guard<std::mutex> lock(mNatMutex);
        mNatInternal = internal;
        mNatExternal = external;
    }
    return true;
}

bool ConnectivityManager::stopTethering(int type) {
    if (type != TETHERING_WIFI) return false;
    teardownRecordedNat();
    return WifiManager::getInstance().stopSoftAp();
}

void ConnectivityManager::teardownRecordedNat() {
    std::string internal, external;
    {
        std::lock_guard<std::mutex> lock(mNatMutex);
        internal = mNatInternal;
        external = mNatExternal;
        mNatInternal.clear();
        mNatExternal.clear();
    }
    if (!internal.empty() && !external.empty())
        NatController::disableNat(internal, external);
}

void ConnectivityManager::onWifiApStateChanged(int wifiApState) {
    /* The AP the NAT served is gone by any road (explicit stop — which
     * already tore the NAT down and left the record empty — idle shutdown,
     * hostapd death): drop the recorded pair so its rules cannot outlive
     * the interface they forward for. */
    if (wifiApState == WifiManager::WIFI_AP_STATE_DISABLED)
        teardownRecordedNat();
}

} // namespace cdroid
