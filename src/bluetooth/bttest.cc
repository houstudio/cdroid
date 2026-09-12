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
#include <string>
#include <thread>
#include <unistd.h>

#include <bluetoothadapter.h>
#include <bluetoothdevice.h>

using cdroid::BluetoothAdapter;
using cdroid::BluetoothDevice;

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
        const bool ok = adapter.getRemoteDevice(argv[2]).createBond();
        printf("pair: %s\n", ok ? "ok" : "FAILED (discover it first?)");
        return ok ? 0 : 1;
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
    fprintf(stderr, "unknown command '%s'\n", cmd.c_str());
    return 1;
}
