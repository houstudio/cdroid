/*
 * nettests — self-contained pure-logic tests for the cdnet stack: address
 * math, config serialization, state maps, route-table hex parsing. No
 * privileges and no interfaces required. Exit code = failed checks.
 */
#include <stdio.h>
#include <sstream>
#include <string>
#include <vector>

#include <arpa/inet.h>
#include <linux/rtnetlink.h>
#include <net/if.h>

#include <connectivitymanager.h>
#include <dhcpclient.h>
#include <dhcpinfo.h>
#include <ethernet/ethernetmanager.h>
#include <netlinkmonitor.h>
#include <wifi/nl80211radio.h>
#include <ipconfiguration.h>
#include <linkaddress.h>
#include <networkcapabilities.h>
#include <networkinfo.h>
#include <wifi/supplicantstate.h>
#include <wifi/wifiinfo.h>

using cdroid::ConnectivityManager;
using cdroid::DhcpClient;
using cdroid::DhcpInfo;
using cdroid::EthernetManager;
using cdroid::IpConfiguration;
using cdroid::LinkAddress;
using cdroid::NetworkCapabilities;
using cdroid::NetworkInfo;
using cdroid::NetlinkMonitor;
using cdroid::Nl80211Radio;
using cdroid::ScanResult;
using cdroid::StaticIpConfiguration;
using cdroid::SupplicantState;
using cdroid::WifiInfo;

static int gFailures = 0;
static int gChecks = 0;

#define CHECK(cond) do { \
    gChecks++; \
    if (!(cond)) { gFailures++; fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); } \
} while (0)

template <typename T>
static std::string describeValue(const T& v) {
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

#define CHECK_EQ(actual, expected) do { \
    gChecks++; \
    const auto& a_ = (actual); \
    const auto& e_ = (expected); \
    if (!(a_ == e_)) { \
        gFailures++; \
        fprintf(stderr, "FAIL %s:%d: %s == %s (got '%s', want '%s')\n", \
                __FILE__, __LINE__, #actual, #expected, \
                describeValue(a_).c_str(), describeValue(e_).c_str()); \
    } \
} while (0)

static void testLinkAddress() {
    const LinkAddress parsed("192.168.1.5/24");
    CHECK_EQ(parsed.getAddress(), std::string("192.168.1.5"));
    CHECK_EQ(parsed.getPrefixLength(), 24);
    CHECK_EQ(parsed.toString(), std::string("192.168.1.5/24"));
    CHECK(parsed.isIpv4());

    const LinkAddress bare("10.0.0.1");
    CHECK_EQ(bare.getPrefixLength(), 32);

    CHECK_EQ(LinkAddress::prefixLengthToNetmaskV4(24), std::string("255.255.255.0"));
    CHECK_EQ(LinkAddress::prefixLengthToNetmaskV4(0), std::string("0.0.0.0"));
    CHECK_EQ(LinkAddress::prefixLengthToNetmaskV4(32), std::string("255.255.255.255"));
    CHECK_EQ(LinkAddress::prefixLengthToNetmaskV4(33), std::string(""));

    CHECK_EQ(LinkAddress::netmaskToPrefixLengthV4("255.255.255.0"), 24);
    CHECK_EQ(LinkAddress::netmaskToPrefixLengthV4("255.255.0.0"), 16);
    CHECK_EQ(LinkAddress::netmaskToPrefixLengthV4("0.0.0.0"), 0);
    CHECK_EQ(LinkAddress::netmaskToPrefixLengthV4("255.0.255.0"), -1); /* non-contiguous */
}

static void testDhcpInfo() {
    /* little-endian int form: 192.168.1.5 -> 0x0501A8C0 */
    const int encoded = DhcpInfo::stringToInt("192.168.1.5");
    CHECK_EQ(encoded, (int) 0x0501A8C0);
    CHECK_EQ(DhcpInfo::intToStr(encoded), std::string("192.168.1.5"));
    DhcpInfo info;
    info.ipAddress = encoded;
    info.gateway = DhcpInfo::stringToInt("192.168.1.1");
    info.leaseDuration = 86400;
    const std::string text = info.toString();
    CHECK(text.find("ipaddr 192.168.1.5") != std::string::npos);
    CHECK(text.find("gateway 192.168.1.1") != std::string::npos);
    CHECK(text.find("lease 86400 seconds") != std::string::npos);
}

