#ifndef __SCALEANIMATION_H__
#define __SCALEANIMATION_H__
#include <animation/animation.h>
#include <typedvalue.h>

namespace cdroid{
class ScaleAnimation:public Animation{
private :
    float mFromX;
    float mToX;
    float mFromY;
    float mToY;

    int mFromXType= TypedValue::TYPE_NULL;
    int mToXType  = TypedValue::TYPE_NULL;
    int mFromYType= TypedValue::TYPE_NULL;
    int mToYType  = TypedValue::TYPE_NULL;

    int mFromXData = 0;
    int mToXData = 0;
    int mFromYData = 0;
    int mToYData = 0;

    int mPivotXType = ABSOLUTE;
    int mPivotYType = ABSOLUTE;
    float mPivotXValue = 0.0f;
    float mPivotYValue = 0.0f;

    float mPivotX;
    float mPivotY;
private:
    void initializePivotPoint();
protected:
    void applyTransformation(float interpolatedTime, Transformation& t)override;
    float resolveScale(float scale, int type, int data, int size, int psize);
public:
    ScaleAnimation(Context* context,const AttributeSet& attrs);
    ScaleAnimation(float fromX, float toX, float fromY, float toY);
    ScaleAnimation(float fromX, float toX, float fromY, float toY,
            float pivotX, float pivotY);
    ScaleAnimation(float fromX, float toX, float fromY, float toY,
            int pivotXType, float pivotXValue, int pivotYType, float pivotYValue);
    void initialize(int width, int height, int parentWidth, int parentHeight)override;
};

}

#endif
