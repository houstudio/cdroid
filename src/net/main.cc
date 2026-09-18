/*
 * wpatest — smoke/demo driver for the cdwifi stack (WifiManager over
 * wpa_supplicant ctrl iface). No cdroid dependency: plain stdio.
 *
 * usage: wpatest [--ctrl /var/run/wpa_supplicant/wlan0] <command> [args]
 *   ping | status | scan | list | save
 *   connect <ssid> <psk> | connectid <networkId>
 *   disconnect | reconnect | disable <id> | remove <id> | forget <id>
 *   events              — print supplicant events + listener callbacks
 *   apiface <iface>     — Soft AP interface (default: the STA interface)
 *   apconf <ssid> [psk] — store + persist the hotspot configuration
 *   apstart [ssid] [psk]— start tethered hotspot (no args = stored/default
 *                         config; ssid only = open network)
 *   apstop              — stop the hotspot
 *   apstatus            — AP state + effective configuration
 *   aptether            — startTethering(WIFI): hotspot + NAT to upstream
 *   apuntether          — stopTethering(WIFI)
 *   powersave <iface> <on|off> — WEXT SIOCSIWPOWER (the bench has no iw;
 *                         STA power save off is what makes unicast downlink
 *                         reach the client on mac80211_hwsim)
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/wireless.h>

#include <string>

#include <wifi/supplicantclient.h>
#include <wifi/wifimanager.h>
#include <wifi/nl80211radio.h>
#include <connectivitymanager.h>
#include <natcontroller.h>

using cdroid::ScanResult;
using cdroid::SoftApConfiguration;
using cdroid::WifiConfiguration;
using cdroid::WifiManager;

namespace {

/* Value-semantics listener set (CallbackBase typedefs + lambdas). */
struct DemoListeners {
    WifiManager::WifiStateListener wifiState{
        [](int wifiState) {
            printf("[listener] wifi state -> %d\n", wifiState);
        }};
    WifiManager::ScanResultsListener scanResults{
        [] { printf("[listener] scan results available\n"); }};
    WifiManager::NetworkStateListener networkState{
        [](const cdroid::WifiInfo& info) {
            printf("[listener] network state: %s\n", info.toString().c_str());
        }};
    WifiManager::RssiListener rssi{
        [](int newRssi) {
            printf("[listener] rssi -> %d\n", newRssi);
        }};
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
    } else if (command == "apiface" && first < argc) {
        wifi.configureSoftApInterface(argv[first]);
        printf("soft ap iface -> %s\n", argv[first]);
    } else if (command == "apifacenew" && first + 1 < argc) {
        /* dedicated AP interface via nl80211 NEW_INTERFACE (product ap0
         * shape), then point the Soft AP at it */
        std::string error;
        if (cdroid::Nl80211Radio::createApInterface(argv[first], argv[first + 1],
                                                    &error)) {
            wifi.configureSoftApInterface(argv[first + 1]);
            printf("ap interface %s created on radio %s\n",
                   argv[first + 1], argv[first]);
        } else {
            fprintf(stderr, "E: %s\n", error.c_str());
            result = 1;
        }
    } else if (command == "apconf" && first < argc) {
        /* store (and persist) the hotspot configuration.
         * argv[first] = ssid, argv[first+1] = passphrase (optional — an
         * open network stores no passphrase). The outer guard checks the
         * SSID only (apstart's shape): requiring first+1 here made the psk
         * mandatory and dropped the documented ssid-only form into the
         * unknown-command handler. */
        SoftApConfiguration::Builder builder;
        builder.setSsid(argv[first]);
        if (first + 1 < argc)
            builder.setPassphrase(argv[first + 1],
                    SoftApConfiguration::SECURITY_TYPE_WPA2_PSK);
        const SoftApConfiguration config = builder.build();
        printf("setSoftApConfiguration: %d\n%s\n",
               wifi.setSoftApConfiguration(config) ? 1 : 0,
               config.toString().c_str());
    } else if (command == "powersave" && first + 1 < argc) {
        /* WEXT SIOCSIWPOWER: 1 = power management off. wpa_supplicant
         * re-enables PS after each (re)association — call this once the
         * STA is COMPLETED on benches where unicast downlink must not be
         * buffered (mac80211_hwsim). */
        const int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0) {
            fprintf(stderr, "E: socket: %s\n", strerror(errno));
            return 1;
        }
        struct iwreq wr;
        memset(&wr, 0, sizeof(wr));
        strncpy(wr.ifr_name, argv[first], IFNAMSIZ - 1);
        wr.u.power.disabled = strcmp(argv[first + 1], "off") == 0 ? 1 : 0;
        const int rc = ioctl(fd, SIOCSIWPOWER, &wr);
        const int savedErrno = errno;
        close(fd);
        if (rc != 0) {
            fprintf(stderr, "E: SIOCSIWPOWER %s %s: %s\n", argv[first],
                    argv[first + 1], strerror(savedErrno));
            return 1;
        }
        printf("power save %s on %s\n", argv[first + 1], argv[first]);
    } else if (command == "aptether") {
        /* full tethering: hotspot + NAT toward the default-route interface */
        cdroid::ConnectivityManager& connectivity =
                cdroid::ConnectivityManager::getInstance();
        printf("upstream iface: %s\n",
               connectivity.startTethering(cdroid::ConnectivityManager::TETHERING_WIFI)
               ? cdroid::NatController::defaultRouteInterface().c_str()
               : std::string("(failed)").c_str());
        result = 0;
        /* hold ownership long enough for an external test harness to run
         * its gates against the live AP + NAT (teardown = process exit) */
        sleep(60);
    } else if (command == "apuntether") {
        cdroid::ConnectivityManager& connectivity =
                cdroid::ConnectivityManager::getInstance();
        printf("stopTethering: %s\n",
               connectivity.stopTethering(cdroid::ConnectivityManager::TETHERING_WIFI)
               ? "OK" : "FAIL");
        result = 0;
    } else if (command == "apstart") {
        /* AP mode is independent of the STA supplicant transport.
         * argv[first] = ssid (optional), argv[first+1] = passphrase. */
        SoftApConfiguration config;
        if (first < argc) {
            SoftApConfiguration::Builder builder;
            builder.setSsid(argv[first]);
            if (first + 1 < argc)
                builder.setPassphrase(argv[first + 1],
                        SoftApConfiguration::SECURITY_TYPE_WPA2_PSK);
            config = builder.build();
        }
        WifiManager::WifiApStateListener apState = [](int state) {
            printf("[listener] wifi ap state -> %d\n", state);
        };
        WifiManager::SoftApCallback printer;
        printer.onStateChanged =
                [](int state, int failureCode) {
                    printf("[softap-cb] state=%d failure=%d\n", state, failureCode);
                };
        printer.onConnectedClientsChanged =
                [](const std::vector<cdroid::WifiClient>& clients, int reasonCode) {
                    printf("[softap-cb] clients=%zu (reason %d):\n",
                           clients.size(), reasonCode);
                    for (const cdroid::WifiClient& client : clients)
                        printf("    %s\n", client.toString().c_str());
                };
        printer.onInfoChanged =
                [](const std::vector<cdroid::SoftApInfo>& infos) {
                    for (const cdroid::SoftApInfo& info : infos)
                        printf("[softap-cb] info: %s\n", info.toString().c_str());
                };
        printer.onCapabilityChanged =
                [](const cdroid::SoftApCapability& cap) {
                    printf("[softap-cb] capability: %s\n", cap.toString().c_str());
                };
        wifi.addWifiApStateListener(apState);
        wifi.registerSoftApCallback(printer);
        const bool ok = wifi.startTetheredHotspot(
                first < argc ? &config : nullptr);
        printf("startTetheredHotspot: %s (state %d)\n", ok ? "OK" : "FAIL",
               wifi.getWifiApState());
        if (ok) {
            /* keep the process alive briefly: AP-STA events + leases arrive
             * asynchronously and the printer shows them live */
            printf("listening for softap events 20s...\n");
            sleep(20);
        }
        wifi.unregisterSoftApCallback(printer);
        wifi.removeWifiApStateListener(apState);
        result = ok ? 0 : 1;
    } else if (command == "apstop") {
        const bool ok = wifi.stopSoftAp();
        printf("stopSoftAp: %s (state %d)\n", ok ? "OK" : "FAIL",
               wifi.getWifiApState());
        result = ok ? 0 : 1;
    } else if (command == "apstatus") {
        printf("wifi ap state: %d (enabled %d)\n", wifi.getWifiApState(),
               wifi.isWifiApEnabled() ? 1 : 0);
        printf("%s\n", wifi.getSoftApConfiguration().toString().c_str());
    } else if (command == "events") {
        DemoListeners listeners;
        wifi.addWifiStateListener(listeners.wifiState);
        wifi.addScanResultsListener(listeners.scanResults);
        wifi.addNetworkStateListener(listeners.networkState);
        wifi.addRssiListener(listeners.rssi);
        printf("listening (ctrl-c to stop)\n");
        while (true) sleep(1);
    } else {
        fprintf(stderr, "unknown/malformed command: %s\n", command.c_str());
        result = 2;
    }
    return result;
}