static void testNetworkInfo() {
    NetworkInfo info((int) ConnectivityManager::TYPE_WIFI);
    CHECK_EQ((int) info.getState(), (int) NetworkInfo::State::DISCONNECTED); /* ctor IDLE */
    CHECK(!info.isConnected());

    info.setIsAvailable(true);
    info.setDetailedState(NetworkInfo::DetailedState::CONNECTED, std::string(), "\"ssid\"");
    CHECK(info.isConnected());
    CHECK(info.isConnectedOrConnecting());
    CHECK_EQ(info.getTypeName(), std::string("WIFI"));
    CHECK_EQ((int) NetworkInfo::stateFromDetailedState(NetworkInfo::DetailedState::OBTAINING_IPADDR),
             (int) NetworkInfo::State::CONNECTING);
    CHECK_EQ((int) NetworkInfo::stateFromDetailedState(NetworkInfo::DetailedState::DISCONNECTED),
             (int) NetworkInfo::State::DISCONNECTED);

    const std::string text = info.toString();
    CHECK(text.find("type: WIFI[]") != std::string::npos);
    CHECK(text.find("state: CONNECTED/CONNECTED") != std::string::npos);
    CHECK(text.find("available: true") != std::string::npos);
    /* AOSP: isConnected() is purely mState — setDetailedState(CONNECTED)
     * alone suffices, no availability flag gate. */
    NetworkInfo plain((int) ConnectivityManager::TYPE_WIFI);
    plain.setDetailedState(NetworkInfo::DetailedState::CONNECTED, std::string(), std::string());
    CHECK(plain.isConnected());
    CHECK_EQ((int) NetworkInfo::stateFromDetailedState(NetworkInfo::DetailedState::DISCONNECTING),
             (int) NetworkInfo::State::DISCONNECTING);

    NetworkInfo eth((int) ConnectivityManager::TYPE_ETHERNET);
    CHECK_EQ(eth.getTypeName(), std::string("ETHERNET"));
}

static void testSupplicantMapping() {
    /* AOSP WifiInfo.stateMap, verbatim expectations */
    CHECK_EQ((int) WifiInfo::getDetailedStateOf(SupplicantState::COMPLETED),
             (int) NetworkInfo::DetailedState::OBTAINING_IPADDR);
    CHECK_EQ((int) WifiInfo::getDetailedStateOf(SupplicantState::FOUR_WAY_HANDSHAKE),
             (int) NetworkInfo::DetailedState::AUTHENTICATING);
    CHECK_EQ((int) WifiInfo::getDetailedStateOf(SupplicantState::SCANNING),
             (int) NetworkInfo::DetailedState::SCANNING);
    CHECK_EQ((int) WifiInfo::getDetailedStateOf(SupplicantState::INVALID),
             (int) NetworkInfo::DetailedState::FAILED);
    CHECK_EQ((int) WifiInfo::getDetailedStateOf(SupplicantState::INACTIVE),
             (int) NetworkInfo::DetailedState::IDLE);
}

