/* Pure wpa_supplicant reply parsers (see wparesponseparser.h). */
#include <wifi/wparesponseparser.h>

#include <hexencoding.h>

#include <cstdlib>
#include <functional>
#include <stdexcept>

namespace cdroid {

/* --- small text helpers -------------------------------------------------- */

static std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::string line;
    for (const char c : text) {
        if (c == '\n') {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            lines.push_back(line);
            line.clear();
        } else {
            line.push_back(c);
        }
    }
    if (!line.empty()) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line);
    }
    return lines;
}

static std::vector<std::string> splitTabs(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    for (const char c : line) {
        if (c == '\t') {
            fields.push_back(field);
            field.clear();
        } else {
            field.push_back(c);
        }
    }
    fields.push_back(field);
    return fields;
}

/* The "is this a raw 64-hex PSK" gate must agree with
 * WifiSsid::fromString's decoder — one HexEncoding backs both. */
static bool isAllHex(const std::string& s) {
    return HexEncoding::isAllHex(s);
}

std::string WpaResponseParser::unquote(const std::string& value) {
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
        return value.substr(1, value.size() - 2);
    return value;
}

/* --- SCAN_RESULTS --------------------------------------------------------- */

std::vector<ScanResult> WpaResponseParser::parseScanResults(const std::string& scanResults) {
    std::vector<ScanResult> results;
    for (const std::string& line : splitLines(scanResults)) {
        if (line.empty() || line.compare(0, 5, "bssid") == 0) continue; /* header */
        const std::vector<std::string> fields = splitTabs(line);
        if (fields.size() < 5) continue;
        ScanResult result;
        result.BSSID = fields[0];
        result.frequency = atoi(fields[1].c_str());
        result.level = atoi(fields[2].c_str());
        result.capabilities = fields[3];
        /* SSID: wpa prints quoted UTF-8, unquoted hex, or empty for hidden.
         * A bare non-hex token (nonstandard reply) falls back to literal.
         * ScanResult.SSID is the plain decoded text (ScanResult.java:84) —
         * the quoted form belongs to WifiConfiguration.SSID, which callers
         * build with the '"' + SSID + '"' idiom. */
        result.SSID = fields[4];
        if (!fields[4].empty()) {
            try {
                result.wifiSsid = WifiSsid::fromString(fields[4]);
            } catch (const std::invalid_argument&) {
                result.wifiSsid = WifiSsid::fromUtf8Text(fields[4]);
            }
            result.SSID = ScanResult::displaySsid(result.wifiSsid);
        }
        results.push_back(result);
    }
    return results;
}

/* --- STATUS / SIGNAL_POLL -------------------------------------------------- */

static void applyKeyValueLines(const std::string& text,
        const std::function<void(const std::string&, const std::string&)>& apply) {
    for (const std::string& line : splitLines(text)) {
        const size_t eq = line.find('=');
        if (eq == std::string::npos || eq == 0) continue;
        apply(line.substr(0, eq), line.substr(eq + 1));
    }
}

void WpaResponseParser::applyStatus(WifiInfo& info, const std::string& status) {
    applyKeyValueLines(status, [&info](const std::string& key, const std::string& value) {
        if (key == "bssid") {
            info.setBSSID(value);
        } else if (key == "ssid") {
            /* STATUS prints the ssid UNQUOTED (ctrl_iface wpa_ssid_txt) —
             * unlike SCAN_RESULTS/GET_NETWORK, whose quoted/hex form goes
             * through WifiSsid::fromString. Literal text here. */
            info.setSSID(WifiSsid::fromUtf8Text(value));
        } else if (key == "id") {
            info.setNetworkId(atoi(value.c_str()));
        } else if (key == "ip_address") {
            info.setInetAddress(value);
        } else if (key == "freq") {
            info.setFrequency(atoi(value.c_str()));
        } else if (key == "wpa_state") {
            /* Unknown supplicant states must not take the caller down
             * (AOSP WifiMonitor drops such events). */
            try {
                info.setSupplicantState(SupplicantState::fromString(value));
            } catch (const std::invalid_argument&) {
            }
        }
    });
}

void WpaResponseParser::applySignalPoll(WifiInfo& info, const std::string& signalPoll) {
    applyKeyValueLines(signalPoll, [&info](const std::string& key, const std::string& value) {
        if (key == "RSSI") {
            info.setRssi(atoi(value.c_str()));
        } else if (key == "LINKSPEED") {
            info.setLinkSpeed(atoi(value.c_str()));
        } else if (key == "FREQUENCY") {
            info.setFrequency(atoi(value.c_str()));
        }
    });
}

/* --- LIST_NETWORKS --------------------------------------------------------- */

