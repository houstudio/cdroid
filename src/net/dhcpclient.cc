/* Built-in DHCP client (AOSP DhcpClient counterpart, RFC 2131/2132). */
#include <dhcpclient.h>

#include <arpa/inet.h>
#include <errno.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <chrono>

#define DHCP_LOGE(...) do { fprintf(stderr, "DhcpClient E: " __VA_ARGS__); fprintf(stderr, "\n"); } while (0)

namespace cdroid {

/* --- BOOTP layout constants (RFC 2131 §2) ----------------------------------- */

static constexpr size_t BOOTP_MIN_LEN = 244;         /* 236 fixed + cookie */
static constexpr size_t OFF_XID       = 4;
static constexpr size_t OFF_CIADDR    = 12;
static constexpr size_t OFF_YIADDR    = 16;
static constexpr size_t OFF_SIADDR    = 20;
static constexpr size_t OFF_CHADDR    = 28;
static constexpr size_t OFF_COOKIE    = 236;
static constexpr uint32_t MAGIC_COOKIE = 0x63825363;

/* RFC 2132 option codes used here. */
static constexpr int OPT_PAD          = 0;
static constexpr int OPT_SUBNET_MASK  = 1;
static constexpr int OPT_ROUTER       = 3;
static constexpr int OPT_DNS          = 6;
static constexpr int OPT_REQUESTED_IP = 50;
static constexpr int OPT_LEASE_TIME   = 51;
static constexpr int OPT_MSG_TYPE     = 53;
static constexpr int OPT_SERVER_ID    = 54;
static constexpr int OPT_PARAM_LIST   = 55;
static constexpr int OPT_T1           = 58;
static constexpr int OPT_T2           = 59;
static constexpr int OPT_END          = 255;

static uint32_t nowMs() {
    return (uint32_t) std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
}

static void appendOption(std::vector<unsigned char>& out, int code, const void* data, size_t len) {
    out.push_back((unsigned char) code);
    out.push_back((unsigned char) len);
    const unsigned char* bytes = static_cast<const unsigned char*>(data);
    out.insert(out.end(), bytes, bytes + len);
}

static void appendU32Option(std::vector<unsigned char>& out, int code, uint32_t value) {
    const uint32_t be = htonl(value);
    appendOption(out, code, &be, sizeof(be));
}

static void appendIpOption(std::vector<unsigned char>& out, int code, const std::string& dotted) {
    struct in_addr addr;
    if (inet_pton(AF_INET, dotted.c_str(), &addr) != 1) return;
    appendOption(out, code, &addr, sizeof(addr));
}

/* Fixed 236-byte BOOTP header shared by every builder. */
static void initBootpHeader(std::vector<unsigned char>& packet, uint32_t transactionId,
                             const unsigned char* mac, size_t macLen, uint16_t flags) {
    packet.assign(BOOTP_MIN_LEN, 0);
    packet[0] = 1;                 /* op = BOOTREQUEST */
    packet[1] = 1;                 /* htype = ethernet */
    packet[2] = (unsigned char) macLen;
    packet[3] = 0;                 /* hops */
    uint32_t xid = htonl(transactionId);
    memcpy(&packet[OFF_XID], &xid, sizeof(xid));
    uint16_t flagsBe = htons(flags);
    memcpy(&packet[10], &flagsBe, sizeof(flagsBe));
    if (macLen > 16) macLen = 16;
    memcpy(&packet[OFF_CHADDR], mac, macLen);
    uint32_t cookie = htonl(MAGIC_COOKIE);
    memcpy(&packet[OFF_COOKIE], &cookie, sizeof(cookie));
}

std::vector<unsigned char> DhcpClient::buildDiscover(uint32_t transactionId,
                                                     const unsigned char* mac, size_t macLen) {
    std::vector<unsigned char> packet;
    initBootpHeader(packet, transactionId, mac, macLen, 0x8000 /* broadcast */);
    appendOption(packet, OPT_MSG_TYPE, "\x01", 1);
    /* parameter request list: mask, router, dns, domain, broadcast, lease,
     * T1, T2 — the AOSP set */
    static const unsigned char paramList[] = {1, 3, 6, 15, 28, 51, 58, 59};
    appendOption(packet, OPT_PARAM_LIST, paramList, sizeof(paramList));
    packet.push_back(OPT_END);
    return packet;
}

std::vector<unsigned char> DhcpClient::buildRequest(uint32_t transactionId,
                                                    const unsigned char* mac, size_t macLen,
                                                    const std::string& requestedIp,
                                                    const std::string& serverId,
                                                    bool renewing) {
    std::vector<unsigned char> packet;
    initBootpHeader(packet, transactionId, mac, macLen, 0x8000);
    if (renewing) {
        /* RENEWING form: ciaddr carries the held address, no 50/54 */
        struct in_addr addr;
        if (inet_pton(AF_INET, requestedIp.c_str(), &addr) == 1)
            memcpy(&packet[OFF_CIADDR], &addr, sizeof(addr));
    }
    appendOption(packet, OPT_MSG_TYPE, "\x03", 1);
    if (!renewing) {
        appendIpOption(packet, OPT_REQUESTED_IP, requestedIp);
        appendIpOption(packet, OPT_SERVER_ID, serverId);
    }
    static const unsigned char paramList[] = {1, 3, 6, 15, 28, 51, 58, 59};
    appendOption(packet, OPT_PARAM_LIST, paramList, sizeof(paramList));
    packet.push_back(OPT_END);
    return packet;
}

std::vector<unsigned char> DhcpClient::buildRelease(uint32_t transactionId,
                                                    const unsigned char* mac, size_t macLen,
                                                    const std::string& ciaddr,
                                                    const std::string& serverId) {
    std::vector<unsigned char> packet;
    initBootpHeader(packet, transactionId, mac, macLen, 0 /* unicast intent */);
    struct in_addr addr;
    if (inet_pton(AF_INET, ciaddr.c_str(), &addr) == 1)
        memcpy(&packet[OFF_CIADDR], &addr, sizeof(addr));
    appendOption(packet, OPT_MSG_TYPE, "\x07", 1);
    appendIpOption(packet, OPT_SERVER_ID, serverId);
    packet.push_back(OPT_END);
    return packet;
}

static std::string dottedAt(const unsigned char* p) {
    char buf[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET, p, buf, sizeof(buf))) return std::string();
    return buf;
}

