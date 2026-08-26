// Standalone unit test for ResXMLTree — the binary-AXML pull parser (stage 2 of
// the aapt2 binary-resource runtime). Builds against the androidfw static lib:
//   make -C outX64-Debug androidfw_resxmltree_test && ./outX64-Debug/src/gui/androidfw/androidfw_resxmltree_test
//
// Fixture axml_fixture.h is the aapt2 output for res/layout/test.xml. Ground
// truth (event order, element names/lines, attribute resource-ids/types/values)
// comes from `aapt2 dump xmltree`; note aapt2 SORTS attributes by resource id,
// so lookups use indexOfAttribute(ns, name), not positional indexing.
#include "resourcetypes.h"
#include "axml_fixture.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace {

int g_failures = 0;
int g_checks = 0;

#define CHECK(cond)                                                       \
    do {                                                                  \
        ++g_checks;                                                       \
        if (!(cond)) {                                                    \
            ++g_failures;                                                 \
            std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__,  \
                         #cond);                                          \
        }                                                                 \
    } while (0)

#define CHECK_EQ(actual, expected)                                        \
    do {                                                                  \
        ++g_checks;                                                       \
        auto _a = (actual);                                               \
        auto _e = (expected);                                             \
        if (!(_a == _e)) {                                                \
            ++g_failures;                                                 \
            std::fprintf(stderr, "FAIL %s:%d: %s != %s (0x%x vs 0x%x)\n",  \
                         __FILE__, __LINE__, #actual, #expected,          \
                         (unsigned)_a, (unsigned)_e);                     \
        }                                                                 \
    } while (0)

using cdroid::ResXMLTree;
using cdroid::ResXMLParser;
using cdroid::Res_value;
using cdroid::ResStringPool;
using cdroid::status_t;

const char* kNsAndroid = "http://schemas.android.com/apk/res/android";
const char* kNsApp     = "http://schemas.android.com/apk/res-auto";

