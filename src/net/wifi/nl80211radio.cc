/* nl80211 radio-data backend (AOSP wifi-HAL scan source counterpart). */
#include <wifi/nl80211radio.h>

#include <arpa/inet.h>
#include <errno.h>
#include <linux/genetlink.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/nl80211.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define RADIO_LOGE(...) do { fprintf(stderr, "Nl80211Radio E: " __VA_ARGS__); fprintf(stderr, "\n"); } while (0)

namespace cdroid {

/* 802.11 element ids used below (IEEE names; local constants — not every
 * toolchain ships linux/ieee80211.h). */
static constexpr int IE_SSID        = 0;
static constexpr int IE_VENDOR      = 221;
static constexpr int IE_HT_CAP      = 45;
static constexpr int IE_HT_OP       = 61;
static constexpr int IE_RSN         = 48;
static constexpr int IE_VHT_CAP     = 191;
static constexpr int IE_VHT_OP      = 192;
static constexpr int IE_HE_CAP      = 232;
static constexpr int IE_EHT_CAP     = 255;

static const unsigned char OUI_MICROSOFT_WPA[] = {0x00, 0x50, 0xf2, 0x01};

/* --- small helpers --------------------------------------------------------- */

static uint16_t getU16(const unsigned char* p) {
    return (uint16_t)(p[0] | (p[1] << 8));
}

static std::string macToString(const unsigned char* mac) {
    char buf[18];
    snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return buf;
}

static const struct rtattr* findAttr(const void* data, size_t length, int wantType) {
    size_t remaining = length;
    for (const struct rtattr* rta = static_cast<const struct rtattr*>(data);
         RTA_OK(rta, remaining); rta = RTA_NEXT(rta, remaining)) {
        if ((rta->rta_type & NLA_TYPE_MASK) == wantType) return rta;
    }
    return nullptr;
}

static void appendJoined(std::string& out, const std::vector<std::string>& names) {
    for (size_t i = 0; i < names.size(); i++) {
        if (i) out += "+";
        out += names[i];
    }
}

/* --- pure IE helpers (unit-testable) ---------------------------------------- */

std::vector<ScanResult::InformationElement> Nl80211Radio::parseIeStream(
        const unsigned char* ies, size_t length) {
    std::vector<ScanResult::InformationElement> elements;
    size_t pos = 0;
    while (pos + 2 <= length) {
        const int id = ies[pos];
        const size_t len = ies[pos + 1];
        if (pos + 2 + len > length) break;
        ScanResult::InformationElement element;
        element.id = id;
        element.bytes.assign(ies + pos + 2, ies + pos + 2 + len);
        elements.push_back(element);
        pos += 2 + len;
    }
    return elements;
}

static const ScanResult::InformationElement* findIe(
        const std::vector<ScanResult::InformationElement>& ies, int id) {
    for (const auto& element : ies)
        if (element.id == id) return &element;
    return nullptr;
}

std::string Nl80211Radio::buildCapabilitiesString(
        const std::vector<ScanResult::InformationElement>& ies,
        unsigned capabilityInfo) {
    static const auto cipherName = [](unsigned type) -> const char* {
        switch (type) {
        case 1:  return "WEP-40";
        case 2:  return "TKIP";
        case 4:  return "CCMP";
        case 5:  return "WEP-104";
        case 8:  return "GCMP-128";
        case 9:  return "GCMP-256";
        case 10: return "CCMP-256";
        default: return nullptr;
        }
    };
    static const auto akmName = [](unsigned type) -> const char* {
        switch (type) {
        case 1:  return "EAP";
        case 2:  return "PSK";
        case 3:  return "FT-EAP";
        case 4:  return "FT-PSK";
        case 6:  return "PSK-SHA256";
        case 5:  return "EAP-SHA256";
        case 8:  return "SAE";
        case 9:  return "FT-SAE";
        default: return nullptr;
        }
    };
    /* one security token (RSN or vendor-WPA): "[WPA2-PSK-CCMP+TKIP]" */
    const auto renderSecurity = [&](const unsigned char* body, size_t len,
                                    const char* prefix) -> std::string {
        /* body: version u16, group cipher 4, pairwise count u16, suites,
         * akm count u16, suites — every suite is OUI(3)+type(1). */
        std::vector<std::string> ciphers, akms;
        size_t pos = 2 + 4;
        if (pos + 2 > len) return std::string();
        unsigned count = getU16(body + pos);
        pos += 2;
        for (unsigned i = 0; i < count && pos + 4 <= len; i++, pos += 4) {
            const char* name = cipherName(body[pos + 3]);
            if (name) ciphers.push_back(name);
        }
        if (pos + 2 > len) return std::string();
        count = getU16(body + pos);
        pos += 2;
        for (unsigned i = 0; i < count && pos + 4 <= len; i++, pos += 4) {
            const char* name = akmName(body[pos + 3]);
            if (name) akms.push_back(name);
        }
        std::string token = prefix;
        if (!akms.empty()) { token += "-"; appendJoined(token, akms); }
        if (!ciphers.empty()) { token += "-"; appendJoined(token, ciphers); }
        return token;
    };

    std::string caps;
    if (const auto* rsn = findIe(ies, IE_RSN); rsn && rsn->bytes.size() >= 2) {
        const std::string token = renderSecurity(rsn->bytes.data(), rsn->bytes.size(), "WPA2");
        if (!token.empty()) caps += "[" + token + "]";
    }
    for (const auto& element : ies) {
        if (element.id != IE_VENDOR || element.bytes.size() < 4) continue;
        if (memcmp(element.bytes.data(), OUI_MICROSOFT_WPA, 4) != 0) continue;
        const std::string token = renderSecurity(element.bytes.data() + 4,
                element.bytes.size() - 4, "WPA");
        if (!token.empty()) caps += "[" + token + "]";
    }
    /* privacy with no WPA/RSN element = WEP (wpa_cli vocabulary order) */
    const bool wep = (caps.empty() && (capabilityInfo & 0x0010));
    if (wep) caps += "[WEP]";
    if (capabilityInfo & 0x0001) caps += "[ESS]";
    return caps;
}

void Nl80211Radio::applyScanEnhancements(ScanResult& result) {
    const auto& ies = result.informationElements;
    if (const auto* ssid = findIe(ies, IE_SSID)) {
        result.wifiSsid = WifiSsid::fromBytes(
                std::string(reinterpret_cast<const char*>(ssid->bytes.data()),
                            ssid->bytes.size()));
        result.SSID = result.wifiSsid.toString();
    }
    /* channel width: HT operation secondary offset then the VHT operation
     * width field (AOSP InformationElementUtil derivation). */
    if (const auto* htOp = findIe(ies, IE_HT_OP); htOp && htOp->bytes.size() >= 2) {
        if (((htOp->bytes[1] >> 2) & 0x3) != 0)
            result.channelWidth = ScanResult::CHANNEL_WIDTH_40MHZ;
    }
    if (const auto* vhtOp = findIe(ies, IE_VHT_OP); vhtOp && vhtOp->bytes.size() >= 3) {
        const int width = vhtOp->bytes[0];
        const int seg0 = vhtOp->bytes[1];
        const int seg1 = vhtOp->bytes[2];
        if (width == 1) {
            result.channelWidth = ScanResult::CHANNEL_WIDTH_80MHZ;
            result.centerFreq0 = seg0 * 5;
        } else if (width == 2) {
            result.channelWidth = ScanResult::CHANNEL_WIDTH_160MHZ;
            result.centerFreq0 = seg1 * 5;  /* seg1 carries the 160MHz center */
        } else if (width == 3 && seg0 != 0 && seg1 != 0) {
            result.channelWidth = ScanResult::CHANNEL_WIDTH_80MHZ_PLUS_MHZ;
            result.centerFreq0 = seg0 * 5;
            result.centerFreq1 = seg1 * 5;
        }
    }
    /* standard from the presence of capability elements, newest first */
    if (findIe(ies, IE_EHT_CAP)) result.mWifiStandard = ScanResult::WIFI_STANDARD_11BE;
    else if (findIe(ies, IE_HE_CAP)) result.mWifiStandard = ScanResult::WIFI_STANDARD_11AX;
    else if (findIe(ies, IE_VHT_CAP)) result.mWifiStandard = ScanResult::WIFI_STANDARD_11AC;
    else if (findIe(ies, IE_HT_CAP)) result.mWifiStandard = ScanResult::WIFI_STANDARD_11N;
    else result.mWifiStandard = ScanResult::WIFI_STANDARD_LEGACY;
}

/* --- generic netlink client --------------------------------------------------- */

Nl80211Radio::Nl80211Radio() = default;

Nl80211Radio::~Nl80211Radio() {
    if (mSock >= 0) close(mSock);
}

int Nl80211Radio::resolveFamilyId() {
    unsigned char buffer[512];
    memset(buffer, 0, sizeof(buffer));
    auto* nlh = reinterpret_cast<struct nlmsghdr*>(buffer);
    nlh->nlmsg_type = GENL_ID_CTRL;
    nlh->nlmsg_flags = NLM_F_REQUEST;
    auto* genl = static_cast<struct genlmsghdr*>(NLMSG_DATA(nlh));
    genl->cmd = CTRL_CMD_GETFAMILY;
    genl->version = 1;
    size_t offset = NLMSG_ALIGN(NLMSG_LENGTH(GENL_HDRLEN));
    auto* rta = reinterpret_cast<struct rtattr*>(buffer + offset);
    rta->rta_type = CTRL_ATTR_FAMILY_NAME;
    rta->rta_len = RTA_LENGTH(strlen("nl80211") + 1);
    memcpy(RTA_DATA(rta), "nl80211", strlen("nl80211") + 1);
    offset += RTA_ALIGN(rta->rta_len);
    nlh->nlmsg_len = static_cast<unsigned int>(offset);
    if (send(mSock, buffer, offset, 0) < 0) return -1;
    const ssize_t len = recv(mSock, buffer, sizeof(buffer), 0);
    if (len <= 0) return -1;
    const auto* reply = reinterpret_cast<const struct nlmsghdr*>(buffer);
    if (reply->nlmsg_type == NLMSG_ERROR) return -1;
    const auto* replyGenl = static_cast<const struct genlmsghdr*>(NLMSG_DATA(reply));
    const size_t attrLen = reply->nlmsg_len - NLMSG_HDRLEN - GENL_HDRLEN;
    const auto* idAttr = findAttr(replyGenl + 1, attrLen, CTRL_ATTR_FAMILY_ID);
    if (!idAttr || RTA_PAYLOAD(idAttr) < 2) return -1;
    return getU16(static_cast<const unsigned char*>(RTA_DATA(idAttr)));
}

bool Nl80211Radio::ensureStarted() {
    if (mFamilyId > 0) return true;
    if (mSock < 0) {
        mSock = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_GENERIC);
        if (mSock < 0) return false;
        struct sockaddr_nl local;
        memset(&local, 0, sizeof(local));
        local.nl_family = AF_NETLINK;
        if (bind(mSock, reinterpret_cast<struct sockaddr*>(&local), sizeof(local)) != 0) {
            close(mSock);
            mSock = -1;
            return false;
        }
    }
    mFamilyId = resolveFamilyId();
    if (mFamilyId <= 0) {
        RADIO_LOGE("cannot resolve nl80211 family (%s)", strerror(errno));
        return false;
    }
    return true;
}

