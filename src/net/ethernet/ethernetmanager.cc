/* Port of android.net.EthernetManager (android-36), ioctl/spawn backed. */
#include <ethernet/ethernetmanager.h>

#include <ipapplicator.h>  /* shared apply path (was the RTM_NEWROUTE seam) */

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>

/* Self-contained module logging, same shim as supplicantclient. */
#define NET_LOGI(...) do { fprintf(stdout, "EthernetManager: " __VA_ARGS__); fprintf(stdout, "\n"); } while (0)
#define NET_LOGE(...) do { fprintf(stderr, "EthernetManager E: " __VA_ARGS__); fprintf(stderr, "\n"); } while (0)

static constexpr int LINK_POLL_INTERVAL_MS = 2000;

namespace cdroid {

EthernetManager& EthernetManager::getInstance() {
    static EthernetManager instance;
    return instance;
}

EthernetManager::EthernetManager() {
    /* platform event source first (rtnetlink on Linux — AOSP netd
     * NetlinkMonitor); the 2s poll survives only as a fallback. */
    mEventMonitor = NetworkEventMonitor::create();
    if (!mEventMonitor || !mEventMonitor->start(this)) {
        delete mEventMonitor;
        mEventMonitor = nullptr;
        mPolling.store(true);
        mPollThread = std::thread(&EthernetManager::pollLoop, this);
    }
    /* monitors deliver no initial state — seed the snapshot once */
    refreshAndNotify();
}

EthernetManager::~EthernetManager() {
    if (mEventMonitor) {
        mEventMonitor->stop();
        delete mEventMonitor;
        mEventMonitor = nullptr;
    }
    if (mPolling.exchange(false)) {
        if (mPollThread.joinable()) mPollThread.join();
    }
    while (true) {
        std::string iface;
        {
            std::lock_guard<std::mutex> lock(mConfigMutex);
            if (mDhcpSessions.empty()) break;
            iface = mDhcpSessions.begin()->first;
        }
        stopDhcp(iface);
    }
}

/* --- enumeration & state (unprivileged) ------------------------------------ */

bool EthernetManager::readInterfaceState(const std::string& iface, InterfaceState& state) {
    struct ifaddrs* ifap = nullptr;
    if (getifaddrs(&ifap) != 0) return false;
    bool found = false;
    for (struct ifaddrs* ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (iface != ifa->ifa_name) continue;
        found = true;
        if (ifa->ifa_flags & IFF_UP) state.up = true;
        if (ifa->ifa_flags & IFF_LOWER_UP) state.lowerUp = true;
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) state.hasAddress = true;
    }
    freeifaddrs(ifap);
    state.exists = found;
    return found;
}

std::vector<std::string> EthernetManager::getAvailableInterfaces() {
    std::vector<std::string> result;
    const std::shared_ptr<const std::regex> pattern = [&]() {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        return mInterfacePattern;
    }();
    struct ifaddrs* ifap = nullptr;
    if (getifaddrs(&ifap) != 0) return result;
    std::map<std::string, bool> up;
    for (struct ifaddrs* ifa = ifap; ifa; ifa = ifa->ifa_next) {
        const std::string name = ifa->ifa_name;
        if (!std::regex_search(name, *pattern)) continue;
        if (up.find(name) == up.end())
            up[name] = (ifa->ifa_flags & (IFF_UP | IFF_LOWER_UP)) == (IFF_UP | IFF_LOWER_UP);
        else if ((ifa->ifa_flags & (IFF_UP | IFF_LOWER_UP)) == (IFF_UP | IFF_LOWER_UP))
            up[name] = true;
    }
    freeifaddrs(ifap);
    for (const auto& entry : up)
        if (entry.second) result.push_back(entry.first);
    return result;
}

bool EthernetManager::isAvailable(const std::string& iface) {
    InterfaceState state;
    return readInterfaceState(iface, state) && state.up && state.lowerUp;
}

/* --- configuration ---------------------------------------------------------- */

IpConfiguration EthernetManager::getConfiguration(const std::string& iface) {
    {
        std::lock_guard<std::mutex> lock(mConfigMutex);
        const auto it = mConfigurations.find(iface);
        if (it != mConfigurations.end()) return it->second;
    }
    loadPersistedConfiguration(iface);
    std::lock_guard<std::mutex> lock(mConfigMutex);
    return mConfigurations[iface];
}

