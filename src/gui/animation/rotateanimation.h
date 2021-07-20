#pragma once

#include <animation/animation.h>
namespace cdroid{

class RotateAnimation:public Animation{
private:
    float mFromDegrees;
    float mToDegrees;

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
public:
    RotateAnimation(float fromDegrees, float toDegrees);
    RotateAnimation(float fromDegrees, float toDegrees, float pivotX, float pivotY);
    RotateAnimation(float fromDegrees, float toDegrees, int pivotXType, float pivotXValue,
            int pivotYType, float pivotYValue);
    void initialize(int width, int height, int parentWidth, int parentHeight);
};

}
