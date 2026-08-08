// Minimal TypedArray demo — proves a basic TypedArray runs on the CURRENT
// androidfw state (ResXMLTree only, no Theme/applyStyle needed), because aapt2
// already typed every attribute value.
//
// MiniTypedArray wraps a ResXMLTree positioned at a START_TAG: it looks up each
// attribute by (namespace, name) and exposes the Android TypedArray getters,
// each dispatching on Res_value.dataType exactly like the real one. It does NOT
// do style/theme inheritance or format validation (those need Theme +
// AttributeResolution, later stages).
//
//   make -C outX64-Debug androidfw_typedarray_demo
//   ./outX64-Debug/src/gui/androidfw/androidfw_typedarray_demo
#include "resourcetypes.h"
#include "axml_fixture.h"

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <string>

using cdroid::ResXMLTree;
using cdroid::ResXMLParser;
using cdroid::Res_value;
using cdroid::ResStringPool;

// Decode a TYPE_DIMENSION/TYPE_FRACTION complex to its floating magnitude.
static float complexToFloat(uint32_t data) {
    const uint32_t radix = (data >> Res_value::COMPLEX_RADIX_SHIFT) & Res_value::COMPLEX_RADIX_MASK;
    const uint32_t mantissa = (data >> Res_value::COMPLEX_MANTISSA_SHIFT) & Res_value::COMPLEX_MANTISSA_MASK;
    switch (radix) {
        case Res_value::COMPLEX_RADIX_23p0: return (float)(int32_t)mantissa;
        case Res_value::COMPLEX_RADIX_16p7: return mantissa * (1.0f / (1 << 7));
        case Res_value::COMPLEX_RADIX_8p15: return mantissa * (1.0f / (1 << 15));
        default:                            return mantissa * (1.0f / (1 << 23));
    }
}

static const char* dimUnit(uint32_t data) {
    switch ((data >> Res_value::COMPLEX_UNIT_SHIFT) & Res_value::COMPLEX_UNIT_MASK) {
        case Res_value::COMPLEX_UNIT_PX:  return "px";
        case Res_value::COMPLEX_UNIT_DIP: return "dp";
        case Res_value::COMPLEX_UNIT_SP:  return "sp";
        case Res_value::COMPLEX_UNIT_PT:  return "pt";
        case Res_value::COMPLEX_UNIT_IN:  return "in";
        case Res_value::COMPLEX_UNIT_MM:  return "mm";
        default: return "?";
    }
}

// A minimal TypedArray: read an element's own typed attributes off a ResXMLTree.
// Android's real TypedArray indexes by position in R.styleable.XXX; here we look
// up by (ns, name) for readability — the per-attribute extraction is identical.
class MiniTypedArray {
public:
    MiniTypedArray(const ResXMLTree& tree, float density) : mTree(tree), mDensity(density) {}

    ssize_t indexOf(const char* ns, const char* name) const {
        return mTree.indexOfAttribute(ns, name);
    }
    bool hasValue(const char* ns, const char* name) const { return indexOf(ns, name) >= 0; }

