#ifndef __FRAGMENTTRANSITIONIMPL_H__
#define __FRAGMENTTRANSITIONIMPL_H__
/*********************************************************************************
 * Port of androidx.fragment.app.FragmentTransitionImpl. Builds the Transition for a
 * fragment enter/exit, including shared-element transitions (ChangeBounds) when shared
 * views are mapped. Uses CDROID's android.transition port (transition/).
 *********************************************************************************/
#include <map>
#include <string>
#include <transition/transition.h>
namespace cdroid{
class View;
class ViewGroup;
namespace fragment{

// shared-element name -> view mapping passed across a fragment transition.
using SharedElementMapping = std::map<std::string, View*>;

class FragmentTransitionImpl{
public:
    // Returns a Transition to feed TransitionManager.beginDelayedTransition().
    // If sharedElements is non-empty, a ChangeBounds is used to animate the shared
    // views from source bounds to target bounds; otherwise a default Fade.
    static Transition* makeEnterTransition(const SharedElementMapping& sharedElements = {});
    static Transition* makeExitTransition(const SharedElementMapping& sharedElements = {});
};

}//namespace fragment
}//namespace cdroid
#endif
