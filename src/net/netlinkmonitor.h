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

    /* Pure message builder — unit-testable: RTM_NEWROUTE for the IPv4
     * default route via `gateway` on `iface`. Returns the byte length, or 0
     * when the arguments do not parse. */
    static size_t buildDefaultRouteMessage(const std::string& iface, const std::string& gateway,
                                           unsigned char* buffer, size_t bufferLen);
    /* Send the built message and await the ACK. */
    static bool addDefaultRoute(const std::string& iface, const std::string& gateway);

private:
    void recvLoop();
    void dispatchMessage(const struct nlmsghdr* nlh);

    int mSock = -1;
    std::thread mThread;
    std::atomic<bool> mRunning{false};
    Events* mEvents = nullptr;
    std::mutex mEventsMutex;
};

} // namespace cdroid

#endif /* __NETLINK_MONITOR_H__ */
