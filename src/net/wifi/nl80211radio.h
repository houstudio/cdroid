#ifndef __NL80211_RADIO_H__
#define __NL80211_RADIO_H__

#include <string>
#include <vector>

#include <wifi/wifiradio.h>

namespace cdroid {

/**
 * Linux backend of WifiRadioData: NL80211_CMD_GET_SCAN dump over generic
 * netlink. Fills what the supplicant ctrl_iface list cannot: TSF timestamp,
 * raw information elements, channel width / center frequencies / standard
 * (derived from the HT/VHT operation elements like AOSP's
 * InformationElementUtil) and a capabilities string rendered from the
 * RSN/WPA elements.
 */
class Nl80211Radio : public WifiRadioData {
public:
    Nl80211Radio();
    ~Nl80211Radio() override;

    std::vector<ScanResult> getScanResults(const std::string& iface) override;

    /* ---- pure IE helpers (unit-testable) ---- */
    /* Split a beacon/probe IE stream (id, len, bytes...) into records. */
    static std::vector<ScanResult::InformationElement> parseIeStream(
            const unsigned char* ies, size_t length);
    /* Fill channelWidth / centerFreq0/1 / mWifiStandard / SSID from the
     * parsed elements (AOSP InformationElementUtil derivation). */
    static void applyScanEnhancements(ScanResult& result);
    /* "[WPA2-PSK-CCMP][ESS]" style string from RSN/WPA elements + the
     * 802.11 capability bitmap (same vocabulary the supplicant uses). */
    static std::string buildCapabilitiesString(
            const std::vector<ScanResult::InformationElement>& ies,
            unsigned capabilityInfo);

private:
    int resolveFamilyId();
    bool ensureStarted();

    int mSock = -1;
    int mFamilyId = -1;   /* resolved nl80211 generic-netlink family id */
};

} // namespace cdroid

#endif /* __NL80211_RADIO_H__ */
