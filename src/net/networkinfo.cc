/* Port of android.net.NetworkInfo (android-36). */
#include <networkinfo.h>

namespace cdroid {

NetworkInfo::NetworkInfo(int type) : mNetworkType(type) {
    /* AOSP constructor: start at IDLE. */
    setDetailedState(DetailedState::IDLE, std::string(), std::string());
}

int NetworkInfo::getType() const {
    return mNetworkType;
}

void NetworkInfo::setType(int type) {
    mNetworkType = type;
}

void NetworkInfo::setSubtype(int subtype, const std::string& subtypeName) {
    mSubtype = subtype;
    mSubtypeName = subtypeName;
}

int NetworkInfo::getSubtype() const {
    return mSubtype;
}

std::string NetworkInfo::getSubtypeName() const {
    return mSubtypeName;
}

std::string NetworkInfo::getTypeName() const {
    /* ConnectivityManager.getNetworkTypeName equivalent for the types this
     * module tracks. */
    switch (mNetworkType) {
    case 0:  return "MOBILE";
    case 1:  return "WIFI";
    case 7:  return "BLUETOOTH";
    case 9:  return "ETHERNET";
    case 17: return "VPN";
    default: return "UNKNOWN";
    }
}

NetworkInfo::State::Type NetworkInfo::stateFromDetailedState(
        DetailedState::Type detailedState) {
    switch (detailedState) {
    case DetailedState::IDLE:                return State::DISCONNECTED;
    case DetailedState::SCANNING:            return State::DISCONNECTED;
    case DetailedState::CONNECTING:          return State::CONNECTING;
    case DetailedState::AUTHENTICATING:      return State::CONNECTING;
    case DetailedState::OBTAINING_IPADDR:    return State::CONNECTING;
    case DetailedState::VERIFYING_POOR_LINK: return State::CONNECTING;
    case DetailedState::CONNECTED:           return State::CONNECTED;
    case DetailedState::SUSPENDED:           return State::SUSPENDED;
    case DetailedState::DISCONNECTING:       return State::DISCONNECTED;
    case DetailedState::DISCONNECTED:        return State::DISCONNECTED;
    case DetailedState::FAILED:              return State::DISCONNECTED;
    case DetailedState::BLOCKED:             return State::DISCONNECTED;
    }
    return State::UNKNOWN;
}

void NetworkInfo::setDetailedState(DetailedState::Type detailedState, const std::string& reason,
                                   const std::string& extraInfo) {
    mDetailedState = detailedState;
    mState = stateFromDetailedState(detailedState);
    mReason = reason;
    mExtraInfo = extraInfo;
}

NetworkInfo::DetailedState::Type NetworkInfo::getDetailedState() const {
    return mDetailedState;
}

NetworkInfo::State::Type NetworkInfo::getState() const {
    return mState;
}

std::string NetworkInfo::getReason() const {
    return mReason;
}

void NetworkInfo::setExtraInfo(const std::string& extraInfo) {
    mExtraInfo = extraInfo;
}

std::string NetworkInfo::getExtraInfo() const {
    return mExtraInfo;
}

bool NetworkInfo::isConnected() const {
    return mState == State::CONNECTED && mIsAvailable;
}

bool NetworkInfo::isConnectedOrConnecting() const {
    return (mState == State::CONNECTED || mState == State::CONNECTING) && mIsAvailable;
}

bool NetworkInfo::isFailover() const {
    return mIsFailover;
}

void NetworkInfo::setFailover(bool isFailover) {
    mIsFailover = isFailover;
}

bool NetworkInfo::isAvailable() const {
    return mIsAvailable;
}

void NetworkInfo::setIsAvailable(bool isAvailable) {
    mIsAvailable = isAvailable;
}

bool NetworkInfo::isRoaming() const {
    return mIsRoaming;
}

void NetworkInfo::setRoaming(bool isRoaming) {
    mIsRoaming = isRoaming;
}

std::string NetworkInfo::toString() const {
    const char* stateNames[] = {"CONNECTING", "CONNECTED", "SUSPENDED",
            "DISCONNECTING", "DISCONNECTED", "UNKNOWN"};
    const char* detailedNames[] = {"IDLE", "SCANNING", "CONNECTING", "AUTHENTICATING",
            "OBTAINING_IPADDR", "CONNECTED", "SUSPENDED", "DISCONNECTING", "DISCONNECTED",
            "FAILED", "BLOCKED", "VERIFYING_POOR_LINK"};
    std::string result = "type: " + getTypeName() + "[" + mSubtypeName + "]";
    result += ", state: " + std::string(stateNames[(int) mState]) + "/"
            + std::string(detailedNames[(int) mDetailedState]);
    result += ", reason: " + (mReason.empty() ? std::string("(unspecified)") : mReason);
    result += ", extra: " + (mExtraInfo.empty() ? std::string("(none)") : mExtraInfo);
    result += ", failover: " + std::string(mIsFailover ? "true" : "false");
    result += ", available: " + std::string(mIsAvailable ? "true" : "false");
    result += ", roaming: " + std::string(mIsRoaming ? "true" : "false");
    return result;
}

} // namespace cdroid
