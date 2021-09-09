#pragma once
#include <animation/layoutanimationcontroller.h>
namespace cdroid{

class AnimationUtils{
public:
    static long currentAnimationTimeMillis();
    static Animation* loadAnimation(Context* context,const std::string&id);
    static LayoutAnimationController* loadLayoutAnimation(Context* context,const std::string&id);
    static Animation* makeInAnimation(Context* c, bool fromLeft);
    static Animation* makeOutAnimation(Context* c, bool toRight);
    static Animation* makeInChildBottomAnimation(Context* c);
    static Interpolator* loadInterpolator(Context* context,const std::string&id);
};

}//endof namespace

