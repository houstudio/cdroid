/* Port of android.net.DhcpInfo (android-36). */
#include <dhcpinfo.h>

#include <cstdlib>

namespace cdroid {

int DhcpInfo::stringToInt(const std::string& addrString) {
    int parts[4];
    const char* p = addrString.c_str();
    char* end = nullptr;
    for (int i = 0; i < 4; i++) {
        parts[i] = (int) strtol(p, &end, 10);
        if (end == p || parts[i] < 0 || parts[i] > 255) return 0;
        p = end + 1;
    }
    return parts[0] | (parts[1] << 8) | (parts[2] << 16) | (parts[3] << 24);
}

std::string DhcpInfo::intToStr(int addr) {
    return std::to_string(addr & 0xff) + "." + std::to_string((addr >> 8) & 0xff)
            + "." + std::to_string((addr >> 16) & 0xff) + "." + std::to_string((addr >> 24) & 0xff);
}

std::string DhcpInfo::toString() const {
    return "ipaddr " + intToStr(ipAddress)
            + " gateway " + intToStr(gateway)
            + " netmask " + intToStr(netmask)
            + " dns1 " + intToStr(dns1)
            + " dns2 " + intToStr(dns2)
            + " DHCP server " + intToStr(serverAddress)
            + " lease " + std::to_string(leaseDuration) + " seconds";
}

} // namespace cdroid