std::vector<WpaResponseParser::NetworkListEntry>
WpaResponseParser::parseListNetworks(const std::string& listNetworks) {
    std::vector<NetworkListEntry> entries;
    for (const std::string& line : splitLines(listNetworks)) {
        if (line.empty() || line.compare(0, 10, "network id") == 0) continue; /* header */
        const std::vector<std::string> fields = splitTabs(line);
        if (fields.size() < 2) continue;
        NetworkListEntry entry;
        entry.networkId = atoi(fields[0].c_str());
        entry.ssid = fields[1];
        entry.flags = fields.size() >= 4 ? fields[3] : std::string();
        entries.push_back(entry);
    }
    return entries;
}

/* --- GET_NETWORK variables -------------------------------------------------- */

void WpaResponseParser::applyNetworkVariable(WifiConfiguration& config, const std::string& variable,
                                             const std::string& value) {
    if (variable == "ssid") {
        config.SSID = value;
    } else if (variable == "bssid") {
        config.BSSID = unquote(value);
    } else if (variable == "psk") {
        config.preSharedKey = unquote(value);
    } else if (variable == "key_mgmt") {
        applyKeyMgmt(config, value);
    } else if (variable == "proto") {
        applyProtocols(config, value);
    } else if (variable == "pairwise") {
        applyPairwiseCiphers(config, value);
    } else if (variable == "group") {
        applyGroupCiphers(config, value);
    } else if (variable == "auth_alg") {
        applyAuthAlgorithms(config, value);
    } else if (variable == "priority") {
        config.priority = atoi(value.c_str());
    } else if (variable == "scan_ssid") {
        config.hiddenSSID = (atoi(value.c_str()) != 0);
    } else if (variable == "disabled") {
        const int disabled = atoi(value.c_str());
        if (disabled != 0) config.status = WifiConfiguration::Status::DISABLED;
        else if (config.status == WifiConfiguration::Status::DISABLED)
            config.status = WifiConfiguration::Status::ENABLED;
    }
    /* Unmapped network-block variables (eap, identity, ...) belong to
     * WifiEnterpriseConfig, which is not ported yet. */
}

/* --- token ↔ bitset tables ---------------------------------------------------
 * Tokens are the wpa_supplicant ctrl_iface spellings; the mapping mirrors
 * AOSP's WifiConfigurationUtil token tables (WPA2-PSK has no supplicant
 * token — both PSK bits serialize back to "WPA-PSK"). */

static std::vector<std::string> splitTokens(const std::string& tokens) {
    std::vector<std::string> out;
    std::string token;
    for (const char c : tokens) {
        if (c == ' ') {
            if (!token.empty()) out.push_back(token);
            token.clear();
        } else {
            token.push_back(c);
        }
    }
    if (!token.empty()) out.push_back(token);
    return out;
}

void WpaResponseParser::applyKeyMgmt(WifiConfiguration& config, const std::string& tokens) {
    for (const std::string& token : splitTokens(tokens)) {
        if (token == "WPA-PSK") config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::WPA_PSK);
        else if (token == "WPA2-PSK") config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::WPA2_PSK);
        else if (token == "WPA-EAP") config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::WPA_EAP);
        else if (token == "IEEE8021X") config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::IEEE8021X);
        else if (token == "NONE") config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::NONE);
        else if (token == "OSEN") config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::OSEN);
        else if (token == "FT-PSK") config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::FT_PSK);
        else if (token == "FT-EAP") config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::FT_EAP);
        else if (token == "SAE") config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::SAE);
        else if (token == "OWE") config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::OWE);
        else if (token == "WPA-EAP-SUITE-B-192") config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::SUITE_B_192);
        else if (token == "WPA-PSK-SHA256") config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::WPA_PSK_SHA256);
        else if (token == "WPA-EAP-SHA256") config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::WPA_EAP_SHA256);
        else if (token == "WAPI-PSK") config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::WAPI_PSK);
        else if (token == "WAPI-CERT") config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::WAPI_CERT);
        else if (token == "FILS-SHA256") config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::FILS_SHA256);
        else if (token == "FILS-SHA384") config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::FILS_SHA384);
        else if (token == "DPP") config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::DPP);
    }
}

