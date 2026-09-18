/**
 * SDP PDU assembly/parsing — Bluetooth Core Spec Vol 3 Part B byte
 * formats, kept radio-free for unit testing.
 *
 * Data element layout: 5-bit type (high), 3-bit size descriptor (low).
 *   type codes: 1 = unsigned int, 3 = UUID, 6 = sequence
 *   size descriptor: 0..4 = 1/2/4/8/16 bytes inline,
 *                    5/6/7 = length in 1/2/4 following bytes
 */
#include <cstring>

#include "sdppdu.h"

namespace cdroid {
namespace sdp {

namespace {

constexpr uint8_t TYPECODE_UINT = 1;
constexpr uint8_t TYPECODE_UUID = 3;
constexpr uint8_t TYPECODE_SEQ = 6;

bool elementSize(uint8_t desc, size_t& extraBytes, size_t& inlineLen) {
    switch (desc) {
    case 0: inlineLen = 1; extraBytes = 0; return true;
    case 1: inlineLen = 2; extraBytes = 0; return true;
    case 2: inlineLen = 4; extraBytes = 0; return true;
    case 3: inlineLen = 8; extraBytes = 0; return true;
    case 4: inlineLen = 16; extraBytes = 0; return true;
    case 5: inlineLen = 0; extraBytes = 1; return true;
    case 6: inlineLen = 0; extraBytes = 2; return true;
    case 7: inlineLen = 0; extraBytes = 4; return true;
    default: return false;
    }
}

} // namespace

bool ElementReader::nextHeader(uint8_t& type, size_t& size) {
    if (pos >= len) return false;
    const uint8_t head = data[pos++];
    type = head >> 3;
    size_t extra = 0, inlineLen = 0;
    if (!elementSize(head & 0x07, extra, inlineLen)) { failed = true; return false; }
    size = inlineLen;
    for (size_t i = 0; i < extra; i++) {
        if (pos >= len) { failed = true; return false; }
        size = (size << 8) | data[pos++];
    }
    return true;
}

bool ElementReader::readUuid(uint16_t& uuid16, std::vector<uint8_t>& uuid128) {
    uint8_t type = 0; size_t size = 0;
    if (!nextHeader(type, size) || type != TYPECODE_UUID) { failed = true; return false; }
    if (pos + size > len) { failed = true; return false; }
    if (size == 2) {
        uuid16 = (uint16_t)((data[pos] << 8) | data[pos + 1]);
        pos += 2;
        return true;
    }
    if (size == 4 || size == 16) {
        uuid16 = 0;
        uuid128.assign(data + pos, data + pos + size);
        pos += size;
        return true;
    }
    failed = true;
    return false;
}

bool ElementReader::readUint(uint64_t& value, size_t bytes) {
    if (pos + bytes > len) { failed = true; return false; }
    value = 0;
    for (size_t i = 0; i < bytes; i++) value = (value << 8) | data[pos++];
    return true;
}

bool ElementReader::enterSequence(size_t& innerLen) {
    uint8_t type = 0; size_t size = 0;
    if (!nextHeader(type, size) || type != TYPECODE_SEQ) { failed = true; return false; }
    if (pos + size > len) { failed = true; return false; }
    innerLen = size;
    return true;
}

std::vector<uint8_t> buildSearchAttributeRequest(
        const std::vector<uint8_t>& uuid128, uint16_t maxByteCount,
        const std::vector<uint8_t>& continuationState) {
    /* serviceSearchPattern = SEQ8 { UUID128(uuid) } */
    std::vector<uint8_t> pattern;
    pattern.push_back(TYPE_SEQ8);
    pattern.push_back(1 + 16);
    pattern.push_back(TYPE_UUID128);
    pattern.insert(pattern.end(), uuid128.begin(), uuid128.end());

    /* attributeIDList = SEQ8 { UINT16 0x0004 } */
    std::vector<uint8_t> attrIds;
    attrIds.push_back(TYPE_SEQ8);
    attrIds.push_back(3);
    attrIds.push_back(TYPE_UINT16);
    attrIds.push_back((uint8_t)(ATTR_PROTOCOL_DESCRIPTOR_LIST >> 8));
    attrIds.push_back((uint8_t)(ATTR_PROTOCOL_DESCRIPTOR_LIST & 0xFF));

    std::vector<uint8_t> body;
    body.push_back((uint8_t)(maxByteCount >> 8));
    body.push_back((uint8_t)(maxByteCount & 0xFF));
    body.insert(body.end(), pattern.begin(), pattern.end());
    body.insert(body.end(), attrIds.begin(), attrIds.end());
    if (continuationState.empty()) {
        body.push_back(0);
    } else {
        body.push_back((uint8_t)continuationState.size());
        body.insert(body.end(), continuationState.begin(),
                    continuationState.end());
    }
    return body;
}

namespace {

/* One ProtocolDescriptorList: SEQ { SEQ{ UUID, params... } ... } — find
 * the RFCOMM descriptor and return its channel parameter. */
int channelFromProtocolDescriptorList(const uint8_t* data, size_t len) {
    /* `data/len` are the sequence CONTENTS (the caller already consumed
     * the SEQ header through its own enterSequence) — walk the protocol
     * descriptors directly. */
    ElementReader list(data, len);
    const size_t listEnd = len;
    while (list.pos < listEnd) {
        size_t protoLen = 0;
        if (!list.enterSequence(protoLen)) return -1;
        const size_t protoEnd = list.pos + protoLen;
        uint16_t uuid16 = 0;
        std::vector<uint8_t> uuid128;
        if (!list.readUuid(uuid16, uuid128)) return -1;
        if (uuid16 == UUID_RFCOMM) {
            /* next element: UINT8 server channel */
            uint8_t type = 0; size_t size = 0;
            uint64_t value = 0;
            if (list.nextHeader(type, size) && type == TYPECODE_UINT
                    && size == 1 && list.readUint(value, 1))
                return (int)value;
            return -1;
        }
        list.pos = protoEnd;   /* skip this protocol's parameters */
    }
    return -1;
}

} // namespace

int parseSearchAttributeResponse(const std::vector<uint8_t>& body) {
    if (body.size() < 3) return -1;
    /* attributeListByteCount(2) attributeLists continuation(1+) */
    const size_t byteCount = ((size_t)body[0] << 8) | body[1];
    if (body.size() < 2 + byteCount) return -1;
    ElementReader r(body.data() + 2, byteCount);
    size_t outerLen = 0;
    if (!r.enterSequence(outerLen)) return -1;
    const size_t outerEnd = r.pos + outerLen;
    while (r.pos < outerEnd) {
        /* attribute = UINT16 id + value element */
        uint8_t type = 0; size_t size = 0;
        uint64_t attrId = 0;
        if (!r.nextHeader(type, size) || type != TYPECODE_UINT || size != 2
                || !r.readUint(attrId, 2))
            return -1;
        size_t valueLen = 0;
        if (!r.enterSequence(valueLen)) return -1;
        if (attrId == ATTR_PROTOCOL_DESCRIPTOR_LIST) {
            const int channel = channelFromProtocolDescriptorList(
                    r.data + r.pos, valueLen);
            if (channel > 0) return channel;
        }
        r.pos += valueLen;
    }
    return -1;
}

} // namespace sdp
} // namespace cdroid
