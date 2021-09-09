#include <animation/animationutils.h>
#include <core/systemclock.h>

namespace cdroid{

long AnimationUtils::currentAnimationTimeMillis(){
    return SystemClock::uptimeMillis();
}

Animation* AnimationUtils::loadAnimation(Context* context,const std::string&id){
    return nullptr;
}

LayoutAnimationController* AnimationUtils::loadLayoutAnimation(Context* context,const std::string&id){
    return nullptr;
}

Animation* AnimationUtils::makeInAnimation(Context* c, bool fromLeft){
    Animation*a=loadAnimation(c,fromLeft?"cdroid:anim/slide_in_left.xml":"cdroid:anim/slide_in_right.xml");
    a->setInterpolator(new DecelerateInterpolator());
    a->setStartTime(currentAnimationTimeMillis());
    return a;
}

Animation* AnimationUtils::makeOutAnimation(Context* c, bool toRight){
    Animation*a=loadAnimation(c,toRight?"cdroid:anim/slide_out_right.xml":"cdroid:anim/slide_out_left.xml");
    a->setInterpolator(new AccelerateInterpolator());
    a->setStartTime(currentAnimationTimeMillis());
    return a;
}

Animation* AnimationUtils::makeInChildBottomAnimation(Context* c){
    Animation*a = loadAnimation(c,"cdroid:anim/slide_in_child_bottom.xml");
    a->setInterpolator(new AccelerateInterpolator());
    a->setStartTime(currentAnimationTimeMillis());
    return a;
}

Interpolator* AnimationUtils::loadInterpolator(Context* context,const std::string&id){
    return nullptr;
}

}
