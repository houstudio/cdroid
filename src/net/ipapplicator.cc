/* Shared IP application path (AOSP IpClient apply half). */
#include <ipapplicator.h>

#include <arpa/inet.h>
#include <errno.h>
#include <fstream>
#include <linux/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sstream>
#include <unistd.h>

#include <linkaddress.h>
#include <netlinkmonitor.h>

#define IPA_LOGE(...) do { fprintf(stderr, "IpApplicator E: " __VA_ARGS__); fprintf(stderr, "\n"); } while (0)

namespace cdroid {

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
    if (ok) {
        ifr.ifr_flags |= IFF_UP;
        if (ioctl(fd, SIOCSIFFLAGS, &ifr) != 0)
            IPA_LOGE("SIOCSIFFLAGS(up) %s: %s (need root?)", iface.c_str(), strerror(errno));

        struct sockaddr_in* sin = reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_addr);
        const LinkAddress& address = config.getIpAddress();
        if (!address.getAddress().empty()) {
            sin->sin_family = AF_INET;
            sin->sin_addr.s_addr = inet_addr(address.getAddress().c_str());
            if (ioctl(fd, SIOCSIFADDR, &ifr) != 0)
                IPA_LOGE("SIOCSIFADDR %s: %s", address.toString().c_str(), strerror(errno));
            sin->sin_addr.s_addr = inet_addr(
                    LinkAddress::prefixLengthToNetmaskV4(address.getPrefixLength()).c_str());
            if (ioctl(fd, SIOCSIFNETMASK, &ifr) != 0)
                IPA_LOGE("SIOCSIFNETMASK: %s", strerror(errno));
        }
    }
    close(fd);
    if (!ok) return false;

    /* default route via rtnetlink (AOSP netd RouteController path) */
    if (!config.getGateway().empty()) {
        if (!NetlinkMonitor::addDefaultRoute(iface, config.getGateway()))
            IPA_LOGE("RTM_NEWROUTE gw %s on %s failed", config.getGateway().c_str(),
                     iface.c_str());
    }

    /* DNS: append nameservers to /etc/resolv.conf when writable (root). */
    if (!config.getDnsServers().empty() && access("/etc/resolv.conf", W_OK) == 0) {
        std::ifstream in("/etc/resolv.conf");
        std::stringstream existing;
        if (in.is_open()) existing << in.rdbuf();
        std::string content = existing.str();
        std::ofstream out("/etc/resolv.conf", std::ios::trunc);
        if (out.is_open()) {
            for (const std::string& dns : config.getDnsServers()) {
                const std::string line = "nameserver " + dns;
                if (content.find(line) == std::string::npos)
                    content += line + "\n";
            }
            out << content;
        }
    }
    return true;
}

} // namespace cdroid
