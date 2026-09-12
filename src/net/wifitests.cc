/*
 * wifitests — self-contained pure-logic tests for the cdwifi stack: reply
 * parsers, WifiSsid codec, signal-level math, auth-type mapping. No
 * wpa_supplicant needed. Exit code = number of failed checks.
 */
#include <stdio.h>
#include <sstream>
#include <string>

#include <wifi/supplicantclient.h>
#include <wifi/supplicantstate.h>
#include <wifi/wifimanager.h>
#include <wifi/wparesponseparser.h>

using cdroid::ScanResult;
using cdroid::SupplicantClient;
using cdroid::SupplicantEvent;
using cdroid::SupplicantState;
using cdroid::WifiConfiguration;
using cdroid::WifiInfo;
using cdroid::WifiManager;
using cdroid::WifiSsid;
using cdroid::WpaResponseParser;

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

static void testParseEventMessage() {
    SupplicantEvent event = SupplicantClient::parseEventMessage(
            "<3>CTRL-EVENT-STATE-CHANGE id=0 state=COMPLETED BSSID=aa:bb:cc:dd:ee:ff SSID=\"my net\"");
    CHECK_EQ(event.name, std::string("CTRL-EVENT-STATE-CHANGE"));
    CHECK_EQ(event.args.at("id"), std::string("0"));
    CHECK_EQ(event.args.at("state"), std::string("COMPLETED"));
    CHECK_EQ(event.args.at("BSSID"), std::string("aa:bb:cc:dd:ee:ff"));
    /* quoted values keep their quotes and embedded spaces */
    CHECK_EQ(event.args.at("SSID"), std::string("\"my net\""));

    event = SupplicantClient::parseEventMessage(
            "<3>CTRL-EVENT-CONNECTED - Connection to 00:11:22:33:44:55 completed [id=1 id_str=]");
    CHECK_EQ(event.name, std::string("CTRL-EVENT-CONNECTED"));
    CHECK_EQ(event.args.at("id"), std::string("1"));
    CHECK(event.args.find("id_str") != event.args.end()); /* empty value kept */

    event = SupplicantClient::parseEventMessage("<3>CTRL-EVENT-SCAN-RESULTS");
    CHECK_EQ(event.name, std::string("CTRL-EVENT-SCAN-RESULTS"));
    CHECK(event.args.empty());
}

static void testWifiSsid() {
    const WifiSsid quoted = WifiSsid::fromString("\"office\"");
    CHECK_EQ(quoted.toString(), std::string("\"office\""));
    CHECK_EQ(quoted.getUtf8Text(), std::string("office"));

    /* non-UTF8 bytes take the unquoted hex path, lowercase like AOSP */
    const WifiSsid hex = WifiSsid::fromBytes(std::string("\xc3\x28", 2));
    CHECK_EQ(hex.toString(), std::string("c328"));
    CHECK(hex.getUtf8Text().empty());

    /* hex form decodes back to the same bytes */
    const WifiSsid round = WifiSsid::fromString("c328");
    CHECK_EQ(round.getBytes(), std::string("\xc3\x28", 2));

    bool threw = false;
    try { WifiSsid::fromString("abc"); } catch (const std::invalid_argument&) { threw = true; }
    CHECK(threw); /* odd-length hex throws */
}

static void testParseScanResults() {
    const std::string reply =
            "bssid / frequency / signal level / flags / ssid\n"
            "00:11:22:33:44:55\t2412\t-45\t[WPA2-PSK-CCMP][ESS]\t\"myssid\"\n"
            "aa:bb:cc:dd:ee:ff\t5180\t-67\t[WPA-PSK-TKIP][ESS]\t\x68\x65\x78\x6e\x65\x74\n";
    const std::vector<ScanResult> results = WpaResponseParser::parseScanResults(reply);
    CHECK_EQ(results.size(), (size_t)2);
    CHECK_EQ(results[0].BSSID, std::string("00:11:22:33:44:55"));
    CHECK_EQ(results[0].frequency, 2412);
    CHECK_EQ(results[0].level, -45);
    CHECK_EQ(results[0].capabilities, std::string("[WPA2-PSK-CCMP][ESS]"));
    CHECK_EQ(results[0].SSID, std::string("\"myssid\""));
    CHECK_EQ(results[0].wifiSsid.getUtf8Text(), std::string("myssid"));
    /* unquoted token is hex */
    CHECK_EQ(results[1].wifiSsid.getBytes(), std::string("hexnet"));
}

