#include <fragment/fragmentstatemanager.h>
#include <fragment/fragment.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenthostcallback.h>
#include <fragment/fragmenttransitionimpl.h>
#include <transition/transitionmanager.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <view/layoutinflater.h>
#include <algorithm>
#include <porting/cdlog.h>

namespace cdroid{
namespace fragment{

// Map a Lifecycle::State ceiling to the Fragment int-state it permits (androidx
// FragmentStateManager.computeExpectedState clamps managerState by mFragment.mMaxState).
static int maxStateToInt(lifecycle::Lifecycle::State s){
    using L = lifecycle::Lifecycle::State;
    switch(s){
        case L::RESUMED:     return Fragment::RESUMED;      // 7
        case L::STARTED:     return Fragment::STARTED;      // 5
        case L::CREATED:     return Fragment::CREATED;      // 1
        case L::INITIALIZED: return Fragment::ATTACHED;     // 0
        case L::DESTROYED:   return Fragment::INITIALIZING; // -1
    }
    return Fragment::RESUMED;
}

FragmentStateManager::FragmentStateManager(FragmentManager* fm, Fragment* f)
    : mFragmentManager(fm), mFragment(f){
}

int FragmentStateManager::computeExpectedState(){
    // Detached fragments freeze at their current state.
    if(!mFragment->mFragmentManager) return mFragment->mState;
    int maxState = mFragmentManagerState;
    // Cap by the fragment's max lifecycle (setMaxLifecycle).
    maxState = std::min(maxState, maxStateToInt(mFragment->mMaxState));
    // Fragments not currently added sit at no higher than CREATED. (int) cast avoids
    // ODR-using the static const int (passed by reference to std::min).
    if(!mFragment->mAdded) maxState = std::min(maxState, (int)Fragment::CREATED);
    return maxState;
}

void FragmentStateManager::moveToExpectedState(){
    if(mMovingToState) return; // re-entrancy guard (androidx FragmentStateManager)
    mMovingToState = true;
    int s;
    while((s = computeExpectedState()) != mFragment->mState){
        if(s > mFragment->mState) stepUp();
        else                       stepDown();
    }
    mMovingToState = false;
}

void FragmentStateManager::moveToState(int explicitTarget){
    if(mMovingToState) return;
    mMovingToState = true;
    while(mFragment->mState < explicitTarget) stepUp();
    while(mFragment->mState > explicitTarget) stepDown();
    mMovingToState = false;
}

void FragmentStateManager::stepUp(){
    switch(mFragment->mState){
        case Fragment::INITIALIZING:
            mFragment->performAttach(); mFragment->mState = Fragment::ATTACHED; break;
        case Fragment::ATTACHED:
            mFragment->performCreate(nullptr); mFragment->mState = Fragment::CREATED; break;
        case Fragment::CREATED: {
            cdroid::LayoutInflater* inflater = mFragmentManager->mHost
                ? mFragmentManager->mHost->onGetLayoutInflater() : nullptr;
            mFragment->performCreateView(inflater, mFragment->mContainer, nullptr);
            LOGD("FSM.stepUp CREATED: who=%s mView=%p mContainer=%p",
                 mFragment->mWho.c_str(), mFragment->mView, mFragment->mContainer);
            if(mFragment->mView && mFragment->mContainer){
                // Resolve shared-element targets (by transitionName) in this entering fragment.
                SharedElementMapping shared;
                if(!mFragmentManager->mPendingSharedNames.empty()){
                    for(const std::string& name : mFragmentManager->mPendingSharedNames){
                        cdroid::View* target = FragmentManager::findViewByTransitionName(mFragment->mView, name);
                        if(target) shared[name] = target;
                    }
                    mFragmentManager->mPendingSharedNames.clear();
                }
                // A fragment view must fill its host container. A programmatically created view
                // (e.g. NavHostFragment's FrameLayout) carries no LayoutParams — default MATCH_PARENT.
                if(mFragment->mView->getLayoutParams() == nullptr){
                    mFragment->mView->setLayoutParams(new cdroid::LayoutParams(
                        cdroid::LayoutParams::MATCH_PARENT, cdroid::LayoutParams::MATCH_PARENT));
                }
                TransitionManager::beginDelayedTransition(mFragment->mContainer,
                    FragmentTransitionImpl::makeEnterTransition(shared));
                mFragment->mContainer->addView(mFragment->mView);
            }
            mFragment->performViewCreated(nullptr);
            mFragment->mState = Fragment::VIEW_CREATED;
            break;
        }
        case Fragment::VIEW_CREATED:
            mFragment->performActivityCreated(nullptr); mFragment->mState = Fragment::ACTIVITY_CREATED; break;
        case Fragment::ACTIVITY_CREATED:
            mFragment->performStart(); mFragment->mState = Fragment::STARTED; break;
        case Fragment::AWAITING_EXIT_EFFECTS:
        case Fragment::STARTED:
            mFragment->performResume(); mFragment->mState = Fragment::RESUMED; break;
        case Fragment::AWAITING_ENTER_EFFECTS:
            mFragment->mState = Fragment::RESUMED; break;
        default: break;
    }
}

void FragmentStateManager::stepDown(){
    switch(mFragment->mState){
        case Fragment::RESUMED:
        case Fragment::AWAITING_ENTER_EFFECTS:
            mFragment->performPause(); mFragment->mState = Fragment::STARTED; break;
        case Fragment::STARTED:
        case Fragment::AWAITING_EXIT_EFFECTS:
            mFragment->performStop(); mFragment->mState = Fragment::ACTIVITY_CREATED; break;
        case Fragment::ACTIVITY_CREATED:
            mFragment->mState = Fragment::VIEW_CREATED; break; // no callback on the way down here
        case Fragment::VIEW_CREATED:
            if(mFragment->mView && mFragment->mContainer){
                TransitionManager::beginDelayedTransition(mFragment->mContainer,
                    FragmentTransitionImpl::makeExitTransition());
                mFragment->mContainer->removeView(mFragment->mView);
            }
            mFragment->performDestroyView();
            mFragment->mView = nullptr;
            mFragment->mState = Fragment::CREATED;
            break;
        case Fragment::CREATED:
            mFragment->performDestroy(); mFragment->mState = Fragment::ATTACHED; break;
        case Fragment::ATTACHED:
            mFragment->performDetach(); mFragment->mState = Fragment::INITIALIZING; break;
        default: break;
    }
}

}//namespace fragment
}//namespace cdroid