static void testStaticIpAndIpConfiguration() {
    StaticIpConfiguration staticIp;
    staticIp.setIpAddress(LinkAddress("192.168.10.20/24"))
            .setGateway("192.168.10.1")
            .setDnsServers({"8.8.8.8", "8.8.4.4"})
            .setDomains("example.com");
    CHECK_EQ(staticIp.getIpAddress().toString(), std::string("192.168.10.20/24"));
    CHECK_EQ(staticIp.getGateway(), std::string("192.168.10.1"));
    CHECK_EQ(staticIp.getDnsServers().size(), (size_t) 2);
    const std::string text = staticIp.toString();
    CHECK(text.find("192.168.10.20/24") != std::string::npos);
    CHECK(text.find("8.8.8.8") != std::string::npos);

    IpConfiguration config;
    config.setIpAssignment(IpConfiguration::IpAssignment::STATIC);
    config.setProxySettings(IpConfiguration::ProxySettings::NONE);
    config.setStaticIpConfiguration(staticIp);
    CHECK(config.toString().find("IP assignment: STATIC") != std::string::npos);
    CHECK(config.toString().find("Proxy settings: NONE") != std::string::npos);

    IpConfiguration copy;
    copy.setIpAssignment(IpConfiguration::IpAssignment::STATIC);
    copy.setProxySettings(IpConfiguration::ProxySettings::NONE);
    copy.setStaticIpConfiguration(staticIp);
    CHECK(config == copy);
    copy.setIpAssignment(IpConfiguration::IpAssignment::DHCP);
    CHECK(!(config == copy));

    staticIp.clear();
    CHECK_EQ(staticIp.getDnsServers().size(), (size_t) 0);
    CHECK(staticIp.getGateway().empty());
}

static void testNetworkCapabilities() {
    NetworkCapabilities caps;
    caps.addTransport(NetworkCapabilities::TRANSPORT_ETHERNET);
    caps.addCapability(NetworkCapabilities::NET_CAPABILITY_INTERNET);
    caps.addCapability(NetworkCapabilities::NET_CAPABILITY_TRUSTED);
    CHECK(caps.hasTransport(NetworkCapabilities::TRANSPORT_ETHERNET));
    CHECK(!caps.hasTransport(NetworkCapabilities::TRANSPORT_WIFI));
    CHECK(caps.hasCapability(NetworkCapabilities::NET_CAPABILITY_INTERNET));
    caps.removeCapability(NetworkCapabilities::NET_CAPABILITY_TRUSTED);
    CHECK(!caps.hasCapability(NetworkCapabilities::NET_CAPABILITY_TRUSTED));
    CHECK(caps.toString().find("ETHERNET") != std::string::npos);
    NetworkCapabilities same;
    same.addTransport(NetworkCapabilities::TRANSPORT_ETHERNET);
    same.addCapability(NetworkCapabilities::NET_CAPABILITY_INTERNET);
    CHECK(caps == same);
}

static void testRouteParsing() {
    /* /proc/net/route columns are little-endian hex */
    CHECK_EQ(EthernetManager::parseHexLittleEndianAddress("0003A8C0"),
             std::string("192.168.3.0"));
    CHECK_EQ(EthernetManager::parseHexLittleEndianAddress("0100A8C0"),
             std::string("192.168.0.1"));
    CHECK_EQ(EthernetManager::parseHexLittleEndianAddress("00000000"),
             std::string("0.0.0.0"));
    CHECK(EthernetManager::parseHexLittleEndianAddress("XYZ").empty());
    CHECK(EthernetManager::parseHexLittleEndianAddress("0003A8").empty());
}

static void testConfigurationSerialization() {
    IpConfiguration config;
    config.setIpAssignment(IpConfiguration::IpAssignment::STATIC);
    config.setProxySettings(IpConfiguration::ProxySettings::NONE);
    StaticIpConfiguration staticIp;
    staticIp.setIpAddress(LinkAddress("10.1.2.3/16"))
            .setGateway("10.1.0.1")
            .setDnsServers({"1.1.1.1", "1.0.0.1"});
    config.setStaticIpConfiguration(staticIp);

    const std::string text = EthernetManager::serializeConfiguration(config);
    IpConfiguration parsed;
    CHECK(EthernetManager::parseConfiguration(text, parsed));
    CHECK_EQ((int) parsed.getIpAssignment(), (int) IpConfiguration::IpAssignment::STATIC);
    CHECK_EQ((int) parsed.getProxySettings(), (int) IpConfiguration::ProxySettings::NONE);
    CHECK_EQ(parsed.getStaticIpConfiguration().getIpAddress().toString(),
             std::string("10.1.2.3/16"));
    CHECK_EQ(parsed.getStaticIpConfiguration().getGateway(), std::string("10.1.0.1"));
    CHECK_EQ(parsed.getStaticIpConfiguration().getDnsServers().size(), (size_t) 2);
    CHECK_EQ(parsed.getStaticIpConfiguration().getDnsServers()[0], std::string("1.1.1.1"));

    IpConfiguration dhcp;
    dhcp.setIpAssignment(IpConfiguration::IpAssignment::DHCP);
    dhcp.setProxySettings(IpConfiguration::ProxySettings::NONE);
    IpConfiguration dhcpParsed;
    CHECK(EthernetManager::parseConfiguration(
            EthernetManager::serializeConfiguration(dhcp), dhcpParsed));
    CHECK_EQ((int) dhcpParsed.getIpAssignment(), (int) IpConfiguration::IpAssignment::DHCP);
}

