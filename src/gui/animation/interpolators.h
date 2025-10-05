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
namespace cdroid{
class Context;
/* Interploater is owned by caller.
 * All cdroid's internal interpolators is goblal pointer which is  not freeable
 * */
class TimeInterpolator{
public:
    virtual float getInterpolation(float input)const=0;
    virtual ~TimeInterpolator()=default;
};

using Interpolator=TimeInterpolator;

class BaseInterpolator:public Interpolator{
private:
    int mChangingConfiguration;
public:
    BaseInterpolator();
    int getChangingConfiguration();
    void setChangingConfiguration(int changingConfiguration);
};

class LinearInterpolator:public BaseInterpolator{
public:
    static const LinearInterpolator*const Instance;
    LinearInterpolator():BaseInterpolator(){}
    float getInterpolation(float input)const{return input;}
};

class AccelerateInterpolator:public BaseInterpolator{
private:
    float mFactor;
    double mDoubleFactor;
public:
    static const AccelerateInterpolator*const Instance;
    AccelerateInterpolator(Context*ctx,const AttributeSet&);
    AccelerateInterpolator(double f=1.0);
    float getInterpolation(float input)const override;
};

class DecelerateInterpolator:public BaseInterpolator{
private:
    float mFactor;
public:
    static const DecelerateInterpolator*const Instance;
    DecelerateInterpolator(Context*ctx,const AttributeSet&);
    DecelerateInterpolator(float factor=1.0f);
    float getInterpolation(float input)const override;
};

class AnticipateInterpolator:public BaseInterpolator{
private:
    float mTension;
public:
    static const AnticipateInterpolator*const Instance;
    AnticipateInterpolator(Context*ctx,const AttributeSet&);
    AnticipateInterpolator(float tension=2.0f);
    float getInterpolation(float t)const override;
};

class CycleInterpolator:public BaseInterpolator{
private:
    float mCycles;
public:
    CycleInterpolator(Context*ctx,const AttributeSet&);
    CycleInterpolator(float cycles);
    float getInterpolation(float input)const override;
};

class OvershootInterpolator:public BaseInterpolator{
private:
    float mTension;
public:
    static const OvershootInterpolator*const Instance;
    OvershootInterpolator(Context*ctx,const AttributeSet&);
    OvershootInterpolator(float tension=2.0f);
    float getInterpolation(float t)const override;
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
    AnticipateOvershootInterpolator(float tension=2.5f, float extraTension=1.5f);

    float getInterpolation(float t)const override;
};

class BounceInterpolator:public BaseInterpolator{
private:
    static float bounce(float t);
public:
    static const BounceInterpolator*const Instance;
    float getInterpolation(float t)const override;
};

class AccelerateDecelerateInterpolator:public BaseInterpolator{
public:
    static const AccelerateDecelerateInterpolator*const Instance;
    float getInterpolation(float input)const override;
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
    float getInterpolation(float t)const override;
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
    float getInterpolation(float input)const override;
};

class FastOutSlowInInterpolator :public LookupTableInterpolator{
public:
    static const FastOutSlowInInterpolator*const Instance;
    FastOutSlowInInterpolator();
};

class LinearOutSlowInInterpolator:public LookupTableInterpolator{
public:
    static const LinearOutSlowInInterpolator*const Instance;
    LinearOutSlowInInterpolator();
};

class FastOutLinearInInterpolator:public LookupTableInterpolator{
public:
    static const FastOutLinearInInterpolator*const Instance;
    FastOutLinearInInterpolator();
};

class BezierSCurveInterpolator:public TimeInterpolator {
public:
    static const BezierSCurveInterpolator*const Instance;
public:
    BezierSCurveInterpolator();
    float getInterpolation(float input)const override;
};
}//namespace 

#endif
