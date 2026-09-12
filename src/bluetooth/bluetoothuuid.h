#ifndef __CDROID_BLUETOOTH_UUID_H__
#define __CDROID_BLUETOOTH_UUID_H__

#include <cstdint>
#include <string>

namespace cdroid {

/**
 * Port of android.bluetooth.BluetoothUuid (android-36) constants — the
 * 16-bit well-known UUIDs expanded to full 128-bit form (0000XXXX-0000-
 * 1000-8000-00805F9B34FB) plus the java.util.UUID analog as a plain
 * 16-byte value with the canonical toString().
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

    /* Expand a 16-bit well-known UUID to its 128-bit form. */
    static BluetoothUuid fromShortUuid(uint16_t u16) {
        return BluetoothUuid(0x0000000000001000ULL | ((uint64_t)u16 << 32),
                             0x800000805F9B34FBULL);
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

    /* --- well-known service classes (BluetoothUuid.*) ------------------- */
    static BluetoothUuid AudioSink()      { return fromShortUuid(0x110B); }
    static BluetoothUuid AudioSource()    { return fromShortUuid(0x110A); }
    static BluetoothUuid AdvAudioDist()   { return fromShortUuid(0x110D); }
    static BluetoothUuid HSP()            { return fromShortUuid(0x1108); }
    static BluetoothUuid HSP_AG()         { return fromShortUuid(0x1112); }
    static BluetoothUuid Handsfree()      { return fromShortUuid(0x111E); }
    static BluetoothUuid Handsfree_AG()   { return fromShortUuid(0x111F); }
    static BluetoothUuid AvrcpTarget()    { return fromShortUuid(0x110C); }
    static BluetoothUuid AvrcpController(){ return fromShortUuid(0x110E); }
    static BluetoothUuid ObexObjectPush() { return fromShortUuid(0x1105); }
    static BluetoothUuid ObexFileTransfer()   { return fromShortUuid(0x1106); }
    static BluetoothUuid ObexSync()       { return fromShortUuid(0x1104); }
    static BluetoothUuid ObexPbapC()      { return fromShortUuid(0x112E); }
    static BluetoothUuid ObexMapMns()     { return fromShortUuid(0x1133); }
    static BluetoothUuid ObexMapMas()     { return fromShortUuid(0x1132); }
    static BluetoothUuid PANU()           { return fromShortUuid(0x1115); }
    static BluetoothUuid NAP()            { return fromShortUuid(0x1116); }
    static BluetoothUuid BIP()            { return fromShortUuid(0x111A); }
    static BluetoothUuid BIP_Responder()  { return fromShortUuid(0x111B); }
    static BluetoothUuid SAP()            { return fromShortUuid(0x111D); }
    static BluetoothUuid HearingAccess()  { return fromShortUuid(0x1100); }
    /* Serial Port Profile — the classic data service. */
    static BluetoothUuid SerialPort()     { return fromShortUuid(0x1101); }
    /* The base UUID every 16-bit shorthand expands with. */
    static BluetoothUuid BASE() {
        return fromString("00000000-0000-1000-8000-00805F9B34FB");
    }
};

} // namespace cdroid

#endif /* __CDROID_BLUETOOTH_UUID_H__ */
