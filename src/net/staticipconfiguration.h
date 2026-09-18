#ifndef __STATIC_IP_CONFIGURATION_H__
#define __STATIC_IP_CONFIGURATION_H__

#include <string>
#include <vector>

#include <linkaddress.h>

namespace cdroid {

/**
 * Port of android.net.StaticIpConfiguration (android-36). Addresses are
 * carried as string literals (InetAddress is not ported). RouteInfo /
 * toLinkProperties need LinkProperties and are TODO (faithful-stub rule).
 */
class StaticIpConfiguration {
public:
    StaticIpConfiguration() = default;
    StaticIpConfiguration(const StaticIpConfiguration& source) = default;
    StaticIpConfiguration& operator=(const StaticIpConfiguration&) = default;

    /* AOSP Builder, flattened into mutating setters for C++14 ergonomics:
     * each returns *this so Builder-style chains still read. */
    StaticIpConfiguration& setIpAddress(const LinkAddress& ipAddress);
    StaticIpConfiguration& setGateway(const std::string& gateway);
    StaticIpConfiguration& setDnsServers(const std::vector<std::string>& dnsServers);
    StaticIpConfiguration& setDomains(const std::string& domains);

    void clear();
    const LinkAddress& getIpAddress() const;
    /* Empty string = unset (Java null). */
    std::string getGateway() const;
    const std::vector<std::string>& getDnsServers() const;
    std::string getDomains() const;
    void addDnsServer(const std::string& server);

    bool operator==(const StaticIpConfiguration& other) const;
    std::string toString() const;

private:
    LinkAddress mIpAddress;
    std::string mGateway;
    std::vector<std::string> mDnsServers;
    std::string mDomains;
};

} // namespace cdroid

#endif /* __STATIC_IP_CONFIGURATION_H__ */
