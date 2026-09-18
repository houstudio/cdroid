/*
 * wifitests — self-contained pure-logic tests for the cdwifi stack: reply
 * parsers, WifiSsid codec, signal-level math, auth-type mapping, SoftAp
 * configuration + hostapd.conf rendering. No wpa_supplicant/hostapd daemon
 * needed. Exit code = number of failed checks.
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>

#include <macaddress.h>
#include <natcontroller.h>
#include <wifi/hostapdclient.h>
#include <wifi/softapcapability.h>
#include <wifi/softapconfigstore.h>
#include <wifi/softapconfiguration.h>
#include <wifi/supplicantclient.h>
#include <wifi/supplicantstate.h>
#include <wifi/wifimanager.h>
#include <wifi/wparesponseparser.h>

using cdroid::HostapdClient;
using cdroid::MacAddress;
using cdroid::ScanResult;
using cdroid::SoftApConfiguration;
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

    /* the 32 cap counts decoded chars, not bytes (CharBuffer.allocate(32)):
     * 30 CJK chars = 90 utf-8 bytes survive whole; 33 ASCII stops at 32. */
    std::string cjkBytes;
    for (int i = 0; i < 30; i++) cjkBytes.append("\xe4\xb8\xad");
    CHECK_EQ(WifiSsid::fromUtf8Text(cjkBytes).getUtf8Text().size(), (size_t)90);
    const std::string ascii33(33, 'a');
    CHECK_EQ(WifiSsid::fromUtf8Text(ascii33).getUtf8Text().size(), (size_t)32);
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
    /* ScanResult.SSID is the plain decoded text (ScanResult.java:84) */
    CHECK_EQ(results[0].SSID, std::string("myssid"));
    CHECK_EQ(results[0].wifiSsid.getUtf8Text(), std::string("myssid"));
    /* unquoted token is hex; its SSID is the decoded text too */
    CHECK_EQ(results[1].wifiSsid.getBytes(), std::string("hexnet"));
    CHECK_EQ(results[1].SSID, std::string("hexnet"));
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

static void testGetIpAddress() {
    WifiInfo info;
    CHECK_EQ(info.getIpAddress(), 0);   /* empty -> 0 */
    info.setInetAddress("192.168.1.100");
    /* HTL: first octet in the LSB */
    CHECK_EQ(info.getIpAddress(), 192 | (168 << 8) | (1 << 16) | (100 << 24));
    /* strict dotted quad: short, garbage-tailed, bad separator -> 0 */
    info.setInetAddress("192.168.1");
    CHECK_EQ(info.getIpAddress(), 0);
    info.setInetAddress("10.0.2.15 trailing");
    CHECK_EQ(info.getIpAddress(), 0);
    info.setInetAddress("10.0 2.15");
    CHECK_EQ(info.getIpAddress(), 0);
    info.setInetAddress("10.0.2.300");
    CHECK_EQ(info.getIpAddress(), 0);
    info.setInetAddress("::1");
    CHECK_EQ(info.getIpAddress(), 0);
}

static void testSetRssiClamp() {
    /* AOSP clamps into [INVALID_RSSI=-127, MAX_RSSI=200]. */
    WifiInfo info;
    info.setRssi(-200);
    CHECK_EQ(info.getRssi(), (int) WifiInfo::INVALID_RSSI);
    info.setRssi(300);
    CHECK_EQ(info.getRssi(), (int) WifiInfo::MAX_RSSI);
    info.setRssi(-52);
    CHECK_EQ(info.getRssi(), -52);
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

    /* hex WEP key (10/26/58 chars) goes raw; ASCII passphrase stays quoted */
    WifiConfiguration wep;
    wep.SSID = "\"w\"";
    wep.wepKeys[0] = "0123456789";   /* 40-bit hex */
    wep.wepKeys[1] = "abcde";        /* 5-char ASCII passphrase */
    bool sawHexWep = false, sawAsciiWep = false;
    for (const auto& var : WpaResponseParser::networkVariables(wep)) {
        if (var.first == "wep_key0") { sawHexWep = true; CHECK_EQ(var.second, std::string("0123456789")); }
        if (var.first == "wep_key1") { sawAsciiWep = true; CHECK_EQ(var.second, std::string("\"abcde\"")); }
    }
    CHECK(sawHexWep && sawAsciiWep);

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
    /* Wire forms: STATE-CHANGE prints the numeric wpa_states value
     * (ctrl_iface.c "state=%d", same order as the first ten enum values);
     * STATUS wpa_state= spells "4WAY_HANDSHAKE" (wpa_supplicant_state_txt). */
    CHECK_EQ(SupplicantState::fromString("9"), SupplicantState::COMPLETED);
    CHECK_EQ(SupplicantState::fromString("7"), SupplicantState::FOUR_WAY_HANDSHAKE);
    CHECK_EQ(SupplicantState::fromString("0"), SupplicantState::DISCONNECTED);
    CHECK_EQ(SupplicantState::fromString("2"), SupplicantState::INACTIVE);
    CHECK_EQ(SupplicantState::fromString("4WAY_HANDSHAKE"),
             SupplicantState::FOUR_WAY_HANDSHAKE);
    threw = false;
    try { SupplicantState::fromString("10"); } catch (const std::invalid_argument&) { threw = true; }
    CHECK(threw);
    threw = false;
    try { SupplicantState::fromString("-1"); } catch (const std::invalid_argument&) { threw = true; }
    CHECK(threw);
}

