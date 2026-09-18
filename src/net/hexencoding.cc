/* Port of android.util.HexEncoding (android-36). */
#include <hexencoding.h>

#include <cstdint>
#include <stdexcept>

namespace cdroid {

int HexEncoding::hexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

bool HexEncoding::isAllHex(const std::string& s) {
    if (s.empty()) return false;
    for (const char c : s)
        if (hexValue(c) < 0) return false;
    return true;
}

std::string HexEncoding::decode(const std::string& in) {
    if (in.size() % 2 != 0)
        throw std::invalid_argument("Odd number of characters: " + std::to_string(in.size()));
    std::string out;
    out.reserve(in.size() / 2);
    for (size_t i = 0; i < in.size(); i += 2) {
        const int hi = hexValue(in[i]);
        const int lo = hexValue(in[i + 1]);
        if (hi < 0 || lo < 0)
            throw std::invalid_argument(std::string("Unexpected hex digit: ") + in[i] + in[i + 1]);
        out.push_back(static_cast<char>((hi << 4) | lo));
    }
    return out;
}

std::string HexEncoding::encode(const std::string& in) {
    static const char digits[] = "0123456789abcdef";
    std::string out;
    out.reserve(in.size() * 2);
    for (const unsigned char c : in) {
        out.push_back(digits[c >> 4]);
        out.push_back(digits[c & 0xF]);
    }
    return out;
}

} // namespace cdroid