static void testNetlinkRouteMessage() {
    /* pure builder: RTM_NEWROUTE default-route message for loopback */
    const unsigned int loIndex = if_nametoindex("lo");
    if (loIndex == 0) return; /* not on Linux — skip */
    unsigned char buffer[128];
    const size_t len = NetlinkMonitor::buildDefaultRouteMessage(
            "lo", "127.0.0.1", buffer, sizeof(buffer));
    CHECK(len > 0);
    const auto* nlh = reinterpret_cast<const struct nlmsghdr*>(buffer);
    CHECK_EQ((int) nlh->nlmsg_type, (int) RTM_NEWROUTE);
    CHECK_EQ((int) nlh->nlmsg_len, (int) len);
    CHECK((nlh->nlmsg_flags & NLM_F_CREATE) != 0);
    CHECK((nlh->nlmsg_flags & NLM_F_ACK) != 0);
    const auto* rtm = static_cast<const struct rtmsg*>(NLMSG_DATA(nlh));
    CHECK_EQ((int) rtm->rtm_family, (int) AF_INET);
    CHECK_EQ((int) rtm->rtm_table, (int) RT_TABLE_MAIN);
    CHECK_EQ((int) rtm->rtm_type, (int) RTN_UNICAST);
    bool sawGateway = false, sawOif = false;
    size_t attrLen = RTM_PAYLOAD(nlh);
    for (const struct rtattr* rta = RTM_RTA(rtm); RTA_OK(rta, attrLen);
         rta = RTA_NEXT(rta, attrLen)) {
        if (rta->rta_type == RTA_GATEWAY) {
            sawGateway = true;
            struct in_addr expected;
            inet_pton(AF_INET, "127.0.0.1", &expected);
            CHECK(memcmp(RTA_DATA(rta), &expected, sizeof(expected)) == 0);
        } else if (rta->rta_type == RTA_OIF) {
            sawOif = true;
            CHECK_EQ(*reinterpret_cast<const unsigned int*>(RTA_DATA(rta)), loIndex);
        }
    }
    CHECK(sawGateway && sawOif);
    /* invalid arguments produce no message */
    CHECK(NetlinkMonitor::buildDefaultRouteMessage(
            "lo", "not-an-ip", buffer, sizeof(buffer)) == 0);
    /* undersized buffer: rejected before any write (48 bytes needed) */
    unsigned char small[32];
    CHECK(NetlinkMonitor::buildDefaultRouteMessage(
            "lo", "127.0.0.1", small, sizeof(small)) == 0);
}

