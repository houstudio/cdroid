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
#include <animation/keyframe.h>

namespace cdroid{

Keyframe*Keyframe::ofInt(float fraction, int value){
    return new IntKeyframe(fraction, value);
}

Keyframe*Keyframe::ofInt(float fraction){
    return new IntKeyframe(fraction);
}

Keyframe*Keyframe::ofFloat(float fraction, float value){
    return new FloatKeyframe(fraction, value);
}

Keyframe*Keyframe::ofFloat(float fraction){
    return new FloatKeyframe(fraction);
}

Keyframe*Keyframe::ofObject(float fraction, const AnimateValue& value){
    return new ObjectKeyframe(fraction, value);
}

Keyframe*Keyframe::ofObject(float fraction){
    return new ObjectKeyframe(fraction);
}

ObjectKeyframe::ObjectKeyframe(float fraction, const AnimateValue& value){
    mFraction = fraction;
    mValue = value;
    mHasValue = true;
    // AOSP: mValueType = value.getClass() — map the variant alternative to the
    // Property::*_TYPE space (they do not line up numerically).
    switch(value.index()){
        case 0:  mValueType = Property::INT_TYPE;  break;   // int
        case 1:  mValueType = Property::FLOAT_TYPE;break;   // float
        case 2:  mValueType = Property::PATH_TYPE; break;   // PathData
        default: mValueType = Property::UNDEFINED;break;    // Rect/RectF/PointF
    }
}

ObjectKeyframe::ObjectKeyframe(float fraction){
    mFraction = fraction;
    mHasValue = false;
    mValueType = Property::UNDEFINED;
}

AnimateValue ObjectKeyframe::getValue()const{
    return mValue;
}

void ObjectKeyframe::setValue(const AnimateValue& value){
    mValue = value;
    mHasValue = true;
}

ObjectKeyframe*ObjectKeyframe::clone()const{
    ObjectKeyframe*kfClone = new ObjectKeyframe(getFraction(), mValue);
    kfClone->mValueWasSetOnStart = mValueWasSetOnStart;
    kfClone->setInterpolator(getInterpolator());
    return kfClone;
}

IntKeyframe::IntKeyframe(float fraction, int value){
    mFraction = fraction;
    mValue = value;
    mValueType = Property::INT_TYPE;
    mHasValue = true;
}

IntKeyframe::IntKeyframe(float fraction){
    mFraction = fraction;
    mValue = 0;   // Java field zero-init; filled by setupValue when hasValue()==false
    mValueType = Property::INT_TYPE;
}

AnimateValue IntKeyframe::getValue()const{
    return mValue;
}

void IntKeyframe::setValue(const AnimateValue& value){
    // AOSP: only accepts Integer; other types leave the keyframe unchanged.
    if(value.index() == 0/*int in AnimateValue*/){
        mValue = GET_VARIANT(value,int);
        mHasValue = true;
    }
}

IntKeyframe*IntKeyframe::clone()const{
    IntKeyframe*kfClone = mHasValue ?
            new IntKeyframe(getFraction(), mValue) :
            new IntKeyframe(getFraction());
    kfClone->setInterpolator(getInterpolator());
    kfClone->mValueWasSetOnStart = mValueWasSetOnStart;
    return kfClone;
}

FloatKeyframe::FloatKeyframe(float fraction, float value){
    mFraction = fraction;
    mValue = value;
    mValueType = Property::FLOAT_TYPE;
    mHasValue = true;
}

FloatKeyframe::FloatKeyframe(float fraction){
    mFraction = fraction;
    mValue = 0.f;   // Java field zero-init; filled by setupValue when hasValue()==false
    mValueType = Property::FLOAT_TYPE;
}

AnimateValue FloatKeyframe::getValue()const{
    return mValue;
}

void FloatKeyframe::setValue(const AnimateValue& value){
    // AOSP: only accepts Float; other types leave the keyframe unchanged.
    if(value.index() == 1/*float in AnimateValue*/){
        mValue = GET_VARIANT(value,float);
        mHasValue = true;
    }
}

FloatKeyframe*FloatKeyframe::clone()const{
    FloatKeyframe*kfClone = mHasValue ?
            new FloatKeyframe(getFraction(), mValue) :
            new FloatKeyframe(getFraction());
    kfClone->setInterpolator(getInterpolator());
    kfClone->mValueWasSetOnStart = mValueWasSetOnStart;
    return kfClone;
}

}//endof namespace
