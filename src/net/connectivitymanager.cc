/* Port of android.net.ConnectivityManager (android-36), aggregation form. */
#include <connectivitymanager.h>

#include <ifaddrs.h>
#include <linux/if.h>   /* IFF_LOWER_UP */
#include <linux/sockios.h>  /* SIOCBRADDBR/SIOCBRDELBR */
#include <regex>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

#include <dhcpserver.h>
#include <ipapplicator.h>
#include <linkaddress.h>
#include <natcontroller.h>
#include <staticipconfiguration.h>

#include <algorithm>
#include <cstdio>

namespace cdroid {

/* The Tethering<->PanService binder seam (see the header): the app layer
 * registers cdblue's BluetoothPan::setBluetoothTethering here. */
std::function<bool(bool)> ConnectivityManager::sBluetoothPanEnabler = nullptr;

/* ODR: pointer constants bound through std::string/pair initializers need
 * the out-of-line definition (the (int)-cast trick only covers ints). */
constexpr const char* ConnectivityManager::BT_TETHERING_BRIDGE;

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

/* --- TETHERING_BLUETOOTH data plane (the netd half) ------------------- */

/* AOSP netd entrusts the bridge to the kernel and the addressing to
 * Tethering's static IP logic (config_tether_bluetooth_ranges =
 * 192.168.44.0/24, frameworks/base core/res config.xml); the standalone
 * module does the same steps directly: SIOCBRADDBR + the shared
 * bring-up/addressing helpers. */
static bool ensureBtPanBridge() {
    const char* bridge = ConnectivityManager::BT_TETHERING_BRIDGE;
    if (access(("/sys/class/net/" + std::string(bridge)).c_str(), F_OK) != 0) {
        const int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) return false;
        ifreq ifr = {};
        strncpy(ifr.ifr_name, bridge, IFNAMSIZ - 1);
        const int rc = ioctl(sock, SIOCBRADDBR, &ifr);
        close(sock);
        if (rc != 0 && errno != EEXIST) {
            fprintf(stderr, "ConnectivityManager E: SIOCBRADDBR %s: %s\n",
                    bridge, strerror(errno));
            return false;
        }
    }
    if (!bringInterfaceUp(bridge, true)) return false;
    StaticIpConfiguration ip;
    ip.setIpAddress(LinkAddress("192.168.44.1", 24));
    return applyIpConfiguration(bridge, ip);
}

/* Dropping the bridge also kicks the enslaved bnep peers — exactly what
 * netd's interface teardown achieves (peers see the NAP go away). */
static void destroyBtPanBridge() {
    const char* bridge = ConnectivityManager::BT_TETHERING_BRIDGE;
    const int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return;
    ifreq ifr = {};
    strncpy(ifr.ifr_name, bridge, IFNAMSIZ - 1);
    ioctl(sock, SIOCBRDELBR, &ifr);
    close(sock);
}

void ConnectivityManager::setBluetoothPanEnabler(
        const std::function<bool(bool enabled)>& enabler) {
    sBluetoothPanEnabler = enabler;
}

bool ConnectivityManager::startTethering(int type) {
    if (type == TETHERING_WIFI) {
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
        std::lock_guard<std::mutex> lock(mNatMutex);
        mNatPairs[type] = {internal, external};
        return true;
    }
    if (type == TETHERING_BLUETOOTH) {
        /* AOSP Tethering.startTethering(BLUETOOTH): enable the BNEP server
         * (PanService half) and tether the bt-pan interface (netd half).
         * Refusal of the PanService half fails the request, as upstream. */
        if (!ensureBtPanBridge()) return false;
        if (sBluetoothPanEnabler) {
            if (!sBluetoothPanEnabler(true)) {
                fprintf(stderr, "ConnectivityManager E: bluetooth pan enabler "
                        "refused tethering\n");
                destroyBtPanBridge();
                return false;
            }
        } else {
            fprintf(stderr, "ConnectivityManager W: no bluetooth pan enabler "
                    "registered (setBluetoothPanEnabler) — data plane up, "
                    "BNEP server NOT enabled\n");
        }
        /* netd starts dnsmasq per tethered interface; the standalone module
         * uses its own DHCP server with the same address shape. */
        mkdir("/tmp/cdroid-tether", 0755);   /* pid/log dir; EEXIST is fine */
        DhcpServer::Config dhcp;
        dhcp.iface = BT_TETHERING_BRIDGE;
        dhcp.serverIp = "192.168.44.1";
        dhcp.prefixLength = 24;
        dhcp.rangeStart = "192.168.44.2";
        dhcp.rangeEnd = "192.168.44.254";
        dhcp.netmask = "255.255.255.0";
        dhcp.workDir = "/tmp/cdroid-tether";
        DhcpServer* server = DhcpServer::create(dhcp);
        if (server == nullptr || !server->start()) {
            delete server;
            destroyBtPanBridge();
            if (sBluetoothPanEnabler) sBluetoothPanEnabler(false);
            return false;
        }
        mBtDhcpServer = server;

        const std::string external = tetheringUpstreamIface();
        if (external.empty()) {
            fprintf(stderr, "ConnectivityManager E: no upstream interface for "
                    "tethering (bridge stays up, no NAT)\n");
            return true;
        }
        if (!NatController::enableNat(BT_TETHERING_BRIDGE, external)) {
            server->stop();
            delete server;
            mBtDhcpServer = nullptr;
            destroyBtPanBridge();
            if (sBluetoothPanEnabler) sBluetoothPanEnabler(false);
            return false;
        }
        std::lock_guard<std::mutex> lock(mNatMutex);
        mNatPairs[type] = {BT_TETHERING_BRIDGE, external};
        return true;
    }
    fprintf(stderr, "ConnectivityManager E: tethering type %d has no "
            "interface owner yet\n", type);
    return false;
}

bool ConnectivityManager::stopTethering(int type) {
    if (type == TETHERING_WIFI) {
        teardownRecordedNat(type);
        return WifiManager::getInstance().stopSoftAp();
    }
    if (type == TETHERING_BLUETOOTH) {
        teardownRecordedNat(type);
        if (mBtDhcpServer != nullptr) {
            mBtDhcpServer->stop();
            delete mBtDhcpServer;
            mBtDhcpServer = nullptr;
        }
        if (sBluetoothPanEnabler) sBluetoothPanEnabler(false);
        destroyBtPanBridge();
        return true;
    }
    return false;
}

void ConnectivityManager::teardownRecordedNat(int type) {
    std::string internal, external;
    {
        std::lock_guard<std::mutex> lock(mNatMutex);
        const auto it = mNatPairs.find(type);
        if (it == mNatPairs.end()) return;
        internal = it->second.first;
        external = it->second.second;
        mNatPairs.erase(it);
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
        teardownRecordedNat(TETHERING_WIFI);
}

} // namespace cdroid