int DhcpClient::parseReply(const unsigned char* payload, size_t length, uint32_t transactionId,
                           const unsigned char* mac, size_t macLen, Lease& lease) {
    if (length < BOOTP_MIN_LEN) return 0;
    if (payload[0] != 2 /* BOOTREPLY */) return 0;
    if (payload[1] != 1 /* htype ethernet */) return 0;
    uint32_t xid = 0;
    memcpy(&xid, payload + OFF_XID, sizeof(xid));
    if (ntohl(xid) != transactionId) return 0;
    /* chaddr must be ours or blanked (some relays zero it) */
    bool chaddrMatch = memcmp(payload + OFF_CHADDR, mac, macLen < 16 ? macLen : 16) == 0;
    bool chaddrBlank = true;
    for (size_t i = 0; i < 16; i++)
        if (payload[OFF_CHADDR + i] != 0) { chaddrBlank = false; break; }
    if (!chaddrMatch && !chaddrBlank) return 0;
    uint32_t cookie = 0;
    memcpy(&cookie, payload + OFF_COOKIE, sizeof(cookie));
    if (ntohl(cookie) != MAGIC_COOKIE) return 0;

    lease = Lease();
    lease.ipAddress = dottedAt(payload + OFF_YIADDR);
    lease.serverId = dottedAt(payload + OFF_SIADDR); /* fallback: sname-adjacent server */

    int messageType = 0;
    size_t pos = OFF_COOKIE + 4;
    while (pos < length) {
        const int code = payload[pos++];
        if (code == OPT_PAD) continue;
        if (code == OPT_END) break;
        if (pos >= length) break;
        const size_t len = payload[pos++];
        if (pos + len > length) break;
        const unsigned char* value = payload + pos;
        switch (code) {
        case OPT_MSG_TYPE:
            if (len >= 1) messageType = value[0];
            break;
        case OPT_SERVER_ID:
            if (len >= 4) lease.serverId = dottedAt(value);
            break;
        case OPT_LEASE_TIME:
            if (len >= 4) { uint32_t v; memcpy(&v, value, 4); lease.leaseDurationSec = ntohl(v); }
            break;
        case OPT_T1:
            if (len >= 4) { uint32_t v; memcpy(&v, value, 4); lease.t1Sec = ntohl(v); }
            break;
        case OPT_T2:
            if (len >= 4) { uint32_t v; memcpy(&v, value, 4); lease.t2Sec = ntohl(v); }
            break;
        case OPT_SUBNET_MASK:
            if (len >= 4) lease.netmask = dottedAt(value);
            break;
        case OPT_ROUTER:
            if (len >= 4) lease.gateway = dottedAt(value);
            break;
        case OPT_DNS:
            for (size_t i = 0; i + 4 <= len; i += 4)
                lease.dnsServers.push_back(dottedAt(value + i));
            break;
        default:
            break;
        }
        pos += len;
    }
    lease.bound = (messageType == DHCP_ACK);
    return messageType;
}

