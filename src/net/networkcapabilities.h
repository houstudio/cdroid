#ifndef __NETWORK_CAPABILITIES_H__
#define __NETWORK_CAPABILITIES_H__

#include <cstdint>
#include <string>

namespace cdroid {

/**
 * Port of android.net.NetworkCapabilities (android-36), reduced to the
 * transport/capability bitmasks and their queries — the subset a
 * single-process ConnectivityManager can honestly populate. NetworkAgent /
 * signal-strength / UID policy members are TODO (faithful-stub rule).
 */
class NetworkCapabilities {
public:
    static constexpr int TRANSPORT_CELLULAR   = 0;
    static constexpr int TRANSPORT_WIFI       = 1;
    static constexpr int TRANSPORT_BLUETOOTH  = 2;
    static constexpr int TRANSPORT_ETHERNET   = 3;
    static constexpr int TRANSPORT_VPN        = 4;
    static constexpr int TRANSPORT_WIFI_AWARE = 5;
    static constexpr int TRANSPORT_LOWPAN     = 6;
    static constexpr int TRANSPORT_TEST       = 7;
    static constexpr int TRANSPORT_USB        = 8;
    static constexpr int TRANSPORT_THREAD     = 9;
    static constexpr int TRANSPORT_SATELLITE  = 10;

    static constexpr int NET_CAPABILITY_MMS            = 0;
    static constexpr int NET_CAPABILITY_SUPL           = 1;
    static constexpr int NET_CAPABILITY_DUN            = 2;
    static constexpr int NET_CAPABILITY_FOTA           = 3;
    static constexpr int NET_CAPABILITY_IMS            = 4;
    static constexpr int NET_CAPABILITY_CBS            = 5;
    static constexpr int NET_CAPABILITY_WIFI_P2P       = 6;
    static constexpr int NET_CAPABILITY_IA             = 7;
    static constexpr int NET_CAPABILITY_RCS            = 8;
    static constexpr int NET_CAPABILITY_XCAP           = 9;
    static constexpr int NET_CAPABILITY_EIMS           = 10;
    static constexpr int NET_CAPABILITY_NOT_METERED    = 11;
    static constexpr int NET_CAPABILITY_INTERNET       = 12;
    static constexpr int NET_CAPABILITY_NOT_RESTRICTED = 13;
    static constexpr int NET_CAPABILITY_TRUSTED        = 14;
    static constexpr int NET_CAPABILITY_NOT_VPN        = 15;
    static constexpr int NET_CAPABILITY_VALIDATED      = 16;
    static constexpr int NET_CAPABILITY_CAPTIVE_PORTAL = 17;

    NetworkCapabilities() = default;

    void addTransport(int transportType);
    void removeTransport(int transportType);
    bool hasTransport(int transportType) const;
    uint32_t getTransports() const { return mTransports; }

    void addCapability(int capability);
    void removeCapability(int capability);
    bool hasCapability(int capability) const;
    uint32_t getCapabilities() const { return mCapabilities; }

    bool operator==(const NetworkCapabilities& other) const;
    bool operator!=(const NetworkCapabilities& other) const;

    std::string toString() const;

private:
    uint32_t mTransports = 0;   /* TRANSPORT_* bits */
    uint32_t mCapabilities = 0; /* NET_CAPABILITY_* bits */
};

} // namespace cdroid

#endif /* __NETWORK_CAPABILITIES_H__ */