static void testMacAddress() {
    CHECK_EQ(MacAddress::fromString("02:00:11:22:33:44").toString(),
             std::string("02:00:11:22:33:44"));
    /* dash and bare-hex forms canonicalize to colon-separated lowercase */
    CHECK_EQ(MacAddress::fromString("02-00-11-22-33-44").toString(),
             std::string("02:00:11:22:33:44"));
    CHECK_EQ(MacAddress::fromString("020011223344").toString(),
             std::string("02:00:11:22:33:44"));
    /* malformed inputs are the Java-null sentinel (empty) */
    CHECK_EQ(MacAddress::fromString("02:00:11:22:33").toString(), std::string());
    CHECK_EQ(MacAddress::fromString("zz:00:11:22:33:44").toString(), std::string());
    CHECK_EQ(MacAddress::fromString("02-00:11:22:33:44").toString(), std::string());
    CHECK_EQ(MacAddress::fromBytes(std::string("short")).toString(), std::string());
    CHECK_EQ(MacAddress::fromBytes(std::string("\x02\x00\x11\x22\x33\x44", 6)).toString(),
             std::string("02:00:11:22:33:44"));
    /* address classification + local-administration bit */
    CHECK_EQ(MacAddress::fromString("02:00:11:22:33:44").getAddressType(),
             (int)MacAddress::TYPE_UNICAST);
    CHECK(MacAddress::fromString("02:00:11:22:33:44").isLocallyAssigned());
    CHECK(!MacAddress::fromString("04:00:11:22:33:44").isLocallyAssigned());
    CHECK_EQ(MacAddress::fromString("01:00:5e:00:00:01").getAddressType(),
             (int)MacAddress::TYPE_MULTICAST);
    CHECK_EQ(MacAddress::fromString("ff:ff:ff:ff:ff:ff").getAddressType(),
             (int)MacAddress::TYPE_BROADCAST);
    CHECK_EQ(MacAddress::ALL_ZEROS_MAC_ADDRESS.toString(),
             std::string("00:00:00:00:00:00"));
    CHECK_EQ(MacAddress::fromString("aabbccddeeff").toOuiString(),
             std::string("aabbcc"));
    CHECK(MacAddress::fromString("aabbccddeeff")
            == MacAddress::fromString("AA:BB:CC:DD:EE:FF"));
}