DhcpInfo DhcpClient::Lease::toDhcpInfo() const {
    DhcpInfo info;
    info.ipAddress = DhcpInfo::stringToInt(ipAddress);
    info.netmask = DhcpInfo::stringToInt(netmask);
    info.gateway = DhcpInfo::stringToInt(gateway);
    info.serverAddress = DhcpInfo::stringToInt(serverId);
    if (!dnsServers.empty()) info.dns1 = DhcpInfo::stringToInt(dnsServers[0]);
    if (dnsServers.size() > 1) info.dns2 = DhcpInfo::stringToInt(dnsServers[1]);
    info.leaseDuration = (int) leaseDurationSec;
    return info;
}

/* --- Linux packet-socket backend --------------------------------------------- */

namespace {

class PacketDhcpClient : public DhcpClient {
public:
    explicit PacketDhcpClient(const std::string& iface)
        : mIface(iface), mIfindex(if_nametoindex(iface.c_str())) {
        srand(nowMs() ^ (getpid() << 16));
        mTransactionId = ((uint32_t) rand() << 16) ^ (uint32_t) rand();
    }

    ~PacketDhcpClient() override {
        if (mSock >= 0) close(mSock);
    }

    bool requestLease(Lease& lease, int timeoutMs) override {
        if (!openSocket()) return false;
        const uint32_t deadline = nowMs() + (uint32_t) timeoutMs;
        if (!sendBootp(DhcpClient::buildDiscover(mTransactionId, mMac, sizeof(mMac)),
                       "0.0.0.0"))
            return false;
        bool requested = false;
        std::string offeredIp, serverId;
        while (nowMs() < deadline) {
            const int type = receiveReply(lease, (int) (deadline - nowMs()));
            if (type == 0) continue;
            if (type == DHCP_NAK) return false;
            if (type == DHCP_OFFER && !requested) {
                offeredIp = lease.ipAddress;
                serverId = lease.serverId;
                if (offeredIp.empty()) continue;
                requested = true;
                if (!sendBootp(DhcpClient::buildRequest(mTransactionId, mMac, sizeof(mMac),
                                                        offeredIp, serverId, false),
                               "0.0.0.0"))
                    return false;
            } else if (type == DHCP_ACK && requested) {
                lease.bound = true;
                return true;
            }
        }
        return false;
    }

    bool renewLease(const Lease& lease, Lease& renewed, int timeoutMs) override {
        if (!openSocket()) return false;
        /* broadcast RENEWING-form REQUEST (ciaddr set) — see header note */
        if (!sendBootp(DhcpClient::buildRequest(mTransactionId, mMac, sizeof(mMac),
                                                lease.ipAddress, std::string(), true),
                       lease.ipAddress))
            return false;
        const uint32_t deadline = nowMs() + (uint32_t) timeoutMs;
        while (nowMs() < deadline) {
            const int type = receiveReply(renewed, (int) (deadline - nowMs()));
            if (type == DHCP_ACK) { renewed.bound = true; return true; }
            if (type == DHCP_NAK) return false;
        }
        return false;
    }

    bool releaseLease(const Lease& lease) override {
        if (!openSocket()) return false;
        return sendBootp(DhcpClient::buildRelease(mTransactionId, mMac, sizeof(mMac),
                                                  lease.ipAddress, lease.serverId),
                         lease.ipAddress);
    }

private:
    bool openSocket() {
        if (mSock >= 0) return true;
        if (mIfindex == 0) {
            DHCP_LOGE("unknown interface %s", mIface.c_str());
            return false;
        }
        if (!readMacAddress()) return false;
        mSock = socket(AF_PACKET, SOCK_DGRAM | SOCK_CLOEXEC, htons(ETH_P_IP));
        if (mSock < 0) {
            DHCP_LOGE("AF_PACKET socket: %s (need CAP_NET_RAW)", strerror(errno));
            return false;
        }
        struct sockaddr_ll address;
        memset(&address, 0, sizeof(address));
        address.sll_family = AF_PACKET;
        address.sll_protocol = htons(ETH_P_IP);
        address.sll_ifindex = (int) mIfindex;
        if (bind(mSock, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) != 0) {
            DHCP_LOGE("bind: %s", strerror(errno));
            close(mSock);
            mSock = -1;
            return false;
        }
        const int rcvbuf = 64 * 1024;
        setsockopt(mSock, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));
        return true;
    }

