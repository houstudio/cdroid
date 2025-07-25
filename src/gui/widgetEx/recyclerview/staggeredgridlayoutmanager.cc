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
#include <widgetEx/recyclerview/staggeredgridlayoutmanager.h>
#include <widgetEx/recyclerview/orientationhelper.h>
#include <widgetEx/recyclerview/layoutstate.h>
#include <widgetEx/recyclerview/scrollbarhelper.h>
#include <widgetEx/recyclerview/adapterhelper.h>
#include <widgetEx/recyclerview/linearsmoothscroller.h>
#include <cassert>
#if defined(_WIN32)||defined(_WIN64)||defined(_MSC_VER)
#include <intrin.h>
#endif

namespace cdroid{

StaggeredGridLayoutManager::StaggeredGridLayoutManager(Context* context,const AttributeSet& attrs)
	:LayoutManager(){//, int defStyleAttr,int defStyleRes) {
    Properties properties = getProperties(context, attrs,0,0);//, defStyleAttr, defStyleRes);
    initLayoutManager();
    setOrientation(properties.orientation);

    setSpanCount(properties.spanCount);
    setReverseLayout(properties.reverseLayout);
    createOrientationHelpers();
}

StaggeredGridLayoutManager::StaggeredGridLayoutManager(int spanCount, int orientation) {
    initLayoutManager();
    setOrientation(orientation);
    setSpanCount(spanCount);
    createOrientationHelpers();
}

StaggeredGridLayoutManager::~StaggeredGridLayoutManager(){
    delete mPendingSavedState;
    delete mPrimaryOrientation;
    delete mSecondaryOrientation;
    delete mRemainingSpans;
    delete mAnchorInfo;
    delete mLazySpanLookup;
    delete mLayoutState;
}

void StaggeredGridLayoutManager::initLayoutManager(){
    mPendingSavedState = nullptr;
    mPrimaryOrientation = nullptr;
    mSecondaryOrientation= nullptr;
    mRemainingSpans = nullptr;
    mAnchorInfo = new AnchorInfo(this);
    mLazySpanLookup = new LazySpanLookup;
    mLayoutState = new LayoutState();
    mCheckForGapsRunnable =[this](){
       checkForGaps();
    };
}

bool StaggeredGridLayoutManager::isAutoMeasureEnabled() const{
    return mGapStrategy != GAP_HANDLING_NONE;
}

void StaggeredGridLayoutManager::createOrientationHelpers() {
    mPrimaryOrientation = OrientationHelper::createOrientationHelper(this, mOrientation);
    mSecondaryOrientation = OrientationHelper::createOrientationHelper(this, 1 - mOrientation);
}

bool StaggeredGridLayoutManager::checkForGaps() {
    if (getChildCount() == 0 || mGapStrategy == GAP_HANDLING_NONE || !isAttachedToWindow()) {
        return false;
    }
    int minPos, maxPos;
    if (mShouldReverseLayout) {
        minPos = getLastChildPosition();
        maxPos = getFirstChildPosition();
    } else {
        minPos = getFirstChildPosition();
        maxPos = getLastChildPosition();
    }
    if (minPos == 0) {
        View* gapView = hasGapsToFix();
        if (gapView != nullptr) {
            mLazySpanLookup->clear();
            requestSimpleAnimationsInNextLayout();
            requestLayout();
            return true;
        }
    }
    if (!mLaidOutInvalidFullSpan) {
        return false;
    }
    int invalidGapDir = mShouldReverseLayout ? LayoutState::LAYOUT_START : LayoutState::LAYOUT_END;
    LazySpanLookup::FullSpanItem* invalidFsi = mLazySpanLookup->getFirstFullSpanItemInRange(minPos, maxPos + 1, invalidGapDir, true);
    if (invalidFsi == nullptr) {
        mLaidOutInvalidFullSpan = false;
        mLazySpanLookup->forceInvalidateAfter(maxPos + 1);
        return false;
    }
    LazySpanLookup::FullSpanItem* validFsi = mLazySpanLookup->getFirstFullSpanItemInRange(minPos, invalidFsi->mPosition,
                    invalidGapDir * -1, true);
    if (validFsi == nullptr) {
        mLazySpanLookup->forceInvalidateAfter(invalidFsi->mPosition);
    } else {
        mLazySpanLookup->forceInvalidateAfter(validFsi->mPosition + 1);
    }
    requestSimpleAnimationsInNextLayout();
    requestLayout();
    return true;
}

void StaggeredGridLayoutManager::onScrollStateChanged(int state) {
    if (state == RecyclerView::SCROLL_STATE_IDLE) {
        checkForGaps();
    }
}

void StaggeredGridLayoutManager::onDetachedFromWindow(RecyclerView& view, RecyclerView::Recycler& recycler) {
    LayoutManager::onDetachedFromWindow(view, recycler);

    removeCallbacks(mCheckForGapsRunnable);
    for (int i = 0; i < mSpanCount; i++) {
        mSpans[i]->clear();
    }
    // SGLM will require fresh layout call to recover state after detach
    view.requestLayout();
}

View* StaggeredGridLayoutManager::hasGapsToFix() {
    int startChildIndex = 0;
    int endChildIndex = getChildCount() - 1;
    BitSet64 mSpansToCheck(mSpanCount);// = new BitSet(mSpanCount);
    for(int i=0;i<mSpanCount;i++)mSpansToCheck.markBit(i);//mSpansToCheck.set(0, mSpanCount, true);

    int firstChildIndex, childLimit;
    const int preferredSpanDir = mOrientation == VERTICAL && isLayoutRTL() ? 1 : -1;

    if (mShouldReverseLayout) {
        firstChildIndex = endChildIndex;
        childLimit = startChildIndex - 1;
    } else {
        firstChildIndex = startChildIndex;
        childLimit = endChildIndex + 1;
    }
    const int nextChildDiff = firstChildIndex < childLimit ? 1 : -1;
    for (int i = firstChildIndex; i != childLimit; i += nextChildDiff) {
        View* child = getChildAt(i);
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
        if (mSpansToCheck.hasBit(lp->mSpan->mIndex)) {
            if (checkSpanForGap(*lp->mSpan)) {
                return child;
            }
            mSpansToCheck.clearBit(lp->mSpan->mIndex);
        }
        if (lp->mFullSpan) {
            continue; // quick reject
        }

        if (i + nextChildDiff != childLimit) {
            View* nextChild = getChildAt(i + nextChildDiff);
            bool compareSpans = false;
            if (mShouldReverseLayout) {
                // ensure child's end is below nextChild's end
                int myEnd = mPrimaryOrientation->getDecoratedEnd(child);
                int nextEnd = mPrimaryOrientation->getDecoratedEnd(nextChild);
                if (myEnd < nextEnd) {
                    return child; //i should have a better position
                } else if (myEnd == nextEnd) {
                    compareSpans = true;
                }
            } else {
                int myStart = mPrimaryOrientation->getDecoratedStart(child);
                int nextStart = mPrimaryOrientation->getDecoratedStart(nextChild);
                if (myStart > nextStart) {
                    return child; //i should have a better position
                } else if (myStart == nextStart) {
                    compareSpans = true;
                }
            }
            if (compareSpans) {
                // equal, check span indices.
                LayoutParams* nextLp = (LayoutParams*) nextChild->getLayoutParams();
                if (lp->mSpan->mIndex - nextLp->mSpan->mIndex < 0 != preferredSpanDir < 0) {
                    return child;
                }
            }
        }
    }
    // everything looks good
    return nullptr;
}

bool StaggeredGridLayoutManager::checkSpanForGap(Span& span) {
    if (mShouldReverseLayout) {
        if (span.getEndLine() < mPrimaryOrientation->getEndAfterPadding()) {
            // if it is full span, it is OK
            View* endView = span.mViews.at(span.mViews.size() - 1);
            LayoutParams* lp = span.getLayoutParams(endView);
            return !lp->mFullSpan;
        }
    } else if (span.getStartLine() > mPrimaryOrientation->getStartAfterPadding()) {
        // if it is full span, it is OK
        View* startView = span.mViews.at(0);
        LayoutParams* lp = span.getLayoutParams(startView);
        return !lp->mFullSpan;
    }
    return false;
}

void StaggeredGridLayoutManager::setSpanCount(int spanCount) {
    assertNotInLayoutOrScroll(std::string());
    if (spanCount != mSpanCount) {
        invalidateSpanAssignments();
        mSpanCount = spanCount;
        mRemainingSpans = new BitSet(mSpanCount);
        mSpans.resize(mSpanCount);// = new Span[mSpanCount];
        for (int i = 0; i < mSpanCount; i++) {
            mSpans[i] = new Span(this,i);
        }
        requestLayout();
    }
}

void StaggeredGridLayoutManager::setOrientation(int orientation) {
    if (orientation != HORIZONTAL && orientation != VERTICAL) {
        FATAL("invalid orientation.");
    }
    assertNotInLayoutOrScroll(std::string());
    if (orientation == mOrientation) {
        return;
    }
    mOrientation = orientation;
    OrientationHelper* tmp = mPrimaryOrientation;
    mPrimaryOrientation = mSecondaryOrientation;
    mSecondaryOrientation = tmp;
    requestLayout();
}

void StaggeredGridLayoutManager::setReverseLayout(bool reverseLayout) {
    assertNotInLayoutOrScroll(std::string());
    if (mPendingSavedState != nullptr && mPendingSavedState->mReverseLayout != reverseLayout) {
        mPendingSavedState->mReverseLayout = reverseLayout;
    }
    mReverseLayout = reverseLayout;
    requestLayout();
}

int StaggeredGridLayoutManager::getGapStrategy() {
    return mGapStrategy;
}

void StaggeredGridLayoutManager::setGapStrategy(int gapStrategy) {
    assertNotInLayoutOrScroll(std::string());
    if (gapStrategy == mGapStrategy) {
        return;
    }
    if (gapStrategy != GAP_HANDLING_NONE
            && gapStrategy != GAP_HANDLING_MOVE_ITEMS_BETWEEN_SPANS) {
        FATAL("invalid gap strategy. Must be GAP_HANDLING_NONE "
                "or GAP_HANDLING_MOVE_ITEMS_BETWEEN_SPANS");
    }
    mGapStrategy = gapStrategy;
    requestLayout();
}

void StaggeredGridLayoutManager::assertNotInLayoutOrScroll(const std::string& message) {
    if (mPendingSavedState == nullptr) {
	    LayoutManager::assertNotInLayoutOrScroll(message);
    }
}

int StaggeredGridLayoutManager::getSpanCount() {
    return mSpanCount;
}

void StaggeredGridLayoutManager::invalidateSpanAssignments() {
    mLazySpanLookup->clear();
    requestLayout();
}

void StaggeredGridLayoutManager::resolveShouldLayoutReverse() {
    // A == B is the same result, but we rather keep it readable
    if (mOrientation == VERTICAL || !isLayoutRTL()) {
        mShouldReverseLayout = mReverseLayout;
    } else {
        mShouldReverseLayout = !mReverseLayout;
    }
}

bool StaggeredGridLayoutManager::isLayoutRTL() {
    return getLayoutDirection() == View::LAYOUT_DIRECTION_RTL;
}

bool StaggeredGridLayoutManager::getReverseLayout() {
    return mReverseLayout;
}

void StaggeredGridLayoutManager::setMeasuredDimension(Rect& childrenBounds, int wSpec, int hSpec) {
    // we don't like it to wrap content in our non-scroll direction.
    int width, height;
    const int horizontalPadding = getPaddingLeft() + getPaddingRight();
    const int verticalPadding = getPaddingTop() + getPaddingBottom();
    if (mOrientation == VERTICAL) {
        const int usedHeight = childrenBounds.height + verticalPadding;
        height = chooseSize(hSpec, usedHeight, getMinimumHeight());
        width = chooseSize(wSpec, mSizePerSpan * mSpanCount + horizontalPadding,
                getMinimumWidth());
    } else {
        const int usedWidth = childrenBounds.width + horizontalPadding;
        width = chooseSize(wSpec, usedWidth, getMinimumWidth());
        height = chooseSize(hSpec, mSizePerSpan * mSpanCount + verticalPadding,
                getMinimumHeight());
    }
    LayoutManager::setMeasuredDimension(width, height);
}

void StaggeredGridLayoutManager::onLayoutChildren(RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    onLayoutChildren(recycler, state, true);
}

void StaggeredGridLayoutManager::onAdapterChanged(RecyclerView::Adapter* oldAdapter,
        RecyclerView::Adapter* newAdapter) {
    // RV will remove all views so we should clear all spans and assignments of views into spans
    mLazySpanLookup->clear();
    for (int i = 0; i < mSpanCount; i++) {
        mSpans[i]->clear();
    }
}

void StaggeredGridLayoutManager::onLayoutChildren(RecyclerView::Recycler& recycler, RecyclerView::State& state,
        bool shouldCheckForGaps) {
    AnchorInfo* anchorInfo = mAnchorInfo;
    if (mPendingSavedState != nullptr || mPendingScrollPosition != RecyclerView::NO_POSITION) {
        if (state.getItemCount() == 0) {
            removeAndRecycleAllViews(recycler);
            anchorInfo->reset();
            return;
        }
    }

    bool recalculateAnchor = !anchorInfo->mValid || mPendingScrollPosition != RecyclerView::NO_POSITION
            || mPendingSavedState != nullptr;
    if (recalculateAnchor) {
        anchorInfo->reset();
        if (mPendingSavedState != nullptr) {
            applyPendingSavedState(*anchorInfo);
        } else {
            resolveShouldLayoutReverse();
            anchorInfo->mLayoutFromEnd = mShouldReverseLayout;
        }
        updateAnchorInfoForLayout(state, *anchorInfo);
        anchorInfo->mValid = true;
    }
    if (mPendingSavedState == nullptr && mPendingScrollPosition == RecyclerView::NO_POSITION) {
        if (anchorInfo->mLayoutFromEnd != mLastLayoutFromEnd
                || isLayoutRTL() != mLastLayoutRTL) {
            mLazySpanLookup->clear();
            anchorInfo->mInvalidateOffsets = true;
        }
    }

    if (getChildCount() > 0 && (mPendingSavedState == nullptr
            || mPendingSavedState->mSpanOffsetsSize < 1)) {
        if (anchorInfo->mInvalidateOffsets) {
            for (int i = 0; i < mSpanCount; i++) {
                // Scroll to position is set, clear.
                mSpans[i]->clear();
                if (anchorInfo->mOffset != INVALID_OFFSET) {
                    mSpans[i]->setLine(anchorInfo->mOffset);
                }
            }
        } else {
            if (recalculateAnchor || mAnchorInfo->mSpanReferenceLines.empty()) {
                for (int i = 0; i < mSpanCount; i++) {
                    mSpans[i]->cacheReferenceLineAndClear(mShouldReverseLayout,
                            anchorInfo->mOffset);
                }
                mAnchorInfo->saveSpanReferenceLines(mSpans);
            } else {
                for (int i = 0; i < mSpanCount; i++) {
                    Span* span = mSpans[i];
                    span->clear();
                    span->setLine(mAnchorInfo->mSpanReferenceLines[i]);
                }
            }
        }
    }
    detachAndScrapAttachedViews(recycler);
    mLayoutState->mRecycle = false;
    mLaidOutInvalidFullSpan = false;
    updateMeasureSpecs(mSecondaryOrientation->getTotalSpace());
    updateLayoutState(anchorInfo->mPosition, state);
    if (anchorInfo->mLayoutFromEnd) {
        // Layout start.
        setLayoutStateDirection(LayoutState::LAYOUT_START);
        fill(recycler, *mLayoutState, state);
        // Layout end.
        setLayoutStateDirection(LayoutState::LAYOUT_END);
        mLayoutState->mCurrentPosition = anchorInfo->mPosition + mLayoutState->mItemDirection;
        fill(recycler, *mLayoutState, state);
    } else {
        // Layout end.
        setLayoutStateDirection(LayoutState::LAYOUT_END);
        fill(recycler, *mLayoutState, state);
        // Layout start.
        setLayoutStateDirection(LayoutState::LAYOUT_START);
        mLayoutState->mCurrentPosition = anchorInfo->mPosition + mLayoutState->mItemDirection;
        fill(recycler, *mLayoutState, state);
    }

    repositionToWrapContentIfNecessary();

    if (getChildCount() > 0) {
        if (mShouldReverseLayout) {
            fixEndGap(recycler, state, true);
            fixStartGap(recycler, state, false);
        } else {
            fixStartGap(recycler, state, true);
            fixEndGap(recycler, state, false);
        }
    }
    bool hasGaps = false;
    if (shouldCheckForGaps && !state.isPreLayout()) {
        const bool needToCheckForGaps = mGapStrategy != GAP_HANDLING_NONE
                && getChildCount() > 0 && (mLaidOutInvalidFullSpan || hasGapsToFix() != nullptr);
        if (needToCheckForGaps) {
            removeCallbacks(mCheckForGapsRunnable);
            if (checkForGaps()) {
                hasGaps = true;
            }
        }
    }
    if (state.isPreLayout()) {
        mAnchorInfo->reset();
    }
    mLastLayoutFromEnd = anchorInfo->mLayoutFromEnd;
    mLastLayoutRTL = isLayoutRTL();
    if (hasGaps) {
        mAnchorInfo->reset();
        onLayoutChildren(recycler, state, false);
    }
}

void StaggeredGridLayoutManager::onLayoutCompleted(RecyclerView::State& state) {
    LayoutManager::onLayoutCompleted(state);
    mPendingScrollPosition = RecyclerView::NO_POSITION;
    mPendingScrollPositionOffset = INVALID_OFFSET;
    mPendingSavedState = nullptr; // we don't need this anymore
    mAnchorInfo->reset();
}

void StaggeredGridLayoutManager::repositionToWrapContentIfNecessary() {
    if (mSecondaryOrientation->getMode() == MeasureSpec::EXACTLY) {
        return; // nothing to do
    }
    float maxSize = 0;
    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        float size = static_cast<float>(mSecondaryOrientation->getDecoratedMeasurement(child));
        if (size < maxSize) {
            continue;
        }
        LayoutParams* layoutParams = (LayoutParams*) child->getLayoutParams();
        if (layoutParams->isFullSpan()) {
            size = 1.f * size / mSpanCount;
        }
        maxSize = std::max(maxSize, size);
    }
    int before = mSizePerSpan;
    int desired = static_cast<int>(std::round(maxSize * mSpanCount));
    if (mSecondaryOrientation->getMode() == MeasureSpec::AT_MOST) {
        desired = std::min(desired, mSecondaryOrientation->getTotalSpace());
    }
    updateMeasureSpecs(desired);
    if (mSizePerSpan == before) {
        return; // nothing has changed
    }
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        const LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
        if (lp->mFullSpan) {
            continue;
        }
        if (isLayoutRTL() && mOrientation == VERTICAL) {
            const int newOffset = -(mSpanCount - 1 - lp->mSpan->mIndex) * mSizePerSpan;
            const int prevOffset = -(mSpanCount - 1 - lp->mSpan->mIndex) * before;
            child->offsetLeftAndRight(newOffset - prevOffset);
        } else {
            const int newOffset = lp->mSpan->mIndex * mSizePerSpan;
            const int prevOffset = lp->mSpan->mIndex * before;
            if (mOrientation == VERTICAL) {
                child->offsetLeftAndRight(newOffset - prevOffset);
            } else {
                child->offsetTopAndBottom(newOffset - prevOffset);
            }
        }
    }
}

