#include <typedvalue.h>
namespace cdroid{

constexpr float MANTISSA_MULT = 1.0f / (1<<TypedValue::COMPLEX_MANTISSA_SHIFT);
constexpr float RADIX_MULTS[] = {
        1.0f*MANTISSA_MULT, 1.0f/(1<<7)*MANTISSA_MULT,
        1.0f/(1<<15)*MANTISSA_MULT, 1.0f/(1<<23)*MANTISSA_MULT
    };

int TypedValue::getComplexUnit(){
    return COMPLEX_UNIT_MASK & (data>>TypedValue::COMPLEX_UNIT_SHIFT);
}

float TypedValue::complexToFloat(int complex){
    return (complex&(TypedValue::COMPLEX_MANTISSA_MASK
                   <<TypedValue::COMPLEX_MANTISSA_SHIFT))
            * RADIX_MULTS[(complex>>TypedValue::COMPLEX_RADIX_SHIFT)
                            & TypedValue::COMPLEX_RADIX_MASK];
}

float TypedValue::complexToFraction(int data, float base, float pbase){
    switch ((data>>COMPLEX_UNIT_SHIFT)&COMPLEX_UNIT_MASK) {
    case COMPLEX_UNIT_FRACTION:
        return complexToFloat(data) * base;
    case COMPLEX_UNIT_FRACTION_PARENT:
        return complexToFloat(data) * pbase;
    }
    return 0;
}

float TypedValue::getFraction(float base, float pbase){
    return complexToFraction(data, base, pbase);
}
}