    bool readMacAddress() {
        const int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0) return false;
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, mIface.c_str(), IFNAMSIZ - 1);
        const bool ok = ioctl(fd, SIOCGIFHWADDR, &ifr) == 0;
        close(fd);
        if (ok) memcpy(mMac, ifr.ifr_hwaddr.sa_data, sizeof(mMac));
        return ok;
    }

    /* IPv4 header checksum (RFC 1071). */
    static uint16_t ipChecksum(const unsigned char* header, size_t len) {
        uint32_t sum = 0;
        for (size_t i = 0; i < len; i += 2)
            sum += (uint32_t)((header[i] << 8) | (i + 1 < len ? header[i + 1] : 0));
        while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
        return (uint16_t) (~sum & 0xFFFF);
    }

    bool sendBootp(const std::vector<unsigned char>& bootp, const std::string& srcIp) {
        /* IP + UDP + BOOTP; cooked packet socket supplies the ethernet header */
        std::vector<unsigned char> frame(20 + 8 + bootp.size(), 0);
        unsigned char* ip = frame.data();
        ip[0] = 0x45;                                  /* v4, ihl 5 */
        ip[1] = 0x10;                                  /* tos */
        const uint16_t totalLen = htons((uint16_t) frame.size());
        memcpy(ip + 2, &totalLen, sizeof(totalLen));
        ip[4] = (unsigned char) (mTransactionId >> 8); /* id */
        ip[5] = (unsigned char) mTransactionId;
        ip[8] = 64;                                    /* ttl */
        ip[9] = 17;                                    /* proto UDP */
        struct in_addr src, dst;
        inet_pton(AF_INET, srcIp.c_str(), &src);
        inet_pton(AF_INET, "255.255.255.255", &dst);
        memcpy(ip + 12, &src, 4);
        memcpy(ip + 16, &dst, 4);
        const uint16_t checksum = ipChecksum(ip, 20);
        memcpy(ip + 10, &checksum, sizeof(checksum));
        unsigned char* udp = ip + 20;
        const uint16_t sport = htons(68), dport = htons(67);
        memcpy(udp + 0, &sport, sizeof(sport));
        memcpy(udp + 2, &dport, sizeof(dport));
        const uint16_t udpLen = htons((uint16_t) (8 + bootp.size()));
        memcpy(udp + 4, &udpLen, sizeof(udpLen));      /* checksum 0: legal IPv4 */

        struct sockaddr_ll dest;
        memset(&dest, 0, sizeof(dest));
        dest.sll_family = AF_PACKET;
        dest.sll_protocol = htons(ETH_P_IP);
        dest.sll_ifindex = (int) mIfindex;
        dest.sll_halen = 6;
        memset(dest.sll_addr, 0xFF, 6);                /* broadcast MAC */
        const ssize_t sent = sendto(mSock, frame.data(), frame.size(), 0,
                                    reinterpret_cast<struct sockaddr*>(&dest), sizeof(dest));
        if (sent != (ssize_t) frame.size()) {
            DHCP_LOGE("sendto: %s", strerror(errno));
            return false;
        }
        return true;
    }

    int receiveReply(Lease& lease, int timeoutMs) {
        if (timeoutMs <= 0) timeoutMs = 1;
        struct pollfd pfd = {mSock, POLLIN, 0};
        const int rc = poll(&pfd, 1, timeoutMs);
        if (rc <= 0) return 0;
        unsigned char buffer[2048];
        const ssize_t len = recv(mSock, buffer, sizeof(buffer), 0);
        if (len < (ssize_t) (20 + 8 + BOOTP_MIN_LEN)) return 0;
        /* software filter: IPv4 + UDP destined to the bootpc port */
        const unsigned char* ip = buffer;
        if ((ip[0] >> 4) != 4) return 0;
        const size_t ihl = (size_t) (ip[0] & 0xF) * 4;
        if (ihl < 20 || (size_t) len < ihl + 8 + BOOTP_MIN_LEN) return 0;
        if (ip[9] != 17) return 0;                     /* UDP */
        const unsigned char* udp = ip + ihl;
        const uint16_t dport = (uint16_t) ((udp[2] << 8) | udp[3]);
        if (dport != 68) return 0;
        return DhcpClient::parseReply(udp + 8, (size_t) len - ihl - 8, mTransactionId,
                                      mMac, sizeof(mMac), lease);
    }

    std::string mIface;
    unsigned int mIfindex;
    unsigned char mMac[6];
    int mSock = -1;
    uint32_t mTransactionId;
};

} // namespace

DhcpClient* DhcpClient::create(const std::string& iface) {
    return new PacketDhcpClient(iface);
}

} // namespace cdroid
