/* Port of android.net.wifi.ScanResult (android-36). */
#include <wifi/scanresult.h>

#include <wifi/wifimanager.h>

namespace cdroid {

ScanResult::ScanResult() {
}

std::string ScanResult::wifiStandardToString(int wifiStandard) {
    switch (wifiStandard) {
    case WIFI_STANDARD_LEGACY: return "Legacy";
    case WIFI_STANDARD_11N:    return "11n";
    case WIFI_STANDARD_11AC:   return "11ac";
    case WIFI_STANDARD_11AX:   return "11ax";
    case WIFI_STANDARD_11AD:   return "11ad";
    case WIFI_STANDARD_11BE:   return "11be";
    default:                   return "Unknown";
    }
}

bool ScanResult::isPasspointNetwork() const {
    /* TODO(porting): FLAG_PASSPOINT_NETWORK bit constant once passpoint
     * data is available; ctrl_iface never sets it today. */
    return false;
}

std::string ScanResult::toString() const {
    std::string sb;
    const std::string none = "<none>";
    sb += "SSID: ";
    /* wifiSsid empty (Java null) prints WifiManager.UNKNOWN_SSID */
    sb += wifiSsid.getBytes().empty() ? WifiManager::UNKNOWN_SSID : wifiSsid.toString();
    sb += ", BSSID: ";
    sb += BSSID.empty() ? none : BSSID;
    sb += ", capabilities: ";
    sb += capabilities.empty() ? none : capabilities;
    sb += ", level: " + std::to_string(level);
    sb += ", frequency: " + std::to_string(frequency);
    sb += ", timestamp: " + std::to_string(timestamp);
    sb += ", distance: ";
    sb += (distanceCm != UNSPECIFIED ? std::to_string(distanceCm) : "?");
    sb += "(cm)";
    sb += ", distanceSd: ";
    sb += (distanceSdCm != UNSPECIFIED ? std::to_string(distanceSdCm) : "?");
    sb += "(cm)";
    sb += ", passpoint: no"; /* flags never set via ctrl_iface */
    sb += ", ChannelBandwidth: " + std::to_string(channelWidth);
    sb += ", centerFreq0: " + std::to_string(centerFreq0);
    sb += ", centerFreq1: " + std::to_string(centerFreq1);
    sb += ", standard: " + wifiStandardToString(mWifiStandard);
    sb += ", 80211mcResponder: is not supported"; /* TODO: FLAG_80211mc_RESPONDER */
    /* TODO(porting): 80211az/TWT responder flags, radio chain infos and
     * MLO lines follow once those fields are populated. */
    sb += ", interface name: " + ifaceName;
    return sb;
}

} // namespace cdroid
