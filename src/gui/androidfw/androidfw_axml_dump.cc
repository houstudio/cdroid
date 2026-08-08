// Closed-loop demo / tool for the binary-AXML parser: loads a binary XML file and
// pretty-prints its tree (events, namespaces, elements, attributes with typed
// values) — analogous to `aapt2 dump xmltree`. Verifies the full ResXMLTree
// pipeline end to end against a known-good aapt2 fixture, and doubles as a handy
// inspector for any compiled AXML (e.g. an APK's AndroidManifest.xml).
//
//   make -C outX64-Debug androidfw_axml_dump
//   ./outX64-Debug/src/gui/androidfw/androidfw_axml_dump                 # fixture
//   ./outX64-Debug/src/gui/androidfw/androidfw_axml_dump AndroidManifest.xml  # any file
#include "resourcetypes.h"
#include "axml_fixture.h"

#include <porting/cdlog.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>

using cdroid::ResXMLTree;
using cdroid::ResXMLParser;
using cdroid::Res_value;

static void put_indent(int depth) { for (int i = 0; i < depth; ++i) std::fputs("  ", stdout); }

// Print a char16_t run as UTF-8.
static void print_u16(const char16_t* s, size_t len) {
    for (size_t i = 0; s && i < len; ++i) {
        uint32_t c = s[i];
        // Collapse a surrogate pair.
        if (c >= 0xD800 && c <= 0xDBFF && i + 1 < len && s[i + 1] >= 0xDC00 && s[i + 1] <= 0xDFFF) {
            c = 0x10000 + ((c - 0xD800) << 10) + (s[++i] - 0xDC00);
        }
        if (c < 0x80) std::fputc((char)c, stdout);
        else if (c < 0x800) { std::fputc((char)(0xC0 | (c >> 6)), stdout); std::fputc((char)(0x80 | (c & 0x3F)), stdout); }
        else if (c < 0x10000) { std::fputc((char)(0xE0 | (c >> 12)), stdout); std::fputc((char)(0x80 | ((c >> 6) & 0x3F)), stdout); std::fputc((char)(0x80 | (c & 0x3F)), stdout); }
        else { std::fputc((char)(0xF0 | (c >> 18)), stdout); std::fputc((char)(0x80 | ((c >> 12) & 0x3F)), stdout); std::fputc((char)(0x80 | ((c >> 6) & 0x3F)), stdout); std::fputc((char)(0x80 | (c & 0x3F)), stdout); }
    }
}

static const char* unit_name(int unit) {
    switch (unit) {
        case Res_value::COMPLEX_UNIT_PX: return "px";
        case Res_value::COMPLEX_UNIT_DIP: return "dp";
        case Res_value::COMPLEX_UNIT_SP: return "sp";
        case Res_value::COMPLEX_UNIT_PT: return "pt";
        case Res_value::COMPLEX_UNIT_IN: return "in";
        case Res_value::COMPLEX_UNIT_MM: return "mm";
        default: return "?";
    }
}

// Format a typed attribute value like aapt2 (color/int/dim/float/ref/string...).
static void print_typed_value(const Res_value& v, const char16_t* rawStr, size_t rawLen) {
    switch (v.dataType) {
        case Res_value::TYPE_STRING:
            std::fputs("\"", stdout); print_u16(rawStr, rawLen); std::fputs("\"", stdout);
            break;
        case Res_value::TYPE_INT_BOOLEAN:
            std::fputs(v.data != 0 ? "true" : "false", stdout);
            break;
        case Res_value::TYPE_INT_DEC:
            std::printf("%d", (int)v.data);
            break;
        case Res_value::TYPE_INT_HEX:
            std::printf("0x%08x", v.data);
            break;
        case Res_value::TYPE_INT_COLOR_ARGB8:
        case Res_value::TYPE_INT_COLOR_RGB8:
        case Res_value::TYPE_INT_COLOR_ARGB4:
        case Res_value::TYPE_INT_COLOR_RGB4: {
            int nib = (v.dataType == Res_value::TYPE_INT_COLOR_ARGB8 || v.dataType == Res_value::TYPE_INT_COLOR_RGB8) ? 8 : 4;
            std::printf("#%0*x", nib / 4, v.data);
            break;
        }
        case Res_value::TYPE_DIMENSION: {
            const uint32_t radix = (v.data >> Res_value::COMPLEX_RADIX_SHIFT) & Res_value::COMPLEX_RADIX_MASK;
            const uint32_t mantissa = (v.data >> Res_value::COMPLEX_MANTISSA_SHIFT) & Res_value::COMPLEX_MANTISSA_MASK;
            const uint32_t unit = (v.data >> Res_value::COMPLEX_UNIT_SHIFT) & Res_value::COMPLEX_UNIT_MASK;
            float mag;
            switch (radix) {
                case Res_value::COMPLEX_RADIX_23p0: mag = (float)(int32_t)mantissa; break;
                case Res_value::COMPLEX_RADIX_16p7: mag = mantissa * (1.0f / (1 << 7)); break;
                case Res_value::COMPLEX_RADIX_8p15: mag = mantissa * (1.0f / (1 << 15)); break;
                default: mag = mantissa * (1.0f / (1 << 23)); break;
            }
            std::printf("%f", mag);
            std::fputs(unit_name(unit), stdout);
            break;
        }
        case Res_value::TYPE_FLOAT: {
            float f; std::memcpy(&f, &v.data, sizeof(f));
            std::printf("%ffloat", f);
            break;
        }
        case Res_value::TYPE_REFERENCE:
        case Res_value::TYPE_ATTRIBUTE:
            std::printf("@0x%08x", v.data);
            break;
        default:
            std::printf("(type=0x%02x data=0x%08x)", v.dataType, v.data);
            break;
    }
    (void)rawStr; (void)rawLen;
}

