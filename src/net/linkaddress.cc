/* Port of android.net.LinkAddress (android-36), reduced form. */
#include <linkaddress.h>

#include <arpa/inet.h>
#include <cstdlib>

namespace cdroid {

LinkAddress::LinkAddress() : mPrefixLength(0) {
}

LinkAddress::LinkAddress(const std::string& address) {
    /* "a.b.c.d/nn" form: split the prefix suffix (AOSP LinkAddress(String)). */
    const size_t slash = address.find('/');
    if (slash != std::string::npos) {
        mAddress = address.substr(0, slash);
        mPrefixLength = atoi(address.substr(slash + 1).c_str());
    } else {
        mAddress = address;
        mPrefixLength = defaultPrefixFor(address);
    }
}

LinkAddress::LinkAddress(const std::string& address, int prefixLength)
    : mAddress(address), mPrefixLength(prefixLength) {
}

std::string LinkAddress::getAddress() const {
    return mAddress;
}

int LinkAddress::getPrefixLength() const {
    return mPrefixLength;
}

bool LinkAddress::isIpv4() const {
    struct in_addr addr;
    return inet_pton(AF_INET, mAddress.c_str(), &addr) == 1;
}

bool LinkAddress::isSameAddressAs(const LinkAddress& other) const {
    return mAddress == other.mAddress;
}

std::string LinkAddress::toString() const {
    return mAddress + "/" + std::to_string(mPrefixLength);
}

std::string LinkAddress::prefixLengthToNetmaskV4(int prefixLength) {
    if (prefixLength < 0 || prefixLength > 32) return std::string();
    const uint32_t mask = (prefixLength == 0)
            ? 0 : htonl(~uint32_t(0) << (32 - prefixLength));
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&mask);
    char buf[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET, bytes, buf, sizeof(buf))) return std::string();
    return buf;
}

int LinkAddress::netmaskToPrefixLengthV4(const std::string& netmask) {
    struct in_addr addr;
    if (inet_pton(AF_INET, netmask.c_str(), &addr) != 1) return -1;
    const uint32_t host = ntohl(addr.s_addr);
    if (~host == 0) return 32;
    /* contiguous <=> the zero bits form a suffix run: ~host is 0...011..1,
     * i.e. (~host + 1) has no bit in common with ~host. */
    if ((~host & (~host + 1)) != 0) return -1;
    return __builtin_popcount(host);
}

int LinkAddress::defaultPrefixFor(const std::string& address) {
    struct in_addr v4;
    if (inet_pton(AF_INET, address.c_str(), &v4) == 1) return 32;
    struct in6_addr v6;
    if (inet_pton(AF_INET6, address.c_str(), &v6) == 1) return 128;
    return 0;
}

} // namespace cdroid
