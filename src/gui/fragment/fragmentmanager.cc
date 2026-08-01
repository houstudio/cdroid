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
#include <fragment/fragmentmanager.h>
#include <fragment/fragment.h>
#include <fragment/fragmenthostcallback.h>
#include <fragment/fragmentcontainer.h>
#include <fragment/fragmentfactory.h>
#include <fragment/fragmenttransaction.h>
#include <fragment/backstackrecord.h>
#include <fragment/fragmentstatemanager.h>
#include <fragment/fragmentstate.h>
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

FragmentManager::FragmentManager(){
    // mExecCommit is assigned exactly once and never rebound. Runnable identity is its shared
    // Functor pointer (CallbackBase::operator==), and Handler::removeCallbacks matches posted
    // runnables by that pointer — a stable identity is required for dedup (R4).
    mExecCommit = [this]{ execPendingActions(true); };
}

FragmentManager::~FragmentManager(){
    for(BackStackRecord* r : mBackStack) delete r;
    mBackStack.clear();
    // Pending records are owned by this FM (commit transferred ownership); free any that never
    // executed.
    for(BackStackRecord* r : mPendingActions) delete r;
    mPendingActions.clear();
    mAdded.clear();
    mActive.clear();
    for(auto& kv : mStateManagers) delete kv.second;
    mStateManagers.clear();
    // Per-fragment saved state (androidx FragmentStore.mSavedState) — owned by this FM.
    for(auto& kv : mSavedState) delete kv.second;
    mSavedState.clear();
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
    // androidx FragmentManager.dispatchStateChange: guard the sweep so commits issued inside
    // lifecycle callbacks enqueue rather than re-enter, then drain pending at the end (at the new
    // state). Draining in dispatch also makes deferred constructor-time commits converge: the
    // first dispatch drains them instead of depending on the posted mExecCommit firing at the
    // right mCurState (which would otherwise add fragments at INITIALIZING).
    mExecutingActions = true;
    mCurState = state;
    for(Fragment* f : mAdded){
        if(!f) continue;
        FragmentStateManager* fsm = getOrCreateStateManager(f);
        fsm->setFragmentManagerState(state);
        fsm->moveToExpectedState();
    }
    mExecutingActions = false;
    execPendingActions(true);
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
    // Retire any in-flight special-effects ops before tearing views down, so a fragment mid-effect
    // does not strand its op in mRunningOperations and hold the awaiting clamp (androidx
    // SpecialEffectsController.forceCompleteAll on destroy).
    forceCompleteAllSpecialEffects();
    dispatchStateChange(Fragment::CREATED);
}

void FragmentManager::dispatchDestroy(){
    mDestroyed = true;
    forceCompleteAllSpecialEffects();
    dispatchStateChange(Fragment::INITIALIZING);
}

void FragmentManager::forceCompleteAllSpecialEffects(){
    // One SEC per container; forceCompleteAll is idempotent, so dedup is optional — call it for
    // every active fragment's container (redundant calls on a shared SEC are no-ops).
    for(auto& kv : mActive){
        Fragment* f = kv.second;
        if(!f) continue;
        if(FragmentStateManager* fsm = getOrCreateStateManager(f)){
            fsm->forceCompleteSpecialEffects();
        }
    }
}

// androidx FragmentStore.setSavedState: store-or-remove-and-return. setSavedState(who, nullptr) is
// the retrieve-and-clear used by BackStackState.instantiate on restore; setSavedState(who, s) stores
// s and returns the previous value (nullptr if none). FM takes ownership of stored FragmentState.
FragmentState* FragmentManager::setSavedState(const std::string& who, FragmentState* s){
    if(s != nullptr){
        FragmentState* prev = nullptr;
        auto it = mSavedState.find(who);
        if(it != mSavedState.end()) prev = it->second;
        mSavedState[who] = s;
        return prev;
    }
    FragmentState* prev = nullptr;
    auto it = mSavedState.find(who);
    if(it != mSavedState.end()){ prev = it->second; mSavedState.erase(it); }
    return prev;
}

