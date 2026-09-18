#ifndef __DHCP_SERVER_H__
#define __DHCP_SERVER_H__

#include <sys/types.h>

#include <string>

namespace cdroid {

/**
 * Soft AP DHCP server: the serving half next to DhcpClient (android.net's
 * IpServer runs a DHCP server on the AP interface; AOSP's own is the
 * frameworks DhcpServer+dnsmasq or the Java server — this port drives the
 * classic dnsmasq daemon, like the bench and most embedded targets).
 *
 * start(): bring the interface up, own the server address
 * (applyIpConfiguration), then spawn
 *   dnsmasq --interface=<iface> --bind-interfaces --dhcp-range=...<lease>
 * and wait for its pid file. stop(): SIGTERM (SIGKILL after 3 s).
 *
 * Needs CAP_NET_ADMIN (address) and the port-67 free on that interface —
 * a second dnsmasq bound to the same interface makes the spawn fail fast.
 */
class DhcpServer {
public:
    struct Config {
        std::string iface;
        /* AOSP Tethering WIFI_AP static subnet shape (192.168.49.1/24;
         * the pre-R hotspot was 192.168.43.1). TODO(porting): verify
         * against packages/modules/Connectivity when that tree lands. */
        std::string serverIp = "192.168.49.1";
        int prefixLength = 24;
        std::string rangeStart = "192.168.49.2";
        std::string rangeEnd = "192.168.49.254";
        std::string netmask = "255.255.255.0";
        /* 12 h, the bench pool's lease time. */
        int leaseTimeSec = 43200;
        /* Working directory for pid/log files (the SoftAp config dir). */
        std::string workDir;
    };

    static DhcpServer* create(const Config& config);
    ~DhcpServer();

    bool start();
    void stop();
    bool isRunning() const;

private:
    explicit DhcpServer(const Config& config);
    bool spawnDaemon();
    std::string pidFile() const;
    std::string logFile() const;
    bool readPid(pid_t* pid) const;

    Config mConfig;
    bool mStarted = false;
};

} // namespace cdroid

#endif /* __DHCP_SERVER_H__ */