void EthernetManager::setConfiguration(const std::string& iface, const IpConfiguration& config) {
    {
        std::lock_guard<std::mutex> lock(mConfigMutex);
        mConfigurations[iface] = config;
    }
    persistConfiguration(iface, config);
    if (config.getIpAssignment() == IpConfiguration::IpAssignment::DHCP) {
        /* The old static address/route/DNS must not ride along into the
         * DORA window — the real teardown, not an empty apply. */
        clearIpConfiguration(iface);
        startDhcp(iface);
    } else if (config.getIpAssignment() == IpConfiguration::IpAssignment::STATIC) {
        stopDhcp(iface);
        applyStaticConfiguration(iface, config.getStaticIpConfiguration());
    }
}

/* --- enable/disable (root) ---------------------------------------------------- */

static bool setInterfaceFlags(const std::string& iface, bool up) {
    const int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return false;
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, iface.c_str(), IFNAMSIZ - 1);
    bool ok = ioctl(fd, SIOCGIFFLAGS, &ifr) == 0;
    if (ok) {
        if (up) ifr.ifr_flags |= IFF_UP;
        else ifr.ifr_flags &= ~IFF_UP;
        ok = ioctl(fd, SIOCSIFFLAGS, &ifr) == 0;
    }
    close(fd);
    return ok;
}

bool EthernetManager::enableInterface(const std::string& iface) {
    return setInterfaceFlags(iface, true);
}

bool EthernetManager::disableInterface(const std::string& iface) {
    stopDhcp(iface);
    return setInterfaceFlags(iface, false);
}

/* --- static IP application (root) --------------------------------------------- */

void EthernetManager::applyStaticConfiguration(const std::string& iface,
                                               const StaticIpConfiguration& config) {
    /* shared apply path (AOSP IpClient apply half) — also serves the wifi
     * DHCP lease */
    applyIpConfiguration(iface, config);
}

/* --- DHCP (built-in packet-socket client) ------------------------------------- */

void EthernetManager::applyLease(const std::string& iface, const DhcpClient::Lease& lease) {
    StaticIpConfiguration staticIp;
    staticIp.setIpAddress(LinkAddress(lease.ipAddress,
            LinkAddress::netmaskToPrefixLengthV4(lease.netmask)))
            .setGateway(lease.gateway)
            .setDnsServers(lease.dnsServers);
    applyStaticConfiguration(iface, staticIp);
    {
        std::lock_guard<std::mutex> lock(mConfigMutex);
        mLeases[iface] = lease;
    }
    NET_LOGI("lease on %s: %s/%d gw %s dns %zu server %s (%us)",
             iface.c_str(), lease.ipAddress.c_str(),
             LinkAddress::netmaskToPrefixLengthV4(lease.netmask),
             lease.gateway.c_str(), lease.dnsServers.size(),
             lease.serverId.c_str(), lease.leaseDurationSec);
}

DhcpInfo EthernetManager::getDhcpInfo(const std::string& iface) {
    std::lock_guard<std::mutex> lock(mConfigMutex);
    const auto it = mLeases.find(iface);
    return (it == mLeases.end()) ? DhcpInfo() : it->second.toDhcpInfo();
}

bool EthernetManager::startDhcp(const std::string& iface) {
    stopDhcp(iface);
    setInterfaceFlags(iface, true);
    /* built-in client (AOSP DhcpClient port): acquire then renew at T1 —
     * replaces the spawned udhcpc/dhclient system-command dependency. */
    DhcpSession* session = new DhcpSession();
    {
        std::lock_guard<std::mutex> lock(mConfigMutex);
        mDhcpSessions[iface] = session;
    }
    session->thread = std::thread([this, iface, session]() {
        DhcpClient* client = DhcpClient::create(iface);
        DhcpClient::Lease lease;
        if (client->requestLease(lease)) {
            applyLease(iface, lease);
            /* renewal loop: wait T1 (or half the lease, RFC default), renew;
             * on failure fall back to a fresh DISCOVER cycle. */
            while (!session->stop.load()) {
                uint32_t waitSec = lease.t1Sec ? lease.t1Sec : lease.leaseDurationSec / 2;
                if (!waitSec) break;                     /* infinite lease */
                for (uint32_t waited = 0; waited < waitSec && !session->stop.load(); waited++)
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                if (session->stop.load()) break;
                DhcpClient::Lease renewed;
                if (client->renewLease(lease, renewed)) {
                    lease = renewed;
                    applyLease(iface, lease);
                } else if (client->requestLease(lease)) {
                    applyLease(iface, lease);
                } else {
                    NET_LOGE("renewal failed on %s — retrying next cycle", iface.c_str());
                }
            }
        } else {
            NET_LOGE("no lease on %s (timeout/NAK)", iface.c_str());
        }
        delete client;
    });
    NET_LOGI("dhcp session started on %s (built-in client)", iface.c_str());
    return true;
}

