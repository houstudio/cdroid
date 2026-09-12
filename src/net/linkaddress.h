#ifndef __LINK_ADDRESS_H__
#define __LINK_ADDRESS_H__

#include <string>

namespace cdroid {

/**
 * Port of android.net.LinkAddress (android-36), reduced to the identity and
 * representation the ethernet static-IP path needs: an IP literal plus a
 * prefix length. Flags/scope and the InterfaceAddress source are TODO
 * (faithful-stub rule) — they carry no data on this path.
 */
class LinkAddress {
public:
    LinkAddress();
    /* "192.168.1.5/24" or a bare literal (then /32 for IPv4, /128 for IPv6,
     * like the AOSP LinkAddress(String) constructor). */
    explicit LinkAddress(const std::string& address);
    LinkAddress(const std::string& address, int prefixLength);

    /* The address literal. */
    std::string getAddress() const;
    int getPrefixLength() const;

    bool isIpv4() const;
    bool isSameAddressAs(const LinkAddress& other) const;
    std::string toString() const;   /* "address/prefix" */

    /* ---- IPv4 helpers for the ioctl application path ---- */
    /* "/24" -> "255.255.255.0"; empty when the prefix is not IPv4-legal. */
    static std::string prefixLengthToNetmaskV4(int prefixLength);
    /* "255.255.0.0" -> 16; -1 when malformed */
    static int netmaskToPrefixLengthV4(const std::string& netmask);

private:
    static int defaultPrefixFor(const std::string& address);

    std::string mAddress;
    int mPrefixLength;
};

} // namespace cdroid

#endif /* __LINK_ADDRESS_H__ */
