/* Port of android.net.NetworkCapabilities (android-36), reduced form. */
#include <networkcapabilities.h>

namespace cdroid {

static const char* const kTransportNames[] = {
    "CELLULAR", "WIFI", "BLUETOOTH", "ETHERNET", "VPN", "WIFI_AWARE",
    "LOWPAN", "TEST", "USB", "THREAD", "SATELLITE",
};
static const int kTransportCount =
        static_cast<int>(sizeof(kTransportNames) / sizeof(kTransportNames[0]));

void NetworkCapabilities::addTransport(int transportType) {
    mTransports |= (1u << transportType);
}

void NetworkCapabilities::removeTransport(int transportType) {
    mTransports &= ~(1u << transportType);
}

bool NetworkCapabilities::hasTransport(int transportType) const {
    return (mTransports & (1u << transportType)) != 0;
}

void NetworkCapabilities::addCapability(int capability) {
    mCapabilities |= (1u << capability);
}

void NetworkCapabilities::removeCapability(int capability) {
    mCapabilities &= ~(1u << capability);
}

bool NetworkCapabilities::hasCapability(int capability) const {
    return (mCapabilities & (1u << capability)) != 0;
}

bool NetworkCapabilities::operator==(const NetworkCapabilities& other) const {
    return mTransports == other.mTransports && mCapabilities == other.mCapabilities;
}

bool NetworkCapabilities::operator!=(const NetworkCapabilities& other) const {
    return !(*this == other);
}

std::string NetworkCapabilities::toString() const {
    std::string result = "NetworkCapabilities: transports:";
    for (int i = 0; i < kTransportCount; i++) {
        if (hasTransport(i)) result += std::string(" ") + kTransportNames[i];
    }
    result += " capabilities:";
    for (int i = 0; i <= NET_CAPABILITY_CAPTIVE_PORTAL; i++) {
        if (hasCapability(i)) result += " " + std::to_string(i);
    }
    return result;
}

} // namespace cdroid