std::vector<ScanResult> Nl80211Radio::getScanResults(const std::string& iface) {
    std::vector<ScanResult> results;
    if (!ensureStarted()) return results;
    const unsigned int ifindex = if_nametoindex(iface.c_str());
    if (ifindex == 0) return results;

    /* NL80211_CMD_GET_SCAN dump request */
    unsigned char request[128];
    memset(request, 0, sizeof(request));
    auto* nlh = reinterpret_cast<struct nlmsghdr*>(request);
    nlh->nlmsg_type = static_cast<unsigned short>(mFamilyId);
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    auto* genl = static_cast<struct genlmsghdr*>(NLMSG_DATA(nlh));
    genl->cmd = NL80211_CMD_GET_SCAN;
    genl->version = 1;
    size_t offset = NLMSG_ALIGN(NLMSG_LENGTH(GENL_HDRLEN));
    auto* rta = reinterpret_cast<struct rtattr*>(request + offset);
    rta->rta_type = NL80211_ATTR_IFINDEX;
    rta->rta_len = RTA_LENGTH(sizeof(unsigned int));
    memcpy(RTA_DATA(rta), &ifindex, sizeof(ifindex));
    offset += RTA_ALIGN(rta->rta_len);
    nlh->nlmsg_len = static_cast<unsigned int>(offset);
    if (send(mSock, request, offset, 0) < 0) {
        RADIO_LOGE("GET_SCAN send: %s (unprivileged dump denied?)", strerror(errno));
        return results;
    }

    unsigned char buffer[8192];
    while (true) {
        const ssize_t len = recv(mSock, buffer, sizeof(buffer), 0);
        if (len <= 0) break;
        int remaining = static_cast<int>(len);
        bool done = false;
        for (const struct nlmsghdr* msg = reinterpret_cast<const struct nlmsghdr*>(buffer);
             NLMSG_OK(msg, remaining) && !done; msg = NLMSG_NEXT(msg, remaining)) {
            if (msg->nlmsg_type == NLMSG_DONE || msg->nlmsg_type == NLMSG_ERROR) {
                done = true;
                break;
            }
            const auto* scanGenl = static_cast<const struct genlmsghdr*>(NLMSG_DATA(msg));
            const size_t attrLen = msg->nlmsg_len - NLMSG_HDRLEN - GENL_HDRLEN;
            const auto* bssAttr = findAttr(scanGenl + 1, attrLen, NL80211_ATTR_BSS);
            if (!bssAttr) continue;
            const size_t bssLen = RTA_PAYLOAD(bssAttr);
            const auto* bssData = RTA_DATA(bssAttr);

            const auto* mac = findAttr(bssData, bssLen, NL80211_BSS_BSSID);
            const auto* freq = findAttr(bssData, bssLen, NL80211_BSS_FREQUENCY);
            const auto* tsf = findAttr(bssData, bssLen, NL80211_BSS_TSF);
            const auto* capab = findAttr(bssData, bssLen, NL80211_BSS_CAPABILITY);
            const auto* ies = findAttr(bssData, bssLen, NL80211_BSS_INFORMATION_ELEMENTS);
            const auto* signal = findAttr(bssData, bssLen, NL80211_BSS_SIGNAL_MBM);
            if (!mac || RTA_PAYLOAD(mac) < 6) continue;

            ScanResult result;
            result.BSSID = macToString(static_cast<const unsigned char*>(RTA_DATA(mac)));
            if (freq && RTA_PAYLOAD(freq) >= 4) {
                /* u32 MHz per the uapi */
                uint32_t mhz = 0;
                memcpy(&mhz, RTA_DATA(freq), sizeof(mhz));
                result.frequency = (int) mhz;
            }
            if (tsf && RTA_PAYLOAD(tsf) >= 8) {
                uint64_t stamp = 0;
                memcpy(&stamp, RTA_DATA(tsf), sizeof(stamp));
                result.timestamp = (int64_t) stamp;
            }
            if (signal && RTA_PAYLOAD(signal) >= 4) {
                int32_t mbm = 0;
                memcpy(&mbm, RTA_DATA(signal), sizeof(mbm));
                result.level = mbm / 100;
            }
            unsigned capabilityInfo = 0;
            if (capab && RTA_PAYLOAD(capab) >= 2) {
                capabilityInfo = getU16(static_cast<const unsigned char*>(RTA_DATA(capab)));
            }
            if (ies && RTA_PAYLOAD(ies) > 0) {
                result.informationElements = parseIeStream(
                        static_cast<const unsigned char*>(RTA_DATA(ies)), RTA_PAYLOAD(ies));
            }
            result.capabilities = buildCapabilitiesString(result.informationElements,
                                                           capabilityInfo);
            applyScanEnhancements(result);
            result.ifaceName = iface;
            results.push_back(result);
        }
        if (done) break;
    }
    return results;
}

WifiRadioData* WifiRadioData::create() {
    return new Nl80211Radio();
}

} // namespace cdroid
