#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include <widgetEx/recyclerview/orientationhelper.h>
#include <widgetEx/recyclerview/scrollbarhelper.h>
#include <widgetEx/recyclerview/linearsmoothscroller.h>

namespace cdroid{

LinearLayoutManager::LinearLayoutManager(Context* context)
   :LinearLayoutManager(context, RecyclerView::DEFAULT_ORIENTATION, false){
}

LinearLayoutManager::LinearLayoutManager(Context* context,int orientation,bool reverseLayout):RecyclerView::LayoutManager(){
    mPendingSavedState = nullptr;
    mLayoutState = nullptr;
    mOrientationHelper = nullptr;
    mLastStackFromEnd = false;
    mStackFromEnd = false;
    mShouldReverseLayout = false;
    mRecycleChildrenOnDetach = true;
    mInitialPrefetchItemCount = 2;
    mAnchorInfo = new AnchorInfo();
    mLayoutChunkResult = new LayoutChunkResult();
    setOrientation(orientation);
    setReverseLayout(reverseLayout);
}

LinearLayoutManager::LinearLayoutManager(Context* context, const AttributeSet& attrs)
     :LinearLayoutManager(context){
    Properties* properties = getProperties(context, attrs,0,0);//, defStyleAttr, defStyleRes);
    setOrientation(properties->orientation);
    setReverseLayout(properties->reverseLayout);
    setStackFromEnd(properties->stackFromEnd);
    delete properties;
}

LinearLayoutManager::~LinearLayoutManager(){
    delete mPendingSavedState;
    delete mLayoutState;
    delete mAnchorInfo;
    delete mLayoutChunkResult;
    delete mOrientationHelper;
}

bool LinearLayoutManager::isAutoMeasureEnabled()const{
    return true;
}

RecyclerView::LayoutParams* LinearLayoutManager::generateDefaultLayoutParams()const {
    return new RecyclerView::LayoutParams(ViewGroup::LayoutParams::WRAP_CONTENT,
            ViewGroup::LayoutParams::WRAP_CONTENT);
}

bool LinearLayoutManager::getRecycleChildrenOnDetach() const{
    return mRecycleChildrenOnDetach;
}

void LinearLayoutManager::setRecycleChildrenOnDetach(bool recycleChildrenOnDetach) {
    mRecycleChildrenOnDetach = recycleChildrenOnDetach;
}

void LinearLayoutManager::onDetachedFromWindow(RecyclerView& view, RecyclerView::Recycler& recycler) {
    LayoutManager::onDetachedFromWindow(view, recycler);
    if (mRecycleChildrenOnDetach) {
        removeAndRecycleAllViews(recycler);
        recycler.clear();
    }
}
#if 0
void LinearLayoutManager::onInitializeAccessibilityEvent(AccessibilityEvent& event) {
    LayoutManager::onInitializeAccessibilityEvent(event);
    if (getChildCount() > 0) {
        event.setFromIndex(findFirstVisibleItemPosition());
        event.setToIndex(findLastVisibleItemPosition());
    }
}
#endif
Parcelable* LinearLayoutManager::onSaveInstanceState() {
    if (mPendingSavedState != nullptr) {
        return new SavedState(*mPendingSavedState);
    }
    SavedState* state = new SavedState();
    if (getChildCount() > 0) {
        ensureLayoutState();
        bool didLayoutFromEnd = mLastStackFromEnd ^ mShouldReverseLayout;
        state->mAnchorLayoutFromEnd = didLayoutFromEnd;
        if (didLayoutFromEnd) {
            View* refChild = getChildClosestToEnd();
            state->mAnchorOffset = mOrientationHelper->getEndAfterPadding()
                    - mOrientationHelper->getDecoratedEnd(refChild);
            state->mAnchorPosition = getPosition(refChild);
        } else {
            View* refChild = getChildClosestToStart();
            state->mAnchorPosition = getPosition(refChild);
            state->mAnchorOffset = mOrientationHelper->getDecoratedStart(refChild)
                    - mOrientationHelper->getStartAfterPadding();
        }
    } else {
        state->invalidateAnchor();
    }
    return state;
}

void LinearLayoutManager::onRestoreInstanceState(Parcelable& state) {
    if (dynamic_cast<SavedState*>(&state)) {
        mPendingSavedState = (SavedState*)&state;
        requestLayout();
        LOGD("loaded saved state");
    } else if (_Debug) {
        LOGD("invalid saved state class");
    }
}

bool LinearLayoutManager::canScrollHorizontally()const{
    return mOrientation == HORIZONTAL;
}

bool LinearLayoutManager::canScrollVertically()const{
    return mOrientation == VERTICAL;
}

void LinearLayoutManager::setStackFromEnd(bool stackFromEnd) {
    assertNotInLayoutOrScroll("");
    if (mStackFromEnd == stackFromEnd) {
        return;
    }
    mStackFromEnd = stackFromEnd;
    requestLayout();
}

bool LinearLayoutManager::getStackFromEnd()const {
    return mStackFromEnd;
}

int LinearLayoutManager::getOrientation()const {
    return mOrientation;
}

void LinearLayoutManager::setOrientation(int orientation) {
    FATAL_IF( (orientation != HORIZONTAL)&&(orientation != VERTICAL),"invalid orientation:%d",orientation);

    assertNotInLayoutOrScroll("");

    if ( (orientation != mOrientation) || (mOrientationHelper == nullptr) ) {
        delete mOrientationHelper;
        mOrientationHelper =  OrientationHelper::createOrientationHelper(this, orientation);
        mAnchorInfo->mOrientationHelper = mOrientationHelper;
        mOrientation = orientation;
        requestLayout();
    }
}

void  LinearLayoutManager::resolveShouldLayoutReverse() {
        // A == B is the same result, but we rather keep it readable
    if ( (mOrientation == VERTICAL) || !isLayoutRTL()) {
        mShouldReverseLayout = mReverseLayout;
    } else {
        mShouldReverseLayout = !mReverseLayout;
    }
}

bool  LinearLayoutManager::getReverseLayout()const {
    return mReverseLayout;
}

void  LinearLayoutManager::setReverseLayout(bool reverseLayout) {
    assertNotInLayoutOrScroll("");
    if (reverseLayout == mReverseLayout) {
        return;
    }
    mReverseLayout = reverseLayout;
    requestLayout();
}

View* LinearLayoutManager::findViewByPosition(int position) {
    const int childCount = getChildCount();
    if (childCount == 0) {
        return nullptr;
    }
    const int firstChild = getPosition(getChildAt(0));
    const int viewPosition = position - firstChild;
    if ( (viewPosition >= 0) && (viewPosition < childCount) ) {
        View* child = getChildAt(viewPosition);
        if (getPosition(child) == position) {
            return child; // in pre-layout, this may not match
        }
    }
    // fallback to traversal. This might be necessary in pre-layout.
    return  LayoutManager::findViewByPosition(position);
}

int LinearLayoutManager::getExtraLayoutSpace(RecyclerView::State& state) {
    if (state.hasTargetScrollPosition()) {
        return mOrientationHelper->getTotalSpace();
    } else {
        return 0;
    }
}

void LinearLayoutManager::calculateExtraLayoutSpace(RecyclerView::State& state,int extraLayoutSpace[2]) {
    int extraLayoutSpaceStart = 0;
    int extraLayoutSpaceEnd = 0;

    // If calculateExtraLayoutSpace is not overridden, call the
    // deprecated getExtraLayoutSpace for backwards compatibility
    //@SuppressWarnings("deprecation")
    int extraScrollSpace = getExtraLayoutSpace(state);
    if (mLayoutState->mLayoutDirection == LayoutState::LAYOUT_START) {
        extraLayoutSpaceStart = extraScrollSpace;
    } else {
        extraLayoutSpaceEnd = extraScrollSpace;
    }
    extraLayoutSpace[0] = extraLayoutSpaceStart;
    extraLayoutSpace[1] = extraLayoutSpaceEnd;
}

void LinearLayoutManager::smoothScrollToPosition(RecyclerView& recyclerView, RecyclerView::State& state,int position) {
    LinearSmoothScroller* linearSmoothScroller = new LinearSmoothScroller(recyclerView.getContext());
    linearSmoothScroller->setTargetPosition(position);
    startSmoothScroll(linearSmoothScroller);
}

bool LinearLayoutManager::computeScrollVectorForPosition(int targetPosition,PointF&point) {
    if (getChildCount() == 0) {
        return false;
    }
    const int firstChildPos = getPosition(getChildAt(0));
    const int direction = targetPosition < firstChildPos != mShouldReverseLayout ? -1 : 1;
    if (mOrientation == HORIZONTAL) {
        point.set(float(direction), 0.f);
    } else {
        point.set(0.f, float(direction));
    }
    return true;
}

void LinearLayoutManager::onLayoutChildren(RecyclerView::Recycler& recycler, RecyclerView::State& state){
    // layout algorithm:
    // 1) by checking children and other variables, find an anchor coordinate and an anchor
    //  item position.
    // 2) fill towards start, stacking from bottom
    // 3) fill towards end, stacking from top
    // 4) scroll to fulfill requirements like stack from bottom.
    // create layout state
    LOGD_IF(_Debug,"is pre layout:%d",state.isPreLayout());
    if (mPendingSavedState || (mPendingScrollPosition != RecyclerView::NO_POSITION) ) {
        if (state.getItemCount() == 0) {
            removeAndRecycleAllViews(recycler);
            return;
        }
    }
    if (mPendingSavedState && mPendingSavedState->hasValidAnchor()) {
        mPendingScrollPosition = mPendingSavedState->mAnchorPosition;
    }

    ensureLayoutState();
    mLayoutState->mRecycle = false;
    // resolve layout direction
    resolveShouldLayoutReverse();

    View* focused = getFocusedChild();
    if (!mAnchorInfo->mValid || (mPendingScrollPosition != RecyclerView::NO_POSITION)
            || (mPendingSavedState != nullptr) ) {
        mAnchorInfo->reset();
        mAnchorInfo->mLayoutFromEnd = mShouldReverseLayout ^ mStackFromEnd;
        // calculate anchor position and coordinate
        updateAnchorInfoForLayout(recycler, state, *mAnchorInfo);
        mAnchorInfo->mValid = true;
    } else if (focused && ( (mOrientationHelper->getDecoratedStart(focused)>= mOrientationHelper->getEndAfterPadding())
            || (mOrientationHelper->getDecoratedEnd(focused) <= mOrientationHelper->getStartAfterPadding()) )) {
        // This case relates to when the anchor child is the focused view and due to layout
        // shrinking the focused view fell outside the viewport, e.g. when soft keyboard shows
        // up after tapping an EditText which shrinks RV causing the focused view (The tapped
        // EditText which is the anchor child) to get kicked out of the screen. Will update the
        // anchor coordinate in order to make sure that the focused view is laid out. Otherwise,
        // the available space in layoutState will be calculated as negative preventing the
        // focused view from being laid out in fill.
        // Note that we won't update the anchor position between layout passes (refer to
        // TestResizingRelayoutWithAutoMeasure), which happens if we were to call
        // updateAnchorInfoForLayout for an anchor that's not the focused view (e.g. a reference
        // child which can change between layout passes).
        mAnchorInfo->assignFromViewAndKeepVisibleRect(focused, getPosition(focused));
    }
    LOGD_IF(_Debug,"Anchor info:%p",mAnchorInfo);

    // LLM may decide to layout items for "extra" pixels to account for scrolling target,
    // caching or predictive animations.

    mLayoutState->mLayoutDirection = mLayoutState->mLastScrollDelta >= 0
            ? LayoutState::LAYOUT_END : LayoutState::LAYOUT_START;
    mReusableIntPair[0] = 0;
    mReusableIntPair[1] = 0;
    calculateExtraLayoutSpace(state, mReusableIntPair);
    int extraForStart = std::max(0, mReusableIntPair[0]) + mOrientationHelper->getStartAfterPadding();
    int extraForEnd = std::max(0, mReusableIntPair[1]) + mOrientationHelper->getEndPadding();
    if (state.isPreLayout() && (mPendingScrollPosition != RecyclerView::NO_POSITION)
            && (mPendingScrollPositionOffset != INVALID_OFFSET) ) {
        // if the child is visible and we are going to move it around, we should layout
        // extra items in the opposite direction to make sure new items animate nicely
        // instead of just fading in
        View* existing = findViewByPosition(mPendingScrollPosition);
        if (existing != nullptr) {
            int current;
            int upcomingOffset;
            if (mShouldReverseLayout) {
                current = mOrientationHelper->getEndAfterPadding()
                        - mOrientationHelper->getDecoratedEnd(existing);
                upcomingOffset = current - mPendingScrollPositionOffset;
            } else {
                current = mOrientationHelper->getDecoratedStart(existing)
                        - mOrientationHelper->getStartAfterPadding();
                upcomingOffset = mPendingScrollPositionOffset - current;
            }
            if (upcomingOffset > 0) {
                extraForStart += upcomingOffset;
            } else {
                extraForEnd -= upcomingOffset;
            }
        }
    }
    int startOffset;
    int endOffset;
    int firstLayoutDirection;
    if (mAnchorInfo->mLayoutFromEnd) {
        firstLayoutDirection = mShouldReverseLayout ? LayoutState::ITEM_DIRECTION_TAIL
                : LayoutState::ITEM_DIRECTION_HEAD;
    } else {
        firstLayoutDirection = mShouldReverseLayout ? LayoutState::ITEM_DIRECTION_HEAD
                : LayoutState::ITEM_DIRECTION_TAIL;
    }

    onAnchorReady(recycler, state, *mAnchorInfo, firstLayoutDirection);
    detachAndScrapAttachedViews(recycler);
    mLayoutState->mInfinite = resolveIsInfinite();
    mLayoutState->mIsPreLayout = state.isPreLayout();
    // noRecycleSpace not needed: recycling doesn't happen in below's fill
    // invocations because mScrollingOffset is set to SCROLLING_OFFSET_NaN
    mLayoutState->mNoRecycleSpace = 0;
    if (mAnchorInfo->mLayoutFromEnd) {
        // fill towards start
        updateLayoutStateToFillStart(*mAnchorInfo);
        mLayoutState->mExtraFillSpace = extraForStart;
        fill(recycler, *mLayoutState, state, false);
        startOffset = mLayoutState->mOffset;
        int firstElement = mLayoutState->mCurrentPosition;
        if (mLayoutState->mAvailable > 0) {
            extraForEnd += mLayoutState->mAvailable;
        }
        // fill towards end
        updateLayoutStateToFillEnd(*mAnchorInfo);
        mLayoutState->mExtraFillSpace = extraForEnd;
        mLayoutState->mCurrentPosition += mLayoutState->mItemDirection;
        fill(recycler, *mLayoutState, state, false);
        endOffset = mLayoutState->mOffset;

        if (mLayoutState->mAvailable > 0) {
            // end could not consume all. add more items towards start
            extraForStart = mLayoutState->mAvailable;
            updateLayoutStateToFillStart(firstElement, startOffset);
            mLayoutState->mExtraFillSpace = extraForStart;
            fill(recycler, *mLayoutState, state, false);
            startOffset = mLayoutState->mOffset;
        }
    } else {
        // fill towards end
        updateLayoutStateToFillEnd(*mAnchorInfo);
        mLayoutState->mExtraFillSpace = extraForEnd;
        fill(recycler, *mLayoutState, state, false);
        endOffset = mLayoutState->mOffset;
        const int lastElement = mLayoutState->mCurrentPosition;
        if (mLayoutState->mAvailable > 0) {
            extraForStart += mLayoutState->mAvailable;
        }
        // fill towards start
        updateLayoutStateToFillStart(*mAnchorInfo);
        mLayoutState->mExtraFillSpace = extraForStart;
        mLayoutState->mCurrentPosition += mLayoutState->mItemDirection;
        fill(recycler, *mLayoutState, state, false);
        startOffset = mLayoutState->mOffset;

        if (mLayoutState->mAvailable > 0) {
            extraForEnd = mLayoutState->mAvailable;
            // start could not consume all it should. add more items towards end
            updateLayoutStateToFillEnd(lastElement, endOffset);
            mLayoutState->mExtraFillSpace = extraForEnd;
            fill(recycler, *mLayoutState, state, false);
            endOffset = mLayoutState->mOffset;
        }
    }

    // changes may cause gaps on the UI, try to fix them.
    // TODO we can probably avoid this if neither stackFromEnd/reverseLayout/RTL values have
    // changed
    if (getChildCount() > 0) {
        // because layout from end may be changed by scroll to position
        // we re-calculate it.
        // find which side we should check for gaps.
        if (mShouldReverseLayout ^ mStackFromEnd) {
            int fixOffset = fixLayoutEndGap(endOffset, recycler, state, true);
            startOffset += fixOffset;
            endOffset += fixOffset;
            fixOffset = fixLayoutStartGap(startOffset, recycler, state, false);
            startOffset += fixOffset;
            endOffset += fixOffset;
        } else {
            int fixOffset = fixLayoutStartGap(startOffset, recycler, state, true);
            startOffset += fixOffset;
            endOffset += fixOffset;
            fixOffset = fixLayoutEndGap(endOffset, recycler, state, false);
            startOffset += fixOffset;
            endOffset += fixOffset;
        }
    }
    layoutForPredictiveAnimations(recycler, state, startOffset, endOffset);
    if (!state.isPreLayout()) {
        mOrientationHelper->onLayoutComplete();
    } else {
        mAnchorInfo->reset();
    }
    mLastStackFromEnd = mStackFromEnd;
    if (_Debug) {
        validateChildOrder();
    }
}

void LinearLayoutManager::onLayoutCompleted(RecyclerView::State& state) {
    LayoutManager::onLayoutCompleted(state);
    mPendingSavedState = nullptr; // we don't need this anymore
    mPendingScrollPosition = RecyclerView::NO_POSITION;
    mPendingScrollPositionOffset = INVALID_OFFSET;
    mAnchorInfo->reset();
}

void LinearLayoutManager::onAnchorReady(RecyclerView::Recycler& recycler, RecyclerView::State& state,
        AnchorInfo& anchorInfo, int firstLayoutItemDirection) {
}

void LinearLayoutManager::layoutForPredictiveAnimations(RecyclerView::Recycler& recycler,
            RecyclerView::State& state, int startOffset, int endOffset) {
    // If there are scrap children that we did not layout, we need to find where they did go
    // and layout them accordingly so that animations can work as expected.
    // This case may happen if new views are added or an existing view expands and pushes
    // another view out of bounds.
    if (!state.willRunPredictiveAnimations() ||  getChildCount() == 0 || state.isPreLayout()
            || !supportsPredictiveItemAnimations()) {
        return;
    }
    // to make the logic simpler, we calculate the size of children and call fill.
    int scrapExtraStart = 0, scrapExtraEnd = 0;
    std::vector<RecyclerView::ViewHolder*> scrapList = recycler.getScrapList();
    const size_t scrapSize = scrapList.size();
    const int firstChildPos = getPosition(getChildAt(0));
    for (size_t i = 0; i < scrapSize; i++) {
        RecyclerView::ViewHolder* scrap = scrapList.at(i);
        if (scrap->isRemoved()) {
            continue;
        }
        const int position = scrap->getLayoutPosition();
        const int direction = position < firstChildPos != mShouldReverseLayout
                ? LayoutState::LAYOUT_START : LayoutState::LAYOUT_END;
        if (direction == LayoutState::LAYOUT_START) {
            scrapExtraStart += mOrientationHelper->getDecoratedMeasurement(scrap->itemView);
        } else {
            scrapExtraEnd += mOrientationHelper->getDecoratedMeasurement(scrap->itemView);
        }
    }

    LOGD_IF(_Debug,"for unused scrap, decided to add %d towards start and %d towards end",
		    scrapExtraStart,scrapExtraEnd);
    mLayoutState->mScrapList = scrapList;
    if (scrapExtraStart > 0) {
        View* anchor = getChildClosestToStart();
        updateLayoutStateToFillStart(getPosition(anchor), startOffset);
        mLayoutState->mExtraFillSpace = scrapExtraStart;
        mLayoutState->mAvailable = 0;
        mLayoutState->assignPositionFromScrapList();
        fill(recycler, *mLayoutState, state, false);
    }

    if (scrapExtraEnd > 0) {
        View* anchor = getChildClosestToEnd();
        updateLayoutStateToFillEnd(getPosition(anchor), endOffset);
        mLayoutState->mExtraFillSpace = scrapExtraEnd;
        mLayoutState->mAvailable = 0;
        mLayoutState->assignPositionFromScrapList();
        fill(recycler, *mLayoutState, state, false);
    }
    mLayoutState->mScrapList.clear();
    //LOGW("mLayoutState->mScrapList = nullptr;TOBE OPENED");
}

void LinearLayoutManager::updateAnchorInfoForLayout(RecyclerView::Recycler& recycler, RecyclerView::State& state,
        AnchorInfo& anchorInfo) {
    if (updateAnchorFromPendingData(state, anchorInfo)) {
        LOGD_IF(_Debug,"updated anchor info from pending information");
        return;
    }

    if (updateAnchorFromChildren(recycler, state, anchorInfo)) {
        LOGD_IF(_Debug,"updated anchor info from existing children");
        return;
    }
    LOGD_IF(_Debug,"deciding anchor info for fresh state");
    anchorInfo.assignCoordinateFromPadding();
    anchorInfo.mPosition = mStackFromEnd ? state.getItemCount() - 1 : 0;
}

bool LinearLayoutManager::updateAnchorFromChildren(RecyclerView::Recycler& recycler,
            RecyclerView::State& state, AnchorInfo& anchorInfo) {
    if (getChildCount() == 0) {
        return false;
    }
    View* focused = getFocusedChild();
    if (focused != nullptr && anchorInfo.isViewValidAsAnchor(focused, state)) {
        anchorInfo.assignFromViewAndKeepVisibleRect(focused, getPosition(focused));
        return true;
    }
    if (mLastStackFromEnd != mStackFromEnd) {
        return false;
    }
    View* referenceChild = anchorInfo.mLayoutFromEnd
            ? findReferenceChildClosestToEnd(recycler, state)
            : findReferenceChildClosestToStart(recycler, state);
    if (referenceChild != nullptr) {
        anchorInfo.assignFromView(referenceChild, getPosition(referenceChild));
        // If all visible views are removed in 1 pass, reference child might be out of bounds.
        // If that is the case, offset it back to 0 so that we use these pre-layout children.
        if (!state.isPreLayout() && supportsPredictiveItemAnimations()) {
            // validate this child is at least partially visible. if not, offset it to start
            const bool notVisible =  mOrientationHelper->getDecoratedStart(referenceChild)
		    >= mOrientationHelper->getEndAfterPadding()
                    || mOrientationHelper->getDecoratedEnd(referenceChild)
                    < mOrientationHelper->getStartAfterPadding();
            if (notVisible) {
                anchorInfo.mCoordinate = anchorInfo.mLayoutFromEnd
                        ? mOrientationHelper->getEndAfterPadding()
                        : mOrientationHelper->getStartAfterPadding();
            }
        }
        return true;
    }
    return false;
}

bool LinearLayoutManager::updateAnchorFromPendingData(RecyclerView::State& state, AnchorInfo& anchorInfo) {
    if (state.isPreLayout() || (mPendingScrollPosition == RecyclerView::NO_POSITION) ) {
        return false;
    }
    // validate scroll position
    if ( (mPendingScrollPosition < 0) || (mPendingScrollPosition >= state.getItemCount()) ) {
        mPendingScrollPosition = RecyclerView::NO_POSITION;
        mPendingScrollPositionOffset = INVALID_OFFSET;
        LOGD_IF(_Debug,"ignoring invalid scroll position %d",mPendingScrollPosition);
        return false;
    }

    // if child is visible, try to make it a reference child and ensure it is fully visible.
    // if child is not visible, align it depending on its virtual position.
    anchorInfo.mPosition = mPendingScrollPosition;
    if (mPendingSavedState && mPendingSavedState->hasValidAnchor()) {
        // Anchor offset depends on how that child was laid out. Here, we update it
        // according to our current view bounds
        anchorInfo.mLayoutFromEnd = mPendingSavedState->mAnchorLayoutFromEnd;
        if (anchorInfo.mLayoutFromEnd) {
            anchorInfo.mCoordinate = mOrientationHelper->getEndAfterPadding()
                    - mPendingSavedState->mAnchorOffset;
        } else {
            anchorInfo.mCoordinate = mOrientationHelper->getStartAfterPadding()
                    + mPendingSavedState->mAnchorOffset;
        }
        return true;
    }

    if (mPendingScrollPositionOffset == INVALID_OFFSET) {
        View* child = findViewByPosition(mPendingScrollPosition);
        if (child != nullptr) {
            const int childSize = mOrientationHelper->getDecoratedMeasurement(child);
            if (childSize > mOrientationHelper->getTotalSpace()) {
                // item does not fit. fix depending on layout direction
                anchorInfo.assignCoordinateFromPadding();
                return true;
            }
            const int startGap = mOrientationHelper->getDecoratedStart(child)
                    - mOrientationHelper->getStartAfterPadding();
            if (startGap < 0) {
                anchorInfo.mCoordinate = mOrientationHelper->getStartAfterPadding();
                anchorInfo.mLayoutFromEnd = false;
                return true;
            }
            const int endGap = mOrientationHelper->getEndAfterPadding()
                    - mOrientationHelper->getDecoratedEnd(child);
            if (endGap < 0) {
                anchorInfo.mCoordinate = mOrientationHelper->getEndAfterPadding();
                anchorInfo.mLayoutFromEnd = true;
                return true;
            }
            anchorInfo.mCoordinate = anchorInfo.mLayoutFromEnd
                    ? (mOrientationHelper->getDecoratedEnd(child) + mOrientationHelper
                    ->getTotalSpaceChange())
                    : mOrientationHelper->getDecoratedStart(child);
        } else { // item is not visible.
            if (getChildCount() > 0) {
                // get position of any child, does not matter
                int pos = getPosition(getChildAt(0));
                anchorInfo.mLayoutFromEnd = mPendingScrollPosition < pos
                        == mShouldReverseLayout;
            }
            anchorInfo.assignCoordinateFromPadding();
        }
        return true;
    }
    // override layout from end values for consistency
    anchorInfo.mLayoutFromEnd = mShouldReverseLayout;
    // if this changes, we should update prepareForDrop as well
    if (mShouldReverseLayout) {
        anchorInfo.mCoordinate = mOrientationHelper->getEndAfterPadding()
                - mPendingScrollPositionOffset;
    } else {
        anchorInfo.mCoordinate = mOrientationHelper->getStartAfterPadding()
                + mPendingScrollPositionOffset;
    }
    return true;
}

int LinearLayoutManager::fixLayoutEndGap(int endOffset, RecyclerView::Recycler& recycler,
            RecyclerView::State& state, bool canOffsetChildren) {
    int gap = mOrientationHelper->getEndAfterPadding() - endOffset;
    int fixOffset = 0;
    if (gap > 0) {
        fixOffset = -scrollBy(-gap, recycler, state);
    } else {
        return 0; // nothing to fix
    }
    // move offset according to scroll amount
    endOffset += fixOffset;
    if (canOffsetChildren) {
        // re-calculate gap, see if we could fix it
        gap = mOrientationHelper->getEndAfterPadding() - endOffset;
        if (gap > 0) {
            mOrientationHelper->offsetChildren(gap);
            return gap + fixOffset;
        }
    }
    return fixOffset;
}

int LinearLayoutManager::fixLayoutStartGap(int startOffset, RecyclerView::Recycler& recycler,
            RecyclerView::State& state, bool canOffsetChildren) {
    int gap = startOffset - mOrientationHelper->getStartAfterPadding();
    int fixOffset = 0;
    if (gap > 0) {
        // check if we should fix this gap.
        fixOffset = -scrollBy(gap, recycler, state);
    } else {
        return 0; // nothing to fix
    }
    startOffset += fixOffset;
    if (canOffsetChildren) {
        // re-calculate gap, see if we could fix it
        gap = startOffset - mOrientationHelper->getStartAfterPadding();
        if (gap > 0) {
            mOrientationHelper->offsetChildren(-gap);
            return fixOffset - gap;
        }
    }
    return fixOffset;
}

void LinearLayoutManager::updateLayoutStateToFillEnd(AnchorInfo& anchorInfo) {
    updateLayoutStateToFillEnd(anchorInfo.mPosition, anchorInfo.mCoordinate);
}

void LinearLayoutManager::updateLayoutStateToFillEnd(int itemPosition, int offset) {
    mLayoutState->mAvailable = mOrientationHelper->getEndAfterPadding() - offset;
    mLayoutState->mItemDirection = mShouldReverseLayout ? LayoutState::ITEM_DIRECTION_HEAD :
            LayoutState::ITEM_DIRECTION_TAIL;
    mLayoutState->mCurrentPosition = itemPosition;
    mLayoutState->mLayoutDirection = LayoutState::LAYOUT_END;
    mLayoutState->mOffset = offset;
    mLayoutState->mScrollingOffset = LayoutState::SCROLLING_OFFSET_NaN;
}

void LinearLayoutManager::updateLayoutStateToFillStart(AnchorInfo& anchorInfo) {
    updateLayoutStateToFillStart(anchorInfo.mPosition, anchorInfo.mCoordinate);
}

void LinearLayoutManager::updateLayoutStateToFillStart(int itemPosition, int offset) {
    mLayoutState->mAvailable = offset - mOrientationHelper->getStartAfterPadding();
    mLayoutState->mCurrentPosition = itemPosition;
    mLayoutState->mItemDirection = mShouldReverseLayout ? LayoutState::ITEM_DIRECTION_TAIL :
            LayoutState::ITEM_DIRECTION_HEAD;
    mLayoutState->mLayoutDirection = LayoutState::LAYOUT_START;
    mLayoutState->mOffset = offset;
    mLayoutState->mScrollingOffset = LayoutState::SCROLLING_OFFSET_NaN;

}

bool LinearLayoutManager::isLayoutRTL(){
    return getLayoutDirection() == View::LAYOUT_DIRECTION_RTL;
}

void LinearLayoutManager::ensureLayoutState() {
    if (mLayoutState == nullptr) {
        mLayoutState = createLayoutState();
    }
}

LinearLayoutManager::LayoutState* LinearLayoutManager::createLayoutState() {
    return new LayoutState();
}

void LinearLayoutManager::scrollToPosition(int position) {
    mPendingScrollPosition = position;
    mPendingScrollPositionOffset = INVALID_OFFSET;
    if (mPendingSavedState != nullptr) {
        mPendingSavedState->invalidateAnchor();
    }
    requestLayout();
}

void LinearLayoutManager::scrollToPositionWithOffset(int position, int offset) {
    mPendingScrollPosition = position;
    mPendingScrollPositionOffset = offset;
    if (mPendingSavedState != nullptr) {
        mPendingSavedState->invalidateAnchor();
    }
    requestLayout();
}

int LinearLayoutManager::scrollHorizontallyBy(int dx, RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    if (mOrientation == VERTICAL) {
        return 0;
    }
    return scrollBy(dx, recycler, state);
}

int LinearLayoutManager::scrollVerticallyBy(int dy, RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    if (mOrientation == HORIZONTAL) {
        return 0;
    }
    return scrollBy(dy, recycler, state);
}

int LinearLayoutManager::computeHorizontalScrollOffset(RecyclerView::State& state) {
    return computeScrollOffset(state);
}

int LinearLayoutManager::computeVerticalScrollOffset(RecyclerView::State& state) {
    return computeScrollOffset(state);
}

int LinearLayoutManager::computeHorizontalScrollExtent(RecyclerView::State& state) {
    return computeScrollExtent(state);
}

int LinearLayoutManager::computeVerticalScrollExtent(RecyclerView::State& state) {
    return computeScrollExtent(state);
}

int LinearLayoutManager::computeHorizontalScrollRange(RecyclerView::State& state) {
    return computeScrollRange(state);
}

int LinearLayoutManager::computeVerticalScrollRange(RecyclerView::State& state) {
    return computeScrollRange(state);
}

int LinearLayoutManager::computeScrollOffset(RecyclerView::State& state) {
    if (getChildCount() == 0) {
        return 0;
    }
    ensureLayoutState();
    return ScrollbarHelper::computeScrollOffset(state, *mOrientationHelper,
            findFirstVisibleChildClosestToStart(!mSmoothScrollbarEnabled, true),
            findFirstVisibleChildClosestToEnd(!mSmoothScrollbarEnabled, true),
            *this, mSmoothScrollbarEnabled, mShouldReverseLayout);
}

int LinearLayoutManager::computeScrollExtent(RecyclerView::State& state) {
    if (getChildCount() == 0) {
        return 0;
    }
    ensureLayoutState();
    return ScrollbarHelper::computeScrollExtent(state, *mOrientationHelper,
            findFirstVisibleChildClosestToStart(!mSmoothScrollbarEnabled, true),
            findFirstVisibleChildClosestToEnd(!mSmoothScrollbarEnabled, true),
            *this,  mSmoothScrollbarEnabled);
}

int LinearLayoutManager::computeScrollRange(RecyclerView::State& state) {
    if (getChildCount() == 0) {
        return 0;
    }
    ensureLayoutState();
    return ScrollbarHelper::computeScrollRange(state, *mOrientationHelper,
            findFirstVisibleChildClosestToStart(!mSmoothScrollbarEnabled, true),
            findFirstVisibleChildClosestToEnd(!mSmoothScrollbarEnabled, true),
            *this, mSmoothScrollbarEnabled);
}

void LinearLayoutManager::setSmoothScrollbarEnabled(bool enabled) {
     mSmoothScrollbarEnabled = enabled;
}

bool LinearLayoutManager::isSmoothScrollbarEnabled() {
    return mSmoothScrollbarEnabled;
}

void LinearLayoutManager::updateLayoutState(int layoutDirection, int requiredSpace,
            bool canUseExistingSpace, RecyclerView::State& state) {
    // If parent provides a hint, don't measure unlimited.
    mLayoutState->mInfinite = resolveIsInfinite();
    mLayoutState->mLayoutDirection = layoutDirection;
    mReusableIntPair[0] = 0;
    mReusableIntPair[1] = 0;
    calculateExtraLayoutSpace(state, mReusableIntPair);
    int extraForStart = std::max(0, mReusableIntPair[0]);
    int extraForEnd = std::max(0, mReusableIntPair[1]);
    bool layoutToEnd = layoutDirection == LayoutState::LAYOUT_END;
    mLayoutState->mExtraFillSpace = layoutToEnd ? extraForEnd : extraForStart;
    mLayoutState->mNoRecycleSpace = layoutToEnd ? extraForStart : extraForEnd;
    int scrollingOffset;
    if (layoutToEnd) {
        mLayoutState->mExtraFillSpace += mOrientationHelper->getEndPadding();
        // get the first child in the direction we are going
        View* child = getChildClosestToEnd();
        // the direction in which we are traversing children
        mLayoutState->mItemDirection = mShouldReverseLayout ? LayoutState::ITEM_DIRECTION_HEAD
                : LayoutState::ITEM_DIRECTION_TAIL;
        mLayoutState->mCurrentPosition = getPosition(child) + mLayoutState->mItemDirection;
        mLayoutState->mOffset = mOrientationHelper->getDecoratedEnd(child);
        // calculate how much we can scroll without adding new children (independent of layout)
        scrollingOffset = mOrientationHelper->getDecoratedEnd(child)
                - mOrientationHelper->getEndAfterPadding();

    } else {
        View* child = getChildClosestToStart();
        mLayoutState->mExtraFillSpace += mOrientationHelper->getStartAfterPadding();
        mLayoutState->mItemDirection = mShouldReverseLayout ? LayoutState::ITEM_DIRECTION_TAIL
                : LayoutState::ITEM_DIRECTION_HEAD;
        mLayoutState->mCurrentPosition = getPosition(child) + mLayoutState->mItemDirection;
        mLayoutState->mOffset = mOrientationHelper->getDecoratedStart(child);
        scrollingOffset = -mOrientationHelper->getDecoratedStart(child)
                + mOrientationHelper->getStartAfterPadding();
    }
    mLayoutState->mAvailable = requiredSpace;
    if (canUseExistingSpace) {
        mLayoutState->mAvailable -= scrollingOffset;
    }
    mLayoutState->mScrollingOffset = scrollingOffset;
}

bool LinearLayoutManager::resolveIsInfinite() {
    return mOrientationHelper->getMode() == MeasureSpec::UNSPECIFIED
            && mOrientationHelper->getEnd() == 0;
}

void LinearLayoutManager::collectPrefetchPositionsForLayoutState(RecyclerView::State& state, LayoutState& layoutState,
        LayoutPrefetchRegistry& layoutPrefetchRegistry) {
    const int pos = layoutState.mCurrentPosition;
    if (pos >= 0 && pos < state.getItemCount()) {
        layoutPrefetchRegistry(pos, std::max(0, layoutState.mScrollingOffset));//addPosition
    }
}

void LinearLayoutManager::collectInitialPrefetchPositions(int adapterItemCount, LayoutPrefetchRegistry& layoutPrefetchRegistry) {
    bool fromEnd;
    int anchorPos;
    if (mPendingSavedState != nullptr && mPendingSavedState->hasValidAnchor()) {
        // use restored state, since it hasn't been resolved yet
        fromEnd = mPendingSavedState->mAnchorLayoutFromEnd;
        anchorPos = mPendingSavedState->mAnchorPosition;
    } else {
        resolveShouldLayoutReverse();
        fromEnd = mShouldReverseLayout;
        if (mPendingScrollPosition == RecyclerView::NO_POSITION) {
            anchorPos = fromEnd ? adapterItemCount - 1 : 0;
        } else {
            anchorPos = mPendingScrollPosition;
        }
    }

    const int direction = fromEnd
            ? LayoutState::ITEM_DIRECTION_HEAD
            : LayoutState::ITEM_DIRECTION_TAIL;
    int targetPos = anchorPos;
    for (int i = 0; i < mInitialPrefetchItemCount; i++) {
        if (targetPos >= 0 && targetPos < adapterItemCount) {
            layoutPrefetchRegistry(targetPos, 0);//addPosition
        } else {
            break; // no more to prefetch
        }
        targetPos += direction;
    }
}

void LinearLayoutManager::setInitialPrefetchItemCount(int itemCount) {
    mInitialPrefetchItemCount = itemCount;
}

/**
 * Gets the number of items to prefetch in
 * {@link #collectInitialPrefetchPositions(int, LayoutPrefetchRegistry)}, which defines
 * how many inner items should be prefetched when this LayoutManager's RecyclerView
 * is nested inside another RecyclerView.
 *
 * @see #isItemPrefetchEnabled()
 * @see #setInitialPrefetchItemCount(int)
 * @see #collectInitialPrefetchPositions(int, LayoutPrefetchRegistry)
 *
 * @return number of items to prefetch.
 */
int LinearLayoutManager::getInitialPrefetchItemCount() {
    return mInitialPrefetchItemCount;
}

void LinearLayoutManager::collectAdjacentPrefetchPositions(int dx, int dy, RecyclerView::State& state,
        LayoutPrefetchRegistry& layoutPrefetchRegistry) {
    int delta = (mOrientation == HORIZONTAL) ? dx : dy;
    if (getChildCount() == 0 || delta == 0) {
        // can't support this scroll, so don't bother prefetching
        return;
    }

    ensureLayoutState();
    const int layoutDirection = delta > 0 ? LayoutState::LAYOUT_END : LayoutState::LAYOUT_START;
    const int absDelta = std::abs(delta);
    updateLayoutState(layoutDirection, absDelta, true, state);
    collectPrefetchPositionsForLayoutState(state, *mLayoutState, layoutPrefetchRegistry);
}

int LinearLayoutManager::scrollBy(int delta, RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    if (getChildCount() == 0 || delta == 0) {
        return 0;
    }
    ensureLayoutState();
    mLayoutState->mRecycle = true;
    const int layoutDirection = delta > 0 ? LayoutState::LAYOUT_END : LayoutState::LAYOUT_START;
    const int absDelta = std::abs(delta);
    updateLayoutState(layoutDirection, absDelta, true, state);
    const int consumed = mLayoutState->mScrollingOffset + fill(recycler, *mLayoutState, state, false);
    if (consumed < 0) {
        LOGD_IF(_Debug,"Don't have any more elements to scroll");
        return 0;
    }
    const int scrolled = absDelta > consumed ? layoutDirection * consumed : delta;
    mOrientationHelper->offsetChildren(-scrolled);
    LOGD_IF(_Debug,"scroll req: %d scrolled %d",delta,scrolled);
    mLayoutState->mLastScrollDelta = scrolled;
    return scrolled;
}

void LinearLayoutManager::assertNotInLayoutOrScroll(const std::string& message) {
    if (mPendingSavedState == nullptr) {
        LayoutManager::assertNotInLayoutOrScroll(message);
    }
}

void LinearLayoutManager::recycleChildren(RecyclerView::Recycler& recycler, int startIndex, int endIndex) {
    if (startIndex == endIndex) {
        return;
    }
    LOGD_IF(_Debug,"Recycling %d items",std::abs(startIndex - endIndex));
    if (endIndex > startIndex) {
        for (int i = endIndex - 1; i >= startIndex; i--) {
            removeAndRecycleViewAt(i, recycler);
        }
    } else {
        for (int i = startIndex; i > endIndex; i--) {
            removeAndRecycleViewAt(i, recycler);
        }
    }
}

/**
 * Recycles views that went out of bounds after scrolling towards the end of the layout.
 * <p>
 * Checks both layout position and visible position to guarantee that the view is not visible.
 *
 * @param recycler Recycler instance of {@link RecyclerView}
 * @param scrollingOffset This can be used to add additional padding to the visible area. This
 *                        is used to detect children that will go out of bounds after scrolling,
 *                        without actually moving them.
 * @param noRecycleSpace Extra space that should be excluded from recycling. This is the space
 *                       from {@code extraLayoutSpace[0]}, calculated in {@link
 *                       #calculateExtraLayoutSpace}.
 */
void LinearLayoutManager::recycleViewsFromStart(RecyclerView::Recycler& recycler, int scrollingOffset,int noRecycleSpace) {
    if (scrollingOffset < 0) {
        LOGD_IF(_Debug,"Called recycle from start with a negative value. This might happen"
                  " during layout changes but may be sign of a bug");
        return;
    }
    // ignore padding, ViewGroup may not clip children.
    const int limit = scrollingOffset - noRecycleSpace;
    const int childCount = getChildCount();
    if (mShouldReverseLayout) {
        for (int i = childCount - 1; i >= 0; i--) {
            View* child = getChildAt(i);
            if (mOrientationHelper->getDecoratedEnd(child) > limit
                    || mOrientationHelper->getTransformedEndWithDecoration(child) > limit) {
                // stop here
                recycleChildren(recycler, childCount - 1, i);
                return;
            }
        }
    } else {
        for (int i = 0; i < childCount; i++) {
            View* child = getChildAt(i);
            if (mOrientationHelper->getDecoratedEnd(child) > limit
                    || mOrientationHelper->getTransformedEndWithDecoration(child) > limit) {
                // stop here
                recycleChildren(recycler, 0, i);
                return;
            }
        }
    }
}

void LinearLayoutManager::recycleViewsFromEnd(RecyclerView::Recycler& recycler, int scrollingOffset,
            int noRecycleSpace) {
    const int childCount = getChildCount();
    if (scrollingOffset < 0) {
        LOGD_IF(_Debug,"Called recycle from end with a negative value. This might happen"
                " during layout changes but may be sign of a bug");
        return;
    }
    const int limit = mOrientationHelper->getEnd() - scrollingOffset + noRecycleSpace;
    if (mShouldReverseLayout) {
        for (int i = 0; i < childCount; i++) {
            View* child = getChildAt(i);
            if (mOrientationHelper->getDecoratedStart(child) < limit
                    || mOrientationHelper->getTransformedStartWithDecoration(child) < limit) {
                // stop here
                recycleChildren(recycler, 0, i);
                return;
            }
        }
    } else {
        for (int i = childCount - 1; i >= 0; i--) {
            View* child = getChildAt(i);
            if (mOrientationHelper->getDecoratedStart(child) < limit
                    || mOrientationHelper->getTransformedStartWithDecoration(child) < limit) {
                // stop here
                recycleChildren(recycler, childCount - 1, i);
                return;
            }
        }
    }
}

/**
 * Helper method to call appropriate recycle method depending on current layout direction
 *
 * @param recycler    Current recycler that is attached to RecyclerView
 * @param layoutState Current layout state. Right now, this object does not change but
 *                    we may consider moving it out of this view so passing around as a
 *                    parameter for now, rather than accessing {@link #mLayoutState}
 * @see #recycleViewsFromStart(RecyclerView.Recycler, int, int)
 * @see #recycleViewsFromEnd(RecyclerView.Recycler, int, int)
 * @see LinearLayoutManager.LayoutState#mLayoutDirection
 */
void LinearLayoutManager::recycleByLayoutState(RecyclerView::Recycler& recycler, LayoutState& layoutState) {
    if (!layoutState.mRecycle || layoutState.mInfinite) {
        return;
    }
    const int scrollingOffset = layoutState.mScrollingOffset;
    const int noRecycleSpace = layoutState.mNoRecycleSpace;
    if (layoutState.mLayoutDirection == LayoutState::LAYOUT_START) {
        recycleViewsFromEnd(recycler, scrollingOffset, noRecycleSpace);
    } else {
        recycleViewsFromStart(recycler, scrollingOffset, noRecycleSpace);
    }
}

int LinearLayoutManager::fill(RecyclerView::Recycler& recycler, LayoutState& layoutState,
        RecyclerView::State& state, bool stopOnFocusable) {
    // max offset we should set is mFastScroll + available
    const int start = layoutState.mAvailable;
    if (layoutState.mScrollingOffset != LayoutState::SCROLLING_OFFSET_NaN) {
        // TODO ugly bug fix. should not happen
        if (layoutState.mAvailable < 0) {
            layoutState.mScrollingOffset += layoutState.mAvailable;
        }
        recycleByLayoutState(recycler, layoutState);
    }
    int remainingSpace = layoutState.mAvailable + layoutState.mExtraFillSpace;
    LayoutChunkResult* layoutChunkResult = mLayoutChunkResult;
    while ((layoutState.mInfinite || remainingSpace > 0) && layoutState.hasMore(state)) {
        layoutChunkResult->resetInternal();
        layoutChunk(recycler, state, layoutState,*layoutChunkResult);
        if (layoutChunkResult->mFinished) {
            break;
        }
        layoutState.mOffset += layoutChunkResult->mConsumed * layoutState.mLayoutDirection;
        /**
         * Consume the available space if:
         * * layoutChunk did not request to be ignored
         * * OR we are laying out scrap children
         * * OR we are not doing pre-layout
         */
        if (!layoutChunkResult->mIgnoreConsumed || layoutState.mScrapList.size()/* != nullptr*/
                || !state.isPreLayout()) {
            layoutState.mAvailable -= layoutChunkResult->mConsumed;
            // we keep a separate remaining space because mAvailable is important for recycling
            remainingSpace -= layoutChunkResult->mConsumed;
        }

        if (layoutState.mScrollingOffset != LayoutState::SCROLLING_OFFSET_NaN) {
            layoutState.mScrollingOffset += layoutChunkResult->mConsumed;
            if (layoutState.mAvailable < 0) {
                layoutState.mScrollingOffset += layoutState.mAvailable;
            }
            recycleByLayoutState(recycler, layoutState);
        }
        if (stopOnFocusable && layoutChunkResult->mFocusable) {
            break;
        }
    }
    if (_Debug) {
        validateChildOrder();
    }
    return start - layoutState.mAvailable;
}

void LinearLayoutManager::layoutChunk(RecyclerView::Recycler& recycler, RecyclerView::State& state,
            LayoutState& layoutState, LayoutChunkResult& result) {
    View* view = layoutState.next(recycler);
    if (view == nullptr) {
        if (_Debug && layoutState.mScrapList.empty()){// == nullptr) {
            LOGE("received null view when unexpected");
        }
        // if we are laying out views in scrap, this may return null which means there is
        // no more items to layout.
        result.mFinished = true;
        return;
    }
    RecyclerView::LayoutParams* params = (RecyclerView::LayoutParams*) view->getLayoutParams();
    if (layoutState.mScrapList.empty()){// == nullptr) {
        if (mShouldReverseLayout == (layoutState.mLayoutDirection
                == LayoutState::LAYOUT_START)) {
            addView(view);
        } else {
            addView(view, 0);
        }
    } else {
        if (mShouldReverseLayout == (layoutState.mLayoutDirection
                == LayoutState::LAYOUT_START)) {
            addDisappearingView(view);
        } else {
            addDisappearingView(view, 0);
        }
    }
    measureChildWithMargins(view, 0, 0);
    result.mConsumed = mOrientationHelper->getDecoratedMeasurement(view);
    int left, top, right, bottom;
    if (mOrientation == VERTICAL) {
        if (isLayoutRTL()) {
            right = getWidth() - getPaddingRight();
            left = right - mOrientationHelper->getDecoratedMeasurementInOther(view);
        } else {
            left = getPaddingLeft();
            right = left + mOrientationHelper->getDecoratedMeasurementInOther(view);
        }
        if (layoutState.mLayoutDirection == LayoutState::LAYOUT_START) {
            bottom = layoutState.mOffset;
            top = layoutState.mOffset - result.mConsumed;
        } else {
            top = layoutState.mOffset;
            bottom = layoutState.mOffset + result.mConsumed;
        }
    } else {
        top = getPaddingTop();
        bottom = top + mOrientationHelper->getDecoratedMeasurementInOther(view);

        if (layoutState.mLayoutDirection == LayoutState::LAYOUT_START) {
            right = layoutState.mOffset;
            left = layoutState.mOffset - result.mConsumed;
        } else {
            left = layoutState.mOffset;
            right = layoutState.mOffset + result.mConsumed;
        }
    }
    // We calculate everything with View's bounding box (which includes decor and margins)
    // To calculate correct layout position, we subtract margins.
    layoutDecoratedWithMargins(view, left, top, right-left, bottom-top);
    LOGD_IF(_Debug,"laid out child at position %d,with(%d,%d,%d,%d)",getPosition(view),
                (left + params->leftMargin),(top + params->topMargin),
                (right - params->rightMargin),(bottom - params->bottomMargin));

    // Consume the available space if the view is not removed OR changed
    if (params->isItemRemoved() || params->isItemChanged()) {
        result.mIgnoreConsumed = true;
    }
    result.mFocusable = view->hasFocusable();
}

bool LinearLayoutManager::shouldMeasureTwice() {
    return getHeightMode() != MeasureSpec::EXACTLY
            && getWidthMode() != MeasureSpec::EXACTLY
            && hasFlexibleChildInBothOrientations();
}

int LinearLayoutManager::convertFocusDirectionToLayoutDirection(int focusDirection) {
    switch (focusDirection) {
    case View::FOCUS_BACKWARD:
        if (mOrientation == VERTICAL) {
            return LayoutState::LAYOUT_START;
        } else if (isLayoutRTL()) {
            return LayoutState::LAYOUT_END;
        } else {
            return LayoutState::LAYOUT_START;
        }
    case View::FOCUS_FORWARD:
        if (mOrientation == VERTICAL) {
            return LayoutState::LAYOUT_END;
        } else if (isLayoutRTL()) {
            return LayoutState::LAYOUT_START;
        } else {
            return LayoutState::LAYOUT_END;
        }
    case View::FOCUS_UP:
        return mOrientation == VERTICAL ? LayoutState::LAYOUT_START
                : LayoutState::INVALID_LAYOUT;
    case View::FOCUS_DOWN:
        return mOrientation == VERTICAL ? LayoutState::LAYOUT_END
                : LayoutState::INVALID_LAYOUT;
    case View::FOCUS_LEFT:
        return mOrientation == HORIZONTAL ? LayoutState::LAYOUT_START
                : LayoutState::INVALID_LAYOUT;
    case View::FOCUS_RIGHT:
        return mOrientation == HORIZONTAL ? LayoutState::LAYOUT_END
                : LayoutState::INVALID_LAYOUT;
    default:
        LOGD("Unknown focus request:%d",focusDirection);
        return LayoutState::INVALID_LAYOUT;
    }

}

View* LinearLayoutManager::getChildClosestToStart() {
    return getChildAt(mShouldReverseLayout ? getChildCount() - 1 : 0);
}

View* LinearLayoutManager::getChildClosestToEnd() {
    return getChildAt(mShouldReverseLayout ? 0 : getChildCount() - 1);
}

View* LinearLayoutManager::findFirstVisibleChildClosestToStart(bool completelyVisible, bool acceptPartiallyVisible) {
    if (mShouldReverseLayout) {
        return findOneVisibleChild(getChildCount() - 1, -1, completelyVisible,
                acceptPartiallyVisible);
    } else {
        return findOneVisibleChild(0, getChildCount(), completelyVisible,
                acceptPartiallyVisible);
    }
}

View* LinearLayoutManager::findFirstVisibleChildClosestToEnd(bool completelyVisible, bool acceptPartiallyVisible) {
    if (mShouldReverseLayout) {
        return findOneVisibleChild(0, getChildCount(), completelyVisible,
                acceptPartiallyVisible);
    } else {
        return findOneVisibleChild(getChildCount() - 1, -1, completelyVisible,
                acceptPartiallyVisible);
    }
}

View* LinearLayoutManager::findReferenceChildClosestToEnd(RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    return mShouldReverseLayout ? findFirstReferenceChild(recycler, state) :
            findLastReferenceChild(recycler, state);
}

View* LinearLayoutManager::findReferenceChildClosestToStart(RecyclerView::Recycler& recycler,RecyclerView::State& state) {
    return mShouldReverseLayout ? findLastReferenceChild(recycler, state) :
            findFirstReferenceChild(recycler, state);
}

View* LinearLayoutManager::findFirstReferenceChild(RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    return findReferenceChild(recycler, state, 0, getChildCount(), state.getItemCount());
}

View* LinearLayoutManager::findLastReferenceChild(RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    return findReferenceChild(recycler, state, getChildCount() - 1, -1, state.getItemCount());
}

View* LinearLayoutManager::findReferenceChild(RecyclerView::Recycler& recycler, RecyclerView::State& state,
        int start, int end, int itemCount) {
    ensureLayoutState();
    View* invalidMatch = nullptr;
    View* outOfBoundsMatch = nullptr;
    const int boundsStart = mOrientationHelper->getStartAfterPadding();
    const int boundsEnd = mOrientationHelper->getEndAfterPadding();
    const int diff = end > start ? 1 : -1;
    for (int i = start; i != end; i += diff) {
        View* view = getChildAt(i);
        const int position = getPosition(view);
        if (position >= 0 && position < itemCount) {
            if (((RecyclerView::LayoutParams*) view->getLayoutParams())->isItemRemoved()) {
                if (invalidMatch == nullptr) {
                    invalidMatch = view; // removed item, least preferred
                }
            } else if (mOrientationHelper->getDecoratedStart(view) >= boundsEnd
                    || mOrientationHelper->getDecoratedEnd(view) < boundsStart) {
                if (outOfBoundsMatch == nullptr) {
                    outOfBoundsMatch = view; // item is not visible, less preferred
                }
            } else {
                return view;
            }
        }
    }
    return outOfBoundsMatch != nullptr ? outOfBoundsMatch : invalidMatch;
}

View* LinearLayoutManager::findPartiallyOrCompletelyInvisibleChildClosestToEnd() {
    return mShouldReverseLayout ? findFirstPartiallyOrCompletelyInvisibleChild()
            : findLastPartiallyOrCompletelyInvisibleChild();
}

// returns the out-of-bound child view closest to RV's starting bounds. An out-of-bound child is
// defined as a child that's either partially or fully invisible (outside RV's padding area).
View* LinearLayoutManager::findPartiallyOrCompletelyInvisibleChildClosestToStart() {
    return mShouldReverseLayout ? findLastPartiallyOrCompletelyInvisibleChild() :
            findFirstPartiallyOrCompletelyInvisibleChild();
}

View* LinearLayoutManager::findFirstPartiallyOrCompletelyInvisibleChild() {
    return findOnePartiallyOrCompletelyInvisibleChild(0, getChildCount());
}

View* LinearLayoutManager::findLastPartiallyOrCompletelyInvisibleChild() {
    return findOnePartiallyOrCompletelyInvisibleChild(getChildCount() - 1, -1);
}

int LinearLayoutManager::findFirstVisibleItemPosition() {
    View* child = findOneVisibleChild(0, getChildCount(), false, true);
    return child == nullptr ? RecyclerView::NO_POSITION : getPosition(child);
}

int LinearLayoutManager::findFirstCompletelyVisibleItemPosition() {
    View* child = findOneVisibleChild(0, getChildCount(), true, false);
    return child == nullptr ? RecyclerView::NO_POSITION : getPosition(child);
}

int LinearLayoutManager::findLastVisibleItemPosition() {
    View* child = findOneVisibleChild(getChildCount() - 1, -1, false, true);
    return child == nullptr ? RecyclerView::NO_POSITION : getPosition(child);
}

int LinearLayoutManager::findLastCompletelyVisibleItemPosition() {
    View*child = findOneVisibleChild(getChildCount() - 1, -1, true, false);
    return child == nullptr ? RecyclerView::NO_POSITION : getPosition(child);
}

View* LinearLayoutManager::findOneVisibleChild(int fromIndex, int toIndex, bool completelyVisible,
        bool acceptPartiallyVisible) {
    ensureLayoutState();
    int preferredBoundsFlag = 0;/*ViewBoundsCheck.ViewBounds*/
    int acceptableBoundsFlag = 0;/*ViewBoundsCheck.ViewBounds*/
    if (completelyVisible) {
        preferredBoundsFlag = (ViewBoundsCheck::FLAG_CVS_GT_PVS | ViewBoundsCheck::FLAG_CVS_EQ_PVS
                | ViewBoundsCheck::FLAG_CVE_LT_PVE | ViewBoundsCheck::FLAG_CVE_EQ_PVE);
    } else {
        preferredBoundsFlag = (ViewBoundsCheck::FLAG_CVS_LT_PVE | ViewBoundsCheck::FLAG_CVE_GT_PVS);
    }
    if (acceptPartiallyVisible) {
        acceptableBoundsFlag = (ViewBoundsCheck::FLAG_CVS_LT_PVE | ViewBoundsCheck::FLAG_CVE_GT_PVS);
    }
    return (mOrientation == HORIZONTAL) ? mHorizontalBoundCheck
            ->findOneViewWithinBoundFlags(fromIndex, toIndex, preferredBoundsFlag,
                    acceptableBoundsFlag) : mVerticalBoundCheck
            ->findOneViewWithinBoundFlags(fromIndex, toIndex, preferredBoundsFlag,
                    acceptableBoundsFlag);
}

View* LinearLayoutManager::findOnePartiallyOrCompletelyInvisibleChild(int fromIndex, int toIndex) {
    ensureLayoutState();
    const int next = toIndex > fromIndex ? 1 : (toIndex < fromIndex ? -1 : 0);
    if (next == 0) {
        return getChildAt(fromIndex);
    }
    int preferredBoundsFlag = 0;/*ViewBoundsCheck.ViewBounds*/
    int acceptableBoundsFlag = 0;/*ViewBoundsCheck.ViewBounds*/
    if (mOrientationHelper->getDecoratedStart(getChildAt(fromIndex))
            < mOrientationHelper->getStartAfterPadding()) {
        preferredBoundsFlag = (ViewBoundsCheck::FLAG_CVS_LT_PVS | ViewBoundsCheck::FLAG_CVE_LT_PVE
                | ViewBoundsCheck::FLAG_CVE_GT_PVS);
        acceptableBoundsFlag = (ViewBoundsCheck::FLAG_CVS_LT_PVS | ViewBoundsCheck::FLAG_CVE_LT_PVE);
    } else {
        preferredBoundsFlag = (ViewBoundsCheck::FLAG_CVE_GT_PVE | ViewBoundsCheck::FLAG_CVS_GT_PVS
                | ViewBoundsCheck::FLAG_CVS_LT_PVE);
        acceptableBoundsFlag = (ViewBoundsCheck::FLAG_CVE_GT_PVE | ViewBoundsCheck::FLAG_CVS_GT_PVS);
    }
    return (mOrientation == HORIZONTAL) ? mHorizontalBoundCheck
            ->findOneViewWithinBoundFlags(fromIndex, toIndex, preferredBoundsFlag,
                    acceptableBoundsFlag) : mVerticalBoundCheck
            ->findOneViewWithinBoundFlags(fromIndex, toIndex, preferredBoundsFlag,
                    acceptableBoundsFlag);
}

View* LinearLayoutManager::onFocusSearchFailed(View* focused, int focusDirection,
        RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    resolveShouldLayoutReverse();
    if (getChildCount() == 0) {
        return nullptr;
    }

    const int layoutDir = convertFocusDirectionToLayoutDirection(focusDirection);
    if (layoutDir == LayoutState::INVALID_LAYOUT) {
        return nullptr;
    }
    ensureLayoutState();
    const int maxScroll = (int) (MAX_SCROLL_FACTOR * mOrientationHelper->getTotalSpace());
    updateLayoutState(layoutDir, maxScroll, false, state);
    mLayoutState->mScrollingOffset = LayoutState::SCROLLING_OFFSET_NaN;
    mLayoutState->mRecycle = false;
    fill(recycler, *mLayoutState, state, true);

    // nextCandidate is the first child view in the layout direction that's partially
    // within RV's bounds, i.e. part of it is visible or it's completely invisible but still
    // touching RV's bounds. This will be the unfocusable candidate view to become visible onto
    // the screen if no focusable views are found in the given layout direction.
    View* nextCandidate;
    if (layoutDir == LayoutState::LAYOUT_START) {
        nextCandidate = findPartiallyOrCompletelyInvisibleChildClosestToStart();
    } else {
        nextCandidate = findPartiallyOrCompletelyInvisibleChildClosestToEnd();
    }
    // nextFocus is meaningful only if it refers to a focusable child, in which case it
    // indicates the next view to gain focus.
    View* nextFocus;
    if (layoutDir == LayoutState::LAYOUT_START) {
        nextFocus = getChildClosestToStart();
    } else {
        nextFocus = getChildClosestToEnd();
    }
    if (nextFocus->hasFocusable()) {
        if (nextCandidate == nullptr) {
            return nullptr;
        }
        return nextFocus;
    }
    return nextCandidate;
}

void LinearLayoutManager::logChildren() {
    LOGD("internal representation of views on the screen");
    for (int i = 0; i < getChildCount(); i++) {
        View* child = getChildAt(i);
        LOGD("item %d,coord:%d",getPosition(child),mOrientationHelper->getDecoratedStart(child));
    }
    LOGD("==============");
}

/**
 * Used for debugging.
 * Validates that child views are laid out in correct order. This is important because rest of
 * the algorithm relies on this constraint.
 *
 * In default layout, child 0 should be closest to screen position 0 and last child should be
 * closest to position WIDTH or HEIGHT.
 * In reverse layout, last child should be closes to screen position 0 and first child should
 * be closest to position WIDTH  or HEIGHT
 */
void LinearLayoutManager::validateChildOrder() {
    LOGD("validating child count %d",getChildCount());
    if (getChildCount() < 1) {
        return;
    }
    int lastPos = getPosition(getChildAt(0));
    int lastScreenLoc = mOrientationHelper->getDecoratedStart(getChildAt(0));
    if (mShouldReverseLayout) {
        for (int i = 1; i < getChildCount(); i++) {
            View* child = getChildAt(i);
            int pos = getPosition(child);
            int screenLoc = mOrientationHelper->getDecoratedStart(child);
            if (pos < lastPos) {
                logChildren();
                LOGE("detected invalid position . loc invalid? "
                        + (screenLoc < lastScreenLoc));
            }
            if (screenLoc > lastScreenLoc) {
                logChildren();
                LOGE("detected invalid location");
            }
        }
    } else {
        for (int i = 1; i < getChildCount(); i++) {
            View* child = getChildAt(i);
            int pos = getPosition(child);
            int screenLoc = mOrientationHelper->getDecoratedStart(child);
            if (pos < lastPos) {
                logChildren();
                LOGE("detected invalid position. loc invalid? "
                        + (screenLoc < lastScreenLoc));
            }
            if (screenLoc < lastScreenLoc) {
                logChildren();
                LOGE("detected invalid location");
            }
        }
    }
}

bool LinearLayoutManager::supportsPredictiveItemAnimations() {
    return mPendingSavedState == nullptr && mLastStackFromEnd == mStackFromEnd;
}

/**
 * {@inheritDoc}
 */
// This method is only intended to be called (and should only ever be called) by
// ItemTouchHelper.
void LinearLayoutManager::prepareForDrop(View* view,View* target, int x, int y) {
    assertNotInLayoutOrScroll("Cannot drop a view during a scroll or layout calculation");
    ensureLayoutState();
    resolveShouldLayoutReverse();
    const int myPos = getPosition(view);
    const int targetPos = getPosition(target);
    const int dropDirection = myPos < targetPos ? LayoutState::ITEM_DIRECTION_TAIL
            : LayoutState::ITEM_DIRECTION_HEAD;
    if (mShouldReverseLayout) {
        if (dropDirection == LayoutState::ITEM_DIRECTION_TAIL) {
            scrollToPositionWithOffset(targetPos,
                    mOrientationHelper->getEndAfterPadding()
                            - (mOrientationHelper->getDecoratedStart(target)
                            + mOrientationHelper->getDecoratedMeasurement(view)));
        } else {
            scrollToPositionWithOffset(targetPos,
                    mOrientationHelper->getEndAfterPadding()
                            - mOrientationHelper->getDecoratedEnd(target));
        }
    } else {
        if (dropDirection == LayoutState::ITEM_DIRECTION_HEAD) {
            scrollToPositionWithOffset(targetPos, mOrientationHelper->getDecoratedStart(target));
        } else {
            scrollToPositionWithOffset(targetPos,
                    mOrientationHelper->getDecoratedEnd(target)
                            - mOrientationHelper->getDecoratedMeasurement(view));
        }
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LinearLayoutManager::LayoutState::hasMore(RecyclerView::State& state) {
    return mCurrentPosition >= 0 && mCurrentPosition < state.getItemCount();
}

View* LinearLayoutManager::LayoutState::next(RecyclerView::Recycler& recycler) {
    if (mScrapList.size()){// != nullptr) {
        return nextViewFromScrapList();
    }
    View* view = recycler.getViewForPosition(mCurrentPosition);
    mCurrentPosition += mItemDirection;
    return view;
}

View* LinearLayoutManager::LayoutState::nextViewFromScrapList() {
    const size_t size = mScrapList.size();
    for (size_t i = 0; i < size; i++) {
        View* view = mScrapList.at(i)->itemView;
        RecyclerView::LayoutParams* lp = (RecyclerView::LayoutParams*) view->getLayoutParams();
        if (lp->isItemRemoved()) {
            continue;
        }
        if (mCurrentPosition == lp->getViewLayoutPosition()) {
            assignPositionFromScrapList(view);
            return view;
        }
    }
    return nullptr;
}

void LinearLayoutManager::LayoutState::assignPositionFromScrapList() {
     assignPositionFromScrapList(nullptr);
}

void LinearLayoutManager::LayoutState::assignPositionFromScrapList(View* ignore) {
    View* closest = nextViewInLimitedList(ignore);
    if (closest == nullptr) {
        mCurrentPosition = RecyclerView::NO_POSITION;
    } else {
        mCurrentPosition = ((RecyclerView::LayoutParams*) closest->getLayoutParams())
                ->getViewLayoutPosition();
    }
}

View* LinearLayoutManager::LayoutState::nextViewInLimitedList(View* ignore) {
    const size_t size = mScrapList.size();
    View* closest = nullptr;
    int closestDistance = INT_MAX;
    if (_Debug && mIsPreLayout) {
        LOGE("Scrap list cannot be used in pre layout");
    }
    for (size_t i = 0; i < size; i++) {
        View* view = mScrapList.at(i)->itemView;
        RecyclerView::LayoutParams* lp = (RecyclerView::LayoutParams*) view->getLayoutParams();
        if (view == ignore || lp->isItemRemoved()) {
            continue;
        }
        const int distance = (lp->getViewLayoutPosition() - mCurrentPosition) * mItemDirection;
        if (distance < 0) {
            continue; // item is not in current direction
        }
        if (distance < closestDistance) {
            closest = view;
            closestDistance = distance;
            if (distance == 0) {
                break;
            }
        }
    }
    return closest;
}

void LinearLayoutManager::LayoutState::log() {
    LOGD("avail:%d, ind:%d,dir:%d,offset:%d,layoutDir:%d",mAvailable,mCurrentPosition
          ,mItemDirection, mOffset,mLayoutDirection);
}
/***************************************************************************************************/
LinearLayoutManager::SavedState::SavedState() {

}
LinearLayoutManager::SavedState::SavedState(Parcel& in) {
    mAnchorPosition = in.readInt();
    mAnchorOffset = in.readInt();
    mAnchorLayoutFromEnd = in.readInt() == 1;
}
LinearLayoutManager::SavedState::SavedState(SavedState& other) {
    mAnchorPosition = other.mAnchorPosition;
    mAnchorOffset = other.mAnchorOffset;
    mAnchorLayoutFromEnd = other.mAnchorLayoutFromEnd;
}
bool LinearLayoutManager::SavedState::hasValidAnchor()const{
    return mAnchorPosition >= 0;
}
void LinearLayoutManager::SavedState::invalidateAnchor() {
    mAnchorPosition = RecyclerView::NO_POSITION;
}
int LinearLayoutManager::SavedState::describeContents() {
    return 0;
}

void LinearLayoutManager::SavedState::writeToParcel(Parcel& dest, int flags) {
    dest.writeInt(mAnchorPosition);
    dest.writeInt(mAnchorOffset);
    dest.writeInt(mAnchorLayoutFromEnd ? 1 : 0);
}
/***************************************************************************************************/

LinearLayoutManager::AnchorInfo::AnchorInfo() {
    mOrientationHelper = nullptr;
    /*mOrientationHelper is owned/destroied by LinearLayoutManager*/
    reset();
}

void LinearLayoutManager::AnchorInfo::reset() {
    mPosition = RecyclerView::NO_POSITION;
    mCoordinate = INVALID_OFFSET;
    mLayoutFromEnd = false;
    mValid = false;
}

void LinearLayoutManager::AnchorInfo::assignCoordinateFromPadding() {
    mCoordinate = mLayoutFromEnd
            ? mOrientationHelper->getEndAfterPadding()
            : mOrientationHelper->getStartAfterPadding();
}

bool LinearLayoutManager::AnchorInfo::isViewValidAsAnchor(View* child, RecyclerView::State& state) {
    RecyclerView::LayoutParams* lp = (RecyclerView::LayoutParams*) child->getLayoutParams();
    return !lp->isItemRemoved() && lp->getViewLayoutPosition() >= 0
            && lp->getViewLayoutPosition() < state.getItemCount();
}

void LinearLayoutManager::AnchorInfo::assignFromViewAndKeepVisibleRect(View* child, int position) {
    const int spaceChange = mOrientationHelper->getTotalSpaceChange();
    if (spaceChange >= 0) {
        assignFromView(child, position);
        return;
    }
    mPosition = position;
    if (mLayoutFromEnd) {
        const int prevLayoutEnd = mOrientationHelper->getEndAfterPadding() - spaceChange;
        const int childEnd = mOrientationHelper->getDecoratedEnd(child);
        const int previousEndMargin = prevLayoutEnd - childEnd;
        mCoordinate = mOrientationHelper->getEndAfterPadding() - previousEndMargin;
        // ensure we did not push child's top out of bounds because of this
        if (previousEndMargin > 0) { // we have room to shift bottom if necessary
            const int childSize = mOrientationHelper->getDecoratedMeasurement(child);
            const int estimatedChildStart = mCoordinate - childSize;
            const int layoutStart = mOrientationHelper->getStartAfterPadding();
            const int previousStartMargin = mOrientationHelper->getDecoratedStart(child)
                    - layoutStart;
            const int startReference = layoutStart + std::min(previousStartMargin, 0);
            const int startMargin = estimatedChildStart - startReference;
            if (startMargin < 0) {
                // offset to make top visible but not too much
                mCoordinate += std::min(previousEndMargin, -startMargin);
            }
        }
    } else {
        const int childStart = mOrientationHelper->getDecoratedStart(child);
        const int startMargin = childStart - mOrientationHelper->getStartAfterPadding();
        mCoordinate = childStart;
        if (startMargin > 0) { // we have room to fix end as well
            const int estimatedEnd = childStart
                    + mOrientationHelper->getDecoratedMeasurement(child);
            const int previousLayoutEnd = mOrientationHelper->getEndAfterPadding()
                    - spaceChange;
            const int previousEndMargin = previousLayoutEnd
                    - mOrientationHelper->getDecoratedEnd(child);
            const int endReference = mOrientationHelper->getEndAfterPadding()
                    - std::min(0, previousEndMargin);
            const int endMargin = endReference - estimatedEnd;
            if (endMargin < 0) {
                mCoordinate -= std::min(startMargin, -endMargin);
            }
        }
    }
}

void LinearLayoutManager::AnchorInfo::assignFromView(View* child, int position) {
    if (mLayoutFromEnd) {
        mCoordinate = mOrientationHelper->getDecoratedEnd(child)
                + mOrientationHelper->getTotalSpaceChange();
    } else {
        mCoordinate = mOrientationHelper->getDecoratedStart(child);
    }

    mPosition = position;
}

/**********************************************************************************************/

void LinearLayoutManager::LayoutChunkResult::resetInternal() {
    mConsumed = 0;
    mFinished = false;
    mIgnoreConsumed = false;
    mFocusable = false;
}


}/*endof namespace*/