void StaggeredGridLayoutManager::applyPendingSavedState(AnchorInfo& anchorInfo) {
    LOGD_IF(_Debug,"found saved state: %p",mPendingSavedState);
    if (mPendingSavedState->mSpanOffsetsSize > 0) {
        if (mPendingSavedState->mSpanOffsetsSize == mSpanCount) {
            for (int i = 0; i < mSpanCount; i++) {
                mSpans[i]->clear();
                int line = mPendingSavedState->mSpanOffsets[i];
                if (line != Span::INVALID_LINE) {
                    if (mPendingSavedState->mAnchorLayoutFromEnd) {
                        line += mPrimaryOrientation->getEndAfterPadding();
                    } else {
                        line += mPrimaryOrientation->getStartAfterPadding();
                    }
                }
                mSpans[i]->setLine(line);
            }
        } else {
            mPendingSavedState->invalidateSpanInfo();
            mPendingSavedState->mAnchorPosition = mPendingSavedState->mVisibleAnchorPosition;
        }
    }
    mLastLayoutRTL = mPendingSavedState->mLastLayoutRTL;
    setReverseLayout(mPendingSavedState->mReverseLayout);
    resolveShouldLayoutReverse();

    if (mPendingSavedState->mAnchorPosition != RecyclerView::NO_POSITION) {
        mPendingScrollPosition = mPendingSavedState->mAnchorPosition;
        anchorInfo.mLayoutFromEnd = mPendingSavedState->mAnchorLayoutFromEnd;
    } else {
        anchorInfo.mLayoutFromEnd = mShouldReverseLayout;
    }
    if (mPendingSavedState->mSpanLookupSize > 1) {
        mLazySpanLookup->mData = mPendingSavedState->mSpanLookup;
        mLazySpanLookup->mFullSpanItems = mPendingSavedState->mFullSpanItems;
    }
}

void StaggeredGridLayoutManager::updateAnchorInfoForLayout(RecyclerView::State& state, AnchorInfo& anchorInfo) {
    if (updateAnchorFromPendingData(state, anchorInfo)) {
        return;
    }
    if (updateAnchorFromChildren(state, anchorInfo)) {
        return;
    }
    LOGD_IF(_Debug,"Deciding anchor info from fresh state");
    anchorInfo.assignCoordinateFromPadding();
    anchorInfo.mPosition = 0;
}

bool StaggeredGridLayoutManager::updateAnchorFromChildren(RecyclerView::State& state, AnchorInfo& anchorInfo) {
    // We don't recycle views out of adapter order. This way, we can rely on the first or
    // last child as the anchor position.
    // Layout direction may change but we should select the child depending on the latest
    // layout direction. Otherwise, we'll choose the wrong child.
    anchorInfo.mPosition = mLastLayoutFromEnd
            ? findLastReferenceChildPosition(state.getItemCount())
            : findFirstReferenceChildPosition(state.getItemCount());
    anchorInfo.mOffset = INVALID_OFFSET;
    return true;
}

bool StaggeredGridLayoutManager::updateAnchorFromPendingData(RecyclerView::State& state, AnchorInfo& anchorInfo) {
    // Validate scroll position if exists.
    if (state.isPreLayout() || mPendingScrollPosition == RecyclerView::NO_POSITION) {
        return false;
    }
    // Validate it.
    if (mPendingScrollPosition < 0 || mPendingScrollPosition >= state.getItemCount()) {
        mPendingScrollPosition = RecyclerView::NO_POSITION;
        mPendingScrollPositionOffset = INVALID_OFFSET;
        return false;
    }

    if (mPendingSavedState == nullptr || mPendingSavedState->mAnchorPosition == RecyclerView::NO_POSITION
            || mPendingSavedState->mSpanOffsetsSize < 1) {
        // If item is visible, make it fully visible.
        View* child = findViewByPosition(mPendingScrollPosition);
        if (child != nullptr) {
            // Use regular anchor position, offset according to pending offset and target
            // child
            anchorInfo.mPosition = mShouldReverseLayout ? getLastChildPosition()
                    : getFirstChildPosition();
            if (mPendingScrollPositionOffset != INVALID_OFFSET) {
                if (anchorInfo.mLayoutFromEnd) {
                    const int target = mPrimaryOrientation->getEndAfterPadding()
                            - mPendingScrollPositionOffset;
                    anchorInfo.mOffset = target - mPrimaryOrientation->getDecoratedEnd(child);
                } else {
                    const int target = mPrimaryOrientation->getStartAfterPadding()
                            + mPendingScrollPositionOffset;
                    anchorInfo.mOffset = target - mPrimaryOrientation->getDecoratedStart(child);
                }
                return true;
            }

            // no offset provided. Decide according to the child location
            const int childSize = mPrimaryOrientation->getDecoratedMeasurement(child);
            if (childSize > mPrimaryOrientation->getTotalSpace()) {
                // Item does not fit. Fix depending on layout direction.
                anchorInfo.mOffset = anchorInfo.mLayoutFromEnd
                        ? mPrimaryOrientation->getEndAfterPadding()
                        : mPrimaryOrientation->getStartAfterPadding();
                return true;
            }

            const int startGap = mPrimaryOrientation->getDecoratedStart(child)
                    - mPrimaryOrientation->getStartAfterPadding();
            if (startGap < 0) {
                anchorInfo.mOffset = -startGap;
                return true;
            }
            const int endGap = mPrimaryOrientation->getEndAfterPadding()
                    - mPrimaryOrientation->getDecoratedEnd(child);
            if (endGap < 0) {
                anchorInfo.mOffset = endGap;
                return true;
            }
            // child already visible. just layout as usual
            anchorInfo.mOffset = INVALID_OFFSET;
        } else {
            // Child is not visible. Set anchor coordinate depending on in which direction
            // child will be visible.
            anchorInfo.mPosition = mPendingScrollPosition;
            if (mPendingScrollPositionOffset == INVALID_OFFSET) {
                const int position = calculateScrollDirectionForPosition(
                        anchorInfo.mPosition);
                anchorInfo.mLayoutFromEnd = position == LayoutState::LAYOUT_END;
                anchorInfo.assignCoordinateFromPadding();
            } else {
                anchorInfo.assignCoordinateFromPadding(mPendingScrollPositionOffset);
            }
            anchorInfo.mInvalidateOffsets = true;
        }
    } else {
        anchorInfo.mOffset = INVALID_OFFSET;
        anchorInfo.mPosition = mPendingScrollPosition;
    }
    return true;
}

void StaggeredGridLayoutManager::updateMeasureSpecs(int totalSpace) {
    mSizePerSpan = totalSpace / mSpanCount;
    //noinspection ResourceType
    mFullSizeSpec = MeasureSpec::makeMeasureSpec(
            totalSpace, mSecondaryOrientation->getMode());
}

bool StaggeredGridLayoutManager::supportsPredictiveItemAnimations() {
    return mPendingSavedState == nullptr;
}

int StaggeredGridLayoutManager::findFirstVisibleItemPositions(std::vector<int>& into) {
    into.resize(mSpanCount);
    for (int i = 0; i < mSpanCount; i++) {
        into[i] = mSpans[i]->findFirstVisibleItemPosition();
    }
    return mSpanCount;
}

int StaggeredGridLayoutManager::findFirstCompletelyVisibleItemPositions(std::vector<int>& into) {
    into.resize(mSpanCount);
    for (int i = 0; i < mSpanCount; i++) {
        into[i] = mSpans[i]->findFirstCompletelyVisibleItemPosition();
    }
    return mSpanCount;
}

int StaggeredGridLayoutManager::findLastVisibleItemPositions(std::vector<int>& into) {
    into.resize(mSpanCount);
    for (int i = 0; i < mSpanCount; i++) {
        into[i] = mSpans[i]->findLastVisibleItemPosition();
    }
    return mSpanCount;
}

int StaggeredGridLayoutManager::findLastCompletelyVisibleItemPositions(std::vector<int>&into) {
    into.resize(mSpanCount);
    for (int i = 0; i < mSpanCount; i++) {
        into[i] = mSpans[i]->findLastCompletelyVisibleItemPosition();
    }
    return mSpanCount;
}

int StaggeredGridLayoutManager::computeHorizontalScrollOffset(RecyclerView::State& state) {
    return computeScrollOffset(state);
}

int StaggeredGridLayoutManager::computeScrollOffset(RecyclerView::State& state) {
    if (getChildCount() == 0) {
        return 0;
    }
    return ScrollbarHelper::computeScrollOffset(state, *mPrimaryOrientation,
            findFirstVisibleItemClosestToStart(!mSmoothScrollbarEnabled),
            findFirstVisibleItemClosestToEnd(!mSmoothScrollbarEnabled),
            *this, mSmoothScrollbarEnabled, mShouldReverseLayout);
}

int StaggeredGridLayoutManager::computeVerticalScrollOffset(RecyclerView::State& state) {
    return computeScrollOffset(state);
}

int StaggeredGridLayoutManager::computeHorizontalScrollExtent(RecyclerView::State& state) {
    return computeScrollExtent(state);
}

int StaggeredGridLayoutManager::computeScrollExtent(RecyclerView::State& state) {
    if (getChildCount() == 0) {
        return 0;
    }
    return ScrollbarHelper::computeScrollExtent(state, *mPrimaryOrientation,
            findFirstVisibleItemClosestToStart(!mSmoothScrollbarEnabled),
            findFirstVisibleItemClosestToEnd(!mSmoothScrollbarEnabled),
            *this, mSmoothScrollbarEnabled);
}

int StaggeredGridLayoutManager::computeVerticalScrollExtent(RecyclerView::State& state) {
    return computeScrollExtent(state);
}

int StaggeredGridLayoutManager::computeHorizontalScrollRange(RecyclerView::State& state) {
    return computeScrollRange(state);
}

int StaggeredGridLayoutManager::computeScrollRange(RecyclerView::State& state) {
    if (getChildCount() == 0) {
        return 0;
    }
    return ScrollbarHelper::computeScrollRange(state, *mPrimaryOrientation,
            findFirstVisibleItemClosestToStart(!mSmoothScrollbarEnabled),
            findFirstVisibleItemClosestToEnd(!mSmoothScrollbarEnabled),
            *this, mSmoothScrollbarEnabled);
}

