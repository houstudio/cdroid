#ifndef __ETHERNET_MANAGER_H__
#define __ETHERNET_MANAGER_H__

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <regex>
#include <string>
#include <thread>
#include <vector>

#include <networkeventmonitor.h>
#include <dhcpclient.h>
#include <ipconfiguration.h>

namespace cdroid {

/**
 * Port of android.net.EthernetManager (android-36). AOSP talks to
 * EthernetService over binder and applies configurations through netd's
 * IpClient; in CDROID's single-process world this class owns the interface
 * directly:
 *   - enumeration/link state: getifaddrs
 *   - static IP: ioctl(SIOCSIFADDR/SIOCSIFNETMASK/SIOCSIFFLAGS) +
 *     RTM_NEWROUTE default route + /etc/resolv.conf (all need root)
 *   - DHCP: spawns udhcpc (busybox) or dhclient, whichever is on PATH
 *   - availability callbacks: NetworkEventMonitor push (rtnetlink on
 *     Linux, iphlpapi on Windows later); 2s poll as fallback
 *
 * Configurations persist per-interface as key=value files under
 * getConfigurationStoreDir() (default /var/lib/cdnet), best-effort.
 *
 * AOSP's async updateConfiguration/enableInterface OutcomeListener forms are
 * collapsed to synchronous bools for the module phase (documented
 * divergence; the listener forms come back with the broadcast era).
 */
class EthernetManager : private NetworkEventMonitor::Events {
public:
    /* InterfaceState values. */
    static constexpr int STATE_ABSENT   = 0;
    static constexpr int STATE_LINK_DOWN = 1;
    static constexpr int STATE_LINK_UP   = 2;

    /** AOSP EthernetManager.Listener. */
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void onAvailabilityChanged(const std::string& iface, bool isAvailable) = 0;
    };

    static EthernetManager& getInstance();

    /* Interfaces matching getInterfacePattern() with link up. */
    std::vector<std::string> getAvailableInterfaces();
    bool isAvailable(const std::string& iface);

    IpConfiguration getConfiguration(const std::string& iface);
    void setConfiguration(const std::string& iface, const IpConfiguration& config);
    /* Last lease acquired on `iface` by the built-in DHCP client (zeroed
     * when none). Not an AOSP surface (IpClient delivers DhcpResults
     * internally); exposed for the module phase. */
    DhcpInfo getDhcpInfo(const std::string& iface);

    bool enableInterface(const std::string& iface);
    bool disableInterface(const std::string& iface);

    void addListener(Listener* listener);
    void removeListener(Listener* listener);

    /* Interface-name filter, AOSP config_ethernet_iface_regex (default
     * "eth\d"). Must be set before the poll thread observes anything. */
    void setInterfacePattern(const std::string& pattern);
    std::string getInterfacePattern();
    /* Where per-iface configs persist ("" disables persistence). */
    void setConfigurationStoreDir(const std::string& dir);

    /* "/proc/net/route" hex columns are little-endian; convert to dotted. */
    static std::string parseHexLittleEndianAddress(const std::string& hex);

    /* Pure serialization for the per-iface config files (unit-testable). */
    static std::string serializeConfiguration(const IpConfiguration& config);
    static bool parseConfiguration(const std::string& text, IpConfiguration& config);

private:
    EthernetManager();
    ~EthernetManager();
    EthernetManager(const EthernetManager&) = delete;
    EthernetManager& operator=(const EthernetManager&) = delete;

    struct InterfaceState {
        bool exists = false;
        bool up = false;
        bool lowerUp = false;
        bool hasAddress = false;
    };
    static bool readInterfaceState(const std::string& iface, InterfaceState& state);
    /* getifaddrs sweep + dedup vs mLastSnapshots + Listener dispatch; runs
     * from the netlink thread on events and (fallback only) from the poll
     * thread when rtnetlink is unavailable. */
    void refreshAndNotify();
    void pollLoop();
    void notifyAvailability(const std::string& iface, bool available);

    /* NetlinkMonitor::EventCallback: rtnetlink pushes carry authoritative
     * flags, but a fresh sweep keeps one shared code path with the poll
     * fallback and dedups through mLastSnapshots. */
    void onLinkStateChanged(const std::string& iface, bool up, bool lowerUp) override;
    void onAddressChanged(const std::string& iface, bool added,
                          const std::string& address) override;
    void applyStaticConfiguration(const std::string& iface, const StaticIpConfiguration& config);
    void applyLease(const std::string& iface, const DhcpClient::Lease& lease);
    bool startDhcp(const std::string& iface);
    void stopDhcp(const std::string& iface);
    void loadPersistedConfiguration(const std::string& iface);
    void persistConfiguration(const std::string& iface, const IpConfiguration& config);

    /* The pattern is swapped by setInterfacePattern at runtime while the
     * monitor thread and app threads regex_search it — readers copy the
     * shared_ptr under mListenersMutex and search the immutable regex it
     * points at (std::regex assignment is not thread-safe to read through).
     * The pattern string returns by value for the same reason. */
    std::shared_ptr<const std::regex> mInterfacePattern =
            std::make_shared<const std::regex>(R"(eth\d+)");
    std::string mInterfacePatternString = R"(eth\d+)";  /* pre-C++17 regex has no pattern() */
    std::string mStoreDir = "/var/lib/cdnet";
    std::mutex mConfigMutex;
    std::map<std::string, IpConfiguration> mConfigurations;
    std::map<std::string, DhcpClient::Lease> mLeases;
    /* one worker per DHCP-managed interface: acquire, then renew at T1 */
    struct DhcpSession {
        std::thread thread;
        std::atomic<bool> stop{false};
    };
    std::map<std::string, DhcpSession*> mDhcpSessions;
    std::mutex mListenersMutex;
    std::vector<Listener*> mListeners;
    /* dedup snapshot per interface: link availability + IPv4 presence */
    struct InterfaceSnapshot {
        bool available = false;
        bool hasAddress = false;
    };
    std::map<std::string, InterfaceSnapshot> mLastSnapshots;
    NetworkEventMonitor* mEventMonitor = nullptr;
    std::atomic<bool> mPolling{false};
    std::thread mPollThread;
};

} // namespace cdroid

#endif /* __ETHERNET_MANAGER_H__ */
