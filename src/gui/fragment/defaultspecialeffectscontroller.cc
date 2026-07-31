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
            cont->post([op, self](){ op->completeEffect(self); });
        };
        animClone->setAnimationListener(lst);
        view->startAnimation(animClone);
    }
}

void TransitionEffect::onCommit(ViewGroup* container){
    if(!mTransition || !container){ mOperation->completeEffect(this); return; }
    // beginDelayedTransition captures the container state; the subsequent applyState (addView/
    // removeView) is what it animates. Complete synchronously — the transition runs async.
    TransitionManager::beginDelayedTransition(container, mTransition);
    mOperation->completeEffect(this);
}

}}//namespace fragment::cdroid