int StaggeredGridLayoutManager::computeVerticalScrollRange(RecyclerView::State& state) {
    return computeScrollRange(state);
}

void StaggeredGridLayoutManager::measureChildWithDecorationsAndMargin(View* child, LayoutParams& lp,
        bool alreadyMeasured) {
    if (lp.mFullSpan) {
        if (mOrientation == VERTICAL) {
            measureChildWithDecorationsAndMargin(child, mFullSizeSpec,
                    getChildMeasureSpec( getHeight(), getHeightMode(),
                        getPaddingTop() + getPaddingBottom(), lp.height, true),
                    alreadyMeasured);
        } else {
            measureChildWithDecorationsAndMargin(child,
	            getChildMeasureSpec( getWidth(), getWidthMode(),
                            getPaddingLeft() + getPaddingRight(), lp.width, true),
                    mFullSizeSpec, alreadyMeasured);
        }
    } else {
        if (mOrientation == VERTICAL) {
            // Padding for width measure spec is 0 because left and right padding were already
            // factored into mSizePerSpan.
            measureChildWithDecorationsAndMargin( child,
                    getChildMeasureSpec( mSizePerSpan, getWidthMode(), 0,lp.width,false),
                    getChildMeasureSpec( getHeight(),  getHeightMode(), getPaddingTop() + getPaddingBottom(), lp.height,true),
		    alreadyMeasured );
        } else {
            // Padding for height measure spec is 0 because top and bottom padding were already
            // factored into mSizePerSpan.
            measureChildWithDecorationsAndMargin( child,
                    getChildMeasureSpec( getWidth(), getWidthMode(), getPaddingLeft() + getPaddingRight(),  lp.width, true),
                    getChildMeasureSpec( mSizePerSpan, getHeightMode(), 0,lp.height, false),
		    alreadyMeasured );
        }
    }
}

void StaggeredGridLayoutManager::measureChildWithDecorationsAndMargin(View* child, int widthSpec,
        int heightSpec, bool alreadyMeasured) {
    calculateItemDecorationsForChild(child, mTmpRect);
    LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
    widthSpec = updateSpecWithExtra(widthSpec, lp->leftMargin + mTmpRect.left,
            lp->rightMargin + mTmpRect.right());
    heightSpec = updateSpecWithExtra(heightSpec, lp->topMargin + mTmpRect.top,
            lp->bottomMargin + mTmpRect.bottom());
    const bool measure = alreadyMeasured
            ? shouldReMeasureChild(child, widthSpec, heightSpec, lp)
            : shouldMeasureChild(child, widthSpec, heightSpec, lp);
    if (measure) {
        child->measure(widthSpec, heightSpec);
    }

}

int StaggeredGridLayoutManager::updateSpecWithExtra(int spec, int startInset, int endInset) {
    if (startInset == 0 && endInset == 0) {
        return spec;
    }
    const int mode = MeasureSpec::getMode(spec);
    if (mode == MeasureSpec::AT_MOST || mode == MeasureSpec::EXACTLY) {
        return MeasureSpec::makeMeasureSpec(
                std::max(0, MeasureSpec::getSize(spec) - startInset - endInset), mode);
    }
    return spec;
}

void StaggeredGridLayoutManager::onRestoreInstanceState(Parcelable& state) {
    if (dynamic_cast<SavedState*>(&state)) {
        mPendingSavedState =(SavedState*) &state;
        if (mPendingScrollPosition != RecyclerView::NO_POSITION) {
            mPendingSavedState->invalidateAnchorPositionInfo();
            mPendingSavedState->invalidateSpanInfo();
        }        
        requestLayout();
    } else if (_Debug) {
        LOGD("invalid saved state class");
    }
}

Parcelable* StaggeredGridLayoutManager::onSaveInstanceState() {
#if 0
    if (mPendingSavedState != nullptr) {
        return new SavedState(mPendingSavedState);
    }
    SavedState state = new SavedState();
    state.mReverseLayout = mReverseLayout;
    state.mAnchorLayoutFromEnd = mLastLayoutFromEnd;
    state.mLastLayoutRTL = mLastLayoutRTL;

    if (mLazySpanLookup != nullptr && mLazySpanLookup.mData.size()){// != null) {
        state.mSpanLookup = mLazySpanLookup.mData;
        state.mSpanLookupSize = state.mSpanLookup.length;
        state.mFullSpanItems = mLazySpanLookup.mFullSpanItems;
    } else {
        state.mSpanLookupSize = 0;
    }

    if (getChildCount() > 0) {
        state.mAnchorPosition = mLastLayoutFromEnd ? getLastChildPosition()
                : getFirstChildPosition();
        state.mVisibleAnchorPosition = findFirstVisibleItemPositionInt();
        state.mSpanOffsetsSize = mSpanCount;
        state.mSpanOffsets.resize(mSpanCount);// = new int[mSpanCount];
        for (int i = 0; i < mSpanCount; i++) {
            int line;
            if (mLastLayoutFromEnd) {
                line = mSpans[i]->getEndLine(Span.INVALID_LINE);
                if (line != Span::INVALID_LINE) {
                    line -= mPrimaryOrientation->getEndAfterPadding();
                }
            } else {
                line = mSpans[i]->getStartLine(Span.INVALID_LINE);
                if (line != Span::INVALID_LINE) {
                    line -= mPrimaryOrientation->getStartAfterPadding();
                }
            }
            state.mSpanOffsets[i] = line;
        }
    } else {
        state.mAnchorPosition = RecyclerView::NO_POSITION;
        state.mVisibleAnchorPosition = RecyclerView::NO_POSITION;
        state.mSpanOffsetsSize = 0;
    }
    LOGD_IF(_Debug,"saved state:%p" ,state);
    return state;
#else
    return nullptr;
#endif
}
#if 0
void StaggeredGridLayoutManager::onInitializeAccessibilityNodeInfoForItem(RecyclerView::Recycler& recycler,
        RecyclerView::State& state, View host, AccessibilityNodeInfoCompat info) {
    ViewGroup::LayoutParams lp = host.getLayoutParams();
    if (!(lp instanceof LayoutParams)) {
        super.onInitializeAccessibilityNodeInfoForItem(host, info);
        return;
    }
    LayoutParams sglp = (LayoutParams) lp;
    if (mOrientation == HORIZONTAL) {
        info.setCollectionItemInfo(AccessibilityNodeInfoCompat.CollectionItemInfoCompat.obtain(
                sglp.getSpanIndex(), sglp.mFullSpan ? mSpanCount : 1,
                -1, -1,
                sglp.mFullSpan, false));
    } else { // VERTICAL
        info.setCollectionItemInfo(AccessibilityNodeInfoCompat.CollectionItemInfoCompat.obtain(
                -1, -1,
                sglp.getSpanIndex(), sglp.mFullSpan ? mSpanCount : 1,
                sglp.mFullSpan, false));
    }
}

void StaggeredGridLayoutManager::onInitializeAccessibilityEvent(AccessibilityEvent event) {
    super.onInitializeAccessibilityEvent(event);
    if (getChildCount() > 0) {
        final View start = findFirstVisibleItemClosestToStart(false);
        final View end = findFirstVisibleItemClosestToEnd(false);
        if (start == null || end == null) {
            return;
        }
        final int startPos = getPosition(start);
        final int endPos = getPosition(end);
        if (startPos < endPos) {
            event.setFromIndex(startPos);
            event.setToIndex(endPos);
        } else {
            event.setFromIndex(endPos);
            event.setToIndex(startPos);
        }
    }
}
#endif
int StaggeredGridLayoutManager::findFirstVisibleItemPositionInt() {
    View* first = mShouldReverseLayout ? findFirstVisibleItemClosestToEnd(true) :
            findFirstVisibleItemClosestToStart(true);
    return first == nullptr ? RecyclerView::NO_POSITION : getPosition(first);
}

int StaggeredGridLayoutManager::getRowCountForAccessibility(RecyclerView::Recycler& recycler,
        RecyclerView::State& state) {
    if (mOrientation == HORIZONTAL) {
        return mSpanCount;
    }
    return LayoutManager::getRowCountForAccessibility(recycler, state);
}

int StaggeredGridLayoutManager::getColumnCountForAccessibility(RecyclerView::Recycler& recycler,
        RecyclerView::State& state) {
    if (mOrientation == VERTICAL) {
        return mSpanCount;
    }
    return LayoutManager::getColumnCountForAccessibility(recycler, state);
}

View* StaggeredGridLayoutManager::findFirstVisibleItemClosestToStart(bool fullyVisible) {
    const int boundsStart = mPrimaryOrientation->getStartAfterPadding();
    const int boundsEnd = mPrimaryOrientation->getEndAfterPadding();
    const int limit = getChildCount();
    View* partiallyVisible = nullptr;
    for (int i = 0; i < limit; i++) {
        View* child = getChildAt(i);
        const int childStart = mPrimaryOrientation->getDecoratedStart(child);
        const int childEnd = mPrimaryOrientation->getDecoratedEnd(child);
        if (childEnd <= boundsStart || childStart >= boundsEnd) {
            continue; // not visible at all
        }
        if (childStart >= boundsStart || !fullyVisible) {
            // when checking for start, it is enough even if part of the child's top is visible
            // as long as fully visible is not requested.
            return child;
        }
        if (partiallyVisible == nullptr) {
            partiallyVisible = child;
        }
    }
    return partiallyVisible;
}

View* StaggeredGridLayoutManager::findFirstVisibleItemClosestToEnd(bool fullyVisible) {
    const int boundsStart = mPrimaryOrientation->getStartAfterPadding();
    const int boundsEnd = mPrimaryOrientation->getEndAfterPadding();
    View* partiallyVisible = nullptr;
    for (int i = getChildCount() - 1; i >= 0; i--) {
        View* child = getChildAt(i);
        const int childStart = mPrimaryOrientation->getDecoratedStart(child);
        const int childEnd = mPrimaryOrientation->getDecoratedEnd(child);
        if (childEnd <= boundsStart || childStart >= boundsEnd) {
            continue; // not visible at all
        }
        if (childEnd <= boundsEnd || !fullyVisible) {
            // when checking for end, it is enough even if part of the child's bottom is visible
            // as long as fully visible is not requested.
            return child;
        }
        if (partiallyVisible == nullptr) {
            partiallyVisible = child;
        }
    }
    return partiallyVisible;
}

void StaggeredGridLayoutManager::fixEndGap(RecyclerView::Recycler& recycler, RecyclerView::State& state,
        bool canOffsetChildren) {
    const int maxEndLine = getMaxEnd(INT_MIN);//Integer.MIN_VALUE);
    if (maxEndLine == INT_MIN){//Integer.MIN_VALUE) {
        return;
    }
    int gap = mPrimaryOrientation->getEndAfterPadding() - maxEndLine;
    int fixOffset;
    if (gap > 0) {
        fixOffset = -scrollBy(-gap, recycler, state);
    } else {
        return; // nothing to fix
    }
    gap -= fixOffset;
    if (canOffsetChildren && gap > 0) {
        mPrimaryOrientation->offsetChildren(gap);
    }
}

void StaggeredGridLayoutManager::fixStartGap(RecyclerView::Recycler& recycler, RecyclerView::State& state,
        bool canOffsetChildren) {
    const int minStartLine = getMinStart(INT_MAX);//Integer.MAX_VALUE);
    if (minStartLine == INT_MAX){//Integer.MAX_VALUE) {
        return;
    }
    int gap = minStartLine - mPrimaryOrientation->getStartAfterPadding();
    int fixOffset;
    if (gap > 0) {
        fixOffset = scrollBy(gap, recycler, state);
    } else {
        return; // nothing to fix
    }
    gap -= fixOffset;
    if (canOffsetChildren && gap > 0) {
        mPrimaryOrientation->offsetChildren(-gap);
    }
}

void StaggeredGridLayoutManager::updateLayoutState(int anchorPosition, RecyclerView::State& state) {
    mLayoutState->mAvailable = 0;
    mLayoutState->mCurrentPosition = anchorPosition;
    int startExtra = 0;
    int endExtra = 0;
    if (isSmoothScrolling()) {
        const int targetPos = state.getTargetScrollPosition();
        if (targetPos != RecyclerView::NO_POSITION) {
            if (mShouldReverseLayout == targetPos < anchorPosition) {
                endExtra = mPrimaryOrientation->getTotalSpace();
            } else {
                startExtra = mPrimaryOrientation->getTotalSpace();
            }
        }
    }

    // Line of the furthest row.
    const bool clipToPadding = getClipToPadding();
    if (clipToPadding) {
        mLayoutState->mStartLine = mPrimaryOrientation->getStartAfterPadding() - startExtra;
        mLayoutState->mEndLine = mPrimaryOrientation->getEndAfterPadding() + endExtra;
    } else {
        mLayoutState->mEndLine = mPrimaryOrientation->getEnd() + endExtra;
        mLayoutState->mStartLine = -startExtra;
    }
    mLayoutState->mStopInFocusable = false;
    mLayoutState->mRecycle = true;
    mLayoutState->mInfinite = mPrimaryOrientation->getMode() == MeasureSpec::UNSPECIFIED
            && mPrimaryOrientation->getEnd() == 0;
}

void StaggeredGridLayoutManager::setLayoutStateDirection(int direction) {
    mLayoutState->mLayoutDirection = direction;
    mLayoutState->mItemDirection = (mShouldReverseLayout == (direction == LayoutState::LAYOUT_START))
            ? LayoutState::ITEM_DIRECTION_TAIL : LayoutState::ITEM_DIRECTION_HEAD;
}

void StaggeredGridLayoutManager::offsetChildrenHorizontal(int dx) {
    LayoutManager::offsetChildrenHorizontal(dx);
    for (int i = 0; i < mSpanCount; i++) {
        mSpans[i]->onOffset(dx);
    }
}

void StaggeredGridLayoutManager::offsetChildrenVertical(int dy) {
    LayoutManager::offsetChildrenVertical(dy);
    for (int i = 0; i < mSpanCount; i++) {
        mSpans[i]->onOffset(dy);
    }
}

void StaggeredGridLayoutManager::onItemsRemoved(RecyclerView& recyclerView, int positionStart, int itemCount) {
    handleUpdate(positionStart, itemCount, AdapterHelper::UpdateOp::REMOVE);
}

void StaggeredGridLayoutManager::onItemsAdded(RecyclerView& recyclerView, int positionStart, int itemCount) {
    handleUpdate(positionStart, itemCount, AdapterHelper::UpdateOp::ADD);
}

void StaggeredGridLayoutManager::onItemsChanged(RecyclerView& recyclerView) {
    mLazySpanLookup->clear();
    requestLayout();
}

void StaggeredGridLayoutManager::onItemsMoved(RecyclerView& recyclerView, int from, int to, int itemCount) {
    handleUpdate(from, to, AdapterHelper::UpdateOp::MOVE);
}

