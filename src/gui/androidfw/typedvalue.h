// Port of AOSP android.util.TypedValue — container for a single resolved
// resource value. Lives in androidfw (self-contained, no GUI/porting deps) so
// both the androidfw resource layer and the cdroid core (animation/drawable)
// share one definition. DisplayMetrics is borrowed from core (forward via the
// header only; methods that need it use its fields, not its .cc).
#ifndef __ANDROIDFW_TYPEDVALUE_H__
#define __ANDROIDFW_TYPEDVALUE_H__

#include <cstdint>
#include <cstddef>
#include <androidfw/resourcetypes.h>  // Res_value (TypedValue::from)
#include <core/displaymetrics.h>   // DisplayMetrics (complexToDimension param)

namespace cdroid {

class TypedValue {
public:
    static constexpr int TYPE_NULL = 0x00;
    static constexpr int TYPE_REFERENCE = 0x01;
    static constexpr int TYPE_ATTRIBUTE = 0x02;
    static constexpr int TYPE_STRING = 0x03;
    static constexpr int TYPE_FLOAT = 0x04;
    static constexpr int TYPE_DIMENSION = 0x05;
    static constexpr int TYPE_FRACTION = 0x06;
    static constexpr int TYPE_FIRST_INT = 0x10;
    static constexpr int TYPE_INT_DEC = 0x10;
    static constexpr int TYPE_INT_HEX = 0x11;
    static constexpr int TYPE_INT_BOOLEAN = 0x12;
    static constexpr int TYPE_FIRST_COLOR_INT = 0x1c;
    static constexpr int TYPE_INT_COLOR_ARGB8 = 0x1c;
    static constexpr int TYPE_INT_COLOR_RGB8 = 0x1d;
    static constexpr int TYPE_INT_COLOR_ARGB4 = 0x1e;
    static constexpr int TYPE_INT_COLOR_RGB4 = 0x1f;
    static constexpr int TYPE_LAST_COLOR_INT = 0x1f;
    static constexpr int TYPE_LAST_INT = 0x1f;

    static constexpr int COMPLEX_UNIT_SHIFT = 0;
    static constexpr int COMPLEX_UNIT_MASK = 0xf;
    static constexpr int COMPLEX_UNIT_PX = 0;
    static constexpr int COMPLEX_UNIT_DIP = 1;
    static constexpr int COMPLEX_UNIT_SP = 2;
    static constexpr int COMPLEX_UNIT_PT = 3;
    static constexpr int COMPLEX_UNIT_IN = 4;
    static constexpr int COMPLEX_UNIT_MM = 5;
    static constexpr int COMPLEX_UNIT_FRACTION = 0;
    static constexpr int COMPLEX_UNIT_FRACTION_PARENT = 1;
    static constexpr int COMPLEX_RADIX_SHIFT = 4;
    static constexpr int COMPLEX_RADIX_MASK = 0x3;
    static constexpr int COMPLEX_RADIX_23p0 = 0;
    static constexpr int COMPLEX_RADIX_16p7 = 1;
    static constexpr int COMPLEX_RADIX_8p15 = 2;
    static constexpr int COMPLEX_RADIX_0p23 = 3;
    static constexpr int COMPLEX_MANTISSA_SHIFT = 8;
    static constexpr int COMPLEX_MANTISSA_MASK = 0xffffff;

    static constexpr int DATA_NULL_UNDEFINED = 0;
    static constexpr int DATA_NULL_EMPTY = 1;
    static constexpr int DENSITY_DEFAULT = 0;
    static constexpr int DENSITY_NONE = 0xffff;

    int type = 0;                       // Res_value::TYPE_*
    int data = 0;                       // raw payload (int / complex / pool index)
    int density = 0;                    // density of the config the value came from
    // Resource-resolution extensions:
    uint32_t resourceId = 0;            // the resource id this value came from
    uint32_t changingConfigurations = 0;
    int assetCookie = 0;                // owning ApkAssets cookie (reserved)
    const char16_t* string = nullptr;   // TYPE_STRING payload (borrowed)
    size_t stringLen = 0;

    int   getComplexUnit();
    // Adopt the type/data payload of a raw Res_value (androidfw internal).
    // AOSP ResXMLTree.getAttributeValue fills a TypedValue directly; CDROID's
    // TypedArray hands out Res_value, so this is the seam.
    static TypedValue from(const Res_value& v) {
        TypedValue tv;
        tv.type = v.dataType;
        tv.data = v.data;
        return tv;
    }
    static float complexToFloat(int complex);
    static float complexToFraction(int data, float base, float pbase);
    float getFraction(float base, float pbase);
    float getFloat() const;
    float complexToDimension(const DisplayMetrics& m) const;
    int   complexToDimensionPixelOffset(const DisplayMetrics& m) const;
    int   complexToDimensionPixelSize(const DisplayMetrics& m) const;
};

// Convert a (unit, value) pair to pixels. Faithful to AOSP TypedValue.applyDimension.
float applyDimension(int unit, float value, const DisplayMetrics& metrics);

} // namespace cdroid
#endif // __ANDROIDFW_TYPEDVALUE_H__
