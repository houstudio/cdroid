#ifndef __DHCP_CLIENT_H__
#define __DHCP_CLIENT_H__

#include <cstdint>
#include <string>
#include <vector>

#include <dhcpinfo.h>

namespace cdroid {

/**
 * Port of AOSP's built-in DhcpClient (frameworks/base/services/core/
 * java/com/android/server/dhcp/) — the packet-socket DHCP engine that
 * replaced spawned dhcpcd/udhcpc clients. Implemented against RFC 2131/2132
 * with the AOSP shape (the framework sources are not in the local reference
 * trees; structure/state/option handling follow the known design).
 *
 * Linux backend: one AF_PACKET (SOCK_DGRAM, cooked) socket per client on
 * the target interface — DISCOVER/OFFER/REQUEST/ACK over hand-built
 * IP+UDP+BOOTP frames, software-filtered replies. Needs CAP_NET_RAW (the
 * socket) and CAP_NET_ADMIN (applying the lease).
 *
 * Platform seam (Windows): create() — Windows DHCP is a per-interface OS
 * service, so a future backend implements the same surface by toggling it
 * (netsh / iphlpapi) and reading the result, no packet engine needed.
 */
class DhcpClient {
public:
    /* RFC 2131 message types (DHCP option 53). */
    static constexpr int DHCP_DISCOVER = 1;
    static constexpr int DHCP_OFFER    = 2;
    static constexpr int DHCP_REQUEST  = 3;
    static constexpr int DHCP_DECLINE  = 4;
    static constexpr int DHCP_ACK      = 5;
    static constexpr int DHCP_NAK      = 6;
    static constexpr int DHCP_RELEASE  = 7;

    /* A parsed lease — AOSP DhcpResults, convertible to the legacy
     * android.net.DhcpInfo (little-endian int fields). */
    struct Lease {
        std::string ipAddress;               /* yiaddr, dotted */
        std::string serverId;                /* option 54, dotted */
        std::string netmask;                 /* option 1, dotted */
        std::string gateway;                 /* option 3, first router */
        std::vector<std::string> dnsServers; /* option 6 */
        uint32_t leaseDurationSec = 0;       /* option 51 */
        uint32_t t1Sec = 0;                  /* option 58 (renewal) */
        uint32_t t2Sec = 0;                  /* option 59 (rebinding) */
        bool bound = false;

        DhcpInfo toDhcpInfo() const;
    };

    virtual ~DhcpClient() = default;

    /* One-shot DISCOVER -> OFFER -> REQUEST -> ACK. False on timeout/NAK. */
    virtual bool requestLease(Lease& lease, int timeoutMs = 20000) = 0;
    /* Renewal REQUEST carrying ciaddr (bound lease). NOTE: sent broadcast
     * (REBIND-style) to avoid an ARP dependency on the server MAC —
     * documented divergence, servers accept it. */
    virtual bool renewLease(const Lease& lease, Lease& renewed, int timeoutMs = 5000) = 0;
    /* Best-effort RELEASE back to the server. */
    virtual bool releaseLease(const Lease& lease) = 0;

    static DhcpClient* create(const std::string& iface);

    /* ---- pure BOOTP/DHCP codecs (unit-testable) ---- */
    static std::vector<unsigned char> buildDiscover(uint32_t transactionId,
                                                    const unsigned char* mac, size_t macLen);
    static std::vector<unsigned char> buildRequest(uint32_t transactionId,
                                                   const unsigned char* mac, size_t macLen,
                                                   const std::string& requestedIp,
                                                   const std::string& serverId,
                                                   bool renewing);
    static std::vector<unsigned char> buildRelease(uint32_t transactionId,
                                                   const unsigned char* mac, size_t macLen,
                                                   const std::string& ciaddr,
                                                   const std::string& serverId);
    /* Parse a BOOTP reply payload (eth/ip/udp headers already stripped).
     * Returns the option-53 message type — 0 when the packet is malformed,
     * not a reply, or fails the xid/chaddr match. Fills `lease` on
     * OFFER/ACK. */
    static int parseReply(const unsigned char* payload, size_t length, uint32_t transactionId,
                          const unsigned char* mac, size_t macLen, Lease& lease);
};

} // namespace cdroid

#endif /* __DHCP_CLIENT_H__ */
