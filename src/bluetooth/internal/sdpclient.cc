/**
 * SDP client transport: an L2CAP socket to the remote's PSM 0x0001
 * carrying ServiceSearchAttributeRequest/Response PDUs. The PDU layer
 * is sdppdu.{h,cc}; this half needs a live peer (exercised on the
 * two-host bench).
 */
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>

#include "btuapi.h"
#include "sdppdu.h"
#include "sdpclient.h"

namespace cdroid {

namespace {

/* minimal L2CAP sockaddr (kernel UAPI subset, vendored alongside the
 * RFCOMM one the day GATT forced it in — SDP needs it first) */
struct sockaddr_l2 {
    sa_family_t l2_family;
    uint16_t    l2_psm;
    uint8_t     l2_bdaddr_type;
    bdaddr_t    l2_bdaddr;
    uint8_t     l2_cid[2];
};
#define BTPROTO_L2CAP 0

constexpr uint16_t SDP_PSM = 0x0001;

/* Send the request PDU (header + body) and read response fragments
 * until the continuation state is empty. Each response body is fed to
 * the parser; the first fragment carrying an RFCOMM channel wins. */
int transact(int fd, const std::vector<uint8_t>& body,
             const std::vector<uint8_t>& uuid128) {
    std::vector<uint8_t> continuation;
    for (int attempt = 0; attempt < 8; attempt++) {
        /* PDU header: id, len(2), tid(2) — params len covers the body */
        std::vector<uint8_t> pdu;
        pdu.push_back(sdp::PDU_SEARCH_ATTR_REQUEST);
        const uint16_t tid = (uint16_t)(attempt + 1);
        pdu.push_back((uint8_t)(tid >> 8));
        pdu.push_back((uint8_t)(tid & 0xFF));
        std::vector<uint8_t> inner = sdp::buildSearchAttributeRequest(
                uuid128, 0xFFFF, continuation);
        pdu.push_back((uint8_t)(inner.size() >> 8));
        pdu.push_back((uint8_t)(inner.size() & 0xFF));
        pdu.insert(pdu.end(), inner.begin(), inner.end());
        if (write(fd, pdu.data(), pdu.size()) != (ssize_t)pdu.size()) return -1;

        /* response: header(5) + body; read greedily (SDP responses are
         * small — 4 KiB covers a full descriptor list with margin) */
        uint8_t buf[4096];
        const ssize_t n = read(fd, buf, sizeof(buf));
        if (n < 7) return -1;
        if (buf[0] != sdp::PDU_SEARCH_ATTR_RESPONSE) return -1;
        const size_t paramLen = ((size_t)buf[3] << 8) | buf[4];
        if ((size_t)n < 5 + paramLen) return -1;
        std::vector<uint8_t> respBody(buf + 5, buf + 5 + paramLen);

        const int channel = sdp::parseSearchAttributeResponse(respBody);
        if (channel > 0) return channel;

        /* continuation state = last byte(s) of the body */
        const uint8_t contLen = respBody.empty() ? 0 : respBody.back();
        if (contLen == 0 || respBody.size() < 1 + contLen) return -1;
        continuation.assign(respBody.end() - contLen, respBody.end());
    }
    return -1;
}

} // namespace

int sdpResolveRfcommChannel(const std::string& bdaddr,
                            const std::vector<uint8_t>& uuid128) {
    if (uuid128.size() != 16) return -1;
    const int fd = ::socket(AF_BLUETOOTH, SOCK_STREAM | SOCK_NONBLOCK,
                            BTPROTO_L2CAP);
    if (fd < 0) return -1;

    struct sockaddr_l2 addr;
    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    addr.l2_psm = SDP_PSM;
    str2ba(bdaddr.c_str(), &addr.l2_bdaddr);
    /* Bound connect + read (a peer with no SDP server must not hang
     * the caller — kernel L2CAP connect retries indefinitely). */
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0
            && errno != EINPROGRESS) {
        ::close(fd);
        return -1;
    }
    if (errno == EINPROGRESS) {
        struct pollfd pfd = { fd, POLLOUT, 0 };
        if (poll(&pfd, 1, 3000) != 1) { ::close(fd); return -1; }
        int err = 0; socklen_t elen = sizeof(err);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &elen) < 0 || err) {
            ::close(fd);
            return -1;
        }
    }
    /* bound reads for the transaction */
    struct timeval tv = { 3, 0 };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    const int channel = transact(fd, uuid128, uuid128);
    ::close(fd);
    return channel;
}

} // namespace cdroid