void StaggeredGridLayoutManager::onItemsUpdated(RecyclerView& recyclerView, int positionStart, int itemCount,
        Object* payload) {
    handleUpdate(positionStart, itemCount, AdapterHelper::UpdateOp::UPDATE);
}

void StaggeredGridLayoutManager::handleUpdate(int positionStart, int itemCountOrToPosition, int cmd) {
    int minPosition = mShouldReverseLayout ? getLastChildPosition() : getFirstChildPosition();
    int affectedRangeEnd; // exclusive
    int affectedRangeStart; // inclusive

    if (cmd == AdapterHelper::UpdateOp::MOVE) {
        if (positionStart < itemCountOrToPosition) {
            affectedRangeEnd = itemCountOrToPosition + 1;
            affectedRangeStart = positionStart;
        } else {
            affectedRangeEnd = positionStart + 1;
            affectedRangeStart = itemCountOrToPosition;
        }
    } else {
        affectedRangeStart = positionStart;
        affectedRangeEnd = positionStart + itemCountOrToPosition;
    }

    mLazySpanLookup->invalidateAfter(affectedRangeStart);
    switch (cmd) {
    case AdapterHelper::UpdateOp::ADD:
        mLazySpanLookup->offsetForAddition(positionStart, itemCountOrToPosition);
        break;
    case AdapterHelper::UpdateOp::REMOVE:
        mLazySpanLookup->offsetForRemoval(positionStart, itemCountOrToPosition);
        break;
    case AdapterHelper::UpdateOp::MOVE:
        // TODO optimize
        mLazySpanLookup->offsetForRemoval(positionStart, 1);
        mLazySpanLookup->offsetForAddition(itemCountOrToPosition, 1);
        break;
    }

    if (affectedRangeEnd <= minPosition) {
        return;
    }

    int maxPosition = mShouldReverseLayout ? getFirstChildPosition() : getLastChildPosition();
    if (affectedRangeStart <= maxPosition) {
        requestLayout();
    }
}

int StaggeredGridLayoutManager::fill(RecyclerView::Recycler& recycler, LayoutState& layoutState, RecyclerView::State& state) {
    mRemainingSpans->set(0, mSpanCount, true);
    // The target position we are trying to reach.
    int targetLine;

    // Line of the furthest row.
    if (mLayoutState->mInfinite) {
        if (layoutState.mLayoutDirection == LayoutState::LAYOUT_END) {
            targetLine = INT_MAX;//Integer.MAX_VALUE;
        } else { // LAYOUT_START
            targetLine = INT_MIN;//Integer.MIN_VALUE;
        }
    } else {
        if (layoutState.mLayoutDirection == LayoutState::LAYOUT_END) {
            targetLine = layoutState.mEndLine + layoutState.mAvailable;
        } else { // LAYOUT_START
            targetLine = layoutState.mStartLine - layoutState.mAvailable;
        }
    }

    updateAllRemainingSpans(layoutState.mLayoutDirection, targetLine);
    LOGD_IF(_Debug,"FILLING targetLine: %d,remaining spans:%p, state:%p",
            targetLine,mRemainingSpans,layoutState);

    // the default coordinate to add new view.
    const int defaultNewViewLine = mShouldReverseLayout
            ? mPrimaryOrientation->getEndAfterPadding()
            : mPrimaryOrientation->getStartAfterPadding();
    bool added = false;
    while (layoutState.hasMore(state)
            && (mLayoutState->mInfinite || !mRemainingSpans->isEmpty())) {
        View* view = layoutState.next(recycler);
        LayoutParams* lp = ((LayoutParams*) view->getLayoutParams());
        const int position = lp->getViewLayoutPosition();
        const int spanIndex = mLazySpanLookup->getSpan(position);
        Span* currentSpan;
        const bool assignSpan = spanIndex == (int)LayoutParams::INVALID_SPAN_ID;
        if (assignSpan) {
            currentSpan = lp->mFullSpan ? mSpans[0] : getNextSpan(layoutState);
            mLazySpanLookup->setSpan(position, currentSpan);
            LOGD_IF(_Debug,"assigned %d for %d",currentSpan->mIndex,position);
        } else {
            LOGD_IF(_Debug,"using %d for pos %d",spanIndex,position);
            currentSpan = mSpans[spanIndex];
        }
        // assign span before measuring so that item decorators can get updated span index
        lp->mSpan = currentSpan;
        if (layoutState.mLayoutDirection == LayoutState::LAYOUT_END) {
            addView(view);
        } else {
            addView(view, 0);
        }
        measureChildWithDecorationsAndMargin(view, *lp, false);

        int start,end;
        if (layoutState.mLayoutDirection == LayoutState::LAYOUT_END) {
            start = lp->mFullSpan ? getMaxEnd(defaultNewViewLine)
                    : currentSpan->getEndLine(defaultNewViewLine);
            end = start + mPrimaryOrientation->getDecoratedMeasurement(view);
            if (assignSpan && lp->mFullSpan) {
		LazySpanLookup::FullSpanItem* fullSpanItem;
                fullSpanItem = (LazySpanLookup::FullSpanItem*)createFullSpanItemFromEnd(start);
                fullSpanItem->mGapDir = LayoutState::LAYOUT_START;
                fullSpanItem->mPosition = position;
                mLazySpanLookup->addFullSpanItem(fullSpanItem);
            }
        } else {
            end = lp->mFullSpan ? getMinStart(defaultNewViewLine)
                    : currentSpan->getStartLine(defaultNewViewLine);
            start = end - mPrimaryOrientation->getDecoratedMeasurement(view);
            if (assignSpan && lp->mFullSpan) {
		LazySpanLookup::FullSpanItem* fullSpanItem;
                fullSpanItem = (LazySpanLookup::FullSpanItem*)createFullSpanItemFromStart(end);
                fullSpanItem->mGapDir = LayoutState::LAYOUT_END;
                fullSpanItem->mPosition = position;
                mLazySpanLookup->addFullSpanItem(fullSpanItem);
            }
        }

        // check if this item may create gaps in the future
        if (lp->mFullSpan && layoutState.mItemDirection == LayoutState::ITEM_DIRECTION_HEAD) {
            if (assignSpan) {
                mLaidOutInvalidFullSpan = true;
            } else {
                bool hasInvalidGap;
                if (layoutState.mLayoutDirection == LayoutState::LAYOUT_END) {
                    hasInvalidGap = !areAllEndsEqual();
                } else { // layoutState.mLayoutDirection == LAYOUT_START
                    hasInvalidGap = !areAllStartsEqual();
                }
                if (hasInvalidGap) {
		    LazySpanLookup::FullSpanItem* fullSpanItem = mLazySpanLookup->getFullSpanItem(position);
                    if (fullSpanItem != nullptr) {
                        fullSpanItem->mHasUnwantedGapAfter = true;
                    }
                    mLaidOutInvalidFullSpan = true;
                }
            }
        }
        attachViewToSpans(view, *lp, layoutState);
        int otherStart, otherEnd;
        if (isLayoutRTL() && mOrientation == VERTICAL) {
            otherEnd = lp->mFullSpan ? mSecondaryOrientation->getEndAfterPadding() :
                    mSecondaryOrientation->getEndAfterPadding() - (mSpanCount - 1 - currentSpan->mIndex) * mSizePerSpan;
            otherStart = otherEnd - mSecondaryOrientation->getDecoratedMeasurement(view);
        } else {
            otherStart = lp->mFullSpan ? mSecondaryOrientation->getStartAfterPadding()
                    : currentSpan->mIndex * mSizePerSpan + mSecondaryOrientation->getStartAfterPadding();
            otherEnd = otherStart + mSecondaryOrientation->getDecoratedMeasurement(view);
        }

        if (mOrientation == VERTICAL) {
            layoutDecoratedWithMargins(view, otherStart, start, otherEnd-otherStart, end-start);
        } else {
            layoutDecoratedWithMargins(view, start, otherStart, end-start, otherEnd-otherStart);
        }

        if (lp->mFullSpan) {
            updateAllRemainingSpans(mLayoutState->mLayoutDirection, targetLine);
        } else {
            updateRemainingSpans(*currentSpan, mLayoutState->mLayoutDirection, targetLine);
        }
        recycle(recycler, *mLayoutState);
        if (mLayoutState->mStopInFocusable && view->hasFocusable()) {
            if (lp->mFullSpan) {
                mRemainingSpans->clear();
            } else {
                mRemainingSpans->set(currentSpan->mIndex, false);
            }
        }
        added = true;
    }
    if (!added) {
        recycle(recycler, *mLayoutState);
    }
    int diff;
    if (mLayoutState->mLayoutDirection == LayoutState::LAYOUT_START) {
        const int minStart = getMinStart(mPrimaryOrientation->getStartAfterPadding());
        diff = mPrimaryOrientation->getStartAfterPadding() - minStart;
    } else {
        const int maxEnd = getMaxEnd(mPrimaryOrientation->getEndAfterPadding());
        diff = maxEnd - mPrimaryOrientation->getEndAfterPadding();
    }
    return diff > 0 ? std::min(layoutState.mAvailable, diff) : 0;
}

//LazySpanLookup::FullSpanItem* 
void*StaggeredGridLayoutManager::createFullSpanItemFromEnd(int newItemTop) {
    LazySpanLookup::FullSpanItem* fsi = new LazySpanLookup::FullSpanItem();
    fsi->mGapPerSpan.resize(mSpanCount);// = new int[mSpanCount];
    for (int i = 0; i < mSpanCount; i++) {
        fsi->mGapPerSpan[i] = newItemTop - mSpans[i]->getEndLine(newItemTop);
    }
    return fsi;
}

//LazySpanLookup::FullSpanItem*
void*StaggeredGridLayoutManager::createFullSpanItemFromStart(int newItemBottom) {
    LazySpanLookup::FullSpanItem* fsi = new LazySpanLookup::FullSpanItem();
    fsi->mGapPerSpan.resize(mSpanCount);// = new int[mSpanCount];
    for (int i = 0; i < mSpanCount; i++) {
        fsi->mGapPerSpan[i] = mSpans[i]->getStartLine(newItemBottom) - newItemBottom;
    }
    return fsi;
}

void StaggeredGridLayoutManager::attachViewToSpans(View* view, LayoutParams& lp, LayoutState& layoutState) {
    if (layoutState.mLayoutDirection == LayoutState::LAYOUT_END) {
        if (lp.mFullSpan) {
            appendViewToAllSpans(view);
        } else {
            lp.mSpan->appendToSpan(view);
        }
    } else {
        if (lp.mFullSpan) {
            prependViewToAllSpans(view);
        } else {
            lp.mSpan->prependToSpan(view);
        }
    }
}

void StaggeredGridLayoutManager::recycle(RecyclerView::Recycler& recycler, LayoutState& layoutState) {
    if (!layoutState.mRecycle || layoutState.mInfinite) {
        return;
    }
    if (layoutState.mAvailable == 0) {
        // easy, recycle line is still valid
        if (layoutState.mLayoutDirection == LayoutState::LAYOUT_START) {
            recycleFromEnd(recycler, layoutState.mEndLine);
        } else {
            recycleFromStart(recycler, layoutState.mStartLine);
        }
    } else {
        // scrolling case, recycle line can be shifted by how much space we could cover
        // by adding new views
        if (layoutState.mLayoutDirection == LayoutState::LAYOUT_START) {
            // calculate recycle line
            int scrolled = layoutState.mStartLine - getMaxStart(layoutState.mStartLine);
            int line;
            if (scrolled < 0) {
                line = layoutState.mEndLine;
            } else {
                line = layoutState.mEndLine - std::min(scrolled, layoutState.mAvailable);
            }
            recycleFromEnd(recycler, line);
        } else {
            // calculate recycle line
            int scrolled = getMinEnd(layoutState.mEndLine) - layoutState.mEndLine;
            int line;
            if (scrolled < 0) {
                line = layoutState.mStartLine;
            } else {
                line = layoutState.mStartLine + std::min(scrolled, layoutState.mAvailable);
            }
            recycleFromStart(recycler, line);
        }
    }

}

void StaggeredGridLayoutManager::appendViewToAllSpans(View* view) {
    // traverse in reverse so that we end up assigning full span items to 0
    for (int i = mSpanCount - 1; i >= 0; i--) {
        mSpans[i]->appendToSpan(view);
    }
}

void StaggeredGridLayoutManager::prependViewToAllSpans(View* view) {
    // traverse in reverse so that we end up assigning full span items to 0
    for (int i = mSpanCount - 1; i >= 0; i--) {
        mSpans[i]->prependToSpan(view);
    }
}

void StaggeredGridLayoutManager::updateAllRemainingSpans(int layoutDir, int targetLine) {
    for (int i = 0; i < mSpanCount; i++) {
        if (mSpans[i]->mViews.empty()) {
            continue;
        }
        updateRemainingSpans(*mSpans[i], layoutDir, targetLine);
    }
}

void StaggeredGridLayoutManager::updateRemainingSpans(Span& span, int layoutDir, int targetLine) {
    const int deletedSize = span.getDeletedSize();
    if (layoutDir == LayoutState::LAYOUT_START) {
        const int line = span.getStartLine();
        if (line + deletedSize <= targetLine) {
            mRemainingSpans->set(span.mIndex, false);
        }
    } else {
        const int line = span.getEndLine();
        if (line - deletedSize >= targetLine) {
            mRemainingSpans->set(span.mIndex, false);
        }
    }
}

int StaggeredGridLayoutManager::getMaxStart(int def) {
    int maxStart = mSpans[0]->getStartLine(def);
    for (int i = 1; i < mSpanCount; i++) {
        const int spanStart = mSpans[i]->getStartLine(def);
        if (spanStart > maxStart) {
            maxStart = spanStart;
        }
    }
    return maxStart;
}

int StaggeredGridLayoutManager::getMinStart(int def) {
    int minStart = mSpans[0]->getStartLine(def);
    for (int i = 1; i < mSpanCount; i++) {
        const int spanStart = mSpans[i]->getStartLine(def);
        if (spanStart < minStart) {
            minStart = spanStart;
        }
    }
    return minStart;
}

bool StaggeredGridLayoutManager::areAllEndsEqual() {
    int end = mSpans[0]->getEndLine(Span::INVALID_LINE);
    for (int i = 1; i < mSpanCount; i++) {
        if (mSpans[i]->getEndLine(Span::INVALID_LINE) != end) {
            return false;
        }
    }
    return true;
}

bool StaggeredGridLayoutManager::areAllStartsEqual() {
    int start = mSpans[0]->getStartLine(Span::INVALID_LINE);
    for (int i = 1; i < mSpanCount; i++) {
        if (mSpans[i]->getStartLine(Span::INVALID_LINE) != start) {
            return false;
        }
    }
    return true;
}

