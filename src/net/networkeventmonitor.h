#ifndef __NETWORK_EVENT_MONITOR_H__
#define __NETWORK_EVENT_MONITOR_H__

#include <string>

namespace cdroid {

/**
 * Platform-neutral seam for network state events — the Windows-porting
 * extension point (same role as porting/cdgraph.h for displays):
 *
 *   Linux   : rtnetlink push (RTMGRP_LINK | RTMGRP_*_IFADDR), see
 *             netlinkmonitor.{h,cc} — AOSP netd NetlinkMonitor counterpart.
 *   Windows : iphlpapi NotifyIpInterfaceChange / NotifyUnicastIpAddressChange
 *             (own create() in the WIN32 build).
 *   Fallback: consumers keep a poll path when start() fails.
 *
 * Each consumer creates its own instance (the kernel fan-outs multiple
 * sockets; one thread+fd per consumer, no shared lifetime).
 */
class NetworkEventMonitor {
public:
    class Events {
    public:
        virtual ~Events() = default;
        /* link layer state of `iface` (IFF_UP / IFF_LOWER_UP view). */
        virtual void onLinkStateChanged(const std::string& iface, bool up, bool lowerUp) = 0;
        /* an interface address was added or removed. */
        virtual void onAddressChanged(const std::string& iface, bool added,
                                      const std::string& address) = 0;
    };

    virtual ~NetworkEventMonitor() = default;

    /* Subscribe and start delivery. Returns false when the platform source
     * is unavailable — callers keep their poll fallback. */
    virtual bool start(Events* events) = 0;
    virtual void stop() = 0;

    /* Platform factory. Never returns null today (Linux rtnetlink always
     * constructs; failure surfaces at start()). */
    static NetworkEventMonitor* create();
};

} // namespace cdroid

#endif /* __NETWORK_EVENT_MONITOR_H__ */