void EthernetManager::stopDhcp(const std::string& iface) {
    DhcpSession* session = nullptr;
    {
        std::lock_guard<std::mutex> lock(mConfigMutex);
        const auto it = mDhcpSessions.find(iface);
        if (it == mDhcpSessions.end()) return;
        session = it->second;
        mDhcpSessions.erase(it);
    }
    if (!session) return;
    session->stop.store(true);
    if (session->thread.joinable()) session->thread.join();
    delete session;
}

/* --- listeners -------------------------------------------------------------- */

void EthernetManager::addListener(Listener* listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mListeners.push_back(listener);
}

void EthernetManager::removeListener(Listener* listener) {
    std::lock_guard<std::mutex> lock(mListenersMutex);
    mListeners.erase(std::remove(mListeners.begin(), mListeners.end(), listener),
                     mListeners.end());
}

void EthernetManager::notifyAvailability(const std::string& iface, bool available) {
    std::vector<Listener*> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        listeners = mListeners;
    }
    for (Listener* listener : listeners)
        listener->onAvailabilityChanged(iface, available);
}

void EthernetManager::onLinkStateChanged(const std::string&, bool, bool) {
    refreshAndNotify();
}

void EthernetManager::onAddressChanged(const std::string&, bool, const std::string&) {
    refreshAndNotify();
}

void EthernetManager::refreshAndNotify() {
    const std::shared_ptr<const std::regex> pattern = [&]() {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        return mInterfacePattern;
    }();
    struct ifaddrs* ifap = nullptr;
    if (getifaddrs(&ifap) != 0) return;
    std::map<std::string, InterfaceSnapshot> current;
    for (struct ifaddrs* ifa = ifap; ifa; ifa = ifa->ifa_next) {
        const std::string name = ifa->ifa_name;
        if (!std::regex_search(name, *pattern)) continue;
        InterfaceSnapshot& snapshot = current[name];
        snapshot.available = snapshot.available
                || (ifa->ifa_flags & (IFF_UP | IFF_LOWER_UP)) == (IFF_UP | IFF_LOWER_UP);
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET)
            snapshot.hasAddress = true;
    }
    freeifaddrs(ifap);
    std::map<std::string, bool> changes;
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        for (const auto& entry : current) {
            const auto known = mLastSnapshots.find(entry.first);
            if (known == mLastSnapshots.end()
                    || known->second.available != entry.second.available
                    || known->second.hasAddress != entry.second.hasAddress)
                changes[entry.first] = entry.second.available;
        }
        /* Interfaces absent from the sweep (netdev unregistered — USB dongle
         * unplugged) never appear in `current`: surface them as unavailable
         * too, or listeners keep the stale available=true state (AOSP's
         * EthernetManager.Listener contract requires the false call). */
        for (const auto& known : mLastSnapshots) {
            if (current.find(known.first) == current.end())
                changes[known.first] = false;
        }
        mLastSnapshots = current;
    }
    for (const auto& change : changes)
        notifyAvailability(change.first, change.second);
}

