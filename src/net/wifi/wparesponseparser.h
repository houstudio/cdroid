#ifndef __WPA_RESPONSE_PARSER_H__
#define __WPA_RESPONSE_PARSER_H__

#include <string>
#include <utility>
#include <vector>

#include <wifi/scanresult.h>
#include <wifi/wificonfiguration.h>
#include <wifi/wifiinfo.h>

namespace cdroid {

/**
 * Pure string→object mapping for wpa_supplicant ctrl_iface replies — the
 * layer AOSP implements inside SupplicantStaIfaceHal / WifiNative. Every
 * function here is side-effect free so the parsers are unit-testable
 * without a running daemon.
 */
class WpaResponseParser {
public:
    /* "\"text\"" → text; a value without quotes is returned as-is. */
    static std::string unquote(const std::string& value);

    /* "bssid / frequency / signal level / flags / ssid" table → ScanResults.
     * SSID keeps the wpa convention: quoted UTF-8 or unquoted hex / empty. */
    static std::vector<ScanResult> parseScanResults(const std::string& scanResults);

    /* STATUS key=value lines → WifiInfo fields (bssid, ssid, id, key_mgmt,
     * wpa_state, ip_address, freq when present). Only the fields present in
     * the reply are touched. */
    static void applyStatus(WifiInfo& info, const std::string& status);

    /* SIGNAL_POLL reply (RSSI=/LINKSPEED=/FREQUENCY= lines) → WifiInfo. */
    static void applySignalPoll(WifiInfo& info, const std::string& signalPoll);

    /* LIST_NETWORKS row: network id, ssid (raw), flags ([CURRENT][DISABLED]). */
    struct NetworkListEntry {
        int networkId;
        std::string ssid;   /* raw value: quoted or hex, exactly as stored */
        std::string flags;
    };
    static std::vector<NetworkListEntry> parseListNetworks(const std::string& listNetworks);

    /* One GET_NETWORK variable applied onto a WifiConfiguration. Recognized
     * variables: ssid, bssid, psk, key_mgmt, proto, pairwise, group,
     * auth_alg, priority, scan_ssid, disabled. Unknown ones are ignored. */
    static void applyNetworkVariable(WifiConfiguration& config, const std::string& variable,
                                     const std::string& value);

    /* WifiConfiguration → wpa_supplicant SET_NETWORK arguments, in the order
     * the variables appear in AOSP's saveNetwork (WifiConfigManager). The
     * SSID/PSK values are pre-quoted with escaping exactly as wpa_cli would
     * type them ("\"" + s + "\""). */
    static std::vector<std::pair<std::string, std::string>> networkVariables(
            const WifiConfiguration& config);

    /* key_mgmt tokens ("WPA-PSK WPA-EAP") ↔ allowedKeyManagement bits, and
     * the same for proto / pairwise / group / auth_alg. */
    static void applyKeyMgmt(WifiConfiguration& config, const std::string& tokens);
    static std::string keyMgmtString(const WifiConfiguration& config);
    static void applyProtocols(WifiConfiguration& config, const std::string& tokens);
    static std::string protocolsString(const WifiConfiguration& config);
    static void applyPairwiseCiphers(WifiConfiguration& config, const std::string& tokens);
    static std::string pairwiseCiphersString(const WifiConfiguration& config);
    static void applyGroupCiphers(WifiConfiguration& config, const std::string& tokens);
    static std::string groupCiphersString(const WifiConfiguration& config);
    static void applyAuthAlgorithms(WifiConfiguration& config, const std::string& tokens);
    static std::string authAlgorithmsString(const WifiConfiguration& config);
};

} // namespace cdroid

#endif /* __WPA_RESPONSE_PARSER_H__ */
