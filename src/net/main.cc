/*
 * wpatest — smoke/demo driver for the cdwifi stack (WifiManager over
 * wpa_supplicant ctrl iface). No cdroid dependency: plain stdio.
 *
 * usage: wpatest [--ctrl /var/run/wpa_supplicant/wlan0] <command> [args]
 *   ping | status | scan | list | save
 *   connect <ssid> <psk> | connectid <networkId>
 *   disconnect | reconnect | disable <id> | remove <id> | forget <id>
 *   events              — print supplicant events + listener callbacks
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>

#include <wifi/supplicantclient.h>
#include <wifi/wifimanager.h>

using cdroid::ScanResult;
using cdroid::WifiConfiguration;
using cdroid::WifiManager;

namespace {

class DemoListeners : public WifiManager::WifiStateListener,
                      public WifiManager::ScanResultsListener,
                      public WifiManager::NetworkStateListener,
                      public WifiManager::RssiListener {
public:
    void onWifiStateChanged(int wifiState) override {
        printf("[listener] wifi state -> %d\n", wifiState);
    }
    void onScanResultsAvailable() override {
        printf("[listener] scan results available\n");
    }
    void onNetworkStateChanged(const cdroid::WifiInfo& info) override {
        printf("[listener] network state: %s\n", info.toString().c_str());
    }
    void onRssiChanged(int newRssi) override {
        printf("[listener] rssi -> %d\n", newRssi);
    }
};

void printScanResults(WifiManager& wifi) {
    const std::vector<ScanResult> results = wifi.getScanResults();
    printf("%zu scan results\n", results.size());
    for (const ScanResult& result : results)
        printf("  %s\n", result.toString().c_str());
}

void printConfigs(WifiManager& wifi) {
    const std::vector<WifiConfiguration> configs = wifi.getConfiguredNetworks();
    printf("%zu configured networks\n", configs.size());
    for (const WifiConfiguration& config : configs)
        printf("  %s\n", config.toString().c_str());
}

} // namespace

int main(int argc, char* argv[]) {
    /* monitor-thread printf must survive SIGTERM even when redirected */
    setvbuf(stdout, nullptr, _IOLBF, 0);
    std::string ctrlPath = cdroid::SupplicantClient::defaultCtrlPath();
    int first = 1;
    while (first + 1 < argc && !strcmp(argv[first], "--ctrl")) {
        ctrlPath = argv[first + 1];
        first += 2;
    }
    if (first >= argc) {
        fprintf(stderr, "usage: %s [--ctrl <path>] <command> [args]\n", argv[0]);
        return 1;
    }
    const std::string command = argv[first++];

    WifiManager& wifi = WifiManager::getInstance();
    if (!wifi.initialize(ctrlPath))
        fprintf(stderr, "warning: supplicant not reachable at %s (retrying in background)\n",
                ctrlPath.c_str());

    int result = 0;
    if (command == "ping") {
        result = wifi.pingSupplicant() ? 0 : 1;
        printf("ping: %s\n", result == 0 ? "OK" : "FAIL");
    } else if (command == "status") {
        printf("wifi state: %d\n", wifi.getWifiState());
        printf("%s\n", wifi.getConnectionInfo().toString().c_str());
    } else if (command == "scan") {
        if (!wifi.startScan()) printf("scan request rejected (busy?)\n");
        /* the results land with CTRL-EVENT-SCAN-RESULTS; poll like wpa_cli */
        sleep(2);
        printScanResults(wifi);
    } else if (command == "list") {
        printConfigs(wifi);
    } else if (command == "save") {
        printf("saveConfiguration: %s\n", wifi.saveConfiguration() ? "OK" : "FAIL");
    } else if (command == "connect" && first + 1 < argc) {
        WifiConfiguration config;
        config.SSID = std::string("\"") + argv[first] + "\"";
        config.preSharedKey = argv[first + 1];
        config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::WPA_PSK);
        config.allowedKeyManagement.set(WifiConfiguration::KeyMgmt::WPA2_PSK);
        wifi.connect(config, nullptr);
        printf("connect requested\n");
        sleep(8);
        printf("%s\n", wifi.getConnectionInfo().toString().c_str());
    } else if (command == "connectid" && first < argc) {
        wifi.connect(atoi(argv[first]), nullptr);
        printf("connect requested\n");
        sleep(8);
        printf("%s\n", wifi.getConnectionInfo().toString().c_str());
    } else if (command == "disconnect") {
        printf("disconnect: %s\n", wifi.disconnect() ? "OK" : "FAIL");
    } else if (command == "reconnect") {
        printf("reconnect: %s\n", wifi.reconnect() ? "OK" : "FAIL");
    } else if (command == "disable" && first < argc) {
        printf("disable %d: %s\n", atoi(argv[first]),
               wifi.disableNetwork(atoi(argv[first])) ? "OK" : "FAIL");
    } else if (command == "remove" && first < argc) {
        printf("remove %d: %s\n", atoi(argv[first]),
               wifi.removeNetwork(atoi(argv[first])) ? "OK" : "FAIL");
    } else if (command == "forget" && first < argc) {
        wifi.forget(atoi(argv[first]), nullptr);
        printf("forget %d done\n", atoi(argv[first]));
    } else if (command == "events") {
        DemoListeners listeners;
        wifi.addWifiStateListener(&listeners);
        wifi.addScanResultsListener(&listeners);
        wifi.addNetworkStateListener(&listeners);
        wifi.addRssiListener(&listeners);
        printf("listening (ctrl-c to stop)\n");
        while (true) sleep(1);
    } else {
        fprintf(stderr, "unknown/malformed command: %s\n", command.c_str());
        result = 2;
    }
    return result;
}
