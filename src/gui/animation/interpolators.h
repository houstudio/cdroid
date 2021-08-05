#ifndef __INTERPOLATOR_H__
#define __INTERPOLATOR_H__
#include <math.h>
#include <vector>

namespace cdroid{

class Interpolator{
public:
    virtual float getInterpolation(float input)=0;
};

class LinearInterpolator:public Interpolator{
public:
    float getInterpolation(float input){return input;}
};

class AccelerateInterpolator:public Interpolator{
private:
    float mFactor;
    double mDoubleFactor;
public:
    AccelerateInterpolator(double f=1.){
       mFactor=f;
       mDoubleFactor=f*2;
    }
    float getInterpolation(float input) {
        if (mFactor == 1.0f) {
            return input * input;
        } else {
            return (float)pow(input, mDoubleFactor);
        }
    }
};

class DecelerateInterpolator:public Interpolator{
private:
    float mFactor;
public:
    DecelerateInterpolator(float factor=1.0) {
        mFactor = factor;
    }
    float getInterpolation(float input) {
        float result;
        if (mFactor == 1.0f) {
            result = (float)(1.0f - (1.0f - input) * (1.0f - input));
        } else {
            result = (float)(1.0f - pow((1.0f - input), 2 * mFactor));
        }
        return result;
    }
};

class AnticipateInterpolator:public Interpolator{
private:
    float mTension;
public:
    AnticipateInterpolator(float tension=2.0) {
        mTension = tension;
    }
    float getInterpolation(float t) {
        return t * t * ((mTension + 1) * t - mTension);
    }
};

class CycleInterpolator:public Interpolator{
private:
    float mCycles;
public:
    CycleInterpolator(float cycles) {
        mCycles = cycles;
    }
     float getInterpolation(float input) {
        return (float)(sin(2 * mCycles * M_PI * input));
    }
};

class OvershootInterpolator:public Interpolator{
private:
    float mTension;
public:
    OvershootInterpolator(float tension=2.0f) {
        mTension = tension;
    }
    float getInterpolation(float t) {
        t -= 1.0f;
        return t * t * ((mTension + 1) * t + mTension) + 1.0f;
    }
};

/**
 * An interpolator where the change starts backward then flings forward and overshoots
 * the target value and finally goes back to the final value.
 */
class AnticipateOvershootInterpolator:public Interpolator{
private:
    float mTension;
    static float a(float t, float s) {
        return t * t * ((s + 1) * t - s);
    }
    static float o(float t, float s) {
        return t * t * ((s + 1) * t + s);
    }
public:
    AnticipateOvershootInterpolator(float tension=2.5, float extraTension=1.5) {
        mTension = tension * extraTension;
    }

    float getInterpolation(float t) {
        if (t < 0.5f) return 0.5f * a(t * 2.0f, mTension);
        else return 0.5f * (o(t * 2.0f - 2.0f, mTension) + 2.0f);
    }
};

class BounceInterpolator:public Interpolator{
private:
    static float bounce(float t) {
        return t * t * 8.0f;
    }
public:
     float getInterpolation(float t) {
        // _b(t) = t * t * 8
        // bs(t) = _b(t) for t < 0.3535
        // bs(t) = _b(t - 0.54719) + 0.7 for t < 0.7408
        // bs(t) = _b(t - 0.8526) + 0.9 for t < 0.9644
        // bs(t) = _b(t - 1.0435) + 0.95 for t <= 1.0
        // b(t) = bs(t * 1.1226)
        t *= 1.1226f;
        if (t < 0.3535f) return bounce(t);
        else if (t < 0.7408f) return bounce(t - 0.54719f) + 0.7f;
        else if (t < 0.9644f) return bounce(t - 0.8526f) + 0.9f;
        else return bounce(t - 1.0435f) + 0.95f;
    }
};

class AccelerateDecelerateInterpolator:public Interpolator{
public:
    float getInterpolation(float input) {
        return (float)(cos((input + 1) * M_PI) / 2.0f) + 0.5f;
    }
};


class PathInterpolator:public Interpolator{
private:
    std::vector<float>mX;
    std::vector<float>mY;
#if 0
//https://www.androidos.net.cn/android/9.0.0_r8/xref/frameworks/base/core/java/android/view/animation/PathInterpolator.java
    PathInterpolator(float controlX, float controlY) {
        initQuad(controlX, controlY);
    }
    PathInterpolator(float controlX1, float controlY1, float controlX2, float controlY2) {
        initCubic(controlX1, controlY1, controlX2, controlY2);
    }
    void initQuad(float controlX, float controlY);
    void initCubic(float x1, float y1, float x2, float y2);
    float getInterpolation(float t);
#endif
};
}//namespace 

#endif
