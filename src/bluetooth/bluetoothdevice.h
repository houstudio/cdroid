#ifndef __CDROID_BLUETOOTH_DEVICE_H__
#define __CDROID_BLUETOOTH_DEVICE_H__

#include <string>

#include <bluetoothclass.h>
#include <bluetoothuuid.h>

namespace cdroid {

class BluetoothAdapter;
class BluetoothGatt;
class BluetoothGattCallback;
class BluetoothSocket;

/**
 * Port of android.bluetooth.BluetoothDevice (android-36), module phase.
 * Like the Java original this is a remote-device HANDLE: the address is
 * the identity, every property getter resolves through the stack cache
 * at call time (the address may not have been seen yet — getters then
 * return the "unknown" defaults AOSP returns for an unbonded,
 * undiscovered address).
 *
 * Instances are values: copyable, equality/hash on the address, exactly
 * the parcelable-handle semantics of the Java class.
 */
class BluetoothDevice {
public:
    /* --- bond state -------------------------------------------------- */
    static constexpr int BOND_NONE = 10;
    static constexpr int BOND_BONDING = 11;
    static constexpr int BOND_BONDED = 12;

    /* --- pairing variants (EXTRA_PAIRING_VARIANT values) ---------------- */
    static constexpr int PAIRING_VARIANT_PIN = 0;
    static constexpr int PAIRING_VARIANT_PASSKEY = 1;
    static constexpr int PAIRING_VARIANT_PASSKEY_CONFIRMATION = 2;
    static constexpr int PAIRING_VARIANT_CONSENT = 3;
    static constexpr int PAIRING_VARIANT_DISPLAY_PASSKEY = 4;
    static constexpr int PAIRING_VARIANT_DISPLAY_PIN = 5;
    static constexpr int PAIRING_VARIANT_OOB_CONSENT = 6;
    static constexpr int PAIRING_VARIANT_PIN_16_DIGITS = 7;

    /* --- device type (getType) ---------------------------------------- */
    static constexpr int DEVICE_TYPE_UNKNOWN = 0;
    static constexpr int DEVICE_TYPE_CLASSIC = 1;
    static constexpr int DEVICE_TYPE_LE = 2;
    static constexpr int DEVICE_TYPE_DUAL = 3;

    /* Broadcast intents, kept as constants for the future broadcast
     * system (same strings the Java side broadcasts). */
    static constexpr const char* ACTION_FOUND =
            "android.bluetooth.device.action.FOUND";
    static constexpr const char* ACTION_NAME_CHANGED =
            "android.bluetooth.device.action.NAME_CHANGED";
    static constexpr const char* ACTION_ALIAS_CHANGED =
            "android.bluetooth.device.action.ALIAS_CHANGED";
    static constexpr const char* ACTION_BOND_STATE_CHANGED =
            "android.bluetooth.device.action.BOND_STATE_CHANGED";
    static constexpr const char* ACTION_CLASS_CHANGED =
            "android.bluetooth.device.action.CLASS_CHANGED";

    /* Requires a context is an AOSP note only; here the address must be
     * non-empty and "AA:BB:CC:DD:EE:FF"-shaped (upper-cased like the
     * Java helper does). Invalid addresses yield an unusable instance,
     * matching the Java behavior of throwing — represented by an empty
     * address the adapter refuses to resolve. */
    BluetoothDevice() = default;
    explicit BluetoothDevice(const std::string& address);

    /* Returns the remote bluetooth hardware address. */
    std::string getAddress() const { return mAddress; }

    /* Get the friendly Bluetooth name (remote name, alias when set). */
    std::string getName() const;
    /* User-writable alias (persists across reboots on the BlueZ side). */
    std::string getAlias() const;
    bool setAlias(const std::string& alias);

    /* Get the bond state of the remote device. */
    int getBondState() const;
    /* Get the Bluetooth Class of Device (CoD) snapshot. */
    BluetoothClass getBluetoothClass() const;
    /* Get the device type: CLASSIC / LE / DUAL / UNKNOWN. */
    int getType() const;

    /* Start the bonding (pairing) process. Blocking on this module-phase
     * port: AOSP's call is fire-and-forget with the outcome arriving via
     * ACTION_BOND_STATE_CHANGED; here createBond() returns when
     * Device1.Pair completes (or fails) and the bond-state listeners
     * still fire from the property signals. */
    bool createBond();
    /* Remove bond (remote side keeps its link key — same caveat as AOSP). */
    bool removeBond();

    /* --- pairing answers (call from a pairing listener) -------------------- */
    /* Answer a pending PAIRING_VARIANT_PIN request. */
    bool setPin(const std::string& pin);
    /* Answer a PASSKEY_CONFIRMATION/CONSENT request. */
    bool setPairingConfirmation(bool confirm);
    /* Abort the pending request. */
    bool cancelPairingUserInput();

    bool operator==(const BluetoothDevice& other) const {
        return mAddress == other.mAddress;
    }
    bool operator!=(const BluetoothDevice& other) const {
        return mAddress != other.mAddress;
    }
    bool operator<(const BluetoothDevice& other) const {
        return mAddress < other.mAddress;
    }

    /* --- GATT client ----------------------------------------------------- */
    /* connectGatt analog (the context parameter is a no-op here). The
     * returned BluetoothGatt is caller-owned; close() it when done. */
    BluetoothGatt* connectGatt(bool autoConnect,
                               BluetoothGattCallback* callback) const;

    /* --- RFCOMM socket factories -------------------------------------- */

    /* Create an RFCOMM socket ready to connect to the given channel
     * (AOSP createRfcommSocket — the "expert" API that skips SDP). */
    BluetoothSocket* createRfcommSocket(int channel) const;
    /* Create an RFCOMM socket to the remote service named by the UUID.
     * The SDP lookup (UUID -> channel) needs the radio up: until the
     * resolver lands this returns a socket on SPP's channel 1 when the
     * UUID is SerialPort(), nullptr otherwise. */
    BluetoothSocket* createRfcommSocketToServiceRecord(
            const BluetoothUuid& uuid) const;
    /* Same, without authentication/encryption (AOSP insecure flavor). */
    BluetoothSocket* createInsecureRfcommSocketToServiceRecord(
            const BluetoothUuid& uuid) const;

    /* AOSP toString: the address in brackets. */
    std::string toString() const;

private:
    friend class BluetoothAdapter;
    /* The adapter mints devices and resolves properties. */
    std::string mAddress;
};

} // namespace cdroid

#endif /* __CDROID_BLUETOOTH_DEVICE_H__ */
