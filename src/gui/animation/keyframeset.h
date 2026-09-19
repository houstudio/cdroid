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
#ifndef __ANIMATION_KEYFRAMESET_H__
#define __ANIMATION_KEYFRAMESET_H__
// Port of android.animation.KeyframeSet (+Int/FloatKeyframeSet, android-36).
// The set owns its Keyframe objects (AOSP relies on GC).
#include <string>
#include <vector>
#include <animation/keyframes.h>
namespace cdroid{

class KeyframeSet:public virtual Keyframes{
protected:
    int mNumKeyframes;
    Keyframe*mFirstKeyframe;
    Keyframe*mLastKeyframe;
    const TimeInterpolator*mInterpolator; // only used in the 2-keyframe case
    std::vector<Keyframe*>mKeyframes;
    TypeEvaluatorFn mEvaluator;
    AnimateValue mAnimatedValue;   // scratch backing getValue()
public:
    KeyframeSet(const std::vector<Keyframe*>& keyframes);
    virtual ~KeyframeSet();
    std::vector<Keyframe*>& getKeyframes()override;
    static KeyframeSet*ofInt(const std::vector<int>& values);
    static KeyframeSet*ofFloat(const std::vector<float>& values);
    static KeyframeSet*ofKeyframe(const std::vector<Keyframe*>& keyframes);
    static KeyframeSet*ofObject(const std::vector<AnimateValue>& values);
    void setEvaluator(TypeEvaluatorFn evaluator)override;
    int getType()const override;
    KeyframeSet*clone()const override;
    const AnimateValue& getValue(float fraction)override;
    std::string toString()const;
};

class IntKeyframeSet:public KeyframeSet,public virtual IntKeyframes{
public:
    IntKeyframeSet(const std::vector<Keyframe*>& keyframes);
    const AnimateValue& getValue(float fraction)override;
    IntKeyframeSet*clone()const override;
    int getIntValue(float fraction)override;
};

class FloatKeyframeSet:public KeyframeSet,public virtual FloatKeyframes{
public:
    FloatKeyframeSet(const std::vector<Keyframe*>& keyframes);
    const AnimateValue& getValue(float fraction)override;
    FloatKeyframeSet*clone()const override;
    float getFloatValue(float fraction)override;
};

}//endof namespace
#endif/*__ANIMATION_KEYFRAMESET_H__*/
