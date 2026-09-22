#ifndef __TRANSLATEANIMATION_H__
#define __TRANSLATEANIMATION_H__
#include <animation/animation.h>
namespace cdroid{

class TranslateAnimation :public Animation{
private:
    int mFromXType = ABSOLUTE;
    int mToXType   = ABSOLUTE;
    int mFromYType = ABSOLUTE;
    int mToYType   = ABSOLUTE;
protected:
    float mFromXValue = 0.0f;
    float mToXValue   = 0.0f;
    float mFromYValue = 0.0f;
    float mToYValue   = 0.0f;
    float mFromXDelta;
    float mToXDelta;
    float mFromYDelta;
    float mToYDelta;
    TranslateAnimation(const TranslateAnimation&);
    void applyTransformation(float interpolatedTime, Transformation& t)override;
public:
    TranslateAnimation(Context* context,const AttributeSet& attrs);
    TranslateAnimation(float fromXDelta, float toXDelta, float fromYDelta, float toYDelta);
    TranslateAnimation(int fromXType, float fromXValue, int toXType, float toXValue,
            int fromYType, float fromYValue, int toYType, float toYValue);
    void initialize(int width, int height, int parentWidth, int parentHeight)override;
    TranslateAnimation*clone()const override;
    // CDROID extension: read a delta WITHOUT running initialize() (which also
    // resets timing state). The window-transition parameter-extraction path
    // only needs the sign; a unit size preserves it for every type.
    float resolveFromX(int size, int parentSize) { return resolveSize(mFromXType, mFromXValue, size, parentSize); }
    float resolveToX(int size, int parentSize)   { return resolveSize(mToXType, mToXValue, size, parentSize); }
    float resolveFromY(int size, int parentSize) { return resolveSize(mFromYType, mFromYValue, size, parentSize); }
    float resolveToY(int size, int parentSize)   { return resolveSize(mToYType, mToYValue, size, parentSize); }
    // CDROID extension (whole-window transition extraction): the authored
    // deltas WITH their units, so window-level slides can reproduce the
    // resource's own motion (popup_enter_material's 20dp rise) instead of the
    // edge-based full-offscreen fly-in.
    int   fromXType()  const { return mFromXType; }
    float fromXValue() const { return mFromXValue; }
    int   fromYType()  const { return mFromYType; }
    float fromYValue() const { return mFromYValue; }
    int   toXType()    const { return mToXType; }
    float toXValue()   const { return mToXValue; }
    int   toYType()    const { return mToYType; }
    float toYValue()   const { return mToYValue; }
};

class TranslateXAnimation :public TranslateAnimation{
protected:
    void applyTransformation(float interpolatedTime, Transformation& t)override;
public:
    TranslateXAnimation(float fromXDelta, float toXDelta);
    TranslateXAnimation(int fromXType, float fromXValue, int toXType, float toYValue);
};

class TranslateYAnimation :public TranslateAnimation{
protected:
    void applyTransformation(float interpolatedTime, Transformation& t)override;
public:
    TranslateYAnimation(float fromYDelta, float toYDelta);
    TranslateYAnimation(int fromYType, float fromYValue, int toYType, float toYValue);
};

}//end namespace

#endif
