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
        lst.onAnimationEnd = [cont, v, op, self](Animation&){
            if(v->getParent()) cont->removeView(v);
            // The animClone has ended and the view is detached — reclaim it (CDROID's removeView is
            // detach-only; java GC would reclaim it). completeEffect retires the op; mView is null by
            // then so the FSM re-entry touches a different (null) pointer, not this view.
            cont->post([op, self, v](){ op->completeEffect(self); delete v; });
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
            auto scheduleDelete = [cont, view, fired](){
                if(*fired) return; *fired = true;
                cont->post([view]{ delete view; });
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