bool u16eq(const char16_t* s, size_t len, const char* utf8) {
    if (!s) return utf8 == nullptr;
    // Decode the UTF-8 expectation to UTF-16 (multi-byte + surrogate pairs), then
    // compare against the parser's char16_t output.
    std::u16string w;
    for (const unsigned char* p = (const unsigned char*)utf8; *p;) {
        unsigned int c;
        if ((*p & 0x80) == 0) { c = *p++; }
        else if ((*p & 0xE0) == 0xC0) { c = ((*p & 0x1F) << 6) | (p[1] & 0x3F); p += 2; }
        else if ((*p & 0xF0) == 0xE0) { c = ((*p & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F); p += 3; }
        else { c = ((*p & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F); p += 4; }
        if (c > 0xFFFF) {
            w.push_back((char16_t)(0xD800 + ((c - 0x10000) >> 10)));
            w.push_back((char16_t)(0xDC00 + ((c - 0x10000) & 0x3FF)));
        } else {
            w.push_back((char16_t)c);
        }
    }
    return len == w.size() && std::memcmp(s, w.data(), len * sizeof(char16_t)) == 0;
}

// Find an attribute on the current START_TAG by (namespace URI, name) and fetch
// its typed value. Returns false if not found.
bool attrValue(ResXMLTree& tree, const char* nsUri, const char* name, Res_value* out) {
    ssize_t idx = tree.indexOfAttribute(nsUri, name);
    if (idx < 0) return false;
    return tree.getAttributeValue((size_t)idx, out) == sizeof(Res_value);
}

// Expected dimension encoding for 14sp: unit SP(2), radix 23p0(0), mantissa 14.
const uint32_t k14sp = (Res_value::COMPLEX_UNIT_SP)
                     | (Res_value::COMPLEX_RADIX_23p0 << Res_value::COMPLEX_RADIX_SHIFT)
                     | (14u << Res_value::COMPLEX_MANTISSA_SHIFT);

void test_parse_and_walk() {
    ResXMLTree tree;
    CHECK_EQ(tree.setTo(kAXML, kAXMLLen), (status_t)cdroid::NO_ERROR);
    CHECK(tree.getError() == cdroid::NO_ERROR);
    CHECK(tree.getStrings().size() > 0);

    // next() from START_DOCUMENT yields the root event: START_NAMESPACE android.
    ResXMLParser::event_code_t ev = tree.next();
    CHECK_EQ(ev, ResXMLParser::START_NAMESPACE);
    {
        size_t ulen = 0;
        const char16_t* uri = tree.getNamespaceUri(&ulen);
        CHECK(u16eq(uri, ulen, kNsAndroid)); // android namespace is emitted first
    }

    ev = tree.next();
    CHECK_EQ(ev, ResXMLParser::START_NAMESPACE); // app namespace
    {
        size_t plen = 0, ulen = 0;
        const char16_t* prefix = tree.getNamespacePrefix(&plen);
        const char16_t* uri = tree.getNamespaceUri(&ulen);
        CHECK(u16eq(uri, ulen, kNsApp));
        CHECK(u16eq(prefix, plen, "app"));
    }

    // LinearLayout START_TAG.
    ev = tree.next();
    CHECK_EQ(ev, ResXMLParser::START_TAG);
    {
        size_t nlen = 0;
        const char16_t* name = tree.getElementName(&nlen);
        CHECK(u16eq(name, nlen, "LinearLayout"));
        CHECK_EQ(tree.getLineNumber(), (uint32_t)3);
        CHECK_EQ(tree.getAttributeCount(), (size_t)9);
    }

    // Attribute types/values (aapt2 sorted by resource id; lookup by name).
    Res_value v;
    CHECK(attrValue(tree, kNsAndroid, "layout_width", &v));
    CHECK_EQ(v.dataType, (int)Res_value::TYPE_INT_DEC);
    CHECK_EQ(v.data, (uint32_t)0xFFFFFFFEu); // wrap_content == -2

    CHECK(attrValue(tree, kNsAndroid, "layout_height", &v));
    CHECK_EQ(v.dataType, (int)Res_value::TYPE_INT_DEC);
    CHECK_EQ(v.data, (uint32_t)0xFFFFFFFEu);

    CHECK(attrValue(tree, kNsApp, "boolv", &v));
    CHECK_EQ(v.dataType, (int)Res_value::TYPE_INT_BOOLEAN);
    CHECK_EQ(v.data, (uint32_t)0xFFFFFFFFu);

    CHECK(attrValue(tree, kNsApp, "colorv", &v));
    CHECK_EQ(v.dataType, (int)Res_value::TYPE_INT_COLOR_RGB8);
    CHECK_EQ(v.data, (uint32_t)0xFFFF8800u);

    CHECK(attrValue(tree, kNsApp, "dimv", &v));
    CHECK_EQ(v.dataType, (int)Res_value::TYPE_DIMENSION);
    CHECK_EQ(v.data, k14sp);

    CHECK(attrValue(tree, kNsApp, "intdec", &v));
    CHECK_EQ(v.dataType, (int)Res_value::TYPE_INT_DEC);
    CHECK_EQ(v.data, (uint32_t)42u);

    CHECK(attrValue(tree, kNsApp, "inthex", &v));
    CHECK_EQ(v.dataType, (int)Res_value::TYPE_INT_HEX);
    CHECK_EQ(v.data, (uint32_t)0xFFu);

    CHECK(attrValue(tree, kNsApp, "refv", &v));
    CHECK_EQ(v.dataType, (int)Res_value::TYPE_REFERENCE);
    CHECK_EQ(v.data, (uint32_t)0x0106000cu);

    CHECK(attrValue(tree, kNsApp, "strv", &v));
    CHECK_EQ(v.dataType, (int)Res_value::TYPE_STRING);
    {
        ssize_t idx = tree.indexOfAttribute(kNsApp, "strv");
        size_t slen = 0;
        const char16_t* raw = tree.getAttributeStringValue((size_t)idx, &slen);
        CHECK(u16eq(raw, slen, "Hello \xe4\xb8\x96\xe7\x95\x8c")); // "Hello 世界"
    }

    // Resource-id map: framework layout_width resolves to a real android id.
    {
        ssize_t idx = tree.indexOfAttribute(kNsAndroid, "layout_width");
        CHECK(idx >= 0);
        CHECK_EQ(tree.getAttributeNameResID((size_t)idx), (uint32_t)0x010100f4u);
    }

    // TextView child.
    ev = tree.next();
    CHECK_EQ(ev, ResXMLParser::START_TAG);
    {
        size_t nlen = 0;
        const char16_t* name = tree.getElementName(&nlen);
        CHECK(u16eq(name, nlen, "TextView"));
        CHECK_EQ(tree.getLineNumber(), (uint32_t)14);
        CHECK_EQ(tree.getAttributeCount(), (size_t)3);
        CHECK(attrValue(tree, kNsApp, "intdec", &v));
        CHECK_EQ(v.data, (uint32_t)7u);
    }

    // END_TAG (TextView), END_TAG (LinearLayout).
    CHECK_EQ(tree.next(), ResXMLParser::END_TAG);
    CHECK_EQ(tree.next(), ResXMLParser::END_TAG);
    // END_NAMESPACE x2, then END_DOCUMENT.
    CHECK_EQ(tree.next(), ResXMLParser::END_NAMESPACE);
    CHECK_EQ(tree.next(), ResXMLParser::END_NAMESPACE);
    CHECK_EQ(tree.next(), ResXMLParser::END_DOCUMENT);
    // Past the end, next() stays at END_DOCUMENT.
    CHECK_EQ(tree.next(), ResXMLParser::END_DOCUMENT);
}

void test_restart_and_position() {
    ResXMLTree tree;
    CHECK_EQ(tree.setTo(kAXML, kAXMLLen), (status_t)cdroid::NO_ERROR);

    // Walk a couple of events, then snapshot + restart + restore.
    tree.next(); // START_NAMESPACE
    tree.next(); // START_NAMESPACE
    CHECK_EQ(tree.next(), ResXMLParser::START_TAG); // LinearLayout
    ResXMLParser::ResXMLPosition pos;
    tree.getPosition(&pos);

    tree.restart();
    CHECK_EQ(tree.getEventType(), ResXMLParser::START_DOCUMENT);
    tree.setPosition(pos);
    CHECK_EQ(tree.getEventType(), ResXMLParser::START_TAG);
    {
        size_t nlen = 0;
        const char16_t* name = tree.getElementName(&nlen);
        CHECK(u16eq(name, nlen, "LinearLayout"));
    }
}

void test_error_cases() {
    ResXMLTree tree;
    CHECK_EQ(tree.setTo(nullptr, 0), (status_t)cdroid::BAD_TYPE);
    CHECK_EQ(tree.setTo(kAXML, 16), (status_t)cdroid::BAD_TYPE); // truncated
    std::vector<uint8_t> garbage(128, 0xFF);
    CHECK_EQ(tree.setTo(garbage.data(), garbage.size()), (status_t)cdroid::BAD_TYPE);
}

void test_copy_data() {
    std::vector<uint8_t> owned(kAXML, kAXML + kAXMLLen);
    ResXMLTree tree;
    CHECK_EQ(tree.setTo(owned.data(), owned.size(), true), (status_t)cdroid::NO_ERROR);
    owned.assign(owned.size(), 0); // wipe source; tree has its own copy
    tree.next(); tree.next();      // skip namespaces
    CHECK_EQ(tree.next(), ResXMLParser::START_TAG);
    size_t nlen = 0;
    const char16_t* name = tree.getElementName(&nlen);
    CHECK(u16eq(name, nlen, "LinearLayout"));
}

} // namespace

int main() {
    test_parse_and_walk();
    test_restart_and_position();
    test_error_cases();
    test_copy_data();

    std::printf("ResXMLTree: %d checks, %d failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