int StaggeredGridLayoutManager::getMaxEnd(int def) {
    int maxEnd = mSpans[0]->getEndLine(def);
    for (int i = 1; i < mSpanCount; i++) {
        const int spanEnd = mSpans[i]->getEndLine(def);
        if (spanEnd > maxEnd) {
            maxEnd = spanEnd;
        }
    }
    return maxEnd;
}

int StaggeredGridLayoutManager::getMinEnd(int def) {
    int minEnd = mSpans[0]->getEndLine(def);
    for (int i = 1; i < mSpanCount; i++) {
        const int spanEnd = mSpans[i]->getEndLine(def);
        if (spanEnd < minEnd) {
            minEnd = spanEnd;
        }
    }
    return minEnd;
}

void StaggeredGridLayoutManager::recycleFromStart(RecyclerView::Recycler& recycler, int line) {
    while (getChildCount() > 0) {
        View* child = getChildAt(0);
        if (mPrimaryOrientation->getDecoratedEnd(child) <= line
                && mPrimaryOrientation->getTransformedEndWithDecoration(child) <= line) {
            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
            // Don't recycle the last View in a span not to lose span's start/end lines
            if (lp->mFullSpan) {
                for (int j = 0; j < mSpanCount; j++) {
                    if (mSpans[j]->mViews.size() == 1) {
                        return;
                    }
                }
                for (int j = 0; j < mSpanCount; j++) {
                    mSpans[j]->popStart();
                }
            } else {
                if (lp->mSpan->mViews.size() == 1) {
                    return;
                }
                lp->mSpan->popStart();
            }
            removeAndRecycleView(child, recycler);
        } else {
            return; // done
        }
    }
}

void StaggeredGridLayoutManager::recycleFromEnd(RecyclerView::Recycler& recycler, int line) {
    const int childCount = getChildCount();
    int i;
    for (i = childCount - 1; i >= 0; i--) {
        View* child = getChildAt(i);
        if (mPrimaryOrientation->getDecoratedStart(child) >= line
                && mPrimaryOrientation->getTransformedStartWithDecoration(child) >= line) {
            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
            // Don't recycle the last View in a span not to lose span's start/end lines
            if (lp->mFullSpan) {
                for (int j = 0; j < mSpanCount; j++) {
                    if (mSpans[j]->mViews.size() == 1) {
                        return;
                    }
                }
                for (int j = 0; j < mSpanCount; j++) {
                    mSpans[j]->popEnd();
                }
            } else {
                if (lp->mSpan->mViews.size() == 1) {
                    return;
                }
                lp->mSpan->popEnd();
            }
            removeAndRecycleView(child, recycler);
        } else {
            return; // done
        }
    }
}

bool StaggeredGridLayoutManager::preferLastSpan(int layoutDir) {
    if (mOrientation == HORIZONTAL) {
        return (layoutDir == LayoutState::LAYOUT_START) != mShouldReverseLayout;
    }
    return ((layoutDir == LayoutState::LAYOUT_START) == mShouldReverseLayout) == isLayoutRTL();
}

StaggeredGridLayoutManager::Span* StaggeredGridLayoutManager::getNextSpan(LayoutState& layoutState) {
    const bool bPreferLastSpan = preferLastSpan(layoutState.mLayoutDirection);
    int startIndex, endIndex, diff;
    if (bPreferLastSpan) {
        startIndex = mSpanCount - 1;
        endIndex = -1;
        diff = -1;
    } else {
        startIndex = 0;
        endIndex = mSpanCount;
        diff = 1;
    }
    if (layoutState.mLayoutDirection == LayoutState::LAYOUT_END) {
        Span* min = nullptr;
        int minLine = INT_MAX;//Integer.MAX_VALUE;
        const int defaultLine = mPrimaryOrientation->getStartAfterPadding();
        for (int i = startIndex; i != endIndex; i += diff) {
            Span* other = mSpans[i];
            int otherLine = other->getEndLine(defaultLine);
            if (otherLine < minLine) {
                min = other;
                minLine = otherLine;
            }
        }
        return min;
    } else {
        Span* max = nullptr;
        int maxLine = INT_MIN;//Integer.MIN_VALUE;
        const int defaultLine = mPrimaryOrientation->getEndAfterPadding();
        for (int i = startIndex; i != endIndex; i += diff) {
            Span* other = mSpans[i];
            int otherLine = other->getStartLine(defaultLine);
            if (otherLine > maxLine) {
                max = other;
                maxLine = otherLine;
            }
        }
        return max;
    }
}

bool StaggeredGridLayoutManager::canScrollVertically()const{
    return mOrientation == VERTICAL;
}

bool StaggeredGridLayoutManager::canScrollHorizontally()const{
    return mOrientation == HORIZONTAL;
}

int StaggeredGridLayoutManager::scrollHorizontallyBy(int dx, RecyclerView::Recycler& recycler,RecyclerView::State& state) {
    return scrollBy(dx, recycler, state);
}

int StaggeredGridLayoutManager::scrollVerticallyBy(int dy, RecyclerView::Recycler& recycler,RecyclerView::State& state) {
    return scrollBy(dy, recycler, state);
}

int StaggeredGridLayoutManager::calculateScrollDirectionForPosition(int position) {
    if (getChildCount() == 0) {
        return mShouldReverseLayout ? LayoutState::LAYOUT_END : LayoutState::LAYOUT_START;
    }
    const int firstChildPos = getFirstChildPosition();
    return position < firstChildPos != mShouldReverseLayout ? LayoutState::LAYOUT_START : LayoutState::LAYOUT_END;
}

bool StaggeredGridLayoutManager::computeScrollVectorForPosition(int targetPosition,PointF&outVector) {
    const int direction = calculateScrollDirectionForPosition(targetPosition);
    if (direction == 0) {
        return false;
    }
    if (mOrientation == HORIZONTAL) {
        outVector.set(float(direction),0.f);
    } else {
        outVector.set(0.f,float(direction));
    }
    return true;
}

void StaggeredGridLayoutManager::smoothScrollToPosition(RecyclerView& recyclerView, RecyclerView::State& state,int position) {
    LinearSmoothScroller* scroller = new LinearSmoothScroller(recyclerView.getContext());
    scroller->setTargetPosition(position);
    startSmoothScroll(scroller);
}

void StaggeredGridLayoutManager::scrollToPosition(int position) {
    if (mPendingSavedState != nullptr && mPendingSavedState->mAnchorPosition != position) {
        mPendingSavedState->invalidateAnchorPositionInfo();
    }
    mPendingScrollPosition = position;
    mPendingScrollPositionOffset = INVALID_OFFSET;
    requestLayout();
}

void StaggeredGridLayoutManager::scrollToPositionWithOffset(int position, int offset) {
    if (mPendingSavedState != nullptr) {
        mPendingSavedState->invalidateAnchorPositionInfo();
    }
    mPendingScrollPosition = position;
    mPendingScrollPositionOffset = offset;
    requestLayout();
}

void StaggeredGridLayoutManager::collectAdjacentPrefetchPositions(int dx, int dy, RecyclerView::State& state,
        LayoutPrefetchRegistry& layoutPrefetchRegistry) {

    int delta = (mOrientation == HORIZONTAL) ? dx : dy;
    if (getChildCount() == 0 || delta == 0) {
        // can't support this scroll, so don't bother prefetching
        return;
    }
    prepareLayoutStateForDelta(delta, state);

    // build sorted list of distances to end of each span (though we don't care which is which)
    if (mPrefetchDistances.size() < mSpanCount) {
        mPrefetchDistances.resize(mSpanCount);
    }

    int itemPrefetchCount = 0;
    for (int i = 0; i < mSpanCount; i++) {
        // compute number of pixels past the edge of the viewport that the current span extends
        int distance = mLayoutState->mItemDirection == LayoutState::LAYOUT_START
                ? mLayoutState->mStartLine - mSpans[i]->getStartLine(mLayoutState->mStartLine)
                : mSpans[i]->getEndLine(mLayoutState->mEndLine) - mLayoutState->mEndLine;
        if (distance >= 0) {
            // span extends to the edge, so prefetch next item
            mPrefetchDistances[itemPrefetchCount] = distance;
            itemPrefetchCount++;
        }
    }
    //Arrays.sort(mPrefetchDistances, 0, itemPrefetchCount);
    std::sort(mPrefetchDistances.begin(),mPrefetchDistances.end());

    // then assign them in order to the next N views (where N = span count)
    for (int i = 0; i < itemPrefetchCount && mLayoutState->hasMore(state); i++) {
        layoutPrefetchRegistry.addPosition(mLayoutState->mCurrentPosition, mPrefetchDistances[i]);
        mLayoutState->mCurrentPosition += mLayoutState->mItemDirection;
    }
}

void StaggeredGridLayoutManager::prepareLayoutStateForDelta(int delta, RecyclerView::State state) {
    int referenceChildPosition;
    int layoutDir;
    if (delta > 0) { // layout towards end
        layoutDir = LayoutState::LAYOUT_END;
        referenceChildPosition = getLastChildPosition();
    } else {
        layoutDir = LayoutState::LAYOUT_START;
        referenceChildPosition = getFirstChildPosition();
    }
    mLayoutState->mRecycle = true;
    updateLayoutState(referenceChildPosition, state);
    setLayoutStateDirection(layoutDir);
    mLayoutState->mCurrentPosition = referenceChildPosition + mLayoutState->mItemDirection;
    mLayoutState->mAvailable = std::abs(delta);
}

int StaggeredGridLayoutManager::scrollBy(int dt, RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    if (getChildCount() == 0 || dt == 0) {
        return 0;
    }

    prepareLayoutStateForDelta(dt, state);
    int consumed = fill(recycler, *mLayoutState, state);
    const int available = mLayoutState->mAvailable;
    int totalScroll;
    if (available < consumed) {
        totalScroll = dt;
    } else if (dt < 0) {
        totalScroll = -consumed;
    } else { // dt > 0
        totalScroll = consumed;
    }
    LOGD_IF(_Debug,"asked %d scrolled", dt, totalScroll);

    mPrimaryOrientation->offsetChildren(-totalScroll);
    // always reset this if we scroll for a proper save instance state
    mLastLayoutFromEnd = mShouldReverseLayout;
    mLayoutState->mAvailable = 0;
    recycle(recycler, *mLayoutState);
    return totalScroll;
}

int StaggeredGridLayoutManager::getLastChildPosition() {
    const int childCount = getChildCount();
    return childCount == 0 ? 0 : getPosition(getChildAt(childCount - 1));
}

int StaggeredGridLayoutManager::getFirstChildPosition() {
    const int childCount = getChildCount();
    return childCount == 0 ? 0 : getPosition(getChildAt(0));
}

int StaggeredGridLayoutManager::findFirstReferenceChildPosition(int itemCount) {
    const int limit = getChildCount();
    for (int i = 0; i < limit; i++) {
        View* view = getChildAt(i);
        const int position = getPosition(view);
        if (position >= 0 && position < itemCount) {
            return position;
        }
    }
    return 0;
}

int StaggeredGridLayoutManager::findLastReferenceChildPosition(int itemCount) {
    for (int i = getChildCount() - 1; i >= 0; i--) {
        View* view = getChildAt(i);
        int position = getPosition(view);
        if (position >= 0 && position < itemCount) {
            return position;
        }
    }
    return 0;
}

StaggeredGridLayoutManager::LayoutParams* StaggeredGridLayoutManager::generateDefaultLayoutParams()const{
    if (mOrientation == HORIZONTAL) {
        return new LayoutParams(LayoutParams::WRAP_CONTENT, LayoutParams::MATCH_PARENT);
    } else {
        return new LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::WRAP_CONTENT);
    }
}

StaggeredGridLayoutManager::LayoutParams* StaggeredGridLayoutManager::generateLayoutParams(Context* c,const AttributeSet& attrs)const{
    return new LayoutParams(c, attrs);
}

StaggeredGridLayoutManager::LayoutParams* StaggeredGridLayoutManager::generateLayoutParams(const ViewGroup::LayoutParams& lp)const{
    if (dynamic_cast<const ViewGroup::MarginLayoutParams*>(&lp)) {
        return new LayoutParams((const ViewGroup::MarginLayoutParams&) lp);
    }
    return new LayoutParams(lp);
}

bool StaggeredGridLayoutManager::checkLayoutParams(const RecyclerView::LayoutParams* lp)const{
    return dynamic_cast<const LayoutParams*>(lp);
}

int StaggeredGridLayoutManager::getOrientation() const{
    return mOrientation;
}

View* StaggeredGridLayoutManager::onFocusSearchFailed(View* focused, int direction, RecyclerView::Recycler& recycler,
        RecyclerView::State& state) {
    if (getChildCount() == 0) {
        return nullptr;
    }

    View* directChild = findContainingItemView(focused);
    if (directChild == nullptr) {
        return nullptr;
    }

    resolveShouldLayoutReverse();
    const int layoutDir = convertFocusDirectionToLayoutDirection(direction);
    if (layoutDir == LayoutState::INVALID_LAYOUT) {
        return nullptr;
    }
    LayoutParams* prevFocusLayoutParams = (LayoutParams*) directChild->getLayoutParams();
    bool prevFocusFullSpan = prevFocusLayoutParams->mFullSpan;
    Span* prevFocusSpan = prevFocusLayoutParams->mSpan;
    int referenceChildPosition;
    if (layoutDir == LayoutState::LAYOUT_END) { // layout towards end
        referenceChildPosition = getLastChildPosition();
    } else {
        referenceChildPosition = getFirstChildPosition();
    }
    updateLayoutState(referenceChildPosition, state);
    setLayoutStateDirection(layoutDir);

    mLayoutState->mCurrentPosition = referenceChildPosition + mLayoutState->mItemDirection;
    mLayoutState->mAvailable = (int) (MAX_SCROLL_FACTOR * mPrimaryOrientation->getTotalSpace());
    mLayoutState->mStopInFocusable = true;
    mLayoutState->mRecycle = false;
    fill(recycler, *mLayoutState, state);
    mLastLayoutFromEnd = mShouldReverseLayout;
    if (!prevFocusFullSpan) {
        View* view = prevFocusSpan->getFocusableViewAfter(referenceChildPosition, layoutDir);
        if (view != nullptr && view != directChild) {
            return view;
        }
    }

    // either could not find from the desired span or prev view is full span.
    // traverse all spans
    if (preferLastSpan(layoutDir)) {
        for (int i = mSpanCount - 1; i >= 0; i--) {
            View* view = mSpans[i]->getFocusableViewAfter(referenceChildPosition, layoutDir);
            if (view != nullptr && view != directChild) {
                return view;
            }
        }
    } else {
        for (int i = 0; i < mSpanCount; i++) {
            View* view = mSpans[i]->getFocusableViewAfter(referenceChildPosition, layoutDir);
            if (view != nullptr && view != directChild) {
                return view;
            }
        }
    }

    // Could not find any focusable views from any of the existing spans. Now start the search
    // to find the best unfocusable candidate to become visible on the screen next. The search
    // is done in the same fashion: first, check the views in the desired span and if no
    // candidate is found, traverse the views in all the remaining spans.
    bool shouldSearchFromStart = !mReverseLayout == (layoutDir == LayoutState::LAYOUT_START);
    View* unfocusableCandidate = nullptr;
    if (!prevFocusFullSpan) {
        unfocusableCandidate = findViewByPosition(shouldSearchFromStart
                ? prevFocusSpan->findFirstPartiallyVisibleItemPosition() :
                prevFocusSpan->findLastPartiallyVisibleItemPosition());
        if (unfocusableCandidate != nullptr && unfocusableCandidate != directChild) {
            return unfocusableCandidate;
        }
    }

    if (preferLastSpan(layoutDir)) {
        for (int i = mSpanCount - 1; i >= 0; i--) {
            if (i == prevFocusSpan->mIndex) {
                continue;
            }
            unfocusableCandidate = findViewByPosition(shouldSearchFromStart
                    ? mSpans[i]->findFirstPartiallyVisibleItemPosition() :
                    mSpans[i]->findLastPartiallyVisibleItemPosition());
            if (unfocusableCandidate != nullptr && unfocusableCandidate != directChild) {
                return unfocusableCandidate;
            }
        }
    } else {
        for (int i = 0; i < mSpanCount; i++) {
            unfocusableCandidate = findViewByPosition(shouldSearchFromStart
                    ? mSpans[i]->findFirstPartiallyVisibleItemPosition() :
                    mSpans[i]->findLastPartiallyVisibleItemPosition());
            if (unfocusableCandidate != nullptr && unfocusableCandidate != directChild) {
                return unfocusableCandidate;
            }
        }
    }
    return nullptr;
}

