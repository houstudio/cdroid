#ifndef __WIFI_SCANNER_H__
#define __WIFI_SCANNER_H__

#include <string>
#include <vector>
#include <functional>
#include <wpa_ctrl.h>
namespace cdroid{
class Looper;

struct WiFiNetwork {
    std::string bssid;
    int frequency;
    int signalLevel;
    std::string flags;
    std::string ssid;
};

class WiFiScanner {
private:
    void handleEvent();
    struct wpa_ctrl* ctrl_;
    int ctrlFd_;
    static int FDCallback(int fd, int events, void* data);
    std::vector<WiFiNetwork> parseScanResults(const std::string& results);
public:
    WiFiScanner(Looper*looper,const std::string& ctrlPath);
    ~WiFiScanner();

    bool startScan();
    std::vector<WiFiNetwork> getScanResults();
    // Callback for scan completion
    std::function<void()> onScanComplete;
};
}/*endof namespace*/
#endif // __WIFI_SCANNER_H__
