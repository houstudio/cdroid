#ifndef __WIFI_SSID_H__
#define __WIFI_SSID_H__

#include <string>

namespace cdroid {

/**
 * Port of android.net.wifi.WifiSsid (android-36): representation of a Wi-Fi
 * Service Set Identifier. Java byte[] is carried as std::string (binary-safe,
 * may contain NULs).
 */
class WifiSsid {
public:
    WifiSsid();

    /* Creates a WifiSsid from raw bytes; a null (empty) input yields the
     * empty WifiSsid. */
    static WifiSsid fromBytes(const std::string& bytes);
    /* @hide: creates a UTF-8 WifiSsid from unquoted plaintext. */
    static WifiSsid fromUtf8Text(const std::string& utf8Text);
    /* Creates a WifiSsid from a string in the toString() format: double-
     * quoted plain text, or unquoted hex digits. Throws
     * std::invalid_argument (Java IllegalArgumentException) when the string
     * is unquoted but not valid hex, or the hex is odd-length. */
    static WifiSsid fromString(const std::string& string);

    /* Raw byte representation. */
    std::string getBytes() const;
    /* Decoded UTF-8 text; empty when the bytes are not valid UTF-8 (Java
     * returns null — empty string is the sentinel, per the no-optional
     * convention). */
    std::string getUtf8Text() const;
    /* Double-quoted plain text when UTF-8 decodable, otherwise unquoted
     * lowercase hex digits. Consistent with WifiInfo#getSSID() and
     * WifiConfiguration#SSID. */
    std::string toString() const;

    bool operator==(const WifiSsid& other) const;
    bool operator!=(const WifiSsid& other) const;

private:
    explicit WifiSsid(const std::string& bytes);
    /* Decoded text or empty on malformed input (Java null), capped at 32
     * chars like the CharBuffer.allocate(32) in the original. */
    static std::string decodeSsid(const std::string& bytes);

    std::string mBytes;
};

} // namespace cdroid

#endif /* __WIFI_SSID_H__ */