void EthernetManager::pollLoop() {
    while (mPolling.load()) {
        refreshAndNotify();
        for (int waited = 0; waited < LINK_POLL_INTERVAL_MS && mPolling.load(); waited += 100)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

/* --- tuning knobs ----------------------------------------------------------- */

void EthernetManager::setInterfacePattern(const std::string& pattern) {
    /* Compile first: a regex_error leaves the previous pattern in place. */
    try {
        auto compiled = std::make_shared<const std::regex>(pattern);
        std::lock_guard<std::mutex> lock(mListenersMutex);
        mInterfacePattern = compiled;
        mInterfacePatternString = pattern;
    } catch (const std::regex_error& e) {
        NET_LOGE("bad interface pattern '%s': %s", pattern.c_str(), e.what());
    }
}

std::string EthernetManager::getInterfacePattern() {
    /* std::regex has no pattern() accessor pre-C++17; keep the literal for
     * reporting purposes alongside the compiled regex. Returned by value:
     * the member is swapped by setInterfacePattern at runtime. */
    std::lock_guard<std::mutex> lock(mListenersMutex);
    return mInterfacePatternString;
}

void EthernetManager::setConfigurationStoreDir(const std::string& dir) {
    mStoreDir = dir;
}

/* --- persistence --------------------------------------------------------------- */

std::string EthernetManager::parseHexLittleEndianAddress(const std::string& hex) {
    if (hex.size() != 8) return std::string();
    /* /proc/net/route prints the network-order address bytes read as a
     * little-endian word, formatted as big-endian hex: accumulate the hex
     * left-to-right, then split the word LSB-first. */
    unsigned int word = 0;
    for (int i = 0; i < 8; i++) {
        const char c = hex[i];
        int value;
        if (c >= '0' && c <= '9') value = c - '0';
        else if (c >= 'a' && c <= 'f') value = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') value = c - 'A' + 10;
        else return std::string();
        word = (word << 4) | value;
    }
    return std::to_string(word & 0xFF) + "." + std::to_string((word >> 8) & 0xFF)
            + "." + std::to_string((word >> 16) & 0xFF) + "." + std::to_string((word >> 24) & 0xFF);
}

std::string EthernetManager::serializeConfiguration(const IpConfiguration& config) {
    const char* assignmentNames[] = {"STATIC", "DHCP", "UNASSIGNED"};
    const char* proxyNames[] = {"NONE", "STATIC", "UNASSIGNED", "PAC"};
    std::string text;
    text += "ipAssignment=" + std::string(assignmentNames[config.getIpAssignment()]) + "\n";
    text += "proxySettings=" + std::string(proxyNames[config.getProxySettings()]) + "\n";
    const StaticIpConfiguration& staticIp = config.getStaticIpConfiguration();
    if (config.getIpAssignment() == IpConfiguration::IpAssignment::STATIC) {
        if (!staticIp.getIpAddress().getAddress().empty())
            text += "address=" + staticIp.getIpAddress().toString() + "\n";
        if (!staticIp.getGateway().empty())
            text += "gateway=" + staticIp.getGateway() + "\n";
        for (const std::string& dns : staticIp.getDnsServers())
            text += "dns=" + dns + "\n";
        if (!staticIp.getDomains().empty())
            text += "domains=" + staticIp.getDomains() + "\n";
    }
    return text;
}

bool EthernetManager::parseConfiguration(const std::string& text, IpConfiguration& config) {
    const char* assignmentNames[] = {"STATIC", "DHCP", "UNASSIGNED"};
    const char* proxyNames[] = {"NONE", "STATIC", "UNASSIGNED", "PAC"};
    StaticIpConfiguration staticIp;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        const size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        const std::string key = line.substr(0, eq);
        const std::string value = line.substr(eq + 1);
        if (key == "ipAssignment") {
            for (int i = 0; i < 3; i++)
                if (value == assignmentNames[i])
                    config.setIpAssignment((IpConfiguration::IpAssignment::Type) i);
        } else if (key == "proxySettings") {
            for (int i = 0; i < 4; i++)
                if (value == proxyNames[i])
                    config.setProxySettings((IpConfiguration::ProxySettings::Type) i);
        } else if (key == "address") {
            staticIp.setIpAddress(LinkAddress(value));
        } else if (key == "gateway") {
            staticIp.setGateway(value);
        } else if (key == "dns") {
            staticIp.addDnsServer(value);
        } else if (key == "domains") {
            staticIp.setDomains(value);
        }
    }
    config.setStaticIpConfiguration(staticIp);
    return true;
}

void EthernetManager::loadPersistedConfiguration(const std::string& iface) {
    if (mStoreDir.empty()) return;
    const std::string path = mStoreDir + "/" + iface + ".conf";
    std::ifstream in(path);
    if (!in.is_open()) return;
    std::stringstream buffer;
    buffer << in.rdbuf();
    IpConfiguration config;
    parseConfiguration(buffer.str(), config);
    std::lock_guard<std::mutex> lock(mConfigMutex);
    mConfigurations[iface] = config;
}

void EthernetManager::persistConfiguration(const std::string& iface,
                                           const IpConfiguration& config) {
    if (mStoreDir.empty()) return;
    const std::string path = mStoreDir + "/" + iface + ".conf";
    std::ofstream out(path, std::ios::trunc);
    if (!out.is_open()) {
        NET_LOGI("config store %s not writable; runtime-only", path.c_str());
        return;
    }
    out << serializeConfiguration(config);
}

} // namespace cdroid
