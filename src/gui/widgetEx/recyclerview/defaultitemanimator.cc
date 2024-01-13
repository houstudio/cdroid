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
    const bool removalsPending = !mPendingRemovals.empty();
    const bool movesPending = !mPendingMoves.empty();
    const bool changesPending = !mPendingChanges.empty();
    const bool additionsPending = !mPendingAdditions.empty();
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
        mMovesList.push_back(mPendingMoves);
        std::vector<MoveInfo*>&moves=mMovesList.back();
        mPendingMoves.clear();
        Runnable mover;
        mover=[this,&moves]() {
            for (MoveInfo* moveInfo : moves) {
                animateMoveImpl(*moveInfo->holder, moveInfo->fromX, moveInfo->fromY,
                        moveInfo->toX, moveInfo->toY);
            }
            auto it = std::find(mMovesList.begin(),mMovesList.end(),moves);
            moves.clear();
            mMovesList.erase(it);//remove(moves);
        };
        if (removalsPending) {
            View* view = moves.at(0)->holder->itemView;
            view->postOnAnimationDelayed(mover, getRemoveDuration());
        } else {
            mover();//.run();
        }
    }
    // Next, change stuff, to run in parallel with move animations
    if (changesPending) {
        mChangesList.push_back(mPendingChanges);
        std::vector<ChangeInfo*>& changes = mChangesList.back();
        mPendingChanges.clear();
        Runnable changer;
        changer= [this,&changes]() {
            for (ChangeInfo* change : changes) {
                animateChangeImpl(*change);
            }
            auto it = std::find(mChangesList.begin(),mChangesList.end(),changes);
            changes.clear();
            mChangesList.erase(it);//remove(changes);
        };
        if (removalsPending) {
            RecyclerView::ViewHolder* holder = changes.at(0)->oldHolder;
            holder->itemView->postOnAnimationDelayed(changer, getRemoveDuration());
        } else {
            changer();//.run();
        }
    }
    // Next, add stuff
    if (additionsPending) {
       mAdditionsList.push_back(mPendingAdditions);
       std::vector<RecyclerView::ViewHolder*>& additions=mAdditionsList.back();
       mPendingAdditions.clear();
       Runnable adder;
       adder = [this,&additions]() {
            for (RecyclerView::ViewHolder* holder : additions) {
                animateAddImpl(*holder);
            }
            additions.clear();
            auto it = std::find(mAdditionsList.begin(),mAdditionsList.end(),additions);
            mAdditionsList.erase(it);//remove(additions);
        };
        if (removalsPending || movesPending || changesPending) {
            long removeDuration = removalsPending ? getRemoveDuration() : 0;
            long moveDuration = movesPending ? getMoveDuration() : 0;
            long changeDuration = changesPending ? getChangeDuration() : 0;
            long totalDelay = removeDuration + std::max(moveDuration, changeDuration);
            View* view = additions.at(0)->itemView;
            view->postOnAnimationDelayed(adder, totalDelay);
        } else {
            adder();//.run();
        }
    }
}

bool DefaultItemAnimator::animateRemove(RecyclerView::ViewHolder& holder) {
    resetAnimation(holder);
    mPendingRemovals.push_back(&holder);
    return true;
}

void DefaultItemAnimator::animateRemoveImpl(RecyclerView::ViewHolder& holder) {
 
    View* view = holder.itemView;
    ViewPropertyAnimator* animation;// = view->animate();
    mRemoveAnimations.push_back(&holder);//add(holder);
    //AnimatorListenerAdapter
    Animator::AnimatorListener al;
    al.onAnimationStart=[this,&holder](Animator&animator,bool isReverse){
        dispatchRemoveStarting(holder);
    };
    al.onAnimationEnd=[this,animation,&holder](Animator& animator,bool isReverse){
        animation->setListener({});
        holder.itemView->setAlpha(1);
        dispatchRemoveFinished(holder);
        //mRemoveAnimations.remove(holder);
        dispatchFinishedWhenDone();
    };
    animation->setDuration(getRemoveDuration()).alpha(0).setListener(al).start();
}

