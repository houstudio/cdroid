#ifndef __CDROID_BLUETOOTH_PAIRING_H__
#define __CDROID_BLUETOOTH_PAIRING_H__

#include <cstdint>
#include <string>

#include <bluetoothdevice.h>

namespace cdroid {

/**
 * Port of the pairing-request surface of android.bluetooth
 * (android-36): ACTION_PAIRING_REQUEST + BluetoothDevice.setPin /
 * setPairingConfirmation / cancelPairingUserInput, backed by a BlueZ
 * org.bluez.Agent1 registered on the shared client connection.
 *
 * Flow (AOSP shape): a pairing attempt raises the request on the
 * listener (the monitor thread — marshal for UI work); the application
 * answers with BluetoothDevice.setPin()/setPairingConfirmation() or
 * cancels, and the pending agent request replies to BlueZ.
 */
class BluetoothPairingListener {
public:
    virtual ~BluetoothPairingListener() = default;
    /* Pairing input required. pairingVariant is one of
     * BluetoothDevice::PAIRING_VARIANT_*; passkey carries the 6-digit
     * code for PASSKEY_CONFIRMATION (AOSP's EXTRA_PAIRING_KEY) and is
     * 0 otherwise. */
    virtual void onPairingRequest(const BluetoothDevice& device,
                                  int pairingVariant, uint32_t passkey) {}
    /* A 6-digit passkey should be shown for the user to compare /
     * type (PAIRING_VARIANT_DISPLAY_PASSKEY / _PIN). */
    virtual void onDisplayPasskey(const BluetoothDevice& device,
                                  uint32_t passkey, int pairedDuration) {}
    /* Pairing was cancelled by the remote / stack. */
    virtual void onPairingCancelled(const BluetoothDevice& device) {}
};

} // namespace cdroid

#endif /* __CDROID_BLUETOOTH_PAIRING_H__ */
