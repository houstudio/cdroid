/* Port of android.net.wifi.WifiInfo (android-36). */
#include <wifi/wifiinfo.h>

#include <cstdint>
#include <cstdlib>

#include <wifi/wifimanager.h>

namespace cdroid {

WifiInfo::WifiInfo() {
}

NetworkInfo::DetailedState::Type WifiInfo::getDetailedStateOf(
        SupplicantState::State suppState) {
    switch (suppState) {
    case SupplicantState::DISCONNECTED:       return NetworkInfo::DetailedState::DISCONNECTED;
    case SupplicantState::INTERFACE_DISABLED: return NetworkInfo::DetailedState::DISCONNECTED;
    case SupplicantState::INACTIVE:           return NetworkInfo::DetailedState::IDLE;
    case SupplicantState::SCANNING:           return NetworkInfo::DetailedState::SCANNING;
    case SupplicantState::AUTHENTICATING:     return NetworkInfo::DetailedState::CONNECTING;
    case SupplicantState::ASSOCIATING:        return NetworkInfo::DetailedState::CONNECTING;
    case SupplicantState::ASSOCIATED:         return NetworkInfo::DetailedState::CONNECTING;
    case SupplicantState::FOUR_WAY_HANDSHAKE: return NetworkInfo::DetailedState::AUTHENTICATING;
    case SupplicantState::GROUP_HANDSHAKE:    return NetworkInfo::DetailedState::AUTHENTICATING;
    case SupplicantState::COMPLETED:          return NetworkInfo::DetailedState::OBTAINING_IPADDR;
    case SupplicantState::DORMANT:            return NetworkInfo::DetailedState::DISCONNECTED;
    case SupplicantState::UNINITIALIZED:      return NetworkInfo::DetailedState::IDLE;
    case SupplicantState::INVALID:            return NetworkInfo::DetailedState::FAILED;
    }
    return NetworkInfo::DetailedState::IDLE;
}

void WifiInfo::setSSID(WifiSsid wifiSsid) {
    mWifiSsid = std::move(wifiSsid);
}

WifiSsid WifiInfo::getWifiSsid() const {
    return mWifiSsid;
}

std::string WifiInfo::getSSID() const {
    if (!mWifiSsid.getBytes().empty()) {
        const std::string ssidString = mWifiSsid.toString();
        if (!ssidString.empty())
            return ssidString;
    }
    return WifiManager::UNKNOWN_SSID;
}

void WifiInfo::setBSSID(const std::string& BSSID) {
    mBSSID = BSSID;
}

std::string WifiInfo::getBSSID() const {
    return mBSSID;
}

int WifiInfo::getRssi() const {
    return mRssi;
}

void WifiInfo::setRssi(int rssi) {
    mRssi = rssi;
}

int WifiInfo::getLinkSpeed() const {
    return mLinkSpeed;
}

void WifiInfo::setLinkSpeed(int linkSpeed) {
    mLinkSpeed = linkSpeed;
}

int WifiInfo::getTxLinkSpeedMbps() const {
    return mTxLinkSpeed;
}

void WifiInfo::setTxLinkSpeedMbps(int txLinkSpeed) {
    mTxLinkSpeed = txLinkSpeed;
}

int WifiInfo::getMaxSupportedTxLinkSpeedMbps() const {
    return mMaxSupportedTxLinkSpeed;
}

void WifiInfo::setMaxSupportedTxLinkSpeedMbps(int maxSupportedTxLinkSpeed) {
    mMaxSupportedTxLinkSpeed = maxSupportedTxLinkSpeed;
}

int WifiInfo::getRxLinkSpeedMbps() const {
    return mRxLinkSpeed;
}

void WifiInfo::setRxLinkSpeedMbps(int rxLinkSpeed) {
    mRxLinkSpeed = rxLinkSpeed;
}

int WifiInfo::getMaxSupportedRxLinkSpeedMbps() const {
    return mMaxSupportedRxLinkSpeed;
}

void WifiInfo::setMaxSupportedRxLinkSpeedMbps(int maxSupportedRxLinkSpeed) {
    mMaxSupportedRxLinkSpeed = maxSupportedRxLinkSpeed;
}

int WifiInfo::getFrequency() const {
    return mFrequency;
}

void WifiInfo::setFrequency(int frequency) {
    mFrequency = frequency;
}

bool WifiInfo::is24GHz() const {
    return mFrequency > 2400 && mFrequency < 2500;
}

bool WifiInfo::is5GHz() const {
    return mFrequency > 4900 && mFrequency < 5900;
}

bool WifiInfo::is6GHz() const {
    return mFrequency > 5925 && mFrequency < 7125;
}

void WifiInfo::setMacAddress(const std::string& macAddress) {
    mMacAddress = macAddress;
}

std::string WifiInfo::getMacAddress() const {
    return mMacAddress;
}

bool WifiInfo::hasRealMacAddress() const {
    return !mMacAddress.empty() && mMacAddress != DEFAULT_MAC_ADDRESS;
}

int WifiInfo::getNetworkId() const {
    return mNetworkId;
}

void WifiInfo::setNetworkId(int networkId) {
    mNetworkId = networkId;
}

SupplicantState::State WifiInfo::getSupplicantState() const {
    return mSupplicantState;
}

void WifiInfo::setSupplicantState(SupplicantState::State state) {
    mSupplicantState = state;
}

void WifiInfo::setInetAddress(const std::string& address) {
    mIpAddress = address;
}

int WifiInfo::getIpAddress() const {
    if (mIpAddress.empty()) return 0;
    /* Inet4AddressUtils.inet4AddressToIntHTL: HTL = Host-To-LITTLE — the
     * int stores a.b.c.d little-endian (a in the LSB), matching the classic
     * "%d.%d.%d.%d", ip&0xff, ip>>8&0xff... display idiom. Same convention
     * as DhcpInfo::stringToInt. */
    unsigned int parts[4];
    const char* p = mIpAddress.c_str();
    char* end = nullptr;
    for (int i = 0; i < 4; i++) {
        parts[i] = strtoul(p, &end, 10);
        if (end == p || parts[i] > 255) return 0;
        p = end + 1;
    }
    return static_cast<int>((parts[3] << 24) | (parts[2] << 16) | (parts[1] << 8) | parts[0]);
}

bool WifiInfo::getHiddenSSID() const {
    return mIsHiddenSsid;
}

void WifiInfo::setHiddenSSID(bool isHiddenSsid) {
    mIsHiddenSsid = isHiddenSsid;
}

void WifiInfo::setMeteredHint(bool meteredHint) {
    mMeteredHint = meteredHint;
}

bool WifiInfo::getMeteredHint() const {
    return mMeteredHint;
}

void WifiInfo::setEphemeral(bool ephemeral) {
    mEphemeral = ephemeral;
}

bool WifiInfo::isEphemeral() const {
    return mEphemeral;
}

void WifiInfo::setTrusted(bool trusted) {
    mTrusted = trusted;
}

bool WifiInfo::isTrusted() const {
    return mTrusted;
}

void WifiInfo::setRestricted(bool restricted) {
    mRestricted = restricted;
}

bool WifiInfo::isRestricted() const {
    return mRestricted;
}

void WifiInfo::setOemPaid(bool oemPaid) {
    mOemPaid = oemPaid;
}

bool WifiInfo::isOemPaid() const {
    return mOemPaid;
}

void WifiInfo::setOemPrivate(bool oemPrivate) {
    mOemPrivate = oemPrivate;
}

bool WifiInfo::isOemPrivate() const {
    return mOemPrivate;
}

void WifiInfo::setCarrierMerged(bool carrierMerged) {
    mCarrierMerged = carrierMerged;
}

bool WifiInfo::isCarrierMerged() const {
    return mCarrierMerged;
}

void WifiInfo::setOsuAp(bool osuAp) {
    mOsuAp = osuAp;
}

bool WifiInfo::isOsuAp() const {
    return mOsuAp;
}

int WifiInfo::getScore() const {
    return score;
}

void WifiInfo::setScore(int score) {
    this->score = score;
}

bool WifiInfo::isUsable() const {
    return mIsUsable;
}

void WifiInfo::setUsable(bool isUsable) {
    mIsUsable = isUsable;
}

std::string WifiInfo::toString() const {
    const std::string none = "<none>";
    std::string sb;
    sb += "SSID: " + getSSID();
    sb += ", BSSID: " + (mBSSID.empty() ? none : mBSSID);
    sb += ", MAC: " + (mMacAddress.empty() ? none : mMacAddress);
    /* InetAddress#toString prints a leading '/' */
    sb += ", IP: " + (mIpAddress.empty() ? none : "/" + mIpAddress);
    sb += ", Security type: " + std::to_string(mSecurityType);
    sb += ", Supplicant state: " + SupplicantState::toString(mSupplicantState);
    sb += ", RSSI: " + std::to_string(mRssi);
    sb += ", Link speed: " + std::to_string(mLinkSpeed) + LINK_SPEED_UNITS;
    sb += ", Tx Link speed: " + std::to_string(mTxLinkSpeed) + LINK_SPEED_UNITS;
    sb += ", Max Supported Tx Link speed: "
            + std::to_string(mMaxSupportedTxLinkSpeed) + LINK_SPEED_UNITS;
    sb += ", Rx Link speed: " + std::to_string(mRxLinkSpeed) + LINK_SPEED_UNITS;
    sb += ", Max Supported Rx Link speed: "
            + std::to_string(mMaxSupportedRxLinkSpeed) + LINK_SPEED_UNITS;
    sb += ", Frequency: " + std::to_string(mFrequency) + FREQUENCY_UNITS;
    sb += ", Net ID: " + std::to_string(mNetworkId);
    sb += ", Metered hint: " + std::string(mMeteredHint ? "true" : "false");
    sb += ", score: " + std::to_string(score);
    sb += ", isUsable: " + std::string(mIsUsable ? "true" : "false");
    sb += ", CarrierMerged: " + std::string(mCarrierMerged ? "true" : "false");
    sb += ", Is Primary: true";
    sb += ", Trusted: " + std::string(mTrusted ? "true" : "false");
    sb += ", Restricted: " + std::string(mRestricted ? "true" : "false");
    sb += ", Ephemeral: " + std::string(mEphemeral ? "true" : "false");
    sb += ", OEM paid: " + std::string(mOemPaid ? "true" : "false");
    sb += ", OEM private: " + std::string(mOemPrivate ? "true" : "false");
    sb += ", OSU AP: " + std::string(mOsuAp ? "true" : "false");
    /* TODO(porting): FQDN/provider/requesting-package, MLO and vendor-data
     * tail follow once those types are ported. */
    return sb;
}

} // namespace cdroid
