/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/

/*
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.CustomAttribute.
 *
 * A typed custom attribute (int/float/color/string/boolean/dimension/reference) carried on a
 * WidgetFrame, interpolated across a motion transition. Colors interpolate as 4 floats
 * (linearized RGB + alpha); the HSV round-trip is in hsvToRgb. Java's Object value maps to
 * std::variant<int,float,bool,std::string>.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_CUSTOM_ATTRIBUTE_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_CUSTOM_ATTRIBUTE_H

#include <string>
#include <vector>

namespace cdroid {

class CustomAttribute {
  public:
    enum class AttributeType {
        INT_TYPE,
        FLOAT_TYPE,
        COLOR_TYPE,
        COLOR_DRAWABLE_TYPE,
        STRING_TYPE,
        BOOLEAN_TYPE,
        DIMENSION_TYPE,
        REFERENCE_TYPE
    };

    CustomAttribute(const std::string& name, AttributeType attributeType);

    // Java's Object value maps to a templated ctor forwarding to a typed setValue overload. The
    // static type passed must match mType (the Java original makes the same assumption).
    template <typename T>
    CustomAttribute(const std::string& name, AttributeType attributeType, const T& value, bool method)
        : mName(name), mType(attributeType), mMethod(method) {
        setValue(value);
    }
    template <typename T>
    CustomAttribute(const CustomAttribute& source, const T& value)
        : mName(source.mName), mType(source.mType) {
        setValue(value);
    }

    AttributeType getType() const {
        return mType;
    }
    const std::string& getName() const {
        return mName;
    }

    bool isContinuous() const;
    int  numberOfInterpolatedValues() const;

    void setFloatValue(float value);
    void setColorValue(int value);
    void setIntValue(int value);
    void setStringValue(const std::string& value);

    float getValueToInterpolate() const;             // single-value types only
    void  getValuesToInterpolate(std::vector<float>& ret) const;
    void  setValue(const std::vector<float>& value); // inverse of getValuesToInterpolate (Java: setValue(float[]))
    // Java: setValue(Object) — dispatches by mType to the matching field.
    void setValue(int value);
    void setValue(float value);
    void setValue(bool value);
    void setValue(const std::string& value);

    bool  diff(const CustomAttribute& other) const;

    static int hsvToRgb(float hue, float saturation, float value);

    std::string mName;
    bool mBooleanValue = false;

  private:
    static int clamp(int c);

    AttributeType mType;
    int mIntegerValue = 0;
    float mFloatValue = 0;
    std::string mStringValue;
    int mColorValue = 0;
    bool mMethod = false;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_CUSTOM_ATTRIBUTE_H
