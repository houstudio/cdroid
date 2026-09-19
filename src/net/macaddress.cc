/* Port of android.net.MacAddress (android-36). */
#include <macaddress.h>

#include <cstdio>
#include <cstring>

namespace cdroid {

/* explicit-length strings: the literals embed NUL bytes */
const MacAddress MacAddress::ALL_ZEROS_MAC_ADDRESS =
        MacAddress(std::string("\x00\x00\x00\x00\x00\x00", 6));
const MacAddress MacAddress::BROADCAST_ADDRESS =
        MacAddress(std::string("\xff\xff\xff\xff\xff\xff", 6));

MacAddress::MacAddress(const std::string& bytes) {
    if (bytes.size() == 6) mAddr = bytes;
    /* wrong length: keep empty (the Java-null sentinel) */
}

static int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

MacAddress MacAddress::fromString(const std::string& address) {
    /* Accepts XX:XX:XX:XX:XX:XX, XX-XX-XX-XX-XX-XX and XXXXXXXXXXXX —
     * everything else (odd hex, wrong separators, wrong length) is the
     * Java-null sentinel. */
    std::string hex;
    char separator = '\0';
    for (const char c : address) {
        if (c == ':' || c == '-') {
            if (separator != '\0' && separator != c) return MacAddress();
            separator = c;
            continue;
        }
        if (hexVal(c) < 0) return MacAddress();
        hex.push_back(c);
    }
    if (hex.size() != 12) return MacAddress();
    std::string bytes;
    for (size_t i = 0; i < 12; i += 2)
        bytes.push_back((char)((hexVal(hex[i]) << 4) | hexVal(hex[i + 1])));
    return MacAddress(bytes);
}

MacAddress MacAddress::fromBytes(const std::string& bytes) {
    return MacAddress(bytes);
}

std::string MacAddress::toString() const {
    if (mAddr.size() != 6) return std::string();
    char buf[18];
    snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
             (unsigned char)mAddr[0], (unsigned char)mAddr[1], (unsigned char)mAddr[2],
             (unsigned char)mAddr[3], (unsigned char)mAddr[4], (unsigned char)mAddr[5]);
    return std::string(buf);
}

std::string MacAddress::toOuiString() const {
    if (mAddr.size() != 6) return std::string();
    char buf[7];
    snprintf(buf, sizeof(buf), "%02x%02x%02x",
             (unsigned char)mAddr[0], (unsigned char)mAddr[1], (unsigned char)mAddr[2]);
    return std::string(buf);
}

int MacAddress::getAddressType() const {
    if (mAddr.size() != 6) return TYPE_ANY;   /* Java would NPE; benign default */
    bool allFF = true;
    for (const char c : mAddr)
        if ((unsigned char)c != 0xff) { allFF = false; break; }
    if (allFF) return TYPE_BROADCAST;
    if (mAddr[0] & 0x01) return TYPE_MULTICAST;
    return TYPE_UNICAST;
}

bool MacAddress::isLocallyAssigned() const {
    return mAddr.size() == 6 && (mAddr[0] & 0x02) != 0;
}

bool MacAddress::operator==(const MacAddress& other) const {
    return mAddr == other.mAddr;
}

bool MacAddress::operator!=(const MacAddress& other) const {
    return !(*this == other);
}

} // namespace cdroid