int StaggeredGridLayoutManager::convertFocusDirectionToLayoutDirection(int focusDirection) {
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
        LOGD_IF(_Debug,"Unknown focus request:%d",focusDirection);
        return LayoutState::INVALID_LAYOUT;
    }

}

////////////////////////////////////////////LayoutParams//////////////////////////////////////////

StaggeredGridLayoutManager::LayoutParams::LayoutParams(Context* c,const  AttributeSet& attrs)
   :RecyclerView::LayoutParams(c, attrs){
}

StaggeredGridLayoutManager::LayoutParams::LayoutParams(int width, int height)
    :RecyclerView::LayoutParams(width, height){
}

StaggeredGridLayoutManager::LayoutParams::LayoutParams(const ViewGroup::MarginLayoutParams& source)
    :RecyclerView::LayoutParams(source){
}

StaggeredGridLayoutManager::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& source)
    :RecyclerView::LayoutParams(source){
}

StaggeredGridLayoutManager::LayoutParams::LayoutParams(const RecyclerView::LayoutParams& source)
    :RecyclerView::LayoutParams(source){
}

void StaggeredGridLayoutManager::LayoutParams::setFullSpan(bool fullSpan) {
    mFullSpan = fullSpan;
}

bool StaggeredGridLayoutManager::LayoutParams::isFullSpan() {
    return mFullSpan;
}

int StaggeredGridLayoutManager::LayoutParams::getSpanIndex() {
    if (mSpan == nullptr) {
        return (int)INVALID_SPAN_ID;
    }
    return mSpan->mIndex;
}

// Package scoped to access from tests.

StaggeredGridLayoutManager::Span::Span(StaggeredGridLayoutManager*lm,int index) {
    mIndex = index;
    mLM = lm;
}

int StaggeredGridLayoutManager::Span::getStartLine(int def) {
    if (mCachedStart != INVALID_LINE) {
        return mCachedStart;
    }
    if (mViews.size() == 0) {
        return def;
    }
    calculateCachedStart();
    return mCachedStart;
}

void StaggeredGridLayoutManager::Span::calculateCachedStart() {
    View* startView = mViews.at(0);
    LayoutParams* lp = getLayoutParams(startView);
    mCachedStart = mLM->mPrimaryOrientation->getDecoratedStart(startView);
    if (lp->mFullSpan) {
        LazySpanLookup::FullSpanItem* fsi = mLM->mLazySpanLookup->getFullSpanItem(lp->getViewLayoutPosition());
        if (fsi != nullptr && fsi->mGapDir == LayoutState::LAYOUT_START) {
            mCachedStart -= fsi->getGapForSpan(mIndex);
        }
    }
}

// Use this one when default value does not make sense and not having a value means a bug.
int StaggeredGridLayoutManager::Span::getStartLine() {
    if (mCachedStart != INVALID_LINE) {
        return mCachedStart;
    }
    calculateCachedStart();
    return mCachedStart;
}

int StaggeredGridLayoutManager::Span::getEndLine(int def) {
    if (mCachedEnd != INVALID_LINE) {
        return mCachedEnd;
    }
    const size_t size = mViews.size();
    if (size == 0) {
        return def;
    }
    calculateCachedEnd();
    return mCachedEnd;
}

void StaggeredGridLayoutManager::Span::calculateCachedEnd() {
    View* endView = mViews.at(mViews.size() - 1);
    LayoutParams* lp = getLayoutParams(endView);
    mCachedEnd = mLM->mPrimaryOrientation->getDecoratedEnd(endView);
    if (lp->mFullSpan) {
        LazySpanLookup::FullSpanItem* fsi = mLM->mLazySpanLookup->getFullSpanItem(lp->getViewLayoutPosition());
        if (fsi != nullptr && fsi->mGapDir == LayoutState::LAYOUT_END) {
            mCachedEnd += fsi->getGapForSpan(mIndex);
        }
    }
}

// Use this one when default value does not make sense and not having a value means a bug.
int StaggeredGridLayoutManager::Span::getEndLine() {
    if (mCachedEnd != INVALID_LINE) {
        return mCachedEnd;
    }
    calculateCachedEnd();
    return mCachedEnd;
}

void StaggeredGridLayoutManager::Span::prependToSpan(View* view) {
    LayoutParams* lp = getLayoutParams(view);
    lp->mSpan = this;
    mViews.insert(mViews.begin(),view);//add(0, view);
    mCachedStart = INVALID_LINE;
    if (mViews.size() == 1) {
        mCachedEnd = INVALID_LINE;
    }
    if (lp->isItemRemoved() || lp->isItemChanged()) {
        mDeletedSize += mLM->mPrimaryOrientation->getDecoratedMeasurement(view);
    }
}

void StaggeredGridLayoutManager::Span::appendToSpan(View* view) {
    LayoutParams* lp = getLayoutParams(view);
    lp->mSpan = this;
    mViews.push_back(view);
    mCachedEnd = INVALID_LINE;
    if (mViews.size() == 1) {
        mCachedStart = INVALID_LINE;
    }
    if (lp->isItemRemoved() || lp->isItemChanged()) {
        mDeletedSize += mLM->mPrimaryOrientation->getDecoratedMeasurement(view);
    }
}

// Useful method to preserve positions on a re-layout.
void StaggeredGridLayoutManager::Span::cacheReferenceLineAndClear(bool reverseLayout, int offset) {
    int reference;
    if (reverseLayout) {
        reference = getEndLine(INVALID_LINE);
    } else {
        reference = getStartLine(INVALID_LINE);
    }
    clear();
    if (reference == INVALID_LINE) {
        return;
    }
    if ((reverseLayout && reference < mLM->mPrimaryOrientation->getEndAfterPadding())
            || (!reverseLayout && reference > mLM->mPrimaryOrientation->getStartAfterPadding())) {
        return;
    }
    if (offset != INVALID_OFFSET) {
        reference += offset;
    }
    mCachedStart = mCachedEnd = reference;
}

void StaggeredGridLayoutManager::Span::clear() {
    mViews.clear();
    invalidateCache();
    mDeletedSize = 0;
}

void StaggeredGridLayoutManager::Span::invalidateCache() {
    mCachedStart = INVALID_LINE;
    mCachedEnd = INVALID_LINE;
}

void StaggeredGridLayoutManager::Span::setLine(int line) {
    mCachedEnd = mCachedStart = line;
}

void StaggeredGridLayoutManager::Span::popEnd() {
    const int size = (int)mViews.size();
    View* end = mViews.back();mViews.pop_back();//remove(size - 1);
    LayoutParams* lp = getLayoutParams(end);
    lp->mSpan = nullptr;
    if (lp->isItemRemoved() || lp->isItemChanged()) {
        mDeletedSize -= mLM->mPrimaryOrientation->getDecoratedMeasurement(end);
    }
    if (size == 1) {
        mCachedStart = INVALID_LINE;
    }
    mCachedEnd = INVALID_LINE;
}

void StaggeredGridLayoutManager::Span::popStart() {
    View* start = mViews.front(); mViews.erase(mViews.begin());
    //mViews.remove(0);
    LayoutParams* lp = getLayoutParams(start);
    lp->mSpan = nullptr;
    if (mViews.size() == 0) {
        mCachedEnd = INVALID_LINE;
    }
    if (lp->isItemRemoved() || lp->isItemChanged()) {
        mDeletedSize -= mLM->mPrimaryOrientation->getDecoratedMeasurement(start);
    }
    mCachedStart = INVALID_LINE;
}

int StaggeredGridLayoutManager::Span::getDeletedSize() {
    return mDeletedSize;
}

StaggeredGridLayoutManager::LayoutParams* StaggeredGridLayoutManager::Span::getLayoutParams(View* view) {
    return (LayoutParams*) view->getLayoutParams();
}

void StaggeredGridLayoutManager::Span::onOffset(int dt) {
    if (mCachedStart != INVALID_LINE) {
        mCachedStart += dt;
    }
    if (mCachedEnd != INVALID_LINE) {
        mCachedEnd += dt;
    }
}

int StaggeredGridLayoutManager::Span::findFirstVisibleItemPosition() {
    return mLM->mReverseLayout
            ? findOneVisibleChild(int(mViews.size() - 1), -1, false)
            : findOneVisibleChild(0, (int)mViews.size(), false);
}

int StaggeredGridLayoutManager::Span::findFirstPartiallyVisibleItemPosition() {
    return mLM->mReverseLayout
            ? findOnePartiallyVisibleChild(int(mViews.size() - 1), -1, true)
            : findOnePartiallyVisibleChild(0, (int)mViews.size(), true);
}

int StaggeredGridLayoutManager::Span::findFirstCompletelyVisibleItemPosition() {
    return mLM->mReverseLayout
            ? findOneVisibleChild(int(mViews.size() - 1), -1, true)
            : findOneVisibleChild(0, (int)mViews.size(), true);
}

int StaggeredGridLayoutManager::Span::findLastVisibleItemPosition() {
    return mLM->mReverseLayout
            ? findOneVisibleChild(0, (int)mViews.size(), false)
            : findOneVisibleChild(int(mViews.size() - 1), -1, false);
}

int StaggeredGridLayoutManager::Span::findLastPartiallyVisibleItemPosition() {
    return mLM->mReverseLayout
            ? findOnePartiallyVisibleChild(0, (int)mViews.size(), true)
            : findOnePartiallyVisibleChild(int(mViews.size() - 1), -1, true);
}

int StaggeredGridLayoutManager::Span::findLastCompletelyVisibleItemPosition() {
    return mLM->mReverseLayout
            ? findOneVisibleChild(0, (int)mViews.size(), true)
            : findOneVisibleChild(int(mViews.size() - 1), -1, true);
}

int StaggeredGridLayoutManager::Span::findOnePartiallyOrCompletelyVisibleChild(int fromIndex, int toIndex,
        bool completelyVisible,bool acceptCompletelyVisible, bool acceptEndPointInclusion) {
    const int start = mLM->mPrimaryOrientation->getStartAfterPadding();
    const int end = mLM->mPrimaryOrientation->getEndAfterPadding();
    const int next = toIndex > fromIndex ? 1 : -1;
    for (int i = fromIndex; i != toIndex; i += next) {
        View* child = mViews.at(i);
        const int childStart = mLM->mPrimaryOrientation->getDecoratedStart(child);
        const int childEnd = mLM->mPrimaryOrientation->getDecoratedEnd(child);
        bool childStartInclusion = acceptEndPointInclusion ? (childStart <= end)
                : (childStart < end);
        bool childEndInclusion = acceptEndPointInclusion ? (childEnd >= start)
                : (childEnd > start);
        if (childStartInclusion && childEndInclusion) {
            if (completelyVisible && acceptCompletelyVisible) {
                // the child has to be completely visible to be returned.
                if (childStart >= start && childEnd <= end) {
                    return mLM->getPosition(child);
                }
            } else if (acceptCompletelyVisible) {
                // can return either a partially or completely visible child.
                return mLM->getPosition(child);
            } else if (childStart < start || childEnd > end) {
                // should return a partially visible child if exists and a completely
                // visible child is not acceptable in this case.
                return mLM->getPosition(child);
            }
        }
    }
    return RecyclerView::NO_POSITION;
}

int StaggeredGridLayoutManager::Span::findOneVisibleChild(int fromIndex, int toIndex, bool completelyVisible) {
    return findOnePartiallyOrCompletelyVisibleChild(fromIndex, toIndex, completelyVisible,
            true, false);
}

int StaggeredGridLayoutManager::Span::findOnePartiallyVisibleChild(int fromIndex, int toIndex, bool acceptEndPointInclusion) {
    return findOnePartiallyOrCompletelyVisibleChild(fromIndex, toIndex, false, false,
            acceptEndPointInclusion);
}

View* StaggeredGridLayoutManager::Span::getFocusableViewAfter(int referenceChildPosition, int layoutDir) {
    View* candidate = nullptr;
    if (layoutDir == LayoutState::LAYOUT_START) {
        const int limit = (int)mViews.size();
        for (int i = 0; i < limit; i++) {
            View* view = mViews.at(i);
            if ((mLM->mReverseLayout && mLM->getPosition(view) <= referenceChildPosition)
                    || (!mLM->mReverseLayout && mLM->getPosition(view) >= referenceChildPosition)) {
                break;
            }
            if (view->hasFocusable()) {
                candidate = view;
            } else {
                break;
            }
        }
    } else {
        for (int i = int(mViews.size() - 1); i >= 0; i--) {
            View* view = mViews.at(i);
            if ((mLM->mReverseLayout && mLM->getPosition(view) >= referenceChildPosition)
                    || (!mLM->mReverseLayout && mLM->getPosition(view) <= referenceChildPosition)) {
                break;
            }
            if (view->hasFocusable()) {
                candidate = view;
            } else {
                break;
            }
        }
    }
    return candidate;
}

////////////////////////////////////////////////////////////////////////////////////////////

