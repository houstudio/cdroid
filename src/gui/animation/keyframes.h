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
#ifndef __ANIMATION_KEYFRAMES_H__
#define __ANIMATION_KEYFRAMES_H__
// Port of android.animation.Keyframes (android-36) — the interface every
// keyframe collection implements (KeyframeSet and friends). Header-only.
#include <vector>
#include <animation/keyframe.h>
#include <animation/property.h>

namespace cdroid{

// The free-function evaluator type PropertyValuesHolder uses; defined here so
// the Keyframes interface can name it without depending on PHV.
using TypeEvaluatorFn = AnimateValue&(*)(float fraction,AnimateValue&out,
                                        const AnimateValue&startValue,const AnimateValue&endValue);

class Keyframes{
public:
    virtual ~Keyframes()=default;
    virtual void setEvaluator(TypeEvaluatorFn evaluator)=0;
    virtual int getType()const=0;
    // Returns a scratch-buffer-backed value (AOSP returns the same mutable
    // Object across calls); valid until the next getValue on this set.
    virtual const AnimateValue& getValue(float fraction)=0;
    virtual std::vector<Keyframe*>& getKeyframes()=0;
    virtual Keyframes*clone()const=0;
};

// Virtual base: AOSP's `class X extends KeyframeSet implements IntKeyframes`
// maps to C++ multiple inheritance over the shared Keyframes interface.
class IntKeyframes:public virtual Keyframes{
public:
    virtual int getIntValue(float fraction)=0;
};

class FloatKeyframes:public virtual Keyframes{
public:
    virtual float getFloatValue(float fraction)=0;
};

}//endof namespace
#endif/*__ANIMATION_KEYFRAMES_H__*/
