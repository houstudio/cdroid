#ifndef __WIFI_CLIENT_H__
#define __WIFI_CLIENT_H__

#include <string>

#include <macaddress.h>

namespace cdroid {

/**
 * Port of android.net.wifi.WifiClient (android-36): a Soft AP client
 * identified by its MAC address and the AP interface it connected through.
 * Bridged APs report one WifiClient per interface; the single-interface
 * backend fills the interface name of the running AP.
 */
class WifiClient {
public:
    WifiClient() = default;
    WifiClient(const MacAddress& macAddress, const std::string& interfaceName)
        : mMacAddress(macAddress), mInterfaceName(interfaceName) {}

    const MacAddress& getMacAddress() const { return mMacAddress; }
    const std::string& getInterfaceName() const { return mInterfaceName; }

    bool operator==(const WifiClient& other) const {
        return mMacAddress == other.mMacAddress
                && mInterfaceName == other.mInterfaceName;
    }
    bool operator!=(const WifiClient& other) const { return !(*this == other); }

    std::string toString() const {
        return "WifiClient{mMacAddress = " + mMacAddress.toString()
                + ", mInterfaceName = " + mInterfaceName + "}";
    }

private:
    MacAddress mMacAddress;
    std::string mInterfaceName;
};

} // namespace cdroid

#endif /* __WIFI_CLIENT_H__ */
