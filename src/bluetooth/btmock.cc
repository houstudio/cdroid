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

/* Hand-rolled PropertiesChanged(bool) — sd_bus_emit_properties_changed
 * fails (EPIPE/EDOM) on this sd-bus for vtable-backed values, while the
 * raw signal form delivers. */
static int emit_prop_bool(sd_bus* bus, const char* path, const char* iface,
                          const char* name, bool value) {
    sd_bus_message* sig = nullptr;
    int rc = sd_bus_message_new_signal(bus, &sig, path,
                "org.freedesktop.DBus.Properties", "PropertiesChanged");
    if (rc >= 0) rc = sd_bus_message_append(sig, "s", iface);
    if (rc >= 0) rc = sd_bus_message_open_container(sig, 'a', "{sv}");
    if (rc >= 0) rc = sd_bus_message_open_container(sig, 'e', "sv");
    if (rc >= 0) rc = sd_bus_message_append(sig, "s", name);
    if (rc >= 0) rc = sd_bus_message_open_container(sig, 'v', "b");
    if (rc >= 0) rc = sd_bus_message_append(sig, "b", value ? 1 : 0);
    if (rc >= 0) rc = sd_bus_message_close_container(sig);
    if (rc >= 0) rc = sd_bus_message_close_container(sig);
    if (rc >= 0) rc = sd_bus_message_close_container(sig);
    if (rc >= 0) rc = sd_bus_send(bus, sig, nullptr);
    sd_bus_message_unref(sig);
    return rc;
}

static const char* kAdapterPath = "/org/bluez/hci0";
static const char* kSvcPath = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service0010";
static const char* kCharPath = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service0010/char0011";
static const char* kSvcUuid = "0000ffe0-0000-1000-8000-00805f9b34fb";
static const char* kCharUuid = "0000ffe1-0000-1000-8000-00805f9b34fb";
static uint8_t gCharValue[] = { 'h', 'e', 'l', 'l', 'o' };
static const int gCharValueLen = 5;
static const char* kDev1 = "/org/bluez/hci0/dev_11_22_33_44_55_66";
static const char* kDev2 = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF";
static const char* kDev3 = "/org/bluez/hci0/dev_77_88_99_00_11_22";

static bool gPowered = true;
static bool gDiscovering = false;
static int gRssi = -42;