static void testApplyStatus() {
    WifiInfo info;
    WpaResponseParser::applyStatus(info,
            "bssid=00:11:22:33:44:55\n"
            "freq=2412\n"
            "ssid=home\n"   /* STATUS prints unquoted plain text */
            "id=3\n"
            "mode=station\n"
            "wpa_state=COMPLETED\n"
            "ip_address=192.168.1.5\n");
    CHECK_EQ(info.getBSSID(), std::string("00:11:22:33:44:55"));
    CHECK_EQ(info.getSSID(), std::string("\"home\""));
    CHECK_EQ(info.getNetworkId(), 3);
    CHECK(info.getSupplicantState() == SupplicantState::COMPLETED);
    CHECK_EQ(info.getFrequency(), 2412);
    /* dotted form -> deprecated int getter (little-endian, a in LSB) */
    CHECK_EQ(info.getIpAddress(), (int)0x0501A8C0);
    CHECK(info.is24GHz());

    WpaResponseParser::applySignalPoll(info, "RSSI=-52\nLINKSPEED=72\nFREQUENCY=2437\n");
    CHECK_EQ(info.getRssi(), -52);
    CHECK_EQ(info.getLinkSpeed(), 72);
    CHECK_EQ(info.getFrequency(), 2437);
}

static void testParseListNetworks() {
    const auto entries = WpaResponseParser::parseListNetworks(
            "network id / ssid / bssid / flags\n"
            "0\thome\tany\t[CURRENT]\n"
            "1\t\"old\"\tany\t[DISABLED]\n");
    CHECK_EQ(entries.size(), (size_t)2);
    CHECK_EQ(entries[0].networkId, 0);
    CHECK_EQ(entries[0].ssid, std::string("home"));
    CHECK_EQ(entries[0].flags, std::string("[CURRENT]"));
    CHECK_EQ(entries[1].ssid, std::string("\"old\""));
    CHECK_EQ(entries[1].flags, std::string("[DISABLED]"));
}

static void testNetworkVariables() {
    WifiConfiguration config;
    WpaResponseParser::applyNetworkVariable(config, "ssid", "\"cafe\"");
    WpaResponseParser::applyNetworkVariable(config, "psk", "\"topsecret\"");
    WpaResponseParser::applyNetworkVariable(config, "key_mgmt", "WPA-PSK");
    WpaResponseParser::applyNetworkVariable(config, "proto", "RSN");
    WpaResponseParser::applyNetworkVariable(config, "priority", "7");
    WpaResponseParser::applyNetworkVariable(config, "scan_ssid", "1");
    CHECK_EQ(config.SSID, std::string("\"cafe\""));
    CHECK_EQ(config.preSharedKey, std::string("topsecret"));
    CHECK(config.allowedKeyManagement.test(WifiConfiguration::KeyMgmt::WPA_PSK));
    CHECK(config.allowedProtocols.test(WifiConfiguration::Protocol::RSN));
    CHECK_EQ(config.priority, 7);
    CHECK(config.hiddenSSID);

    /* SET_NETWORK generation: PSK passphrase quoted, key_mgmt emits the
     * single WPA-PSK token for both PSK bits, scan_ssid for hidden. */
    config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::WPA2_PSK);
    const auto vars = WpaResponseParser::networkVariables(config);
    bool sawSsid = false, sawPsk = false, sawKeyMgmt = false, sawPriority = false;
    for (const auto& var : vars) {
        if (var.first == "ssid") { sawSsid = true; CHECK_EQ(var.second, std::string("\"cafe\"")); }
        if (var.first == "psk") { sawPsk = true; CHECK_EQ(var.second, std::string("\"topsecret\"")); }
        if (var.first == "key_mgmt") { sawKeyMgmt = true; CHECK_EQ(var.second, std::string("WPA-PSK")); }
        if (var.first == "priority") { sawPriority = true; CHECK_EQ(var.second, std::string("7")); }
    }
    CHECK(sawSsid && sawPsk && sawKeyMgmt && sawPriority);

    /* 64-hex PSK passes through raw (already a computed key) */
    WifiConfiguration hexKey;
    hexKey.SSID = "\"x\"";
    hexKey.preSharedKey = std::string(64, 'a');
    for (const auto& var : WpaResponseParser::networkVariables(hexKey))
        if (var.first == "psk") CHECK_EQ(var.second, std::string(64, 'a'));

    /* token tables round-trip coverage lives in testCiphersRoundTrip() */
}

