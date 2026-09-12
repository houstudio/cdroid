/*********************************************************************************
 * btmock — an org.bluez stand-in on a private D-Bus, the no-hardware bench for
 * the cdblue transport (the role btvirt + vhci would play with real
 * controllers). Serves one adapter and two Device1 objects, answers the
 * methods cdblue issues and emits the signals it listens for:
 *
 *   Adapter1: Powered/Alias/Address/Discovering properties,
 *             StartDiscovery/StopDiscovery/RemoveDevice
 *   Device1:  Address/Name/Alias/Paired/Connected/RSSI/Class/UUIDs/AddressType,
 *             Pair
 *   signals:  PropertiesChanged (RSSI flips each second, Powered follows the
 *             setter), InterfacesAdded (a third device appears on discovery)
 *
 * Run under a session bus and point bttest at it:
 *   dbus-run-session -- ./btmock &
 *   DBUS_SYSTEM_BUS_ADDRESS=<that bus> ./bttest state
 *********************************************************************************/
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <systemd/sd-bus.h>
#include <unistd.h>

static const char* kAdapterPath = "/org/bluez/hci0";
static const char* kDev1 = "/org/bluez/hci0/dev_11_22_33_44_55_66";
static const char* kDev2 = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF";
static const char* kDev3 = "/org/bluez/hci0/dev_77_88_99_00_11_22";

static bool gPowered = true;
static bool gDiscovering = false;
static int gRssi = -42;

/* --- adapter property getters ------------------------------------------ */
static int get_adapter_powered(sd_bus* bus, const char* path,
                               const char* interface, const char* property,
                               sd_bus_message* reply, void* userdata,
                               sd_bus_error* err) {
    return sd_bus_message_append(reply, "b", gPowered);
}
static int get_adapter_discovering(sd_bus*, const char*, const char*,
                                   const char*, sd_bus_message* reply, void*,
                                   sd_bus_error*) {
    return sd_bus_message_append(reply, "b", gDiscovering);
}
static int get_adapter_alias(sd_bus*, const char*, const char*, const char*,
                             sd_bus_message* reply, void*, sd_bus_error*) {
    return sd_bus_message_append(reply, "s", "cdroid-mock");
}
static int get_adapter_address(sd_bus*, const char*, const char*, const char*,
                               sd_bus_message* reply, void*, sd_bus_error*) {
    return sd_bus_message_append(reply, "s", "00:11:22:33:44:55");
}

/* --- adapter methods ---------------------------------------------------- */
static int start_discovery(sd_bus_message* m, void*, sd_bus_error*) {
    sd_bus* bus = sd_bus_message_get_bus(m);
    gDiscovering = true;
    sd_bus_reply_method_return(m, "");
    /* RSSI wobble + a new device appearing: what a real scan delivers. */
    sd_bus_emit_properties_changed_strv(bus, kAdapterPath, "org.bluez.Adapter1",
                                        nullptr);
    return sd_bus_emit_object_added(bus, kDev3);
}
static int stop_discovery(sd_bus_message* m, void*, sd_bus_error*) {
    sd_bus* bus = sd_bus_message_get_bus(m);
    gDiscovering = false;
    sd_bus_reply_method_return(m, "");
    return sd_bus_emit_properties_changed_strv(bus, kAdapterPath,
                                               "org.bluez.Adapter1", nullptr);
}
static int remove_device(sd_bus_message* m, void*, sd_bus_error*) {
    sd_bus* bus = sd_bus_message_get_bus(m);
    const char* path = nullptr;
    sd_bus_message_read(m, "o", &path);
    sd_bus_reply_method_return(m, "");
    printf("[btmock] RemoveDevice %s\n", path);
    return sd_bus_emit_object_removed(bus, path);
}

/* --- device property getters --------------------------------------------- */
typedef struct {
    const char* address;
    const char* name;
    const char* alias;
    const char* addressType;   /* null = BR/EDR */
    bool paired;
    uint32_t cod;
} MockDevice;