bool DefaultItemAnimator::animateAdd(RecyclerView::ViewHolder& holder) {
    resetAnimation(holder);
    holder.itemView->setAlpha(0);
    mPendingAdditions.push_back(&holder);
    return true;
}

void DefaultItemAnimator::animateAddImpl(RecyclerView::ViewHolder& holder) {
    View* view = holder.itemView;
    ViewPropertyAnimator* animation;// = view.animate();
    mAddAnimations.push_back(&holder);//add(holder);
    Animator::AnimatorListener al;
    al.onAnimationStart=[this,&holder](Animator&animator,bool isReverse){
        dispatchAddStarting(holder);
    };
    al.onAnimationCancel=[&holder](Animator&){
        holder.itemView->setAlpha(1); 
    };
    al.onAnimationEnd=[this,&holder,animation](Animator& animator,bool isReverse){
        animation->setListener({});
        dispatchAddFinished(holder);
        //mAddAnimations.remove(holder);
        dispatchFinishedWhenDone();
    };
    animation->alpha(1).setDuration(getAddDuration()).setListener(al).start();
}

bool DefaultItemAnimator::animateMove(RecyclerView::ViewHolder& holder, int fromX, int fromY, int toX, int toY) {
    View* view = holder.itemView;
    fromX += (int) holder.itemView->getTranslationX();
    fromY += (int) holder.itemView->getTranslationY();
    resetAnimation(holder);
    int deltaX = toX - fromX;
    int deltaY = toY - fromY;
    if (deltaX == 0 && deltaY == 0) {
        dispatchMoveFinished(holder);
        return false;
    }
    if (deltaX != 0) {
        view->setTranslationX(-deltaX);
    }
    if (deltaY != 0) {
        view->setTranslationY(-deltaY);
    }
    mPendingMoves.push_back(new MoveInfo(holder, fromX, fromY, toX, toY));
    return true;
}

void DefaultItemAnimator::animateMoveImpl(RecyclerView::ViewHolder& holder, int fromX, int fromY, int toX, int toY) {
    View* view = holder.itemView;
    int deltaX = toX - fromX;
    int deltaY = toY - fromY;
    if (deltaX != 0) {
        //view->animate()->translationX(0);
    }
    if (deltaY != 0) {
        //view->animate()->translationY(0);
    }
    // TODO: make EndActions end listeners instead, since end actions aren't called when
    // vpas are canceled (and can't end them. why?)
    // need listener functionality in VPACompat for this. Ick.
    ViewPropertyAnimator* animation;// = view->animate();
    mMoveAnimations.push_back(&holder);//add(holder);
    Animator::AnimatorListener al;

    al.onAnimationStart=[this,&holder](Animator& animator,bool isReverse) {
        dispatchMoveStarting(holder);
    };
    al.onAnimationCancel=[this,view,deltaX,deltaY](Animator& animator) {
        if (deltaX != 0) {
            view->setTranslationX(0);
        }
        if (deltaY != 0) {
            view->setTranslationY(0);
        }
    };
    al.onAnimationEnd=[this,animation,&holder](Animator& animator,bool isReverse) {
        animation->setListener({});
        dispatchMoveFinished(holder);
        //mMoveAnimations.remove(holder);
        dispatchFinishedWhenDone();
    };
    animation->setDuration(getMoveDuration()).setListener(al).start();
}

bool DefaultItemAnimator::animateChange(RecyclerView::ViewHolder& oldHolder, RecyclerView::ViewHolder& newHolder,
        int fromX, int fromY, int toX, int toY) {
    if (&oldHolder == &newHolder) {
        // Don't know how to run change animations when the same view holder is re-used.
        // run a move animation to handle position changes.
        return animateMove(oldHolder, fromX, fromY, toX, toY);
    }
    float prevTranslationX = oldHolder.itemView->getTranslationX();
    float prevTranslationY = oldHolder.itemView->getTranslationY();
    float prevAlpha = oldHolder.itemView->getAlpha();
    resetAnimation(oldHolder);
    int deltaX = (int) (toX - fromX - prevTranslationX);
    int deltaY = (int) (toY - fromY - prevTranslationY);
    // recover prev translation state after ending animation
    oldHolder.itemView->setTranslationX(prevTranslationX);
    oldHolder.itemView->setTranslationY(prevTranslationY);
    oldHolder.itemView->setAlpha(prevAlpha);
    if (1/*newHolder != nullptr*/) {
        // carry over translation values
        resetAnimation(newHolder);
        newHolder.itemView->setTranslationX(-deltaX);
        newHolder.itemView->setTranslationY(-deltaY);
        newHolder.itemView->setAlpha(0);
    }
    mPendingChanges.push_back(new ChangeInfo(oldHolder, newHolder, fromX, fromY, toX, toY));
    return true;
}

