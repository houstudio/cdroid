#ifndef __WIFI_CONFIGURATION_H__
#define __WIFI_CONFIGURATION_H__

#include <array>
#include <bitset>
#include <cstdint>
#include <string>
#include <vector>

namespace cdroid {

/**
 * Port of android.net.wifi.WifiConfiguration (android-36), trimmed to what
 * wpa_supplicant's network block (GET_NETWORK/SET_NETWORK) can round-trip.
 * Java BitSet becomes std::bitset<64> (every AOSP cipher/keymgmt constant is
 * below 64); WifiEnterpriseConfig / IpConfiguration / NetworkSelectionStatus
 * depend on unported types and are TODO, following the faithful-stub rule.
 */
class WifiConfiguration {
public:
    static constexpr int INVALID_NETWORK_ID = -1;

    class Status {
    public:
        static constexpr int CURRENT  = 0;
        static constexpr int DISABLED = 1;
        static constexpr int ENABLED  = 2;
        static std::string statusToString(int status);
    };

    /* Recognized key management schemes (wpa_supplicant key_mgmt). */
    class KeyMgmt {
    public:
        static constexpr int NONE           = 0;
        static constexpr int WPA_PSK        = 1;
        static constexpr int WPA_EAP        = 2;
        static constexpr int IEEE8021X      = 3;
        static constexpr int WPA2_PSK       = 4;
        static constexpr int OSEN           = 5;
        static constexpr int FT_PSK         = 6;
        static constexpr int FT_EAP         = 7;
        static constexpr int SAE            = 8;
        static constexpr int OWE            = 9;
        static constexpr int SUITE_B_192    = 10;
        static constexpr int WPA_PSK_SHA256 = 11;
        static constexpr int WPA_EAP_SHA256 = 12;
        static constexpr int WAPI_PSK       = 13;
        static constexpr int WAPI_CERT      = 14;
        static constexpr int FILS_SHA256    = 15;
        static constexpr int FILS_SHA384    = 16;
        static constexpr int DPP            = 17;
    };

    class Protocol {
    public:
        static constexpr int WPA  = 0;
        static constexpr int RSN  = 1;
        static constexpr int OSEN = 2;
        static constexpr int WAPI = 3;
    };

    class PairwiseCipher {
    public:
        static constexpr int NONE      = 0;
        static constexpr int TKIP      = 1;
        static constexpr int CCMP      = 2;
        static constexpr int GCMP_256  = 3;
        static constexpr int SMS4      = 4;
        static constexpr int GCMP_128  = 5;
    };

    class GroupCipher {
    public:
        static constexpr int WEP40         = 0;
        static constexpr int WEP104        = 1;
        static constexpr int TKIP          = 2;
        static constexpr int CCMP          = 3;
        static constexpr int GTK_NOT_USED  = 4;
        static constexpr int GCMP_256      = 5;
        static constexpr int SMS4          = 6;
        static constexpr int GCMP_128      = 7;
    };

    class AuthAlgorithm {
    public:
        static constexpr int OPEN   = 0;
        static constexpr int SHARED = 1;
        static constexpr int LEAP   = 2;
        static constexpr int SAE    = 3;
    };

    /* Soft AP band selection (WifiConfiguration#AP_BAND_*, @deprecated in
     * favor of SoftApConfiguration but kept for the legacy startSoftAp). */
    static constexpr int AP_BAND_2GHZ  = 0;
    static constexpr int AP_BAND_5GHZ  = 1;
    static constexpr int AP_BAND_60GHZ = 2;
    static constexpr int AP_BAND_ANY   = -1;