static const MockDevice kDevices[] = {
    {"11:22:33:44:55:66", "Mock Keyboard", "Mock Keyboard", nullptr, true,  0x002540},
    {"AA:BB:CC:DD:EE:FF", "Mock Phone",    "Mock Phone",    "public", false, 0},
    {"77:88:99:00:11:22", "Mock Headset",  "Mock Headset",  nullptr, false, 0x240404},
};

/* Property getters switch on the object path so every mock device is
 * enumerated by GetManagedObjects (the vtable is what makes an object
 * visible to ObjectManager). */
static const MockDevice* device_for_path(const char* path) {
    if (!path) return &kDevices[0];
    const std::string p = path;
    if (p == kDev2) return &kDevices[1];
    if (p == kDev3) return &kDevices[2];
    return &kDevices[0];   /* kDev1 and anything else */
}
static int get_device_address(sd_bus*, const char* path, const char*,
                              const char*, sd_bus_message* reply, void*,
                              sd_bus_error*) {
    return sd_bus_message_append(reply, "s", device_for_path(path)->address);
}
static int get_device_name(sd_bus*, const char* path, const char*,
                           const char*, sd_bus_message* reply, void*,
                           sd_bus_error*) {
    return sd_bus_message_append(reply, "s", device_for_path(path)->name);
}
static bool gDev2Paired = false;   /* Pair() flips this */
static int get_device_paired(sd_bus*, const char* path, const char*,
                             const char*, sd_bus_message* reply, void*,
                             sd_bus_error*) {
    const MockDevice* d = device_for_path(path);
    const bool paired = d == &kDevices[1] ? gDev2Paired : d->paired;
    return sd_bus_message_append(reply, "b", paired ? 1 : 0);
}

static int pair_device(sd_bus_message* m, void*, sd_bus_error*) {
    sd_bus* bus = sd_bus_message_get_bus(m);
    const char* path = sd_bus_message_get_path(m);
    sd_bus_reply_method_return(m, "");
    printf("[btmock] Pair %s\n", path);
    /* bond completed: flip the Paired property and announce it */
    gDev2Paired = true;
    return sd_bus_emit_properties_changed(bus, path, "org.bluez.Device1",
                                          "Paired", NULL);   /* names only: the vtable getter supplies the value */
}

/* --- vtables --------------------------------------------------------------- */
static const sd_bus_vtable kAdapterVtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_PROPERTY("Powered", "b", get_adapter_powered, 0, 0),
    SD_BUS_PROPERTY("Discovering", "b", get_adapter_discovering, 0, 0),
    SD_BUS_PROPERTY("Alias", "s", get_adapter_alias, 0, 0),
    SD_BUS_PROPERTY("Address", "s", get_adapter_address, 0, 0),
    SD_BUS_METHOD("StartDiscovery", NULL, NULL, start_discovery, 0),
    SD_BUS_METHOD("StopDiscovery", NULL, NULL, stop_discovery, 0),
    SD_BUS_METHOD("RemoveDevice", "o", NULL, remove_device, 0),
    SD_BUS_VTABLE_END,
};

static const sd_bus_vtable kDeviceVtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_PROPERTY("Address", "s", get_device_address, 0, 0),
    SD_BUS_PROPERTY("Name", "s", get_device_name, 0, 0),
    SD_BUS_PROPERTY("Paired", "b", get_device_paired, 0, 0),
    SD_BUS_METHOD("Pair", NULL, NULL, pair_device, 0),
    SD_BUS_VTABLE_END,
};

/* Emit an InterfacesAdded with a full Device1 property set — the shape a
 * scan result delivers and what BluezClient's parser must survive. */
