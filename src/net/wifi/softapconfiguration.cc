/* Port of android.net.wifi.SoftApConfiguration (android-36). */
#include <wifi/softapconfiguration.h>

#include <algorithm>
#include <stdexcept>

#include <wifi/wificonfiguration.h>

namespace cdroid {

/* WifiManager.UNKNOWN_SSID literal, kept local to avoid the (fatal) include
 * cycle: WifiManager includes this header. */
static const char* const UNKNOWN_SSID = "<unknown ssid>";

/* Channel number ranges per band (AOSP private constants). */
static constexpr int MIN_CH_2G_BAND  = 1;
static constexpr int MAX_CH_2G_BAND  = 14;
static constexpr int MIN_CH_5G_BAND  = 34;
static constexpr int MAX_CH_5G_BAND  = 196;
static constexpr int MIN_CH_6G_BAND  = 1;
static constexpr int MAX_CH_6G_BAND  = 253;
static constexpr int MIN_CH_60G_BAND = 1;
static constexpr int MAX_CH_60G_BAND = 6;

bool SoftApConfiguration::isBandValid(int band) {
    const int bandAny = BAND_2GHZ | BAND_5GHZ | BAND_6GHZ | BAND_60GHZ;
    return band != 0 && (band & ~bandAny) == 0;
}

bool SoftApConfiguration::isChannelBandPairValid(int channel, int band) {
    switch (band) {
        case BAND_2GHZ:
            if (channel < MIN_CH_2G_BAND || channel > MAX_CH_2G_BAND) return false;
            break;
        case BAND_5GHZ:
            if (channel < MIN_CH_5G_BAND || channel > MAX_CH_5G_BAND) return false;
            break;
        case BAND_6GHZ:
            if (channel < MIN_CH_6G_BAND || channel > MAX_CH_6G_BAND) return false;
            break;
        case BAND_60GHZ:
            if (channel < MIN_CH_60G_BAND || channel > MAX_CH_60G_BAND) return false;
            break;
        default:
            return false;
    }
    return true;
}

/* --- private constructor (Builder + parcel path in AOSP) ------------------ */

SoftApConfiguration::SoftApConfiguration(const WifiSsid& ssid,
        const MacAddress& bssid, const std::string& passphrase, bool hiddenSsid,
        const std::map<int, int>& channels, int securityType,
        int maxNumberOfClients, bool shutdownTimeoutEnabled,
        int64_t shutdownTimeoutMillis, bool clientControlByUser,
        const std::vector<MacAddress>& blockedList,
        const std::vector<MacAddress>& allowedList,
        int macRandomizationSetting,
        bool bridgedModeOpportunisticShutdownEnabled,
        bool ieee80211axEnabled, bool ieee80211beEnabled,
        bool isUserConfiguration,
        int64_t bridgedModeOpportunisticShutdownTimeoutMillis,
        const std::vector<ScanResult::InformationElement>& vendorElements,
        const MacAddress& persistentRandomizedMacAddress,
        const std::set<int>& allowedAcsChannels24g,
        const std::set<int>& allowedAcsChannels5g,
        const std::set<int>& allowedAcsChannels6g,
        int maxChannelBandwidth, bool isClientIsolationEnabled)
    : mWifiSsid(ssid),
      mBssid(bssid),
      mPassphrase(passphrase),
      mHiddenSsid(hiddenSsid),
      mMaxNumberOfClients(maxNumberOfClients),
      mSecurityType(securityType),
      mAutoShutdownEnabled(shutdownTimeoutEnabled),
      mShutdownTimeoutMillis(shutdownTimeoutMillis),
      mClientControlByUser(clientControlByUser),
      mBlockedClientList(blockedList),
      mAllowedClientList(allowedList),
      mMacRandomizationSetting(macRandomizationSetting),
      mBridgedModeOpportunisticShutdownEnabled(bridgedModeOpportunisticShutdownEnabled),
      mIeee80211axEnabled(ieee80211axEnabled),
      mIeee80211beEnabled(ieee80211beEnabled),
      mIsUserConfiguration(isUserConfiguration),
      mBridgedModeOpportunisticShutdownTimeoutMillis(
              bridgedModeOpportunisticShutdownTimeoutMillis),
      mVendorElements(vendorElements),
      mPersistentRandomizedMacAddress(persistentRandomizedMacAddress),
      mAllowedAcsChannels2g(allowedAcsChannels24g),
      mAllowedAcsChannels5g(allowedAcsChannels5g),
      mAllowedAcsChannels6g(allowedAcsChannels6g),
      mMaxChannelBandwidth(maxChannelBandwidth),
      mIsClientIsolationEnabled(isClientIsolationEnabled) {
    if (!channels.empty()) {
        mChannels = channels;
    } else {
        mChannels[(int)BAND_2GHZ] = 0;   /* (int) cast: map[] binds a reference */
    }
}

/* --- getters --------------------------------------------------------------- */

std::string SoftApConfiguration::getSsid() const {
    /* Java: null SSID -> null; undecodable bytes -> WifiManager.UNKNOWN_SSID. */
    if (mWifiSsid.getBytes().empty()) return std::string();
    const std::string utf8 = mWifiSsid.getUtf8Text();
    return !utf8.empty() ? utf8 : std::string(UNKNOWN_SSID);
}

int SoftApConfiguration::getBand() const {
    /* SparseIntArray#keyAt(0) == lowest key: std::map iteration order. The
     * "never empty" invariant is enforced by construction (the no-arg ctor
     * is private to the Builder); the empty() guard keeps a stray
     * default-constructed instance from dereferencing end(). */
    return mChannels.empty() ? BAND_2GHZ : mChannels.begin()->first;
}

std::vector<int> SoftApConfiguration::getBands() const {
    std::vector<int> bands;
    for (const auto& entry : mChannels) bands.push_back(entry.first);
    return bands;
}

int SoftApConfiguration::getChannel() const {
    /* see getBand(): 0 = auto-select, the documented empty-map default */
    return mChannels.empty() ? 0 : mChannels.begin()->second;
}

std::set<int> SoftApConfiguration::getAllowedAcsChannels(int band) const {
    switch (band) {
        case BAND_2GHZ: return mAllowedAcsChannels2g;
        case BAND_5GHZ: return mAllowedAcsChannels5g;
        case BAND_6GHZ: return mAllowedAcsChannels6g;
        default:
            throw std::invalid_argument("getAllowedAcsChannels: Invalid band: "
                    + std::to_string(band));
    }
}

WifiConfiguration* SoftApConfiguration::toWifiConfiguration() const {
    WifiConfiguration* wifiConfig = new WifiConfiguration();
    const std::string utf8 = mWifiSsid.getUtf8Text();
    wifiConfig->SSID = !utf8.empty() ? utf8 : std::string(UNKNOWN_SSID);
    wifiConfig->preSharedKey = mPassphrase;
    wifiConfig->hiddenSSID = mHiddenSsid;
    wifiConfig->apChannel = getChannel();
    switch (mSecurityType) {
        case SECURITY_TYPE_OPEN:
            wifiConfig->allowedKeyManagement.set(WifiConfiguration::KeyMgmt::NONE);
            break;
        case SECURITY_TYPE_WPA2_PSK:
        case SECURITY_TYPE_WPA3_SAE_TRANSITION:
            wifiConfig->allowedKeyManagement.set(WifiConfiguration::KeyMgmt::WPA2_PSK);
            break;
        default:
            /* "Convert fail, unsupported security type" — Java returns null. */
            delete wifiConfig;
            return nullptr;
    }
    switch (getBand()) {
        case BAND_2GHZ:
            wifiConfig->apBand = WifiConfiguration::AP_BAND_2GHZ;
            break;
        case BAND_5GHZ:
            wifiConfig->apBand = WifiConfiguration::AP_BAND_5GHZ;
            break;
        case BAND_2GHZ | BAND_5GHZ:
            wifiConfig->apBand = WifiConfiguration::AP_BAND_ANY;
            break;
        case BAND_ANY:
            wifiConfig->apBand = WifiConfiguration::AP_BAND_ANY;
            break;
        default:
            delete wifiConfig;
            return nullptr;
    }
    return wifiConfig;
}

bool SoftApConfiguration::operator==(const SoftApConfiguration& other) const {
    /* Java compares the channel map via toString() — content equality. */
    return mWifiSsid == other.mWifiSsid
            && mBssid == other.mBssid
            && mPassphrase == other.mPassphrase
            && mHiddenSsid == other.mHiddenSsid
            && mChannels == other.mChannels
            && mSecurityType == other.mSecurityType
            && mMaxNumberOfClients == other.mMaxNumberOfClients
            && mAutoShutdownEnabled == other.mAutoShutdownEnabled
            && mShutdownTimeoutMillis == other.mShutdownTimeoutMillis
            && mClientControlByUser == other.mClientControlByUser
            && mBlockedClientList == other.mBlockedClientList
            && mAllowedClientList == other.mAllowedClientList
            && mMacRandomizationSetting == other.mMacRandomizationSetting
            && mBridgedModeOpportunisticShutdownEnabled
                    == other.mBridgedModeOpportunisticShutdownEnabled
            && mIeee80211axEnabled == other.mIeee80211axEnabled
            && mIeee80211beEnabled == other.mIeee80211beEnabled
            && mIsUserConfiguration == other.mIsUserConfiguration
            && mBridgedModeOpportunisticShutdownTimeoutMillis
                    == other.mBridgedModeOpportunisticShutdownTimeoutMillis
            && mVendorElements == other.mVendorElements
            && mPersistentRandomizedMacAddress == other.mPersistentRandomizedMacAddress
            && mAllowedAcsChannels2g == other.mAllowedAcsChannels2g
            && mAllowedAcsChannels5g == other.mAllowedAcsChannels5g
            && mAllowedAcsChannels6g == other.mAllowedAcsChannels6g
            && mMaxChannelBandwidth == other.mMaxChannelBandwidth
            && mIsClientIsolationEnabled == other.mIsClientIsolationEnabled;
}

bool SoftApConfiguration::operator!=(const SoftApConfiguration& other) const {
    return !(*this == other);
}

std::string SoftApConfiguration::toString() const {
    std::string sbuf;
    auto append = [&sbuf](const std::string& text) { sbuf += text; };
    append("ssid = " + (mWifiSsid.getBytes().empty()
                    ? std::string() : mWifiSsid.toString()));
    if (mBssid.getBytes().size() == 6) append(" \n bssid = " + mBssid.toString());
    append(" \n Passphrase = ");
    append(mPassphrase.empty() ? "<empty>" : "<non-empty>");
    append(std::string(" \n HiddenSsid = ") + (mHiddenSsid ? "true" : "false"));
    append(" \n Channels = {");
    for (const auto& entry : mChannels)
        append(std::to_string(entry.first) + "=" + std::to_string(entry.second) + ",");
    append("}");
    append(" \n SecurityType = " + std::to_string(getSecurityType()));
    append(" \n MaxClient = " + std::to_string(mMaxNumberOfClients));
    append(std::string(" \n AutoShutdownEnabled = ")
            + (mAutoShutdownEnabled ? "true" : "false"));
    append(" \n ShutdownTimeoutMillis = " + std::to_string(mShutdownTimeoutMillis));
    append(std::string(" \n ClientControlByUser = ")
            + (mClientControlByUser ? "true" : "false"));
    append(" \n BlockedClientList = [");
    for (const MacAddress& mac : mBlockedClientList) append(mac.toString() + ",");
    append("]");
    append(" \n AllowedClientList= [");
    for (const MacAddress& mac : mAllowedClientList) append(mac.toString() + ",");
    append("]");
    append(" \n MacRandomizationSetting = " + std::to_string(mMacRandomizationSetting));
    append(std::string(" \n BridgedModeInstanceOpportunisticEnabled = ")
            + (mBridgedModeOpportunisticShutdownEnabled ? "true" : "false"));
    append(" \n BridgedModeOpportunisticShutdownTimeoutMillis = "
            + std::to_string(mBridgedModeOpportunisticShutdownTimeoutMillis));
    append(std::string(" \n Ieee80211axEnabled = ")
            + (mIeee80211axEnabled ? "true" : "false"));
    append(std::string(" \n Ieee80211beEnabled = ")
            + (mIeee80211beEnabled ? "true" : "false"));
    append(std::string(" \n isUserConfiguration = ")
            + (mIsUserConfiguration ? "true" : "false"));
    append(" \n vendorElements size = " + std::to_string(mVendorElements.size()));
    append(" \n mPersistentRandomizedMacAddress = "
            + mPersistentRandomizedMacAddress.toString());
    append(" \n mAllowedAcsChannels2g size = "
            + std::to_string(mAllowedAcsChannels2g.size()));
    append(" \n mAllowedAcsChannels5g size = "
            + std::to_string(mAllowedAcsChannels5g.size()));
    append(" \n mAllowedAcsChannels6g size = "
            + std::to_string(mAllowedAcsChannels6g.size()));
    append(" \n mMaxChannelBandwidth = " + std::to_string(mMaxChannelBandwidth));
    append(std::string(" \n mIsClientIsolationEnabled = ")
            + (mIsClientIsolationEnabled ? "true" : "false"));
    return sbuf;
}

/* --- Builder ---------------------------------------------------------------- */

SoftApConfiguration::Builder::Builder() {
    mChannels[(int)BAND_2GHZ] = 0;   /* (int) cast: map[] binds a reference */
}

SoftApConfiguration::Builder::Builder(const SoftApConfiguration& other)
    : mWifiSsid(other.mWifiSsid),
      mBssid(other.mBssid),
      mPassphrase(other.mPassphrase),
      mHiddenSsid(other.mHiddenSsid),
      mChannels(other.mChannels),
      mMaxNumberOfClients(other.mMaxNumberOfClients),
      mSecurityType(other.mSecurityType),
      mAutoShutdownEnabled(other.mAutoShutdownEnabled),
      mShutdownTimeoutMillis(other.mShutdownTimeoutMillis),
      mClientControlByUser(other.mClientControlByUser),
      mBlockedClientList(other.mBlockedClientList),
      mAllowedClientList(other.mAllowedClientList),
      mMacRandomizationSetting(other.mMacRandomizationSetting),
      mBridgedModeOpportunisticShutdownEnabled(
              other.mBridgedModeOpportunisticShutdownEnabled),
      mIeee80211axEnabled(other.mIeee80211axEnabled),
      mIeee80211beEnabled(other.mIeee80211beEnabled),
      mIsUserConfiguration(other.mIsUserConfiguration),
      mBridgedModeOpportunisticShutdownTimeoutMillis(
              other.mBridgedModeOpportunisticShutdownTimeoutMillis),
      mVendorElements(other.mVendorElements),
      mPersistentRandomizedMacAddress(other.mPersistentRandomizedMacAddress),
      mAllowedAcsChannels2g(other.mAllowedAcsChannels2g),
      mAllowedAcsChannels5g(other.mAllowedAcsChannels5g),
      mAllowedAcsChannels6g(other.mAllowedAcsChannels6g),
      mMaxChannelBandwidth(other.mMaxChannelBandwidth),
      mIsClientIsolationEnabled(other.mIsClientIsolationEnabled) {
    if (mBssid.getBytes().size() == 6) {
        /* Auto-set for legacy configurations (S+ behavior in AOSP). */
        mMacRandomizationSetting = RANDOMIZATION_NONE;
    }
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setSsid(
        const std::string& ssid) {
    if (ssid.empty()) {
        /* Java: null -> framework-chosen SSID (the empty WifiSsid). */
        mWifiSsid = WifiSsid();
        return *this;
    }
    /* Preconditions.checkStringNotEmpty; the UTF-8 encodability check is a
     * no-op for byte strings (std::string carries bytes verbatim). */
    mWifiSsid = WifiSsid::fromUtf8Text(ssid);
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setWifiSsid(
        const WifiSsid& wifiSsid) {
    mWifiSsid = wifiSsid;
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setBssid(
        const MacAddress& bssid) {
    if (bssid.getBytes().size() == 6) {
        if (bssid == MacAddress::ALL_ZEROS_MAC_ADDRESS)
            throw std::invalid_argument("bssid is the all-zeros address");
        if (bssid.getAddressType() != MacAddress::TYPE_UNICAST)
            throw std::invalid_argument(
                    "bssid doesn't support multicast or broadcast mac address");
    }
    mBssid = bssid;
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setPassphrase(
        const std::string& passphrase, int securityType) {
    if (securityType == SECURITY_TYPE_OPEN
            || securityType == SECURITY_TYPE_WPA3_OWE_TRANSITION
            || securityType == SECURITY_TYPE_WPA3_OWE) {
        if (!passphrase.empty())
            throw std::invalid_argument(
                    "passphrase should be null when security type is open");
    } else {
        if (passphrase.empty())
            throw std::invalid_argument("passphrase must not be empty");
        /* The 8..63 byte check only applied before TIRAMISU (SdkLevel gate
         * in AOSP) — resolved away at android-36; PSK_MIN/MAX stay test
         * surface. */
    }
    mSecurityType = securityType;
    mPassphrase = passphrase;
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setHiddenSsid(
        bool hiddenSsid) {
    mHiddenSsid = hiddenSsid;
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setBand(int band) {
    if (!isBandValid(band))
        throw std::invalid_argument("Invalid band type: " + std::to_string(band));
    mChannels.clear();
    mChannels[band] = 0;
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setBands(
        const std::vector<int>& bands) {
    if (bands.empty() || bands.size() > 2)
        throw std::invalid_argument("Unsupported number of bands("
                + std::to_string(bands.size()) + ") configured");
    mChannels.clear();
    for (const int val : bands) {
        if (!isBandValid(val))
            throw std::invalid_argument("Invalid band type: " + std::to_string(val));
        mChannels[val] = 0;
    }
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setChannel(
        int channel, int band) {
    if (!isChannelBandPairValid(channel, band))
        throw std::invalid_argument("Invalid channel(" + std::to_string(channel)
                + ") & band (" + std::to_string(band) + ") configured");
    mChannels.clear();
    mChannels[band] = channel;
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setChannels(
        const std::map<int, int>& channels) {
    if (channels.empty() || channels.size() > 2)
        throw std::invalid_argument("Unsupported number of channels("
                + std::to_string(channels.size()) + ") configured");
    for (const auto& entry : channels) {
        if (entry.second == 0) {
            if (!isBandValid(entry.first))
                throw std::invalid_argument(
                        "Invalid band type: " + std::to_string(entry.first));
        } else if (!isChannelBandPairValid(entry.second, entry.first)) {
            throw std::invalid_argument("Invalid channel("
                    + std::to_string(entry.second) + ") & band ("
                    + std::to_string(entry.first) + ") configured");
        }
    }
    mChannels = channels;
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setMaxNumberOfClients(
        int maxNumberOfClients) {
    if (maxNumberOfClients < 0)
        throw std::invalid_argument("maxNumberOfClients should be not negative");
    mMaxNumberOfClients = maxNumberOfClients;
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setAutoShutdownEnabled(
        bool enable) {
    mAutoShutdownEnabled = enable;
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setShutdownTimeoutMillis(
        int64_t timeoutMillis) {
    /* REMOVE_ZERO_FOR_TIMEOUT_SETTING is resolved as enabled (android-36):
     * anything below 1 is invalid except DEFAULT_TIMEOUT. */
    if (timeoutMillis < 1 && timeoutMillis != DEFAULT_TIMEOUT)
        throw std::invalid_argument("Invalid timeout value: "
                + std::to_string(timeoutMillis));
    mShutdownTimeoutMillis = timeoutMillis;
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setClientControlByUserEnabled(
        bool enabled) {
    mClientControlByUser = enabled;
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setAllowedClientList(
        const std::vector<MacAddress>& allowedClientList) {
    mAllowedClientList = allowedClientList;
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setBlockedClientList(
        const std::vector<MacAddress>& blockedClientList) {
    mBlockedClientList = blockedClientList;
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setMacRandomizationSetting(
        int macRandomizationSetting) {
    mMacRandomizationSetting = macRandomizationSetting;
    return *this;
}

SoftApConfiguration::Builder&
SoftApConfiguration::Builder::setBridgedModeOpportunisticShutdownEnabled(
        bool enable) {
    mBridgedModeOpportunisticShutdownEnabled = enable;
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setIeee80211axEnabled(
        bool enable) {
    mIeee80211axEnabled = enable;
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setIeee80211beEnabled(
        bool enable) {
    mIeee80211beEnabled = enable;
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setUserConfiguration(
        bool isUserConfigured) {
    mIsUserConfiguration = isUserConfigured;
    return *this;
}

SoftApConfiguration::Builder&
SoftApConfiguration::Builder::setBridgedModeOpportunisticShutdownTimeoutMillis(
        int64_t timeoutMillis) {
    if (timeoutMillis < 1 && timeoutMillis != DEFAULT_TIMEOUT)
        throw std::invalid_argument("Invalid timeout value: "
                + std::to_string(timeoutMillis));
    mBridgedModeOpportunisticShutdownTimeoutMillis = timeoutMillis;
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setVendorElements(
        const std::vector<ScanResult::InformationElement>& vendorElements) {
    for (const ScanResult::InformationElement& e : vendorElements) {
        if (e.id != ScanResult::InformationElement::EID_VSA)
            throw std::invalid_argument(
                    "received InformationElement which is not related to "
                    "VendorElements. VendorElement block should start with dd");
    }
    /* HashSet-based duplicate detection (AOSP compares equals/hashCode). */
    for (size_t i = 0; i < vendorElements.size(); ++i) {
        for (size_t j = i + 1; j < vendorElements.size(); ++j) {
            if (vendorElements[i].id == vendorElements[j].id
                    && vendorElements[i].bytes == vendorElements[j].bytes)
                throw std::invalid_argument(
                        "vendor elements array contain duplicates. Please "
                        "avoid passing duplicated and keep structure clean.");
        }
    }
    mVendorElements = vendorElements;
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setAllowedAcsChannels(
        int band, const std::vector<int>& channels) {
    if (band != BAND_2GHZ && band != BAND_5GHZ && band != BAND_6GHZ)
        throw std::invalid_argument("Passing an invalid band to setAllowedAcsChannels");
    for (const int channel : channels) {
        if (!isChannelBandPairValid(channel, band))
            throw std::invalid_argument(
                    "Invalid channel to setAllowedAcsChannels: band: "
                    + std::to_string(band) + "channel: " + std::to_string(channel));
    }
    std::set<int> set(channels.begin(), channels.end());
    switch (band) {
        case BAND_2GHZ: mAllowedAcsChannels2g = set; break;
        case BAND_5GHZ: mAllowedAcsChannels5g = set; break;
        case BAND_6GHZ: mAllowedAcsChannels6g = set; break;
    }
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setMaxChannelBandwidth(
        int maxChannelBandwidth) {
    switch (maxChannelBandwidth) {
        case SoftApInfo::CHANNEL_WIDTH_AUTO:
        case SoftApInfo::CHANNEL_WIDTH_20MHZ:
        case SoftApInfo::CHANNEL_WIDTH_40MHZ:
        case SoftApInfo::CHANNEL_WIDTH_80MHZ:
        case SoftApInfo::CHANNEL_WIDTH_160MHZ:
        case SoftApInfo::CHANNEL_WIDTH_320MHZ:
            mMaxChannelBandwidth = maxChannelBandwidth;
            break;
        default:
            throw std::invalid_argument("Invalid channel bandwidth value("
                    + std::to_string(maxChannelBandwidth) + ")  configured");
    }
    return *this;
}

SoftApConfiguration::Builder& SoftApConfiguration::Builder::setClientIsolationEnabled(
        bool isClientIsolationEnabled) {
    mIsClientIsolationEnabled = isClientIsolationEnabled;
    return *this;
}

SoftApConfiguration SoftApConfiguration::Builder::buildWithoutCheck() {
    return SoftApConfiguration(mWifiSsid, mBssid, mPassphrase, mHiddenSsid,
            mChannels, mSecurityType, mMaxNumberOfClients, mAutoShutdownEnabled,
            mShutdownTimeoutMillis, mClientControlByUser, mBlockedClientList,
            mAllowedClientList, mMacRandomizationSetting,
            mBridgedModeOpportunisticShutdownEnabled, mIeee80211axEnabled,
            mIeee80211beEnabled, mIsUserConfiguration,
            mBridgedModeOpportunisticShutdownTimeoutMillis, mVendorElements,
            mPersistentRandomizedMacAddress, mAllowedAcsChannels2g,
            mAllowedAcsChannels5g, mAllowedAcsChannels6g, mMaxChannelBandwidth,
            mIsClientIsolationEnabled);
}

SoftApConfiguration SoftApConfiguration::Builder::build() {
    for (const MacAddress& client : mAllowedClientList) {
        if (std::find(mBlockedClientList.begin(), mBlockedClientList.end(), client)
                != mBlockedClientList.end())
            throw std::invalid_argument("A MacAddress exist in both client list");
    }
    /* FORCE_MUTUAL_EXCLUSIVE_BSSID_MAC_RAMDONIZATION_SETTING resolved as
     * enabled (android-36). */
    if (mBssid.getBytes().size() == 6
            && mMacRandomizationSetting != RANDOMIZATION_NONE)
        throw std::invalid_argument("A BSSID had configured but MAC randomization"
                " setting is not NONE");
    /* BAKLAVA: 11be depends on 11ax. */
    if (!mIeee80211axEnabled) mIeee80211beEnabled = false;
    return buildWithoutCheck();
}

} // namespace cdroid
