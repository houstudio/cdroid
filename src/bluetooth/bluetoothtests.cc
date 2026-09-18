/*********************************************************************************
 * bluetoothtests — pure-logic suite for cdblue (the wifitests analog):
 * no radio, no bus. Covers the SDP PDU layer byte-for-byte and the
 * BluetoothUuid expansions.
 *********************************************************************************/
#include <cstdio>
#include <cstring>
#include <vector>

#include <bluetoothuuid.h>
#include <internal/sdppdu.h>
#include "internal/btuapi.h"

using cdroid::BluetoothUuid;
using cdroid::sdp::parseSearchAttributeResponse;
using cdroid::sdp::buildSearchAttributeRequest;

static int gFailures = 0;
#define CHECK(cond) do { if (!(cond)) { \
    printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #cond); gFailures++; } } while (0)

/* Append helpers building data elements by hand (independent of the
 * code under test). */
static void putSeq8(std::vector<uint8_t>& v, const std::vector<uint8_t>& inner) {
    v.push_back(0x35);
    v.push_back((uint8_t)inner.size());
    v.insert(v.end(), inner.begin(), inner.end());
}
static void putUuid16(std::vector<uint8_t>& v, uint16_t u) {
    v.push_back(0x19);
    v.push_back((uint8_t)(u >> 8));
    v.push_back((uint8_t)(u & 0xFF));
}
static void putUint8(std::vector<uint8_t>& v, uint8_t x) {
    v.push_back(0x08);
    v.push_back(x);
}
static void putUint16(std::vector<uint8_t>& v, uint16_t x) {
    v.push_back(0x09);
    v.push_back((uint8_t)(x >> 8));
    v.push_back((uint8_t)(x & 0xFF));
}

/* Build a full ServiceSearchAttributeResponse body for a service whose
 * ProtocolDescriptorList is L2CAP + RFCOMM(channel). */
static std::vector<uint8_t> makeResponse(uint8_t channel) {
    std::vector<uint8_t> pdl;              /* SEQ of protocol SEQs */
    std::vector<uint8_t> l2cap;
    putUuid16(l2cap, cdroid::sdp::UUID_L2CAP);
    putSeq8(pdl, l2cap);
    std::vector<uint8_t> rfcomm;
    putUuid16(rfcomm, cdroid::sdp::UUID_RFCOMM);
    putUint8(rfcomm, channel);
    putSeq8(pdl, rfcomm);

    std::vector<uint8_t> attribute;        /* UINT16 id + SEQ value */
    putUint16(attribute, cdroid::sdp::ATTR_PROTOCOL_DESCRIPTOR_LIST);
    putSeq8(attribute, pdl);

    std::vector<uint8_t> attrLists;
    putSeq8(attrLists, attribute);

    std::vector<uint8_t> body;             /* byteCount + lists + cont */
    body.push_back((uint8_t)(attrLists.size() >> 8));
    body.push_back((uint8_t)(attrLists.size() & 0xFF));
    body.insert(body.end(), attrLists.begin(), attrLists.end());
    body.push_back(0);
    return body;
}

static void testBdaddrByteOrder() {
    /* bluez str2ba: first pair -> b[5]; round-trips through ba2str */
    bdaddr_t ba;
    str2ba("00:AA:01:01:00:00", &ba);
    CHECK(ba.b[0] == 0x00 && ba.b[1] == 0x00 && ba.b[2] == 0x01
          && ba.b[3] == 0x01 && ba.b[4] == 0xAA && ba.b[5] == 0x00);
    char out[18] = {0};
    ba2str(&ba, out, sizeof(out));
    CHECK(std::string(out) == "00:AA:01:01:00:00");
    str2ba("02:00:00:00:01:00", &ba);
    CHECK(ba.b[0] == 0x00 && ba.b[1] == 0x01 && ba.b[5] == 0x02);
}

static void testUuidExpansion() {
    CHECK(BluetoothUuid::A2DP_SINK().toString()
          == "0000110B-0000-1000-8000-00805F9B34FB");
    CHECK(BluetoothUuid::SerialPort().toString()
          == "00001101-0000-1000-8000-00805F9B34FB");
    CHECK(BluetoothUuid::fromString("00001101-0000-1000-8000-00805F9B34FB")
          == BluetoothUuid::SerialPort());
    CHECK(BluetoothUuid::HAS().toString()
          == "00001854-0000-1000-8000-00805F9B34FB");
    CHECK(BluetoothUuid::HEARING_AID().toString()
          == "0000FDF0-0000-1000-8000-00805F9B34FB");
}

static void testRequestAssembly() {
    std::vector<uint8_t> uuid(16, 0xAB);
    const std::vector<uint8_t> req = buildSearchAttributeRequest(uuid, 0xFFFF, {});
    /* maxByteCount(2) + SEQ8{UUID128} (2+17) + SEQ8{UINT16} (2+3) + cont(1) */
    CHECK(req.size() == 2 + (2 + 17) + (2 + 3) + 1);
    CHECK(req[0] == 0xFF && req[1] == 0xFF);
    CHECK(req[2] == 0x35 && req[3] == 17);          /* pattern seq */
    CHECK(req[4] == 0x1C);                          /* UUID128 */
    CHECK(req[21] == 0x35 && req[22] == 3);         /* attr seq */
    CHECK(req[23] == 0x09 && req[24] == 0x00 && req[25] == 0x04);
    CHECK(req.back() == 0);
}

static void testResponseParsing() {
    for (uint8_t ch = 1; ch <= 5; ch++)
        CHECK(parseSearchAttributeResponse(makeResponse(ch)) == ch);
    /* truncated body */
    std::vector<uint8_t> bad = makeResponse(3);
    bad.resize(bad.size() - 4);
    CHECK(parseSearchAttributeResponse(bad) == -1);
    /* empty */
    CHECK(parseSearchAttributeResponse({}) == -1);
    /* PDL without an RFCOMM entry (L2CAP-only service) */
    {
        std::vector<uint8_t> pdl, l2;
        putUuid16(l2, cdroid::sdp::UUID_L2CAP);
        putSeq8(pdl, l2);
        std::vector<uint8_t> attribute;
        putUint16(attribute, cdroid::sdp::ATTR_PROTOCOL_DESCRIPTOR_LIST);
        putSeq8(attribute, pdl);
        std::vector<uint8_t> attrLists;
        putSeq8(attrLists, attribute);
        std::vector<uint8_t> body;
        body.push_back((uint8_t)(attrLists.size() >> 8));
        body.push_back((uint8_t)(attrLists.size() & 0xFF));
        body.insert(body.end(), attrLists.begin(), attrLists.end());
        body.push_back(0);
        CHECK(parseSearchAttributeResponse(body) == -1);
    }
}

int main() {
    testBdaddrByteOrder();
    testUuidExpansion();
    testRequestAssembly();
    testResponseParsing();
    printf(gFailures == 0 ? "ALL PASS\n" : "%d FAILURES\n", gFailures);
    return gFailures == 0 ? 0 : 1;
}
