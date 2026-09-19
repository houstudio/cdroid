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
#ifndef __ANIMATION_PATHKEYFRAMES_H__
#define __ANIMATION_PATHKEYFRAMES_H__
// Port of android.animation.PathKeyframes (android-36): keyframes sampled
// from a Path (fraction/x/y triples from Path::approximate) plus the X/Y
// projections used by AnimatorInflater.setupObjectAnimator's path case.
#include <memory>
#include <vector>
#include <animation/keyframes.h>
#include <core/path.h>
namespace cdroid{

// enable_shared_from_this: the X/Y projections share ownership of the parent
// (AOSP inner classes capture the outer instance; GC keeps it alive).
class PathKeyframes:public virtual Keyframes,public std::enable_shared_from_this<PathKeyframes>{
private:
    static constexpr int FRACTION_OFFSET = 0;
    static constexpr int X_OFFSET = 1;
    static constexpr int Y_OFFSET = 2;
    static constexpr int NUM_COMPONENTS = 3;
    std::vector<float> mKeyframeData;
    PointF mTempPointF;
    AnimateValue mAnimatedValue;
    const PointF& pointForIndex(int index);
    const PointF& interpolateInRange(float fraction,int startIndex,int endIndex);
    static float interpolate(float fraction,float startValue,float endValue);
    static std::vector<Keyframe*>& emptyKeyframes();
public:
    // AOSP SimpleKeyframes subclasses projecting one coordinate of the path.
    class FloatKeyframesBase;
    class IntKeyframesBase;
    class XFloatKeyframes;
    class YFloatKeyframes;
    class XIntKeyframes;
    class YIntKeyframes;

    PathKeyframes(const Cairo::RefPtr<Path>&path,float error = 0.5f);

    std::vector<Keyframe*>& getKeyframes()override;
    const AnimateValue& getValue(float fraction)override;
    void setEvaluator(TypeEvaluatorFn evaluator)override;
    int getType()const override;
    Keyframes*clone()const override;
    // The coordinate projections read the sampled point directly (AOSP inner
    // classes call the outer getValue()).
    const PointF& pointForFraction(float fraction);

    FloatKeyframes*createXFloatKeyframes();
    FloatKeyframes*createYFloatKeyframes();
    IntKeyframes*createXIntKeyframes();
    IntKeyframes*createYIntKeyframes();
};

class PathKeyframes::FloatKeyframesBase:public virtual FloatKeyframes{
protected:
    std::shared_ptr<PathKeyframes>mPath;
    AnimateValue mValue;
public:
    FloatKeyframesBase(const std::shared_ptr<PathKeyframes>&path);
    void setEvaluator(TypeEvaluatorFn evaluator)override;   // no-op (AOSP)
    std::vector<Keyframe*>& getKeyframes()override;
    int getType()const override;
    const AnimateValue& getValue(float fraction)override;
};

class PathKeyframes::IntKeyframesBase:public virtual IntKeyframes{
protected:
    std::shared_ptr<PathKeyframes>mPath;
    AnimateValue mValue;
public:
    IntKeyframesBase(const std::shared_ptr<PathKeyframes>&path);
    void setEvaluator(TypeEvaluatorFn evaluator)override;   // no-op (AOSP)
    std::vector<Keyframe*>& getKeyframes()override;
    int getType()const override;
    const AnimateValue& getValue(float fraction)override;
};

class PathKeyframes::XFloatKeyframes:public PathKeyframes::FloatKeyframesBase{
public:
    XFloatKeyframes(const std::shared_ptr<PathKeyframes>&path);
    float getFloatValue(float fraction)override;
    Keyframes*clone()const override;
};

class PathKeyframes::YFloatKeyframes:public PathKeyframes::FloatKeyframesBase{
public:
    YFloatKeyframes(const std::shared_ptr<PathKeyframes>&path);
    float getFloatValue(float fraction)override;
    Keyframes*clone()const override;
};

class PathKeyframes::XIntKeyframes:public PathKeyframes::IntKeyframesBase{
public:
    XIntKeyframes(const std::shared_ptr<PathKeyframes>&path);
    int getIntValue(float fraction)override;
    Keyframes*clone()const override;
};

class PathKeyframes::YIntKeyframes:public PathKeyframes::IntKeyframesBase{
public:
    YIntKeyframes(const std::shared_ptr<PathKeyframes>&path);
    int getIntValue(float fraction)override;
    Keyframes*clone()const override;
};

}//endof namespace
#endif/*__ANIMATION_PATHKEYFRAMES_H__*/
