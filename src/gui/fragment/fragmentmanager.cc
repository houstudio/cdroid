#include <fragment/fragmentmanager.h>
#include <fragment/fragment.h>
#include <fragment/fragmenthostcallback.h>
#include <fragment/fragmentcontainer.h>
#include <fragment/fragmentfactory.h>
#include <fragment/fragmenttransaction.h>
#include <fragment/backstackrecord.h>
#include <fragment/fragmentstatemanager.h>
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
    for(auto& kv : mStateManagers) delete kv.second;
    mStateManagers.clear();
}

void FragmentManager::attachController(FragmentHostCallback* host, FragmentContainer* container, Fragment* parent){
    mHost = host;
    mContainer = container;
    mParent = parent;
    // androidx instanceof probes for ViewModelStoreOwner / SavedStateRegistryOwner /
    // OnBackPressedDispatcherOwner / FragmentOnAttachListener are deferred until the
    // corresponding host interfaces are wired on FragmentActivity (stage 2b-5).
}

FragmentStateManager* FragmentManager::getOrCreateStateManager(Fragment* f){
    auto it = mStateManagers.find(f);
    if(it != mStateManagers.end()) return it->second;
    FragmentStateManager* fsm = new FragmentStateManager(this, f);
    fsm->setFragmentManagerState(mCurState);
    mStateManagers[f] = fsm;
    return fsm;
}

// --- lifecycle dispatch (host -> FM -> each added fragment) ---
void FragmentManager::dispatchStateChange(int state){
    mCurState = state;
    for(Fragment* f : mAdded){
        if(!f) continue;
        FragmentStateManager* fsm = getOrCreateStateManager(f);
        fsm->setFragmentManagerState(state);
        fsm->moveToExpectedState();
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
    FragmentStateManager* fsm = getOrCreateStateManager(f);
    fsm->setFragmentManagerState(mCurState);
    fsm->moveToExpectedState();
}

void FragmentManager::removeFragment(Fragment* f){
    if(!f) return;
    f->mRemoving = true;
    f->mAdded = false;
    if(FragmentStateManager* fsm = getOrCreateStateManager(f)) fsm->moveToState(Fragment::INITIALIZING);
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
    if(FragmentStateManager* fsm = getOrCreateStateManager(f)) fsm->moveToState(Fragment::CREATED);
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
    FragmentStateManager* fsm = getOrCreateStateManager(f);
    fsm->setFragmentManagerState(mCurState);
    fsm->moveToExpectedState();
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

// Drive a fragment to newState. Delegates to its FragmentStateManager (explicit target,
// used by remove/retain and any legacy caller; the normal lifecycle path goes through
// FSM.moveToExpectedState / computeExpectedState instead).
void FragmentManager::moveToState(Fragment* f, int newState){
    if(!f) return;
    getOrCreateStateManager(f)->moveToState(newState);
}

void FragmentManager::setMaxLifecycle(Fragment* f, lifecycle::Lifecycle::State state){
    if(!f) return;
    f->mMaxState = state;
    // Re-drive via FSM: computeExpectedState folds in the new mMaxState. The mMovingToState guard
    // makes this safe even when called from inside a lifecycle callback (e.g. onViewCreated).
    getOrCreateStateManager(f)->moveToExpectedState();
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
