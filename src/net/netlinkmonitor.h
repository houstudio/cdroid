#ifndef __NETLINK_MONITOR_H__
#define __NETLINK_MONITOR_H__

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

#include <networkeventmonitor.h>

/* kernel uapi types, defined by <linux/netlink.h> in the .cc — declared at
 * GLOBAL scope so the member below does not invent cdroid::nlmsghdr */
struct nlmsghdr;

namespace cdroid {

/**
 * Linux backend of NetworkEventMonitor: rtnetlink push (RTMGRP_LINK |
 * RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR), the counterpart of AOSP netd's
 * NetlinkMonitor. Windows gets its own backend; consumers only see the
 * seam interface.
 *
 * Also carries the route half of the static-IP application path:
 * RTM_NEWROUTE replaces the deprecated SIOCADDRT rtentry ioctl. (This is
 * the future OsRoute seam resident until the full platform split.)
 */
class NetlinkMonitor : public NetworkEventMonitor {
public:
    NetlinkMonitor() = default;
    ~NetlinkMonitor() override;

    bool start(Events* events) override;
    void stop() override;

    /* if_nametoindex wrapper (route messages reference interfaces by index). */
    static int interfaceIndex(const std::string& iface);

    /* Pure message builders — unit-testable. buildDefaultRouteMessage is
     * the RTM_NEWROUTE form (NLM_F_CREATE|NLM_F_EXCL); the delete form
     * drops the gateway attribute so it matches any default route on the
     * interface. Return the byte length, or 0 when the arguments do not
     * parse. */
    static size_t buildDefaultRouteMessage(const std::string& iface, const std::string& gateway,
                                           unsigned char* buffer, size_t bufferLen);
    static size_t buildDeleteDefaultRouteMessage(const std::string& iface,
                                                 unsigned char* buffer, size_t bufferLen);
    /* Send the built message and await the ACK. */
    static bool addDefaultRoute(const std::string& iface, const std::string& gateway);
    /* RTM_DELROUTE the IPv4 default route on `iface` (any gateway). An
     * absent route (ESRCH) counts as success. */
    static bool deleteDefaultRoute(const std::string& iface);

private:
    void recvLoop();
    void dispatchMessage(const struct nlmsghdr* nlh);
    static size_t buildRouteMessage(const std::string& iface, const std::string& gateway,
                                    unsigned char* buffer, size_t bufferLen,
                                    int msgType, unsigned short msgFlags);

    int mSock = -1;
    std::thread mThread;
    std::atomic<bool> mRunning{false};
    Events* mEvents = nullptr;
    std::mutex mEventsMutex;
};

} // namespace cdroid

#endif /* __NETLINK_MONITOR_H__ */
