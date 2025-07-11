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
#ifndef __WIFIMANAGER_H__
#define __WIFIMANAGER_H__

#include <string>
#include <vector>
#include <functional>
#include <mutex>

namespace cdroid {

class WifiManager {
public:
    static constexpr int WIFI_STATE_DISABLING=0;
    static constexpr int WIFI_STATE_DISABLED =1;
    static constexpr int WIFI_STATE_ENABLING =2;
    static constexpr int WIFI_STATE_ENABLED  =3;
    static constexpr int WIFI_STATE_UNKNOWN  =4;
    struct WifiNetwork {
        std::string ssid;
        std::string bssid;
        int signalLevel;
        std::string flags;
    };

    struct ConnectionInfo {
        std::string ssid;
        std::string bssid;
        std::string ipAddress;
        int signalLevel;
    };

    WifiManager();
    ~WifiManager();

    // Wi-Fi State Management
    int getWifiState();
    bool setWifiEnabled(bool enabled);

    // Scanning
    bool startScan();
    std::vector<WifiNetwork> getScanResults();

    // Connection Management
    bool connect(const std::string& ssid, const std::string& psk);
    bool disconnect();
    ConnectionInfo getConnectionInfo();

    // Network Management
    bool addNetwork(const std::string& ssid, const std::string& psk);
    bool removeNetwork(int networkId);
    bool enableNetwork(int networkId, bool disableOthers);
    bool disableNetwork(int networkId);

    // Listener for Wi-Fi state changes
    void setWifiStateChangeListener(const std::function<void(int)>& listener);

private:
    std::mutex mutex_;
    std::function<void(int)> wifiStateChangeListener_;
    std::string executeCommand(const std::string& command);
};

} // namespace cdroid

#endif // __WIFIMANAGER_H__
