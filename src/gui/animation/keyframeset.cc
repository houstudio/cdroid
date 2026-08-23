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
#include <animation/keyframeset.h>
#include <porting/cdlog.h>
#include <algorithm>
#include <cmath>

namespace cdroid{

// AOSP's Int/Float KeyframeSets contain a `mEvaluator == null ? mEvaluator.evaluate(...)`
// branch — an upstream typo that would NPE if the evaluator were really null
// (callers always set one). The port falls back to linear interpolation
// instead of crashing on a null evaluator.
static AnimateValue& defaultIntEvaluator(float fraction,AnimateValue&out,
        const AnimateValue&startValue,const AnimateValue&endValue){
    const int from = startValue.index()==0 ? GET_VARIANT(startValue,int) : (int)GET_VARIANT(startValue,float);
    const int to   = endValue.index()  ==0 ? GET_VARIANT(endValue,int)   : (int)GET_VARIANT(endValue,float);
    out = (int)((1.f - fraction)*from + fraction*to);
    return out;
}
static AnimateValue& defaultFloatEvaluator(float fraction,AnimateValue&out,
        const AnimateValue&startValue,const AnimateValue&endValue){
    const float from = startValue.index()==0 ? (float)GET_VARIANT(startValue,int) : GET_VARIANT(startValue,float);
    const float to   = endValue.index()  ==0 ? (float)GET_VARIANT(endValue,int)   : GET_VARIANT(endValue,float);
    out = from*(1.f - fraction) + to*fraction;
    return out;
}

KeyframeSet::KeyframeSet(const std::vector<Keyframe*>& keyframes){
    mNumKeyframes = (int)keyframes.size();
    mKeyframes = keyframes;
    mFirstKeyframe = keyframes[0];
    mLastKeyframe = keyframes[mNumKeyframes - 1];
    mInterpolator = mLastKeyframe->getInterpolator();
    mEvaluator = nullptr;
}

KeyframeSet::~KeyframeSet(){
    for(Keyframe*kf:mKeyframes)
        delete kf;
}

std::vector<Keyframe*>&KeyframeSet::getKeyframes(){
    return mKeyframes;
}

KeyframeSet*KeyframeSet::ofInt(const std::vector<int>& values){
    const int numKeyframes = (int)values.size();
    std::vector<Keyframe*> keyframes(std::max(numKeyframes,2));
    if (numKeyframes == 1) {
        keyframes[0] = Keyframe::ofInt(0.f);
        keyframes[1] = Keyframe::ofInt(1.f, values[0]);
    } else {
        keyframes[0] = Keyframe::ofInt(0.f, values[0]);
        for (int i = 1; i < numKeyframes; ++i) {
            keyframes[i] = Keyframe::ofInt((float) i / (numKeyframes - 1), values[i]);
        }
    }
    return new IntKeyframeSet(keyframes);
}

KeyframeSet*KeyframeSet::ofFloat(const std::vector<float>& values){
    const int numKeyframes = (int)values.size();
    std::vector<Keyframe*> keyframes(std::max(numKeyframes,2));
    if (numKeyframes == 1) {
        keyframes[0] = Keyframe::ofFloat(0.f);
        keyframes[1] = Keyframe::ofFloat(1.f, values[0]);
    } else {
        keyframes[0] = Keyframe::ofFloat(0.f, values[0]);
        for (int i = 1; i < numKeyframes; ++i) {
            keyframes[i] = Keyframe::ofFloat((float) i / (numKeyframes - 1), values[i]);
        }
    }
    return new FloatKeyframeSet(keyframes);
}

KeyframeSet*KeyframeSet::ofKeyframe(const std::vector<Keyframe*>& keyframes){
    // if all keyframes of same primitive type, create the appropriate KeyframeSet
    const int numKeyframes = (int)keyframes.size();
    bool hasFloat = false;
    bool hasInt = false;
    bool hasOther = false;
    for (int i = 0; i < numKeyframes; ++i) {
        if (dynamic_cast<FloatKeyframe*>(keyframes[i])) {
            hasFloat = true;
        } else if (dynamic_cast<IntKeyframe*>(keyframes[i])) {
            hasInt = true;
        } else {
            hasOther = true;
        }
    }
    if (hasFloat && !hasInt && !hasOther) {
        return new FloatKeyframeSet(keyframes);
    } else if (hasInt && !hasFloat && !hasOther) {
        return new IntKeyframeSet(keyframes);
    }
    return new KeyframeSet(keyframes);
}

KeyframeSet*KeyframeSet::ofObject(const std::vector<AnimateValue>& values){
    const int numKeyframes = (int)values.size();
    std::vector<Keyframe*> keyframes(std::max(numKeyframes,2));
    if (numKeyframes == 1) {
        keyframes[0] = Keyframe::ofObject(0.f);
        keyframes[1] = Keyframe::ofObject(1.f, values[0]);
    } else {
        keyframes[0] = Keyframe::ofObject(0.f, values[0]);
        for (int i = 1; i < numKeyframes; ++i) {
            keyframes[i] = Keyframe::ofObject((float) i / (numKeyframes - 1), values[i]);
        }
    }
    return new KeyframeSet(keyframes);
}

void KeyframeSet::setEvaluator(TypeEvaluatorFn evaluator){
    mEvaluator = evaluator;
}

int KeyframeSet::getType()const{
    return mFirstKeyframe->getType();
}

KeyframeSet*KeyframeSet::clone()const{
    const int numKeyframes = (int)mKeyframes.size();
    std::vector<Keyframe*> newKeyframes(numKeyframes);
    for (int i = 0; i < numKeyframes; ++i) {
        newKeyframes[i] = mKeyframes[i]->clone();
    }
    return new KeyframeSet(newKeyframes);
}

const AnimateValue&KeyframeSet::getValue(float fraction){
    // Special-case optimization for the common case of only two keyframes
    if (mNumKeyframes == 2) {
        if (mInterpolator != nullptr) {
            fraction = mInterpolator->getInterpolation(fraction);
        }
        // Non-AOSP robustness: return the opposite endpoint when a keyframe
        // carries no value yet (Java would NPE on the null Integer unboxing).
        if(!mFirstKeyframe->hasValue()) return mAnimatedValue = mLastKeyframe->getValue();
        if(!mLastKeyframe->hasValue())  return mAnimatedValue = mFirstKeyframe->getValue();
        if(mEvaluator == nullptr)       return mAnimatedValue =
                (fraction < 1.f ? mFirstKeyframe : mLastKeyframe)->getValue();
        return (*mEvaluator)(fraction, mAnimatedValue, mFirstKeyframe->getValue(),
                mLastKeyframe->getValue());
    }
    if (fraction <= 0.f) {
        const Keyframe* nextKeyframe = mKeyframes[1];
        const TimeInterpolator* interpolator = nextKeyframe->getInterpolator();
        if (interpolator != nullptr) {
            fraction = interpolator->getInterpolation(fraction);
        }
        const float prevFraction = mFirstKeyframe->getFraction();
        const float intervalFraction = (fraction - prevFraction) /
            (nextKeyframe->getFraction() - prevFraction);
        if(mEvaluator == nullptr) return mAnimatedValue = mFirstKeyframe->getValue();
        return (*mEvaluator)(intervalFraction, mAnimatedValue, mFirstKeyframe->getValue(),
                nextKeyframe->getValue());
    } else if (fraction >= 1.f) {
        const Keyframe* prevKeyframe = mKeyframes[mNumKeyframes - 2];
        const TimeInterpolator* interpolator = mLastKeyframe->getInterpolator();
        if (interpolator != nullptr) {
            fraction = interpolator->getInterpolation(fraction);
        }
        const float prevFraction = prevKeyframe->getFraction();
        const float intervalFraction = (fraction - prevFraction) /
            (mLastKeyframe->getFraction() - prevFraction);
        if(mEvaluator == nullptr) return mAnimatedValue = mLastKeyframe->getValue();
        return (*mEvaluator)(intervalFraction, mAnimatedValue, prevKeyframe->getValue(),
                mLastKeyframe->getValue());
    }
    Keyframe*prevKeyframe = mFirstKeyframe;
    for (int i = 1; i < mNumKeyframes; ++i) {
        Keyframe*nextKeyframe = mKeyframes[i];
        if (fraction < nextKeyframe->getFraction()) {
            const TimeInterpolator* interpolator = nextKeyframe->getInterpolator();
            const float prevFraction = prevKeyframe->getFraction();
            float intervalFraction = (fraction - prevFraction) /
                (nextKeyframe->getFraction() - prevFraction);
            // Apply interpolator on the proportional duration.
            if (interpolator != nullptr) {
                intervalFraction = interpolator->getInterpolation(intervalFraction);
            }
            if(mEvaluator == nullptr) return mAnimatedValue = prevKeyframe->getValue();
            return (*mEvaluator)(intervalFraction, mAnimatedValue, prevKeyframe->getValue(),
                    nextKeyframe->getValue());
        }
        prevKeyframe = nextKeyframe;
    }
    // shouldn't reach here
    return mAnimatedValue = mLastKeyframe->getValue();
}

std::string KeyframeSet::toString()const{
    std::string returnVal = " ";
    for (int i = 0; i < mNumKeyframes; ++i) {
        AnimateValue v = mKeyframes[i]->getValue();
        if(v.index()==0) returnVal += std::to_string(GET_VARIANT(v,int)) + "  ";
        else if(v.index()==1) returnVal += std::to_string(GET_VARIANT(v,float)) + "  ";
    }
    return returnVal;
}

IntKeyframeSet::IntKeyframeSet(const std::vector<Keyframe*>& keyframes)
    :KeyframeSet(keyframes){
}

const AnimateValue&IntKeyframeSet::getValue(float fraction){
    mAnimatedValue = getIntValue(fraction);
    return mAnimatedValue;
}

int IntKeyframeSet::getIntValue(float fraction){
    if (fraction <= 0.f) {
        IntKeyframe*prevKeyframe = (IntKeyframe*)mKeyframes[0];
        IntKeyframe*nextKeyframe = (IntKeyframe*)mKeyframes[1];
        const int prevValue = prevKeyframe->getIntValue();
        const int nextValue = nextKeyframe->getIntValue();
        const float prevFraction = prevKeyframe->getFraction();
        const float nextFraction = nextKeyframe->getFraction();
        const TimeInterpolator* interpolator = nextKeyframe->getInterpolator();
        if (interpolator != nullptr) {
            fraction = interpolator->getInterpolation(fraction);
        }
        const float intervalFraction = (fraction - prevFraction) / (nextFraction - prevFraction);
        const TypeEvaluatorFn evaluator = mEvaluator ? mEvaluator : defaultIntEvaluator;
        AnimateValue out = evaluator(intervalFraction, mAnimatedValue, prevValue, nextValue);
        return out.index()==0 ? GET_VARIANT(out,int) : (int)GET_VARIANT(out,float);
    } else if (fraction >= 1.f) {
        IntKeyframe*prevKeyframe = (IntKeyframe*)mKeyframes[mNumKeyframes - 2];
        IntKeyframe*nextKeyframe = (IntKeyframe*)mKeyframes[mNumKeyframes - 1];
        const int prevValue = prevKeyframe->getIntValue();
        const int nextValue = nextKeyframe->getIntValue();
        const float prevFraction = prevKeyframe->getFraction();
        const float nextFraction = nextKeyframe->getFraction();
        const TimeInterpolator* interpolator = nextKeyframe->getInterpolator();
        if (interpolator != nullptr) {
            fraction = interpolator->getInterpolation(fraction);
        }
        const float intervalFraction = (fraction - prevFraction) / (nextFraction - prevFraction);
        const TypeEvaluatorFn evaluator = mEvaluator ? mEvaluator : defaultIntEvaluator;
        AnimateValue out = evaluator(intervalFraction, mAnimatedValue, prevValue, nextValue);
        return out.index()==0 ? GET_VARIANT(out,int) : (int)GET_VARIANT(out,float);
    }
    IntKeyframe*prevKeyframe = (IntKeyframe*)mKeyframes[0];
    for (int i = 1; i < mNumKeyframes; ++i) {
        IntKeyframe*nextKeyframe = (IntKeyframe*)mKeyframes[i];
        if (fraction < nextKeyframe->getFraction()) {
            const TimeInterpolator* interpolator = nextKeyframe->getInterpolator();
            const float intervalFraction = (fraction - prevKeyframe->getFraction()) /
                (nextKeyframe->getFraction() - prevKeyframe->getFraction());
            const int prevValue = prevKeyframe->getIntValue();
            const int nextValue = nextKeyframe->getIntValue();
            // Apply interpolator on the proportional duration.
            float interpFraction = intervalFraction;
            if (interpolator != nullptr) {
                interpFraction = interpolator->getInterpolation(intervalFraction);
            }
            const TypeEvaluatorFn evaluator = mEvaluator ? mEvaluator : defaultIntEvaluator;
            AnimateValue out = evaluator(interpFraction, mAnimatedValue, prevValue, nextValue);
            return out.index()==0 ? GET_VARIANT(out,int) : (int)GET_VARIANT(out,float);
        }
        prevKeyframe = nextKeyframe;
    }
    // shouldn't get here
    return GET_VARIANT(mKeyframes[mNumKeyframes - 1]->getValue(),int);
}

IntKeyframeSet*IntKeyframeSet::clone()const{
    const int numKeyframes = (int)mKeyframes.size();
    std::vector<Keyframe*> newKeyframes(numKeyframes);
    for (int i = 0; i < numKeyframes; ++i) {
        newKeyframes[i] = mKeyframes[i]->clone();
    }
    return new IntKeyframeSet(newKeyframes);
}

FloatKeyframeSet::FloatKeyframeSet(const std::vector<Keyframe*>& keyframes)
    :KeyframeSet(keyframes){
}

FloatKeyframeSet*FloatKeyframeSet::clone()const{
    const int numKeyframes = (int)mKeyframes.size();
    std::vector<Keyframe*> newKeyframes(numKeyframes);
    for (int i = 0; i < numKeyframes; ++i) {
        newKeyframes[i] = mKeyframes[i]->clone();
    }
    return new FloatKeyframeSet(newKeyframes);
}

const AnimateValue&FloatKeyframeSet::getValue(float fraction){
    mAnimatedValue = getFloatValue(fraction);
    return mAnimatedValue;
}

float FloatKeyframeSet::getFloatValue(float fraction){
    if (fraction <= 0.f) {
        FloatKeyframe*prevKeyframe = (FloatKeyframe*)mKeyframes[0];
        FloatKeyframe*nextKeyframe = (FloatKeyframe*)mKeyframes[1];
        const float prevValue = prevKeyframe->getFloatValue();
        const float nextValue = nextKeyframe->getFloatValue();
        const float prevFraction = prevKeyframe->getFraction();
        const float nextFraction = nextKeyframe->getFraction();
        const TimeInterpolator* interpolator = nextKeyframe->getInterpolator();
        if (interpolator != nullptr) {
            fraction = interpolator->getInterpolation(fraction);
        }
        const float intervalFraction = (fraction - prevFraction) / (nextFraction - prevFraction);
        const TypeEvaluatorFn evaluator = mEvaluator ? mEvaluator : defaultFloatEvaluator;
        AnimateValue out = evaluator(intervalFraction, mAnimatedValue, prevValue, nextValue);
        return out.index()==1 ? GET_VARIANT(out,float) : (float)GET_VARIANT(out,int);
    } else if (fraction >= 1.f) {
        FloatKeyframe*prevKeyframe = (FloatKeyframe*)mKeyframes[mNumKeyframes - 2];
        FloatKeyframe*nextKeyframe = (FloatKeyframe*)mKeyframes[mNumKeyframes - 1];
        const float prevValue = prevKeyframe->getFloatValue();
        const float nextValue = nextKeyframe->getFloatValue();
        const float prevFraction = prevKeyframe->getFraction();
        const float nextFraction = nextKeyframe->getFraction();
        const TimeInterpolator* interpolator = nextKeyframe->getInterpolator();
        if (interpolator != nullptr) {
            fraction = interpolator->getInterpolation(fraction);
        }
        const float intervalFraction = (fraction - prevFraction) / (nextFraction - prevFraction);
        const TypeEvaluatorFn evaluator = mEvaluator ? mEvaluator : defaultFloatEvaluator;
        AnimateValue out = evaluator(intervalFraction, mAnimatedValue, prevValue, nextValue);
        return out.index()==1 ? GET_VARIANT(out,float) : (float)GET_VARIANT(out,int);
    }
    FloatKeyframe*prevKeyframe = (FloatKeyframe*)mKeyframes[0];
    for (int i = 1; i < mNumKeyframes; ++i) {
        FloatKeyframe*nextKeyframe = (FloatKeyframe*)mKeyframes[i];
        if (fraction < nextKeyframe->getFraction()) {
            const TimeInterpolator* interpolator = nextKeyframe->getInterpolator();
            const float intervalFraction = (fraction - prevKeyframe->getFraction()) /
                (nextKeyframe->getFraction() - prevKeyframe->getFraction());
            const float prevValue = prevKeyframe->getFloatValue();
            const float nextValue = nextKeyframe->getFloatValue();
            // Apply interpolator on the proportional duration.
            float interpFraction = intervalFraction;
            if (interpolator != nullptr) {
                interpFraction = interpolator->getInterpolation(intervalFraction);
            }
            const TypeEvaluatorFn evaluator = mEvaluator ? mEvaluator : defaultFloatEvaluator;
            AnimateValue out = evaluator(interpFraction, mAnimatedValue, prevValue, nextValue);
            return out.index()==1 ? GET_VARIANT(out,float) : (float)GET_VARIANT(out,int);
        }
        prevKeyframe = nextKeyframe;
    }
    // shouldn't get here
    return GET_VARIANT(mKeyframes[mNumKeyframes - 1]->getValue(),float);
}

}//endof namespace
