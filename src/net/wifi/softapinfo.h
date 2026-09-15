#ifndef __SOFT_AP_INFO_H__
#define __SOFT_AP_INFO_H__

#include <cstdint>
#include <string>

namespace cdroid {

/**
 * Port of android.net.wifi.SoftApInfo (android-36), trimmed to the
 * operating-state core: frequency / channel bandwidth / Wi-Fi standard.
 * The Builder, apInterfaceIdentifier (unported MAC/UUID types) and the
 * BAKLAVA+ flagged fields are TODO, following the faithful-stub rule.
 * SoftApConfiguration#Builder carries the channel-bandwidth fields, so this
 * port exists to anchor the CHANNEL_WIDTH_* constants they share.
 */
class SoftApInfo {
public:
    /* Channel bandwidth (SoftApInfo#CHANNEL_WIDTH_*, aligned with
     * ScanResult.CHANNEL_WIDTH_* numbering like the AOSP original). */
    static constexpr int CHANNEL_WIDTH_AUTO      = 0;
    static constexpr int CHANNEL_WIDTH_20MHZ     = 1;
    static constexpr int CHANNEL_WIDTH_40MHZ     = 2;
    static constexpr int CHANNEL_WIDTH_80MHZ     = 3;
    static constexpr int CHANNEL_WIDTH_160MHZ    = 4;
    static constexpr int CHANNEL_WIDTH_320MHZ    = 5;
    static constexpr int CHANNEL_WIDTH_80MHZ_AFTER_MHZ = 6;

    /* Operating frequency in MHz (WifiScanner#WIFI_BAND_* derived); 0 when
     * unknown — Java 0 default. */
    int frequency = 0;
    /* One of CHANNEL_WIDTH_* (auto = 0 when not yet reported). */
    int bandwidth = CHANNEL_WIDTH_AUTO;
    /* SCAN_RESULT_STANDARD_* (ScanInfo); 0 = unknown in the AOSP original. */
    int wifiStandard = 0;

    int getFrequency() const { return frequency; }
    int getBandwidth() const { return bandwidth; }
    int getWifiStandard() const { return wifiStandard; }

    std::string toString() const;
};

} // namespace cdroid

#endif /* __SOFT_AP_INFO_H__ */
