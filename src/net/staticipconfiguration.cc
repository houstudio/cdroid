/* Port of android.net.StaticIpConfiguration (android-36). */
#include <staticipconfiguration.h>

namespace cdroid {

StaticIpConfiguration& StaticIpConfiguration::setIpAddress(const LinkAddress& ipAddress) {
    mIpAddress = ipAddress;
    return *this;
}

StaticIpConfiguration& StaticIpConfiguration::setGateway(const std::string& gateway) {
    mGateway = gateway;
    return *this;
}

StaticIpConfiguration& StaticIpConfiguration::setDnsServers(
        const std::vector<std::string>& dnsServers) {
    mDnsServers = dnsServers;
    return *this;
}

StaticIpConfiguration& StaticIpConfiguration::setDomains(const std::string& domains) {
    mDomains = domains;
    return *this;
}

void StaticIpConfiguration::clear() {
    mIpAddress = LinkAddress();
    mGateway.clear();
    mDnsServers.clear();
    mDomains.clear();
}

const LinkAddress& StaticIpConfiguration::getIpAddress() const {
    return mIpAddress;
}

std::string StaticIpConfiguration::getGateway() const {
    return mGateway;
}

const std::vector<std::string>& StaticIpConfiguration::getDnsServers() const {
    return mDnsServers;
}

std::string StaticIpConfiguration::getDomains() const {
    return mDomains;
}

void StaticIpConfiguration::addDnsServer(const std::string& server) {
    mDnsServers.push_back(server);
}

bool StaticIpConfiguration::operator==(const StaticIpConfiguration& other) const {
    return mIpAddress.toString() == other.mIpAddress.toString()
            && mGateway == other.mGateway
            && mDnsServers == other.mDnsServers
            && mDomains == other.mDomains;
}

std::string StaticIpConfiguration::toString() const {
    const std::string none = "<none>";
    std::string str = "IP address " + (mIpAddress.toString().empty() ? "/" : mIpAddress.toString());
    str += " Gateway " + (mGateway.empty() ? none : mGateway);
    str += " DNS servers: [";
    for (size_t i = 0; i < mDnsServers.size(); i++) {
        if (i) str += ",";
        str += " " + mDnsServers[i];
    }
    str += " ] Domains " + (mDomains.empty() ? none : mDomains);
    return str;
}

} // namespace cdroid
