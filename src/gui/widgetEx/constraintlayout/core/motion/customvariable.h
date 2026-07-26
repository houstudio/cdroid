/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.CustomVariable.
 *
 * A typed custom attribute identified by a (name, TypedValues.Custom.TYPE_*) pair and carried on a
 * WidgetFrame's custom map. Like CustomAttribute but value-typed by the int TYPE_* id (not the
 * AttributeType enum) and applied to a MotionWidget via setCustomAttribute. Java's Object value
 * maps to typed setValue overloads.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_CUSTOM_VARIABLE_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_CUSTOM_VARIABLE_H

#include <climits>
#include <cmath>
#include <string>
#include <vector>

namespace cdroid {

class MotionWidget; // forward — the apply/interpolate hooks are filled in once MotionWidget lands

class CustomVariable {
public:
    CustomVariable() = default; // required as a value type in std::unordered_map
    CustomVariable(const std::string& name, int attributeType);
    CustomVariable(const CustomVariable& c);
    CustomVariable(const std::string& name, int type, const std::string& value);
    CustomVariable(const std::string& name, int type, int value);
    CustomVariable(const std::string& name, int type, float value);
    CustomVariable(const std::string& name, int type, bool value);
    template <typename T>
    CustomVariable(const std::string& name, int attributeType, const T& value)
        : mName(name), mType(attributeType) { setValue(value); }
    template <typename T>
    CustomVariable(const CustomVariable& source, const T& value)
        : mName(source.mName), mType(source.mType) { setValue(value); }

    CustomVariable copy() const;

    int  getType() const { return mType; }
    const std::string& getName() const { return mName; }
    bool getBooleanValue() const { return mBooleanValue; }
    float getFloatValue() const { return mFloatValue; }
    int   getColorValue() const { return mIntegerValue; }
    int   getIntegerValue() const { return mIntegerValue; }
    const std::string& getStringValue() const { return mStringValue; }

    bool isContinuous() const;
    int  numberOfInterpolatedValues() const;
    float getValueToInterpolate() const;
    void  getValuesToInterpolate(std::vector<float>& ret) const;
    void  setValue(const std::vector<float>& value); // inverse of getValuesToInterpolate

    void setFloatValue(float value);
    void setBooleanValue(bool value);
    void setIntValue(int value);
    void setStringValue(const std::string& value);
    // Java: setValue(Object)
    void setValue(int value);
    void setValue(float value);
    void setValue(bool value);
    void setValue(const std::string& value);

    bool diff(const CustomVariable& other) const;

    int getInterpolatedColor(const std::vector<float>& value) const;
    // Apply the interpolated value / the stored value to a MotionWidget. Bodies filled in once
    // MotionWidget (chunk 4) provides setCustomAttribute.
    void setInterpolatedValue(MotionWidget* view, const std::vector<float>& value);
    void applyToWidget(MotionWidget* view);

    static std::string colorString(int v);
    static int hsvToRgb(float hue, float saturation, float value);
    static int rgbaToColor(float r, float g, float b, float a);

    std::string mName;
    bool mBooleanValue = false;

private:
    static int clamp(int c);

    int mType = 0;
    int mIntegerValue = INT_MIN;
    float mFloatValue = NAN;
    std::string mStringValue;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_CUSTOM_VARIABLE_H