    int32_t getInt(const char* ns, const char* name, int32_t def) const {
        Res_value v; if (!get(ns, name, &v)) return def;
        if (v.dataType == Res_value::TYPE_INT_DEC || v.dataType == Res_value::TYPE_INT_HEX) return (int32_t)v.data;
        return def;
    }
    bool getBoolean(const char* ns, const char* name, bool def) const {
        Res_value v; if (!get(ns, name, &v)) return def;
        return v.dataType == Res_value::TYPE_INT_BOOLEAN ? (v.data != 0) : def;
    }
    uint32_t getColor(const char* ns, const char* name, uint32_t def) const {
        Res_value v; if (!get(ns, name, &v)) return def;
        if (v.dataType >= Res_value::TYPE_FIRST_COLOR_INT && v.dataType <= Res_value::TYPE_LAST_COLOR_INT) return v.data;
        return def;
    }
    float getFloat(const char* ns, const char* name, float def) const {
        Res_value v; if (!get(ns, name, &v)) return def;
        if (v.dataType == Res_value::TYPE_FLOAT) { float f; memcpy(&f, &v.data, sizeof(f)); return f; }
        return def;
    }
    float getDimension(const char* ns, const char* name, float def) const {
        Res_value v; if (!get(ns, name, &v)) return def;
        return v.dataType == Res_value::TYPE_DIMENSION ? complexToFloat(v.data) : def;
    }
    // sp/dip scaled by density; px as-is (approximation — real one uses fontScale/density separately).
    int32_t getDimensionPixelSize(const char* ns, const char* name, int32_t def) const {
        Res_value v; if (!get(ns, name, &v)) return def;
        if (v.dataType != Res_value::TYPE_DIMENSION) return def;
        float mag = complexToFloat(v.data);
        int unit = (v.data >> Res_value::COMPLEX_UNIT_SHIFT) & Res_value::COMPLEX_UNIT_MASK;
        float px = (unit == Res_value::COMPLEX_UNIT_PX) ? mag : mag * mDensity;
        return (int32_t)(px + 0.5f);
    }
    uint32_t getResourceId(const char* ns, const char* name, uint32_t def) const {
        Res_value v; if (!get(ns, name, &v)) return def;
        return (v.dataType == Res_value::TYPE_REFERENCE || v.dataType == Res_value::TYPE_ATTRIBUTE) ? v.data : def;
    }
    std::string getString(const char* ns, const char* name) const {
        Res_value v; if (!get(ns, name, &v) || v.dataType != Res_value::TYPE_STRING) return "";
        size_t len = 0;
        const char16_t* s = mTree.getStrings().stringAt(v.data, &len);
        std::string out;
        for (size_t i = 0; s && i < len; i++) {
            uint32_t c = s[i];
            if (c >= 0xD800 && c <= 0xDBFF && i + 1 < len && s[i + 1] >= 0xDC00) c = 0x10000 + ((c - 0xD800) << 10) + (s[++i] - 0xDC00);
            if (c < 0x80) out += (char)c;
            else if (c < 0x800) { out += (char)(0xC0 | (c >> 6)); out += (char)(0x80 | (c & 0x3F)); }
            else { out += (char)(0xE0 | (c >> 12)); out += (char)(0x80 | ((c >> 6) & 0x3F)); out += (char)(0x80 | (c & 0x3F)); }
        }
        return out;
    }
private:
    bool get(const char* ns, const char* name, Res_value* out) const {
        ssize_t idx = indexOf(ns, name);
        if (idx < 0) return false;
        return mTree.getAttributeValue((size_t)idx, out) == sizeof(Res_value);
    }
    const ResXMLTree& mTree;
    float mDensity;
};

static const char* kAndroid = "http://schemas.android.com/apk/res/android";
static const char* kApp = "http://schemas.android.com/apk/res-auto";

int main() {
    ResXMLTree tree;
    if (tree.setTo(kAXML, kAXMLLen) != cdroid::NO_ERROR) {
        std::fprintf(stderr, "failed to load AXML\n");
        return 1;
    }
    // Skip namespace declarations to the first START_TAG (LinearLayout).
    while (tree.next() != ResXMLParser::START_TAG) {
        if (tree.getEventType() == ResXMLParser::END_DOCUMENT) return 1;
    }
    size_t nl = 0;
    const char16_t* elem = tree.getElementName(&nl);
    std::string elemName; for (size_t i = 0; elem && i < nl; i++) elemName += (char)elem[i];
    std::printf("MiniTypedArray over <%s> (density=2.0 xhdpi)\n", elemName.c_str());

    MiniTypedArray a(tree, /*density*/ 2.0f);

    std::printf("  getInt(app:intdec)        = %d\n",   a.getInt(kApp, "intdec", -1));        // 42
    std::printf("  getInt(app:inthex)        = %d\n",   a.getInt(kApp, "inthex", -1));        // 255
    std::printf("  getBoolean(app:boolv)     = %s\n",   a.getBoolean(kApp, "boolv", false) ? "true" : "false");
    std::printf("  getColor(app:colorv)      = 0x%08x\n", a.getColor(kApp, "colorv", 0));     // 0xffff8800
    std::printf("  getDimension(app:dimv)    = %.1f %s\n", a.getDimension(kApp, "dimv", 0), dimUnit(0xE02)); // 14sp
    std::printf("  getDimensionPixelSize     = %d px\n", a.getDimensionPixelSize(kApp, "dimv", 0));          // 28
    std::printf("  getResourceId(app:refv)   = 0x%08x\n", a.getResourceId(kApp, "refv", 0));   // 0x0106000c
    std::printf("  getString(app:strv)       = \"%s\"\n", a.getString(kApp, "strv").c_str()); // Hello 世界
    std::printf("  getInt(android:layout_width) = %d  (wrap_content)\n", a.getInt(kAndroid, "layout_width", 0)); // -2
    std::printf("  hasValue(app:missing)     = %s\n",   a.hasValue(kApp, "missing") ? "true" : "false");
    std::printf("  getInt(app:missing, 7)    = %d  (default returned)\n", a.getInt(kApp, "missing", 7));

    return 0;
}
