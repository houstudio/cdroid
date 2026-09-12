/* Port of android.net.ConnectivityManager (android-36), aggregation form. */
#include <connectivitymanager.h>

#include <ifaddrs.h>

#include <algorithm>

namespace cdroid {

ConnectivityManager& ConnectivityManager::getInstance() {
    static ConnectivityManager instance;
    return instance;
}

ConnectivityManager::ConnectivityManager() {
    /* Bridge both sources into the interim NetworkStateListener surface.
     * Callbacks arrive on the supplicant monitor / ethernet poll threads —
     * same delivery contract as the WifiManager listeners. */
    WifiManager::getInstance().addNetworkStateListener(this);
    EthernetManager::getInstance().addListener(this);
}

ConnectivityManager::~ConnectivityManager() {
    EthernetManager::getInstance().removeListener(this);
    WifiManager::getInstance().removeNetworkStateListener(this);
}

void ConnectivityManager::onNetworkStateChanged(const WifiInfo&) {
    dispatch(buildWifiNetworkInfo());
}

void ConnectivityManager::onAvailabilityChanged(const std::string&, bool) {
    dispatch(buildEthernetNetworkInfo());
}

void ConnectivityManager::dispatch(const NetworkInfo& info) {
    std::vector<NetworkStateListener*> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mListeners;
    }
    for (NetworkStateListener* listener : listeners)
        listener->onNetworkStateChanged(info);
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
    for (const std::string& iface : ethernet.getAvailableInterfaces()) {
        info.setIsAvailable(true);
        /* AOSP promotes LINK_UP to CONNECTED once IpClient finishes; we use
         * the presence of an IPv4 address as the same signal. */
        NetworkInfo::DetailedState::Type detailed = NetworkInfo::DetailedState::DISCONNECTED;
        struct ifaddrs* ifap = nullptr;
        bool hasAddress = false;
        if (getifaddrs(&ifap) == 0) {
            for (struct ifaddrs* ifa = ifap; ifa; ifa = ifa->ifa_next) {
                if (iface != ifa->ifa_name) continue;
                if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) hasAddress = true;
            }
            freeifaddrs(ifap);
        }
        detailed = hasAddress ? NetworkInfo::DetailedState::CONNECTED : NetworkInfo::DetailedState::OBTAINING_IPADDR;
        info.setDetailedState(detailed, std::string(), iface);
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

void ConnectivityManager::addNetworkStateListener(NetworkStateListener* listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mListeners.push_back(listener);
}

void ConnectivityManager::removeNetworkStateListener(NetworkStateListener* listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mListeners.erase(std::remove(mListeners.begin(), mListeners.end(), listener),
                     mListeners.end());
}

} // namespace cdroid
