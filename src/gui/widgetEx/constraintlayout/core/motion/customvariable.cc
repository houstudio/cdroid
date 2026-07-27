/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.CustomVariable.
 */
#include <widgetEx/constraintlayout/core/motion/customvariable.h>

#include <climits>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <widgetEx/constraintlayout/core/motion/motionwidget.h>
#include <widgetEx/constraintlayout/core/motion/typedvalues.h>

namespace cdroid {

CustomVariable::CustomVariable(const std::string& name, int attributeType)
    : mName(name), mType(attributeType) {}

CustomVariable::CustomVariable(const CustomVariable& c)
    : mName(c.mName), mType(c.mType), mIntegerValue(c.mIntegerValue),
      mFloatValue(c.mFloatValue), mStringValue(c.mStringValue), mBooleanValue(c.mBooleanValue) {}

CustomVariable::CustomVariable(const std::string& name, int type, const std::string& value)
    : mName(name), mType(type), mStringValue(value) {}

CustomVariable::CustomVariable(const std::string& name, int type, int value)
    : mName(name), mType(type) {
    if (type == TypedValues::Custom::TYPE_FLOAT) mFloatValue = value; // catch int meant for float
    else mIntegerValue = value;
}

CustomVariable::CustomVariable(const std::string& name, int type, float value)
    : mName(name), mType(type), mFloatValue(value) {}

CustomVariable::CustomVariable(const std::string& name, int type, bool value)
    : mName(name), mType(type), mBooleanValue(value) {}

CustomVariable CustomVariable::copy() const {
    return CustomVariable(*this);
}

std::string CustomVariable::colorString(int v) {
    std::ostringstream ss;
    ss << std::hex << v;
    std::string str = "00000000" + ss.str();
    return "#" + str.substr(str.length() - 8);
}

bool CustomVariable::isContinuous() const {
    switch (mType) {
    case TypedValues::Custom::TYPE_REFERENCE:
    case TypedValues::Custom::TYPE_BOOLEAN:
    case TypedValues::Custom::TYPE_STRING:
        return false;
    default:
        return true;
    }
}

void CustomVariable::setFloatValue(float value) {
    mFloatValue = value;
}
void CustomVariable::setBooleanValue(bool value) {
    mBooleanValue = value;
}
void CustomVariable::setIntValue(int value) {
    mIntegerValue = value;
}
void CustomVariable::setStringValue(const std::string& value) {
    mStringValue = value;
}

int CustomVariable::numberOfInterpolatedValues() const {
    return (mType == TypedValues::Custom::TYPE_COLOR) ? 4 : 1;
}

float CustomVariable::getValueToInterpolate() const {
    switch (mType) {
    case TypedValues::Custom::TYPE_INT:
        return (float) mIntegerValue;
    case TypedValues::Custom::TYPE_FLOAT:
        return mFloatValue;
    case TypedValues::Custom::TYPE_BOOLEAN:
        return mBooleanValue ? 1 : 0;
    case TypedValues::Custom::TYPE_DIMENSION:
        return mFloatValue;
    case TypedValues::Custom::TYPE_COLOR:
        throw std::runtime_error("Color does not have a single value to interpolate");
    case TypedValues::Custom::TYPE_STRING:
        throw std::runtime_error("Cannot interpolate String");
    default:
        return NAN;
    }
}

void CustomVariable::getValuesToInterpolate(std::vector<float>& ret) const {
    switch (mType) {
    case TypedValues::Custom::TYPE_INT:
        ret[0] = (float) mIntegerValue;
        break;
    case TypedValues::Custom::TYPE_FLOAT:
        ret[0] = mFloatValue;
        break;
    case TypedValues::Custom::TYPE_BOOLEAN:
        ret[0] = mBooleanValue ? 1 : 0;
        break;
    case TypedValues::Custom::TYPE_DIMENSION:
        ret[0] = mFloatValue;
        break;
    case TypedValues::Custom::TYPE_COLOR: {
        int a = 0xFF & (mIntegerValue >> 24);
        int r = 0xFF & (mIntegerValue >> 16);
        int g = 0xFF & (mIntegerValue >> 8);
        int b = 0xFF & mIntegerValue;
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

void CustomVariable::setValue(const std::vector<float>& value) {
    switch (mType) {
    case TypedValues::Custom::TYPE_REFERENCE:
    case TypedValues::Custom::TYPE_INT:
        mIntegerValue = (int) value[0];
        break;
    case TypedValues::Custom::TYPE_FLOAT:
    case TypedValues::Custom::TYPE_DIMENSION:
        mFloatValue = value[0];
        break;
    case TypedValues::Custom::TYPE_COLOR: {
        int r = 0xFF & (int) std::round(std::pow(value[0], 1.0 / 2.0) * 255.0f);
        int g = 0xFF & (int) std::round(std::pow(value[1], 1.0 / 2.0) * 255.0f);
        int b = 0xFF & (int) std::round(std::pow(value[2], 1.0 / 2.0) * 255.0f);
        int a = 0xFF & (int) std::round(value[3] * 255.0f);
        mIntegerValue = a << 24 | r << 16 | g << 8 | b;
        break;
    }
    case TypedValues::Custom::TYPE_BOOLEAN:
        mBooleanValue = value[0] > 0.5;
        break;
    default:
        break;
    }
}

void CustomVariable::setValue(int value) {
    if (mType == TypedValues::Custom::TYPE_COLOR
            || mType == TypedValues::Custom::TYPE_INT
            || mType == TypedValues::Custom::TYPE_REFERENCE) mIntegerValue = value;
}
void CustomVariable::setValue(float value) {
    if (mType == TypedValues::Custom::TYPE_FLOAT || mType == TypedValues::Custom::TYPE_DIMENSION) mFloatValue = value;
}
void CustomVariable::setValue(bool value) {
    if (mType == TypedValues::Custom::TYPE_BOOLEAN) mBooleanValue = value;
}
void CustomVariable::setValue(const std::string& value) {
    if (mType == TypedValues::Custom::TYPE_STRING) mStringValue = value;
}

int CustomVariable::hsvToRgb(float hue, float saturation, float value) {
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

bool CustomVariable::diff(const CustomVariable& other) const {
    if (mType != other.mType) return false;
    switch (mType) {
    case TypedValues::Custom::TYPE_INT:
    case TypedValues::Custom::TYPE_REFERENCE:
    case TypedValues::Custom::TYPE_COLOR:
        return mIntegerValue == other.mIntegerValue;
    case TypedValues::Custom::TYPE_FLOAT:
    case TypedValues::Custom::TYPE_DIMENSION:
        return mFloatValue == other.mFloatValue;
    case TypedValues::Custom::TYPE_STRING:
        return mIntegerValue == other.mIntegerValue;
    case TypedValues::Custom::TYPE_BOOLEAN:
        return mBooleanValue == other.mBooleanValue;
    default:
        return false;
    }
}

int CustomVariable::getInterpolatedColor(const std::vector<float>& value) const {
    int r = clamp((int) (std::pow(value[0], 1.0 / 2.2) * 255.0f));
    int g = clamp((int) (std::pow(value[1], 1.0 / 2.2) * 255.0f));
    int b = clamp((int) (std::pow(value[2], 1.0 / 2.2) * 255.0f));
    int a = clamp((int) (value[3] * 255.0f));
    return (a << 24) | (r << 16) | (g << 8) | b;
}

int CustomVariable::rgbaToColor(float r, float g, float b, float a) {
    int ir = clamp((int) (r * 255.0f));
    int ig = clamp((int) (g * 255.0f));
    int ib = clamp((int) (b * 255.0f));
    int ia = clamp((int) (a * 255.0f));
    return (ia << 24) | (ir << 16) | (ig << 8) | ib;
}

void CustomVariable::setInterpolatedValue(MotionWidget* view, const std::vector<float>& value) {
    switch (mType) {
    case TypedValues::Custom::TYPE_INT:
        view->setCustomAttribute(mName, mType, (int) value[0]);
        break;
    case TypedValues::Custom::TYPE_COLOR: {
        int r = clamp((int) (std::pow(value[0], 1.0 / 2.2) * 255.0f));
        int g = clamp((int) (std::pow(value[1], 1.0 / 2.2) * 255.0f));
        int b = clamp((int) (std::pow(value[2], 1.0 / 2.2) * 255.0f));
        int a = clamp((int) (value[3] * 255.0f));
        view->setCustomAttribute(mName, mType, (a << 24) | (r << 16) | (g << 8) | b);
        break;
    }
    case TypedValues::Custom::TYPE_BOOLEAN:
        view->setCustomAttribute(mName, mType, value[0] > 0.5f);
        break;
    case TypedValues::Custom::TYPE_DIMENSION:
    case TypedValues::Custom::TYPE_FLOAT:
        view->setCustomAttribute(mName, mType, value[0]);
        break;
    default:
        break;
    }
}

void CustomVariable::applyToWidget(MotionWidget* view) {
    switch (mType) {
    case TypedValues::Custom::TYPE_INT:
    case TypedValues::Custom::TYPE_COLOR:
    case TypedValues::Custom::TYPE_REFERENCE:
        view->setCustomAttribute(mName, mType, mIntegerValue);
        break;
    case TypedValues::Custom::TYPE_STRING:
        view->setCustomAttribute(mName, mType, mStringValue);
        break;
    case TypedValues::Custom::TYPE_BOOLEAN:
        view->setCustomAttribute(mName, mType, mBooleanValue);
        break;
    case TypedValues::Custom::TYPE_DIMENSION:
    case TypedValues::Custom::TYPE_FLOAT:
        view->setCustomAttribute(mName, mType, mFloatValue);
        break;
    default:
        break;
    }
}

int CustomVariable::clamp(int c) {
    int n = 255;
    c &= ~(c >> 31);
    c -= n;
    c &= (c >> 31);
    c += n;
    return c;
}

} // namespace cdroid
