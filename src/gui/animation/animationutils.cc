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
    return nullptr;
}

Animation* AnimationUtils::makeOutAnimation(Context* c, bool toRight){
    return nullptr;
}

Animation* AnimationUtils::makeInChildBottomAnimation(Context* c){
    return nullptr;
}

Interpolator* AnimationUtils::loadInterpolator(Context* context,const std::string&id){
    return nullptr;
}

}
