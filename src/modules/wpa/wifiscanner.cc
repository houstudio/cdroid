#include <wifiscanner.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <core/looper.h>
namespace cdroid{
WiFiScanner::WiFiScanner(Looper*looper,const std::string& ctrlPath)
    : ctrl_(nullptr), ctrlFd_(-1) {
    ctrl_ = wpa_ctrl_open(ctrlPath.c_str());
    if (!ctrl_) {
        throw std::runtime_error("Failed to open wpa_ctrl at " + ctrlPath);
    }

    if (wpa_ctrl_attach(ctrl_) != 0) {
        wpa_ctrl_close(ctrl_);
        throw std::runtime_error("Failed to attach to wpa_supplicant events");
    }

    ctrlFd_ = wpa_ctrl_get_fd(ctrl_);
    looper->addFd(ctrlFd_,0,Looper::EVENT_INPUT,FDCallback,this);
}

WiFiScanner::~WiFiScanner() {
    if (ctrl_) {
        wpa_ctrl_detach(ctrl_);
        wpa_ctrl_close(ctrl_);
    }
}

int WiFiScanner::FDCallback(int fd, int events, void* data){
    WiFiScanner*thiz=(WiFiScanner*)data;
    if((fd==thiz->ctrlFd_)&&(events&Looper::EVENT_INPUT)){
        char buffer[256];
        size_t len = sizeof(buffer);
        int ret;
        std::ostringstream oss;
        do{
            ret = wpa_ctrl_recv(thiz->ctrl_,buffer,&len);
            if(ret>0){
                buffer[len]=0;
                oss<<buffer;
            }
        }while(ret>0);
    }
}

bool WiFiScanner::startScan() {
    const char* cmd = "SCAN";
    if (wpa_ctrl_request(ctrl_, cmd, strlen(cmd), nullptr, nullptr, nullptr) != 0) {
        std::cerr << "Failed to start scan" << std::endl;
        return false;
    }
    return true;
}

std::vector<WiFiNetwork> WiFiScanner::getScanResults() {
    const char* cmd = "SCAN_RESULTS";
    char buffer[4096];
    size_t len = sizeof(buffer);

    if (wpa_ctrl_request(ctrl_, cmd, strlen(cmd), buffer, &len, nullptr) != 0) {
        std::cerr << "Failed to get scan results" << std::endl;
        return {};
    }

    buffer[len] = '\0';
    return parseScanResults(buffer);
}

void WiFiScanner::handleEvent() {
    char buffer[4096];
    size_t len = sizeof(buffer);

    if (wpa_ctrl_recv(ctrl_, buffer, &len) == 0) {
        buffer[len] = '\0';
        std::string event(buffer);
        std::cout << "Received event: " << event << std::endl;

        if (event.find("CTRL-EVENT-SCAN-RESULTS") != std::string::npos) {
            if (onScanComplete) {
                onScanComplete();
            }
        }
    } else {
        std::cerr << "Failed to receive event" << std::endl;
    }
}

std::vector<WiFiNetwork> WiFiScanner::parseScanResults(const std::string& results) {
    std::vector<WiFiNetwork> networks;
    std::istringstream stream(results);
    std::string line;

    // Skip the first line (header)
    std::getline(stream, line);

    while (std::getline(stream, line)) {
        std::istringstream lineStream(line);
        WiFiNetwork network;

        lineStream >> network.bssid >> network.frequency >> network.signalLevel >> network.flags;
        std::getline(lineStream, network.ssid);
        if (!network.ssid.empty() && network.ssid[0] == '\t') {
            network.ssid.erase(0, 1); // Remove leading tab
        }

        networks.push_back(network);
    }

    return networks;
}
}/*endof namespace*/
