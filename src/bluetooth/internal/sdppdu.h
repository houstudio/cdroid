#ifndef __CDROID_SDP_PDU_H__
#define __CDROID_SDP_PDU_H__

/*
 * SDP PDU assembly/parsing (Bluetooth Core Spec Vol 3 Part B):
 *   ServiceSearchAttributeRequest  (PDU id 0x06)
 *   ServiceSearchAttributeResponse (PDU id 0x07)
 * Pure byte-level functions — unit-testable without a radio.
 */
#include <cstdint>
#include <cstddef>
#include <vector>

namespace cdroid {
namespace sdp {

constexpr uint8_t PDU_SEARCH_ATTR_REQUEST = 0x06;
constexpr uint8_t PDU_SEARCH_ATTR_RESPONSE = 0x07;

/* Data element headers: type in the high 3 bits, size descriptor low 5. */
constexpr uint8_t TYPE_UUID16 = 0x19;   /* (0x03 << 5) | 1 */
constexpr uint8_t TYPE_UUID32 = 0x1A;
constexpr uint8_t TYPE_UUID128 = 0x1C;  /* (0x03 << 5) | 4 */
constexpr uint8_t TYPE_UINT8 = 0x08;
constexpr uint8_t TYPE_UINT16 = 0x09;
constexpr uint8_t TYPE_SEQ8 = 0x35;
constexpr uint8_t TYPE_SEQ16 = 0x36;

constexpr uint16_t ATTR_PROTOCOL_DESCRIPTOR_LIST = 0x0004;

/* Well-known protocol UUIDs (16-bit form). */
constexpr uint16_t UUID_L2CAP = 0x0100;
constexpr uint16_t UUID_RFCOMM = 0x0003;

/* Build a complete ServiceSearchAttributeRequest body (everything after
 * the PDU header): serviceSearchPattern { uuid128 }, attributeIDs {
 * 0x0004 }, plus the leading maxByteCount — the full body the socket
 * sends after the 5-byte PDU header. */
std::vector<uint8_t> buildSearchAttributeRequest(
        const std::vector<uint8_t>& uuid128, uint16_t maxByteCount,
        const std::vector<uint8_t>& continuationState);

/* Parse a ServiceSearchAttributeResponse body (everything after the
 * 5-byte PDU header, as delivered): returns the RFCOMM channel from the
 * first ProtocolDescriptorList that carries one, or -1. Feeding every
 * continuation fragment's body through this (stateless re-scan) is
 * sufficient for the single-service queries this client issues. */
int parseSearchAttributeResponse(const std::vector<uint8_t>& body);

/* ---- low-level cursor over data elements (shared by the parser) ---- */
struct ElementReader {
    const uint8_t* data;
    size_t len;
    size_t pos = 0;
    bool failed = false;

    explicit ElementReader(const uint8_t* d, size_t l) : data(d), len(l) {}
    /* Type byte + declared payload size; false at end or malformed. */
    bool nextHeader(uint8_t& type, size_t& size);
    /* Reads a UUID element (16/32/128-bit) into whichever fits. */
    bool readUuid(uint16_t& uuid16, std::vector<uint8_t>& uuid128);
    bool readUint(uint64_t& value, size_t bytes);
    /* Enter a sequence: reports its inner length, cursor at element 0. */
    bool enterSequence(size_t& innerLen);
};

} // namespace sdp
} // namespace cdroid

#endif /* __CDROID_SDP_PDU_H__ */
