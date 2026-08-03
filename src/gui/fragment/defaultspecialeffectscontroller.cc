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
#include <fragment/defaultspecialeffectscontroller.h>
#include <fragment/fragment.h>
#include <fragment/fragmentanim.h>
#include <fragment/fragmenttransitionimpl.h>
#include <animation/animation.h>
#include <transition/transition.h>
#include <transition/transitionmanager.h>
#include <transition/transitioninflater.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <core/context.h>
#include <memory>

namespace cdroid{
namespace fragment{

void DefaultSpecialEffectsController::collectEffects(std::vector<Operation*>& operations, bool isPop){
    // syncAnimations: the last operation's anims propagate to every fragment in the batch (androidx),
    // so sibling fragments in one transaction share the same enter/exit set.
    if(!operations.empty()){
        Operation* last = operations.back();
        Fragment* lastFrag = last ? last->mFragment : nullptr;
        if(lastFrag){
            for(Operation* op : operations){
                Fragment* f = op->mFragment;
                if(f && f != lastFrag){
                    f->setAnimations(lastFrag->mEnterAnim, lastFrag->mExitAnim,
                                     lastFrag->mPopEnterAnim, lastFrag->mPopExitAnim);
                }
            }
        }
    }
    // Per op: Fragment Transition API (highest) > custom anim (R.anim) > default Fade/shared.
    mHasTransitionEffect = false; // recompute per round; set true when any op takes a Transition clone
    for(Operation* op : operations){
        Fragment* f = op->mFragment;
        if(!f || !f->mView) continue;
        bool enter = (op->mFinalState == Operation::State::VISIBLE);
        // Priority 1: Fragment Transition (enterTransition/exitTransition/reenter/return).
        Transition* fragTrans = nullptr;
        if(enter){
            fragTrans = isPop ? (f->mReenterTransition ? f->mReenterTransition : f->mEnterTransition)
                              : f->mEnterTransition;
        } else {
            fragTrans = isPop ? (f->mReturnTransition ? f->mReturnTransition : f->mExitTransition)
                              : f->mExitTransition;
        }
        if(fragTrans){
            op->addEffect(new TransitionEffect(op, fragTrans->clone()));
            mHasTransitionEffect = true;
        } else {
            // Priority 2: custom anim (R.anim) — legacy Animation.
            Context* ctx = f->getContext();
            Animation* anim = ctx ? FragmentAnim::loadAnimation(ctx, f, enter, isPop) : nullptr;
            if(anim){
                op->addEffect(new AnimationEffect(op, anim));
            } else {
                // Priority 3: default Fade / shared element Transition.
                Transition* t = (op->mFinalState == Operation::State::REMOVED)
                    ? FragmentTransitionImpl::makeExitTransition()
                    : FragmentTransitionImpl::makeEnterTransition({});
                op->addEffect(new TransitionEffect(op, t));
                mHasTransitionEffect = true;
            }
        }
        // Ensure the container change (addView/removeView) applies via applyState on completion.
        op->addCompletionListener([this, op](){ applyContainerChangesToOperation(op); });
    }
}

void AnimationEffect::onCommit(ViewGroup* container){
    Fragment* f = mOperation->mFragment;
    if(!f || !f->mView || !mAnimation){ mOperation->completeEffect(this); return; }
    cdroid::View* view = f->mView;
    // Clone for the View: View::setAnimation deletes the previous animation, so we must hand it
    // its own copy; the original mAnimation is owned by this Effect (deleted in dtor).
    Animation* animClone = mAnimation->clone();
    if(mOperation->mFinalState != SpecialEffectsController::Operation::State::REMOVED){
        // add/show: complete immediately (cannot attach an end listener without clobbering the
        // fragment's own; per androidx AnimationEffect.onCommit).
        view->startAnimation(animClone);
        mOperation->completeEffect(this);
    } else {
        // remove: let the animation play while the view is still attached (don't removeView in
        // commitEffects — set isAwaitingContainerChanges=false). onAnimationEnd removes the view
        // + completes. This mirrors androidx EndViewTransitionAnimation (view stays until anim ends).
        mOperation->mIsAwaitingContainerChanges = false;
        Animation::AnimationListener lst;
        ViewGroup* cont = container;
        auto* op = mOperation;
        auto* self = this;
        cdroid::View* v = view;
        // Copy the reclaim hook BY VALUE (see TransitionEffect): completeEffect runs INSIDE this post
        // (async), so the hook fires after it — that ordering is what makes mState==INITIALIZING
        // observable for reclaimFragment's guard.
        auto hook = mOperation->mReclaimHook;
        lst.onAnimationEnd = [cont, v, op, self, hook](Animation&){
            // Defer to a post: inside this listener the animClone is still on the call stack (it
            // invoked us), so we cannot setAnimation(nullptr) (which deletes it) yet. In the post the
            // listener has returned; clearing mCurrentAnimation first makes removeView take the
            // dispatchDetachedFromWindow branch instead of addDisappearingView — otherwise the view
            // stays in mDisappearingChildren and keeps being drawn, and the delete below would free a
            // view still referenced there (UAF on the next draw: RenderNode/applyLegacyAnimation on a
            // freed view whose mParent is null — the navdemo crash).
            cont->post([cont, v, op, self, hook](){
                v->setAnimation(nullptr);              // delete the ended animClone; clear mCurrentAnimation
                op->completeEffect(self);
                if(hook) hook();                       // reclaim fragment now (independent of the view)
                if(op->mController && op->mController->hasTransitionEffect()){
                    // A sibling TransitionEffect's clone (beginDelayedTransition on this container)
                    // captured v in its startValues, and its ObjectAnimator keeps advancing in
                    // Choreographer CALLBACK_ANIMATION for frames after this legacy anim ends. Freeing
                    // v now (drainMessageQueue) would leave the clone dereferencing freed memory next
                    // CALLBACK_ANIMATION -> UAF. Keep v alive (GONE, still parented) and hand it to the
                    // controller; the clone's true end (TransitionEffect listener) reclaims it once
                    // nothing references it anymore.
                    v->setVisibility(cdroid::View::GONE);
                    op->mController->deferExitViewDelete(v);
                } else {
                    // No Transition clone on this container -> nothing references v post-anim. Safe to
                    // detach + free now (mCurrentAnimation already cleared above so removeView takes
                    // the plain detach branch instead of addDisappearingView).
                    if(v->getParent()) cont->removeView(v);
                    delete v;
                }
            });
        };
        animClone->setAnimationListener(lst);
        view->startAnimation(animClone);
    }
}

void TransitionEffect::onCommit(ViewGroup* container){
    if(!mTransition || !container){ mOperation->completeEffect(this); return; }
    // beginDelayedTransition clones mTransition and returns the RUNNING clone (the one that actually
    // animates). clone() no longer inherits the original's listeners (copyCloneFields clears them),
    // so to reclaim the fragment view when the clone truly ends we addListener() on the returned
    // clone directly — not on mTransition, not on op-completion.
    Transition* clone = TransitionManager::beginDelayedTransition(container, mTransition);
    // Reclaim legacy-Animation exit views (deferred by a sibling AnimationEffect) when this clone
    // truly ends — by then its ObjectAnimator no longer dereferences them, so freeing is safe.
    // The alive-guard makes the callback a no-op if the controller is already destroyed.
    SpecialEffectsController* ctrl = mOperation->mController;
    if(clone && ctrl){
        std::weak_ptr<bool> alive = ctrl->getAlive();
        Transition::TransitionListener reclaimLst;
        reclaimLst.onTransitionEnd    = [ctrl, alive](Transition&){ if(alive.lock()) ctrl->reclaimDeferredExitViews(); };
        reclaimLst.onTransitionCancel = [ctrl, alive](Transition&){ if(alive.lock()) ctrl->reclaimDeferredExitViews(); };
        clone->addListener(reclaimLst);
    }
    if(mOperation->mFinalState == SpecialEffectsController::Operation::State::REMOVED){
        Fragment* f = mOperation->mFragment;
        cdroid::View* view = f ? f->mView : nullptr;
        if(view){
            // Reclaim on the clone's true end, NOT on op-completion: complete() is synchronous below,
            // before the async clone plays, so a delete-on-complete would UAF the clone's
            // startValues->view. completeEffect stays synchronous so the op retires during
            // commitEffects and the FSM's awaiting-effect clamp (floors expected at the un-landable
            // AWAITING_EXIT_EFFECTS=3) is never observed — no busy-loop. The delete is POSTED past
            // onTransitionEnd so end()'s overlay teardown finishes first. Reclaim never calls
            // completeEffect/moveToExpectedState. Sole-deleter: ~Fragment doesn't delete mView.
            auto fired = std::make_shared<bool>(false);
            ViewGroup* cont = container;
            // Copy the reclaim hook BY VALUE into the posted lambda (don't read op->mReclaimHook
            // inside it): trim may delete this op before the post fires. Invoked after `delete view`
            // — by then completeEffect has run synchronously and the FSM has walked the fragment to
            // its final state (INITIALIZING for a hard remove), so reclaimFragment's mState guard is
            // observable.
            auto hook = mOperation->mReclaimHook;
            auto scheduleDelete = [cont, view, fired, hook](){
                if(*fired) return; *fired = true;
                cont->post([cont, view, hook]{
                    // Detach from the parent BEFORE delete: ~View only does mParent->removeViewInternal
                    // (mChildren), and an addDisappearingView'd view has mParent==null while still
                    // listed in mDisappearingChildren — so ~View wouldn't pull it out, leaving the
                    // parent drawing a freed view. Remove explicitly so neither list retains it.
                    if(view->getParent()) view->getParent()->removeView(view);
                    delete view;
                    if(hook) hook();
                });
            };
            if(clone){
                Transition::TransitionListener lst;
                lst.onTransitionEnd = [scheduleDelete](Transition&){ scheduleDelete(); };
                lst.onTransitionCancel = [scheduleDelete](Transition&){ scheduleDelete(); };
                clone->addListener(lst);
            } else {
                // no-op (not laid out / already pending): no clone, no animator referencing the view.
                scheduleDelete();
            }
        }
    }
    mOperation->completeEffect(this);  // synchronous: retire op now (running->completed), dodge the clamp
}

}}//namespace fragment::cdroid
