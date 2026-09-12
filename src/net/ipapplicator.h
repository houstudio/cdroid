#ifndef __IP_APPLICATOR_H__
#define __IP_APPLICATOR_H__

#include <atomic>
#include <functional>
#include <string>

#include <dhcpclient.h>
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
 * Bring an interface up (or down): SIOCGIFFLAGS -> set/clear IFF_UP ->
 * SIOCSIFFLAGS. The single bring-up sequence shared by the apply path and
 * EthernetManager::enable/disableInterface (they used to disagree on
 * failure policy). Returns the ioctl outcome.
 */
bool bringInterfaceUp(const std::string& iface, bool up);

/**
 * Read-side sibling of the apply path: true when the interface carries an
 * IPv4 address; the dotted-quad form is returned through *dotted when
 * non-null. One definition of "has an address" (getifaddrs based) — the
 * pattern sweeps in EthernetManager/ConnectivityManager stay fused (flags
 * and addresses fall out of the same pass), single-interface probes go
 * through here.
 */
bool interfaceHasIpv4Address(const std::string& iface, std::string* dotted = nullptr);

/**
 * The teardown counterpart: drop every IPv4 address on the interface
 * (SIOCDIFADDR), delete the default route (RTM_DELROUTE) and remove the
 * resolv.conf nameserver lines this library previously wrote for it.
 * Switching STATIC -> DHCP (and disconnect teardowns) route through here;
 * an absent route counts as success.
 */
bool clearIpConfiguration(const std::string& iface);

/**
 * AOSP IpClient's lease-to-configuration conversion — the one definition
 * shared by both managers' initial apply and renewal re-apply (three hand
 * copies used to drift independently).
 */
StaticIpConfiguration toStaticIpConfiguration(const DhcpClient::Lease& lease);

/**
 * Shared T1 renewal loop backing both managers' DHCP sessions: wait T1
 * (or half the lease; infinite leases end it), renew, fall back to a
 * fresh DISCOVER on failure. Every successful lease goes through
 * applyLease; onRenewalFailure (optional) reports a failed cycle. Returns
 * when stop is set.
 */
void runLeaseRenewalLoop(DhcpClient* client, DhcpClient::Lease& lease,
                         const std::function<void(const DhcpClient::Lease&)>& applyLease,
                         const std::atomic<bool>& stop,
                         const std::function<void()>& onRenewalFailure = nullptr);

} // namespace cdroid

#endif /* __IP_APPLICATOR_H__ */
