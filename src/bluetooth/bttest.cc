/*********************************************************************************
 * bttest — bench CLI for the cdblue library (android.bluetooth port over BlueZ),
 * the wpatest analog. With no bluetoothd on the system bus every command
 * degrades to a diagnostic line and exit 1 — never hangs, never crashes.
 *
 *   bttest state                 adapter power state (STATE_* int + name)
 *   bttest enable | disable      Adapter1.Powered
 *   bttest name [newname]        get/set the local friendly name (Alias)
 *   bttest address               adapter hardware address
 *   bttest discover on|off       StartDiscovery / StopDiscovery
 *   bttest list                  cached devices (paired flag, CoD, RSSI)
 *   bttest paired                bonded devices only
 *   bttest pair <addr>           Device1.Pair (needs a discovered remote)
 *   bttest remove <addr>         Adapter1.RemoveDevice (forget)
 *   bttest listen [seconds]      pump events, printing listener callbacks
 *********************************************************************************/
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <string>
#include <thread>
#include <unistd.h>

#include <bluetoothadapter.h>
#include <bluetoothdevice.h>
#include <bluetoothpairing.h>
#include <bluetoothprofile.h>
#include <bluetoothgatt.h>
#include <bluetoothle.h>
#include <bluetoothsocket.h>
#include "internal/sdpclient.h"
#include <bluetoothuuid.h>

using cdroid::BluetoothAdapter;
using cdroid::BluetoothDevice;
using cdroid::BluetoothUuid;
using cdroid::BluetoothGatt;
using cdroid::BluetoothPairingListener;
using cdroid::BluetoothProfile;
using cdroid::BluetoothA2dp;
using cdroid::BluetoothHeadset;
using cdroid::BluetoothGattCallback;
using cdroid::BluetoothGattCharacteristic;
using cdroid::BluetoothGattService;
using cdroid::BluetoothLeScanner;
using cdroid::ScanCallback;
using cdroid::ScanFilter;
using cdroid::ScanResult;
using cdroid::ScanSettings;

static const char* stateName(int s) {
    switch (s) {
    case BluetoothAdapter::STATE_OFF: return "STATE_OFF";
    case BluetoothAdapter::STATE_TURNING_ON: return "STATE_TURNING_ON";
    case BluetoothAdapter::STATE_ON: return "STATE_ON";
    case BluetoothAdapter::STATE_TURNING_OFF: return "STATE_TURNING_OFF";
    default: return "STATE_?";
    }
}

static const char* bondName(int s) {
    switch (s) {
    case BluetoothDevice::BOND_NONE: return "BOND_NONE";
    case BluetoothDevice::BOND_BONDING: return "BOND_BONDING";
    case BluetoothDevice::BOND_BONDED: return "BOND_BONDED";
    default: return "BOND_?";
    }
}

/* Event pump: prints listener callbacks so bench runs show live signal flow. */
class Printer : public BluetoothAdapter::AdapterStateListener,
                public BluetoothAdapter::DiscoveryListener,
                public BluetoothAdapter::BondStateListener {
public:
    void onAdapterStateChanged(int newState, int prevState) override {
        printf("[state] %s -> %s\n", stateName(prevState), stateName(newState));
    }
    void onDiscoveryStarted() override { printf("[discovery] started\n"); }
    void onDeviceFound(const BluetoothDevice& device) override {
        printf("[found] %s  name='%s' class=0x%06x bond=%s\n",
               device.getAddress().c_str(), device.getName().c_str(),
               device.getBluetoothClass().getClassOfDevice(),
               bondName(device.getBondState()));
    }
    void onDiscoveryFinished() override { printf("[discovery] finished\n"); }
    void onBondStateChanged(const BluetoothDevice& device, int bond,
                            int prev) override {
        printf("[bond] %s %s -> %s\n", device.getAddress().c_str(),
               bondName(prev), bondName(bond));
    }
};

