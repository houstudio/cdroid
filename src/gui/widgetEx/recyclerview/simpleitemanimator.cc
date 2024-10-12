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
        ItemHolderInfo& preInfo, ItemHolderInfo& postInfo) {
    if (preInfo.left != postInfo.left || preInfo.top != postInfo.top) {
        LOGD_IF(_Debug,"%p PERSISTENT: %p with view %p",this,&viewHolder,viewHolder.itemView);
        return animateMove(viewHolder, preInfo.left, preInfo.top, postInfo.left, postInfo.top);
    }
    dispatchMoveFinished(viewHolder);
    return false;
}

bool SimpleItemAnimator::animateChange(RecyclerView::ViewHolder& oldHolder, RecyclerView::ViewHolder& newHolder,
        ItemHolderInfo& preInfo, ItemHolderInfo& postInfo) {
    LOGD_IF(_Debug,"%p CHANGED: %p with view %p",this,&oldHolder,oldHolder.itemView);
    const int fromLeft = preInfo.left;
    const int fromTop = preInfo.top;
    int toLeft, toTop;
    if (newHolder.shouldIgnore()) {
        toLeft = preInfo.left;
        toTop = preInfo.top;
    } else {
        toLeft = postInfo.left;
        toTop = postInfo.top;
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
