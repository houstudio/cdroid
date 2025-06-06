#ifndef __WIFIMANAGER_H__
#define __WIFIMANAGER_H__

#include <string>
#include <vector>
#include <functional>
#include <mutex>

namespace cdroid {

class WifiManager {
public:
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

    enum WifiState {
        WIFI_STATE_DISABLED,
        WIFI_STATE_DISABLING,
        WIFI_STATE_ENABLED,
        WIFI_STATE_ENABLING,
        WIFI_STATE_UNKNOWN
    };

    WifiManager();
    ~WifiManager();

    // Wi-Fi State Management
    WifiState getWifiState();
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
    void setWifiStateChangeListener(const std::function<void(WifiState)>& listener);

private:
    std::mutex mutex_;
    std::function<void(WifiState)> wifiStateChangeListener_;
    std::string executeCommand(const std::string& command);
};

} // namespace cdroid

#endif // __WIFIMANAGER_H__
