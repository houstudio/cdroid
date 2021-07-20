#pragma once
namespace cdroid{

class TypedValue{
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
public:
    int type;
    int data;
    int density;
public:
    int getComplexUnit();
    static float complexToFloat(int complex);
    static float complexToFraction(int data, float base, float pbase);
    float getFraction(float base, float pbase);
};
}
