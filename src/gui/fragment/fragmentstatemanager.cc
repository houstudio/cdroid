/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <fragment/fragmentstatemanager.h>
#include <fragment/fragment.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenthostcallback.h>
#include <fragment/fragmenttransitionimpl.h>
#include <fragment/fragmentstate.h>
#include <fragment/defaultspecialeffectscontroller.h>
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

SpecialEffectsController* FragmentStateManager::getSpecialEffectsController(){
    if(!mFragment || !mFragment->mContainer) return nullptr;
    // One DefaultSpecialEffectsController per container, cached on the container's tag.
    static const int SEC_TAG = 0x7F0D0001; // arbitrary unique tag key
    cdroid::View* tagView = mFragment->mContainer;
    SpecialEffectsController* sec = static_cast<SpecialEffectsController*>(tagView->getTag(SEC_TAG));
    if(!sec){
        sec = new DefaultSpecialEffectsController(mFragment->mContainer);
        tagView->setTag(SEC_TAG, sec);
    }
    return sec;
}

void FragmentStateManager::forceCompleteSpecialEffects(){
    // androidx SpecialEffectsController.forceCompleteAll: retire every pending/running op for this
    // fragment's container so getAwaitingCompletionLifecycleImpact() returns NONE and the
    // awaiting-effect clamp in computeExpectedState stops holding the fragment above its target.
    if(SpecialEffectsController* sec = getSpecialEffectsController()){
        sec->forceCompleteAll();
    }
}

Bundle* FragmentStateManager::savedInstanceState() const{
    return mFragment->mSavedFragmentState ? mFragment->mSavedFragmentState->savedInstanceState : nullptr;
}

FragmentState* FragmentStateManager::saveState(){
    // androidx FragmentStateManager.saveState (FragmentStateManager.java:703-754): pack FragmentState
    // meta + onSaveInstanceState + SavedStateRegistry + view-hierarchy state + arguments into one
    // per-fragment saved state (CDROID holds the pieces directly instead of a nested Bundle).
    FragmentState* s = new FragmentState();
    // androidx FragmentState(Fragment) — exact field copy.
    s->className       = mFragment->mClassName;
    s->who             = mFragment->mWho;
    s->fromLayout      = mFragment->mFromLayout;
    s->inDynamicContainer = mFragment->mInDynamicContainer;
    s->fragmentId      = mFragment->mFragmentId;
    s->containerId     = mFragment->mContainerId;
    s->tag             = mFragment->mTag;
    s->retainInstance  = mFragment->mRetainInstance;
    s->removing        = mFragment->mRemoving;
    s->detached        = mFragment->mDetached;
    s->hidden          = mFragment->mHidden;
    s->maxLifecycleState = mFragment->mMaxState;
    s->targetWho       = mFragment->mTargetWho;
    s->targetRequestCode = mFragment->mTargetRequestCode;
    s->userVisibleHint = mFragment->mUserVisibleHint;
    // User + view state only once the fragment has run past ATTACHED.
    if(mFragment->mState > Fragment::ATTACHED){
        Bundle* instanceState = new Bundle();
        mFragment->onSaveInstanceState(instanceState);
        s->savedInstanceState = instanceState;
        if(mFragment->mSavedStateRegistryController){
            mFragment->mSavedStateRegistryController->performSave(s->registryState);
        }
        if(mFragment->mView){
            saveViewState();  // populates mFragment->mSavedViewState
            s->viewState = mFragment->mSavedViewState;  // transfer ownership to the saved state
            mFragment->mSavedViewState = nullptr;
        }
    }
    if(mFragment->mArguments){
        s->arguments = new Bundle(*mFragment->mArguments);
    }
    return s;
}

