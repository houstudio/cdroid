#ifndef __FRAGMENTANIM_H__
#define __FRAGMENTANIM_H__
/*********************************************************************************
 * Port of androidx.fragment.app.FragmentAnim. Resolves the enter/exit Animation for
 * a Fragment transition. MVP uses default fade (AlphaAnimation); loading from custom
 * animation resources (setCustomAnimations) can be added later.
 *********************************************************************************/
#include <animation/animation.h>
namespace cdroid{
namespace fragment{
class Fragment;
class FragmentAnim{
public:
    static Animation* loadEnterAnimation(Fragment* fragment);
    static Animation* loadExitAnimation(Fragment* fragment);
};
}//namespace fragment
}//namespace cdroid
#endif