std::string WpaResponseParser::keyMgmtString(const WifiConfiguration& config) {
    std::string out;
    const auto append = [&out](const char* token) {
        if (!out.empty()) out += " ";
        out += token;
    };
    const auto& bits = config.allowedKeyManagement;
    using K = WifiConfiguration::KeyMgmt;
    if (bits.test(K::WPA_PSK) || bits.test(K::WPA2_PSK)) append("WPA-PSK");
    if (bits.test(K::WPA_EAP)) append("WPA-EAP");
    if (bits.test(K::IEEE8021X)) append("IEEE8021X");
    if (bits.test(K::NONE)) append("NONE");
    if (bits.test(K::OSEN)) append("OSEN");
    if (bits.test(K::FT_PSK)) append("FT-PSK");
    if (bits.test(K::FT_EAP)) append("FT-EAP");
    if (bits.test(K::SAE)) append("SAE");
    if (bits.test(K::OWE)) append("OWE");
    if (bits.test(K::SUITE_B_192)) append("WPA-EAP-SUITE-B-192");
    if (bits.test(K::WPA_PSK_SHA256)) append("WPA-PSK-SHA256");
    if (bits.test(K::WPA_EAP_SHA256)) append("WPA-EAP-SHA256");
    if (bits.test(K::WAPI_PSK)) append("WAPI-PSK");
    if (bits.test(K::WAPI_CERT)) append("WAPI-CERT");
    if (bits.test(K::FILS_SHA256)) append("FILS-SHA256");
    if (bits.test(K::FILS_SHA384)) append("FILS-SHA384");
    if (bits.test(K::DPP)) append("DPP");
    return out;
}

void WpaResponseParser::applyProtocols(WifiConfiguration& config, const std::string& tokens) {
    for (const std::string& token : splitTokens(tokens)) {
        if (token == "WPA") config.allowedProtocols.set(WifiConfiguration::Protocol::WPA);
        else if (token == "RSN") config.allowedProtocols.set(WifiConfiguration::Protocol::RSN);
        else if (token == "OSEN") config.allowedProtocols.set(WifiConfiguration::Protocol::OSEN);
        else if (token == "WAPI") config.allowedProtocols.set(WifiConfiguration::Protocol::WAPI);
    }
}

std::string WpaResponseParser::protocolsString(const WifiConfiguration& config) {
    std::string out;
    const auto append = [&out](const char* token) {
        if (!out.empty()) out += " ";
        out += token;
    };
    if (config.allowedProtocols.test(WifiConfiguration::Protocol::WPA)) append("WPA");
    if (config.allowedProtocols.test(WifiConfiguration::Protocol::RSN)) append("RSN");
    if (config.allowedProtocols.test(WifiConfiguration::Protocol::OSEN)) append("OSEN");
    if (config.allowedProtocols.test(WifiConfiguration::Protocol::WAPI)) append("WAPI");
    return out;
}

void WpaResponseParser::applyPairwiseCiphers(WifiConfiguration& config, const std::string& tokens) {
    using P = WifiConfiguration::PairwiseCipher;
    for (const std::string& token : splitTokens(tokens)) {
        if (token == "NONE") config.allowedPairwiseCiphers.set(P::NONE);
        else if (token == "TKIP") config.allowedPairwiseCiphers.set(P::TKIP);
        else if (token == "CCMP") config.allowedPairwiseCiphers.set(P::CCMP);
        else if (token == "GCMP") config.allowedPairwiseCiphers.set(P::GCMP_128);
        else if (token == "GCMP-256") config.allowedPairwiseCiphers.set(P::GCMP_256);
        else if (token == "SMS4") config.allowedPairwiseCiphers.set(P::SMS4);
    }
}

std::string WpaResponseParser::pairwiseCiphersString(const WifiConfiguration& config) {
    using P = WifiConfiguration::PairwiseCipher;
    std::string out;
    const auto append = [&out](const char* token) {
        if (!out.empty()) out += " ";
        out += token;
    };
    if (config.allowedPairwiseCiphers.test(P::NONE)) append("NONE");
    if (config.allowedPairwiseCiphers.test(P::TKIP)) append("TKIP");
    if (config.allowedPairwiseCiphers.test(P::CCMP)) append("CCMP");
    if (config.allowedPairwiseCiphers.test(P::GCMP_128)) append("GCMP");
    if (config.allowedPairwiseCiphers.test(P::GCMP_256)) append("GCMP-256");
    if (config.allowedPairwiseCiphers.test(P::SMS4)) append("SMS4");
    return out;
}

void WpaResponseParser::applyGroupCiphers(WifiConfiguration& config, const std::string& tokens) {
    using G = WifiConfiguration::GroupCipher;
    for (const std::string& token : splitTokens(tokens)) {
        if (token == "WEP-40") config.allowedGroupCiphers.set(G::WEP40);
        else if (token == "WEP-104") config.allowedGroupCiphers.set(G::WEP104);
        else if (token == "TKIP") config.allowedGroupCiphers.set(G::TKIP);
        else if (token == "CCMP") config.allowedGroupCiphers.set(G::CCMP);
        else if (token == "GCMP") config.allowedGroupCiphers.set(G::GCMP_128);
        else if (token == "GCMP-256") config.allowedGroupCiphers.set(G::GCMP_256);
        else if (token == "SMS4") config.allowedGroupCiphers.set(G::SMS4);
    }
}