void DefaultItemAnimator::animateChangeImpl(ChangeInfo& changeInfo) {
    RecyclerView::ViewHolder* holder = changeInfo.oldHolder;
    View* view = holder == nullptr ? nullptr : holder->itemView;
    RecyclerView::ViewHolder* newHolder = changeInfo.newHolder;
    View* newView = newHolder != nullptr ? newHolder->itemView : nullptr;
    if (view != nullptr) {
        ViewPropertyAnimator* oldViewAnim;// = view->animate().setDuration(getChangeDuration());
        mChangeAnimations.push_back(changeInfo.oldHolder);
        oldViewAnim->translationX(changeInfo.toX - changeInfo.fromX);
        oldViewAnim->translationY(changeInfo.toY - changeInfo.fromY);
 
        Animator::AnimatorListener al;
        al.onAnimationStart=[this,&changeInfo](Animator& animator,bool isReverse) {
            dispatchChangeStarting(*changeInfo.oldHolder, true);
        };
        al.onAnimationEnd=[this,view,&changeInfo,oldViewAnim](Animator& animator,bool isReverse) {
            oldViewAnim->setListener({});
            view->setAlpha(1);
            view->setTranslationX(0);
            view->setTranslationY(0);
            dispatchChangeFinished(*changeInfo.oldHolder, true);
            auto it =std::find(mChangeAnimations.begin(),mChangeAnimations.end(),changeInfo.oldHolder);
            mChangeAnimations.erase(it);//remove(*changeInfo.oldHolder);
            dispatchFinishedWhenDone();
        };
        oldViewAnim->alpha(0).setListener(al).start();
    }
    if (newView != nullptr) {
        ViewPropertyAnimator* newViewAnimation;// = newView->animate();
        mChangeAnimations.push_back(changeInfo.newHolder);
        Animator::AnimatorListener al;
        al.onAnimationStart=[this,&changeInfo](Animator& animator,bool isReverse) {
            dispatchChangeStarting(*changeInfo.newHolder, false);
        };
        al.onAnimationEnd=[this,newView,newViewAnimation,&changeInfo](Animator& animator,bool isReverse) {
            newViewAnimation->setListener({});
            newView->setAlpha(1);
            newView->setTranslationX(0);
            newView->setTranslationY(0);
            dispatchChangeFinished(*changeInfo.newHolder, false);
            auto it =std::find(mChangeAnimations.begin(),mChangeAnimations.end(),changeInfo.newHolder);
            mChangeAnimations.erase(it);//remove(changeInfo.newHolder);
            dispatchFinishedWhenDone();
        };
        newViewAnimation->translationX(0).translationY(0).setDuration(getChangeDuration())
                .alpha(1).setListener(al).start();
    }
}

void DefaultItemAnimator::endChangeAnimation(std::vector<ChangeInfo*>& infoList, RecyclerView::ViewHolder& item) {
    for (int i = infoList.size() - 1; i >= 0; i--) {
        ChangeInfo* changeInfo = infoList.at(i);
        if (endChangeAnimationIfNecessary(*changeInfo, item)) {
            if (changeInfo->oldHolder == nullptr && changeInfo->newHolder == nullptr) {
                infoList.erase(infoList.begin()+i);//remove(changeInfo);
            }
        }
    }
}