static void testSoftApConfigurationDefaults() {
    const SoftApConfiguration config = SoftApConfiguration::Builder().build();
    /* Builder defaults (android-36 resolved): open, 2GHz auto channel. */
    CHECK(config.getWifiSsid().getBytes().empty());   /* Java null SSID */
    CHECK_EQ(config.getSsid(), std::string());
    CHECK(config.getBssid().getBytes().empty());
    CHECK_EQ(config.getBand(), (int)SoftApConfiguration::BAND_2GHZ);
    CHECK_EQ(config.getChannel(), 0);
    /* copy the vector first: CHECK_EQ's reference bind must not dangle into
     * the temporary returned by getBands() */
    const std::vector<int> defaultBands = config.getBands();
    CHECK_EQ(defaultBands.size(), (size_t)1);
    CHECK_EQ(defaultBands[0], (int)SoftApConfiguration::BAND_2GHZ);
    CHECK_EQ(config.getSecurityType(), (int)SoftApConfiguration::SECURITY_TYPE_OPEN);
    CHECK_EQ(config.getMaxNumberOfClients(), 0);
    CHECK(config.isAutoShutdownEnabled());
    CHECK_EQ((int)config.getShutdownTimeoutMillis(),
             (int)SoftApConfiguration::DEFAULT_TIMEOUT);
    CHECK(!config.isClientControlByUserEnabled());
    CHECK(config.getBlockedClientList().empty());
    CHECK(config.getAllowedClientList().empty());
    CHECK_EQ(config.getMacRandomizationSetting(),
             (int)SoftApConfiguration::RANDOMIZATION_NON_PERSISTENT);
    CHECK(config.isBridgedModeOpportunisticShutdownEnabled());
    CHECK(config.isIeee80211axEnabled());
    CHECK(config.isIeee80211beEnabled());
    CHECK(config.isUserConfiguration());
    CHECK(config.getVendorElements().empty());
    CHECK_EQ(config.getMaxChannelBandwidth(), 0);   /* CHANNEL_WIDTH_AUTO */
    CHECK(!config.isClientIsolationEnabled());
    CHECK(!config.toString().empty());

    /* A DEFAULT-CONSTRUCTED (unset, flag-paired) instance carries an empty
     * mChannels — the accessors must answer the documented defaults instead
     * of dereferencing end() (the old getBand()/getChannel() were UB on it). */
    const SoftApConfiguration unset;
    CHECK_EQ(unset.getBand(), (int)SoftApConfiguration::BAND_2GHZ);
    CHECK_EQ(unset.getChannel(), 0);
}

static bool builderThrew(const std::function<void()>& fn) {
    try { fn(); return false; } catch (const std::invalid_argument&) { return true; }
}

