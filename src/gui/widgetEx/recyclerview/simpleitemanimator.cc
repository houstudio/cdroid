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
#include <widgetEx/recyclerview/simpleitemanimator.h>
namespace cdroid{
bool SimpleItemAnimator::getSupportsChangeAnimations() {
    return mSupportsChangeAnimations;
}

void SimpleItemAnimator::setSupportsChangeAnimations(bool supportsChangeAnimations) {
    mSupportsChangeAnimations = supportsChangeAnimations;
}

bool  SimpleItemAnimator::canReuseUpdatedViewHolder(RecyclerView::ViewHolder& viewHolder) {
    return !mSupportsChangeAnimations || viewHolder.isInvalid();
}

bool SimpleItemAnimator::animateDisappearance(RecyclerView::ViewHolder& viewHolder,
        ItemHolderInfo& preLayoutInfo, ItemHolderInfo* postLayoutInfo) {
    int oldLeft = preLayoutInfo.left;
    int oldTop = preLayoutInfo.top;
    View* disappearingItemView = viewHolder.itemView;
    int newLeft = postLayoutInfo == nullptr ? disappearingItemView->getLeft() : postLayoutInfo->left;
    int newTop = postLayoutInfo == nullptr ? disappearingItemView->getTop() : postLayoutInfo->top;
    if (!viewHolder.isRemoved() && (oldLeft != newLeft || oldTop != newTop)) {
        disappearingItemView->layout(newLeft, newTop,
                newLeft + disappearingItemView->getWidth(),
                newTop + disappearingItemView->getHeight());
        LOGD_IF(_Debug,"%p DISAPPEARING: %p with view %p",this,&viewHolder,disappearingItemView);
        return animateMove(viewHolder, oldLeft, oldTop, newLeft, newTop);
    } else {
        LOGD_IF(_Debug,"%p REMOVED:  %p with view %p",this,&viewHolder,disappearingItemView);
        return animateRemove(viewHolder);
    }
}

bool SimpleItemAnimator::animateAppearance(RecyclerView::ViewHolder& viewHolder,
        ItemHolderInfo* preLayoutInfo, ItemHolderInfo& postLayoutInfo) {
    if (preLayoutInfo != nullptr && (preLayoutInfo->left != postLayoutInfo.left
            || preLayoutInfo->top != postLayoutInfo.top)) {
        // slide items in if before/after locations differ
        LOGD_IF(_Debug,"%p APPEARING: %p with view %p" ,this,&viewHolder,viewHolder.itemView);
        return animateMove(viewHolder, preLayoutInfo->left, preLayoutInfo->top,
                postLayoutInfo.left, postLayoutInfo.top);
    } else {
        LOGD_IF(_Debug,"ADDED: %p with view %p",&viewHolder,&viewHolder);
        return animateAdd(viewHolder);
    }
}

bool SimpleItemAnimator::animatePersistence(RecyclerView::ViewHolder& viewHolder,
        ItemHolderInfo& preLayoutInfo, ItemHolderInfo& postLayoutInfo) {
    if (preLayoutInfo.left != postLayoutInfo.left || preLayoutInfo.top != postLayoutInfo.top) {
        LOGD_IF(_Debug,"%p PERSISTENT: %p with view %p",this,&viewHolder,viewHolder.itemView);
        return animateMove(viewHolder, preLayoutInfo.left, preLayoutInfo.top, postLayoutInfo.left, postLayoutInfo.top);
    }
    dispatchMoveFinished(viewHolder);
    return false;
}

bool SimpleItemAnimator::animateChange(RecyclerView::ViewHolder& oldHolder, RecyclerView::ViewHolder& newHolder,
        ItemHolderInfo& preLayoutInfo, ItemHolderInfo& postLayoutInfo) {
    LOGD_IF(_Debug,"%p CHANGED: %p with view %p",this,&oldHolder,oldHolder.itemView);
    const int fromLeft = preLayoutInfo.left;
    const int fromTop = preLayoutInfo.top;
    int toLeft, toTop;
    if (newHolder.shouldIgnore()) {
        toLeft = preLayoutInfo.left;
        toTop = preLayoutInfo.top;
    } else {
        toLeft = postLayoutInfo.left;
        toTop = postLayoutInfo.top;
    }
    return animateChange(oldHolder, newHolder, fromLeft, fromTop, toLeft, toTop);
}

void SimpleItemAnimator::dispatchRemoveFinished(RecyclerView::ViewHolder& item) {
    onRemoveFinished(item);
    dispatchAnimationFinished(item);
}

void SimpleItemAnimator::dispatchMoveFinished(RecyclerView::ViewHolder& item) {
    onMoveFinished(item);
    dispatchAnimationFinished(item);
}

void SimpleItemAnimator::dispatchAddFinished(RecyclerView::ViewHolder& item) {
    onAddFinished(item);
    dispatchAnimationFinished(item);
}

void SimpleItemAnimator::dispatchChangeFinished(RecyclerView::ViewHolder& item, bool oldItem) {
    onChangeFinished(item, oldItem);
    dispatchAnimationFinished(item);
}

void SimpleItemAnimator::dispatchRemoveStarting(RecyclerView::ViewHolder& item) {
    onRemoveStarting(item);
}

void SimpleItemAnimator::dispatchMoveStarting(RecyclerView::ViewHolder& item) {
    onMoveStarting(item);
}

void SimpleItemAnimator::dispatchAddStarting(RecyclerView::ViewHolder& item) {
    onAddStarting(item);
}

void SimpleItemAnimator::dispatchChangeStarting(RecyclerView::ViewHolder& item, bool oldItem) {
    onChangeStarting(item, oldItem);
}

void SimpleItemAnimator::onRemoveStarting(RecyclerView::ViewHolder& item) {
}

void SimpleItemAnimator::onRemoveFinished(RecyclerView::ViewHolder& item) {
}

void SimpleItemAnimator::onAddStarting(RecyclerView::ViewHolder& item) {
}

void SimpleItemAnimator::onAddFinished(RecyclerView::ViewHolder& item) {
}

void SimpleItemAnimator::onMoveStarting(RecyclerView::ViewHolder& item) {
}

void SimpleItemAnimator::onMoveFinished(RecyclerView::ViewHolder& item) {
}

void SimpleItemAnimator::onChangeStarting(RecyclerView::ViewHolder& item, bool oldItem) {
}

void SimpleItemAnimator::onChangeFinished(RecyclerView::ViewHolder& item, bool oldItem) {
}
}/*endof namespace*/