std::string WpaResponseParser::groupCiphersString(const WifiConfiguration& config) {
    using G = WifiConfiguration::GroupCipher;
    std::string out;
    const auto append = [&out](const char* token) {
        if (!out.empty()) out += " ";
        out += token;
    };
    if (config.allowedGroupCiphers.test(G::WEP40)) append("WEP-40");
    if (config.allowedGroupCiphers.test(G::WEP104)) append("WEP-104");
    if (config.allowedGroupCiphers.test(G::TKIP)) append("TKIP");
    if (config.allowedGroupCiphers.test(G::CCMP)) append("CCMP");
    if (config.allowedGroupCiphers.test(G::GCMP_128)) append("GCMP");
    if (config.allowedGroupCiphers.test(G::GCMP_256)) append("GCMP-256");
    if (config.allowedGroupCiphers.test(G::SMS4)) append("SMS4");
    return out;
}

void WpaResponseParser::applyAuthAlgorithms(WifiConfiguration& config, const std::string& tokens) {
    using A = WifiConfiguration::AuthAlgorithm;
    for (const std::string& token : splitTokens(tokens)) {
        if (token == "OPEN") config.allowedAuthAlgorithms.set(A::OPEN);
        else if (token == "SHARED") config.allowedAuthAlgorithms.set(A::SHARED);
        else if (token == "LEAP") config.allowedAuthAlgorithms.set(A::LEAP);
        else if (token == "SAE") config.allowedAuthAlgorithms.set(A::SAE);
    }
}

std::string WpaResponseParser::authAlgorithmsString(const WifiConfiguration& config) {
    using A = WifiConfiguration::AuthAlgorithm;
    std::string out;
    const auto append = [&out](const char* token) {
        if (!out.empty()) out += " ";
        out += token;
    };
    if (config.allowedAuthAlgorithms.test(A::OPEN)) append("OPEN");
    if (config.allowedAuthAlgorithms.test(A::SHARED)) append("SHARED");
    if (config.allowedAuthAlgorithms.test(A::LEAP)) append("LEAP");
    if (config.allowedAuthAlgorithms.test(A::SAE)) append("SAE");
    return out;
}

/* --- WifiConfiguration → SET_NETWORK arguments ----------------------------- */

std::vector<std::pair<std::string, std::string>> WpaResponseParser::networkVariables(
        const WifiConfiguration& config) {
    std::vector<std::pair<std::string, std::string>> vars;
    /* SSID is stored in the quoted WifiConfiguration convention; quote bare
     * values, pass a stored 64-char PSK through raw like wpa_cli does. */
    if (!config.SSID.empty())
        vars.push_back({"ssid", config.SSID.front() == '"' ? config.SSID : "\"" + config.SSID + "\""});
    if (!config.BSSID.empty() && config.BSSID != "any")
        vars.push_back({"bssid", config.BSSID});
    if (!config.preSharedKey.empty())
        vars.push_back({"psk", isAllHex(config.preSharedKey) && config.preSharedKey.size() == 64
                        ? config.preSharedKey
                        : "\"" + config.preSharedKey + "\""});
    const std::string keyMgmt = keyMgmtString(config);
    if (!keyMgmt.empty())
        vars.push_back({"key_mgmt", keyMgmt});
    const std::string proto = protocolsString(config);
    if (!proto.empty())
        vars.push_back({"proto", proto});
    const std::string pairwise = pairwiseCiphersString(config);
    if (!pairwise.empty())
        vars.push_back({"pairwise", pairwise});
    const std::string group = groupCiphersString(config);
    if (!group.empty())
        vars.push_back({"group", group});
    if (config.priority != 0)
        vars.push_back({"priority", std::to_string(config.priority)});
    if (config.hiddenSSID)
        vars.push_back({"scan_ssid", "1"});
    const std::string authAlg = authAlgorithmsString(config);
    if (!authAlg.empty())
        vars.push_back({"auth_alg", authAlg});
    for (size_t i = 0; i < config.wepKeys.size(); i++) {
        if (config.wepKeys[i].empty()) continue;
        /* A hex WEP key (40/104/232-bit = 10/26/58 chars) must go raw, or
         * wpa derives the key from the ASCII characters instead of the hex
         * bytes (wpa_config_parse_string: unquoted even-length hex is hex).
         * Real ASCII passphrases are 5/13/29 chars — odd, never misrouted. */
        const bool hexKey = isAllHex(config.wepKeys[i]) && config.wepKeys[i].size() % 2 == 0;
        vars.push_back({"wep_key" + std::to_string(i),
                hexKey ? config.wepKeys[i] : "\"" + config.wepKeys[i] + "\""});
    }
    if (config.wepTxKeyIndex != 0)
        vars.push_back({"wep_tx_key", std::to_string(config.wepTxKeyIndex)});
    return vars;
}

} // namespace cdroid