void DefaultItemAnimator::endChangeAnimationIfNecessary(ChangeInfo& changeInfo) {
    if (changeInfo.oldHolder != nullptr) {
        endChangeAnimationIfNecessary(changeInfo, *changeInfo.oldHolder);
    }
    if (changeInfo.newHolder != nullptr) {
        endChangeAnimationIfNecessary(changeInfo, *changeInfo.newHolder);
    }
}

bool DefaultItemAnimator::endChangeAnimationIfNecessary(ChangeInfo& changeInfo, RecyclerView::ViewHolder& item) {
    bool oldItem = false;
    if (changeInfo.newHolder == &item) {
        changeInfo.newHolder = nullptr;
    } else if (changeInfo.oldHolder == &item) {
        changeInfo.oldHolder = nullptr;
        oldItem = true;
    } else {
        return false;
    }
    item.itemView->setAlpha(1);
    item.itemView->setTranslationX(0);
    item.itemView->setTranslationY(0);
    dispatchChangeFinished(item, oldItem);
    return true;
}

void DefaultItemAnimator::endAnimation(RecyclerView::ViewHolder& item) {
    View* view = item.itemView;
    // this will trigger end callback which should set properties to their target values.
    //view->animate().cancel();
    // TODO if some other animations are chained to end, how do we cancel them as well?
    for (int i = mPendingMoves.size() - 1; i >= 0; i--) {
        MoveInfo* moveInfo = mPendingMoves.at(i);
        if (moveInfo->holder == &item) {
            view->setTranslationY(0);
            view->setTranslationX(0);
            dispatchMoveFinished(item);
            mPendingMoves.erase(mPendingMoves.begin()+i);//remove(i);
        }
    }
    endChangeAnimation(mPendingChanges, item);
	auto it = std::find(mPendingRemovals.begin(),mPendingRemovals.end(),&item);
    if (it!=mPendingRemovals.end()){//if(mPendingRemovals.remove(item)){
        view->setAlpha(1);
		mPendingRemovals.erase(it);
        dispatchRemoveFinished(item);
    }
	it = std::find(mPendingAdditions.begin(),mPendingAdditions.end(),&item);
    if (it!=mPendingAdditions.end()){//mPendingAdditions.remove(item)) {
        view->setAlpha(1);
		mPendingAdditions.erase(it);
        dispatchAddFinished(item);
    }

    for (int i = mChangesList.size() - 1; i >= 0; i--) {
        std::vector<ChangeInfo*> changes = mChangesList.at(i);
        endChangeAnimation(changes, item);
        if (changes.empty()) {
            mChangesList.erase(mChangesList.begin()+i);//.remove(i);
        }
    }
    for (int i = mMovesList.size() - 1; i >= 0; i--) {
        std::vector<MoveInfo*> moves = mMovesList.at(i);
        for (int j = moves.size() - 1; j >= 0; j--) {
            MoveInfo* moveInfo = moves.at(j);
            if (moveInfo->holder == &item) {
                view->setTranslationY(0);
                view->setTranslationX(0);
                dispatchMoveFinished(item);
                moves.erase(moves.begin()+j);//.remove(j);
                if (moves.empty()) {
                    mMovesList.erase(mMovesList.begin()+i);//.remove(i);
                }
                break;
            }
        }
    }
    for (int i = mAdditionsList.size() - 1; i >= 0; i--) {
        std::vector<RecyclerView::ViewHolder*> additions = mAdditionsList.at(i);
		it = std::find(additions.begin(),additions.end(),&item);
        if (it!=additions.end()){//additions.remove(item)) {
            view->setAlpha(1);
			additions.erase(it);
            dispatchAddFinished(item);
            if (additions.empty()) {
                mAdditionsList.erase(mAdditionsList.begin()+i);//.remove(i);
            }
        }
    }

    // animations should be ended by the cancel above.
    //noinspection PointlessBooleanExpression,ConstantConditions
    /*if (mRemoveAnimations.remove(item) && DEBUG) {
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
    }*/
    dispatchFinishedWhenDone();
}

