#ifndef __HEX_ENCODING_H__
#define __HEX_ENCODING_H__

#include <string>

namespace cdroid {

/**
 * Port of android.util.HexEncoding (the subset cdnet uses). One definition
 * of "what a hex digit is" backs every consumer — WifiSsid quoted/hex
 * parsing, the 64-hex PSK gate and the /proc/net/route columns must agree,
 * or a value accepted by one path is rejected (or misparsed) by another.
 */
class HexEncoding {
public:
    /* Hex digit value, or -1 when the character is not a hex digit. */
    static int hexValue(char c);
    static bool isHex(char c) { return hexValue(c) >= 0; }
    /* True when non-empty and every character is a hex digit. */
    static bool isAllHex(const std::string& s);
    /* Strict even-length decode; throws std::invalid_argument on odd input
     * or a non-hex digit (same contract as WifiSsid::fromString's hex arm). */
    static std::string decode(const std::string& in);
    static std::string encode(const std::string& in);
};

} // namespace cdroid

#endif /* __HEX_ENCODING_H__ */
