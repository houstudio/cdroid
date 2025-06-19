#ifndef __WIFI_SSID_H__
#define __WIFI_SSID_H__
#include <string>
#include <vector>
namespace cdroid{
class WifiSsid {
public:
    std::vector<uint8_t> octets;

    static constexpr const char* NONE= "<unknown ssid>";
private:
    static constexpr int HEX_RADIX = 16;

    WifiSsid();
    void convertToBytes(const std::string& asciiEncoded);
    bool isArrayAllZeroes(const std::vector<uint8_t>& ssidBytes)const;
public:
    static WifiSsid* createFromByteArray(const std::vector<uint8_t>& ssid);

    static WifiSsid* createFromAsciiEncoded(const std::string& asciiEncoded);

    static WifiSsid* createFromHex(const std::string& hexStr);

    std::string toString();

    bool operator==(const WifiSsid& thatObject)const;

    bool isHidden() const;

    std::vector<uint8_t> getOctets()const;

    std::string getHexString()const;
};
}/*endof namespace*/
#endif
