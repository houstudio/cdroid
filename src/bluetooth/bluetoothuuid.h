#ifndef __CDROID_BLUETOOTH_UUID_H__
#define __CDROID_BLUETOOTH_UUID_H__

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace cdroid {

/**
 * Port of android.bluetooth.BluetoothUuid (android-36): the well-known
 * service UUIDs with their 128-bit expansions over the Bluetooth BASE
 * UUID, plus the java.util.UUID analog as a plain msb/lsb pair with the
 * canonical toString()/fromString(). Member names and values follow the
 * android-36 source exactly; SerialPort() is the one deliberate
 * addition (SPP has no android-36 constant — the RFCOMM data plane
 * still needs the conventional service id).
 */
class BluetoothUuid {
public:
    /* The 128-bit value, big-endian as the string form reads. */
    uint64_t msb = 0;
    uint64_t lsb = 0;

    BluetoothUuid() = default;
    BluetoothUuid(uint64_t msb_, uint64_t lsb_) : msb(msb_), lsb(lsb_) {}

    /* Build from the canonical 8-4-4-4-12 string form. */
    static BluetoothUuid fromString(const std::string& s) {
        uint64_t hi = 0, lo = 0;
        int n = 0;
        for (char c : s) {
            if (c == '-') continue;
            int v;
            if (c >= '0' && c <= '9') v = c - '0';
            else if (c >= 'a' && c <= 'f') v = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') v = c - 'A' + 10;
            else break;
            if (n < 16) hi = (hi << 4) | (uint64_t)v;
            else if (n < 32) lo = (lo << 4) | (uint64_t)v;
            else break;
            n++;
        }
        return BluetoothUuid(hi, lo);
    }

    /* Expand a 16-bit well-known UUID to its 128-bit form
     * (uuid * 2^96 + BASE_UUID, BluetoothUuid.java's arithmetic). */
    static BluetoothUuid fromShortUuid(uint16_t u16) {
        return BluetoothUuid(0x0000000000001000ULL | ((uint64_t)u16 << 32),
                             0x800000805F9B34FBULL);
    }

    /* The 16-byte big-endian wire form (msb bytes first — the java
     * getUuidBytes analog); the SDP/L2CAP payloads use this. */
    std::vector<uint8_t> toBytes() const {
        std::vector<uint8_t> out;
        out.reserve(16);
        for (int i = 15; i >= 0; i--)
            out.push_back((uint8_t)((i < 8 ? lsb : msb) >> ((i % 8) * 8)));
        return out;
    }

    std::string toString() const {
        char buf[40];
        snprintf(buf, sizeof(buf),
                 "%08X-%04X-%04X-%04X-%012llX",
                 (unsigned)(msb >> 32), (unsigned)((msb >> 16) & 0xFFFF),
                 (unsigned)(msb & 0xFFFF), (unsigned)(lsb >> 48),
                 (unsigned long long)(lsb & 0xFFFFFFFFFFFFULL));
        return std::string(buf);
    }

    bool operator==(const BluetoothUuid& o) const {
        return msb == o.msb && lsb == o.lsb;
    }
    bool operator!=(const BluetoothUuid& o) const { return !(*this == o); }

    /* --- well-known UUIDs (android-36 BluetoothUuid members) ---------- */
    static BluetoothUuid A2DP_SINK()       { return fromShortUuid(0x110B); }
    static BluetoothUuid A2DP_SOURCE()     { return fromShortUuid(0x110A); }
    static BluetoothUuid ADV_AUDIO_DIST()  { return fromShortUuid(0x110D); }
    static BluetoothUuid HSP()             { return fromShortUuid(0x1108); }
    static BluetoothUuid HSP_AG()          { return fromShortUuid(0x1112); }
    static BluetoothUuid HFP()             { return fromShortUuid(0x111E); }
    static BluetoothUuid HFP_AG()          { return fromShortUuid(0x111F); }
    static BluetoothUuid AVRCP()           { return fromShortUuid(0x110E); }
    static BluetoothUuid AVRCP_CONTROLLER(){ return fromShortUuid(0x110F); }
    static BluetoothUuid AVRCP_TARGET()    { return fromShortUuid(0x110C); }
    static BluetoothUuid OBEX_OBJECT_PUSH(){ return fromShortUuid(0x1105); }
    static BluetoothUuid HID()             { return fromShortUuid(0x1124); }
    static BluetoothUuid HOGP()            { return fromShortUuid(0x1812); }
    static BluetoothUuid PANU()            { return fromShortUuid(0x1115); }
    static BluetoothUuid NAP()             { return fromShortUuid(0x1116); }
    static BluetoothUuid BNEP()            { return fromShortUuid(0x000F); }
    static BluetoothUuid PBAP_PCE()        { return fromShortUuid(0x112E); }
    static BluetoothUuid PBAP_PSE()        { return fromShortUuid(0x112F); }
    static BluetoothUuid MAP()             { return fromShortUuid(0x1134); }
    static BluetoothUuid MNS()             { return fromShortUuid(0x1133); }
    static BluetoothUuid MAS()             { return fromShortUuid(0x1132); }
    static BluetoothUuid SAP()             { return fromShortUuid(0x112D); }
    /* Hearing Aid (LE, 128-bit assigned id — not a 16-bit shorthand). */
    static BluetoothUuid HEARING_AID() {
        return fromString("0000FDF0-0000-1000-8000-00805F9B34FB");
    }
    /* Hearing Access Service. */
    static BluetoothUuid HAS()             { return fromShortUuid(0x1854); }
    static BluetoothUuid MFI_HAS() {
        return fromString("7D74F4BD-C74A-4431-862C-CCE884371592");
    }
    /* The base UUID every 16-bit shorthand expands with. */
    static BluetoothUuid BASE_UUID() {
        return fromString("00000000-0000-1000-8000-00805F9B34FB");
    }
    /* CDROID addition (no android-36 constant): Serial Port Profile —
     * the conventional RFCOMM data service the socket plane targets. */
    static BluetoothUuid SerialPort()     { return fromShortUuid(0x1101); }
};

} // namespace cdroid

#endif /* __CDROID_BLUETOOTH_UUID_H__ */
