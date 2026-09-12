#ifndef __CDROID_SDP_CLIENT_H__
#define __CDROID_SDP_CLIENT_H__

/*
 * SDP client — UUID -> RFCOMM channel resolution over an L2CAP socket to
 * the remote's PSM 0x0001 (Bluetooth Core Spec, Service Discovery
 * Protocol). This is the resolver behind
 * BluetoothDevice.createRfcommSocketToServiceRecord(UUID) — the step
 * AOSP's stack performs inside its socket connect path.
 *
 * Pure PDU assembly/parsing lives in sdp-pdu.h so it unit-tests without
 * a radio; the socket half (connect + request/retry on continuation)
 * needs a live peer.
 */
#include <stdint.h>
#include <string>
#include <vector>

namespace cdroid {

/* Resolve the RFCOMM server channel the remote advertises for the given
 * service UUID over SDP. Returns the channel (1..30) or -1 on failure
 * (no SDP server, no such service, malformed response). */
int sdpResolveRfcommChannel(const std::string& bdaddr,
                            const std::vector<uint8_t>& uuid128);

} // namespace cdroid

#endif /* __CDROID_SDP_CLIENT_H__ */