static void testSoftApConfigurationBuilder() {
    const SoftApConfiguration config = SoftApConfiguration::Builder()
            .setSsid("MyAP")
            .setPassphrase("pass12345", SoftApConfiguration::SECURITY_TYPE_WPA2_PSK)
            .setChannel(6, SoftApConfiguration::BAND_2GHZ)
            .setHiddenSsid(true)
            .setMaxNumberOfClients(4)
            .setClientIsolationEnabled(true)
            .build();
    CHECK_EQ(config.getSsid(), std::string("MyAP"));
    CHECK_EQ(config.getPassphrase(), std::string("pass12345"));
    CHECK_EQ(config.getSecurityType(), (int)SoftApConfiguration::SECURITY_TYPE_WPA2_PSK);
    CHECK_EQ(config.getChannel(), 6);
    CHECK(config.isHiddenSsid());
    CHECK_EQ(config.getMaxNumberOfClients(), 4);
    CHECK(config.isClientIsolationEnabled());

    /* copy construction via Builder(other) round-trips every field */
    const SoftApConfiguration copy = SoftApConfiguration::Builder(config).build();
    CHECK(copy == config);
    CHECK(!(copy != config));

    /* A BSSID requires explicit RANDOMIZATION_NONE (the FORCE_MUTUAL_EXCLUSIVE
     * compat change, enabled since S); Builder(other) then auto-preserves it. */
    const SoftApConfiguration withBssid = SoftApConfiguration::Builder()
            .setSsid("BssidAp")
            .setBssid(MacAddress::fromString("02:11:22:33:44:55"))
            .setPassphrase("pass12345", SoftApConfiguration::SECURITY_TYPE_WPA2_PSK)
            .setMacRandomizationSetting(SoftApConfiguration::RANDOMIZATION_NONE)
            .build();
    CHECK_EQ(SoftApConfiguration::Builder(withBssid).build()
                    .getMacRandomizationSetting(),
             (int)SoftApConfiguration::RANDOMIZATION_NONE);
    CHECK(builderThrew([] {
        SoftApConfiguration::Builder()
                .setSsid("BssidAp")
                .setBssid(MacAddress::fromString("02:11:22:33:44:55"))
                .setPassphrase("pass12345", SoftApConfiguration::SECURITY_TYPE_WPA2_PSK)
                .build();   /* randomization still NON_PERSISTENT */
    }));
    /* the working pairing: explicit BSSID + explicit RANDOMIZATION_NONE
     * (what withRandomizedBssid produces; Builder(other)'s auto-fix only
     * fires when the SOURCE config already carries a bssid) */
    bool bssidWithNoneOk = false;
    try {
        SoftApConfiguration::Builder()
                .setSsid("BssidAp")
                .setBssid(MacAddress::fromString("02:11:22:33:44:55"))
                .setMacRandomizationSetting(SoftApConfiguration::RANDOMIZATION_NONE)
                .setPassphrase("pass12345", SoftApConfiguration::SECURITY_TYPE_WPA2_PSK)
                .build();
        bssidWithNoneOk = true;
    } catch (const std::invalid_argument&) {
    }
    CHECK(bssidWithNoneOk);

    /* validation: IllegalArgumentException paths */
    CHECK(builderThrew([] {
        SoftApConfiguration::Builder().setBand(0);
    }));
    CHECK(builderThrew([] {
        SoftApConfiguration::Builder().setBand(SoftApConfiguration::BAND_2GHZ
                | (1 << 4));   /* not a band */
    }));
    CHECK(builderThrew([] {
        SoftApConfiguration::Builder().setChannel(15, SoftApConfiguration::BAND_2GHZ);
    }));
    CHECK(builderThrew([] {
        SoftApConfiguration::Builder().setChannel(13, SoftApConfiguration::BAND_5GHZ);
    }));
    CHECK(builderThrew([] {
        SoftApConfiguration::Builder().setPassphrase(
                "", SoftApConfiguration::SECURITY_TYPE_WPA2_PSK);
    }));
    CHECK(builderThrew([] {
        SoftApConfiguration::Builder().setPassphrase(
                "secret", SoftApConfiguration::SECURITY_TYPE_OPEN);
    }));
    CHECK(builderThrew([] {
        SoftApConfiguration::Builder().setMaxNumberOfClients(-1);
    }));
    CHECK(builderThrew([] {
        SoftApConfiguration::Builder().setShutdownTimeoutMillis(-2);
    }));
    CHECK(builderThrew([] {
        SoftApConfiguration::Builder().setBssid(
                MacAddress::fromString("ff:ff:ff:ff:ff:ff"));
    }));
    CHECK(builderThrew([] {
        MacAddress blocked = MacAddress::fromString("02:00:00:00:00:01");
        SoftApConfiguration::Builder()
                .setAllowedClientList({blocked})
                .setBlockedClientList({blocked})
                .build();
    }));
    CHECK(builderThrew([] {
        ScanResult::InformationElement ie;   /* id -1, not EID_VSA */
        SoftApConfiguration::Builder().setVendorElements({ie});
    }));
    CHECK(builderThrew([] {
        ScanResult::InformationElement ie;
        ie.id = ScanResult::InformationElement::EID_VSA;
        SoftApConfiguration::Builder().setVendorElements({ie, ie});
    }));
    CHECK(builderThrew([] {
        SoftApConfiguration::Builder().setAllowedAcsChannels(
                SoftApConfiguration::BAND_60GHZ, {1});
    }));
    /* 11be depends on 11ax (BAKLAVA resolution in build()) */
    CHECK(!SoftApConfiguration::Builder()
            .setIeee80211axEnabled(false)
            .setIeee80211beEnabled(true)
            .build().isIeee80211beEnabled());

    /* toWifiConfiguration bridging */
    WifiConfiguration* bridged = config.toWifiConfiguration();
    CHECK(bridged != nullptr);
    CHECK_EQ(bridged->SSID, std::string("MyAP"));   /* unquoted utf-8 */
    CHECK_EQ(bridged->preSharedKey, std::string("pass12345"));
    CHECK_EQ(bridged->apBand, (int)WifiConfiguration::AP_BAND_2GHZ);
    CHECK_EQ(bridged->apChannel, 6);
    CHECK(bridged->allowedKeyManagement.test(WifiConfiguration::KeyMgmt::WPA2_PSK));
    delete bridged;
    /* WPA3_SAE_TRANSITION bridges onto the legacy WPA2_PSK surface too */
    const SoftApConfiguration saeTransition = SoftApConfiguration::Builder()
            .setSsid("SaeAp")
            .setPassphrase("pass12345",
                    SoftApConfiguration::SECURITY_TYPE_WPA3_SAE_TRANSITION)
            .build();
    bridged = saeTransition.toWifiConfiguration();
    CHECK(bridged != nullptr);
    CHECK(bridged->allowedKeyManagement.test(WifiConfiguration::KeyMgmt::WPA2_PSK));
    delete bridged;
    /* OWE has no legacy representation (Java null) */
    bridged = SoftApConfiguration::Builder()
            .setPassphrase("", SoftApConfiguration::SECURITY_TYPE_WPA3_OWE)
            .build().toWifiConfiguration();
    CHECK(bridged == nullptr);
}