    /* Canonical security type list (shared with WifiInfo). */
    static constexpr int SECURITY_TYPE_UNKNOWN = -1;
    static constexpr int SECURITY_TYPE_OPEN = 0;
    static constexpr int SECURITY_TYPE_WEP = 1;
    static constexpr int SECURITY_TYPE_PSK = 2;
    static constexpr int SECURITY_TYPE_EAP = 3;
    static constexpr int SECURITY_TYPE_SAE = 4;
    static constexpr int SECURITY_TYPE_EAP_WPA3_ENTERPRISE_192_BIT = 5;
    static constexpr int SECURITY_TYPE_OWE = 6;
    static constexpr int SECURITY_TYPE_WAPI_PSK = 7;
    static constexpr int SECURITY_TYPE_WAPI_CERT = 8;
    static constexpr int SECURITY_TYPE_EAP_WPA3_ENTERPRISE = 9;
    static constexpr int SECURITY_TYPE_OSEN = 10;
    static constexpr int SECURITY_TYPE_PASSPOINT_R1_R2 = 11;
    static constexpr int SECURITY_TYPE_PASSPOINT_R3 = 12;
    static constexpr int SECURITY_TYPE_DPP = 13;

    /* --- public state ---------------------------------------------------- */

    int networkId = INVALID_NETWORK_ID;
    int status = Status::CURRENT;   /* Java int default; CURRENT == 0 */
    /* Quoted SSID string per WifiSsid#toString ("\"MyNetwork\""). */
    std::string SSID;
    std::string BSSID;
    std::string preSharedKey;
    std::array<std::string, 4> wepKeys;
    int wepTxKeyIndex = 0;
    int priority = 0;
    bool hiddenSSID = false;
    bool requirePmf = false;

    std::bitset<64> allowedKeyManagement;
    std::bitset<64> allowedProtocols;
    std::bitset<64> allowedAuthAlgorithms;
    std::bitset<64> allowedPairwiseCiphers;
    std::bitset<64> allowedGroupCiphers;
    std::bitset<64> allowedGroupManagementCiphers;
    std::bitset<64> allowedSuiteBCiphers;

    std::string FQDN;
    std::string providerFriendlyName;
    bool isHomeProviderNetwork = false;
    std::vector<int64_t> roamingConsortiumIds;
    bool shared = true;

    std::string dhcpServer;
    std::string defaultGwMacAddress;
    /* Soft AP operating band/channel (AP_BAND_*; channel 0 = auto). */
    int apBand = AP_BAND_2GHZ;
    int apChannel = 0;
    bool validatedInternetAccess = false;
    int creatorUid = -1;
    int lastConnectUid = -1;
    int lastUpdateUid = -1;
    std::string creatorName;
    std::string lastUpdateName;
    int numNoInternetAccessReports = 0;
    bool noInternetAccessExpected = false;
    bool osu = false;
    int64_t lastConnected = 0;
    int64_t lastDisconnected = 0;
    int64_t lastUpdated = 0;
    int numRebootsSinceLastUse = 0;
    bool selfAdded = false;
    std::string peerWifiConfiguration;
    bool ephemeral = false;
    bool trusted = true;   /* Networks are considered trusted by default. */
    bool oemPaid = false;
    bool oemPrivate = false;
    bool carrierMerged = false;
    bool fromWifiNetworkSuggestion = false;
    bool fromWifiNetworkSpecifier = false;
    bool meteredHint = false;
    bool useExternalScores = false;
    bool restricted = false;
    int mDeletionPriority = 0;
    int dtimInterval = 0;
    /* TODO(porting): enterpriseConfig (WifiEnterpriseConfig), mIpConfiguration
     * (IpConfiguration/StaticIpConfiguration — shared with the planned
     * EthernetManager), NetworkSelectionStatus, randomized MAC, carrier
     * fields: types not ported yet. */

    WifiConfiguration();
    WifiConfiguration(const WifiConfiguration&) = default;
    WifiConfiguration& operator=(const WifiConfiguration&) = default;

    /* @deprecated Single auth type derived from allowedKeyManagement; throws
     * std::logic_error (Java IllegalStateException) on invalid combinations. */
    int getAuthType();
    std::string toString() const;
};

} // namespace cdroid

#endif /* __WIFI_CONFIGURATION_H__ */
