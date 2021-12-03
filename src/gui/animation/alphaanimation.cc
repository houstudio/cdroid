#include<animation/alphaanimation.h>

namespace cdroid{

AlphaAnimation::AlphaAnimation(Context* context,const AttributeSet& attrs):Animation(context,attrs){
    mFromAlpha = attrs.getFloat("fromAlpha",1.f);
    mToAlpha   = attrs.getFloat("toAlpha",1.f);
}

AlphaAnimation::AlphaAnimation(float fromAlpha, float toAlpha){
    mFromAlpha = fromAlpha;
    mToAlpha = toAlpha;
}

void AlphaAnimation::applyTransformation(float interpolatedTime, Transformation& t) {
    t.setAlpha(mFromAlpha + ((mToAlpha - mFromAlpha) * interpolatedTime));
}

bool AlphaAnimation::willChangeTransformationMatrix()const{
    return false;
}

bool AlphaAnimation::willChangeBounds()const{
    return false;
}

bool AlphaAnimation::hasAlpha(){
    return true;
}
}
