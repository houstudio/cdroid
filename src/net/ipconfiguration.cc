/* Port of android.net.IpConfiguration (android-36). */
#include <ipconfiguration.h>

namespace cdroid {

IpConfiguration::IpConfiguration(IpAssignment::Type ipAssignment, ProxySettings::Type proxySettings,
                                 const StaticIpConfiguration& staticIpConfiguration)
    : mIpAssignment(ipAssignment),
      mProxySettings(proxySettings),
      mStaticIpConfiguration(staticIpConfiguration) {
}

IpConfiguration::IpAssignment::Type IpConfiguration::getIpAssignment() const {
    return mIpAssignment;
}

void IpConfiguration::setIpAssignment(IpAssignment::Type ipAssignment) {
    mIpAssignment = ipAssignment;
}

IpConfiguration::ProxySettings::Type IpConfiguration::getProxySettings() const {
    return mProxySettings;
}

void IpConfiguration::setProxySettings(ProxySettings::Type proxySettings) {
    mProxySettings = proxySettings;
}

const StaticIpConfiguration& IpConfiguration::getStaticIpConfiguration() const {
    return mStaticIpConfiguration;
}

void IpConfiguration::setStaticIpConfiguration(
        const StaticIpConfiguration& staticIpConfiguration) {
    mStaticIpConfiguration = staticIpConfiguration;
}

bool IpConfiguration::operator==(const IpConfiguration& other) const {
    return mIpAssignment == other.mIpAssignment
            && mProxySettings == other.mProxySettings
            && mStaticIpConfiguration == other.mStaticIpConfiguration;
}

std::string IpConfiguration::toString() const {
    const char* assignmentNames[] = {"STATIC", "DHCP", "UNASSIGNED"};
    const char* proxyNames[] = {"NONE", "STATIC", "UNASSIGNED", "PAC"};
    std::string str;
    str += mStaticIpConfiguration.toString();
    str += "\nIP assignment: " + std::string(assignmentNames[mIpAssignment]);
    str += "\nProxy settings: " + std::string(proxyNames[mProxySettings]);
    return str;
}

} // namespace cdroid
