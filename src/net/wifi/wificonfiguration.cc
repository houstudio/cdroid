/* Port of android.net.wifi.WifiConfiguration (android-36). */
#include <wifi/wificonfiguration.h>

#include <stdexcept>

namespace cdroid {

std::string WifiConfiguration::Status::statusToString(int status) {
    switch (status) {
    case CURRENT:  return "CURRENT";
    case ENABLED:  return "ENABLED";
    case DISABLED: return "DISABLED";
    default:       return std::string();
    }
}

WifiConfiguration::WifiConfiguration() {
}

int WifiConfiguration::getAuthType() {
    if (allowedKeyManagement.count() > 1) {
        if (allowedKeyManagement.test(KeyMgmt::WPA_EAP)) {
            if (allowedKeyManagement.count() == 2
                    && allowedKeyManagement.test(KeyMgmt::IEEE8021X)) {
                return KeyMgmt::WPA_EAP;
            }
            if (allowedKeyManagement.count() == 3
                    && allowedKeyManagement.test(KeyMgmt::IEEE8021X)
                    && allowedKeyManagement.test(KeyMgmt::SUITE_B_192)) {
                return KeyMgmt::SUITE_B_192;
            }
        }
        throw std::logic_error("Invalid auth type set");
    }
    if (allowedKeyManagement.test(KeyMgmt::WPA_PSK)) {
        return KeyMgmt::WPA_PSK;
    } else if (allowedKeyManagement.test(KeyMgmt::WPA2_PSK)) {
        return KeyMgmt::WPA2_PSK;
    } else if (allowedKeyManagement.test(KeyMgmt::WPA_EAP)) {
        return KeyMgmt::WPA_EAP;
    } else if (allowedKeyManagement.test(KeyMgmt::IEEE8021X)) {
        return KeyMgmt::IEEE8021X;
    } else if (allowedKeyManagement.test(KeyMgmt::SAE)) {
        return KeyMgmt::SAE;
    } else if (allowedKeyManagement.test(KeyMgmt::OWE)) {
        return KeyMgmt::OWE;
    } else if (allowedKeyManagement.test(KeyMgmt::SUITE_B_192)) {
        return KeyMgmt::SUITE_B_192;
    } else if (allowedKeyManagement.test(KeyMgmt::WAPI_PSK)) {
        return KeyMgmt::WAPI_PSK;
    } else if (allowedKeyManagement.test(KeyMgmt::WAPI_CERT)) {
        return KeyMgmt::WAPI_CERT;
    } else if (allowedKeyManagement.test(KeyMgmt::DPP)) {
        return KeyMgmt::DPP;
    }
    return KeyMgmt::NONE;
}

std::string WifiConfiguration::toString() const {
    std::string sbuf;
    if (status == Status::CURRENT) {
        sbuf += "* ";
    } else if (status == Status::DISABLED) {
        sbuf += "- DSBLE ";
    }
    sbuf += "ID: " + std::to_string(networkId) + " SSID: " + SSID
            + " PROVIDER-NAME: " + providerFriendlyName
            + " BSSID: " + BSSID
            + " FQDN: " + FQDN
            + " HOME-PROVIDER-NETWORK: " + std::string(isHomeProviderNetwork ? "true" : "false")
            + " PRIO: " + std::to_string(priority)
            + " HIDDEN: " + std::string(hiddenSSID ? "true" : "false")
            + " PMF: " + std::string(requirePmf ? "true" : "false")
            + "\n";
    /* TODO(porting): NetworkSelectionStatus block follows once that class
     * is ported. */
    if (numNoInternetAccessReports > 0) {
        sbuf += " numNoInternetAccessReports " + std::to_string(numNoInternetAccessReports) + "\n";
    }
    if (validatedInternetAccess) sbuf += " validatedInternetAccess";
    if (shared) sbuf += " shared";
    else        sbuf += " not-shared";
    if (ephemeral) sbuf += " ephemeral";
    if (osu)       sbuf += " osu";
    if (trusted)   sbuf += " trusted";
    if (restricted) sbuf += " restricted";
    if (oemPaid)   sbuf += " oemPaid";
    if (oemPrivate) sbuf += " oemPrivate";
    if (carrierMerged) sbuf += " carrierMerged";
    if (fromWifiNetworkSuggestion) sbuf += " fromWifiNetworkSuggestion";
    if (fromWifiNetworkSpecifier) sbuf += " fromWifiNetworkSpecifier";
    if (meteredHint) sbuf += " meteredHint";
    if (useExternalScores) sbuf += " useExternalScores";
    if (validatedInternetAccess || ephemeral || trusted || oemPaid || oemPrivate
            || carrierMerged || fromWifiNetworkSuggestion || fromWifiNetworkSpecifier
            || meteredHint || useExternalScores || restricted) {
        sbuf += "\n";
    }
    sbuf += "SSID: " + SSID + "\n";
    /* PSK redacted exactly like AOSP ("\"*\""); never print the secret. */
    if (!preSharedKey.empty()) sbuf += "PreShared Key: \"*\"\n";
    for (const auto& wepKey : wepKeys) {
        if (!wepKey.empty()) {
            sbuf += "WEP Key: \"*\"\n";
            break;
        }
    }
    sbuf += "IPCONFIGURATION: TODO(porting)\n";
    return sbuf;
}

} // namespace cdroid