int StaggeredGridLayoutManager::LazySpanLookup::forceInvalidateAfter(int position) {
    if (mFullSpanItems.size()){// != null) {
        for (int i = int(mFullSpanItems.size() - 1); i >= 0; i--) {
            FullSpanItem* fsi = mFullSpanItems.at(i);
            if (fsi->mPosition >= position) {
                mFullSpanItems.erase(mFullSpanItems.begin()+i);//remove(i);
            }
        }
    }
    return invalidateAfter(position);
}

int StaggeredGridLayoutManager::LazySpanLookup::invalidateAfter(int position) {
    if (mData.empty()) {
        return RecyclerView::NO_POSITION;
    }
    if (position >= mData.size()) {
        return RecyclerView::NO_POSITION;
    }
    int endPosition = invalidateFullSpansAfter(position);
    if (endPosition == RecyclerView::NO_POSITION) {
        //Arrays.fill(mData, position, mData.size(), LayoutParams::INVALID_SPAN_ID);
        std::fill(mData.begin() + position, mData.end() , (int)LayoutParams::INVALID_SPAN_ID);
        return (int)mData.size();
    } else {
        // Just invalidate items in between `position` and the next full span item, or the
        // end of the tracked spans in mData if it's not been lengthened yet.
        const int invalidateToIndex = std::min(endPosition + 1, (int)mData.size());
        std::fill(mData.begin() + position,mData.begin() + invalidateToIndex, (int)LayoutParams::INVALID_SPAN_ID);
        return invalidateToIndex;
    }
}

int StaggeredGridLayoutManager::LazySpanLookup::getSpan(int position) {
    if (mData.empty() || position >= mData.size()) {
        return (int)LayoutParams::INVALID_SPAN_ID;
    } else {
        return mData[position];
    }
}

void StaggeredGridLayoutManager::LazySpanLookup::setSpan(int position, Span* span) {
    ensureSize(position);
    mData[position] = span->mIndex;
}

int StaggeredGridLayoutManager::LazySpanLookup::sizeForPosition(int position) {
    int len = (int)mData.size();
    while (len <= position) {
        len *= 2;
    }
    return len;
}

void StaggeredGridLayoutManager::LazySpanLookup::ensureSize(int position) {
    if (mData.empty()){// == null) {
        mData.resize(std::max(position, (int)MIN_SIZE) + 1);// = new int[std::max(position, MIN_SIZE) + 1];
        std::fill(mData.begin(),mData.end(),(int)LayoutParams::INVALID_SPAN_ID);
    } else if (position >= mData.size()) {
        const size_t oldlen = mData.size();//int[] old = mData;
        const size_t newlen = sizeForPosition(position);
        mData.resize(newlen);
	    std::fill(mData.begin()+oldlen,mData.end(),(int)LayoutParams::INVALID_SPAN_ID);
    }
}

void StaggeredGridLayoutManager::LazySpanLookup::clear() {
    if (mData.size()){// != null) {
        //Arrays.fill(mData, LayoutParams::INVALID_SPAN_ID);
        std::fill(mData.begin(),mData.end(),(int)LayoutParams::INVALID_SPAN_ID);
    }
    mFullSpanItems.clear();// = null;
}

void StaggeredGridLayoutManager::LazySpanLookup::offsetForRemoval(int positionStart, int itemCount) {
    if (mData.empty()/* == null*/ || positionStart >= mData.size()) {
        return;
    }
    ensureSize(positionStart + itemCount);
    //System.arraycopy(mData, positionStart + itemCount, mData, positionStart,mData.size() - positionStart - itemCount);
    std::copy(mData.begin()+positionStart + itemCount, mData.begin() + itemCount +
		    mData.size() - positionStart - itemCount, mData.begin()+positionStart);
    //Arrays.fill(mData, mData.size() - itemCount, mData.size(), LayoutParams::INVALID_SPAN_ID);
    std::fill(mData.end() - itemCount , mData.end() , (int)LayoutParams::INVALID_SPAN_ID);
    offsetFullSpansForRemoval(positionStart, itemCount);
}

void StaggeredGridLayoutManager::LazySpanLookup::offsetFullSpansForRemoval(int positionStart, int itemCount) {
    if (mFullSpanItems.empty()){// == null) {
        return;
    }
    const int end = positionStart + itemCount;
    for (int i = int(mFullSpanItems.size() - 1); i >= 0; i--) {
        FullSpanItem* fsi = mFullSpanItems.at(i);
        if (fsi->mPosition < positionStart) {
            continue;
        }
        if (fsi->mPosition < end) {
            mFullSpanItems.erase(mFullSpanItems.begin()+i);//remove(i);
        } else {
            fsi->mPosition -= itemCount;
        }
    }
}

void StaggeredGridLayoutManager::LazySpanLookup::offsetForAddition(int positionStart, int itemCount) {
    if (mData.empty() || positionStart >= mData.size()) {
        return;
    }
    ensureSize(positionStart + itemCount);
    //System.arraycopy(mData, positionStart, mData, positionStart + itemCount,mData.size() - positionStart - itemCount);
    std::copy(mData.begin() + positionStart,mData.begin() + positionStart + mData.size() - positionStart - itemCount,
		    mData.begin() + positionStart + itemCount);

    //Arrays.fill(mData, positionStart, positionStart + itemCount,LayoutParams::INVALID_SPAN_ID);
    std::fill(mData.begin()+positionStart,mData.begin()+positionStart+itemCount,(int)LayoutParams::INVALID_SPAN_ID);
    offsetFullSpansForAddition(positionStart, itemCount);
}

void StaggeredGridLayoutManager::LazySpanLookup::offsetFullSpansForAddition(int positionStart, int itemCount) {
    if (mFullSpanItems.empty()){// == null) {
        return;
    }
    for (int i = int(mFullSpanItems.size() - 1); i >= 0; i--) {
        FullSpanItem* fsi = mFullSpanItems.at(i);
        if (fsi->mPosition < positionStart) {
            continue;
        }
        fsi->mPosition += itemCount;
    }
}

int StaggeredGridLayoutManager::LazySpanLookup::invalidateFullSpansAfter(int position) {
    if (mFullSpanItems.empty()){// == nullptr) {
        return RecyclerView::NO_POSITION;
    }
    FullSpanItem* item = getFullSpanItem(position);
    // if there is an fsi at this position, get rid of it.
    if (item != nullptr) {
	auto it = std::find(mFullSpanItems.begin(),mFullSpanItems.end(),item);
        mFullSpanItems.erase(it);//.remove(item);
    }
    int nextFsiIndex = -1;
    const int count = (int)mFullSpanItems.size();
    for (int i = 0; i < count; i++) {
        FullSpanItem* fsi = mFullSpanItems.at(i);
        if (fsi->mPosition >= position) {
            nextFsiIndex = i;
            break;
        }
    }
    if (nextFsiIndex != -1) {
        FullSpanItem* fsi = mFullSpanItems.at(nextFsiIndex);
        mFullSpanItems.erase(mFullSpanItems.begin()+nextFsiIndex);//remove(nextFsiIndex);
        return fsi->mPosition;
    }
    return RecyclerView::NO_POSITION;
}

void StaggeredGridLayoutManager::LazySpanLookup::addFullSpanItem(FullSpanItem* fullSpanItem) {
    if (mFullSpanItems.empty()){// == null) {
        //mFullSpanItems = new ArrayList<>();
    }
    const size_t size = mFullSpanItems.size();
    for (size_t i = 0; i < size; i++) {
        FullSpanItem* other = mFullSpanItems.at(i);
        if (other->mPosition == fullSpanItem->mPosition) {
            if (_Debug) {
                FATAL("two fsis for same position");
            } else {
                mFullSpanItems.erase(mFullSpanItems.begin()+i);//.remove(i);
            }
        }
        if (other->mPosition >= fullSpanItem->mPosition) {
            mFullSpanItems.insert(mFullSpanItems.begin()+i,fullSpanItem);//add(i, fullSpanItem);
            return;
        }
    }
    // if it is not added to a position.
    mFullSpanItems.push_back(fullSpanItem);
}

StaggeredGridLayoutManager::LazySpanLookup::FullSpanItem* StaggeredGridLayoutManager::LazySpanLookup::getFullSpanItem(int position) {
    if (mFullSpanItems.empty()){// == nullptr) {
        return nullptr;
    }
    for (int i = int(mFullSpanItems.size() - 1); i >= 0; i--) {
        FullSpanItem* fsi = mFullSpanItems.at(i);
        if (fsi->mPosition == position) {
            return fsi;
        }
    }
    return nullptr;
}

