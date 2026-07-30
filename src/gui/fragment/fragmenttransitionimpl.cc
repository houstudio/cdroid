#include <fragment/fragmenttransitionimpl.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <transition/fade.h>
#include <transition/changebounds.h>
#include <transition/transitionset.h>

namespace cdroid{
namespace fragment{

Transition* FragmentTransitionImpl::makeEnterTransition(const SharedElementMapping& sharedElements){
    if(!sharedElements.empty()){
        // Shared-element transition: animate shared views' bounds/transform from source
        // (exiting fragment) to target (entering fragment).
        TransitionSet* set = new TransitionSet();
        ChangeBounds* changeBounds = new ChangeBounds();
        for(const auto& kv : sharedElements){
            if(kv.second) changeBounds->addTarget(kv.second);
        }
        set->addTransition(changeBounds);
        set->addTransition(new Fade());
        return set;
    }
    return new Fade();
}

Transition* FragmentTransitionImpl::makeExitTransition(const SharedElementMapping& sharedElements){
    (void)sharedElements;
    // Exit usually uses a Fade (shared elements animate in the enter side).
    return new Fade();
}

}//namespace fragment
}//namespace cdroid
