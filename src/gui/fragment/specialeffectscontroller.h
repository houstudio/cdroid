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
#ifndef __SPECIALEFFECTSCONTROLLER_H__
#define __SPECIALEFFECTSCONTROLLER_H__
/*********************************************************************************
 * Port of androidx SpecialEffectsController (simplified: Animation + Transition, no Animator/seek).
 * Per-container effect queue: collects Operations, lets the default subclass pick Animation vs
 * Transition, commits them. FragmentStateManager integrates via enqueue + awaiting-effect clamp;
 * an Operation's final effect completion re-enters the FSM.
 *********************************************************************************/
#include <vector>
#include <string>
#include <functional>
#include <memory>
namespace cdroid{
class ViewGroup;
class View;
namespace fragment{
class Fragment;
class FragmentStateManager;

class SpecialEffectsController{
public:
    class Effect; // forward (defined below Operation)
    class Operation{
    public:
        enum class State{ REMOVED, VISIBLE, GONE, INVISIBLE };
        enum class LifecycleImpact{ NONE, ADDING, REMOVING };
        Operation(State finalState, LifecycleImpact impact, Fragment* fragment, SpecialEffectsController* controller);
        virtual ~Operation();
        void addEffect(Effect* e);
        // add-only single-callback event (androidx Operation.addCompletionListener(Runnable)).
        // std::function<void()> value-stored so the vector owns the listeners.
        void addCompletionListener(std::function<void()> l);
        void completeEffect(Effect* e);
        virtual void complete();
        virtual void onStart(){}
        void applyState();
        State mFinalState;
        LifecycleImpact mLifecycleImpact;
        Fragment* mFragment;
        std::vector<Effect*> mEffects;
        std::vector<std::function<void()>> mCompletionListeners;
        bool mIsComplete = false;
        bool mIsStarted = false;
        bool mIsAwaitingContainerChanges = true;
        SpecialEffectsController* mController;
        // Effect-end-gated fragment reclamation hook. Set in enqueue() to capture the owning FM, the
        // fragment, and a weak alive-guard — invoked by TransitionEffect/AnimationEffect's posted
        // end-lambda (after `delete view`) to reclaim the Fragment instance once its exit effect has
        // truly ended. Captures fm+f (NEVER the FSM: destroy-sweep may delete the FSM before a late
        // posted hook fires). Copied BY VALUE into the posted lambda so trimming the op later is safe.
        std::function<void()> mReclaimHook;
    };

    class Effect{
    public:
        Effect(Operation* op) : mOperation(op){}
        virtual ~Effect(){}
        virtual void performStart(ViewGroup*){}
        virtual void onCommit(ViewGroup* container) = 0;
        virtual void onCancel(ViewGroup*){}
    protected:
        Operation* mOperation;
    };

    explicit SpecialEffectsController(ViewGroup* container) : mContainer(container), mAlive(std::make_shared<bool>(true)){}
    virtual ~SpecialEffectsController();

    void enqueueAdd(int finalStateVisibility, FragmentStateManager* fsm);
    void enqueueRemove(FragmentStateManager* fsm);
    void enqueueHide(FragmentStateManager* fsm);
    void enqueueShow(FragmentStateManager* fsm);
    int getAwaitingCompletionLifecycleImpact(FragmentStateManager* fsm);
    void executePendingOperations();
    void forceCompleteAll();
    virtual void collectEffects(std::vector<Operation*>& operations, bool isPop) = 0;
    ViewGroup* getContainer() const { return mContainer; }
    // True if the latest collectEffects round built any Transition (clone-based) effect. An exit
    // AnimationEffect consults this: a sibling clone's ObjectAnimator keeps dereferencing the exit
    // view in Choreographer CALLBACK_ANIMATION for frames after the legacy anim ends, so the view
    // must survive (hidden GONE) until that clone ends — freeing on anim-end would UAF. With no
    // clone on the container, nothing references the view post-anim and it is freed at once.
    bool hasTransitionEffect() const { return mHasTransitionEffect; }
    // Alive-guard source for callbacks (clone-end listeners) that may outlive this controller: a
    // weak handle is captured so a late callback becomes a no-op once the controller is gone.
    std::shared_ptr<bool> getAlive() const { return mAlive; }
    // Park an exit view (already hidden GONE by the caller) to be freed once the sibling clone truly
    // ends. reclaimDeferredExitViews() detaches + deletes every parked view.
    void deferExitViewDelete(View* v);
    void reclaimDeferredExitViews();

private:
    // Delete every completed op in mCompletedOperations whose effects are already drained, then clear
    // the list. Called at executePendingOperations() entry so the completed list never grows unbounded
    // across navigations. NOT called from forceCompleteAll — see .cc (forceCompleteAll's complete()
    // does not drain mEffects, so an Animation REMOVED op whose onAnimationEnd post is still pending
    // would be freed while that post still references it).
    void trimCompletedOperations();
    // Exit views kept alive (GONE, still parented) because a sibling Transition clone still
    // dereferences them in CALLBACK_ANIMATION after their own legacy anim ended. Freed by
    // reclaimDeferredExitViews() at clone-end (TransitionEffect listener, alive-guarded). NOT freed
    // at forceCompleteAll/destroy — the view stays GONE in the container and ~ViewGroup reclaims it,
    // avoiding a free-while-clone-still-runs UAF.
    std::vector<View*> mLingeryExitViews;

protected:
    void processStart(std::vector<Operation*>& ops);
    void commitEffects(std::vector<Operation*>& ops);
    void applyContainerChangesToOperation(Operation* op);
    void enqueue(Operation::State finalState, Operation::LifecycleImpact impact, FragmentStateManager* fsm);

    ViewGroup* mContainer;
    bool mHasTransitionEffect = false;          // set by the derived collectEffects each round
    std::shared_ptr<bool> mAlive;               // weak-guard source for outliving clone callbacks
    std::vector<Operation*> mPendingOperations;
    std::vector<Operation*> mRunningOperations;
    std::vector<Operation*> mCompletedOperations; // owned; freed in dtor
};

// Operation tied to a FragmentStateManager; on complete it re-enters the FSM (androidx bridge).
class FragmentStateManagerOperation : public SpecialEffectsController::Operation{
public:
    FragmentStateManagerOperation(State finalState, LifecycleImpact impact, Fragment* fragment,
                                 SpecialEffectsController* controller, FragmentStateManager* fsm);
    void complete() override;
private:
    FragmentStateManager* mFSM;
};

}}//namespace fragment::cdroid
#endif/*__SPECIALEFFECTSCONTROLLER_H__*/
