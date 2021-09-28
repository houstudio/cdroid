#pragma once
#include <animation/animation.h>

namespace cdroid{

class ClipRectAnimation:public Animation{
private:
    int mFromLeftType = ABSOLUTE;
    int mFromTopType = ABSOLUTE;
    int mFromRightType = ABSOLUTE;
    int mFromBottomType = ABSOLUTE;

    int mToLeftType = ABSOLUTE;
    int mToTopType = ABSOLUTE;
    int mToRightType = ABSOLUTE;
    int mToBottomType = ABSOLUTE;

    float mFromLeftValue;
    float mFromTopValue;
    float mFromRightValue;
    float mFromBottomValue;

    float mToLeftValue;
    float mToTopValue;
    float mToRightValue;
    float mToBottomValue;
protected:
    Rect mFromRect;
    Rect mToRect;
    void applyTransformation(float it, Transformation& tr)override;
    ClipRectAnimation(const ClipRectAnimation&);
public:
    ClipRectAnimation(Context* context, const AttributeSet& attrs);
    ClipRectAnimation(const Rect& fromClip,const Rect& toClip);
    ClipRectAnimation(int fromL, int fromT, int fromR, int fromB,
            int toL, int toT, int toR, int toB);
    bool willChangeTransformationMatrix()const override;
    void initialize(int width, int height, int parentWidth, int parentHeight)override;
    Animation*clone()override;
};

}//endif namespace
