#ifndef __SCAN_RESULT_H__
#define __SCAN_RESULT_H__

#include <cstdint>
#include <string>
#include <vector>

#include <wifi/wifissid.h>

namespace cdroid {

/**
 * Port of android.net.wifi.ScanResult (android-36), trimmed to what the
 * wpa_supplicant control interface can actually provide (bssid / frequency /
 * signal / flags / ssid lines of "SCAN_RESULTS"). Fields whose data comes
 * only from the vendor HAL (InformationElement, radioChainInfos, MLO, ...)
 * are present with defaults and marked TODO, following the faithful-stub
 * convention.
 */
class ScanResult {
public:
    /* Security protocol identifiers (IEEE 802.11). */
    static constexpr int PROTOCOL_NONE = 0;
    static constexpr int PROTOCOL_WPA  = 1;
    static constexpr int PROTOCOL_RSN  = 2;
    static constexpr int PROTOCOL_OSEN = 3;
    static constexpr int PROTOCOL_WAPI = 4;

    /* Key management schemes. */
    static constexpr int KEY_MGMT_NONE             = 0;
    static constexpr int KEY_MGMT_PSK              = 1;
    static constexpr int KEY_MGMT_EAP              = 2;
    static constexpr int KEY_MGMT_FT_PSK           = 3;
    static constexpr int KEY_MGMT_FT_EAP           = 4;
    static constexpr int KEY_MGMT_PSK_SHA256       = 5;
    static constexpr int KEY_MGMT_EAP_SHA256       = 6;
    static constexpr int KEY_MGMT_OSEN             = 7;
    static constexpr int KEY_MGMT_SAE              = 8;
    static constexpr int KEY_MGMT_OWE              = 9;
    static constexpr int KEY_MGMT_EAP_SUITE_B_192  = 10;
    static constexpr int KEY_MGMT_FT_SAE           = 11;
    static constexpr int KEY_MGMT_OWE_TRANSITION   = 12;
    static constexpr int KEY_MGMT_WAPI_PSK         = 13;
    static constexpr int KEY_MGMT_WAPI_CERT        = 14;
    static constexpr int KEY_MGMT_FILS_SHA256      = 15;
    static constexpr int KEY_MGMT_FILS_SHA384      = 16;
    static constexpr int KEY_MGMT_DPP              = 17;
    static constexpr int KEY_MGMT_SAE_EXT_KEY      = 18;
    static constexpr int KEY_MGMT_FT_SAE_EXT_KEY   = 19;
    static constexpr int KEY_MGMT_PASN             = 20;
    static constexpr int KEY_MGMT_EAP_FT_SHA384    = 21;
    static constexpr int KEY_MGMT_FT_PSK_SHA384    = 22;
    static constexpr int KEY_MGMT_UNKNOWN          = 23;

    /* Cipher suites. */
    static constexpr int CIPHER_NONE              = 0;
    static constexpr int CIPHER_NO_GROUP_ADDRESSED = 1;
    static constexpr int CIPHER_TKIP              = 2;
    static constexpr int CIPHER_CCMP              = 3;
    static constexpr int CIPHER_GCMP_256          = 4;
    static constexpr int CIPHER_SMS4              = 5;
    static constexpr int CIPHER_GCMP_128          = 6;
    static constexpr int CIPHER_BIP_GMAC_128      = 7;
    static constexpr int CIPHER_BIP_GMAC_256      = 8;
    static constexpr int CIPHER_BIP_CMAC_256      = 9;
    static constexpr int CIPHER_CCMP_256          = 10;

    /* AP channel bandwidth (channelWidth). */
    static constexpr int CHANNEL_WIDTH_20MHZ       = 0;
    static constexpr int CHANNEL_WIDTH_40MHZ       = 1;
    static constexpr int CHANNEL_WIDTH_80MHZ       = 2;
    static constexpr int CHANNEL_WIDTH_160MHZ      = 3;
    static constexpr int CHANNEL_WIDTH_80MHZ_PLUS_MHZ = 4;
    static constexpr int CHANNEL_WIDTH_320MHZ      = 5;

    /* Wi-Fi standards (2 and 3 are reserved by AOSP). */
    static constexpr int WIFI_STANDARD_UNKNOWN = 0;
    static constexpr int WIFI_STANDARD_LEGACY  = 1;
    static constexpr int WIFI_STANDARD_11N     = 4;
    static constexpr int WIFI_STANDARD_11AC    = 5;
    static constexpr int WIFI_STANDARD_11AX    = 6;
    static constexpr int WIFI_STANDARD_11AD    = 7;
    static constexpr int WIFI_STANDARD_11BE    = 8;

    static constexpr int UNSPECIFIED = -1; /* distanceCm/distanceSdCm sentinel */

    /**
     * Port of android.net.wifi.ScanResult.InformationElement (android-36):
     * one 802.11 information element — id plus raw bytes. The supplicant
     * ctrl_iface path carries none; the radio (nl80211/WlanApi) path does.
     */
    class InformationElement {
    public:
        int id = -1;                          /* element id */
        std::vector<unsigned char> bytes;     /* payload after the length byte */
    };

    /* The network name (quoted legacy form + WifiSsid form). */
    std::string SSID;
    WifiSsid wifiSsid;
    /* The access point BSSID. */
    std::string BSSID;
    /* Security capabilities string, passed through verbatim from
     * wpa_supplicant flags, e.g. "[WPA2-PSK-CCMP][ESS]". */
    std::string capabilities;
    /* Name of the interface this result was obtained on. */
    std::string ifaceName;

    int64_t hessid = 0;
    int anqpDomainId = 0;
    /* Signal level in dBm, 0 valid, -1 invalid. */
    int level = 0;
    /* Primary 20MHz channel frequency in MHz. */
    int frequency = 0;
    /* CHANNEL_WIDTH_*. ctrl_iface does not report it: stays 20MHZ default. */
    int channelWidth = CHANNEL_WIDTH_20MHZ;
    /* Center frequency of the contiguous band, or of the 160/80+80, in MHz. */
    int centerFreq0 = 0;
    int centerFreq1 = 0;
    /* Distance in centimeters to the AP (802.11mc; UNSPECIFIED without RTT). */
    int distanceCm = UNSPECIFIED;
    int distanceSdCm = UNSPECIFIED;
    /* TODO(porting): FLAG_PASSPOINT_NETWORK etc. bit constants + radio
     * chain infos + MLO links need data beyond the current radio dump. */
    int flags = 0;
    int mWifiStandard = WIFI_STANDARD_UNKNOWN;
    /* Timestamp in microseconds of when the scan result was seen (the
     * beacon TSF from the radio dump; 0 on the ctrl_iface fallback). */
    int64_t timestamp = 0;
    /* Raw information elements from the radio dump (empty on the
     * ctrl_iface fallback). */
    std::vector<InformationElement> informationElements;

    /* Construct an empty scan result. */
    ScanResult();

    static std::string wifiStandardToString(int wifiStandard);
    /* AOSP isHiddenSsid(): an all-zero SSID is the "hidden" encoding. */
    static bool isHiddenSsid(const WifiSsid& wifiSsid);
    /* The SSID display mapping AOSP applies where ScanResult instances are
     * built (ScanResult.java:1706-1713): hidden -> "", decodable -> plain
     * utf8 text (no quotes — quoting belongs to WifiConfiguration.SSID),
     * otherwise WifiManager.UNKNOWN_SSID. */
    static std::string displaySsid(const WifiSsid& wifiSsid);
    bool isPasspointNetwork() const;
    std::string toString() const;
};

} // namespace cdroid

#endif /* __SCAN_RESULT_H__ */