void FragmentStateManager::saveViewState(){
    // androidx FragmentStateManager.saveViewState (FragmentStateManager.java:763-782): capture the
    // view's hierarchy state into mFragment->mSavedViewState. (The view-lifecycle SavedStateRegistry
    // piece — mSavedViewRegistryState — is omitted for now; CDROID FragmentViewLifecycleOwner
    // SavedStateRegistry can be wired later.)
    if(!mFragment->mView) return;
    SparseArray<Parcelable*>* stateArray = new SparseArray<Parcelable*>();
    mFragment->mView->saveHierarchyState(*stateArray);
    delete mFragment->mSavedViewState;
    mFragment->mSavedViewState = stateArray;
}

void FragmentStateManager::restoreState(const FragmentState& state){
    // androidx FragmentStateManager.restoreState: re-apply the saved slices. arguments + view state
    // are re-applied directly; savedInstanceState/registryState are consumed by the fragment's
    // lifecycle callbacks once it is re-created (the full slice wiring lands with the Phase 2
    // restore path that sets mSavedFragmentState before re-running lifecycle).
    if(state.arguments){
        delete mFragment->mArguments;
        mFragment->mArguments = new Bundle(*state.arguments);
    }
    delete mFragment->mSavedViewState;
    mFragment->mSavedViewState = state.viewState ? new SparseArray<Parcelable*>(*state.viewState) : nullptr;
}

