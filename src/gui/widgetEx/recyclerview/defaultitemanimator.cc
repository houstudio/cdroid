#if 10
#include <widgetEx/recyclerview/defaultitemanimator.h>
namespace cdroid{

DefaultItemAnimator::MoveInfo::MoveInfo(RecyclerView::ViewHolder& holder, int fromX, int fromY, int toX, int toY) {
    this->holder = &holder;
    this->fromX = fromX;
    this->fromY = fromY;
    this->toX = toX;
    this->toY = toY;
}

DefaultItemAnimator::ChangeInfo::ChangeInfo(RecyclerView::ViewHolder& oldHolder, RecyclerView::ViewHolder& newHolder) {
    this->oldHolder = &oldHolder;
    this->newHolder = &newHolder;
}

DefaultItemAnimator::ChangeInfo::ChangeInfo(RecyclerView::ViewHolder& oldHolder, RecyclerView::ViewHolder& newHolder,
        int fromX, int fromY, int toX, int toY):ChangeInfo(oldHolder, newHolder){
    this->fromX = fromX;
    this->fromY = fromY;
    this->toX = toX;
    this->toY = toY;
}

void DefaultItemAnimator::runPendingAnimations() {
    bool removalsPending = !mPendingRemovals.empty();
    bool movesPending = !mPendingMoves.empty();
    bool changesPending = !mPendingChanges.empty();
    bool additionsPending = !mPendingAdditions.empty();
#if 0
    if (!removalsPending && !movesPending && !additionsPending && !changesPending) {
        // nothing to animate
        return;
    }
    // First, remove stuff
    for (RecyclerView::ViewHolder* holder : mPendingRemovals) {
        animateRemoveImpl(*holder);
    }
    mPendingRemovals.clear();
    // Next, move stuff
    if (movesPending) {
        std::vector<MoveInfo*> moves=mPendingMoves;
        mMovesList.push_back(moves);
        mPendingMoves.clear();
        Runnable mover = new Runnable() {
            public void run() {
                for (MoveInfo moveInfo : moves) {
                    animateMoveImpl(moveInfo.holder, moveInfo.fromX, moveInfo.fromY,
                            moveInfo.toX, moveInfo.toY);
                }
                moves.clear();
                mMovesList.remove(moves);
            }
        };
        if (removalsPending) {
            View* view = moves.at(0)->holder->itemView;
            view->postOnAnimationDelayed(mover, getRemoveDuration());
        } else {
            mover.run();
        }
    }
    // Next, change stuff, to run in parallel with move animations
    if (changesPending) {
        std::vector<ChangeInfo*> changes=mPendingChanges;// = new std::vector<>();
        mChangesList.push_back(changes);
        mPendingChanges.clear();
        Runnable changer = new Runnable() {
            public void run() {
                for (ChangeInfo change : changes) {
                    animateChangeImpl(change);
                }
                changes.clear();
                mChangesList.remove(changes);
            }
        };
        if (removalsPending) {
            RecyclerView::ViewHolder* holder = changes.at(0)->oldHolder;
            holder->itemView->postOnAnimationDelayed(changer, getRemoveDuration());
        } else {
            changer.run();
        }
    }
    // Next, add stuff
    if (additionsPending) {
        std::vector<RecyclerView::ViewHolder*> additions=mPendingAdditions;// = new std::vector<>();
        mAdditionsList.push_back(additions);
        mPendingAdditions.clear();
        Runnable adder = new Runnable() {
            public void run() {
                for (RecyclerView::ViewHolder* holder : additions) {
                    animateAddImpl(*holder);
                }
                additions.clear();
                mAdditionsList.remove(additions);
            }
        };
        if (removalsPending || movesPending || changesPending) {
            long removeDuration = removalsPending ? getRemoveDuration() : 0;
            long moveDuration = movesPending ? getMoveDuration() : 0;
            long changeDuration = changesPending ? getChangeDuration() : 0;
            long totalDelay = removeDuration + std::max(moveDuration, changeDuration);
            View* view = additions.at(0)->itemView;
            view->postOnAnimationDelayed(adder, totalDelay);
        } else {
            adder.run();
        }
    }
#endif
}

bool DefaultItemAnimator::animateRemove(RecyclerView::ViewHolder& holder) {
#if 0
    resetAnimation(holder);
    mPendingRemovals.add(holder);
#endif
    return true;
}

void DefaultItemAnimator::animateRemoveImpl(RecyclerView::ViewHolder& holder) {
#if 0
    View* view = holder.itemView;
    ViewPropertyAnimator* animation = view->animate();
    mRemoveAnimations.add(holder);
    animation.setDuration(getRemoveDuration()).alpha(0).setListener(
            new AnimatorListenerAdapter() {
                public void onAnimationStart(Animator animator) {
                    dispatchRemoveStarting(holder);
                }

                public void onAnimationEnd(Animator animator) {
                    animation.setListener(null);
                    view.setAlpha(1);
                    dispatchRemoveFinished(holder);
                    mRemoveAnimations.remove(holder);
                    dispatchFinishedWhenDone();
                }
            }).start();
#endif
}

bool DefaultItemAnimator::animateAdd(RecyclerView::ViewHolder& holder) {
    resetAnimation(holder);
    holder.itemView->setAlpha(0);
    mPendingAdditions.push_back(&holder);
    return true;
}

void DefaultItemAnimator::animateAddImpl(RecyclerView::ViewHolder& holder) {
#if 0
    View* view = holder.itemView;
    ViewPropertyAnimator* animation = view.animate();
    mAddAnimations.add(holder);
    animation.alpha(1).setDuration(getAddDuration())
            .setListener(new AnimatorListenerAdapter() {
                public void onAnimationStart(Animator animator) {
                    dispatchAddStarting(holder);
                }

                public void onAnimationCancel(Animator animator) {
                    view.setAlpha(1);
                }

                public void onAnimationEnd(Animator animator) {
                    animation.setListener(null);
                    dispatchAddFinished(holder);
                    mAddAnimations.remove(holder);
                    dispatchFinishedWhenDone();
                }
            }).start();
#endif
}

bool DefaultItemAnimator::animateMove(RecyclerView::ViewHolder& holder, int fromX, int fromY, int toX, int toY) {
#if 0
    View* view = holder->itemView;
    fromX += (int) holder.itemView.getTranslationX();
    fromY += (int) holder.itemView.getTranslationY();
    resetAnimation(holder);
    int deltaX = toX - fromX;
    int deltaY = toY - fromY;
    if (deltaX == 0 && deltaY == 0) {
        dispatchMoveFinished(holder);
        return false;
    }
    if (deltaX != 0) {
        view.setTranslationX(-deltaX);
    }
    if (deltaY != 0) {
        view.setTranslationY(-deltaY);
    }
    mPendingMoves.add(new MoveInfo(holder, fromX, fromY, toX, toY));
#endif
    return true;
}

void DefaultItemAnimator::animateMoveImpl(RecyclerView::ViewHolder& holder, int fromX, int fromY, int toX, int toY) {
#if 0
    View* view = holder.itemView;
    int deltaX = toX - fromX;
    int deltaY = toY - fromY;
    if (deltaX != 0) {
        view.animate().translationX(0);
    }
    if (deltaY != 0) {
        view.animate().translationY(0);
    }
    // TODO: make EndActions end listeners instead, since end actions aren't called when
    // vpas are canceled (and can't end them. why?)
    // need listener functionality in VPACompat for this. Ick.
    ViewPropertyAnimator* animation = view->animate();
    mMoveAnimations.add(holder);
    animation.setDuration(getMoveDuration()).setListener(new AnimatorListenerAdapter() {
        public void onAnimationStart(Animator animator) {
            dispatchMoveStarting(holder);
        }

        public void onAnimationCancel(Animator animator) {
            if (deltaX != 0) {
                view.setTranslationX(0);
            }
            if (deltaY != 0) {
                view.setTranslationY(0);
            }
        }

        public void onAnimationEnd(Animator animator) {
            animation.setListener(null);
            dispatchMoveFinished(holder);
            mMoveAnimations.remove(holder);
            dispatchFinishedWhenDone();
        }
    }).start();
#endif
}

bool DefaultItemAnimator::animateChange(RecyclerView::ViewHolder& oldHolder, RecyclerView::ViewHolder& newHolder,
        int fromX, int fromY, int toX, int toY) {
#if 0
    if (oldHolder == newHolder) {
        // Don't know how to run change animations when the same view holder is re-used.
        // run a move animation to handle position changes.
        return animateMove(oldHolder, fromX, fromY, toX, toY);
    }
    float prevTranslationX = oldHolder.itemView.getTranslationX();
    float prevTranslationY = oldHolder.itemView.getTranslationY();
    float prevAlpha = oldHolder.itemView.getAlpha();
    resetAnimation(oldHolder);
    int deltaX = (int) (toX - fromX - prevTranslationX);
    int deltaY = (int) (toY - fromY - prevTranslationY);
    // recover prev translation state after ending animation
    oldHolder.itemView.setTranslationX(prevTranslationX);
    oldHolder.itemView.setTranslationY(prevTranslationY);
    oldHolder.itemView.setAlpha(prevAlpha);
    if (newHolder != nullptr) {
        // carry over translation values
        resetAnimation(newHolder);
        newHolder.itemView.setTranslationX(-deltaX);
        newHolder.itemView.setTranslationY(-deltaY);
        newHolder.itemView.setAlpha(0);
    }
    mPendingChanges.add(new ChangeInfo(oldHolder, newHolder, fromX, fromY, toX, toY));
#endif
    return true;
}

void DefaultItemAnimator::animateChangeImpl(ChangeInfo& changeInfo) {
#if 0
    RecyclerView::ViewHolder* holder = changeInfo.oldHolder;
    View* view = holder == nullptr ? nullptr : holder->itemView;
    RecyclerView::ViewHolder* newHolder = changeInfo.newHolder;
    View* newView = newHolder != nullptr ? newHolder->itemView : nullptr;
    if (view != nullptr) {
        ViewPropertyAnimator* oldViewAnim = view.animate().setDuration(
                getChangeDuration());
        mChangeAnimations.add(changeInfo.oldHolder);
        oldViewAnim.translationX(changeInfo.toX - changeInfo.fromX);
        oldViewAnim.translationY(changeInfo.toY - changeInfo.fromY);
        oldViewAnim.alpha(0).setListener(new AnimatorListenerAdapter() {
            public void onAnimationStart(Animator animator) {
                dispatchChangeStarting(changeInfo.oldHolder, true);
            }

            public void onAnimationEnd(Animator animator) {
                oldViewAnim.setListener(nullptr);
                view.setAlpha(1);
                view.setTranslationX(0);
                view.setTranslationY(0);
                dispatchChangeFinished(changeInfo.oldHolder, true);
                mChangeAnimations.remove(changeInfo.oldHolder);
                dispatchFinishedWhenDone();
            }
        }).start();
    }
    if (newView != nullptr) {
        ViewPropertyAnimator* newViewAnimation = newView->animate();
        mChangeAnimations.add(changeInfo.newHolder);
        newViewAnimation.translationX(0).translationY(0).setDuration(getChangeDuration())
                .alpha(1).setListener(new AnimatorListenerAdapter() {
                    public void onAnimationStart(Animator animator) {
                        dispatchChangeStarting(changeInfo.newHolder, false);
                    }
                    public void onAnimationEnd(Animator animator) {
                        newViewAnimation.setListener(nullptr);
                        newView.setAlpha(1);
                        newView.setTranslationX(0);
                        newView.setTranslationY(0);
                        dispatchChangeFinished(changeInfo.newHolder, false);
                        mChangeAnimations.remove(changeInfo.newHolder);
                        dispatchFinishedWhenDone();
                    }
                }).start();
    }
#endif
}

void DefaultItemAnimator::endChangeAnimation(std::vector<ChangeInfo*>& infoList, RecyclerView::ViewHolder& item) {
#if 0
    for (int i = infoList.size() - 1; i >= 0; i--) {
        ChangeInfo* changeInfo = infoList.at(i);
        if (endChangeAnimationIfNecessary(*changeInfo, item)) {
            if (changeInfo->oldHolder == nullptr && changeInfo->newHolder == nullptr) {
                infoList.remove(changeInfo);
            }
        }
    }
#endif
}

void DefaultItemAnimator::endChangeAnimationIfNecessary(ChangeInfo& changeInfo) {
#if 0
    if (changeInfo.oldHolder != nullptr) {
        endChangeAnimationIfNecessary(changeInfo, changeInfo.oldHolder);
    }
    if (changeInfo.newHolder != nullptr) {
        endChangeAnimationIfNecessary(changeInfo, changeInfo.newHolder);
    }
#endif
}

bool DefaultItemAnimator::endChangeAnimationIfNecessary(ChangeInfo& changeInfo, RecyclerView::ViewHolder& item) {
    bool oldItem = false;
#if 0
    if (changeInfo.newHolder == item) {
        changeInfo.newHolder = nullptr;
    } else if (changeInfo.oldHolder == item) {
        changeInfo.oldHolder = nullptr;
        oldItem = true;
    } else {
        return false;
    }
    item.itemView->setAlpha(1);
    item.itemView->setTranslationX(0);
    item.itemView->setTranslationY(0);
    dispatchChangeFinished(item, oldItem);
#endif
    return true;
}

void DefaultItemAnimator::endAnimation(RecyclerView::ViewHolder& item) {
#if 0
    View* view = item.itemView;
    // this will trigger end callback which should set properties to their target values.
    view->animate().cancel();
    // TODO if some other animations are chained to end, how do we cancel them as well?
    for (int i = mPendingMoves.size() - 1; i >= 0; i--) {
        MoveInfo* moveInfo = mPendingMoves.at(i);
        if (moveInfo->holder == item) {
            view->setTranslationY(0);
            view->setTranslationX(0);
            dispatchMoveFinished(item);
            mPendingMoves.remove(i);
        }
    }
    endChangeAnimation(mPendingChanges, item);
    if (mPendingRemovals.remove(item)) {
        view.setAlpha(1);
        dispatchRemoveFinished(item);
    }
    if (mPendingAdditions.remove(item)) {
        view->setAlpha(1);
        dispatchAddFinished(item);
    }

    for (int i = mChangesList.size() - 1; i >= 0; i--) {
        std::vector<ChangeInfo*> changes = mChangesList.at(i);
        endChangeAnimation(changes, item);
        if (changes.empty()) {
            mChangesList.remove(i);
        }
    }
    for (int i = mMovesList.size() - 1; i >= 0; i--) {
        std::vector<MoveInfo*> moves = mMovesList.at(i);
        for (int j = moves.size() - 1; j >= 0; j--) {
            MoveInfo* moveInfo = moves.at(j);
            if (moveInfo->holder == item) {
                view->setTranslationY(0);
                view->setTranslationX(0);
                dispatchMoveFinished(item);
                moves.remove(j);
                if (moves.empty()) {
                    mMovesList.remove(i);
                }
                break;
            }
        }
    }
    for (int i = mAdditionsList.size() - 1; i >= 0; i--) {
        std::vector<RecyclerView::ViewHolder*> additions = mAdditionsList.at(i);
        if (additions.remove(item)) {
            view->setAlpha(1);
            dispatchAddFinished(item);
            if (additions.empty()) {
                mAdditionsList.remove(i);
            }
        }
    }

    // animations should be ended by the cancel above.
    //noinspection PointlessBooleanExpression,ConstantConditions
    if (mRemoveAnimations.remove(item) && DEBUG) {
        LOGE("after animation is cancelled, item should not be in mRemoveAnimations list");
    }

    //noinspection PointlessBooleanExpression,ConstantConditions
    if (mAddAnimations.remove(item) && DEBUG) {
        LOGE("after animation is cancelled, item should not be in mAddAnimations list");
    }

    //noinspection PointlessBooleanExpression,ConstantConditions
    if (mChangeAnimations.remove(item) && DEBUG) {
        LOGE("after animation is cancelled, item should not be in mChangeAnimations list");
    }

    //noinspection PointlessBooleanExpression,ConstantConditions
    if (mMoveAnimations.remove(item) && DEBUG) {
        LOGE("after animation is cancelled, item should not be in mMoveAnimations list");
    }
    dispatchFinishedWhenDone();
#endif
}

void DefaultItemAnimator::resetAnimation(RecyclerView::ViewHolder& holder) {
#if 0
    if (sDefaultInterpolator == nullptr) {
        sDefaultInterpolator = new ValueAnimator().getInterpolator();
    }
    holder.itemView.animate().setInterpolator(sDefaultInterpolator);
    endAnimation(holder);
#endif
}

bool DefaultItemAnimator::isRunning() {
    return (!mPendingAdditions.empty()
            || !mPendingChanges.empty()
            || !mPendingMoves.empty()
            || !mPendingRemovals.empty()
            || !mMoveAnimations.empty()
            || !mRemoveAnimations.empty()
            || !mAddAnimations.empty()
            || !mChangeAnimations.empty()
            || !mMovesList.empty()
            || !mAdditionsList.empty()
            || !mChangesList.empty());
}

void DefaultItemAnimator::dispatchFinishedWhenDone() {
    if (!isRunning()) {
        dispatchAnimationsFinished();
    }
}

void DefaultItemAnimator::endAnimations() {
#if 0
    int count = mPendingMoves.size();
    for (int i = count - 1; i >= 0; i--) {
        MoveInfo* item = mPendingMoves.at(i);
        View* view = item->holder->itemView;
        view->setTranslationY(0);
        view->setTranslationX(0);
        dispatchMoveFinished(item->holder);
        mPendingMoves.remove(i);
    }
    count = mPendingRemovals.size();
    for (int i = count - 1; i >= 0; i--) {
        RecyclerView::ViewHolder item = mPendingRemovals.at(i);
        dispatchRemoveFinished(item);
        mPendingRemovals.remove(i);
    }
    count = mPendingAdditions.size();
    for (int i = count - 1; i >= 0; i--) {
        RecyclerView::ViewHolder* item = mPendingAdditions.at(i);
        item->itemView->setAlpha(1);
        dispatchAddFinished(item);
        mPendingAdditions.remove(i);
    }
    count = mPendingChanges.size();
    for (int i = count - 1; i >= 0; i--) {
        endChangeAnimationIfNecessary(mPendingChanges.at(i));
    }
    mPendingChanges.clear();
    if (!isRunning()) {
        return;
    }

    int listCount = mMovesList.size();
    for (int i = listCount - 1; i >= 0; i--) {
        std::vector<MoveInfo*> moves = mMovesList.at(i);
        count = moves.size();
        for (int j = count - 1; j >= 0; j--) {
            MoveInfo* moveInfo = moves.get(j);
            RecyclerView::ViewHolder* item = moveInfo->holder;
            View* view = item->itemView;
            view->setTranslationY(0);
            view->setTranslationX(0);
            dispatchMoveFinished(moveInfo->holder);
            moves.remove(j);
            if (moves.empty()) {
                mMovesList.remove(moves);
            }
        }
    }
    listCount = mAdditionsList.size();
    for (int i = listCount - 1; i >= 0; i--) {
        std::vector<RecyclerView::ViewHolder*> additions = mAdditionsList.at(i);
        count = additions.size();
        for (int j = count - 1; j >= 0; j--) {
            RecyclerView::ViewHolder* item = additions.at(j);
            View* view = item->itemView;
            view->setAlpha(1);
            dispatchAddFinished(item);
            additions.remove(j);
            if (additions.empty()) {
                mAdditionsList.remove(additions);
            }
        }
    }
    listCount = mChangesList.size();
    for (int i = listCount - 1; i >= 0; i--) {
        std::vector<ChangeInfo*> changes = mChangesList.at(i);
        count = changes.size();
        for (int j = count - 1; j >= 0; j--) {
            endChangeAnimationIfNecessary(changes.at(j));
            if (changes.empty()) {
                mChangesList.remove(changes);
            }
        }
    }

    cancelAll(mRemoveAnimations);
    cancelAll(mMoveAnimations);
    cancelAll(mAddAnimations);
    cancelAll(mChangeAnimations);

    dispatchAnimationsFinished();
#endif
}

void DefaultItemAnimator::cancelAll(std::vector<RecyclerView::ViewHolder*>& viewHolders) {
    for (int i = viewHolders.size() - 1; i >= 0; i--) {
        //viewHolders.at(i)->itemView->animate().cancel();
    }
}

bool DefaultItemAnimator::canReuseUpdatedViewHolder(RecyclerView::ViewHolder& viewHolder, std::vector<Object*>& payloads) {
    return !payloads.empty() ;//|| SimpleItemAnimator::canReuseUpdatedViewHolder(viewHolder, payloads);
}
}/*endof namespace*/
#endif
