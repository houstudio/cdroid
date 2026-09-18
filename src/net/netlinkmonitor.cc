/* rtnetlink backend (AOSP netd NetlinkMonitor counterpart). Pure Linux. */
#include <netlinkmonitor.h>

#include <arpa/inet.h>
#include <errno.h>
#include <linux/if_addr.h>
#include <linux/if_link.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

/* Self-contained module logging, same shim as the managers. */
#define NL_LOGE(...) do { fprintf(stderr, "NetlinkMonitor E: " __VA_ARGS__); fprintf(stderr, "\n"); } while (0)

/* glibc's <net/if.h> ships only the classic IFF_* set; IFF_LOWER_UP lives in
 * the kernel uapi. Including <linux/if.h> alongside <net/if.h> redefines
 * IFF_UP (known clash), so pin the stable uapi value instead. */
#ifndef IFF_LOWER_UP
#define IFF_LOWER_UP 0x10000
#endif

namespace cdroid {

NetlinkMonitor::~NetlinkMonitor() {
    stop();
}

bool NetlinkMonitor::start(Events* events) {
    {
        std::lock_guard<std::mutex> lock(mEventsMutex);
        mEvents = events;
    }
    mSock = socket(AF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_ROUTE);
    if (mSock < 0) {
        NL_LOGE("NETLINK_ROUTE socket: %s", strerror(errno));
        return false;
    }
    struct sockaddr_nl address;
    memset(&address, 0, sizeof(address));
    address.nl_family = AF_NETLINK;
    address.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR;
    if (bind(mSock, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) != 0) {
        NL_LOGE("bind nl_groups: %s", strerror(errno));
        close(mSock);
        mSock = -1;
        return false;
    }
    /* netlink delivers no initial state — callers seed with their own sweep. */
    mRunning.store(true);
    mThread = std::thread(&NetlinkMonitor::recvLoop, this);
    return true;
}

void NetlinkMonitor::stop() {
    if (mRunning.exchange(false)) {
        if (mThread.joinable()) mThread.join();
    }
    if (mSock >= 0) {
        close(mSock);
        mSock = -1;
    }
}

void NetlinkMonitor::recvLoop() {
    unsigned char buffer[8192];
    while (mRunning.load()) {
        /* select with a timeout: stop() only flips the flag, so a plain
         * blocking recv would hang the join forever (bit us at singleton
         * teardown once already). */
        fd_set readFds;
        FD_ZERO(&readFds);
        FD_SET(mSock, &readFds);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 200 * 1000;
        const int rc = select(mSock + 1, &readFds, nullptr, nullptr, &tv);
        if (!mRunning.load()) break;
        if (rc == 0) continue;              /* poll timeout: re-check mRunning */
        if (rc < 0) {
            if (errno == EINTR) continue;
            NL_LOGE("select: %s", strerror(errno));
            break;
        }
        const ssize_t len = recv(mSock, buffer, sizeof(buffer), 0);
        if (len <= 0) {
            if (errno == EINTR) continue;
            if (!mRunning.load()) break;
            NL_LOGE("recv: %s", strerror(errno));
            break;
        }
        /* NLMSG_NEXT mutates its status argument — recv's const length goes
         * through a mutable copy. */
        int remaining = static_cast<int>(len);
        for (struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*>(buffer);
             NLMSG_OK(nlh, remaining);
             nlh = NLMSG_NEXT(nlh, remaining)) {
            dispatchMessage(nlh);
        }
    }
}

void NetlinkMonitor::dispatchMessage(const struct nlmsghdr* nlh) {
    Events* events;
    {
        std::lock_guard<std::mutex> lock(mEventsMutex);
        events = mEvents;
    }
    if (!events) return;
    if (nlh->nlmsg_type == RTM_NEWLINK || nlh->nlmsg_type == RTM_DELLINK) {
        const struct ifinfomsg* info = static_cast<const struct ifinfomsg*>(NLMSG_DATA(nlh));
        /* the interface name rides in the IFLA_IFNAME attribute */
        char ifname[IF_NAMESIZE] = {0};
        size_t attrLen = IFLA_PAYLOAD(nlh);
        for (struct rtattr* rta = IFLA_RTA(info); RTA_OK(rta, attrLen);
             rta = RTA_NEXT(rta, attrLen)) {
            if (rta->rta_type == IFLA_IFNAME) {
                const size_t nameLen = RTA_PAYLOAD(rta) < IF_NAMESIZE - 1
                        ? RTA_PAYLOAD(rta) : IF_NAMESIZE - 1;
                memcpy(ifname, RTA_DATA(rta), nameLen);
                ifname[nameLen] = '\0';
                break;
            }
        }
        if (ifname[0] != '\0') {
            const bool up = (info->ifi_flags & IFF_UP) != 0;
            const bool lowerUp = (info->ifi_flags & IFF_LOWER_UP) != 0;
            events->onLinkStateChanged(ifname, up, lowerUp);
        }
    } else if (nlh->nlmsg_type == RTM_NEWADDR || nlh->nlmsg_type == RTM_DELADDR) {
        const struct ifaddrmsg* msg = static_cast<const struct ifaddrmsg*>(NLMSG_DATA(nlh));
        char ifname[IF_NAMESIZE] = {0};
        if (if_indextoname(msg->ifa_index, ifname) == nullptr) return;
        size_t attrLen = IFA_PAYLOAD(nlh);
        for (struct rtattr* rta = IFA_RTA(msg); RTA_OK(rta, attrLen);
             rta = RTA_NEXT(rta, attrLen)) {
            if (rta->rta_type != IFA_LOCAL && rta->rta_type != IFA_ADDRESS) continue;
            if (msg->ifa_family != AF_INET && msg->ifa_family != AF_INET6) continue;
            char addrBuf[INET6_ADDRSTRLEN] = {0};
            if (!inet_ntop(msg->ifa_family, RTA_DATA(rta), addrBuf, sizeof(addrBuf))) continue;
            events->onAddressChanged(ifname, nlh->nlmsg_type == RTM_NEWADDR, addrBuf);
            break; /* IFA_LOCAL is the local address; ignore the dup */
        }
    }
}

int NetlinkMonitor::interfaceIndex(const std::string& iface) {
    return if_nametoindex(iface.c_str());
}

/* --- default route application (RTM_NEWROUTE) ------------------------------ */

static void putRtattr(struct rtattr* rta, int type, const void* data, size_t len) {
    rta->rta_type = type;
    rta->rta_len = static_cast<unsigned short>(RTA_LENGTH(len));
    memcpy(RTA_DATA(rta), data, len);
}

size_t NetlinkMonitor::buildDefaultRouteMessage(const std::string& iface,
                                                const std::string& gateway,
                                                unsigned char* buffer, size_t bufferLen) {
    return buildRouteMessage(iface, gateway, buffer, bufferLen, RTM_NEWROUTE,
                             NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL);
}

size_t NetlinkMonitor::buildDeleteDefaultRouteMessage(const std::string& iface,
                                                      unsigned char* buffer, size_t bufferLen) {
    return buildRouteMessage(iface, std::string(), buffer, bufferLen, RTM_DELROUTE,
                             NLM_F_REQUEST | NLM_F_ACK);
}

size_t NetlinkMonitor::buildRouteMessage(const std::string& iface,
                                         const std::string& gateway,
                                         unsigned char* buffer, size_t bufferLen,
                                         int msgType, unsigned short msgFlags) {
    struct in_addr gw;
    bool haveGw = false;
    if (!gateway.empty()) {
        if (inet_pton(AF_INET, gateway.c_str(), &gw) != 1) return 0;
        haveGw = true;
    }
    const unsigned int oif = if_nametoindex(iface.c_str());
    if (oif == 0) return 0;
    /* Validate the buffer up front: the writes below must never run past
     * bufferLen (the old check ran only after everything was written). */
    size_t needed = NLMSG_ALIGN(NLMSG_LENGTH(sizeof(struct rtmsg)))
            + RTA_ALIGN(RTA_LENGTH(sizeof(oif)));
    if (haveGw) needed += RTA_ALIGN(RTA_LENGTH(sizeof(gw)));
    if (buffer == nullptr || bufferLen < needed) return 0;

    memset(buffer, 0, bufferLen);
    auto* nlh = reinterpret_cast<struct nlmsghdr*>(buffer);
    nlh->nlmsg_type = static_cast<unsigned short>(msgType);
    nlh->nlmsg_flags = msgFlags;
    nlh->nlmsg_seq = 1;

    auto* rtm = static_cast<struct rtmsg*>(NLMSG_DATA(nlh));
    rtm->rtm_family = AF_INET;
    rtm->rtm_table = RT_TABLE_MAIN;
    rtm->rtm_protocol = RTPROT_STATIC;
    rtm->rtm_scope = RT_SCOPE_UNIVERSE;
    rtm->rtm_type = RTN_UNICAST;

    size_t offset = NLMSG_ALIGN(NLMSG_LENGTH(sizeof(struct rtmsg)));
    if (haveGw) {
        struct rtattr* rta = reinterpret_cast<struct rtattr*>(buffer + offset);
        putRtattr(rta, RTA_GATEWAY, &gw, sizeof(gw));
        offset += RTA_ALIGN(rta->rta_len);
    }
    struct rtattr* rta = reinterpret_cast<struct rtattr*>(buffer + offset);
    putRtattr(rta, RTA_OIF, &oif, sizeof(oif));
    offset += RTA_ALIGN(rta->rta_len);

    nlh->nlmsg_len = static_cast<unsigned int>(offset);
    return offset;
}

/* Send a built route message and await the ACK (NLMSG_ERROR error==0).
 * `ignoredError` is the "already in the desired state" errno: -EEXIST for
 * add, -ESRCH for delete. Bounded (SO_RCVTIMEO) and EINTR retried — an
 * interrupted or timed-out wait must not confirm the operation. */
static bool sendRouteMessageAndAck(const unsigned char* message, size_t len,
                                   const char* what, int ignoredError) {
    const int fd = socket(AF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_ROUTE);
    if (fd < 0) return false;
    struct sockaddr_nl kernel;
    memset(&kernel, 0, sizeof(kernel));
    kernel.nl_family = AF_NETLINK;
    bool ok = sendto(fd, message, len, 0, reinterpret_cast<struct sockaddr*>(&kernel),
                     sizeof(kernel)) == static_cast<ssize_t>(len);
    if (ok) {
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        unsigned char reply[128];
        ssize_t rlen;
        while ((rlen = recv(fd, reply, sizeof(reply), 0)) < 0 && errno == EINTR) {
        }
        if (rlen > 0) {
            const auto* nlh = reinterpret_cast<const struct nlmsghdr*>(reply);
            if (nlh->nlmsg_type == NLMSG_ERROR) {
                const auto* err = static_cast<const struct nlmsgerr*>(NLMSG_DATA(nlh));
                if (err->error != 0 && err->error != ignoredError) {
                    errno = -err->error;
                    NL_LOGE("%s: %s", what, strerror(errno));
                    ok = false;
                }
            }
        } else {
            NL_LOGE("%s: no ACK (%s)", what, strerror(errno));
            ok = false;
        }
    }
    close(fd);
    return ok;
}

bool NetlinkMonitor::addDefaultRoute(const std::string& iface, const std::string& gateway) {
    unsigned char buffer[128];
    const size_t len = buildDefaultRouteMessage(iface, gateway, buffer, sizeof(buffer));
    if (len == 0) return false;
    const std::string what = "RTM_NEWROUTE " + iface + " via " + gateway;
    return sendRouteMessageAndAck(buffer, len, what.c_str(), -EEXIST);
}

bool NetlinkMonitor::deleteDefaultRoute(const std::string& iface) {
    unsigned char buffer[128];
    const size_t len = buildDeleteDefaultRouteMessage(iface, buffer, sizeof(buffer));
    if (len == 0) return false;
    const std::string what = "RTM_DELROUTE default on " + iface;
    return sendRouteMessageAndAck(buffer, len, what.c_str(), -ESRCH);
}

NetworkEventMonitor* NetworkEventMonitor::create() {
    return new NetlinkMonitor();
}

} // namespace cdroid