static void testCiphersRoundTrip() {
    WifiConfiguration config;
    WpaResponseParser::applyPairwiseCiphers(config, "CCMP TKIP");
    CHECK_EQ(WpaResponseParser::pairwiseCiphersString(config), std::string("TKIP CCMP"));
    WpaResponseParser::applyGroupCiphers(config, "WEP-40 CCMP");
    CHECK_EQ(WpaResponseParser::groupCiphersString(config), std::string("WEP-40 CCMP"));
    WpaResponseParser::applyProtocols(config, "WPA RSN");
    CHECK_EQ(WpaResponseParser::protocolsString(config), std::string("WPA RSN"));
    WpaResponseParser::applyAuthAlgorithms(config, "OPEN SHARED");
    CHECK_EQ(WpaResponseParser::authAlgorithmsString(config), std::string("OPEN SHARED"));
}

static void testSignalLevels() {
    CHECK_EQ(WifiManager::calculateSignalLevel(-127, 5), 0); /* below range */
    CHECK_EQ(WifiManager::calculateSignalLevel(-100, 5), 0);
    CHECK_EQ(WifiManager::calculateSignalLevel(-78, 5), 1);
    CHECK_EQ(WifiManager::calculateSignalLevel(-55, 5), 4);
    CHECK_EQ(WifiManager::calculateSignalLevel(10, 5), 4);  /* above range */
    CHECK_EQ(WifiManager::compareSignalLevel(-60, -70) > 0, true);
    CHECK_EQ(WifiManager::getInstance().getMaxSignalLevel(), 4);
}

static void testGetAuthType() {
    WifiConfiguration psk;
    psk.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::WPA2_PSK);
    CHECK_EQ(psk.getAuthType(), (int)WifiConfiguration::KeyMgmt::WPA2_PSK);

    WifiConfiguration eap;
    eap.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::WPA_EAP);
    eap.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::IEEE8021X);
    CHECK_EQ(eap.getAuthType(), (int)WifiConfiguration::KeyMgmt::WPA_EAP);

    bool threw = false;
    try { psk.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::SAE); psk.getAuthType(); }
    catch (const std::logic_error&) { threw = true; }
    CHECK(threw);
}

static void testSupplicantState() {
    CHECK(SupplicantState::isValidState(SupplicantState::COMPLETED));
    CHECK(!SupplicantState::isValidState(SupplicantState::INVALID));
    CHECK(SupplicantState::isHandshakeState(SupplicantState::FOUR_WAY_HANDSHAKE));
    CHECK(!SupplicantState::isHandshakeState(SupplicantState::SCANNING));
    CHECK(SupplicantState::isConnecting(SupplicantState::ASSOCIATED));
    CHECK_EQ(SupplicantState::toString(SupplicantState::GROUP_HANDSHAKE),
             std::string("GROUP_HANDSHAKE"));
    bool threw = false;
    try { SupplicantState::fromString("NO_SUCH"); } catch (const std::invalid_argument&) { threw = true; }
    CHECK(threw);
}

int main() {
    testParseEventMessage();
    testWifiSsid();
    testParseScanResults();
    testApplyStatus();
    testParseListNetworks();
    testNetworkVariables();
    testCiphersRoundTrip();
    testSignalLevels();
    testGetAuthType();
    testSupplicantState();
    printf("%d checks, %d failures\n", gChecks, gFailures);
    return gFailures;
}
