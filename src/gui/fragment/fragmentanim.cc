#include <fragment/fragmentanim.h>
#include <fragment/fragment.h>
#include <animation/alphaanimation.h>
#include <animation/animationutils.h>
#include <core/context.h>
namespace cdroid{
namespace fragment{

Animation* FragmentAnim::loadEnterAnimation(Fragment* /*fragment*/){
    // Legacy default: fade in.
    return new AlphaAnimation(0.0f, 1.0f);
}

Animation* FragmentAnim::loadExitAnimation(Fragment* /*fragment*/){
    // Legacy default: fade out.
    return new AlphaAnimation(1.0f, 0.0f);
}

Animation* FragmentAnim::loadAnimation(Context* context, Fragment* fragment, bool enter, bool isPop){
    std::string anim = getNextAnim(fragment, enter, isPop);
    if(anim.empty()) return nullptr;
    return AnimationUtils::loadAnimation(context, anim);
}

std::string FragmentAnim::getNextAnim(Fragment* fragment, bool enter, bool isPop){
    if(!fragment) return std::string();
    // enter(=ADDING/VISIBLE) + forward -> mEnterAnim;  enter + pop -> mPopEnterAnim;
    // exit(=REMOVING)        + forward -> mExitAnim;   exit  + pop -> mPopExitAnim.
    if(isPop) return enter ? fragment->mPopEnterAnim : fragment->mPopExitAnim;
    return enter ? fragment->mEnterAnim : fragment->mExitAnim;
}

}//namespace fragment
}//namespace cdroid
