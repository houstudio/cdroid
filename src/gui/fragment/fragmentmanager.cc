#include <fragment/fragmentmanager.h>
#include <fragment/fragment.h>
#include <fragment/fragmenthostcallback.h>
#include <fragment/fragmentcontainer.h>
#include <fragment/fragmentfactory.h>
#include <fragment/fragmenttransaction.h>
#include <fragment/backstackrecord.h>
#include <fragment/fragmentanim.h>
#include <fragment/fragmenttransitionimpl.h>
#include <animation/animation.h>
#include <transition/transitionmanager.h>
#include <transition/fade.h>
#include <transition/changebounds.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <view/layoutinflater.h>
#include <algorithm>
#include <porting/cdlog.h>

namespace cdroid{
namespace fragment{

FragmentManager::FragmentManager() = default;

FragmentManager::~FragmentManager(){
    for(BackStackRecord* r : mBackStack) delete r;
    mBackStack.clear();
    mAdded.clear();
    mActive.clear();
}

void FragmentManager::attachController(FragmentHostCallback* host, FragmentContainer* container, Fragment* parent){
    mHost = host;
    mContainer = container;
    mParent = parent;
    // androidx instanceof probes for ViewModelStoreOwner / SavedStateRegistryOwner /
    // OnBackPressedDispatcherOwner / FragmentOnAttachListener are deferred until the
    // corresponding host interfaces are wired on FragmentActivity (stage 2b-5).
}

// --- lifecycle dispatch (host -> FM -> each added fragment) ---
void FragmentManager::dispatchStateChange(int state){
    mCurState = state;
    for(Fragment* f : mAdded){
        if(f) moveToState(f, state);
    }
}

void FragmentManager::dispatchAttach(){
    dispatchStateChange(Fragment::ATTACHED);
}

void FragmentManager::dispatchCreate(){
    dispatchStateChange(Fragment::CREATED);
}

void FragmentManager::dispatchViewCreated(){
    dispatchStateChange(Fragment::VIEW_CREATED);
}

void FragmentManager::dispatchActivityCreated(){
    dispatchStateChange(Fragment::ACTIVITY_CREATED);
}

void FragmentManager::dispatchStart(){
    mStopped = false;
    dispatchStateChange(Fragment::STARTED);
}

void FragmentManager::dispatchResume(){
    dispatchStateChange(Fragment::RESUMED);
}

void FragmentManager::dispatchPause(){
    dispatchStateChange(Fragment::STARTED);
}

void FragmentManager::dispatchStop(){
    mStopped = true; 
    dispatchStateChange(Fragment::ACTIVITY_CREATED);
}

void FragmentManager::dispatchDestroyView(){
    dispatchStateChange(Fragment::CREATED);
}

void FragmentManager::dispatchDestroy(){
    mDestroyed = true;
    dispatchStateChange(Fragment::INITIALIZING);
}

// --- internal fragment ops ---
void FragmentManager::addFragment(Fragment* f, bool hidden){
    if(!f) return;
    if(mActive.count(f->mWho)) return;
    f->mFragmentManager = this;
    f->mHost = mHost;
    f->mParentFragment = mParent; // nested: set when this FM is a childFragmentManager (parent = NavHostFragment)
    f->mAdded = true;
    f->mHidden = hidden;
    // Resolve the container ViewGroup fragments inflate into / are added to.
    f->mContainer = mContainer ? dynamic_cast<cdroid::ViewGroup*>(mContainer->onFindViewById(f->mContainerId)) : nullptr;
    mAdded.push_back(f);
    mActive[f->mWho] = f;
    moveToState(f, mCurState);
}

void FragmentManager::removeFragment(Fragment* f){
    if(!f) return;
    f->mRemoving = true;
    f->mAdded = false;
    moveToState(f, Fragment::INITIALIZING);
    mAdded.erase(std::remove(mAdded.begin(), mAdded.end(), f), mAdded.end());
    mActive.erase(f->mWho);
    f->mFragmentManager = nullptr;
    f->mHost = nullptr;
}

// Retain a fragment removed by a *reversible* (added-to-back-stack) transaction. androidx
// does NOT destroy such a fragment: it tears down the view but keeps the instance (and its
// LifecycleRegistry, held at CREATED) in mActive so popBackStack can restore it. Destroying
// it would drive the registry to DESTROYED, which can never be advanced again — the exact
// crash "State is DESTROYED and cannot be moved to a new state" on pop.
void FragmentManager::retainFragment(Fragment* f){
    if(!f) return;
    f->mRemoving = true;
    f->mAdded = false;
    moveToState(f, Fragment::CREATED); // onPause/onStop/onDestroyView; lifecycle stops at CREATED (not DESTROYED)
    mAdded.erase(std::remove(mAdded.begin(), mAdded.end(), f), mAdded.end());
    // deliberately kept in mActive with mFragmentManager/mHost intact so pop can restore it
}

// Re-add a fragment retained by retainFragment when its back-stack record is popped.
// It is already in mActive (never dropped); just re-insert into mAdded and walk it back up
// to the FragmentManager's current state (onCreateView rebuilds the view; onCreate is NOT
// re-invoked — the fragment stayed at CREATED).
void FragmentManager::unretainFragment(Fragment* f){
    if(!f) return;
    f->mRemoving = false;
    f->mAdded = true;
    f->mFragmentManager = this;
    f->mHost = mHost;
    f->mContainer = mContainer ? dynamic_cast<cdroid::ViewGroup*>(mContainer->onFindViewById(f->mContainerId)) : nullptr;
    mAdded.push_back(f);
    moveToState(f, mCurState);
}

void FragmentManager::showFragment(Fragment* f){
    if(!f) return;
    f->mHidden = false;
    f->mHiddenChanged = true;
    if(f->mView) f->mView->setVisibility(cdroid::View::VISIBLE);
}

void FragmentManager::hideFragment(Fragment* f){
    if(!f) return;
    f->mHidden = true;
    f->mHiddenChanged = true;
    if(f->mView) f->mView->setVisibility(cdroid::View::GONE);
}

void FragmentManager::attachFragment(Fragment* f){
    if(!f) return;
    f->mDetached = false;
}

void FragmentManager::detachFragment(Fragment* f){
    if(!f) return;
    f->mDetached = true;
    if(f->mView && f->mContainer) f->mContainer->removeView(f->mView);
}

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

// --- simplified per-fragment state machine (no special-effects intermediate states) ---
void FragmentManager::moveToState(Fragment* f, int newState){
    if(!f) return;
    // Clamp by the fragment's max lifecycle so setMaxLifecycle can cap it (e.g. STARTED) even
    // when the host is RESUMED (androidx computeExpectedState).
    const int maxInt = maxStateToInt(f->mMaxState);
    if(newState > maxInt) newState = maxInt;
    // step up
    while(f->mState < newState){
        switch(f->mState){
            case Fragment::INITIALIZING:
                f->performAttach(); f->mState = Fragment::ATTACHED; break;
            case Fragment::ATTACHED:
                f->performCreate(nullptr); f->mState = Fragment::CREATED; break;
            case Fragment::CREATED: {
                cdroid::LayoutInflater* inflater = mHost ? mHost->onGetLayoutInflater() : nullptr;
                f->performCreateView(inflater, f->mContainer, nullptr);
                LOGD("FM.moveToState CREATED: who=%s mView=%p mContainer=%p", f->mWho.c_str(), f->mView, f->mContainer);
                if(f->mView && f->mContainer){
                    // Resolve shared-element targets (by transitionName) in this entering fragment.
                    SharedElementMapping shared;
                    if(!mPendingSharedNames.empty()){
                        for(const std::string& name : mPendingSharedNames){
                            cdroid::View* target = findViewByTransitionName(f->mView, name);
                            if(target) shared[name] = target;
                        }
                        mPendingSharedNames.clear();
                    }
                    // A fragment view must fill its host container. A programmatically created
                    // view (e.g. NavHostFragment's FrameLayout) carries no LayoutParams, so it
                    // would measure to 0 and render nothing — default to MATCH_PARENT like androidx.
                    if(f->mView->getLayoutParams() == nullptr){
                        f->mView->setLayoutParams(new cdroid::LayoutParams(
                            cdroid::LayoutParams::MATCH_PARENT, cdroid::LayoutParams::MATCH_PARENT));
                    }
                    TransitionManager::beginDelayedTransition(f->mContainer,
                        FragmentTransitionImpl::makeEnterTransition(shared));
                    f->mContainer->addView(f->mView);
                }
                f->performViewCreated(nullptr);
                f->mState = Fragment::VIEW_CREATED;
                break;
            }
            case Fragment::VIEW_CREATED:
                f->performActivityCreated(nullptr); f->mState = Fragment::ACTIVITY_CREATED; break;
            case Fragment::ACTIVITY_CREATED:
                f->performStart(); f->mState = Fragment::STARTED; break;
            case Fragment::AWAITING_EXIT_EFFECTS:
            case Fragment::STARTED:
                f->performResume(); f->mState = Fragment::RESUMED; break;
            case Fragment::AWAITING_ENTER_EFFECTS:
                f->mState = Fragment::RESUMED; break;
            default: break;
        }
    }
    // step down
    while(f->mState > newState){
        switch(f->mState){
            case Fragment::RESUMED:
            case Fragment::AWAITING_ENTER_EFFECTS:
                f->performPause(); f->mState = Fragment::STARTED; break;
            case Fragment::STARTED:
            case Fragment::AWAITING_EXIT_EFFECTS:
                f->performStop(); f->mState = Fragment::ACTIVITY_CREATED; break;
            case Fragment::ACTIVITY_CREATED:
                f->mState = Fragment::VIEW_CREATED; break; // no callback on the way down here
            case Fragment::VIEW_CREATED:
                if(f->mView && f->mContainer){
                    TransitionManager::beginDelayedTransition(f->mContainer,
                        FragmentTransitionImpl::makeExitTransition());
                    f->mContainer->removeView(f->mView);
                }
                f->performDestroyView();
                f->mView = nullptr;
                f->mState = Fragment::CREATED;
                break;
            case Fragment::CREATED:
                f->performDestroy(); f->mState = Fragment::ATTACHED; break;
            case Fragment::ATTACHED:
                f->performDetach(); f->mState = Fragment::INITIALIZING; break;
            default: break;
        }
    }
}

void FragmentManager::setMaxLifecycle(Fragment* f, lifecycle::Lifecycle::State state){
    if(!f) return;
    f->mMaxState = state;
    moveToState(f, mCurState); // re-drive; the clamp in moveToState applies the new ceiling
}

// --- lookups ---
Fragment* FragmentManager::findFragmentById(int id){
    for(Fragment* f : mAdded){ if(f && f->mFragmentId == id) return f; }
    return nullptr;
}

Fragment* FragmentManager::findFragmentByTag(const std::string& tag){
    for(Fragment* f : mAdded){ if(f && f->mTag == tag) return f; }
    return nullptr;
}

std::vector<Fragment*> FragmentManager::getFragments() const{ return mAdded; }

Fragment* FragmentManager::getPrimaryNavigationFragment() const{
    // MVP: not tracked yet.
    return nullptr;
}

void FragmentManager::setFragmentFactory(FragmentFactory* factory){
    mFragmentFactory = factory;
}

FragmentFactory* FragmentManager::getFragmentFactory() const{
    return mFragmentFactory;
}

// --- transactions (BackStackRecord-backed; stage 2b-4) ---
FragmentTransaction* FragmentManager::beginTransaction(){
    return new BackStackRecord(this);
}

bool FragmentManager::executePendingTransactions(){
    return false; // stage 2b-4
}

bool FragmentManager::popBackStackImmediate(){
    if(mBackStack.empty()) return false;
    BackStackRecord* record = mBackStack.back();
    mBackStack.pop_back();
    record->executePopOps();
    delete record;
    return true;
}

bool FragmentManager::popBackStackImmediate(const std::string& /*name*/, int /*flags*/){
    return false; // stage 2b-4
}

void FragmentManager::enqueueAction(BackStackRecord* action){
    if(action) mBackStack.push_back(action); // stage 2b-4 wires execution
}

cdroid::View* FragmentManager::findViewByTransitionName(cdroid::View* root, const std::string& name){
    if(!root) return nullptr;
    if(root->getTransitionName() == name) return root;
    cdroid::ViewGroup* vg = dynamic_cast<cdroid::ViewGroup*>(root);
    if(vg){
        for(int i = 0; i < vg->getChildCount(); i++){
            cdroid::View* found = findViewByTransitionName(vg->getChildAt(i), name);
            if(found) return found;
        }
    }
    return nullptr;
}

}//namespace fragment
}//namespace cdroid
