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
#include <core/wifimanager.h>
#include <core/process.h>
#include <utils/textutils.h>
#include <sstream>
#include <iostream>

namespace cdroid {

WifiManager::WifiManager() = default;

WifiManager::~WifiManager() = default;

std::string WifiManager::executeCommand(const std::string& command) {
    Process process;
    process.exec("/bin/sh", {"-c", command});
    process.wait(); // 等待命令执行完成
    return process.readAllStandardOutput();
}

int WifiManager::getWifiState() {
    std::string result = executeCommand("wpa_cli status");
    if (result.find("wpa_state=COMPLETED") != std::string::npos) {
        return WIFI_STATE_ENABLED;
    } else if (result.find("wpa_state=DISCONNECTED") != std::string::npos) {
        return WIFI_STATE_DISABLED;
    }
    return WIFI_STATE_UNKNOWN;
}

bool WifiManager::setWifiEnabled(bool enabled) {
    std::string command = enabled ? "wpa_cli enable" : "wpa_cli disable";
    std::string result = executeCommand(command);
    return result.find("OK") != std::string::npos;
}

bool WifiManager::startScan() {
    std::string result = executeCommand("wpa_cli scan");
    return result.find("OK") != std::string::npos;
}

std::vector<WifiManager::WifiNetwork> WifiManager::getScanResults() {
    std::vector<WifiNetwork> networks;
    std::string result = executeCommand("wpa_cli scan_results");
    std::vector<std::string> lines = TextUtils::split(result, '\n');
    for (size_t i = 2; i < lines.size(); ++i) { // Skip the header lines
        std::vector<std::string> columns = TextUtils::split(lines[i], '\t');
        if (columns.size() >= 5) {
            WifiNetwork network;
            network.bssid = columns[0];
            network.signalLevel = std::stoi(columns[1]);
            network.flags = columns[2];
            network.ssid = columns[4];
            networks.push_back(network);
        }
    }
    return networks;
}

bool WifiManager::connect(const std::string& ssid, const std::string& psk) {
    int networkId = addNetwork(ssid, psk);
    if (networkId < 0) {
        return false;
    }
    return enableNetwork(networkId, true);
}

bool WifiManager::disconnect() {
    std::string result = executeCommand("wpa_cli disconnect");
    return result.find("OK") != std::string::npos;
}

WifiManager::ConnectionInfo WifiManager::getConnectionInfo() {
    ConnectionInfo info;
    std::string result = executeCommand("wpa_cli status");
    std::vector<std::string> lines = TextUtils::split(result, '\n');
    for (const auto& line : lines) {
        if (line.find("ssid=") == 0) {
            info.ssid = line.substr(5);
        } else if (line.find("bssid=") == 0) {
            info.bssid = line.substr(6);
        } else if (line.find("ip_address=") == 0) {
            info.ipAddress = line.substr(11);
        } else if (line.find("signal_level=") == 0) {
            info.signalLevel = std::stoi(line.substr(13));
        }
    }
    return info;
}

bool WifiManager::addNetwork(const std::string& ssid, const std::string& psk) {
    std::string result = executeCommand("wpa_cli add_network");
    if (result.empty()) {
        return -1;
    }
    int networkId = std::stoi(result);
    executeCommand("wpa_cli set_network " + std::to_string(networkId) + " ssid '\"" + ssid + "\"'");
    executeCommand("wpa_cli set_network " + std::to_string(networkId) + " psk '\"" + psk + "\"'");
    return networkId;
}

bool WifiManager::removeNetwork(int networkId) {
    std::string result = executeCommand("wpa_cli remove_network " + std::to_string(networkId));
    return result.find("OK") != std::string::npos;
}

bool WifiManager::enableNetwork(int networkId, bool disableOthers) {
    std::string command = "wpa_cli enable_network " + std::to_string(networkId);
    if (disableOthers) {
        command += " && wpa_cli select_network " + std::to_string(networkId);
    }
    std::string result = executeCommand(command);
    return result.find("OK") != std::string::npos;
}

bool WifiManager::disableNetwork(int networkId) {
    std::string result = executeCommand("wpa_cli disable_network " + std::to_string(networkId));
    return result.find("OK") != std::string::npos;
}

void WifiManager::setWifiStateChangeListener(const std::function<void(int)>& listener) {
    std::lock_guard<std::mutex> lock(mutex_);
    wifiStateChangeListener_ = listener;
}

} // namespace cdroid
