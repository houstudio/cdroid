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
#ifndef __INTERPOLATOR_H__
#define __INTERPOLATOR_H__
#include <math.h>
#include <vector>
#include <core/path.h>
#include <core/attributeset.h>
#include <core/neverdestroyed.h>
namespace cdroid{
class Context;

class TimeInterpolator{
public:
    virtual float getInterpolation(float input)=0;
    virtual ~TimeInterpolator()=default;
};

class Interpolator:public TimeInterpolator{
public:
    static bool isSystemGlobalInterpolator(TimeInterpolator*i);
};

class BaseInterpolator:public Interpolator{
private:
    int mChangingConfiguration;
public:
    int getChangingConfiguration();
    void setChangingConfiguration(int changingConfiguration);
};

class LinearInterpolator:public BaseInterpolator{
public:
    static const NeverDestroyed<LinearInterpolator>gLinearInterpolator;
    float getInterpolation(float input){return input;}
};

class AccelerateInterpolator:public BaseInterpolator{
private:
    float mFactor;
    double mDoubleFactor;
public:
    static const NeverDestroyed<AccelerateInterpolator>gAccelerateInterpolator;
    AccelerateInterpolator(Context*ctx,const AttributeSet&);
    AccelerateInterpolator(double f=1.);
    float getInterpolation(float input)override;
};

class DecelerateInterpolator:public BaseInterpolator{
private:
    float mFactor;
public:
    static const NeverDestroyed<DecelerateInterpolator>gDecelerateInterpolator;
    DecelerateInterpolator(Context*ctx,const AttributeSet&);
    DecelerateInterpolator(float factor=1.0);
    float getInterpolation(float input)override;
};

class AnticipateInterpolator:public BaseInterpolator{
private:
    float mTension;
public:
    AnticipateInterpolator(Context*ctx,const AttributeSet&);
    AnticipateInterpolator(float tension=2.0);
    float getInterpolation(float t)override;
};

class CycleInterpolator:public BaseInterpolator{
private:
    float mCycles;
public:
    CycleInterpolator(Context*ctx,const AttributeSet&);
    CycleInterpolator(float cycles);
    float getInterpolation(float input)override;
};

class OvershootInterpolator:public BaseInterpolator{
private:
    float mTension;
public:
    OvershootInterpolator(Context*ctx,const AttributeSet&);
    OvershootInterpolator(float tension=2.0f);
    float getInterpolation(float t)override;
};

/**
 * An interpolator where the change starts backward then flings forward and overshoots
 * the target value and finally goes back to the final value.
 */
class AnticipateOvershootInterpolator:public BaseInterpolator{
private:
    float mTension;
    static float a(float t, float s);
    static float o(float t, float s);
public:
    AnticipateOvershootInterpolator(Context*ctx,const AttributeSet&);
    AnticipateOvershootInterpolator(float tension=2.5, float extraTension=1.5);

    float getInterpolation(float t)override;
};

class BounceInterpolator:public BaseInterpolator{
private:
    static float bounce(float t);
public:
    float getInterpolation(float t)override;
};

class AccelerateDecelerateInterpolator:public BaseInterpolator{
public:
    static const NeverDestroyed<AccelerateDecelerateInterpolator>gAccelerateDecelerateInterpolator;
    float getInterpolation(float input)override;
};


class PathInterpolator:public BaseInterpolator{
private:
    static constexpr float PRECISION = 0.002f;
    std::vector<float>mX;
    std::vector<float>mY;
private:
    //https://www.androidos.net.cn/android/9.0.0_r8/xref/frameworks/base/core/java/android/view/animation/PathInterpolator.java
    void initQuad(float controlX, float controlY);
    void initCubic(float x1, float y1, float x2, float y2);
    void initPath(cdroid::Path&path);
public:
    PathInterpolator(float controlX, float controlY);
    PathInterpolator(float controlX1, float controlY1, float controlX2, float controlY2);
    PathInterpolator(cdroid::Path&path);
    PathInterpolator(Context*,const AttributeSet&);
    ~PathInterpolator()override;
    float getInterpolation(float t)override;
};

class BackGestureInterpolator:public PathInterpolator{
public:
    BackGestureInterpolator();
};

class LookupTableInterpolator:public BaseInterpolator{
private:
    std::vector<float> mValues;
    float mStepSize;
protected:
    LookupTableInterpolator(const std::vector<float>&values);
    LookupTableInterpolator(const float*values,int count);
public:
    float getInterpolation(float input)override;
    ~LookupTableInterpolator()override;
};

class FastOutSlowInInterpolator :public LookupTableInterpolator{
public:
    static const NeverDestroyed<FastOutSlowInInterpolator>gFastOutSlowInInterpolator;
    FastOutSlowInInterpolator();
};

class LinearOutSlowInInterpolator:public LookupTableInterpolator{
public:
    static const NeverDestroyed<LinearOutSlowInInterpolator>gLinearOutSlowInInterpolator;
    LinearOutSlowInInterpolator();
};

class FastOutLinearInInterpolator:public LookupTableInterpolator{
public:
    static const NeverDestroyed<FastOutLinearInInterpolator>gFastOutLinearInInterpolator;
    FastOutLinearInInterpolator();
};

class BezierSCurveInterpolator:public TimeInterpolator {
public:
    static const NeverDestroyed<BezierSCurveInterpolator>gBezierSCurveInterpolator;
public:
    BezierSCurveInterpolator();
    float getInterpolation(float input)override;
};
}//namespace 

#endif
