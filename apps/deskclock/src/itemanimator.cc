#include <itemanimator.h>

#include <animation/animatorset.h>
#include <animation/objectanimator.h>
#include <animation/propertyvaluesholder.h>
#include <view/view.h>

namespace cdroid {
namespace deskclock {

namespace {

/** Removes the first occurrence of animator from list; true if it was present. */
bool removeAnimator(std::vector<Animator*>& list, Animator* animator) {
    for (auto it = list.begin(); it != list.end(); ++it) {
        if (*it == animator) {
            list.erase(it);
            return true;
        }
    }
    return false;
}

} // namespace

ItemAnimator::~ItemAnimator() {
    for (AnimatorSet* set : mPendingSets) delete set;
    // Pending-list animators never adopted by a set are still owned here.
    for (Animator* a : mAddAnimatorsList) delete a;
    for (Animator* a : mRemoveAnimatorsList) delete a;
    for (Animator* a : mChangeAnimatorsList) delete a;
    for (Animator* a : mMoveAnimatorsList) delete a;
}

bool ItemAnimator::animateRemove(RecyclerView::ViewHolder& holder) {
    endAnimation(holder);

    const float prevAlpha = holder.itemView->getAlpha();

    Animator* removeAnimator = ObjectAnimator::ofFloat(holder.itemView, View::ALPHA, {0.0f});
    removeAnimator->setDuration(getRemoveDuration());
    Animator::AnimatorListener listener;
    listener.onAnimationStart = [this, &holder](Animator&, bool) {
        dispatchRemoveStarting(holder);
    };
    listener.onAnimationEnd = [this, &holder, prevAlpha](Animator& animator, bool) {
        animator.removeAllListeners();
        mAnimators.erase(&holder);
        holder.itemView->setAlpha(prevAlpha);
        dispatchRemoveFinished(holder);
    };
    removeAnimator->addListener(listener);
    mRemoveAnimatorsList.push_back(removeAnimator);
    mAnimators[&holder] = removeAnimator;
    return true;
}

bool ItemAnimator::animateAdd(RecyclerView::ViewHolder& holder) {
    endAnimation(holder);

    const float prevAlpha = holder.itemView->getAlpha();
    holder.itemView->setAlpha(0.0f);

    Animator* addAnimator = ObjectAnimator::ofFloat(holder.itemView, View::ALPHA, {1.0f});
    addAnimator->setDuration(getAddDuration());
    Animator::AnimatorListener listener;
    listener.onAnimationStart = [this, &holder](Animator&, bool) {
        dispatchAddStarting(holder);
    };
    listener.onAnimationEnd = [this, &holder, prevAlpha](Animator& animator, bool) {
        animator.removeAllListeners();
        mAnimators.erase(&holder);
        holder.itemView->setAlpha(prevAlpha);
        dispatchAddFinished(holder);
    };
    addAnimator->addListener(listener);
    mAddAnimatorsList.push_back(addAnimator);
    mAnimators[&holder] = addAnimator;
    return true;
}

bool ItemAnimator::animateMove(RecyclerView::ViewHolder& holder, int fromX, int fromY,
                               int toX, int toY) {
    endAnimation(holder);

    const int deltaX = toX - fromX;
    const int deltaY = toY - fromY;
    const int64_t moveDuration = getMoveDuration();

    if (deltaX == 0 && deltaY == 0) {
        dispatchMoveFinished(holder);
        return false;
    }

    View* view = holder.itemView;
    const float prevTranslationX = view->getTranslationX();
    const float prevTranslationY = view->getTranslationY();
    view->setTranslationX(-deltaX);
    view->setTranslationY(-deltaY);

    ObjectAnimator* moveAnimator;
    if (deltaX != 0 && deltaY != 0) {
        std::vector<PropertyValuesHolder*> holders;
        holders.push_back(PropertyValuesHolder::ofFloat(View::TRANSLATION_X, {0.0f}));
        holders.push_back(PropertyValuesHolder::ofFloat(View::TRANSLATION_Y, {0.0f}));
        moveAnimator = ObjectAnimator::ofPropertyValuesHolder(holder.itemView, holders);
    } else if (deltaX != 0) {
        std::vector<PropertyValuesHolder*> holders;
        holders.push_back(PropertyValuesHolder::ofFloat(View::TRANSLATION_X, {0.0f}));
        moveAnimator = ObjectAnimator::ofPropertyValuesHolder(holder.itemView, holders);
    } else {
        std::vector<PropertyValuesHolder*> holders;
        holders.push_back(PropertyValuesHolder::ofFloat(View::TRANSLATION_Y, {0.0f}));
        moveAnimator = ObjectAnimator::ofPropertyValuesHolder(holder.itemView, holders);
    }

    moveAnimator->setDuration(moveDuration);
    moveAnimator->setInterpolator(AnimatorUtils::INTERPOLATOR_FAST_OUT_SLOW_IN());
    Animator::AnimatorListener listener;
    listener.onAnimationStart = [this, &holder](Animator&, bool) {
        dispatchMoveStarting(holder);
    };
    listener.onAnimationEnd = [this, view, &holder, prevTranslationX, prevTranslationY]
            (Animator& animator, bool) {
        animator.removeAllListeners();
        mAnimators.erase(&holder);
        view->setTranslationX(prevTranslationX);
        view->setTranslationY(prevTranslationY);
        dispatchMoveFinished(holder);
    };
    moveAnimator->addListener(listener);
    mMoveAnimatorsList.push_back(moveAnimator);
    mAnimators[&holder] = moveAnimator;

    return true;
}

bool ItemAnimator::animateChange(RecyclerView::ViewHolder& oldHolder,
                                 RecyclerView::ViewHolder* newHolder,
                                 ItemHolderInfo& preLayoutInfo, ItemHolderInfo& postLayoutInfo) {
    endAnimation(oldHolder);
    if (newHolder != nullptr) endAnimation(*newHolder);

    const int64_t changeDuration = getChangeDuration();
    auto* prePayloadInfo = dynamic_cast<PayloadItemHolderInfo*>(&preLayoutInfo);
    std::vector<Object*>* payloads = prePayloadInfo ? &prePayloadInfo->payloads : nullptr;

    if (newHolder != nullptr && &oldHolder == newHolder) {
        OnAnimateChangeListener* listener = dynamic_cast<OnAnimateChangeListener*>(newHolder);
        Animator* animator = nullptr;
        if (listener) {
            animator = listener->onAnimateChange(payloads, preLayoutInfo.left, preLayoutInfo.top,
                    preLayoutInfo.right, preLayoutInfo.bottom, changeDuration);
        }
        if (animator == nullptr) {
            dispatchChangeFinished(*newHolder, false);
            return false;
        }
        Animator::AnimatorListener al;
        al.onAnimationStart = [this, newHolder](Animator&, bool) {
            dispatchChangeStarting(*newHolder, false);
        };
        al.onAnimationEnd = [this, newHolder](Animator& a, bool) {
            a.removeAllListeners();
            mAnimators.erase(newHolder);
            dispatchChangeFinished(*newHolder, false);
        };
        animator->addListener(al);
        mChangeAnimatorsList.push_back(animator);
        mAnimators[newHolder] = animator;
        return true;
    } else if (dynamic_cast<OnAnimateChangeListener*>(&oldHolder) == nullptr
            || dynamic_cast<OnAnimateChangeListener*>(newHolder) == nullptr) {
        // Both holders must implement OnAnimateChangeListener in order to animate.
        dispatchChangeFinished(oldHolder, true);
        if (newHolder != nullptr) dispatchChangeFinished(*newHolder, true);
        return false;
    }

    Animator* oldChangeAnimator = dynamic_cast<OnAnimateChangeListener&>(oldHolder)
            .onAnimateChange(oldHolder, *newHolder, changeDuration);
    if (oldChangeAnimator != nullptr) {
        Animator::AnimatorListener al;
        al.onAnimationStart = [this, &oldHolder](Animator&, bool) {
            dispatchChangeStarting(oldHolder, true);
        };
        al.onAnimationEnd = [this, &oldHolder](Animator& a, bool) {
            a.removeAllListeners();
            mAnimators.erase(&oldHolder);
            dispatchChangeFinished(oldHolder, true);
        };
        oldChangeAnimator->addListener(al);
        mAnimators[&oldHolder] = oldChangeAnimator;
        mChangeAnimatorsList.push_back(oldChangeAnimator);
    } else {
        dispatchChangeFinished(oldHolder, true);
    }

    Animator* newChangeAnimator = dynamic_cast<OnAnimateChangeListener*>(newHolder)
            ->onAnimateChange(oldHolder, *newHolder, changeDuration);
    if (newChangeAnimator != nullptr) {
        Animator::AnimatorListener al;
        al.onAnimationStart = [this, newHolder](Animator&, bool) {
            dispatchChangeStarting(*newHolder, false);
        };
        al.onAnimationEnd = [this, newHolder](Animator& a, bool) {
            a.removeAllListeners();
            mAnimators.erase(newHolder);
            dispatchChangeFinished(*newHolder, false);
        };
        newChangeAnimator->addListener(al);
        mAnimators[newHolder] = newChangeAnimator;
        mChangeAnimatorsList.push_back(newChangeAnimator);
    } else {
        dispatchChangeFinished(*newHolder, false);
    }

    return true;
}

bool ItemAnimator::animateChange(RecyclerView::ViewHolder& oldHolder,
                                 RecyclerView::ViewHolder* newHolder,
                                 int fromLeft, int fromTop, int toLeft, int toTop) {
    /* Unused */
    throw std::logic_error("This method should not be used");
}

void ItemAnimator::runPendingAnimations() {
    // Prune finished roots (their batches completed; the set and its adopted
    // children can be released now).
    for (auto it = mPendingSets.begin(); it != mPendingSets.end();) {
        if (!(*it)->isRunning()) {
            delete *it;
            it = mPendingSets.erase(it);
        } else {
            ++it;
        }
    }

    AnimatorSet* removeAnimatorSet = new AnimatorSet();
    removeAnimatorSet->playTogether(mRemoveAnimatorsList);
    mRemoveAnimatorsList.clear();

    AnimatorSet* addAnimatorSet = new AnimatorSet();
    addAnimatorSet->playTogether(mAddAnimatorsList);
    mAddAnimatorsList.clear();

    AnimatorSet* changeAnimatorSet = new AnimatorSet();
    changeAnimatorSet->playTogether(mChangeAnimatorsList);
    mChangeAnimatorsList.clear();

    AnimatorSet* moveAnimatorSet = new AnimatorSet();
    moveAnimatorSet->playTogether(mMoveAnimatorsList);
    mMoveAnimatorsList.clear();

    AnimatorSet* pendingAnimatorSet = new AnimatorSet();
    Animator::AnimatorListener finisher;
    finisher.onAnimationEnd = [this](Animator& animator, bool) {
        animator.removeAllListeners();
        dispatchFinishedWhenDone();
    };
    pendingAnimatorSet->addListener(finisher);
    // Required order: removes, then changes & moves simultaneously, then additions. There are
    // redundant edges because changes or moves may be empty, causing the removes to incorrectly
    // play immediately.
    pendingAnimatorSet->play(removeAnimatorSet)->before(changeAnimatorSet);
    pendingAnimatorSet->play(removeAnimatorSet)->before(moveAnimatorSet);
    pendingAnimatorSet->play(changeAnimatorSet)->with(moveAnimatorSet);
    pendingAnimatorSet->play(addAnimatorSet)->after(changeAnimatorSet);
    pendingAnimatorSet->play(addAnimatorSet)->after(moveAnimatorSet);
    pendingAnimatorSet->start();
    mPendingSets.push_back(pendingAnimatorSet);
}

void ItemAnimator::endAnimation(RecyclerView::ViewHolder& holder) {
    auto it = mAnimators.find(&holder);
    Animator* animator = it != mAnimators.end() ? it->second : nullptr;
    mAnimators.erase(&holder);

    // Pending-list membership marks animators this animator still owns (not yet
    // adopted by a started AnimatorSet); those are freed after being ended.
    bool owned = false;
    if (animator != nullptr) {
        owned = removeAnimator(mAddAnimatorsList, animator)
                || removeAnimator(mRemoveAnimatorsList, animator)
                || removeAnimator(mChangeAnimatorsList, animator)
                || removeAnimator(mMoveAnimatorsList, animator);
    }

    if (animator != nullptr) animator->end();
    if (owned) delete animator;
    dispatchFinishedWhenDone();
}

void ItemAnimator::endAnimations() {
    std::vector<Animator*> animatorList;
    for (auto& entry : mAnimators) animatorList.push_back(entry.second);
    for (Animator* animator : animatorList) {
        animator->end();
    }
    dispatchFinishedWhenDone();
}

bool ItemAnimator::isRunning() {
    return !mAnimators.empty();
}

void ItemAnimator::dispatchFinishedWhenDone() {
    if (!isRunning()) {
        dispatchAnimationsFinished();
    }
}

RecyclerView::ItemAnimator::ItemHolderInfo* ItemAnimator::recordPreLayoutInformation(
        RecyclerView::State& state, RecyclerView::ViewHolder& viewHolder,
        int changeFlags, std::vector<Object*>& payloads) {
    ItemHolderInfo* itemHolderInfo = SimpleItemAnimator::recordPreLayoutInformation(
            state, viewHolder, changeFlags, payloads);
    if (auto* info = dynamic_cast<PayloadItemHolderInfo*>(itemHolderInfo)) {
        info->payloads = payloads;
    }
    return itemHolderInfo;
}

RecyclerView::ItemAnimator::ItemHolderInfo* ItemAnimator::obtainHolderInfo() {
    return new PayloadItemHolderInfo();
}

bool ItemAnimator::canReuseUpdatedViewHolder(RecyclerView::ViewHolder& viewHolder,
                                             std::vector<Object*>& payloads) {
    const bool defaultReusePolicy =
            RecyclerView::ItemAnimator::canReuseUpdatedViewHolder(viewHolder, payloads);
    // Whenever we have a payload, this is an in-place animation.
    return !payloads.empty() || defaultReusePolicy;
}

} // namespace deskclock
} // namespace cdroid