/* --- adapter property getters ------------------------------------------ */
static int get_adapter_powered(sd_bus*, const char*, const char*, const char*,
                               sd_bus_message* reply, void*, sd_bus_error*) {
    return sd_bus_message_append(reply, "b", gPowered ? 1 : 0);   /* b wants int */
}
static int set_adapter_powered(sd_bus* bus, const char* path,
                               const char* interface, const char* property,
                               sd_bus_message* m, void* userdata,
                               sd_bus_error* err) {
    int v = 0;
    if (sd_bus_message_read(m, "b", &v) < 0) return -EINVAL;   /* value arrives unwrapped */
    gPowered = v != 0;
    printf("[btmock] Powered -> %s\n", gPowered ? "on" : "off");
    emit_prop_bool(bus, path, "org.bluez.Adapter1", "Powered", gPowered);
    return 0;
}
static int get_adapter_discovering(sd_bus*, const char*, const char*,
                                   const char*, sd_bus_message* reply, void*,
                                   sd_bus_error*) {
    return sd_bus_message_append(reply, "b", gDiscovering ? 1 : 0);
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
static int set_discovery_filter(sd_bus_message* m, void*, sd_bus_error*) {
    sd_bus_message_skip(m, "a{sv}");
    return sd_bus_reply_method_return(m, "");
}
static int start_discovery(sd_bus_message* m, void*, sd_bus_error*) {
    sd_bus* bus = sd_bus_message_get_bus(m);
    gDiscovering = true;
    sd_bus_reply_method_return(m, "");
    /* the Discovering announcement goes out from the main loop — emitting
     * from inside the dispatch stack returned EPIPE on this sd-bus */
    /* a new device appearing: what a real scan delivers */
    const int rc = sd_bus_emit_object_added(bus, kDev3);
    return rc < 0 ? rc : 0;
}
static int stop_discovery(sd_bus_message* m, void*, sd_bus_error*) {
    sd_bus* bus = sd_bus_message_get_bus(m);
    gDiscovering = false;
    sd_bus_reply_method_return(m, "");
    return 0;
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

static int device_connect(sd_bus_message* m, void*, sd_bus_error*) {
    sd_bus* bus = sd_bus_message_get_bus(m);
    const char* path = sd_bus_message_get_path(m);
    sd_bus_reply_method_return(m, "");
    printf("[btmock] Connect %s\n", path);
    return emit_prop_bool(bus, path, "org.bluez.Device1", "Connected", true);
}
static int device_disconnect(sd_bus_message* m, void*, sd_bus_error*) {
    sd_bus* bus = sd_bus_message_get_bus(m);
    const char* path = sd_bus_message_get_path(m);
    sd_bus_reply_method_return(m, "");
    printf("[btmock] Disconnect %s\n", path);
    return emit_prop_bool(bus, path, "org.bluez.Device1", "Connected", false);
}
/* Agent manager: remember who registered an agent, so Pair can ask it. */
static char* gAgentOwner = nullptr;   /* client's unique bus name */
static char* gAgentPath = nullptr;

static int agentmgr_register(sd_bus_message* m, void*, sd_bus_error*) {
    const char* path = nullptr, *cap = nullptr;
    sd_bus_message_read(m, "os", &path, &cap);
    const char* sender = sd_bus_message_get_sender(m);
    free(gAgentOwner); free(gAgentPath);
    gAgentOwner = sender ? strdup(sender) : nullptr;
    gAgentPath = path ? strdup(path) : nullptr;
    printf("[btmock] RegisterAgent %s (%s) by %s\n", path, cap, sender);
    return sd_bus_reply_method_return(m, "");
}
static int agentmgr_default(sd_bus_message* m, void*, sd_bus_error*) {
    return sd_bus_reply_method_return(m, "");
}
static const sd_bus_vtable kAgentMgrVtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("RegisterAgent", "os", NULL, agentmgr_register, 0),
    SD_BUS_METHOD("RequestDefaultAgent", "o", NULL, agentmgr_default, 0),
    SD_BUS_VTABLE_END,
};

static int pair_device(sd_bus_message* m, void*, sd_bus_error*) {
    sd_bus* bus = sd_bus_message_get_bus(m);
    const char* path = sd_bus_message_get_path(m);
    printf("[btmock] Pair %s\n", path);
    /* real stacks consult the registered agent: ask for a PIN and wait. */
    if (gAgentOwner && gAgentPath) {
        sd_bus_error err = SD_BUS_ERROR_NULL;
        sd_bus_message* reply = nullptr;
        const int rc = sd_bus_call_method(bus, gAgentOwner, gAgentPath,
                "org.bluez.Agent1", "RequestPinCode", &err, &reply,
                "o", path);
        if (rc >= 0) {
            const char* pin = nullptr;
            sd_bus_message_read(reply, "s", &pin);
            printf("[btmock] agent supplied PIN '%s'\n", pin ? pin : "?");
            sd_bus_message_unref(reply);
        } else {
            printf("[btmock] agent declined (%s)\n",
                   err.message ? err.message : strerror(-rc));
            const int rrc = sd_bus_reply_method_error(m, &err);
            sd_bus_error_free(&err);
            return rrc;
        }
        sd_bus_error_free(&err);
    }
    sd_bus_reply_method_return(m, "");
    /* bond completed: flip the Paired property and announce it */
    gDev2Paired = true;
    return emit_prop_bool(bus, path, "org.bluez.Device1", "Paired", true);
}

/* --- GATT service/characteristic ---------------------------------------- */
static int get_svc_uuid(sd_bus*, const char*, const char*, const char*,
                        sd_bus_message* reply, void*, sd_bus_error*) {
    return sd_bus_message_append(reply, "s", kSvcUuid);
}
static int get_svc_device(sd_bus*, const char*, const char*, const char*,
                          sd_bus_message* reply, void*, sd_bus_error*) {
    return sd_bus_message_append(reply, "o", kDev2);
}
static int get_svc_primary(sd_bus*, const char*, const char*, const char*,
                           sd_bus_message* reply, void*, sd_bus_error*) {
    return sd_bus_message_append(reply, "b", 1);
}
static int get_char_uuid(sd_bus*, const char*, const char*, const char*,
                         sd_bus_message* reply, void*, sd_bus_error*) {
    return sd_bus_message_append(reply, "s", kCharUuid);
}
static int get_char_service(sd_bus*, const char*, const char*, const char*,
                            sd_bus_message* reply, void*, sd_bus_error*) {
    return sd_bus_message_append(reply, "o", kSvcPath);
}
static int get_char_flags(sd_bus*, const char*, const char*, const char*,
                          sd_bus_message* reply, void*, sd_bus_error*) {
    return sd_bus_message_append_strv(reply, (char**)(const char* const[]){
            (char*)"read", (char*)"write", (char*)"notify", nullptr});
}
static int get_char_value(sd_bus*, const char*, const char*, const char*,
                          sd_bus_message* reply, void*, sd_bus_error*) {
    return sd_bus_message_append_array(reply, 'y', gCharValue, gCharValueLen);
}
static bool gNotifying = false;
static int get_char_notifying(sd_bus*, const char*, const char*, const char*,
                              sd_bus_message* reply, void*, sd_bus_error*) {
    return sd_bus_message_append(reply, "b", gNotifying ? 1 : 0);
}

static int char_read(sd_bus_message* m, void*, sd_bus_error*) {
    sd_bus_message* reply = nullptr;
    sd_bus_message_new_method_return(m, &reply);
    sd_bus_message_append_array(reply, 'y', gCharValue, gCharValueLen);
    printf("[btmock] ReadValue -> 'hello'\n");
    return sd_bus_send(NULL, reply, NULL) < 0 ? 0 : sd_bus_message_unref(reply), 0;
}
static int char_write(sd_bus_message* m, void*, sd_bus_error*) {
    const uint8_t* data = nullptr;
    size_t len = 0;
    sd_bus_message_read_array(m, 'y', (const void**)&data, &len);
    printf("[btmock] WriteValue %zu bytes\n", len);
    sd_bus_message_skip(m, "a{sv}");
    return sd_bus_reply_method_return(m, "");
}
static int char_notify(sd_bus_message* m, void*, sd_bus_error*) {
    const bool start = strcmp(sd_bus_message_get_member(m), "StartNotify") == 0;
    gNotifying = start;
    printf("[btmock] %s\n", start ? "StartNotify" : "StopNotify");
    return sd_bus_reply_method_return(m, "");
}

static const sd_bus_vtable kServiceVtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_PROPERTY("UUID", "s", get_svc_uuid, 0, 0),
    SD_BUS_PROPERTY("Primary", "b", get_svc_primary, 0, 0),
    SD_BUS_PROPERTY("Device", "o", get_svc_device, 0, 0),
    SD_BUS_VTABLE_END,
};
static const sd_bus_vtable kCharVtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_PROPERTY("UUID", "s", get_char_uuid, 0, 0),
    SD_BUS_PROPERTY("Service", "o", get_char_service, 0, 0),
    SD_BUS_PROPERTY("Flags", "as", get_char_flags, 0, 0),
    SD_BUS_PROPERTY("Value", "ay", get_char_value, 0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("Notifying", "b", get_char_notifying, 0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_METHOD("ReadValue", "a{sv}", "ay", char_read, 0),
    SD_BUS_METHOD("WriteValue", "aya{sv}", NULL, char_write, 0),
    SD_BUS_METHOD("StartNotify", NULL, NULL, char_notify, 0),
    SD_BUS_METHOD("StopNotify", NULL, NULL, char_notify, 0),
    SD_BUS_VTABLE_END,
};

/* --- vtables --------------------------------------------------------------- */
static const sd_bus_vtable kAdapterVtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_WRITABLE_PROPERTY("Powered", "b", get_adapter_powered,
                             set_adapter_powered, 0, 0),
    SD_BUS_PROPERTY("Discovering", "b", get_adapter_discovering, 0, 0),
    SD_BUS_PROPERTY("Alias", "s", get_adapter_alias, 0, 0),
    SD_BUS_PROPERTY("Address", "s", get_adapter_address, 0, 0),
    SD_BUS_METHOD("SetDiscoveryFilter", "a{sv}", NULL, set_discovery_filter, 0),
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
    SD_BUS_METHOD("Connect", NULL, NULL, device_connect, 0),
    SD_BUS_METHOD("Disconnect", NULL, NULL, device_disconnect, 0),
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
    sd_bus_add_object_vtable(bus, nullptr, kSvcPath, "org.bluez.GattService1",
                             kServiceVtable, nullptr);
    sd_bus_add_object_vtable(bus, nullptr, kCharPath, "org.bluez.GattCharacteristic1",
                             kCharVtable, nullptr);
    sd_bus_add_object_vtable(bus, nullptr, "/org/bluez",
                             "org.bluez.AgentManager1",
                             kAgentMgrVtable, nullptr);
    sd_bus_add_object_manager(bus, nullptr, "/");   /* real BlueZ registers at the root */

    /* seed the object manager with both devices */
    emit_device(bus, &kDevices[0], kDev1);
    emit_device(bus, &kDevices[1], kDev2);

    printf("btmock: org.bluez on the bus (adapter %s)\n", kAdapterPath);
    bool announcedDiscovering = false;
    for (int iter = 0; ; iter++) {
        if (announcedDiscovering != gDiscovering) {
            announcedDiscovering = gDiscovering;
            emit_prop_bool(bus, kAdapterPath, "org.bluez.Adapter1",
                           "Discovering", gDiscovering);
        }
        if (iter % 5 == 4) {
            /* RSSI wobble on the paired device, like an inquiring scan */
            gRssi = -42 + (iter / 5) % 5;
            emit_prop_bool(bus, kDev1, "org.bluez.Device1", "RSSI",
                           gRssi & 1);   /* placeholder flip: real RSSI is int16 */
        }
        if (gNotifying && iter % 20 == 19) {
            /* notification: bump the first value byte and announce */
            gCharValue[0] = 'a' + (gCharValue[0] - 'a' + 1) % 26;
            sd_bus_message* sig = nullptr;
            if (sd_bus_message_new_signal(bus, &sig, kCharPath,
                        "org.freedesktop.DBus.Properties",
                        "PropertiesChanged") >= 0
             && sd_bus_message_append(sig, "s", "org.bluez.GattCharacteristic1") >= 0
             && sd_bus_message_open_container(sig, 'a', "{sv}") >= 0
             && sd_bus_message_open_container(sig, 'e', "sv") >= 0
             && sd_bus_message_append(sig, "s", "Value") >= 0
             && sd_bus_message_open_container(sig, 'v', "ay") >= 0
             && sd_bus_message_append_array(sig, 'y', gCharValue,
                                             (size_t)gCharValueLen) >= 0
             && sd_bus_message_close_container(sig) >= 0
             && sd_bus_message_close_container(sig) >= 0
             && sd_bus_message_close_container(sig) >= 0) {
                sd_bus_send(bus, sig, nullptr);
            }
            sd_bus_message_unref(sig);
        }
        sd_bus_process(bus, nullptr);
        usleep(100 * 1000);
    }
    return 0;
}
