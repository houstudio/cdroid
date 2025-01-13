#ifndef __ANIMATION_UTILS_H__
#define __ANIMATION_UTILS_H__
#include <memory>
#include <animation/layoutanimationcontroller.h>
namespace cdroid{

class AnimationUtils{
private:
    static std::unordered_map<std::string,std::shared_ptr<Interpolator>>mInterpolators;
public:
    static int64_t currentAnimationTimeMillis();
    static Animation* loadAnimation(Context* context,const std::string&id);
    static LayoutAnimationController* loadLayoutAnimation(Context* context,const std::string&id);
    static Animation* makeInAnimation(Context* c, bool fromLeft);
    static Animation* makeOutAnimation(Context* c, bool toRight);
    static Animation* makeInChildBottomAnimation(Context* c);
    static Interpolator* loadInterpolator(Context* context,const std::string&id);
};

}//endof namespace

#endif/*__ANIMATION_UTILS_H__*/