static void testNl80211IeHelpers() {
    using IE = ScanResult::InformationElement;
    /* parseIeStream: id/len records + truncated tail dropped */
    const unsigned char stream[] = {0x00, 0x04, 't', 'e', 's', 't', 0x03, 0x01, 0x06};
    const auto elements = Nl80211Radio::parseIeStream(stream, sizeof(stream));
    CHECK_EQ(elements.size(), (size_t) 2);
    CHECK_EQ(elements[0].id, 0);
    CHECK_EQ(elements[0].bytes.size(), (size_t) 4);
    const unsigned char truncated[] = {0x00, 0x04, 'a'};
    CHECK_EQ(Nl80211Radio::parseIeStream(truncated, sizeof(truncated)).size(), (size_t) 0);

    /* capabilities: RSN PSK/CCMP with the ESS bit */
    std::vector<IE> ies;
    IE rsn;
    rsn.id = 48;
    const unsigned char rsnBody[] = {0x01, 0x00, 0x00, 0x0F, 0xAC, 0x04, 0x01, 0x00,
                                     0x00, 0x0F, 0xAC, 0x04, 0x01, 0x00, 0x00, 0x0F, 0xAC, 0x02};
    rsn.bytes.assign(rsnBody, rsnBody + sizeof(rsnBody));
    ies.push_back(rsn);
    CHECK_EQ(Nl80211Radio::buildCapabilitiesString(ies, 0x0001),
             std::string("[WPA2-PSK-CCMP][ESS]"));
    /* privacy-only legacy AP renders WEP */
    CHECK_EQ(Nl80211Radio::buildCapabilitiesString({}, 0x0011), std::string("[WEP][ESS]"));
    CHECK_EQ(Nl80211Radio::buildCapabilitiesString({}, 0x0010), std::string("[WEP]"));

    /* enhancements: SSID + HT 40MHz + standard from HT cap */
    ScanResult result;
    IE ssid;
    ssid.id = 0;
    const char ssidText[] = "hwsim";
    ssid.bytes.assign(ssidText, ssidText + 5);
    IE htOp;
    htOp.id = 61;
    const unsigned char htOpBody[] = {0x06, 0x04, 0x00}; /* ch6, secondary above */
    htOp.bytes.assign(htOpBody, htOpBody + sizeof(htOpBody));
    IE htCap;
    htCap.id = 45;
    ies.clear();
    ies.push_back(ssid);
    ies.push_back(htOp);
    ies.push_back(htCap);
    result.informationElements = ies;
    Nl80211Radio::applyScanEnhancements(result);
    CHECK_EQ(result.wifiSsid.getUtf8Text(), std::string("hwsim"));
    CHECK_EQ(result.channelWidth, (int) ScanResult::CHANNEL_WIDTH_40MHZ);
    CHECK_EQ(result.mWifiStandard, (int) ScanResult::WIFI_STANDARD_11N);

    /* VHT 80MHz: centerFreq0 = seg0 * 5, standard 11AC */
    ScanResult vht;
    IE vhtOp;
    vhtOp.id = 192;
    const unsigned char vhtBody[] = {0x01, 200, 0};
    vhtOp.bytes.assign(vhtBody, vhtBody + sizeof(vhtBody));
    IE vhtCap;
    vhtCap.id = 191;
    std::vector<IE> vhtIes{vhtOp, vhtCap};
    vht.informationElements = vhtIes;
    Nl80211Radio::applyScanEnhancements(vht);
    CHECK_EQ(vht.channelWidth, (int) ScanResult::CHANNEL_WIDTH_80MHZ);
    CHECK_EQ(vht.centerFreq0, 1000);
    CHECK_EQ(vht.mWifiStandard, (int) ScanResult::WIFI_STANDARD_11AC);

    /* HE/EHT capabilities are extension elements: 255 + ext id (HE=35,
     * EHT=106); element 232 is S1G Operation and must not classify as HE. */
    IE heCap;
    heCap.id = 255;
    const unsigned char heBody[] = {35, 0x0f, 0x01, 0x00};
    heCap.bytes.assign(heBody, heBody + sizeof(heBody));
    ScanResult he;
    std::vector<IE> heIes{ssid, heCap};
    he.informationElements = heIes;
    Nl80211Radio::applyScanEnhancements(he);
    CHECK_EQ(he.mWifiStandard, (int) ScanResult::WIFI_STANDARD_11AX);

    IE ehtCap;
    ehtCap.id = 255;
    const unsigned char ehtBody[] = {106, 0x08, 0x00};
    ehtCap.bytes.assign(ehtBody, ehtBody + sizeof(ehtBody));
    ScanResult eht;
    std::vector<IE> ehtIes{ssid, ehtCap};
    eht.informationElements = ehtIes;
    Nl80211Radio::applyScanEnhancements(eht);
    CHECK_EQ(eht.mWifiStandard, (int) ScanResult::WIFI_STANDARD_11BE);

    IE s1g;
    s1g.id = 232;   /* S1G Operation: not HE */
    const unsigned char s1gBody[] = {0x00};
    s1g.bytes.assign(s1gBody, s1gBody + sizeof(s1gBody));
    ScanResult legacy;
    std::vector<IE> legacyIes{ssid, s1g};
    legacy.informationElements = legacyIes;
    Nl80211Radio::applyScanEnhancements(legacy);
    CHECK_EQ(legacy.mWifiStandard, (int) ScanResult::WIFI_STANDARD_LEGACY);
}

