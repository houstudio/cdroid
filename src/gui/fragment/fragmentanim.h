#ifndef __FRAGMENTANIM_H__
#define __FRAGMENTANIM_H__
/*********************************************************************************
 * Port of androidx.fragment.app.FragmentAnim. Resolves the enter/exit Animation for
 * a Fragment transition. MVP uses default fade (AlphaAnimation); loading from custom
 * animation resources (setCustomAnimations) can be added later.
 *********************************************************************************/
#include <animation/animation.h>
#include <string>
namespace cdroid{
class Context;
namespace fragment{
class Fragment;
class FragmentAnim{
public:
    // Legacy MVP default fade (kept for compatibility).
    static Animation* loadEnterAnimation(Fragment* fragment);
    static Animation* loadExitAnimation(Fragment* fragment);
    // Resolve the custom-anim Animation for a fragment transition (androidx FragmentAnim.loadAnimation).
    // enter = finalState==VISIBLE (add/show); isPop = back-stack reverse. Returns null if no anim set.
    static Animation* loadAnimation(Context* context, Fragment* fragment, bool enter, bool isPop);
private:
    // 4-anim matrix: forward enter/exit, pop popEnter/popExit (androidx FragmentAnim.getNextAnim).
    static std::string getNextAnim(Fragment* fragment, bool enter, bool isPop);
};
}//namespace fragment
}//namespace cdroid
#endif
