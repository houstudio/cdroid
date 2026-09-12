#ifndef __IP_APPLICATOR_H__
#define __IP_APPLICATOR_H__

#include <string>

#include <staticipconfiguration.h>

namespace cdroid {

/**
 * Apply an IPv4 configuration to an interface: bring it up
 * (SIOCSIFFLAGS), set address + netmask (SIOCSIFADDR/SIOCSIFNETMASK),
 * install the default route (RTM_NEWROUTE) and DNS (/etc/resolv.conf
 * when writable). Shared by the ethernet static path and the wifi DHCP
 * lease path — the apply half of AOSP's per-interface IpClient.
 *
 * Needs CAP_NET_ADMIN (in practice: the app process runs as root on the
 * target). Returns false when the interface does not exist or any
 * address/netmask/route step failed.
 */
bool applyIpConfiguration(const std::string& iface, const StaticIpConfiguration& config);

/**
 * The teardown counterpart: drop every IPv4 address on the interface
 * (SIOCDIFADDR), delete the default route (RTM_DELROUTE) and remove the
 * resolv.conf nameserver lines this library previously wrote for it.
 * Switching STATIC -> DHCP (and disconnect teardowns) route through here;
 * an absent route counts as success.
 */
bool clearIpConfiguration(const std::string& iface);

} // namespace cdroid

#endif /* __IP_APPLICATOR_H__ */
