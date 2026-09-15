#ifndef __MAC_ADDRESS_H__
#define __MAC_ADDRESS_H__

#include <cstdint>
#include <string>

namespace cdroid {

/**
 * Port of android.net.MacAddress (android-36): representation of a MAC
 * address. Java byte[6] is carried as std::string (binary-safe, exactly 6
 * bytes when valid).
 *
 * Java methods that return null on malformed input (fromString, fromBytes)
 * return a default-constructed MacAddress with empty bytes instead — the
 * no-optional sentinel convention (compare against MacAddress() or test
 * toString() for "").
 */
class MacAddress {
public:
    /* Mac address types: classification of the I/G + all-bits-set pattern
     * (MacAddress#getAddressType). TYPE_ANY only ever appears in address
     * *masks* passed to matchesWithMask (TODO(porting)). */
    static constexpr int TYPE_UNICAST   = 0;
    static constexpr int TYPE_MULTICAST = 1;
    static constexpr int TYPE_BROADCAST = 2;
    static constexpr int TYPE_ANY       = 3;

    static const MacAddress ALL_ZEROS_MAC_ADDRESS;
    static const MacAddress BROADCAST_ADDRESS;

    /* Creates a MacAddress from raw bytes; a wrong-length input yields the
     * empty (invalid) MacAddress. */
    explicit MacAddress(const std::string& bytes);
    MacAddress() = default;

    /**
     * Parses a MacAddress from a String. Accepted formats:
     *   00:11:22:33:44:55 / 00-11-22-33-44-55 / 001122334455
     * Java returns null on a malformed input — the empty MacAddress is that
     * null sentinel here.
     */
    static MacAddress fromString(const std::string& address);
    /* @see MacAddress#MacAddress(byte[]) */
    static MacAddress fromBytes(const std::string& bytes);

    /* Raw 6 bytes; empty when invalid. */
    const std::string& getBytes() const { return mAddr; }
    /* Lowercase "aa:bb:cc:dd:ee:ff"; "" when invalid. */
    std::string toString() const;
    /* First three bytes as "aabbcc"; "" when invalid. */
    std::string toOuiString() const;

    int getAddressType() const;
    /* Local administration bit (bit 1 of the first byte). */
    bool isLocallyAssigned() const;

    bool operator==(const MacAddress& other) const;
    bool operator!=(const MacAddress& other) const;

private:
    std::string mAddr;   /* exactly 6 bytes, or empty when invalid */
};

} // namespace cdroid

#endif /* __MAC_ADDRESS_H__ */