int FragmentStateManager::computeExpectedState(){
    // Detached fragments freeze at their current state.
    if(!mFragment->mFragmentManager) return mFragment->mState;
    int maxState = mFragmentManagerState;
    // Cap by the fragment's max lifecycle (setMaxLifecycle).
    maxState = std::min(maxState, maxStateToInt(mFragment->mMaxState));
    // Fragments not currently added sit at no higher than CREATED.
    if(!mFragment->mAdded) maxState = std::min(maxState, (int)Fragment::CREATED);
    // SpecialEffectsController awaiting-effect clamp (androidx :220-240):
    // A fragment mid-add-effect can't pass AWAITING_ENTER_EFFECTS; mid-remove can't drop below
    // AWAITING_EXIT_EFFECTS. This is what freezes the fragment while its Animation/Transition runs.
    if(mFragment->mContainer){
        SpecialEffectsController* sec = getSpecialEffectsController();
        if(sec){
            int impact = sec->getAwaitingCompletionLifecycleImpact(this);
            if(impact == (int)SpecialEffectsController::Operation::LifecycleImpact::ADDING){
                maxState = std::min(maxState, (int)Fragment::AWAITING_ENTER_EFFECTS);
            } else if(impact == (int)SpecialEffectsController::Operation::LifecycleImpact::REMOVING){
                // A fragment mid-exit-effect can't drop below AWAITING_EXIT_EFFECTS while its exit
                // transition runs — UNLESS the FragmentManager itself is tearing down (state below
                // the floor), in which case the fragment must follow the FM down. Without this
                // guard, the destroy path's moveToExpectedState() re-entry sees the floor raise the
                // expected state ABOVE the FM state and resurrects the view; stepUp/stepDown skip
                // the AWAITING states, so the fragment can never settle and loops forever re-creating
                // its view in an orphaned container (seen when popping a parent whose nested host is
                // mid-destruction). androidx completes awaiting effects via forceCompleteAll on
                // destroy; CDROID's FSM skips AWAITING states, so the floor must yield to the FM.
                int exitFloor = (int)Fragment::AWAITING_EXIT_EFFECTS;
                if(mFragmentManagerState >= exitFloor){
                    maxState = std::max(maxState, exitFloor);
                }
            }
        }
    }
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
            mFragment->performCreate(savedInstanceState()); mFragment->mState = Fragment::CREATED; break;
        case Fragment::CREATED: {
            // Resolve the container ViewGroup at view-creation time, by id (androidx
            // FragmentManager.getFragmentContainer): a fragment added before its host's view
            // exists — e.g. a deferred commit drained during the host's onCreate, when the host
            // has no mView yet — had mContainer resolved to null at addFragment() time. Re-resolve
            // now so the view can be added; by then the host's view tree is built.
            if(mFragmentManager->mContainer){
                mFragment->mContainer = dynamic_cast<cdroid::ViewGroup*>(
                    mFragmentManager->mContainer->onFindViewById(mFragment->mContainerId));
            }
            cdroid::LayoutInflater* inflater = mFragmentManager->mHost
                ? mFragmentManager->mHost->onGetLayoutInflater() : nullptr;
            mFragment->performCreateView(inflater, mFragment->mContainer, savedInstanceState());
            LOGD("FSM.stepUp CREATED: who=%s mView=%p mContainer=%p",
                 mFragment->mWho.c_str(), mFragment->mView, mFragment->mContainer);
            if(mFragment->mView && mFragment->mContainer){
                // A fragment view must fill its host container. A programmatically created view
                // (e.g. NavHostFragment's FrameLayout) carries no LayoutParams — default MATCH_PARENT.
                if(mFragment->mView->getLayoutParams() == nullptr){
                    mFragment->mView->setLayoutParams(new cdroid::LayoutParams(
                        cdroid::LayoutParams::MATCH_PARENT, cdroid::LayoutParams::MATCH_PARENT));
                }
                // Resolve shared-element targets for the entering fragment (by transitionName).
                SharedElementMapping shared;
                if(!mFragmentManager->mPendingSharedNames.empty()){
                    for(const std::string& name : mFragmentManager->mPendingSharedNames){
                        cdroid::View* target = FragmentManager::findViewByTransitionName(mFragment->mView, name);
                        if(target) shared[name] = target;
                    }
                    mFragmentManager->mPendingSharedNames.clear();
                }
                // SEC: enqueue add + execute. The controller's collectEffects picks Animation
                // (custom anim) or Transition (default/shared); commit applies the addView via
                // Operation.applyState (delayed → no flash).
                SpecialEffectsController* sec = getSpecialEffectsController();
                if(sec){
                    sec->enqueueAdd((int)cdroid::View::VISIBLE, this);
                    sec->executePendingOperations();
                } else {
                    // Fallback (no container tag): direct addView + default transition.
                    TransitionManager::beginDelayedTransition(mFragment->mContainer,
                        FragmentTransitionImpl::makeEnterTransition(shared));
                    mFragment->mContainer->addView(mFragment->mView);
                }
            }
            // androidx FragmentStateManager.createView: restore the saved view-hierarchy state
            // (scroll position, text, focus…) into the freshly created view — for fragments
            // re-created by restoreBackStack (mSavedViewState set by restoreState).
            if(mFragment->mSavedViewState && mFragment->mView){
                mFragment->mView->restoreHierarchyState(*mFragment->mSavedViewState);
            }
            mFragment->performViewCreated(savedInstanceState());
            mFragment->mState = Fragment::VIEW_CREATED;
            break;
        }
        case Fragment::VIEW_CREATED:
            mFragment->performActivityCreated(savedInstanceState()); mFragment->mState = Fragment::ACTIVITY_CREATED; break;
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
            // androidx FragmentStateManager.moveToExpectedState: when a saveBackStack pop is tearing
            // this fragment down (mBeingSaved), capture its state ONCE into FragmentManager.mSavedState
            // before the view is destroyed — so restoreBackStack can rehydrate it. Normal pops have
            // mBeingSaved=false and skip this (state discarded, as before).
            if(mFragment->mBeingSaved && mFragmentManager && !mFragmentManager->getSavedState(mFragment->mWho)){
                mFragmentManager->setSavedState(mFragment->mWho, saveState());
            }
            if(mFragment->mView && mFragment->mContainer){
                // SEC: enqueue remove + execute. collectEffects picks Animation (custom exit
                // anim) or Transition (default Fade); commit applies the removeView via
                // Operation.applyState (deferred → no flash).
                SpecialEffectsController* sec = getSpecialEffectsController();
                if(sec){
                    sec->enqueueRemove(this);
                    sec->executePendingOperations();
                } else {
                    TransitionManager::beginDelayedTransition(mFragment->mContainer,
                        FragmentTransitionImpl::makeExitTransition());
                    mFragment->mContainer->removeView(mFragment->mView);
                }
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
