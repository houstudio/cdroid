/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
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
