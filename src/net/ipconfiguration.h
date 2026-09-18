#ifndef __IP_CONFIGURATION_H__
#define __IP_CONFIGURATION_H__

#include <string>

#include <staticipconfiguration.h>

namespace cdroid {

/**
 * Port of android.net.IpConfiguration (android-36): how an interface gets
 * its IP settings (static / DHCP) plus its proxy choice. ProxyInfo and the
 * PAC machinery are TODO (faithful-stub rule) — ProxySettings keeps the
 * enum values, STATIC carries no data yet.
 */
class IpConfiguration {
public:
    /* Nested like NetworkInfo::State so the STATIC/UNASSIGNED names of the
     * two enums coexist (Java scopes enums per type). */
    struct IpAssignment {
        enum Type {
            STATIC,     /* use statically configured IP settings */
            DHCP,       /* use dynamically configured IP settings */
            UNASSIGNED, /* no IP details assigned */
        };
    };
    struct ProxySettings {
        enum Type {
            NONE,       /* no proxy */
            STATIC,     /* statically configured proxy */
            UNASSIGNED, /* no proxy details assigned */
            PAC,        /* PAC based proxy (TODO: data not ported) */
        };
    };

    IpConfiguration() = default;
    IpConfiguration(IpAssignment::Type ipAssignment, ProxySettings::Type proxySettings,
                    const StaticIpConfiguration& staticIpConfiguration);

    IpAssignment::Type getIpAssignment() const;
    void setIpAssignment(IpAssignment::Type ipAssignment);
    ProxySettings::Type getProxySettings() const;
    void setProxySettings(ProxySettings::Type proxySettings);
    /* Owned; empty object when unset. */
    const StaticIpConfiguration& getStaticIpConfiguration() const;
    void setStaticIpConfiguration(const StaticIpConfiguration& staticIpConfiguration);

    bool operator==(const IpConfiguration& other) const;
    std::string toString() const;

private:
    IpAssignment::Type mIpAssignment = IpAssignment::UNASSIGNED;
    ProxySettings::Type mProxySettings = ProxySettings::UNASSIGNED;
    StaticIpConfiguration mStaticIpConfiguration;
};

} // namespace cdroid

#endif /* __IP_CONFIGURATION_H__ */