int main(int argc, char** argv) {
    std::vector<uint8_t> data;
    if (argc >= 2) {
        std::ifstream in(argv[1], std::ios::binary);
        if (!in) { std::fprintf(stderr, "cannot open %s\n", argv[1]); return 1; }
        data.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    } else {
        data.assign(kAXML, kAXML + kAXMLLen);
        std::printf("# (no file argument: dumping built-in fixture res/layout/test.xml)\n");
    }

    ResXMLTree tree;
    cdroid::status_t err = tree.setTo(data.data(), data.size());
    if (err != cdroid::NO_ERROR) {
        std::fprintf(stderr, "ResXMLTree::setTo failed: %d\n", (int)err);
        return 1;
    }

    int depth = 0;
    ResXMLParser::event_code_t ev;
    while ((ev = tree.next()) != ResXMLParser::END_DOCUMENT) {
        if (ev == ResXMLParser::BAD_DOCUMENT) {
            std::fprintf(stderr, "\n** BAD_DOCUMENT: malformed XML **\n");
            break;
        }
        switch (ev) {
            case ResXMLParser::START_NAMESPACE: {
                size_t pl = 0, ul = 0;
                put_indent(depth);
                const char16_t* p = tree.getNamespacePrefix(&pl);
                const char16_t* u = tree.getNamespaceUri(&ul);
                std::fputs("N: ", stdout); print_u16(p, pl); std::fputs("=", stdout); print_u16(u, ul);
                std::printf(" (line=%u)\n", tree.getLineNumber());
                ++depth;
                break;
            }
            case ResXMLParser::END_NAMESPACE:
                --depth;
                break;
            case ResXMLParser::START_TAG: {
                size_t nl = 0;
                const char16_t* elemName = tree.getElementName(&nl); // two-step: avoid eval-order trap on nl
                put_indent(depth);
                std::fputs("E: ", stdout); print_u16(elemName, nl);
                std::printf(" (line=%u)\n", tree.getLineNumber());
                const size_t n = tree.getAttributeCount();
                for (size_t i = 0; i < n; ++i) {
                    size_t nsl = 0, anl = 0, rsl = 0;
                    const char16_t* ns = tree.getAttributeNamespace(i, &nsl);
                    const char16_t* an = tree.getAttributeName(i, &anl);
                    const char16_t* rs = tree.getAttributeStringValue(i, &rsl);
                    Res_value v;
                    tree.getAttributeValue(i, &v);
                    put_indent(depth + 1);
                    std::fputs("A: ", stdout);
                    if (ns) { print_u16(ns, nsl); std::fputc(':', stdout); }
                    print_u16(an, anl);
                    uint32_t rid = tree.getAttributeNameResID(i);
                    if (rid) std::printf("(0x%08x)", rid);
                    std::fputc('=', stdout);
                    print_typed_value(v, rs, rsl);
                    if (v.dataType == Res_value::TYPE_STRING && rs) {
                        std::fputs(" (Raw: \"", stdout); print_u16(rs, rsl); std::fputs("\")", stdout);
                    }
                    std::fputc('\n', stdout);
                }
                ++depth;
                break;
            }
            case ResXMLParser::END_TAG:
                --depth;
                break;
            case ResXMLParser::TEXT: {
                size_t tl = 0;
                const char16_t* txt = tree.getText(&tl); // two-step (see START_TAG)
                put_indent(depth);
                std::fputs("T: \"", stdout); print_u16(txt, tl); std::fputs("\"\n", stdout);
                break;
            }
            default:
                break;
        }
    }
    return 0;
}
