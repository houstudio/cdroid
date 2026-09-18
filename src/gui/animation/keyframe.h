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
#ifndef __ANIMATION_KEYFRAME_H__
#define __ANIMATION_KEYFRAME_H__
// Port of android.animation.Keyframe (android-36). AOSP's Object value type
// maps to AnimateValue; Class mValueType maps to Property::*_TYPE ints.
#include <animation/property.h>
#include <animation/interpolators.h>
namespace cdroid{

class Keyframe{
protected:
    bool mHasValue;
    bool mValueWasSetOnStart;
    float mFraction;
    int  mValueType;   // Property::INT_TYPE / FLOAT_TYPE / COLOR_TYPE / PATH_TYPE...
    const TimeInterpolator* mInterpolator;
    Keyframe():mHasValue(false),mValueWasSetOnStart(false),mFraction(0.f),
              mValueType(Property::UNDEFINED),mInterpolator(nullptr){
    }
public:
    virtual ~Keyframe()=default;

    static Keyframe*ofInt(float fraction, int value);
    static Keyframe*ofInt(float fraction);
    static Keyframe*ofFloat(float fraction, float value);
    static Keyframe*ofFloat(float fraction);
    static Keyframe*ofObject(float fraction, const AnimateValue& value);
    static Keyframe*ofObject(float fraction);

    bool hasValue()const{
        return mHasValue;
    }
    bool valueWasSetOnStart()const{
        return mValueWasSetOnStart;
    }
    void setValueWasSetOnStart(bool valueWasSetOnStart){
        mValueWasSetOnStart = valueWasSetOnStart;
    }
    virtual AnimateValue getValue()const=0;
    virtual void setValue(const AnimateValue& value)=0;
    float getFraction()const{
        return mFraction;
    }
    void setFraction(float fraction){
        mFraction = fraction;
    }
    const TimeInterpolator* getInterpolator()const{
        return mInterpolator;
    }
    void setInterpolator(const TimeInterpolator* interpolator){
        mInterpolator = interpolator;
    }
    int getType()const{
        return mValueType;
    }
    virtual Keyframe*clone()const=0;
};

// AOSP keeps these as package-private static classes inside Keyframe.java;
// C++ needs them nameable for the ofKeyframe() instanceof dispatch (RTTI).
class ObjectKeyframe:public Keyframe{
private:
    AnimateValue mValue;
public:
    ObjectKeyframe(float fraction, const AnimateValue& value);
    ObjectKeyframe(float fraction);   // no value — filled later via setupValue
    AnimateValue getValue()const override;
    void setValue(const AnimateValue& value) override;
    ObjectKeyframe*clone()const override;
};

class IntKeyframe:public Keyframe{
private:
    int mValue;
public:
    IntKeyframe(float fraction, int value);
    IntKeyframe(float fraction);
    int getIntValue()const{
        return mValue;
    }
    AnimateValue getValue()const override;
    void setValue(const AnimateValue& value) override;
    IntKeyframe*clone()const override;
};

class FloatKeyframe:public Keyframe{
private:
    float mValue;
public:
    FloatKeyframe(float fraction, float value);
    FloatKeyframe(float fraction);
    float getFloatValue()const{
        return mValue;
    }
    AnimateValue getValue()const override;
    void setValue(const AnimateValue& value) override;
    FloatKeyframe*clone()const override;
};

}//endof namespace
#endif/*__ANIMATION_KEYFRAME_H__*/
