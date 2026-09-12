#ifndef __CDROID_BT_UAPI_H__
#define __CDROID_BT_UAPI_H__
/*
 * Vendored Linux Bluetooth kernel-UAPI definitions the data plane needs
 * (the AF_BLUETOOTH socket family). Only the RFCOMM subset is mirrored —
 * values are stable kernel ABI (include/uapi/linux/bluetooth.h and
 * rfcomm.h in the kernel tree); the system bluez headers are not a build
 * dependency so the module stays self-contained (the cdnet precedent:
 * it vendors wpa_ctrl the same way). Add L2CAP the day GATT lands.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#define AF_BLUETOOTH    31
#define PF_BLUETOOTH    AF_BLUETOOTH
#define BTPROTO_RFCOMM  3

/* sol Bluetooth socket levels/options (kernel include/net/bluetooth) */
#define SOL_BLUETOOTH   274
#define BT_SECURITY     4
#define BT_DEFER_SETUP  8

struct __attribute__((packed)) bdaddr_t {
    uint8_t b[6];
};

/* BT_SECURITY levels */
#define BT_SECURITY_SDP     0
#define BT_SECURITY_LOW     1
#define BT_SECURITY_MEDIUM  2
#define BT_SECURITY_HIGH    3

struct bt_security {
    uint8_t level;
    uint8_t key_size;
};

/* RFCOMM socket address (include/uapi/linux/rfcomm.h) */
struct sockaddr_rc {
    sa_family_t rc_family;
    bdaddr_t    rc_bdaddr;
    uint8_t     rc_channel;
};

/* str2ba/ba2str — the bluez-libs helpers, reimplemented. */
/* Byte order per bluez-libs str2ba: the string's FIRST pair lands in
 * b[5] (the address is stored little-endian, printed high-first).
 * Round-trips with ba2str; pinned by bluetoothtests. */
static inline void str2ba(const char* str, bdaddr_t* ba) {
    if (!ba) return;
    memset(ba, 0, sizeof(*ba));
    if (!str) return;
    unsigned int b[6];
    if (sscanf(str, "%02X:%02X:%02X:%02X:%02X:%02X",
               &b[5], &b[4], &b[3], &b[2], &b[1], &b[0]) == 6) {
        for (int i = 0; i < 6; i++) ba->b[i] = (uint8_t)b[i];
    }
}

static inline void ba2str(const bdaddr_t* ba, char* str, size_t len) {
    if (!str || len < 18) return;
    if (!ba) { str[0] = '\0'; return; }
    snprintf(str, len, "%02X:%02X:%02X:%02X:%02X:%02X",
             ba->b[5], ba->b[4], ba->b[3], ba->b[2], ba->b[1], ba->b[0]);
}

#endif /* __CDROID_BT_UAPI_H__ */
