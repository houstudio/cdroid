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
#include <fragment/specialeffectscontroller.h>
#include <fragment/fragment.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmentstatemanager.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <algorithm>
#include <memory>
#include <porting/cdlog.h>

namespace cdroid{
namespace fragment{

static SpecialEffectsController::Operation::State visibilityToState(int vis){
    if(vis == (int)cdroid::View::GONE) return SpecialEffectsController::Operation::State::GONE;
    if(vis == (int)cdroid::View::INVISIBLE) return SpecialEffectsController::Operation::State::INVISIBLE;
    return SpecialEffectsController::Operation::State::VISIBLE;
}

SpecialEffectsController::~SpecialEffectsController(){
    for(Operation* op : mPendingOperations) delete op;
    for(Operation* op : mRunningOperations) delete op;
    for(Operation* op : mCompletedOperations) delete op;
}

void SpecialEffectsController::enqueueAdd(int finalStateVisibility, FragmentStateManager* fsm){
    enqueue(visibilityToState(finalStateVisibility), Operation::LifecycleImpact::ADDING, fsm);
}
void SpecialEffectsController::enqueueRemove(FragmentStateManager* fsm){
    enqueue(Operation::State::REMOVED, Operation::LifecycleImpact::REMOVING, fsm);
}
void SpecialEffectsController::enqueueHide(FragmentStateManager* fsm){
    enqueue(Operation::State::GONE, Operation::LifecycleImpact::NONE, fsm);
}
void SpecialEffectsController::enqueueShow(FragmentStateManager* fsm){
    enqueue(Operation::State::VISIBLE, Operation::LifecycleImpact::NONE, fsm);
}

void SpecialEffectsController::enqueue(Operation::State finalState, Operation::LifecycleImpact impact, FragmentStateManager* fsm){
    if(!fsm) return;
    Fragment* f = fsm->getFragment();
    if(!f) return;
    Operation* op = new FragmentStateManagerOperation(finalState, impact, f, this, fsm);
    // On completion: retire from running into completed (owned by this controller for dtor).
    op->addCompletionListener([this, op](){
        auto& running = mRunningOperations;
        running.erase(std::remove(running.begin(), running.end(), op), running.end());
        mCompletedOperations.push_back(op);
    });
    // Effect-end-gated fragment reclaim hook. The exit effect (TransitionEffect/AnimationEffect)
    // copies this hook BY VALUE into its posted end-lambda and invokes it after `delete view`. It
    // routes through the FM (never the FSM) so the destroy-sweep deleting the FSM can't UAF a late
    // posted hook; the weak alive-guard makes it a no-op if the FM is already destroyed.
    FragmentManager* fm = fsm->getFragmentManager();
    std::weak_ptr<bool> aliveGuard = fm ? fm->getAlive() : std::weak_ptr<bool>();
    op->mReclaimHook = [fm, f, aliveGuard](){
        if(aliveGuard.lock() && fm) fm->reclaimFragment(f);
    };
    // NOTE: REMOVED view reclamation is NOT done here (on op-completion). For the Transition path
    // complete() fires synchronously inside onCommit, before the async clone transition plays, so a
    // delete-on-complete() would UAF; and deferring completeEffect to transition-end busy-loops
    // (the op staying in mRunningOperations floors the FSM clamp at the un-landable
    // AWAITING_EXIT_EFFECTS=3). Instead each Effect reclaims the view on its OWN true end — see
    // TransitionEffect::onCommit (clone onTransitionEnd) and AnimationEffect::onCommit (onAnimationEnd).
    mPendingOperations.push_back(op);
}

int SpecialEffectsController::getAwaitingCompletionLifecycleImpact(FragmentStateManager* fsm){
    if(!fsm) return 0;
    Fragment* f = fsm->getFragment();
    for(Operation* op : mPendingOperations) if(op->mFragment == f) return (int)op->mLifecycleImpact;
    for(Operation* op : mRunningOperations) if(op->mFragment == f) return (int)op->mLifecycleImpact;
    return 0; // NONE
}

void SpecialEffectsController::trimCompletedOperations(){
    // Delete every completed op whose Effects are already drained, keep the rest. A completed op has
    // been through completeEffect() (which erases+deletes each Effect, emptying mEffects) and
    // complete() (which fired its copied listeners); the async clone/animation reference the VIEW,
    // not the op — so deleting is safe. The mEffects.empty() guard is defense-in-depth: this trim is
    // only called from executePendingOperations() entry (never forceCompleteAll, whose complete()
    // skips the drain), so every op here should already be empty, but a non-empty one is left in the
    // list rather than freed-while-referenced.
    mCompletedOperations.erase(
        std::remove_if(mCompletedOperations.begin(), mCompletedOperations.end(),
            [](Operation* op){
                if(!op) return true; // drop nulls
                if(op->mEffects.empty()){ delete op; return true; }
                return false; // keep: effects still referenced (should not happen at this call site)
            }),
        mCompletedOperations.end());
}

void SpecialEffectsController::executePendingOperations(){
    // Reclaim ops retired by a prior navigation round so mCompletedOperations never grows unbounded.
    trimCompletedOperations();
    if(mPendingOperations.empty()) return;
    std::vector<Operation*> ops = mPendingOperations;
    mPendingOperations.clear();
    for(Operation* op : ops) mRunningOperations.push_back(op);
    collectEffects(ops, false /*isPop threaded later*/);
    processStart(ops);
    commitEffects(ops);
}

void SpecialEffectsController::forceCompleteAll(){
    auto pending = mPendingOperations; mPendingOperations.clear();
    for(Operation* op : pending){ mRunningOperations.push_back(op); op->complete(); }
    auto running = mRunningOperations;
    for(Operation* op : running) if(!op->mIsComplete) op->complete();
}

void SpecialEffectsController::processStart(std::vector<Operation*>& ops){
    for(Operation* op : ops) op->onStart();
    std::vector<Effect*> effects;
    for(Operation* op : ops) for(Effect* e : op->mEffects) effects.push_back(e);
    for(Effect* e : effects) e->performStart(mContainer);
}

void SpecialEffectsController::commitEffects(std::vector<Operation*>& ops){
    std::vector<Effect*> effects;
    for(Operation* op : ops) for(Effect* e : op->mEffects) effects.push_back(e);
    for(Effect* e : effects) e->onCommit(mContainer);
    for(Operation* op : ops) applyContainerChangesToOperation(op);
    std::vector<Operation*> copy = ops;
    for(Operation* op : copy) if(op->mEffects.empty()) op->complete();
}

void SpecialEffectsController::applyContainerChangesToOperation(Operation* op){
    if(op->mIsAwaitingContainerChanges){
        op->applyState();
        op->mIsAwaitingContainerChanges = false;
    }
}

// --- Operation ---

SpecialEffectsController::Operation::Operation(State finalState, LifecycleImpact impact, Fragment* fragment, SpecialEffectsController* controller)
    : mFinalState(finalState), mLifecycleImpact(impact), mFragment(fragment), mController(controller){}

SpecialEffectsController::Operation::~Operation(){
    for(Effect* e : mEffects) delete e;
}

void SpecialEffectsController::Operation::addEffect(Effect* e){ if(e) mEffects.push_back(e); }
void SpecialEffectsController::Operation::addCompletionListener(std::function<void()> l){ mCompletionListeners.push_back(l); }

void SpecialEffectsController::Operation::completeEffect(Effect* e){
    auto it = std::find(mEffects.begin(), mEffects.end(), e);
    if(it != mEffects.end()){ mEffects.erase(it); delete e; }
    if(mEffects.empty()) complete();
}

void SpecialEffectsController::Operation::complete(){
    if(mIsComplete) return;
    mIsComplete = true;
    auto listeners = mCompletionListeners;
    for(auto& l : listeners) l();
}

void SpecialEffectsController::Operation::applyState(){
    if(!mFragment || !mFragment->mView || !mController->getContainer()) return;
    cdroid::View* view = mFragment->mView;
    ViewGroup* container = mController->getContainer();
    switch(mFinalState){
        case State::REMOVED:
            // Container change only; the orphaned view's reclamation is attached as a completion
            // listener back in enqueue() (this also covers the Animation path, which sets
            // isAwaitingContainerChanges=false and therefore never calls applyState).
            // Clear any legacy Animation first: if the view still carries mCurrentAnimation (a prior
            // custom enter/exit Animation that was never cleared), ViewGroup::removeViewInternal
            // routes it through addDisappearingView — the view then stays in mDisappearingChildren and
            // keeps being drawn, while TransitionEffect's clone-end post (or AnimationEffect's anim-end
            // post) frees the view shortly after -> a still-drawn freed view UAFs in applyLegacyAnimation
            // (mParent=null / RenderNode crash — the navdemo segfault, ~View of the view tree logged
            // right before the crash).
            if(view->getAnimation()) view->setAnimation(nullptr);
            if(view->getParent()) container->removeView(view);
            break;
        case State::VISIBLE:
            if(!view->getParent()) container->addView(view);
            view->setVisibility(cdroid::View::VISIBLE);
            break;
        case State::GONE:    view->setVisibility(cdroid::View::GONE); break;
        case State::INVISIBLE: view->setVisibility(cdroid::View::INVISIBLE); break;
    }
}

// --- FragmentStateManagerOperation ---

FragmentStateManagerOperation::FragmentStateManagerOperation(State finalState, LifecycleImpact impact,
        Fragment* fragment, SpecialEffectsController* controller, FragmentStateManager* fsm)
    : Operation(finalState, impact, fragment, controller), mFSM(fsm){}

void FragmentStateManagerOperation::complete(){
    FragmentStateManager* fsm = mFSM; // copy: super() listeners retire this op
    Operation::complete();            // listeners (running -> completed)
    if(fsm) fsm->moveToExpectedState(); // re-enter the FSM; awaiting-effect clamp lifts
}

}}//namespace fragment::cdroid
