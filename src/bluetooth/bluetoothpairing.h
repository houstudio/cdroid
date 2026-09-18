#ifndef __CDROID_BLUETOOTH_PAIRING_H__
#define __CDROID_BLUETOOTH_PAIRING_H__

#include <cstdint>
#include <functional>
#include <string>

#include <core/callbackbase.h>   /* EventSet listener base (header-only) */

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
 *
 * Listener shape: EventSet + std::function slots — identity (==) for
 * add/removePairingListener lives in EventSet; unset slots are no-ops.
 */
class BluetoothPairingListener : public EventSet {
public:
    /* Pairing input required. pairingVariant is one of
     * BluetoothDevice::PAIRING_VARIANT_*; passkey carries the 6-digit
     * code for PASSKEY_CONFIRMATION (AOSP's EXTRA_PAIRING_KEY) and is
     * 0 otherwise. */
    std::function<void(const BluetoothDevice&, int, uint32_t)> onPairingRequest;
    /* A 6-digit passkey should be shown for the user to compare /
     * type (PAIRING_VARIANT_DISPLAY_PASSKEY / _PIN). */
    std::function<void(const BluetoothDevice&, uint32_t, int)> onDisplayPasskey;
    /* Pairing was cancelled by the remote / stack. */
    std::function<void(const BluetoothDevice&)> onPairingCancelled;
};

} // namespace cdroid

#endif /* __CDROID_BLUETOOTH_PAIRING_H__ */