void DefaultItemAnimator::resetAnimation(RecyclerView::ViewHolder& holder) {
    if (sDefaultInterpolator == nullptr) {
        //sDefaultInterpolator = new ValueAnimator()->getInterpolator();
    }
    //holder.itemView->animate().setInterpolator(sDefaultInterpolator);
    endAnimation(holder);
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
    int count = mPendingMoves.size();
    for (int i = count - 1; i >= 0; i--) {
        MoveInfo* item = mPendingMoves.at(i);
        View* view = item->holder->itemView;
        view->setTranslationY(0);
        view->setTranslationX(0);
        dispatchMoveFinished(*item->holder);
        mPendingMoves.erase(mPendingMoves.begin()+i);//.remove(i);
    }
    count = mPendingRemovals.size();
    for (int i = count - 1; i >= 0; i--) {
        RecyclerView::ViewHolder* item = mPendingRemovals.at(i);
        dispatchRemoveFinished(*item);
        mPendingRemovals.erase(mPendingRemovals.begin()+i);//.remove(i);
    }
    count = mPendingAdditions.size();
    for (int i = count - 1; i >= 0; i--) {
        RecyclerView::ViewHolder* item = mPendingAdditions.at(i);
        item->itemView->setAlpha(1);
        dispatchAddFinished(*item);
        mPendingAdditions.erase(mPendingAdditions.begin()+i);//.remove(i);
    }
    count = mPendingChanges.size();
    for (int i = count - 1; i >= 0; i--) {
        endChangeAnimationIfNecessary(*mPendingChanges.at(i));
    }
    mPendingChanges.clear();
    if (!isRunning()) {
        return;
    }

    int listCount = mMovesList.size();
    for (int i = listCount - 1; i >= 0; i--) {
        std::vector<MoveInfo*>& moves = mMovesList.at(i);
        count = moves.size();
        for (int j = count - 1; j >= 0; j--) {
            MoveInfo* moveInfo = moves.at(j);
            RecyclerView::ViewHolder* item = moveInfo->holder;
            View* view = item->itemView;
            view->setTranslationY(0);
            view->setTranslationX(0);
            dispatchMoveFinished(*moveInfo->holder);
            moves.erase(moves.begin()+i);//.remove(j);
            if (moves.empty()) {
                mMovesList.erase(mMovesList.begin()+i);//remove(moves);
            }
        }
    }
    listCount = mAdditionsList.size();
    for (int i = listCount - 1; i >= 0; i--) {
        std::vector<RecyclerView::ViewHolder*>& additions = mAdditionsList.at(i);
        count = additions.size();
        for (int j = count - 1; j >= 0; j--) {
            RecyclerView::ViewHolder* item = additions.at(j);
            View* view = item->itemView;
            view->setAlpha(1);
            dispatchAddFinished(*item);
            additions.erase(additions.begin()+i);//.remove(j);
            if (additions.empty()) {
                mAdditionsList.erase(mAdditionsList.begin()+i);//.remove(additions);
            }
        }
    }
    listCount = mChangesList.size();
    for (int i = listCount - 1; i >= 0; i--) {
        std::vector<ChangeInfo*>& changes = mChangesList.at(i);
        count = changes.size();
        for (int j = count - 1; j >= 0; j--) {
            endChangeAnimationIfNecessary(*changes.at(j));
            if (changes.empty()) {
                mChangesList.erase(mChangesList.begin()+i);//.remove(changes);
            }
        }
    }

    cancelAll(mRemoveAnimations);
    cancelAll(mMoveAnimations);
    cancelAll(mAddAnimations);
    cancelAll(mChangeAnimations);

    dispatchAnimationsFinished();
}

void DefaultItemAnimator::cancelAll(std::vector<RecyclerView::ViewHolder*>& viewHolders) {
    for (int i = viewHolders.size() - 1; i >= 0; i--) {
        //viewHolders.at(i)->itemView->animate().cancel();
    }
}

bool DefaultItemAnimator::canReuseUpdatedViewHolder(RecyclerView::ViewHolder& viewHolder, std::vector<Object*>& payloads) {
    return !payloads.empty() || SimpleItemAnimator::canReuseUpdatedViewHolder(viewHolder);//, payloads);
}
}/*endof namespace*/