static void printDevice(const BluetoothDevice& d) {
    printf("  %-19s %-24s class=0x%06x bond=%-10s type=%d name='%s'\n",
           d.getAddress().c_str(), d.toString().c_str(),
           d.getBluetoothClass().getClassOfDevice(), bondName(d.getBondState()),
           d.getType(), d.getName().c_str());
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <state|enable|disable|name [v]|address|"
                        "discover on|off|list|paired|pair ADDR|remove ADDR|"
                        "listen [sec]>\n", argv[0]);
        return 1;
    }
    const std::string cmd = argv[1];
    BluetoothAdapter& adapter = BluetoothAdapter::getDefaultAdapter();

    if (cmd == "state") {
        printf("state=%d (%s) enabled=%d\n", adapter.getState(),
               stateName(adapter.getState()), (int)adapter.isEnabled());
        return 0;
    }
    if (cmd == "enable" || cmd == "disable") {
        const bool on = cmd == "enable";
        const bool ok = on ? adapter.enable() : adapter.disable();
        printf("%s: %s\n", cmd.c_str(), ok ? "ok" : "FAILED (no bluez/adapter?)");
        return ok ? 0 : 1;
    }
    if (cmd == "name") {
        if (argc >= 3) {
            const bool ok = adapter.setName(argv[2]);
            printf("setName: %s\n", ok ? "ok" : "FAILED");
            return ok ? 0 : 1;
        }
        const std::string name = adapter.getName();
        if (name.empty()) { printf("name: <none> (no adapter)\n"); return 1; }
        printf("name=%s\n", name.c_str());
        return 0;
    }
    if (cmd == "address") {
        const std::string a = adapter.getAddress();
        if (a.empty()) { printf("address: <none> (no adapter)\n"); return 1; }
        printf("address=%s\n", a.c_str());
        return 0;
    }
    if (cmd == "discover" && argc >= 3) {
        const bool on = strcmp(argv[2], "on") == 0;
        const bool ok = on ? adapter.startDiscovery() : adapter.cancelDiscovery();
        printf("discover %s: %s\n", argv[2], ok ? "ok" : "FAILED");
        return ok ? 0 : 1;
    }
    if (cmd == "list" || cmd == "paired") {
        const bool pairedOnly = cmd == "paired";
        int shown = 0;
        for (const BluetoothDevice& d : adapter.getBondedDevices()) {
            printDevice(d);
            shown++;
        }
        if (!pairedOnly) {
            /* the full cache is exposed through a discovery round; devices
             * known to BlueZ (paired) are the synchronous set — list the
             * unpaired ones via the listener path in `listen` instead. */
        }
        printf("%s devices: %d\n", pairedOnly ? "bonded" : "bonded", shown);
        return 0;
    }
    if (cmd == "pair" && argc >= 3) {
        /* Register an agent that auto-answers PIN requests — the bench
         * stand-in for the settings-app pairing dialog. */
        class AutoPin : public BluetoothPairingListener {
        public:
            void onPairingRequest(const BluetoothDevice& device,
                                  int variant) override {
                printf("[pair] agent asked %s variant=%d -> setPin(1234)\n",
                       device.getAddress().c_str(), variant);
                BluetoothDevice d = device;   /* setPin is non-const (AOSP) */
                d.setPin("1234");
            }
        } agent;
        adapter.registerPairingAgent("DisplayYesNo");
        adapter.addPairingListener(&agent);
        const bool ok = adapter.getRemoteDevice(argv[2]).createBond();
        printf("pair: %s\n", ok ? "ok (agent negotiation follows)" : "FAILED (discover it first?)");
        /* createBond is fire-and-forget (AOSP): stay alive for the agent
         * round + the Paired signal before judging the outcome. */
        for (int i = 0; i < 8 && !adapter.getRemoteDevice(argv[2]).getBondState()
                          != BluetoothDevice::BOND_BONDED; i++) usleep(500 * 1000);
        const int bond = adapter.getRemoteDevice(argv[2]).getBondState();
        printf("bond: %s\n", bondName(bond));
        adapter.removePairingListener(&agent);
        return bond == BluetoothDevice::BOND_BONDED ? 0 : 1;
    }
    if (cmd == "remove" && argc >= 3) {
        const bool ok = adapter.getRemoteDevice(argv[2]).removeBond();
        printf("remove: %s\n", ok ? "ok" : "FAILED");
        return ok ? 0 : 1;
    }
    if (cmd == "listen") {
        int seconds = 10;
        if (argc >= 3) seconds = atoi(argv[2]);
        Printer printer;
        adapter.addAdapterStateListener(&printer);
        adapter.addDiscoveryListener(&printer);
        adapter.addBondStateListener(&printer);
        printf("listening for %ds (self-driving: discovery at +1s, cancel at "
               "+4s, pair of the first found device at +6s)\n", seconds);
        /* self-driving sequence so one process exercises the listener
         * flow end to end (signals are per-connection, not broadcast
         * across bttest processes) */
        std::thread driver([&adapter]() {
            sleep(1);
            adapter.startDiscovery();
            sleep(3);
            adapter.cancelDiscovery();
        });
        for (int i = 0; i < seconds * 2; i++) usleep(500 * 1000);
        driver.join();
        return 0;
    }
    if (cmd == "serve" && argc >= 3) {
        /* RFCOMM echo server on the given channel (needs a controller) */
        const int channel = atoi(argv[2]);
        cdroid::BluetoothServerSocket* server =
                adapter.listenUsingRfcommOn(channel);
        if (server == nullptr || !server->isBound()) {
            printf("serve: bind failed (no controller?)\n");
            delete server;
            return 1;
        }
        printf("serving RFCOMM channel %d (echo; Ctrl-C to stop)\n", channel);
        for (;;) {
            cdroid::BluetoothSocket* client = server->accept(1000);
            if (client == nullptr) continue;
            printf("client %s connected\n",
                   client->getRemoteDevice().getAddress().c_str());
            char buf[512];
            int n;
            while ((n = client->getInputStream()->read(buf, sizeof(buf))) > 0) {
                client->getOutputStream()->write(buf, n);
                printf("echo %d bytes\n", n);
            }
            printf("client gone (%d)\n", n);
            delete client;
        }
    }
    if (cmd == "chat" && argc >= 4) {
        /* RFCOMM client: connect ADDR CHANNEL, send stdin lines */
        BluetoothDevice remote = adapter.getRemoteDevice(argv[2]);
        cdroid::BluetoothSocket* sock =
                remote.createRfcommSocket(atoi(argv[3]));
        const int rc = sock->connect();
        if (rc != 0) {
            printf("connect: %s\n", strerror(-rc));
            delete sock;
            return 1;
        }
        printf("connected to %s channel %s\n", argv[2], argv[3]);
        char line[512];
        while (fgets(line, sizeof(line), stdin)) {
            const int len = (int)strlen(line);
            if (sock->getOutputStream()->write(line, len) != len) break;
            const int n = sock->getInputStream()->read(line, sizeof(line));
            if (n <= 0) break;
            printf("<< %.*s", n, line);
        }
        delete sock;
        return 0;
    }
    if (cmd == "uuid") {
        /* pure-logic: canonical string round trip of the constants */
        printf("SPP   %s\n", BluetoothUuid::SerialPort().toString().c_str());
        printf("HFP   %s\n", BluetoothUuid::HFP().toString().c_str());
        printf("A2DP  %s\n", BluetoothUuid::A2DP_SINK().toString().c_str());
        const BluetoothUuid parsed =
                BluetoothUuid::fromString("00001101-0000-1000-8000-00805F9B34FB");
        printf("round-trip %s\n",
               parsed == BluetoothUuid::SerialPort() ? "OK" : "MISMATCH");
        return parsed == BluetoothUuid::SerialPort() ? 0 : 1;
    }
    if (cmd == "blescan" && argc >= 3) {
        const int seconds = atoi(argv[2]);
        class LePrinter : public ScanCallback {
        public:
            void onScanResult(int, const ScanResult& r) override {
                printf("[ble] %s  rssi=%d  name='%s'\n",
                       r.getDevice().getAddress().c_str(), r.getRssi(),
                       r.getDevice().getName().c_str());
            }
            void onScanFailed(int err) override {
                printf("[ble] scan failed err=%d\n", err);
            }
        } printer;
        std::vector<ScanFilter> filters;   /* empty = all */
        ScanSettings settings;
        settings.setScanMode(ScanSettings::SCAN_MODE_LOW_LATENCY);
        BluetoothLeScanner* scanner = adapter.getBluetoothLeScanner();
        if (scanner == nullptr || !scanner->startScan(filters, settings, &printer)) {
            printf("blescan: start failed (no adapter?)\n");
            return 1;
        }
        for (int i = 0; i < seconds * 2; i++) usleep(500 * 1000);
        scanner->stopScan(&printer);
        return 0;
    }
    if (cmd == "gatt" && argc >= 3) {
        /* connect -> discover -> read -> notify -> write, then exit */
        class GattPrinter : public BluetoothGattCallback {
        public:
            void onConnectionStateChange(BluetoothGatt* g, int status,
                                         int newState) override {
                printf("[gatt] state %d -> %d (status %d)\n",
                       g ? 0 : 0, newState, status);
            }
            void onServicesDiscovered(BluetoothGatt* g, int status) override {
                printf("[gatt] services discovered (status %d):\n", status);
                for (BluetoothGattService* s : g->getServices()) {
                    printf("  service %s\n", s->getUuid().toString().c_str());
                    for (BluetoothGattCharacteristic* c :
                            s->getCharacteristics())
                        printf("    char %s props=0x%02x\n",
                               c->getUuid().toString().c_str(),
                               c->getProperties());
                }
            }
            void onCharacteristicRead(BluetoothGatt*, BluetoothGattCharacteristic* c,
                                      int status) override {
                std::string v((const char*)c->getValue().data(), c->getValue().size());
                printf("[gatt] read '%s' (status %d)\n", v.c_str(), status);
            }
            void onCharacteristicWrite(BluetoothGatt*, BluetoothGattCharacteristic*,
                                       int status) override {
                printf("[gatt] write status %d\n", status);
            }
            void onCharacteristicChanged(BluetoothGatt*,
                                         BluetoothGattCharacteristic* c) override {
                std::string v((const char*)c->getValue().data(), c->getValue().size());
                printf("[gatt] notify '%s'\n", v.c_str());
            }
        } printer;
        BluetoothDevice remote = adapter.getRemoteDevice(argv[2]);
        auto gatt = remote.connectGatt(false, &printer);
        if (!gatt->connect()) { printf("gatt: connect failed\n"); return 1; }
        gatt->discoverServices();
        for (BluetoothGattService* s : gatt->getServices()) {
            for (BluetoothGattCharacteristic* c : s->getCharacteristics()) {
                gatt->readCharacteristic(c);
                gatt->setCharacteristicNotification(c, true);
                std::vector<uint8_t> ping{'P','I','N','G'};
                c->setValue(ping);
                gatt->writeCharacteristic(c);
            }
        }
        for (int i = 0; i < 12; i++) usleep(500 * 1000);   /* catch notifies */
        gatt->disconnect();
        gatt->close();
        return 0;
    }
    if (cmd == "profiles") {
        /* stub surface check: proxy hand-off + disconnected defaults */
        class ProxyPrinter : public BluetoothProfile::ServiceListener {
        public:
            void onServiceConnected(int profile,
                                    BluetoothProfile* proxy) override {
                printf("[proxy] profile %d connected: devices=%d state=%d\n",
                       profile,
                       (int)(profile == BluetoothProfile::A2DP
                             ? ((BluetoothA2dp*)proxy)->getConnectedDevices().size()
                             : ((BluetoothHeadset*)proxy)->getConnectedDevices().size()),
                       proxy->getConnectionState(BluetoothDevice("00:00:00:00:00:00")));
                adapter->closeProfileProxy(profile, proxy);
            }
            void onServiceDisconnected(int profile) override {
                printf("[proxy] profile %d disconnected\n", profile);
            }
            BluetoothAdapter* adapter = nullptr;
        } printer;
        printer.adapter = &adapter;
        const bool a2dp = adapter.getProfileProxy(
                &printer, BluetoothProfile::A2DP);
        const bool hfp = adapter.getProfileProxy(
                &printer, BluetoothProfile::HEADSET);
        printf("profiles: a2dp=%d hfp=%d (stubs until the audio "
               "pipeline lands)\n", (int)a2dp, (int)hfp);
        return (a2dp && hfp) ? 0 : 1;
    }
    if (cmd == "sdp" && argc >= 3) {
        /* SDP resolve over L2CAP PSM 1 (needs a live peer with an SDP
         * server — a real phone/dongle; vhci peers have none) */
        const BluetoothUuid uuid = BluetoothUuid::SerialPort();
        std::vector<uint8_t> uuidBytes;
        for (int i = 15; i >= 0; i--)
            uuidBytes.push_back((uint8_t)((i < 8 ? uuid.lsb : uuid.msb)
                    >> ((i % 8) * 8)));
        const int channel = cdroid::sdpResolveRfcommChannel(argv[2], uuidBytes);
        printf("sdp: %s SPP channel = %d\n", argv[2], channel);
        return channel > 0 ? 0 : 1;
    }
    fprintf(stderr, "unknown command '%s'\n", cmd.c_str());
    return 1;
}