static void testDhcpCodecs() {
    const unsigned char mac[6] = {0x02, 0x00, 0x00, 0x12, 0x34, 0x56};

    /* DISCOVER: fixed header + option 53=1 + PRL + END */
    const auto discover = DhcpClient::buildDiscover(0xAABBCCDD, mac, sizeof(mac));
    CHECK(discover.size() >= 247);
    CHECK_EQ((int) discover[0], 1);            /* op BOOTREQUEST */
    CHECK_EQ((int) discover[1], 1);            /* htype ethernet */
    CHECK_EQ((int) discover[2], 6);            /* hlen */
    CHECK_EQ((uint32_t)((discover[4] << 24) | (discover[5] << 16) | (discover[6] << 8) | discover[7]),
             (uint32_t) 0xAABBCCDD);
    CHECK_EQ((int)(discover[10] << 8 | discover[11]), 0x8000);  /* broadcast flag */
    CHECK(memcmp(&discover[28], mac, 6) == 0);
    CHECK_EQ((int) discover[244], 53);         /* first option: msg type */
    CHECK_EQ((int) discover[246], (int) DhcpClient::DHCP_DISCOVER);
    CHECK_EQ((int) discover.back(), 255);      /* END */
    bool sawPrl = false;
    for (size_t i = 244; i + 1 < discover.size(); i++)
        if (discover[i] == 55 && discover[i + 1] == 8) sawPrl = true;
    CHECK(sawPrl);

    /* REQUEST (selecting): carries 50 + 54 */
    const auto request = DhcpClient::buildRequest(0xAABBCCDD, mac, sizeof(mac),
            "192.168.77.51", "192.168.77.1", false);
    bool saw50 = false, saw54 = false;
    for (size_t i = 244; i + 5 < request.size();) {
        const int code = request[i];
        const int len = request[i + 1];
        if (code == 50 && len == 4 &&
                request[i + 2] == 192 && request[i + 5] == 51) saw50 = true;
        if (code == 54 && len == 4 &&
                request[i + 2] == 192 && request[i + 5] == 1) saw54 = true;
        if (code == 0) { i++; continue; }
        if (code == 255) break;
        i += 2 + len;
    }
    CHECK(saw50 && saw54);

    /* synthetic OFFER -> parseReply */
    std::vector<unsigned char> offer(244, 0);
    offer[0] = 2;                              /* BOOTREPLY */
    offer[1] = 1;
    offer[2] = 6;
    const uint32_t xid = htonl(0x11223344);
    memcpy(&offer[4], &xid, 4);
    const unsigned char yiaddr[4] = {192, 168, 77, 51};
    memcpy(&offer[16], yiaddr, 4);
    const unsigned char siaddr[4] = {192, 168, 77, 1};
    memcpy(&offer[20], siaddr, 4);
    memcpy(&offer[28], mac, 6);
    const uint32_t cookie = htonl(0x63825363);
    memcpy(&offer[236], &cookie, 4);
    const auto appendOpt = [&](int code, const std::vector<unsigned char>& value) {
        offer.push_back((unsigned char) code);
        offer.push_back((unsigned char) value.size());
        offer.insert(offer.end(), value.begin(), value.end());
    };
    const auto be32 = [](uint32_t v) {
        return std::vector<unsigned char>{(unsigned char)(v >> 24), (unsigned char)(v >> 16),
                                         (unsigned char)(v >> 8), (unsigned char) v};
    };
    appendOpt(53, {2});
    appendOpt(54, {192, 168, 77, 1});
    appendOpt(51, be32(43200));
    appendOpt(1, {255, 255, 255, 0});
    appendOpt(3, {192, 168, 77, 1});
    appendOpt(6, {8, 8, 8, 8, 8, 8, 4, 4});
    appendOpt(0, {});                          /* pad skipped by the walker */
    appendOpt(58, be32(21600));
    appendOpt(59, be32(37800));
    offer.push_back(255);

    DhcpClient::Lease lease;
    CHECK_EQ(DhcpClient::parseReply(offer.data(), offer.size(), 0x11223344, mac, 6, lease),
             (int) DhcpClient::DHCP_OFFER);
    CHECK_EQ(lease.ipAddress, std::string("192.168.77.51"));
    CHECK_EQ(lease.serverId, std::string("192.168.77.1"));
    CHECK_EQ(lease.netmask, std::string("255.255.255.0"));
    CHECK_EQ(lease.gateway, std::string("192.168.77.1"));
    CHECK_EQ(lease.dnsServers.size(), (size_t) 2);
    CHECK_EQ(lease.dnsServers[1], std::string("8.8.4.4"));
    CHECK_EQ((int) lease.leaseDurationSec, 43200);
    CHECK_EQ((int) lease.t1Sec, 21600);
    CHECK_EQ((int) lease.t2Sec, 37800);

    /* rejections: xid mismatch / not a reply / truncated */
    CHECK_EQ(DhcpClient::parseReply(offer.data(), offer.size(), 0x99999999, mac, 6, lease), 0);
    offer[0] = 1;
    CHECK_EQ(DhcpClient::parseReply(offer.data(), offer.size(), 0x11223344, mac, 6, lease), 0);
    offer[0] = 2;
    CHECK_EQ(DhcpClient::parseReply(offer.data(), 100, 0x11223344, mac, 6, lease), 0);

    /* NAK */
    std::vector<unsigned char> nak(244, 0);
    nak[0] = 2;
    nak[1] = 1;
    memcpy(&nak[4], &xid, 4);
    memcpy(&nak[28], mac, 6);
    memcpy(&nak[236], &cookie, 4);
    nak.push_back(53); nak.push_back(1); nak.push_back(6);
    nak.push_back(255);
    CHECK_EQ(DhcpClient::parseReply(nak.data(), nak.size(), 0x11223344, mac, 6, lease),
             (int) DhcpClient::DHCP_NAK);

    /* Lease -> DhcpInfo little-endian int fields */
    const DhcpInfo info = lease.toDhcpInfo();  /* last parse was the NAK (empty) */
    (void) info;
    DhcpClient::Lease bound;
    bound.ipAddress = "192.168.77.51";
    bound.netmask = "255.255.255.0";
    bound.gateway = "192.168.77.1";
    bound.serverId = "192.168.77.1";
    bound.dnsServers = {"8.8.8.8", "8.8.4.4"};
    bound.leaseDurationSec = 43200;
    const DhcpInfo filled = bound.toDhcpInfo();
    CHECK_EQ(filled.ipAddress, DhcpInfo::stringToInt("192.168.77.51"));
    CHECK_EQ(filled.netmask, DhcpInfo::stringToInt("255.255.255.0"));
    CHECK_EQ(filled.gateway, DhcpInfo::stringToInt("192.168.77.1"));
    CHECK_EQ(filled.dns1, DhcpInfo::stringToInt("8.8.8.8"));
    CHECK_EQ(filled.leaseDuration, 43200);
}

int main() {
    testLinkAddress();
    testDhcpInfo();
    testNetworkInfo();
    testSupplicantMapping();
    testStaticIpAndIpConfiguration();
    testNetworkCapabilities();
    testRouteParsing();
    testConfigurationSerialization();
    testNetlinkRouteMessage();
    testDhcpCodecs();
    testNl80211IeHelpers();
    printf("%d checks, %d failures\n", gChecks, gFailures);
    return gFailures;
}