static int emit_device(sd_bus* bus, const MockDevice* d, const char* path) {
    sd_bus_message* m = nullptr;
    int rc = sd_bus_message_new_signal(bus, &m, "/org/bluez",
                                       "org.freedesktop.DBus.ObjectManager",
                                       "InterfacesAdded");
    if (rc < 0) return rc;
    sd_bus_message_append(m, "o", path);
    sd_bus_message_open_container(m, 'a', "{sa{sv}}");
    sd_bus_message_open_container(m, 'e', "sa{sv}");
    sd_bus_message_append(m, "s", "org.bluez.Device1");
    sd_bus_message_open_container(m, 'a', "{sv}");
    struct KV { const char* k; char type; const char* s; int b; };
    const KV kvs[] = {
        {"Address", 's', d->address, 0},
        {"Name", 's', d->name, 0},
        {"Alias", 's', d->alias, 0},
        {"AddressType", 's', d->addressType ? d->addressType : "", 0},
        {"Paired", 'b', nullptr, (int)d->paired},
        {"Connected", 'b', nullptr, 0},
        {"RSSI", 'n', nullptr, gRssi},
        {"Class", 'u', nullptr, (int)d->cod},
    };
    for (const KV& kv : kvs) {
        sd_bus_message_open_container(m, 'e', "sv");
        sd_bus_message_append(m, "s", kv.k);
        if (kv.type == 's') {
            sd_bus_message_append(m, "v", "s", kv.s);
        } else if (kv.type == 'b') {
            sd_bus_message_append(m, "v", "b", kv.b);
        } else if (kv.type == 'n') {
            sd_bus_message_append(m, "v", "n", (int16_t)kv.b);
        } else if (kv.type == 'u') {
            sd_bus_message_append(m, "v", "u", (uint32_t)kv.b);
        }
        sd_bus_message_close_container(m);
    }
    /* UUIDs: as array-of-string variant */
    sd_bus_message_open_container(m, 'e', "sv");
    sd_bus_message_append(m, "s", "UUIDs");
    sd_bus_message_open_container(m, 'v', "as");
    sd_bus_message_append_strv(m, (char**) (const char*[]) {(char*)"00001101-0000-1000-8000-00805f9b34fb", nullptr});
    sd_bus_message_close_container(m);
    sd_bus_message_close_container(m);
    sd_bus_message_close_container(m);   // a{sv}
    sd_bus_message_close_container(m);   // e sa{sv}
    sd_bus_message_close_container(m);   // a{sa{sv}}
    rc = sd_bus_send(bus, m, nullptr);
    sd_bus_message_unref(m);
    return rc;
}

int main() {
    sd_bus* bus = nullptr;
    int rc = sd_bus_default_user(&bus);
    if (rc < 0) {
        fprintf(stderr, "btmock: no session bus (%s)\n", strerror(-rc));
        return 1;
    }
    rc = sd_bus_request_name(bus, "org.bluez", 0);
    if (rc < 0) {
        fprintf(stderr, "btmock: cannot own org.bluez (%s)\n", strerror(-rc));
        return 1;
    }
    sd_bus_add_object_vtable(bus, nullptr, kAdapterPath, "org.bluez.Adapter1",
                             kAdapterVtable, nullptr);
    sd_bus_add_object_vtable(bus, nullptr, kDev1, "org.bluez.Device1",
                             kDeviceVtable, nullptr);
    sd_bus_add_object_vtable(bus, nullptr, kDev2, "org.bluez.Device1",
                             kDeviceVtable, nullptr);
    sd_bus_add_object_vtable(bus, nullptr, kDev3, "org.bluez.Device1",
                             kDeviceVtable, nullptr);
    sd_bus_add_object_manager(bus, nullptr, "/org/bluez");

    /* seed the object manager with both devices */
    emit_device(bus, &kDevices[0], kDev1);
    emit_device(bus, &kDevices[1], kDev2);

    printf("btmock: org.bluez on the bus (adapter %s)\n", kAdapterPath);
    for (int iter = 0; ; iter++) {
        if (iter % 5 == 4) {
            /* RSSI wobble on the paired device, like an inquiring scan */
            gRssi = -42 + (iter / 5) % 5;
            sd_bus_emit_properties_changed_strv(bus, kDev1,
                                                "org.bluez.Device1", nullptr);
        }
        sd_bus_process(bus, nullptr);
        usleep(100 * 1000);
    }
    return 0;
}
