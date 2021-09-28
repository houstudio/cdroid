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
    Animation*clone()override;
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
