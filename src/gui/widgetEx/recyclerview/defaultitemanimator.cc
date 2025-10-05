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
#include <widgetEx/recyclerview/defaultitemanimator.h>
namespace cdroid{

const TimeInterpolator* DefaultItemAnimator::sDefaultInterpolator = nullptr;

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
    this->useCount = 0;
}

DefaultItemAnimator::ChangeInfo::ChangeInfo(RecyclerView::ViewHolder& oldHolder, RecyclerView::ViewHolder& newHolder,
        int fromX, int fromY, int toX, int toY):ChangeInfo(oldHolder, newHolder){
    this->fromX = fromX;
    this->fromY = fromY;
    this->toX = toX;
    this->toY = toY;
    this->useCount = 0;
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
        std::vector<MoveInfo*>*moves = new std::vector<MoveInfo*>;
        *moves = mPendingMoves;
        mMovesList.push_back(moves);
        mPendingMoves.clear();
        Runnable mover;
        mover=[this,moves]() {
            auto it = std::find(mMovesList.begin(),mMovesList.end(),moves);
            if(it!=mMovesList.end()){
                for (MoveInfo* moveInfo : *moves) {
                    animateMoveImpl(*moveInfo->holder, moveInfo->fromX, moveInfo->fromY,
                        moveInfo->toX, moveInfo->toY);
                    delete moveInfo;
                }
                moves->clear();
                delete moves;
                mMovesList.erase(it);//remove(moves);
            }
        };
        if (removalsPending) {
            View* view = moves->at(0)->holder->itemView;
            view->postOnAnimationDelayed(mover, getRemoveDuration());
        } else {
            mover();//.run();
        }
    }
    // Next, change stuff, to run in parallel with move animations
    if (changesPending) {
        std::vector<ChangeInfo*>* changes = new std::vector<ChangeInfo*>;
        *changes = mPendingChanges;
        mChangesList.push_back(changes);
        mPendingChanges.clear();
        Runnable changer;
        changer= [this,changes]() {
            auto it = std::find(mChangesList.begin(),mChangesList.end(),changes);
            if(it!=mChangesList.end()){
                for (ChangeInfo* change : *changes) {
                    animateChangeImpl(*change);
                    //delete change;
                }
                changes->clear();
                delete changes;
                mChangesList.erase(it);//remove(changes);
            }
        };
        if (removalsPending) {
            RecyclerView::ViewHolder* holder = changes->at(0)->oldHolder;
            holder->itemView->postOnAnimationDelayed(changer, getRemoveDuration());
        } else {
            changer();//.run();
        }
    }
    // Next, add stuff
    if (additionsPending) {
       std::vector<RecyclerView::ViewHolder*>* additions = new std::vector<RecyclerView::ViewHolder*>();
       *additions = mPendingAdditions;
       mAdditionsList.push_back(additions);
       mPendingAdditions.clear();
       Runnable adder;
       adder = [this,additions]() {
            auto it = std::find(mAdditionsList.begin(),mAdditionsList.end(),additions);
            if(it!=mAdditionsList.end()){
                for (RecyclerView::ViewHolder* holder : *additions) {
                    animateAddImpl(*holder);
                }
                additions->clear();
                delete additions;
                mAdditionsList.erase(it);//remove(additions);
            }
        };
        if (removalsPending || movesPending || changesPending) {
            long removeDuration = removalsPending ? getRemoveDuration() : 0;
            long moveDuration = movesPending ? getMoveDuration() : 0;
            long changeDuration = changesPending ? getChangeDuration() : 0;
            long totalDelay = removeDuration + std::max(moveDuration, changeDuration);
            View* view = additions->at(0)->itemView;
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

void DefaultItemAnimator::onRemoveAnimationStart(RecyclerView::ViewHolder*holder,Animator& animator,bool isReverse){
    dispatchRemoveStarting(*holder);
}

void DefaultItemAnimator::onRemoveAnimationEnd(RecyclerView::ViewHolder*holder,Animator& animator,bool isReverse){
    
    ViewPropertyAnimator& animation = holder->itemView->animate();
    animation.setListener({});
    holder->itemView->setAlpha(1);
    dispatchRemoveFinished(*holder);
    auto it = std::find(mRemoveAnimations.begin(),mRemoveAnimations.end(),holder);
    if(it!=mRemoveAnimations.end())mRemoveAnimations.erase(it);//mRemoveAnimations.remove(holder);
    else LOGE("not found.....");
    dispatchFinishedWhenDone();
}

void DefaultItemAnimator::animateRemoveImpl(RecyclerView::ViewHolder& holder) {
 
    View* view = holder.itemView;
    ViewPropertyAnimator& animation = view->animate();
    mRemoveAnimations.push_back(&holder);//add(holder);
    //AnimatorListenerAdapter
    Animator::AnimatorListener al;
    al.onAnimationStart=std::bind(&DefaultItemAnimator::onRemoveAnimationStart,this,&holder,std::placeholders::_1,std::placeholders::_2);
    al.onAnimationEnd = std::bind(&DefaultItemAnimator::onRemoveAnimationEnd,this,&holder,std::placeholders::_1,std::placeholders::_2);
    animation.setDuration(getRemoveDuration()).alpha(0).setListener(al).start();
}

bool DefaultItemAnimator::animateAdd(RecyclerView::ViewHolder& holder) {
    resetAnimation(holder);
    holder.itemView->setAlpha(0);
    mPendingAdditions.push_back(&holder);
    return true;
}

void DefaultItemAnimator::onAddAnimationStart(RecyclerView::ViewHolder*holder,Animator& animator,bool isReverse){
    dispatchAddStarting(*holder);
}

void DefaultItemAnimator::onAddAnimationCancel(RecyclerView::ViewHolder*holder,Animator& animator,bool isReverse){
    holder->itemView->setAlpha(1);
}

void DefaultItemAnimator::onAddAnimationEnd(RecyclerView::ViewHolder*holder,Animator& animator,bool isReverse){
    View* view = holder->itemView;
    ViewPropertyAnimator& animation = view->animate();
    animation.setListener({});
    dispatchAddFinished(*holder);
    auto it = std::find(mAddAnimations.begin(), mAddAnimations.end(),holder);
    mAddAnimations.erase(it);//mAddAnimations.remove(holder);
    dispatchFinishedWhenDone();
}

void DefaultItemAnimator::animateAddImpl(RecyclerView::ViewHolder& holder) {
    View* view = holder.itemView;
    ViewPropertyAnimator& animation = view->animate();
    mAddAnimations.push_back(&holder);//add(holder);
    Animator::AnimatorListener al;
    al.onAnimationStart = std::bind(&DefaultItemAnimator::onAddAnimationStart,this,&holder,std::placeholders::_1,std::placeholders::_2);
    al.onAnimationEnd = std::bind(&DefaultItemAnimator::onAddAnimationEnd,this,&holder,std::placeholders::_1,std::placeholders::_2);
    animation.alpha(1).setDuration(getAddDuration()).setListener(al).start();
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

void DefaultItemAnimator::onMoveAnimationStart(RecyclerView::ViewHolder*holder,Animator& animator,bool isReverse){
    dispatchMoveStarting(*holder);
}

void DefaultItemAnimator::onMoveAnimationCancel(int deltaX,int deltaY,RecyclerView::ViewHolder*holder,Animator& animator){
    if (deltaX != 0) {
        holder->itemView->setTranslationX(0);
    }
    if (deltaY != 0) {
        holder->itemView->setTranslationY(0);
    }
}

void DefaultItemAnimator::onMoveAnimationEnd(RecyclerView::ViewHolder*holder,Animator& animator,bool isReverse){
    ViewPropertyAnimator& animation = holder->itemView->animate();
    animation.setListener({});
    dispatchMoveFinished(*holder);
    auto it = std::find(mMoveAnimations.begin(),mMoveAnimations.end(),holder);
    mMoveAnimations.erase(it);//mMoveAnimations.remove(holder);
    dispatchFinishedWhenDone();
}

void DefaultItemAnimator::animateMoveImpl(RecyclerView::ViewHolder& holder, int fromX, int fromY, int toX, int toY) {
    View* view = holder.itemView;
    const int deltaX = toX - fromX;
    const int deltaY = toY - fromY;
    if (deltaX != 0) {
        view->animate().translationX(0);
    }
    if (deltaY != 0) {
        view->animate().translationY(0);
    }
    // TODO: make EndActions end listeners instead, since end actions aren't called when
    // vpas are canceled (and can't end them. why?)
    // need listener functionality in VPACompat for this. Ick.
    ViewPropertyAnimator& animation = view->animate();
    mMoveAnimations.push_back(&holder);//add(holder);
    Animator::AnimatorListener al;

    al.onAnimationStart = std::bind(&DefaultItemAnimator::onMoveAnimationStart,this,&holder,std::placeholders::_1,std::placeholders::_2);
    al.onAnimationCancel = std::bind(&DefaultItemAnimator::onMoveAnimationCancel,this,deltaX,deltaY,&holder,std::placeholders::_1);
    al.onAnimationEnd = std::bind(&DefaultItemAnimator::onMoveAnimationEnd,this,&holder,std::placeholders::_1,std::placeholders::_2);
    animation.setDuration(getMoveDuration()).setListener(al).start();
}

bool DefaultItemAnimator::animateChange(RecyclerView::ViewHolder& oldHolder, RecyclerView::ViewHolder& newHolder,
        int fromLeft, int fromTop, int toLeft, int toTop) {
    if (&oldHolder == &newHolder) {
        // Don't know how to run change animations when the same view holder is re-used.
        // run a move animation to handle position changes.
        return animateMove(oldHolder, fromLeft, fromTop, toLeft, toTop);
    }
    float prevTranslationX = oldHolder.itemView->getTranslationX();
    float prevTranslationY = oldHolder.itemView->getTranslationY();
    float prevAlpha = oldHolder.itemView->getAlpha();
    resetAnimation(oldHolder);
    int deltaX = (int) (toLeft - fromLeft - prevTranslationX);
    int deltaY = (int) (toTop - fromTop - prevTranslationY);
    // recover prev translation state after ending animation
    oldHolder.itemView->setTranslationX(prevTranslationX);
    oldHolder.itemView->setTranslationY(prevTranslationY);
    oldHolder.itemView->setAlpha(prevAlpha);
#ifndef __clang__
    if (&newHolder != nullptr) 
#endif
    {
        // carry over translation values
        resetAnimation(newHolder);
        newHolder.itemView->setTranslationX(-deltaX);
        newHolder.itemView->setTranslationY(-deltaY);
        newHolder.itemView->setAlpha(0);
    }
    mPendingChanges.push_back(new ChangeInfo(oldHolder, newHolder, fromLeft, fromTop, toLeft, toTop));
    return true;
}

void DefaultItemAnimator::onChangeAnimationStart(bool old,ChangeInfo*changeInfo,Animator& animator,bool isReverse){
    RecyclerView::ViewHolder* holder = old?changeInfo->oldHolder:changeInfo->newHolder;
    View*view = holder == nullptr ? nullptr : holder->itemView;
    if(view&&holder)dispatchChangeStarting(*holder, true);
}

void DefaultItemAnimator::onChangeAnimationEnd(bool old,ChangeInfo*changeInfo,Animator& animator,bool isReverse){
    RecyclerView::ViewHolder* holder = old?changeInfo->oldHolder:changeInfo->newHolder;
    View*view = (holder == nullptr) ? nullptr : holder->itemView;
    if(view&&holder){
        ViewPropertyAnimator& animator = view->animate();
	    animator.setListener({});
        view->setAlpha(1);
        view->setTranslationX(0);
        view->setTranslationY(0);
        dispatchChangeFinished(*holder, old);
        auto it = std::find(mChangeAnimations.begin(),mChangeAnimations.end(),holder);
        if(it!=mChangeAnimations.end())
            mChangeAnimations.erase(it);
        dispatchFinishedWhenDone();
        //LOGD("changeInfo %p useCount=%d",changeInfo,changeInfo->useCount);
        if(--changeInfo->useCount==0){
            delete changeInfo;
        }
    }
}

void DefaultItemAnimator::animateChangeImpl(ChangeInfo& changeInfo) {
    RecyclerView::ViewHolder* holder = changeInfo.oldHolder;
    View* view = holder == nullptr ? nullptr : holder->itemView;
    
    RecyclerView::ViewHolder* newHolder = changeInfo.newHolder;
    View* newView = newHolder != nullptr ? newHolder->itemView : nullptr;
    if (view != nullptr) {
        ViewPropertyAnimator& oldViewAnim = view->animate().setDuration(getChangeDuration());
        mChangeAnimations.push_back(changeInfo.oldHolder);
        oldViewAnim.translationX(changeInfo.toX - changeInfo.fromX);
        oldViewAnim.translationY(changeInfo.toY - changeInfo.fromY);
 
        Animator::AnimatorListener al;
        changeInfo.useCount++;
        al.onAnimationStart = std::bind(&DefaultItemAnimator::onChangeAnimationStart,this,true,&changeInfo,std::placeholders::_1,std::placeholders::_2);
        al.onAnimationEnd = std::bind(&DefaultItemAnimator::onChangeAnimationEnd,this,true,&changeInfo,std::placeholders::_1,std::placeholders::_2);
        oldViewAnim.alpha(0).setListener(al).start();
    }
    if (newView != nullptr) {
        ViewPropertyAnimator& newViewAnimation = newView->animate();
        mChangeAnimations.push_back(changeInfo.newHolder);
        Animator::AnimatorListener al;
        changeInfo.useCount++;
        al.onAnimationStart = std::bind(&DefaultItemAnimator::onChangeAnimationStart,this,false,&changeInfo,std::placeholders::_1,std::placeholders::_2);
        al.onAnimationEnd = std::bind(&DefaultItemAnimator::onChangeAnimationEnd,this,false,&changeInfo,std::placeholders::_1,std::placeholders::_2);
        newViewAnimation.translationX(0).translationY(0).setDuration(getChangeDuration())
                .alpha(1).setListener(al).start();
    }
}

void DefaultItemAnimator::endChangeAnimation(std::vector<ChangeInfo*>& infoList, RecyclerView::ViewHolder& item) {
    for (int i = int(infoList.size() - 1); i >= 0; i--) {
        ChangeInfo* changeInfo = infoList.at(i);
        if (endChangeAnimationIfNecessary(*changeInfo, item)) {
            if (changeInfo->oldHolder == nullptr && changeInfo->newHolder == nullptr) {
                LOGD("changeInfo %p useCount=%d",changeInfo,changeInfo->useCount);
                if(--changeInfo->useCount<=0)
                    delete changeInfo;//TOBE TEST
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
    view->animate().cancel();
    // TODO if some other animations are chained to end, how do we cancel them as well?
    for (int i = int(mPendingMoves.size() - 1); i >= 0; i--) {
        MoveInfo* moveInfo = mPendingMoves.at(i);
        if (moveInfo->holder == &item) {
            view->setTranslationY(0);
            view->setTranslationX(0);
            dispatchMoveFinished(item);
            mPendingMoves.erase(mPendingMoves.begin()+i);//remove(i);
            delete moveInfo;
        }
    }
    endChangeAnimation(mPendingChanges, item);
    auto it = std::find(mPendingRemovals.begin(),mPendingRemovals.end(),&item);
    if (it!=mPendingRemovals.end()) {
        view->setAlpha(1);
        mPendingRemovals.erase(it);
        dispatchRemoveFinished(item);
    }
    it = std::find(mPendingAdditions.begin(),mPendingAdditions.end(),&item);
    if (it!=mPendingAdditions.end()) {
        view->setAlpha(1);
        mPendingAdditions.erase(it);
        dispatchAddFinished(item);
    }

    for (int i = int(mChangesList.size() - 1); i >= 0; i--) {
        std::vector<ChangeInfo*>* changes = mChangesList.at(i);
        endChangeAnimation(*changes, item);
        if (changes->empty()) {
            delete changes;
            mChangesList.erase(mChangesList.begin()+i);//.remove(i);
        }
    }
    for (int i = int(mMovesList.size() - 1); i >= 0; i--) {
        std::vector<MoveInfo*>* moves = mMovesList.at(i);
        for (int j = moves->size() - 1; j >= 0; j--) {
            MoveInfo* moveInfo = moves->at(j);
            if (moveInfo->holder == &item) {
                view->setTranslationY(0);
                view->setTranslationX(0);
                dispatchMoveFinished(item);
                delete moveInfo;//TOBE TEST
                moves->erase(moves->begin()+j);//.remove(j);
                if (moves->empty()) {
                    delete moves;
                    mMovesList.erase(mMovesList.begin()+i);//.remove(i);
                }
                break;
            }
        }
    }
    for (int i = int(mAdditionsList.size() - 1); i >= 0; i--) {
        std::vector<RecyclerView::ViewHolder*>* additions = mAdditionsList.at(i);
        it = std::find(additions->begin(),additions->end(),&item);
        if (it!=additions->end()) {
            view->setAlpha(1);
            additions->erase(it);
            dispatchAddFinished(item);
            if (additions->empty()) {
                delete additions;
                mAdditionsList.erase(mAdditionsList.begin()+i);//.remove(i);
            }
        }
    }

    // animations should be ended by the cancel above.
    //noinspection PointlessBooleanExpression,ConstantConditions
    auto itItem = std::find(mRemoveAnimations.begin(),mRemoveAnimations.end(),&item);
    if ( itItem!=mRemoveAnimations.end()) {
        mRemoveAnimations.erase(itItem);
        LOGE_IF(_Debug,"after animation is cancelled, item should not be in mRemoveAnimations list");
    }

    //noinspection PointlessBooleanExpression,ConstantConditions
    itItem = std::find(mAddAnimations.begin(),mAddAnimations.end(),&item);
    if ( itItem!=mAddAnimations.end() ) {
        mAddAnimations.erase(itItem);
        LOGE_IF(_Debug,"after animation is cancelled, item should not be in mAddAnimations list");
    }

    //noinspection PointlessBooleanExpression,ConstantConditions
    itItem = std::find(mChangeAnimations.begin(),mChangeAnimations.end(),&item);
    if ( itItem!=mChangeAnimations.end() ) {;
        mChangeAnimations.erase(itItem);
        LOGE_IF(_Debug,"after animation is cancelled, item should not be in mChangeAnimations list");
    }

    //noinspection PointlessBooleanExpression,ConstantConditions
    itItem = std::find(mMoveAnimations.begin(),mMoveAnimations.end(),&item);
    if ( itItem!=mMoveAnimations.end() ) {
        mMoveAnimations.erase(itItem);
        LOGE_IF(_Debug,"after animation is cancelled, item should not be in mMoveAnimations list");
    }
    dispatchFinishedWhenDone();
}

void DefaultItemAnimator::resetAnimation(RecyclerView::ViewHolder& holder) {
    if (sDefaultInterpolator == nullptr) {
        sDefaultInterpolator = AccelerateDecelerateInterpolator::Instance;
    }
    holder.itemView->animate().setInterpolator(sDefaultInterpolator);
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
    int count = (int)mPendingMoves.size();
    for (int i = count - 1; i >= 0; i--) {
        MoveInfo* item = mPendingMoves.at(i);
        View* view = item->holder->itemView;
        view->setTranslationY(0);
        view->setTranslationX(0);
        dispatchMoveFinished(*item->holder);
        mPendingMoves.erase(mPendingMoves.begin()+i);//.remove(i);
    }
    count = (int)mPendingRemovals.size();
    for (int i = count - 1; i >= 0; i--) {
        RecyclerView::ViewHolder* item = mPendingRemovals.at(i);
        dispatchRemoveFinished(*item);
        mPendingRemovals.erase(mPendingRemovals.begin()+i);//.remove(i);
    }
    count = (int)mPendingAdditions.size();
    for (int i = count - 1; i >= 0; i--) {
        RecyclerView::ViewHolder* item = mPendingAdditions.at(i);
        item->itemView->setAlpha(1);
        dispatchAddFinished(*item);
        mPendingAdditions.erase(mPendingAdditions.begin()+i);//.remove(i);
    }
    count = (int)mPendingChanges.size();
    for (int i = count - 1; i >= 0; i--) {
        endChangeAnimationIfNecessary(*mPendingChanges.at(i));
    }
    mPendingChanges.clear();
    if (!isRunning()) {
        return;
    }

    int listCount = (int)mMovesList.size();
    for (int i = listCount - 1; i >= 0; i--) {
        std::vector<MoveInfo*>* moves = mMovesList.at(i);
        count = (int)moves->size();
        for (int j = count - 1; j >= 0; j--) {
            MoveInfo* moveInfo = moves->at(j);
            RecyclerView::ViewHolder* item = moveInfo->holder;
            View* view = item->itemView;
            view->setTranslationY(0);
            view->setTranslationX(0);
            dispatchMoveFinished(*moveInfo->holder);
            moves->erase(moves->begin()+j);//.remove(j);
            if (moves->empty()) {
                delete moves;
                mMovesList.erase(mMovesList.begin()+i);//remove(moves);
            }
        }
    }
    listCount = (int)mAdditionsList.size();
    for (int i = listCount - 1; i >= 0; i--) {
        std::vector<RecyclerView::ViewHolder*>* additions = mAdditionsList.at(i);
        count = (int)additions->size();
        for (int j = count - 1; j >= 0; j--) {
            RecyclerView::ViewHolder* item = additions->at(j);
            View* view = item->itemView;
            view->setAlpha(1);
            dispatchAddFinished(*item);
            additions->erase(additions->begin()+j);//.remove(j);
            if (additions->empty()) {
                delete additions;
                mAdditionsList.erase(mAdditionsList.begin()+i);//.remove(additions);
            }
        }
    }
    listCount = (int)mChangesList.size();
    for (int i = listCount - 1; i >= 0; i--) {
        std::vector<ChangeInfo*>* changes = mChangesList.at(i);
        count = (int)changes->size();
        for (int j = count - 1; j >= 0; j--) {
            endChangeAnimationIfNecessary(*changes->at(j));
            if (changes->empty()) {
                delete changes;
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
    for (int i = int(viewHolders.size() - 1); i >= 0; i--) {
        viewHolders.at(i)->itemView->animate().cancel();
    }
}

bool DefaultItemAnimator::canReuseUpdatedViewHolder(RecyclerView::ViewHolder& viewHolder, std::vector<Object*>& payloads) {
    return !payloads.empty() || SimpleItemAnimator::canReuseUpdatedViewHolder(viewHolder);//, payloads);
}
}/*endof namespace*/
