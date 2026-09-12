/* Shared IP application path (AOSP IpClient apply + teardown halves). */
#include <ipapplicator.h>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <ifaddrs.h>
#include <linux/if.h>
#include <map>
#include <mutex>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sstream>
#include <unistd.h>
#include <vector>

#include <linkaddress.h>
#include <netlinkmonitor.h>

#define IPA_LOGE(...) do { fprintf(stderr, "IpApplicator E: " __VA_ARGS__); fprintf(stderr, "\n"); } while (0)

namespace cdroid {

/* resolv.conf is shared by every consumer on the machine, so only the
 * nameserver lines this library itself wrote for an interface are ever
 * removed — per-interface bookkeeping, applied on re-apply and on clear
 * (fixes the old append-only growth on renewals with changing DNS). */
namespace {

std::mutex gDnsMutex;
std::map<std::string, std::vector<std::string>> gAppliedDnsLines;

/* Rewrite resolv.conf with `remove` (whole lines) dropped and `append`
 * (absent) added; empty append just drops. Returns the stored line list. */
std::vector<std::string> rewriteResolvConf(const std::vector<std::string>& remove,
                                           const std::vector<std::string>& append) {
    std::vector<std::string> stored;
    if (access("/etc/resolv.conf", W_OK) != 0) return stored;
    std::ifstream in("/etc/resolv.conf");
    std::stringstream existing;
    if (in.is_open()) existing << in.rdbuf();
    std::string content = existing.str();
    std::string rebuilt;
    size_t pos = 0;
    while (pos <= content.size()) {
        const size_t nl = content.find('\n', pos);
        const std::string line = content.substr(pos, nl == std::string::npos
                ? std::string::npos : nl - pos);
        bool drop = false;
        for (const std::string& r : remove)
            if (line == r) { drop = true; break; }
        if (!drop) {
            rebuilt += line;
            rebuilt += '\n';
        }
        if (nl == std::string::npos) break;
        pos = nl + 1;
    }
    for (const std::string& line : append) {
        if (line.empty()) continue;
        if (rebuilt.find(line) == std::string::npos) {
            rebuilt += line;
            rebuilt += '\n';
        }
        stored.push_back(line);
    }
    std::ofstream out("/etc/resolv.conf", std::ios::trunc);
    if (!out.is_open()) return std::vector<std::string>();
    out << rebuilt;
    return stored;
}

} // namespace

bool applyIpConfiguration(const std::string& iface, const StaticIpConfiguration& config) {
    const int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        IPA_LOGE("socket: %s", strerror(errno));
        return false;
    }
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, iface.c_str(), IFNAMSIZ - 1);

    bool ok = ioctl(fd, SIOCGIFFLAGS, &ifr) == 0;   /* existence check */
    bool applied = true;
    if (ok) {
        ifr.ifr_flags |= IFF_UP;
        if (ioctl(fd, SIOCSIFFLAGS, &ifr) != 0)
            IPA_LOGE("SIOCSIFFLAGS(up) %s: %s (need root?)", iface.c_str(), strerror(errno));

        struct sockaddr_in* sin = reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_addr);
        const LinkAddress& address = config.getIpAddress();
        if (!address.getAddress().empty()) {
            sin->sin_family = AF_INET;
            sin->sin_addr.s_addr = inet_addr(address.getAddress().c_str());
            if (ioctl(fd, SIOCSIFADDR, &ifr) != 0) {
                IPA_LOGE("SIOCSIFADDR %s: %s", address.toString().c_str(), strerror(errno));
                applied = false;
            }
            sin->sin_addr.s_addr = inet_addr(
                    LinkAddress::prefixLengthToNetmaskV4(address.getPrefixLength()).c_str());
            if (ioctl(fd, SIOCSIFNETMASK, &ifr) != 0) {
                IPA_LOGE("SIOCSIFNETMASK: %s", strerror(errno));
                applied = false;
            }
        }
    }
    close(fd);
    if (!ok) return false;

    /* default route via rtnetlink (AOSP netd RouteController path) */
    if (!config.getGateway().empty()) {
        if (!NetlinkMonitor::addDefaultRoute(iface, config.getGateway())) {
            IPA_LOGE("RTM_NEWROUTE gw %s on %s failed", config.getGateway().c_str(),
                     iface.c_str());
            applied = false;
        }
    }

    /* DNS: swap this interface's previously written lines for the new set. */
    {
        std::vector<std::string> newLines;
        for (const std::string& dns : config.getDnsServers())
            newLines.push_back("nameserver " + dns);
        std::lock_guard<std::mutex> lock(gDnsMutex);
        std::vector<std::string> old;
        const auto it = gAppliedDnsLines.find(iface);
        if (it != gAppliedDnsLines.end()) old = it->second;
        if (!newLines.empty() || !old.empty())
            gAppliedDnsLines[iface] = rewriteResolvConf(old, newLines);
        if (gAppliedDnsLines[iface].empty()) gAppliedDnsLines.erase(iface);
    }
    return applied;
}

bool clearIpConfiguration(const std::string& iface) {
    /* Teardown counterpart of apply: drop every IPv4 address, the default
     * route, and the resolv.conf lines written for this interface. */
    bool ok = true;
    const int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd >= 0) {
        struct ifaddrs* ifap = nullptr;
        if (getifaddrs(&ifap) == 0) {
            for (struct ifaddrs* ifa = ifap; ifa; ifa = ifa->ifa_next) {
                if (iface != ifa->ifa_name || !ifa->ifa_addr) continue;
                if (ifa->ifa_addr->sa_family != AF_INET) continue;
                struct ifreq ifr;
                memset(&ifr, 0, sizeof(ifr));
                strncpy(ifr.ifr_name, iface.c_str(), IFNAMSIZ - 1);
                memcpy(&ifr.ifr_addr, ifa->ifa_addr, sizeof(struct sockaddr_in));
                if (ioctl(fd, SIOCDIFADDR, &ifr) != 0) {
                    IPA_LOGE("SIOCDIFADDR %s: %s", iface.c_str(), strerror(errno));
                    ok = false;
                }
            }
            freeifaddrs(ifap);
        } else {
            ok = false;
        }
        close(fd);
    } else {
        ok = false;
    }
    if (!NetlinkMonitor::deleteDefaultRoute(iface)) {
        IPA_LOGE("RTM_DELROUTE default on %s failed", iface.c_str());
        ok = false;
    }
    {
        std::lock_guard<std::mutex> lock(gDnsMutex);
        std::vector<std::string> old;
        const auto it = gAppliedDnsLines.find(iface);
        if (it != gAppliedDnsLines.end()) old = it->second;
        if (!old.empty())
            rewriteResolvConf(old, std::vector<std::string>());   /* drop the lines */
        gAppliedDnsLines.erase(iface);
    }
    return ok;
}

} // namespace cdroid