FragmentState* FragmentManager::getSavedState(const std::string& who) const{
    auto it = mSavedState.find(who);
    return it != mSavedState.end() ? it->second : nullptr;
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

// --- options-menu dispatch (androidx FragmentManager.dispatch*OptionsMenu) ---
bool FragmentManager::isParentMenuVisible(Fragment* parent) const{
    // androidx: parent == null (host Activity) -> true; else parent.isMenuVisible().
    if(parent == nullptr) return true;
    return parent->isMenuVisible();
}

bool FragmentManager::dispatchCreateOptionsMenu(Menu& menu, MenuInflater& inflater){
    if(mCurState < Fragment::CREATED) return false;
    bool show = false;
    std::vector<Fragment*> newMenus;
    for(Fragment* f : getFragments()){
        if(f && isParentMenuVisible(f->getParentFragment()) && f->performCreateOptionsMenu(menu, inflater)){
            show = true;
            newMenus.push_back(f);
        }
    }
    // Notify fragments that contributed last time but no longer do.
    for(Fragment* f : mCreatedMenus){
        bool still = false;
        for(Fragment* n : newMenus){ if(n == f){ still = true; break; } }
        if(!still) f->onDestroyOptionsMenu();
    }
    mCreatedMenus = newMenus;
    return show;
}

bool FragmentManager::dispatchPrepareOptionsMenu(Menu& menu){
    if(mCurState < Fragment::CREATED) return false;
    bool show = false;
    for(Fragment* f : getFragments()){
        if(f && isParentMenuVisible(f->getParentFragment()) && f->performPrepareOptionsMenu(menu)){
            show = true;
        }
    }
    return show;
}

bool FragmentManager::dispatchOptionsItemSelected(MenuItem& item){
    if(mCurState < Fragment::CREATED) return false;
    for(Fragment* f : getFragments()){
        if(f && f->performOptionsItemSelected(item)) return true;
    }
    return false;
}

bool FragmentManager::dispatchContextItemSelected(MenuItem& item){
    if(mCurState < Fragment::CREATED) return false;
    for(Fragment* f : getFragments()){
        if(f && f->performContextItemSelected(item)) return true;
    }
    return false;
}

void FragmentManager::dispatchOptionsMenuClosed(Menu& menu){
    if(mCurState < Fragment::CREATED) return;
    for(Fragment* f : getFragments()){
        if(f) f->performOptionsMenuClosed(menu);
    }
}

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

// --- transactions (BackStackRecord-backed) ---
FragmentTransaction* FragmentManager::beginTransaction(){
    return new BackStackRecord(this);
}

bool FragmentManager::executePendingTransactions(){
    return execPendingActions(true);
}

bool FragmentManager::popBackStackImmediate(){
    // Drain pending forward commits first so records added via addToBackStack have landed in
    // mBackStack before we pop the top (androidx popBackStackImmediate: execPendingActions first).
    execPendingActions(false);
    if(mBackStack.empty()) return false;
    BackStackRecord* record = mBackStack.back();
    mBackStack.pop_back();
    record->executePopOps();
    delete record;
    return true;
}

bool FragmentManager::popBackStackImmediate(const std::string& name, int flags){
    // androidx FragmentManager.popBackStackState(name, id, flags): pop every record above the
    // topmost record whose name matches `name`; if POP_BACK_STACK_INCLUSIVE, pop that record too.
    // A name that was never addToBackStack'd (e.g. the FragmentNavigator initial fragment) is not
    // found → return false (no-op), which is exactly what lets the initial fragment linger.
    execPendingActions(false);
    if(mBackStack.empty()) return false;
    int index = -1;
    for(int i = (int)mBackStack.size() - 1; i >= 0; --i){
        if(mBackStack[i]->getName() == name){ index = i; break; }
    }
    if(index < 0) return false;  // name not on this FragmentManager's back stack
    int stop = (flags & POP_BACK_STACK_INCLUSIVE) ? index - 1 : index;
    while((int)mBackStack.size() > stop + 1){
        BackStackRecord* record = mBackStack.back();
        mBackStack.pop_back();
        record->executePopOps();
        delete record;
    }
    return true;
}

bool FragmentManager::saveBackStack(const std::string& name){
    // androidx FragmentManager.saveBackStackState: find the records named `name`, capture each one's
    // ops + fragment whos, mark them mBeingSaved, then pop them (top-down). Popping runs executePopOps
    // → FragmentStateManager tears the fragments down → since mBeingSaved, saveState() writes each
    // fragment's state into mSavedState[who]. The BackStackState is stored in mBackStackStates[name].
    execPendingActions(false);
    int index = -1;
    for(int i = 0; i < (int)mBackStack.size(); ++i){
        if(mBackStack[i]->getName() == name){ index = i; break; }   // lowest match (inclusive)
    }
    if(index < 0) return false;
    BackStackState bs;
    for(int i = index; i < (int)mBackStack.size(); ++i){
        BackStackRecord* record = mBackStack[i];
        record->mBeingSaved = true;
        bs.transactions.push_back(record->captureState());
        for(const auto& op : record->getOps()){
            if(op.mFragment && !op.mFragment->mWho.empty()) bs.fragmentWhos.push_back(op.mFragment->mWho);
        }
    }
    for(int i = (int)mBackStack.size() - 1; i >= index; --i){
        BackStackRecord* record = mBackStack[i];
        mBackStack.erase(mBackStack.begin() + i);
        record->executePopOps();   // fragments torn down; FSM.saveState (mBeingSaved) → mSavedState
        delete record;
    }
    mBackStackStates[name] = std::move(bs);
    return true;
}

bool FragmentManager::restoreBackStack(const std::string& name){
    // androidx FragmentManager.restoreBackStackState: take the BackStackState, re-instantiate each
    // fragment from its saved state, rebuild the BackStackRecords, and re-run them forward.
    execPendingActions(false);
    auto it = mBackStackStates.find(name);
    if(it == mBackStackStates.end()) return false;
    BackStackState bs = std::move(it->second);
    mBackStackStates.erase(it);
    // Phase A — re-create the Fragment objects from their saved state.
    std::unordered_map<std::string, Fragment*> fragments;
    for(const std::string& who : bs.fragmentWhos){
        if(fragments.count(who)) continue;
        FragmentState* fs = setSavedState(who, nullptr);   // retrieve-and-clear
        if(!fs || fs->className.empty()){ delete fs; continue; }
        FragmentFactory factory;                            // uses the global REGISTER_FRAGMENT registry
        Fragment* f = factory.instantiate(fs->className);
        if(!f){ delete fs; continue; }
        // Restore FragmentState meta (androidx FragmentState.instantiate copies these back).
        f->mWho = fs->who;
        f->mFragmentId = fs->fragmentId;
        f->mContainerId = fs->containerId;
        f->mTag = fs->tag;
        f->mHidden = fs->hidden;
        f->mMaxState = fs->maxLifecycleState;
        if(FragmentStateManager* fsm = getOrCreateStateManager(f)) fsm->restoreState(*fs); // args + view state
        f->mSavedFragmentState = fs;                        // lifecycle consumes savedInstanceState/registry
        fragments[who] = f;
    }
    // Phase B — rebuild + re-run the transactions forward (adds the fragments, moves them up).
    for(BackStackRecordState& brs : bs.transactions){
        BackStackRecord* record = new BackStackRecord(this);
        record->restoreFromState(brs, fragments);
        record->executeOps();
        record->mBeingSaved = false;
        mBackStack.push_back(record);
    }
    return true;
}

bool FragmentManager::clearBackStack(const std::string& name){
    // androidx FragmentManager.clearBackStackState = restore then pop (discard).
    if(!restoreBackStack(name)) return false;
    return popBackStackImmediate(name, POP_BACK_STACK_INCLUSIVE);
}

// Enqueue a record for deferred execution (androidx enqueueAction). Ownership of `action`
// transfers to this FragmentManager.
void FragmentManager::enqueueAction(BackStackRecord* action, bool /*allowStateLoss*/){
    if(!action) return;
    mPendingActions.push_back(action);
    scheduleCommit();
}

// androidx scheduleCommit: on the first pending action post mExecCommit to the host Handler so the
// queue is flushed on the next main-loop iteration. removeCallbacks dedups repeat posts.
void FragmentManager::scheduleCommit(){
    if(mPendingActions.size() != 1) return;   // already scheduled, or empty
    cdroid::Handler* h = mHost ? mHost->getHandler() : nullptr;
    if(h){
        h->removeCallbacks(mExecCommit);
        h->post(mExecCommit);
    } else {
        // No host handler: fall back to a synchronous drain so the commit is not lost.
        execPendingActions(true);
    }
}

void FragmentManager::ensureExecReady(bool /*allowStateLoss*/){
    // androidx checks host/destroyed/main-thread here and throws. CDROID is single-threaded and
    // tolerant; the mExecutingActions re-entrancy guard is handled at execPendingActions entry.
}

void FragmentManager::cleanupExec(){
    mExecutingActions = false;
    mTmpRecords.clear();
    mTmpIsPop.clear();
}

// Drain all pending commits synchronously (androidx execPendingActions). Re-entrancy guarded by
// mExecutingActions; a while-loop keeps draining records enqueued during execution (commits issued
// inside lifecycle callbacks), the androidx non-recursive drain.
bool FragmentManager::execPendingActions(bool allowStateLoss){
    if(mExecutingActions) return false;
    ensureExecReady(allowStateLoss);
    bool didSomething = false;
    mExecutingActions = true;
    while(generateOpsForPendingActions(mTmpRecords, mTmpIsPop)){
        removeRedundantOperationsAndExecute(mTmpRecords, mTmpIsPop);
        didSomething = true;
    }
    cleanupExec();
    return didSomething;
}

// Execute a single record synchronously without enqueueing (androidx execSingleAction, the
// commitNow path): drain pending first, then run this record's ops + state convergence.
void FragmentManager::execSingleAction(BackStackRecord* action, bool allowStateLoss){
    if(!action) return;
    execPendingActions(allowStateLoss);
    mExecutingActions = true;
    mTmpRecords.clear();
    mTmpIsPop.clear();
    action->generateOps(mTmpRecords, mTmpIsPop);
    removeRedundantOperationsAndExecute(mTmpRecords, mTmpIsPop);
    cleanupExec();
}

// Move all pending actions into the scratch buffers (androidx generateOpsForPendingActions).
bool FragmentManager::generateOpsForPendingActions(std::vector<BackStackRecord*>& records,
                                                   std::vector<bool>& isRecordPop){
    if(mPendingActions.empty()) return false;
    records.clear();
    isRecordPop.clear();
    for(BackStackRecord* r : mPendingActions) r->generateOps(records, isRecordPop);
    mPendingActions.clear();
    if(cdroid::Handler* h = mHost ? mHost->getHandler() : nullptr) h->removeCallbacks(mExecCommit);
    return !records.empty();
}

// androidx removeRedundantOperationsAndExecute merges reordering-allowed records. CDROID never
// enables reordering (mReorderingAllowed is unused), so the merge collapses to one pass.
void FragmentManager::removeRedundantOperationsAndExecute(std::vector<BackStackRecord*>& records,
                                                          std::vector<bool>& isRecordPop){
    if(records.empty()) return;
    executeOpsTogether(records, isRecordPop, 0, (int)records.size());
}

void FragmentManager::executeOpsTogether(std::vector<BackStackRecord*>& records,
                                         std::vector<bool>& isRecordPop, int startIndex, int endIndex){
    // 1) Run each record's ops. Forward: executeOps + push to back stack if addToBackStack.
    //    Pop: executePopOps (reverses). The deferred path only carries forward records today (pop
    //    is synchronous via popBackStackImmediate), but the isPop branch is kept for parity.
    for(int i = startIndex; i < endIndex; i++){
        BackStackRecord* record = records[i];
        if(isRecordPop[i]){
            record->executePopOps();
        } else {
            record->executeOps();
            if(record->isAddedToBackStack()) addBackStackState(record);
        }
    }
    // 2) Drive every fragment touched by these ops to its expected state (androidx per-op
    //    moveToExpectedState), then sweep all added fragments to mCurState (androidx
    //    moveToState(mCurState, true)) so deferred-added fragments catch up to the host lifecycle.
    for(int i = startIndex; i < endIndex; i++){
        for(const FragmentTransaction::Op& op : records[i]->getOps()){
            if(op.mFragment) getOrCreateStateManager(op.mFragment)->moveToExpectedState();
        }
    }
    for(Fragment* f : mAdded){
        if(!f) continue;
        FragmentStateManager* fsm = getOrCreateStateManager(f);
        fsm->setFragmentManagerState(mCurState);
        fsm->moveToExpectedState();
    }
    // 3) Ownership: forward non-back-stack records are owned by the FM and freed now. Back-stack
    //    records are retained in mBackStack (freed on pop). (Pop records are owned/freed by the
    //    pop path.) Must happen before the scratch buffers are reused next round.
    for(int i = startIndex; i < endIndex; i++){
        if(!isRecordPop[i] && !records[i]->isAddedToBackStack()) delete records[i];
    }
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
