#include<animation/alphaanimation.h>

namespace cdroid{

AlphaAnimation::AlphaAnimation(Context* context,const AttributeSet& attrs):Animation(context,attrs){
}

AlphaAnimation::AlphaAnimation(float fromAlpha, float toAlpha){
    mFromAlpha = fromAlpha;
    mToAlpha = toAlpha;
}

void AlphaAnimation::applyTransformation(float interpolatedTime, Transformation& t) {
    float alpha = mFromAlpha;
    t.setAlpha(alpha + ((mToAlpha - alpha) * interpolatedTime));
}

bool AlphaAnimation::willChangeTransformationMatrix()const{
    return false;
}

bool AlphaAnimation::willChangeBounds()const{
    return false;
}

bool AlphaAnimation::hasAlpha()const{
    return true;
}
}
