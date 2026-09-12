/* Port of android.net.wifi.WifiSsid (android-36). */
#include <wifi/wifissid.h>

#include <cstdint>
#include <stdexcept>

#include <hexencoding.h>

namespace cdroid {

WifiSsid::WifiSsid() = default;

WifiSsid::WifiSsid(const std::string& bytes) : mBytes(bytes) {
}

WifiSsid WifiSsid::fromBytes(const std::string& bytes) {
    return WifiSsid(bytes);
}

WifiSsid WifiSsid::fromUtf8Text(const std::string& utf8Text) {
    return WifiSsid(utf8Text);
}

WifiSsid WifiSsid::fromString(const std::string& string) {
    const size_t length = string.size();
    if (length > 1 && string[0] == '"' && string[length - 1] == '"')
        return WifiSsid(string.substr(1, length - 2));
    return WifiSsid(HexEncoding::decode(string));
}

std::string WifiSsid::getBytes() const {
    return mBytes;
}

std::string WifiSsid::getUtf8Text() const {
    return decodeSsid(mBytes);
}

std::string WifiSsid::toString() const {
    const std::string utf8 = decodeSsid(mBytes);
    if (utf8.empty())
        return HexEncoding::encode(mBytes);
    return "\"" + utf8 + "\"";
}

bool WifiSsid::operator==(const WifiSsid& other) const {
    return mBytes == other.mBytes;
}

bool WifiSsid::operator!=(const WifiSsid& other) const {
    return !(*this == other);
}

/* Strict UTF-8 decode mirroring the Java CharsetDecoder configured with
 * CodingErrorAction.REPORT; returns "" on any malformed sequence. The Java
 * original decodes into a 32-char buffer and treats overflow as success, so
 * output is capped at 32 chars. */
std::string WifiSsid::decodeSsid(const std::string& bytes) {
    std::string out;
    size_t i = 0;
    size_t chars = 0;   /* the CharBuffer counts chars, not bytes */
    while (i < bytes.size()) {
        const unsigned char b0 = static_cast<unsigned char>(bytes[i]);
        if (b0 < 0x80) {
            out.push_back(static_cast<char>(b0));
            i += 1;
        } else if ((b0 & 0xE0) == 0xC0) {
            if (i + 1 >= bytes.size()) return std::string();
            const unsigned char b1 = static_cast<unsigned char>(bytes[i + 1]);
            if ((b1 & 0xC0) != 0x80) return std::string();
            out.push_back(static_cast<char>(b0));
            out.push_back(static_cast<char>(b1));
            i += 2;
        } else if ((b0 & 0xF0) == 0xE0) {
            if (i + 2 >= bytes.size()) return std::string();
            const unsigned char b1 = static_cast<unsigned char>(bytes[i + 1]);
            const unsigned char b2 = static_cast<unsigned char>(bytes[i + 2]);
            if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80) return std::string();
            out.push_back(static_cast<char>(b0));
            out.push_back(static_cast<char>(b1));
            out.push_back(static_cast<char>(b2));
            i += 3;
        } else if ((b0 & 0xF8) == 0xF0) {
            if (i + 3 >= bytes.size()) return std::string();
            for (int k = 1; k <= 3; k++) {
                if ((static_cast<unsigned char>(bytes[i + k]) & 0xC0) != 0x80)
                    return std::string();
            }
            out.append(bytes, i, 4);
            i += 4;
        } else {
            return std::string();
        }
        /* CharBuffer.allocate(32) cap, in decoded characters: multibyte
         * sequences count as one char each (OVERFLOW is success, the extra
         * input is simply left undecoded). */
        if (++chars >= 32) break;
    }
    return out;
}

} // namespace cdroid
