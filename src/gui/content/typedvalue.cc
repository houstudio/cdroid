// Port of AOSP android.util.TypedValue method bodies + applyDimension. Merged
// from the former core/typedvalue.cc (complexToFraction/getFraction) and the
// androidfw resources.cc TypedValue helpers (getFloat/complexToDimension*).
#include "typedvalue.h"

#include <cstring>

namespace cdroid {

constexpr float MANTISSA_MULT = 1.0f / (1 << TypedValue::COMPLEX_MANTISSA_SHIFT);
constexpr float RADIX_MULTS[] = {
    1.0f * MANTISSA_MULT, 1.0f / (1 << 7) * MANTISSA_MULT,
    1.0f / (1 << 15) * MANTISSA_MULT, 1.0f / (1 << 23) * MANTISSA_MULT
};

int TypedValue::getComplexUnit() {
    return COMPLEX_UNIT_MASK & (data >> COMPLEX_UNIT_SHIFT);
}

float TypedValue::complexToFloat(int complex) {
    return (complex & (COMPLEX_MANTISSA_MASK << COMPLEX_MANTISSA_SHIFT))
        * RADIX_MULTS[(complex >> COMPLEX_RADIX_SHIFT) & COMPLEX_RADIX_MASK];
}

float TypedValue::complexToFraction(int data, float base, float pbase) {
    switch ((data >> COMPLEX_UNIT_SHIFT) & COMPLEX_UNIT_MASK) {
    case COMPLEX_UNIT_FRACTION:       return complexToFloat(data) * base;
    case COMPLEX_UNIT_FRACTION_PARENT:return complexToFloat(data) * pbase;
    }
    return 0;
}

float TypedValue::getFraction(float base, float pbase) {
    return complexToFraction(data, base, pbase);
}

float TypedValue::getFloat() const {
    float f;
    memcpy(&f, &data, sizeof(f));
    return f;
}

float TypedValue::complexToDimension(const DisplayMetrics& m) const {
    const float value = complexToFloat(data);
    const int unit = (data >> COMPLEX_UNIT_SHIFT) & COMPLEX_UNIT_MASK;
    return applyDimension(unit, value, m);
}

int TypedValue::complexToDimensionPixelOffset(const DisplayMetrics& m) const {
    return (int)complexToDimension(m);
}

int TypedValue::complexToDimensionPixelSize(const DisplayMetrics& m) const {
    const float mag = complexToFloat(data);
    const int unit = (data >> COMPLEX_UNIT_SHIFT) & COMPLEX_UNIT_MASK;
    const float f = applyDimension(unit, mag, m);
    const int res = (int)(f + 0.5f);
    if (res != 0) return res;
    if (mag == 0.0f) return 0;
    return mag > 0 ? 1 : -1;
}

float applyDimension(int unit, float value, const DisplayMetrics& m) {
    switch (unit) {
    case TypedValue::COMPLEX_UNIT_PX:  return value;
    case TypedValue::COMPLEX_UNIT_DIP: return value * m.density;
    case TypedValue::COMPLEX_UNIT_SP:  return value * m.scaledDensity;
    case TypedValue::COMPLEX_UNIT_PT:  return value * m.xdpi * (1.0f / 72.0f);
    case TypedValue::COMPLEX_UNIT_IN:  return value * m.xdpi;
    case TypedValue::COMPLEX_UNIT_MM:  return value * m.xdpi * (1.0f / 25.4f);
    default:                           return 0.0f;
    }
}

} // namespace cdroid
