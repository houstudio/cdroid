#include <fragment/defaultspecialeffectscontroller.h>
#include <fragment/fragment.h>
#include <fragment/fragmentanim.h>
#include <fragment/fragmenttransitionimpl.h>
#include <animation/animation.h>
#include <transition/transition.h>
#include <transition/transitionmanager.h>
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
    // Per op: custom anim -> AnimationEffect; otherwise -> TransitionEffect (default Fade/shared).
    for(Operation* op : operations){
        Fragment* f = op->mFragment;
        if(!f || !f->mView) continue;
        bool enter = (op->mFinalState == Operation::State::VISIBLE);
        Context* ctx = f->getContext();
        Animation* anim = ctx ? FragmentAnim::loadAnimation(ctx, f, enter, isPop) : nullptr;
        if(anim){
            op->addEffect(new AnimationEffect(op, anim));
        } else {
            Transition* t = (op->mFinalState == Operation::State::REMOVED)
                ? FragmentTransitionImpl::makeExitTransition()
                : FragmentTransitionImpl::makeEnterTransition({});
            op->addEffect(new TransitionEffect(op, t));
        }
        // Ensure the container change (addView/removeView) applies via applyState on completion.
        op->addCompletionListener([this, op](){ applyContainerChangesToOperation(op); });
    }
}

void AnimationEffect::onCommit(ViewGroup* container){
    Fragment* f = mOperation->mFragment;
    if(!f || !f->mView || !mAnimation){ mOperation->completeEffect(this); return; }
    cdroid::View* view = f->mView;
    if(mOperation->mFinalState != SpecialEffectsController::Operation::State::REMOVED){
        // add/show: complete immediately (cannot attach an end listener without clobbering the
        // fragment's own; per androidx AnimationEffect.onCommit).
        view->startAnimation(mAnimation);
        mOperation->completeEffect(this);
    } else {
        // remove: complete on animation end; defer completeEffect via post so we don't delete the
        // Animation while still inside its own onAnimationEnd callback.
        Animation::AnimationListener lst;
        ViewGroup* cont = container;
        auto* op = mOperation;   // SpecialEffectsController::Operation*
        auto* self = this;       // AnimationEffect* (= SpecialEffectsController::Effect*)
        cdroid::View* v = view;
        lst.onAnimationEnd = [cont, v, op, self](Animation&){
            if(v->getParent()) cont->removeView(v);
            cont->post([op, self](){ op->completeEffect(self); });
        };
        mAnimation->setAnimationListener(lst);
        view->startAnimation(mAnimation);
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