static std::string readTextFile(const std::string& path) {
    std::ifstream in(path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static bool fileContains(const std::string& text, const std::string& needle) {
    return text.find(needle) != std::string::npos;
}

static void testHostapdConfigWriter() {
    const std::string path = "/tmp/cdroid-wifitests-hostapd.conf";
    unlink(path.c_str());
    std::string error;

    /* WPA2 on channel 6 with all the renderable extras */
    const SoftApConfiguration wpa2 = SoftApConfiguration::Builder()
            .setSsid("MyAP")
            .setPassphrase("pass12345", SoftApConfiguration::SECURITY_TYPE_WPA2_PSK)
            .setChannel(6, SoftApConfiguration::BAND_2GHZ)
            .setHiddenSsid(true)
            .setMaxNumberOfClients(4)
            .setClientIsolationEnabled(true)
            .build();
    CHECK(HostapdClient::writeConfigFile(path, "wlan1", "/tmp/ctrl", wpa2, &error));
    const std::string conf = readTextFile(path);
    CHECK(fileContains(conf, "interface=wlan1\n"));
    CHECK(fileContains(conf, "driver=nl80211\n"));
    CHECK(fileContains(conf, "ctrl_interface=/tmp/ctrl\n"));
    CHECK(fileContains(conf, "ssid=MyAP\n"));
    CHECK(fileContains(conf, "hw_mode=g\n"));
    CHECK(fileContains(conf, "channel=6\n"));
    CHECK(fileContains(conf, "ignore_broadcast_ssid=1\n"));
    CHECK(fileContains(conf, "wpa=2\n"));
    CHECK(fileContains(conf, "wpa_key_mgmt=WPA-PSK\n"));
    CHECK(fileContains(conf, "rsn_pairwise=CCMP\n"));
    CHECK(fileContains(conf, "wpa_passphrase=pass12345\n"));
    CHECK(fileContains(conf, "max_num_sta=4\n"));
    CHECK(fileContains(conf, "ap_isolate=1\n"));
    /* band -> hw_mode mapping */
    const SoftApConfiguration band5 = SoftApConfiguration::Builder()
            .setSsid("Ap5G")
            .setPassphrase("pass12345", SoftApConfiguration::SECURITY_TYPE_WPA2_PSK)
            .setChannel(36, SoftApConfiguration::BAND_5GHZ)
            .build();
    CHECK(HostapdClient::writeConfigFile(path, "wlan1", "/tmp/ctrl", band5, &error));
    CHECK(fileContains(readTextFile(path), "hw_mode=a\n"));
    CHECK(fileContains(readTextFile(path), "channel=36\n"));
    /* open network: no wpa lines at all */
    const SoftApConfiguration openNet = SoftApConfiguration::Builder()
            .setSsid("OpenAp")
            .setBand(SoftApConfiguration::BAND_2GHZ)
            .build();
    CHECK(HostapdClient::writeConfigFile(path, "wlan1", "/tmp/ctrl", openNet, &error));
    const std::string openConf = readTextFile(path);
    CHECK(!fileContains(openConf, "wpa="));
    CHECK(!fileContains(openConf, "wpa_passphrase="));
    /* SAE: sae_password + mandatory MFP */
    const SoftApConfiguration sae = SoftApConfiguration::Builder()
            .setSsid("SaeAp")
            .setPassphrase("pass12345", SoftApConfiguration::SECURITY_TYPE_WPA3_SAE)
            .setBand(SoftApConfiguration::BAND_2GHZ)
            .build();
    CHECK(HostapdClient::writeConfigFile(path, "wlan1", "/tmp/ctrl", sae, &error));
    const std::string saeConf = readTextFile(path);
    CHECK(fileContains(saeConf, "wpa_key_mgmt=SAE\n"));
    CHECK(fileContains(saeConf, "sae_password=pass12345\n"));
    CHECK(fileContains(saeConf, "ieee80211w=2\n"));
    /* blocked client list renders a deny file + macaddr_acl=0 */
    const SoftApConfiguration blocked = SoftApConfiguration::Builder()
            .setSsid("DenyAp")
            .setBand(SoftApConfiguration::BAND_2GHZ)
            .setBlockedClientList({MacAddress::fromString("02:00:00:00:00:09")})
            .build();
    CHECK(HostapdClient::writeConfigFile(path, "wlan1", "/tmp/ctrl", blocked, &error));
    CHECK(fileContains(readTextFile(path), "macaddr_acl=0\n"));
    CHECK(fileContains(readTextFile(path), "deny_mac_file=" + path + ".deny\n"));
    CHECK(fileContains(readTextFile(path + ".deny"), "02:00:00:00:00:09\n"));
    /* rejections: OWE security, bridged bands, SSID-less config */
    CHECK(!HostapdClient::writeConfigFile(path, "wlan1", "/tmp/ctrl",
            SoftApConfiguration::Builder()
                    .setPassphrase("", SoftApConfiguration::SECURITY_TYPE_WPA3_OWE)
                    .build(), &error));
    CHECK(!error.empty());
    CHECK(!HostapdClient::writeConfigFile(path, "wlan1", "/tmp/ctrl",
            SoftApConfiguration::Builder()
                    .setBands({SoftApConfiguration::BAND_2GHZ,
                               SoftApConfiguration::BAND_5GHZ})
                    .build(), &error));
    CHECK(!HostapdClient::writeConfigFile(path, "wlan1", "/tmp/ctrl",
            SoftApConfiguration::Builder().build(), &error));
    unlink(path.c_str());
    unlink((path + ".deny").c_str());
    unlink((path + ".accept").c_str());
}

static void testHostapdEventParse() {
    /* hostapd events: no priority prefix, positional mac argument */
    const SupplicantEvent connected = SupplicantClient::parseEventMessage(
            "AP-STA-CONNECTED 02:11:22:33:44:55");
    CHECK_EQ(connected.name, std::string("AP-STA-CONNECTED"));
    CHECK_EQ(connected.args.size(), (size_t)0);
    CHECK_EQ(connected.raw,
             std::string("AP-STA-CONNECTED 02:11:22:33:44:55"));
    const SupplicantEvent disconnected = SupplicantClient::parseEventMessage(
            "AP-STA-DISCONNECTED 02:11:22:33:44:55");
    CHECK_EQ(disconnected.name, std::string("AP-STA-DISCONNECTED"));
}

static void testSoftApSupportClasses() {
    /* WifiClient: identity + rendering */
    const cdroid::WifiClient client(
            MacAddress::fromString("02:00:00:00:00:09"), "wlan1");
    CHECK_EQ(client.getInterfaceName(), std::string("wlan1"));
    CHECK_EQ(client.getMacAddress().toString(), std::string("02:00:00:00:00:09"));
    CHECK(client == cdroid::WifiClient(
            MacAddress::fromString("02:00:00:00:00:09"), "wlan1"));
    CHECK(client != cdroid::WifiClient(
            MacAddress::fromString("02:00:00:00:00:0a"), "wlan1"));
    CHECK(!client.toString().empty());

    /* SoftApCapability: feature-bit semantics */
    cdroid::SoftApCapability caps((int64_t)cdroid::SoftApCapability::SOFTAP_FEATURE_CLIENT_FORCE_DISCONNECT);
    CHECK(caps.areFeaturesSupported(
            (int64_t)cdroid::SoftApCapability::SOFTAP_FEATURE_CLIENT_FORCE_DISCONNECT));
    CHECK(!caps.areFeaturesSupported(
            (int64_t)cdroid::SoftApCapability::SOFTAP_FEATURE_ACS_OFFLOAD));
    caps.setMaxSupportedClients(8);
    CHECK_EQ(caps.getMaxSupportedClients(), 8);
    cdroid::SoftApCapability none;
    CHECK(!none.areFeaturesSupported(
            (int64_t)cdroid::SoftApCapability::SOFTAP_FEATURE_IEEE80211_AX));
}

static void testSoftApConfigStore() {
    using cdroid::SoftApConfigStore;
    /* full roundtrip of the persistable field set, on a test-owned path
     * (the runtime store dir may be root-owned from an E2E run) */
    const std::string path = "/tmp/cdroid-wifitests-store.conf";
    unlink(path.c_str());
    SoftApConfigStore::Record record;
    record.hasConfig = true;
    record.iface = "wlan1";
    record.config = SoftApConfiguration::Builder()
            .setSsid("StoredAp")
            .setPassphrase("pass12345", SoftApConfiguration::SECURITY_TYPE_WPA2_PSK)
            .setChannel(11, SoftApConfiguration::BAND_2GHZ)
            .setHiddenSsid(true)
            .setMaxNumberOfClients(3)
            .setClientIsolationEnabled(true)
            .build();
    CHECK(SoftApConfigStore::save(record, path));
    SoftApConfigStore::Record loaded;
    CHECK(SoftApConfigStore::load(&loaded, path));
    CHECK_EQ(loaded.iface, std::string("wlan1"));
    CHECK(loaded.hasConfig);
    CHECK(loaded.config == record.config);

    /* open-network record: passphrase empty, security OPEN */
    SoftApConfigStore::Record openRecord;
    openRecord.hasConfig = true;
    openRecord.iface = "wlan1";
    openRecord.config = SoftApConfiguration::Builder()
            .setSsid("OpenStored")
            .setBand(SoftApConfiguration::BAND_2GHZ)
            .build();
    CHECK(SoftApConfigStore::save(openRecord, path));
    CHECK(SoftApConfigStore::load(&loaded, path));
    CHECK(loaded.config == openRecord.config);
    CHECK_EQ(loaded.config.getSecurityType(),
             (int)SoftApConfiguration::SECURITY_TYPE_OPEN);

    /* client-control lists round-trip: a blocked client must stay blocked
     * across a process restart (the store used to drop the lists — the
     * blocked client silently rejoined). */
    SoftApConfigStore::Record aclRecord;
    aclRecord.hasConfig = true;
    aclRecord.iface = "wlan1";
    aclRecord.config = SoftApConfiguration::Builder()
            .setSsid("AclAp")
            .setPassphrase("pass12345", SoftApConfiguration::SECURITY_TYPE_WPA2_PSK)
            .setBand(SoftApConfiguration::BAND_2GHZ)
            .setClientControlByUserEnabled(true)
            .setBlockedClientList({MacAddress::fromString("aa:bb:cc:00:00:01"),
                                   MacAddress::fromString("aa:bb:cc:00:00:02")})
            .build();
    CHECK(SoftApConfigStore::save(aclRecord, path));
    CHECK(SoftApConfigStore::load(&loaded, path));
    CHECK(loaded.config == aclRecord.config);
    CHECK_EQ(loaded.config.getBlockedClientList().size(), (size_t)2);
    CHECK(loaded.config.isClientControlByUserEnabled());

    unlink(path.c_str());
    CHECK(!SoftApConfigStore::load(&loaded, path));   /* no store = first boot */

    /* A pre-planted/corrupt store (the runtime dir is under world-writable
     * /tmp) must come back as "no config", not as an escaping
     * std::invalid_argument that terminated the process. */
    {
        FILE* f = fopen(path.c_str(), "w");
        CHECK(f != nullptr);
        /* passphrase with securityType absent (atoi("")/garbage -> 0 = OPEN)
         * is the exact combination setPassphrase rejects */
        fputs("iface=ap0\nssid=\"Evil\"\npassphrase=evilpass\nband=1\nchannel=165\n", f);
        fclose(f);
        SoftApConfigStore::Record evil;
        CHECK(!SoftApConfigStore::load(&evil, path));   /* rejected, not fatal */
        unlink(path.c_str());
    }
}

static void testHostapdChannelAutoSelect() {
    /* channel 0 (framework auto) renders the band default on the
     * classic-daemon backend (no ACS offload) */
    const std::string path = "/tmp/cdroid-wifitests-hostapd.conf";
    std::string error;
    const SoftApConfiguration auto2g = SoftApConfiguration::Builder()
            .setSsid("AutoAp")
            .setPassphrase("pass12345", SoftApConfiguration::SECURITY_TYPE_WPA2_PSK)
            .setBand(SoftApConfiguration::BAND_2GHZ)
            .build();
    CHECK(HostapdClient::writeConfigFile(path, "wlan1", "/tmp/ctrl", auto2g, &error));
    const std::string conf2g = readTextFile(path);
    CHECK(fileContains(conf2g, "channel=6\n"));
    CHECK(fileContains(conf2g, "# channel auto-selected (no ACS offload on this backend)\n"));
    const SoftApConfiguration auto5g = SoftApConfiguration::Builder()
            .setSsid("AutoAp5")
            .setPassphrase("pass12345", SoftApConfiguration::SECURITY_TYPE_WPA2_PSK)
            .setBand(SoftApConfiguration::BAND_5GHZ)
            .build();
    CHECK(HostapdClient::writeConfigFile(path, "wlan1", "/tmp/ctrl", auto5g, &error));
    CHECK(fileContains(readTextFile(path), "channel=36\n"));
    unlink(path.c_str());

    /* Configuration injection: the renderer writes a hostapd.conf FILE, so a
     * line break in the SSID or passphrase would smuggle extra config lines
     * (e.g. "Guest\nwpa_passphrase=evil12345" re-securing an open network).
     * Both must be rejected outright. */
    std::string injectError;
    const SoftApConfiguration injectSsid = SoftApConfiguration::Builder()
            .setSsid("Guest\nwpa_passphrase=evil12345")
            .build();
    CHECK(!HostapdClient::writeConfigFile(path, "wlan1", "/tmp/ctrl",
                injectSsid, &injectError));
    CHECK(!injectError.empty());
    const SoftApConfiguration injectPass = SoftApConfiguration::Builder()
            .setSsid("Ok")
            .setPassphrase("p1234567\nmacaddr_acl=0",
                    SoftApConfiguration::SECURITY_TYPE_WPA2_PSK)
            .build();
    CHECK(!HostapdClient::writeConfigFile(path, "wlan1", "/tmp/ctrl",
                injectPass, &injectError));
    CHECK(!injectError.empty());
    /* and the written (legit) conf must not be world-readable: 0600 */
    struct stat st = {};
    CHECK(HostapdClient::writeConfigFile(path, "wlan1", "/tmp/ctrl", auto2g, &error));
    CHECK(stat(path.c_str(), &st) == 0);
    CHECK_EQ(st.st_mode & 0777, (mode_t)0600);
    unlink(path.c_str());
}

static void testNatControllerRules() {
    /* rule builders: exact netd-style wording */
    CHECK_EQ(cdroid::NatController::masqueradeRule("eno1"),
             std::string("-t nat -A POSTROUTING -o eno1 -j MASQUERADE"));
    const std::vector<std::string> fwd =
            cdroid::NatController::forwardingRules("wlan1", "eno1");
    CHECK_EQ(fwd.size(), (size_t)2);
    CHECK_EQ(fwd[0], std::string("-A FORWARD -i eno1 -o wlan1 -m state "
                                 "--state RELATED,ESTABLISHED -j ACCEPT"));
    CHECK_EQ(fwd[1], std::string("-A FORWARD -i wlan1 -o eno1 -j ACCEPT"));
    /* default-route parse: whatever it returns must be a real /proc entry */
    const std::string upstream = cdroid::NatController::defaultRouteInterface();
    if (!upstream.empty()) {
        std::ifstream route("/proc/net/route");
        std::string line, found;
        while (std::getline(route, line))
            if (line.compare(0, upstream.size(), upstream) == 0) { found = line; break; }
        CHECK(!found.empty());
    }
}

int main() {
    testParseEventMessage();
    testWifiSsid();
    testParseScanResults();
    testApplyStatus();
    testGetIpAddress();
    testSetRssiClamp();
    testParseListNetworks();
    testNetworkVariables();
    testCiphersRoundTrip();
    testSignalLevels();
    testGetAuthType();
    testSupplicantState();
    testMacAddress();
    testSoftApConfigurationDefaults();
    testSoftApConfigurationBuilder();
    testHostapdConfigWriter();
    testHostapdEventParse();
    testSoftApSupportClasses();
    testSoftApConfigStore();
    testHostapdChannelAutoSelect();
    testNatControllerRules();
    printf("%d checks, %d failures\n", gChecks, gFailures);
    return gFailures;
}
