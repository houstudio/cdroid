#ifndef __DHCP_INFO_H__
#define __DHCP_INFO_H__

#include <string>

namespace cdroid {

/**
 * Port of android.net.DhcpInfo (android-36). Every address field is the raw
 * int the legacy DHCP client reported — LITTLE-endian byte order, matching
 * AOSP's intToStr rendering — not WifiInfo's host-order convention.
 */
class DhcpInfo {
public:
    int ipAddress = 0;
    int gateway = 0;
    int netmask = 0;
    int dns1 = 0;
    int dns2 = 0;
    int serverAddress = 0;
    int leaseDuration = 0;

    DhcpInfo() = default;

    /* "192.168.1.5" -> little-endian int (and back), the intToStr form. */
    static int stringToInt(const std::string& addrString);
    static std::string intToStr(int addr);

    std::string toString() const;
};

} // namespace cdroid

#endif /* __DHCP_INFO_H__ */