StaggeredGridLayoutManager::LazySpanLookup::FullSpanItem* StaggeredGridLayoutManager::LazySpanLookup::getFirstFullSpanItemInRange(
	int minPos, int maxPos, int gapDir,bool hasUnwantedGapAfter) {
    if (mFullSpanItems.empty()){// == nullptr) {
        return nullptr;
    }
    const size_t limit = mFullSpanItems.size();
    for (size_t i = 0; i < limit; i++) {
        FullSpanItem* fsi = mFullSpanItems.at(i);
        if (fsi->mPosition >= maxPos) {
            return nullptr;
        }
        if (fsi->mPosition >= minPos
                && (gapDir == 0 || fsi->mGapDir == gapDir
                || (hasUnwantedGapAfter && fsi->mHasUnwantedGapAfter))) {
            return fsi;
        }
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

StaggeredGridLayoutManager::LazySpanLookup::FullSpanItem::FullSpanItem(Parcel& in) {
    mPosition = in.readInt();
    mGapDir = in.readInt();
    mHasUnwantedGapAfter = in.readInt() == 1;
    const int spanCount = in.readInt();
    if (spanCount > 0) {
        mGapPerSpan.resize(spanCount);// = new int[spanCount];
        //in.readIntArray(mGapPerSpan);
    }
}

StaggeredGridLayoutManager::LazySpanLookup::FullSpanItem::FullSpanItem() {
}

int StaggeredGridLayoutManager::LazySpanLookup::FullSpanItem::getGapForSpan(int spanIndex) {
    return mGapPerSpan.empty() ? 0 : mGapPerSpan[spanIndex];
}

int StaggeredGridLayoutManager::LazySpanLookup::FullSpanItem::describeContents() {
    return 0;
}

void StaggeredGridLayoutManager::LazySpanLookup::FullSpanItem::writeToParcel(Parcel& dest, int flags) {
    dest.writeInt(mPosition);
    dest.writeInt(mGapDir);
    dest.writeInt(mHasUnwantedGapAfter ? 1 : 0);
    if (mGapPerSpan.size()){
        dest.writeInt((int32_t)mGapPerSpan.size());
        //dest.writeIntArray(mGapPerSpan);
    } else {
        dest.writeInt(0);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

StaggeredGridLayoutManager::SavedState::SavedState() {
}

StaggeredGridLayoutManager::SavedState::SavedState(Parcel& in) {
    mAnchorPosition = in.readInt();
    mVisibleAnchorPosition = in.readInt();
    mSpanOffsetsSize = in.readInt();
    if (mSpanOffsetsSize > 0) {
        mSpanOffsets.resize(mSpanOffsetsSize);// = new int[mSpanOffsetsSize];
        //in.readIntArray(mSpanOffsets);
    }

    mSpanLookupSize = in.readInt();
    if (mSpanLookupSize > 0) {
        mSpanLookup.resize(mSpanLookupSize);// = new int[mSpanLookupSize];
        //in.readIntArray(mSpanLookup);
    }
    mReverseLayout = in.readInt() == 1;
    mAnchorLayoutFromEnd = in.readInt() == 1;
    mLastLayoutRTL = in.readInt() == 1;
    //noinspection unchecked
    //mFullSpanItems = in.readArrayList(LazySpanLookup.FullSpanItem.class.getClassLoader());
}

StaggeredGridLayoutManager::SavedState::SavedState(SavedState& other) {
    mSpanOffsetsSize = other.mSpanOffsetsSize;
    mAnchorPosition = other.mAnchorPosition;
    mVisibleAnchorPosition = other.mVisibleAnchorPosition;
    mSpanOffsets = other.mSpanOffsets;
    mSpanLookupSize = other.mSpanLookupSize;
    mSpanLookup = other.mSpanLookup;
    mReverseLayout = other.mReverseLayout;
    mAnchorLayoutFromEnd = other.mAnchorLayoutFromEnd;
    mLastLayoutRTL = other.mLastLayoutRTL;
    mFullSpanItems = other.mFullSpanItems;
}

void StaggeredGridLayoutManager::SavedState::invalidateSpanInfo() {
    mSpanOffsets.clear();// = null;
    mSpanOffsetsSize = 0;
    mSpanLookupSize = 0;
    mSpanLookup.clear();// = null;
    mFullSpanItems.clear();// = null;
}

void StaggeredGridLayoutManager::SavedState::invalidateAnchorPositionInfo() {
    mSpanOffsets.clear();// = null;
    mSpanOffsetsSize = 0;
    mAnchorPosition = RecyclerView::NO_POSITION;
    mVisibleAnchorPosition = RecyclerView::NO_POSITION;
}

int StaggeredGridLayoutManager::SavedState::describeContents() {
    return 0;
}

void StaggeredGridLayoutManager::SavedState::writeToParcel(Parcel& dest, int flags) {
    dest.writeInt(mAnchorPosition);
    dest.writeInt(mVisibleAnchorPosition);
    dest.writeInt(mSpanOffsetsSize);
    if (mSpanOffsetsSize > 0) {
        //dest.writeIntArray(mSpanOffsets);
    }
    dest.writeInt(mSpanLookupSize);
    if (mSpanLookupSize > 0) {
        //dest.writeIntArray(mSpanLookup);
    }
    dest.writeInt(mReverseLayout ? 1 : 0);
    dest.writeInt(mAnchorLayoutFromEnd ? 1 : 0);
    dest.writeInt(mLastLayoutRTL ? 1 : 0);
    //dest.writeList(mFullSpanItems);
}

////////////////////////////////////////////////////////////////////////////////////////////

StaggeredGridLayoutManager::AnchorInfo::AnchorInfo(StaggeredGridLayoutManager*lm) {
    mLM = lm;
    reset();
}

void StaggeredGridLayoutManager::AnchorInfo::reset() {
    mPosition = RecyclerView::NO_POSITION;
    mOffset = INVALID_OFFSET;
    mLayoutFromEnd = false;
    mInvalidateOffsets = false;
    mValid = false;
    for(int i=0;i<mSpanReferenceLines.size();i++)mSpanReferenceLines[i]=-1;
    //if (mSpanReferenceLines. != null)  Arrays.fill(mSpanReferenceLines, -1);
}

void StaggeredGridLayoutManager::AnchorInfo::saveSpanReferenceLines(std::vector<Span*>& spans) {
    const size_t spanCount = spans.size();
    mSpanReferenceLines.resize(spanCount);
    for (size_t i = 0; i < spanCount; i++) {
        // does not matter start or end since this is only recorded when span is reset
        mSpanReferenceLines[i] = spans[i]->getStartLine(Span::INVALID_LINE);
    }
}

void StaggeredGridLayoutManager::AnchorInfo::assignCoordinateFromPadding() {
    mOffset = mLayoutFromEnd ? mLM->mPrimaryOrientation->getEndAfterPadding()
            : mLM->mPrimaryOrientation->getStartAfterPadding();
}

void StaggeredGridLayoutManager::AnchorInfo::assignCoordinateFromPadding(int addedDistance) {
    if (mLayoutFromEnd) {
        mOffset = mLM->mPrimaryOrientation->getEndAfterPadding() - addedDistance;
    } else {
        mOffset = mLM->mPrimaryOrientation->getStartAfterPadding() + addedDistance;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////
StaggeredGridLayoutManager::BitSet::BitSet() {
    wordsInUse = 0;
    initWords(BITS_PER_WORD);
}
StaggeredGridLayoutManager::BitSet::BitSet(int nbits) {
    // nbits can't be negative; size 0 is OK
    LOGE_IF(nbits<0,"nbits %d< 0: ",nbits);
    wordsInUse = 0;
    initWords(nbits);
    sizeIsSticky = true;
}

void StaggeredGridLayoutManager::BitSet::initWords(int nbits) {
    wordSize = wordIndex(nbits-1) + 1;
    words = new uint32_t[wordSize];
    memset(words,0,sizeof(uint32_t)*wordSize);
}
void StaggeredGridLayoutManager::BitSet::expandTo(int wordIndex) {
    int wordsRequired = wordIndex+1;
    if (wordsInUse < wordsRequired) {
        ensureCapacity(wordsRequired);
        wordsInUse = wordsRequired;
    }
}

void StaggeredGridLayoutManager::BitSet::checkRange(int fromIndex, int toIndex) {
    FATAL_IF(fromIndex<0,"fromIndex(%d) < 0",fromIndex);
    FATAL_IF(toIndex<0,"toIndex(%d) < 0",toIndex);
    FATAL_IF(fromIndex > toIndex,"fromIndex: %d> toIndex:%d",fromIndex,toIndex);
}

void StaggeredGridLayoutManager::BitSet::set(int bitIndex) {
    if (bitIndex < 0)
    LOGE_IF(bitIndex<0,"bitIndex %d< 0: ",bitIndex);

    int wrdIndex = wordIndex(bitIndex);
    expandTo(wrdIndex);
    words[wrdIndex] |= (1 << bitIndex); // Restores invariants
    checkInvariants();
}

void StaggeredGridLayoutManager::BitSet::ensureCapacity(int wordsRequired) {
    if (wordSize < wordsRequired) {
        // Allocate larger of doubled size or required size
        int request = std::max(2 * wordSize, wordsRequired);
	uint32_t *newWords =new uint32_t[request];
	memcpy(newWords,words,sizeof(uint32_t)*wordSize);
	delete words;
        words = newWords;
        sizeIsSticky = false;
	wordSize = wordsRequired;
    }
}

void StaggeredGridLayoutManager::BitSet::set(int bitIndex, bool value) {
    if (value)
        set(bitIndex);
    else
        clear(bitIndex);
}

void StaggeredGridLayoutManager::BitSet::set(int fromIndex, int toIndex, bool value) {
    if (value)
        set(fromIndex, toIndex);
    else
        clear(fromIndex, toIndex);
}

static int numberOfLeadingZeros(unsigned int n) {
    if (n == 0) return 32;
    return __builtin_clz(n);
}
static int numberOfTrailingZeros(unsigned int n) {
    if (n == 0) return 32;
    return __builtin_ctz(n);
}
static int numberOfSetBits(unsigned int n) {
#if defined(__clang__)||defined(__GNUC__)
    return __builtin_popcount(n);
#elif defined(_WIN32)||defined(_WIN64)
    return __popcnt(n);
#endif
}

void StaggeredGridLayoutManager::BitSet::clear(int bitIndex) {
    FATAL_IF(bitIndex<0,"bitIndex(%d) < 0: ", bitIndex);

    int wrdIndex = wordIndex(bitIndex);
    if (wrdIndex >= wordsInUse)
        return;

    words[wrdIndex] &= ~(1U << bitIndex);

    recalculateWordsInUse();
    checkInvariants();
}

void StaggeredGridLayoutManager::BitSet::clear(int fromIndex, int toIndex) {
    checkRange(fromIndex, toIndex);

    if (fromIndex == toIndex)
        return;

    int startWordIndex = wordIndex(fromIndex);
    if (startWordIndex >= wordsInUse)
        return;

    int endWordIndex = wordIndex(toIndex - 1);
    if (endWordIndex >= wordsInUse) {
        toIndex = length();
        endWordIndex = wordsInUse - 1;
    }

    long firstWordMask = WORD_MASK << fromIndex;
    long lastWordMask  = WORD_MASK >>-toIndex;//>>> -toIndex;
    if (startWordIndex == endWordIndex) {
        // Case 1: One word
        words[startWordIndex] &= ~(firstWordMask & lastWordMask);
    } else {
        // Case 2: Multiple words
        // Handle first word
        words[startWordIndex] &= ~firstWordMask;

        // Handle intermediate words, if any
        for (int i = startWordIndex+1; i < endWordIndex; i++)
            words[i] = 0;

        // Handle last word
        words[endWordIndex] &= ~lastWordMask;
    }

    recalculateWordsInUse();
    checkInvariants();
}

void StaggeredGridLayoutManager::BitSet::clear() {
    while (wordsInUse > 0)
        words[--wordsInUse] = 0;
}

bool StaggeredGridLayoutManager::BitSet::get(int bitIndex) {
    FATAL_IF(bitIndex < 0,"bitIndex(%d) < 0 ",bitIndex);

    checkInvariants();

    int wrdIndex = wordIndex(bitIndex);
    return (wrdIndex < wordsInUse)
        && ((words[wrdIndex] & (1U << bitIndex)) != 0);
}

StaggeredGridLayoutManager::BitSet* StaggeredGridLayoutManager::BitSet::get(int fromIndex, int toIndex) {
    checkRange(fromIndex, toIndex);

    checkInvariants();

    int len = length();

    // If no set bits in range return empty bitset
    if (len <= fromIndex || fromIndex == toIndex)
        return new BitSet(0);

    // An optimization
    if (toIndex > len)
        toIndex = len;

    BitSet* result = new BitSet(toIndex - fromIndex);
    int targetWords = wordIndex(toIndex - fromIndex - 1) + 1;
    int sourceIndex = wordIndex(fromIndex);
    bool wordAligned = ((fromIndex & BIT_INDEX_MASK) == 0);

    // Process all words but the last word
    for (int i = 0; i < targetWords - 1; i++, sourceIndex++)
        result->words[i] = wordAligned ? words[sourceIndex] :
            (words[sourceIndex] >> fromIndex) |
            (words[sourceIndex+1] << -fromIndex);

    // Process the last word
    long lastWordMask = WORD_MASK >> -toIndex;
    result->words[targetWords - 1] =
        ((toIndex-1) & BIT_INDEX_MASK) < (fromIndex & BIT_INDEX_MASK)
        ? /* straddles source words */
        ((words[sourceIndex] >> fromIndex) |
         (words[sourceIndex+1] & lastWordMask) << -fromIndex)
        :
        ((words[sourceIndex] & lastWordMask) >> fromIndex);

    // Set wordsInUse correctly
    result->wordsInUse = targetWords;
    result->recalculateWordsInUse();
    result->checkInvariants();
    return result;
}

int StaggeredGridLayoutManager::BitSet::nextSetBit(int fromIndex) {
    FATAL_IF(fromIndex<0,"fromIndex(%d) < 0 ",fromIndex);

    checkInvariants();

    int u = wordIndex(fromIndex);
    if (u >= wordsInUse)
        return -1;

    uint32_t word = words[u] & (WORD_MASK << fromIndex);

    while (true) {
        if (word != 0)
            return (u * BITS_PER_WORD) + numberOfTrailingZeros(word);
        if (++u == wordsInUse)
            return -1;
        word = words[u];
    }
}

int StaggeredGridLayoutManager::BitSet::nextClearBit(int fromIndex) {
    // Neither spec nor implementation handle bitsets of maximal length.
    // See 4816253.
    FATAL_IF(fromIndex<0,"fromIndex(%d) < 0" ,fromIndex);

    checkInvariants();

    int u = wordIndex(fromIndex);
    if (u >= wordsInUse)
        return fromIndex;

    uint32_t word = ~words[u] & (WORD_MASK << fromIndex);

    while (true) {
        if (word != 0)
            return (u * BITS_PER_WORD) + numberOfTrailingZeros(word);
        if (++u == wordsInUse)
            return wordsInUse * BITS_PER_WORD;
        word = ~words[u];
    }
}
int StaggeredGridLayoutManager::BitSet::previousSetBit(int fromIndex) {
    if (fromIndex < 0) {
        if (fromIndex == -1)
            return -1;
        FATAL_IF(fromIndex<0,"fromIndex(%d) < -1",fromIndex);
    }

    checkInvariants();

    int u = wordIndex(fromIndex);
    if (u >= wordsInUse)
        return length() - 1;

    uint32_t word = words[u] & (WORD_MASK >> -(fromIndex+1));

    while (true) {
        if (word != 0)
            return (u+1) * BITS_PER_WORD - 1 - numberOfLeadingZeros(word);
        if (u-- == 0)
            return -1;
        word = words[u];
    }
}

int StaggeredGridLayoutManager::BitSet::previousClearBit(int fromIndex) {
    if (fromIndex < 0) {
        if (fromIndex == -1)
            return -1;
        FATAL_IF(fromIndex,"fromIndex(%d) < -1" ,fromIndex);
    }

    checkInvariants();

    int u = wordIndex(fromIndex);
    if (u >= wordsInUse)
        return fromIndex;

    uint32_t word = ~words[u] & (WORD_MASK >> -(fromIndex+1));

    while (true) {
        if (word != 0)
            return (u+1) * BITS_PER_WORD -1 - numberOfLeadingZeros(word);
        if (u-- == 0)
            return -1;
        word = ~words[u];
    }
}

int StaggeredGridLayoutManager::BitSet::length()const{
    if (wordsInUse == 0)
        return 0;

    return BITS_PER_WORD * (wordsInUse - 1) +
        (BITS_PER_WORD - numberOfLeadingZeros(words[wordsInUse - 1]));
}

bool StaggeredGridLayoutManager::BitSet::isEmpty()const {
    return wordsInUse == 0;
}

bool StaggeredGridLayoutManager::BitSet::intersects(const BitSet& set) {
    for (int i = std::min(wordsInUse, set.wordsInUse) - 1; i >= 0; i--)
        if ((words[i] & set.words[i]) != 0)
            return true;
    return false;
}

int StaggeredGridLayoutManager::BitSet::cardinality()const{
    int sum = 0;
    for (int i = 0; i < wordsInUse; i++)
        sum += numberOfSetBits(words[i]);//Long.bitCount(words[i]);
    return sum;
}

StaggeredGridLayoutManager::BitSet& StaggeredGridLayoutManager::BitSet::operator&&(const BitSet& set) {
    if (this == &set)
        return *this;

    while (wordsInUse > set.wordsInUse)
        words[--wordsInUse] = 0;

    // Perform logical AND on words in common
    for (int i = 0; i < wordsInUse; i++)
        words[i] &= set.words[i];

    recalculateWordsInUse();
    checkInvariants();
    return *this;
}

StaggeredGridLayoutManager::BitSet& StaggeredGridLayoutManager::BitSet::operator||(const BitSet& set) {
    if (this == &set)
        return *this;

    int wordsInCommon = std::min(wordsInUse, set.wordsInUse);

    if (wordsInUse < set.wordsInUse) {
        ensureCapacity(set.wordsInUse);
        wordsInUse = set.wordsInUse;
    }

    // Perform logical OR on words in common
    for (int i = 0; i < wordsInCommon; i++)
        words[i] |= set.words[i];

    // Copy any remaining words
    if (wordsInCommon < set.wordsInUse){
        /*System.arraycopy(set.words, wordsInCommon,
                         words, wordsInCommon,
                         wordsInUse - wordsInCommon);*/
		std::copy(set.words+wordsInCommon,
		    set.words+wordsInCommon + wordsInUse - wordsInCommon,
			words + wordsInCommon);
    }
    // recalculateWordsInUse() is unnecessary
    checkInvariants();
    return *this;
}

StaggeredGridLayoutManager::BitSet& StaggeredGridLayoutManager::BitSet::operator^(const BitSet& set) {
    int wordsInCommon = std::min(wordsInUse, set.wordsInUse);

    if (wordsInUse < set.wordsInUse) {
        ensureCapacity(set.wordsInUse);
        wordsInUse = set.wordsInUse;
    }

    // Perform logical XOR on words in common
    for (int i = 0; i < wordsInCommon; i++)
        words[i] ^= set.words[i];

    // Copy any remaining words
    if (wordsInCommon < set.wordsInUse){
        /*System.arraycopy(set.words, wordsInCommon,
                         words, wordsInCommon,
                         set.wordsInUse - wordsInCommon);*/
		std::copy(set.words+wordsInCommon,
		    set.words + wordsInCommon + set.wordsInUse - wordsInCommon,
			words + wordsInCommon);
    }
    recalculateWordsInUse();
    checkInvariants();
    return *this;
}

StaggeredGridLayoutManager::BitSet& StaggeredGridLayoutManager::BitSet::andNot(const BitSet& set) {
    // Perform logical (a & !b) on words in common
    for (int i = std::min(wordsInUse, set.wordsInUse) - 1; i >= 0; i--)
        words[i] &= ~set.words[i];

    recalculateWordsInUse();
    checkInvariants();
	return *this;
}


int StaggeredGridLayoutManager::BitSet::hashCode()const{
    long long h = 1234;
    for (int i = wordsInUse; --i >= 0; )
        h ^= words[i] * (i + 1);

    return (int)((h >> 32) ^ h);
}

int StaggeredGridLayoutManager::BitSet::size()const{
    return wordSize * BITS_PER_WORD;
}

int StaggeredGridLayoutManager::BitSet::wordIndex(int bitIndex) {
    return bitIndex >> ADDRESS_BITS_PER_WORD;
}

void StaggeredGridLayoutManager::BitSet::checkInvariants() {
    assert(wordsInUse == 0 || words[wordsInUse - 1] != 0);
    assert(wordsInUse >= 0 && wordsInUse <= wordSize);
    assert(wordsInUse == wordSize || words[wordsInUse] == 0);
}

void StaggeredGridLayoutManager::BitSet::recalculateWordsInUse() {
    int i;
    for (i = wordsInUse-1; i >= 0; i--)
        if (words[i] != 0)
            break;

    wordsInUse = i+1; // The new logical size
}

}/*endof namespace*/
