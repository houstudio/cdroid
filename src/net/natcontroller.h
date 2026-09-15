#ifndef __NAT_CONTROLLER_H__
#define __NAT_CONTROLLER_H__

#include <string>
#include <vector>

namespace cdroid {

/**
 * IPv4 NAT/forwarding for tethering — the port of netd's NatController
 * (frameworks/native/netd/server/controller/NatController.cpp) with classic
 * iptables rules instead of AOSP's BPF tethering offload (the embedded
 * backend every pre-BPF product shipped):
 *
 *   enableNat(internal, external):
 *     net.ipv4.ip_forward = 1
 *     iptables -t nat -A POSTROUTING -o <external> -j MASQUERADE
 *     iptables -A FORWARD -i <external> -o <internal> -m state \
 *              --state RELATED,ESTABLISHED -j ACCEPT
 *     iptables -A FORWARD -i <internal> -o <external> -j ACCEPT
 *   disableNat(): the same rules deleted (-D); ip_forward drops when the
 *   last NAT goes away (netd tracks refcounts — the module runs one).
 *
 * Needs CAP_NET_ADMIN (the tethering owner process runs as root on the
 * target). Rule strings are built by forwardingRules()/masqueradeRule() so
 * the pure-logic layer stays testable without iptables.
 */
class NatController {
public:
    /* Tethering side = internal (AP interface), internet side = external
     * (default-route interface). Returns false when any rule failed. */
    static bool enableNat(const std::string& internalIface,
                          const std::string& externalIface);
    static bool disableNat(const std::string& internalIface,
                           const std::string& externalIface);

    /* Interface that owns the default IPv4 route ("/proc/net/route" parse,
     * lowest metric wins); empty when there is none. The AOSP tethering
     * upstream = the default network's interface. */
    static std::string defaultRouteInterface();

    /* --- rule builders (pure logic, unit-testable) --- */
    static std::vector<std::string> forwardingRules(
            const std::string& internalIface, const std::string& externalIface);
    static std::string masqueradeRule(const std::string& externalIface);

private:
    static bool runIptables(const std::vector<std::string>& args);
    static bool setIpForward(bool enable);
};

} // namespace cdroid

#endif /* __NAT_CONTROLLER_H__ */
