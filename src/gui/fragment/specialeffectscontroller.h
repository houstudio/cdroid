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
#include <core/callbackbase.h> // CallbackBase<void> == Runnable (completion-listener value type)
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
        // CallbackBase<void> (== Runnable): value-stored so the vector owns them, and identity
        // (shared functor pointer) would let a future removeCompletionListener match by handle.
        void addCompletionListener(const CallbackBase<void>& l);
        void completeEffect(Effect* e);
        virtual void complete();
        virtual void onStart(){}
        void applyState();
        State mFinalState;
        LifecycleImpact mLifecycleImpact;
        Fragment* mFragment;
        std::vector<Effect*> mEffects;
        std::vector<CallbackBase<void>> mCompletionListeners;
        bool mIsComplete = false;
        bool mIsStarted = false;
        bool mIsAwaitingContainerChanges = true;
        SpecialEffectsController* mController;
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

    explicit SpecialEffectsController(ViewGroup* container) : mContainer(container){}
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

protected:
    void processStart(std::vector<Operation*>& ops);
    void commitEffects(std::vector<Operation*>& ops);
    void applyContainerChangesToOperation(Operation* op);
    void enqueue(Operation::State finalState, Operation::LifecycleImpact impact, FragmentStateManager* fsm);

    ViewGroup* mContainer;
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
