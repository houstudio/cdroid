/*
 * ethtest — smoke/demo driver for the cdnet stack (EthernetManager /
 * ConnectivityManager). Read-only commands need no privileges; the write
 * path (setstatic/setdhcp/up/down) needs root (ioctls + spawned DHCP
 * client).
 *
 * usage: ethtest [--pattern <regex>] <command> [args]
 *   list | state <iface> | conf <iface> | dhcpinfo <iface> | connectivity | events
 *   setstatic <iface> <ip/prefix> <gateway> <dns1> [dns2]   (root)
 *   setdhcp <iface> | up <iface> | down <iface>             (root)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>
#include <vector>

#include <connectivitymanager.h>
#include <ethernet/ethernetmanager.h>

using cdroid::ConnectivityManager;
using cdroid::EthernetManager;
using cdroid::IpConfiguration;
using cdroid::LinkAddress;
using cdroid::NetworkInfo;
using cdroid::StaticIpConfiguration;

namespace {

void printConnectivity() {
    ConnectivityManager& cm = ConnectivityManager::getInstance();
    const NetworkInfo active = cm.getActiveNetworkInfo();
    printf("active: %s\n", active.toString().c_str());
    printf("active connected: %s\n", active.isConnected() ? "true" : "false");
    for (const NetworkInfo& info : cm.getAllNetworkInfo())
        printf("  %s\n", info.toString().c_str());
}

} // namespace

int main(int argc, char* argv[]) {
    setvbuf(stdout, nullptr, _IOLBF, 0);
    int first = 1;
    if (first + 1 < argc && !strcmp(argv[first], "--pattern")) {
        EthernetManager::getInstance().setInterfacePattern(argv[first + 1]);
        first += 2;
    }
    if (first >= argc) {
        fprintf(stderr, "usage: %s [--pattern <regex>] <command> [args]\n", argv[0]);
        return 1;
    }
    const std::string command = argv[first];
    const int remaining = argc - first - 1;   /* args after the command */
    const char* const* args = argv + first + 1;

    if (command == "list") {
        for (const std::string& iface : EthernetManager::getInstance().getAvailableInterfaces())
            printf("%s\n", iface.c_str());
    } else if (command == "state" && remaining >= 1) {
        const std::string iface = args[0];
        printf("%s: available=%s\n", iface.c_str(),
               EthernetManager::getInstance().isAvailable(iface) ? "true" : "false");
        printf("  config: %s\n",
               EthernetManager::getInstance().getConfiguration(iface).toString().c_str());
    } else if (command == "conf" && remaining >= 1) {
        printf("%s\n",
               EthernetManager::getInstance().getConfiguration(args[0]).toString().c_str());
    } else if (command == "dhcpinfo" && remaining >= 1) {
        printf("%s\n",
               EthernetManager::getInstance().getDhcpInfo(args[0]).toString().c_str());
    } else if (command == "connectivity") {
        printConnectivity();
    } else if (command == "setstatic" && remaining >= 4) {
        IpConfiguration config;
        StaticIpConfiguration staticIp;
        staticIp.setIpAddress(LinkAddress(args[1]));
        staticIp.setGateway(args[2]);
        std::vector<std::string> dns{args[3]};
        if (remaining >= 5) dns.push_back(args[4]);
        staticIp.setDnsServers(dns);
        config.setIpAssignment(IpConfiguration::IpAssignment::STATIC);
        config.setProxySettings(IpConfiguration::ProxySettings::NONE);
        config.setStaticIpConfiguration(staticIp);
        EthernetManager::getInstance().setConfiguration(args[0], config);
        printf("static configuration applied to %s\n", args[0]);
    } else if (command == "setdhcp" && remaining >= 1) {
        IpConfiguration config;
        config.setIpAssignment(IpConfiguration::IpAssignment::DHCP);
        config.setProxySettings(IpConfiguration::ProxySettings::NONE);
        EthernetManager::getInstance().setConfiguration(args[0], config);
        printf("dhcp requested on %s\n", args[0]);
    } else if (command == "up" && remaining >= 1) {
        printf("up %s: %s\n", args[0],
               EthernetManager::getInstance().enableInterface(args[0]) ? "OK" : "FAIL");
    } else if (command == "down" && remaining >= 1) {
        printf("down %s: %s\n", args[0],
               EthernetManager::getInstance().disableInterface(args[0]) ? "OK" : "FAIL");
    } else if (command == "events") {
        struct AvailabilityPrinter : public EthernetManager::Listener {
            void onAvailabilityChanged(const std::string& iface, bool isAvailable) override {
                printf("[eth] %s available=%s\n", iface.c_str(), isAvailable ? "true" : "false");
            }
        };
        struct ConnectivityPrinter : public ConnectivityManager::NetworkStateListener {
            void onNetworkStateChanged(const NetworkInfo& info) override {
                printf("[cm ] %s\n", info.toString().c_str());
            }
        };
        AvailabilityPrinter ethPrinter;
        ConnectivityPrinter cmPrinter;
        EthernetManager::getInstance().addListener(&ethPrinter);
        ConnectivityManager::getInstance().addNetworkStateListener(&cmPrinter);
        printf("listening 30s...\n");
        sleep(30);
    } else {
        fprintf(stderr, "unknown/malformed command: %s\n", command.c_str());
        return 2;
    }
    return 0;
}
