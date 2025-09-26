#ifndef __DYNAMIC_ANIMATION_FLOAT_VALUE_HOLDER_H__
#define __DYNAMIC_ANIMATION_FLOAT_VALUE_HOLDER_H__
#include <animation/property.h>
namespace cdroid{
class FloatValueHolder {
private:
    float mValue;
public:
    /**
     * Constructs a holder for a float value that is initialized to 0.
     */
    FloatValueHolder():FloatValueHolder(0.0f){
    }

    /**
     * Constructs a holder for a float value that is initialized to the input value.
     *
     * @param value the value to initialize the value held in the FloatValueHolder
     */
    FloatValueHolder(float value):mValue(value){
    }

    /**
     * Sets the value held in the FloatValueHolder instance.
     *
     * @param value float value held in the FloatValueHolder instance
     */
    void setValue(float value) {
        mValue = value;
    }

    /**
     * Returns the float value held in the FloatValueHolder instance.
     *
     * @return float value held in the FloatValueHolder instance
     */
    float getValue() {
        return mValue;
    }
};

}/*endof namespace*/
#endif/*__DYNAMIC_ANIMATION_FLOAT_VALUE_HOLDER_H__*/
