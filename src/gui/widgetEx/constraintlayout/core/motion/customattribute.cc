/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.CustomAttribute.
 */
#include <widgetEx/constraintlayout/core/motion/customattribute.h>

#include <cmath>
#include <stdexcept>

namespace cdroid {

CustomAttribute::CustomAttribute(const std::string& name, AttributeType attributeType)
    : mName(name), mType(attributeType) {}

bool CustomAttribute::isContinuous() const {
    switch (mType) {
    case AttributeType::REFERENCE_TYPE:
    case AttributeType::BOOLEAN_TYPE:
    case AttributeType::STRING_TYPE:
        return false;
    default:
        return true;
    }
}

void CustomAttribute::setFloatValue(float value) {
    mFloatValue = value;
}
void CustomAttribute::setColorValue(int value)   {
    mColorValue = value;
}
void CustomAttribute::setIntValue(int value)     {
    mIntegerValue = value;
}
void CustomAttribute::setStringValue(const std::string& value) {
    mStringValue = value;
}

int CustomAttribute::numberOfInterpolatedValues() const {
    switch (mType) {
    case AttributeType::COLOR_TYPE:
    case AttributeType::COLOR_DRAWABLE_TYPE:
        return 4;
    default:
        return 1;
    }
}

float CustomAttribute::getValueToInterpolate() const {
    switch (mType) {
    case AttributeType::INT_TYPE:
        return (float) mIntegerValue;
    case AttributeType::FLOAT_TYPE:
        return mFloatValue;
    case AttributeType::BOOLEAN_TYPE:
        return mBooleanValue ? 1 : 0;
    case AttributeType::DIMENSION_TYPE:
        return mFloatValue;
    case AttributeType::COLOR_TYPE:
    case AttributeType::COLOR_DRAWABLE_TYPE:
        throw std::runtime_error("Color does not have a single value to interpolate");
    case AttributeType::STRING_TYPE:
        throw std::runtime_error("Cannot interpolate String");
    default:
        return NAN;
    }
}

void CustomAttribute::getValuesToInterpolate(std::vector<float>& ret) const {
    switch (mType) {
    case AttributeType::INT_TYPE:
        ret[0] = (float) mIntegerValue;
        break;
    case AttributeType::FLOAT_TYPE:
        ret[0] = mFloatValue;
        break;
    case AttributeType::BOOLEAN_TYPE:
        ret[0] = mBooleanValue ? 1 : 0;
        break;
    case AttributeType::DIMENSION_TYPE:
        ret[0] = mFloatValue;
        break;
    case AttributeType::COLOR_TYPE:
    case AttributeType::COLOR_DRAWABLE_TYPE: {
        int a = 0xFF & (mColorValue >> 24);
        int r = 0xFF & (mColorValue >> 16);
        int g = 0xFF & (mColorValue >> 8);
        int b = 0xFF & mColorValue;
        ret[0] = (float) std::pow(r / 255.0f, 2.2);
        ret[1] = (float) std::pow(g / 255.0f, 2.2);
        ret[2] = (float) std::pow(b / 255.0f, 2.2);
        ret[3] = a / 255.0f;
        break;
    }
    default:
        break;
    }
}

void CustomAttribute::setValue(const std::vector<float>& value) {
    switch (mType) {
    case AttributeType::REFERENCE_TYPE:
    case AttributeType::INT_TYPE:
        mIntegerValue = (int) value[0];
        break;
    case AttributeType::FLOAT_TYPE:
    case AttributeType::DIMENSION_TYPE:
        mFloatValue = value[0];
        break;
    case AttributeType::COLOR_TYPE:
    case AttributeType::COLOR_DRAWABLE_TYPE:
        mColorValue = hsvToRgb(value[0], value[1], value[2]);
        mColorValue = (mColorValue & 0xFFFFFF) | (clamp((int) (0xFF * value[3])) << 24);
        break;
    case AttributeType::BOOLEAN_TYPE:
        mBooleanValue = value[0] > 0.5;
        break;
    default:
        break;
    }
}

void CustomAttribute::setValue(int value) {
    switch (mType) {
    case AttributeType::REFERENCE_TYPE:
    case AttributeType::INT_TYPE:
        mIntegerValue = value;
        break;
    case AttributeType::COLOR_TYPE:
    case AttributeType::COLOR_DRAWABLE_TYPE:
        mColorValue = value;
        break;
    default:
        break;
    }
}

void CustomAttribute::setValue(float value) {
    switch (mType) {
    case AttributeType::FLOAT_TYPE:
    case AttributeType::DIMENSION_TYPE:
        mFloatValue = value;
        break;
    default:
        break;
    }
}

void CustomAttribute::setValue(bool value) {
    if (mType == AttributeType::BOOLEAN_TYPE) mBooleanValue = value;
}

void CustomAttribute::setValue(const std::string& value) {
    if (mType == AttributeType::STRING_TYPE) mStringValue = value;
}

bool CustomAttribute::diff(const CustomAttribute& other) const {
    if (mType != other.mType) return false;
    switch (mType) {
    case AttributeType::INT_TYPE:
    case AttributeType::REFERENCE_TYPE:
        return mIntegerValue == other.mIntegerValue;
    case AttributeType::FLOAT_TYPE:
    case AttributeType::DIMENSION_TYPE:
        return mFloatValue == other.mFloatValue;
    case AttributeType::COLOR_TYPE:
    case AttributeType::COLOR_DRAWABLE_TYPE:
        return mColorValue == other.mColorValue;
    case AttributeType::STRING_TYPE:
        return mIntegerValue == other.mIntegerValue;
    case AttributeType::BOOLEAN_TYPE:
        return mBooleanValue == other.mBooleanValue;
    default:
        return false;
    }
}

int CustomAttribute::hsvToRgb(float hue, float saturation, float value) {
    int h = (int) (hue * 6);
    float f = hue * 6 - h;
    int p = (int) (0.5f + 255 * value * (1 - saturation));
    int q = (int) (0.5f + 255 * value * (1 - f * saturation));
    int t = (int) (0.5f + 255 * value * (1 - (1 - f) * saturation));
    int v = (int) (0.5f + 255 * value);
    switch (h) {
    case 0:
        return 0xFF000000 | (v << 16) + (t << 8) + p;
    case 1:
        return 0xFF000000 | (q << 16) + (v << 8) + p;
    case 2:
        return 0xFF000000 | (p << 16) + (v << 8) + t;
    case 3:
        return 0xFF000000 | (p << 16) + (q << 8) + v;
    case 4:
        return 0xFF000000 | (t << 16) + (p << 8) + v;
    case 5:
        return 0xFF000000 | (v << 16) + (p << 8) + q;
    default:
        return 0;
    }
}

int CustomAttribute::clamp(int c) {
    int n = 255;
    c &= ~(c >> 31);
    c -= n;
    c &= (c >> 31);
    c += n;
    return c;
}

} // namespace cdroid
