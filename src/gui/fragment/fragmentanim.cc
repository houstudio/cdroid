#include <fragment/fragmentanim.h>
#include <fragment/fragment.h>
#include <animation/alphaanimation.h>
namespace cdroid{
namespace fragment{

Animation* FragmentAnim::loadEnterAnimation(Fragment* /*fragment*/){
    // Default: fade in.
    return new AlphaAnimation(0.0f, 1.0f);
}

Animation* FragmentAnim::loadExitAnimation(Fragment* /*fragment*/){
    // Default: fade out.
    return new AlphaAnimation(1.0f, 0.0f);
}

}//namespace fragment
}//namespace cdroid
