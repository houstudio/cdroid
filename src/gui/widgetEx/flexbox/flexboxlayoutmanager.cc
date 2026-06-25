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
#include <widgetEx/flexbox/flexboxlayoutmanager.h>
#include <view/viewgroup.h>
#include <algorithm>
#include <core/context.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include <widgetEx/recyclerview/orientationhelper.h>
#include <widgetEx/recyclerview/layoutstate.h>
#include <widgetEx/recyclerview/linearsmoothscroller.h>

namespace cdroid {

FlexboxLayoutManager::FlexboxLayoutManager(Context* context)
    : FlexboxLayoutManager(context, (int)FlexDirection::ROW, (int)FlexWrap::WRAP) {
}

FlexboxLayoutManager::FlexboxLayoutManager(Context* context, int flexDirection)
    : FlexboxLayoutManager(context, flexDirection, (int)FlexWrap::WRAP) {
}

FlexboxLayoutManager::FlexboxLayoutManager(Context* context, int flexDirection, int flexWrap)
    : mFlexDirection(flexDirection), mFlexWrap(flexWrap), mAlignItems((int)AlignItems::STRETCH),
      mContext(context) {
    init();
}

FlexboxLayoutManager::FlexboxLayoutManager(Context* context, const AttributeSet& attrs)
    : mContext(context) {
    auto properties = getProperties(context, attrs, 0, 0);
    switch (properties.orientation) {
        case LinearLayoutManager::HORIZONTAL:
            mFlexDirection = properties.reverseLayout
                    ? (int)FlexDirection::ROW_REVERSE : (int)FlexDirection::ROW;
            break;
        case LinearLayoutManager::VERTICAL:
            mFlexDirection = properties.reverseLayout
                    ? (int)FlexDirection::COLUMN_REVERSE : (int)FlexDirection::COLUMN;
            break;
        default:
            mFlexDirection = (int)FlexDirection::ROW;
            break;
    }
    mFlexWrap = (int)FlexWrap::WRAP;
    mAlignItems = (int)AlignItems::STRETCH;
    init();
}

void FlexboxLayoutManager::init() {
    ensureFlexboxHelper();
    mFlexLinesResult = new FlexboxHelper::FlexLinesResult();
    mAnchorInfo = new AnchorInfo();
}

FlexboxLayoutManager::~FlexboxLayoutManager() {
    delete mFlexLinesResult;
    delete mFlexboxHelper;
    delete mOrientationHelper;
    delete mSubOrientationHelper;
    delete mAnchorInfo;
}

void FlexboxLayoutManager::ensureFlexboxHelper() {
    if (mFlexboxHelper == nullptr) {
        mFlexboxHelper = new FlexboxHelper(this);
    }
}

void FlexboxLayoutManager::ensureOrientationHelper() {
    if (mOrientationHelper != nullptr) {
        return;
    }
    if (isMainAxisDirectionHorizontal()) {
        if (mFlexWrap == FlexWrap::NOWRAP) {
            mOrientationHelper = OrientationHelper::createHorizontalHelper(this);
            mSubOrientationHelper = OrientationHelper::createVerticalHelper(this);
        } else {
            mOrientationHelper = OrientationHelper::createVerticalHelper(this);
            mSubOrientationHelper = OrientationHelper::createHorizontalHelper(this);
        }
    } else {
        if (mFlexWrap == FlexWrap::NOWRAP) {
            mOrientationHelper = OrientationHelper::createVerticalHelper(this);
            mSubOrientationHelper = OrientationHelper::createHorizontalHelper(this);
        } else {
            mOrientationHelper = OrientationHelper::createHorizontalHelper(this);
            mSubOrientationHelper = OrientationHelper::createVerticalHelper(this);
        }
    }
}

void FlexboxLayoutManager::ensureLayoutState() {
    if (mLayoutState == nullptr) {
        mLayoutState = new LayoutState();
    }
}

void FlexboxLayoutManager::updateLayoutState(int layoutDirection, int absDelta) {
    mLayoutState->mLayoutDirection = layoutDirection;
    bool mainAxisHorizontal = isMainAxisDirectionHorizontal();

    int widthMeasureSpec = View::MeasureSpec::makeMeasureSpec(getWidth(), getWidthMode());
    int heightMeasureSpec = View::MeasureSpec::makeMeasureSpec(getHeight(), getHeightMode());
    bool columnAndRtl = !mainAxisHorizontal && mIsRtl;

    if (layoutDirection == LayoutState::LAYOUT_END) {
        View* lastVisible = getChildAt(getChildCount() - 1);
        if (lastVisible == nullptr) {
            return;
        }
        mLayoutState->mOffset = mOrientationHelper->getDecoratedEnd(lastVisible);
        int lastVisiblePosition = getPosition(lastVisible);
        const std::vector<int>& indexToFlexLine = mFlexboxHelper->getIndexToFlexLine();
        int lastVisibleLinePosition = indexToFlexLine[lastVisiblePosition];
        FlexLine* lastVisibleLine = &mFlexLines[lastVisibleLinePosition];

        View* referenceView = findLastReferenceViewInLine(lastVisible, lastVisibleLine);
        mLayoutState->mItemDirection = LayoutState::ITEM_DIRECTION_TAIL;
        mLayoutState->mPosition = lastVisiblePosition + mLayoutState->mItemDirection;
        if (indexToFlexLine.size() <= mLayoutState->mPosition) {
            mLayoutState->mFlexLinePosition = RecyclerView::NO_POSITION;
        } else {
            mLayoutState->mFlexLinePosition = indexToFlexLine[mLayoutState->mPosition];
        }

        if (columnAndRtl) {
            mLayoutState->mOffset = mOrientationHelper->getDecoratedStart(referenceView);
            mLayoutState->mScrollingOffset = -mOrientationHelper->getDecoratedStart(referenceView)
                    + mOrientationHelper->getStartAfterPadding();
            mLayoutState->mScrollingOffset = std::max(mLayoutState->mScrollingOffset, 0);
        } else {
            mLayoutState->mOffset = mOrientationHelper->getDecoratedEnd(referenceView);
            mLayoutState->mScrollingOffset = mOrientationHelper->getDecoratedEnd(referenceView)
                    - mOrientationHelper->getEndAfterPadding();
        }

        if ((mLayoutState->mFlexLinePosition == RecyclerView::NO_POSITION
                || mLayoutState->mFlexLinePosition > (int)mFlexLines.size() - 1) &&
                mLayoutState->mPosition <= getFlexItemCount()) {
            int needsToFill = absDelta - mLayoutState->mScrollingOffset;
            mFlexLinesResult->reset();
            if (needsToFill > 0) {
                if (mainAxisHorizontal) {
                    mFlexboxHelper->calculateHorizontalFlexLines(mFlexLinesResult,
                            widthMeasureSpec, heightMeasureSpec, needsToFill,
                            mLayoutState->mPosition, &mFlexLines);
                } else {
                    mFlexboxHelper->calculateVerticalFlexLines(mFlexLinesResult,
                            widthMeasureSpec, heightMeasureSpec, needsToFill,
                            mLayoutState->mPosition, &mFlexLines);
                }
                mFlexboxHelper->determineMainSize(widthMeasureSpec, heightMeasureSpec,
                        mLayoutState->mPosition);
                mFlexboxHelper->stretchViews(mLayoutState->mPosition);
            }
        }
    } else {
        View* firstVisible = getChildAt(0);
        if (firstVisible == nullptr) {
            return;
        }
        mLayoutState->mOffset = mOrientationHelper->getDecoratedStart(firstVisible);
        int firstVisiblePosition = getPosition(firstVisible);
        const std::vector<int>& indexToFlexLine = mFlexboxHelper->getIndexToFlexLine();
        int firstVisibleLinePosition = indexToFlexLine[firstVisiblePosition];
        FlexLine* firstVisibleLine = &mFlexLines[firstVisibleLinePosition];

        View* referenceView = findFirstReferenceViewInLine(firstVisible, firstVisibleLine);

        mLayoutState->mItemDirection = LayoutState::ITEM_DIRECTION_TAIL;
        int flexLinePosition = indexToFlexLine[firstVisiblePosition];
        if (flexLinePosition == RecyclerView::NO_POSITION) {
            flexLinePosition = 0;
        }
        if (flexLinePosition > 0) {
            FlexLine* previousLine = &mFlexLines[flexLinePosition - 1];
            mLayoutState->mPosition = firstVisiblePosition - previousLine->getItemCount();
        } else {
            mLayoutState->mPosition = RecyclerView::NO_POSITION;
        }
        mLayoutState->mFlexLinePosition = flexLinePosition > 0 ? flexLinePosition - 1 : 0;

        if (columnAndRtl) {
            mLayoutState->mOffset = mOrientationHelper->getDecoratedEnd(referenceView);
            mLayoutState->mScrollingOffset = mOrientationHelper->getDecoratedEnd(referenceView)
                    - mOrientationHelper->getEndAfterPadding();
            mLayoutState->mScrollingOffset = std::max(mLayoutState->mScrollingOffset, 0);
        } else {
            mLayoutState->mOffset = mOrientationHelper->getDecoratedStart(referenceView);
            mLayoutState->mScrollingOffset = -mOrientationHelper->getDecoratedStart(referenceView)
                    + mOrientationHelper->getStartAfterPadding();
        }
    }
    mLayoutState->mAvailable = absDelta - mLayoutState->mScrollingOffset;
}

void FlexboxLayoutManager::updateLayoutStateToFillEnd(AnchorInfo* anchorInfo, bool fromNextLine, bool considerInfinite) {
    if (considerInfinite) {
        resolveInfiniteAmount();
    } else {
        mLayoutState->mInfinite = false;
    }
    if (!isMainAxisDirectionHorizontal() && mIsRtl) {
        mLayoutState->mAvailable = anchorInfo->mCoordinate - getPaddingRight();
    } else {
        mLayoutState->mAvailable = mOrientationHelper->getEndAfterPadding() - anchorInfo->mCoordinate;
    }
    mLayoutState->mPosition = anchorInfo->mPosition;
    mLayoutState->mItemDirection = LayoutState::ITEM_DIRECTION_TAIL;
    mLayoutState->mLayoutDirection = LayoutState::LAYOUT_END;
    mLayoutState->mOffset = anchorInfo->mCoordinate;
    mLayoutState->mScrollingOffset = LayoutState::SCROLLING_OFFSET_NaN;
    mLayoutState->mFlexLinePosition = anchorInfo->mFlexLinePosition;

    if (fromNextLine && mFlexLines.size() > 1
            && anchorInfo->mFlexLinePosition >= 0
            && anchorInfo->mFlexLinePosition < (int)mFlexLines.size() - 1) {
        FlexLine& currentLine = mFlexLines[anchorInfo->mFlexLinePosition];
        mLayoutState->mFlexLinePosition++;
        mLayoutState->mPosition += currentLine.getItemCount();
    }
}

void FlexboxLayoutManager::updateLayoutStateToFillStart(AnchorInfo* anchorInfo, bool fromPreviousLine, bool considerInfinite) {
    if (considerInfinite) {
        resolveInfiniteAmount();
    } else {
        mLayoutState->mInfinite = false;
    }
    if (!isMainAxisDirectionHorizontal() && mIsRtl) {
        mLayoutState->mAvailable = (mParent != nullptr ? mParent->getWidth() : 0)
                - anchorInfo->mCoordinate - mOrientationHelper->getStartAfterPadding();
    } else {
        mLayoutState->mAvailable = anchorInfo->mCoordinate - mOrientationHelper->getStartAfterPadding();
    }
    mLayoutState->mPosition = anchorInfo->mPosition;
    mLayoutState->mItemDirection = LayoutState::ITEM_DIRECTION_TAIL;
    mLayoutState->mLayoutDirection = LayoutState::LAYOUT_START;
    mLayoutState->mOffset = anchorInfo->mCoordinate;
    mLayoutState->mScrollingOffset = LayoutState::SCROLLING_OFFSET_NaN;
    mLayoutState->mFlexLinePosition = anchorInfo->mFlexLinePosition;

    if (fromPreviousLine && anchorInfo->mFlexLinePosition > 0
            && mFlexLines.size() > (size_t)anchorInfo->mFlexLinePosition) {
        FlexLine& currentLine = mFlexLines[anchorInfo->mFlexLinePosition];
        mLayoutState->mFlexLinePosition--;
        mLayoutState->mPosition -= currentLine.getItemCount();
    }
}

View* FlexboxLayoutManager::findFirstReferenceViewInLine(View* firstView, FlexLine* firstVisibleLine) {
    bool mainAxisHorizontal = isMainAxisDirectionHorizontal();
    View* referenceView = firstView;
    for (int i = 1, to = firstVisibleLine->getItemCount(); i < to; i++) {
        View* viewInSameLine = getChildAt(i);
        if (viewInSameLine == nullptr || viewInSameLine->getVisibility() == View::GONE) {
            continue;
        }
        if (mIsRtl && !mainAxisHorizontal) {
            if (mOrientationHelper->getDecoratedEnd(referenceView)
                    < mOrientationHelper->getDecoratedEnd(viewInSameLine)) {
                referenceView = viewInSameLine;
            }
        } else {
            if (mOrientationHelper->getDecoratedStart(referenceView)
                    > mOrientationHelper->getDecoratedStart(viewInSameLine)) {
                referenceView = viewInSameLine;
            }
        }
    }
    return referenceView;
}

View* FlexboxLayoutManager::findLastReferenceViewInLine(View* lastView, FlexLine* lastVisibleLine) {
    bool mainAxisHorizontal = isMainAxisDirectionHorizontal();
    View* referenceView = lastView;
    for (int i = getChildCount() - 2, to = getChildCount() - lastVisibleLine->getItemCount() - 1;
            i > to; i--) {
        View* viewInSameLine = getChildAt(i);
        if (viewInSameLine == nullptr || viewInSameLine->getVisibility() == View::GONE) {
            continue;
        }
        if (mIsRtl && !mainAxisHorizontal) {
            if (mOrientationHelper->getDecoratedStart(referenceView) >
                    mOrientationHelper->getDecoratedStart(viewInSameLine)) {
                referenceView = viewInSameLine;
            }
        } else {
            if (mOrientationHelper->getDecoratedEnd(referenceView) <
                    mOrientationHelper->getDecoratedEnd(viewInSameLine)) {
                referenceView = viewInSameLine;
            }
        }
    }
    return referenceView;
}

bool FlexboxLayoutManager::isAutoMeasureEnabled() const {
    return true;
}

RecyclerView::LayoutParams* FlexboxLayoutManager::generateDefaultLayoutParams() const {
    return new LayoutParams(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
}

bool FlexboxLayoutManager::canScrollHorizontally() const {
    if (mFlexWrap == (int)FlexWrap::NOWRAP) {
        return isMainAxisDirectionHorizontal();
    } else {
        int parentWidth = mParent != nullptr ? mParent->getWidth() : 0;
        return !isMainAxisDirectionHorizontal() || getWidth() > parentWidth;
    }
}

bool FlexboxLayoutManager::canScrollVertically() const {
    if (mFlexWrap == (int)FlexWrap::NOWRAP) {
        return !isMainAxisDirectionHorizontal();
    } else {
        int parentHeight = mParent != nullptr ? mParent->getHeight() : 0;
        return isMainAxisDirectionHorizontal() || getHeight() > parentHeight;
    }
}

void FlexboxLayoutManager::onLayoutChildren(RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    mRecycler = &recycler;
    mState = &state;
    int childCount = state.getItemCount();
    if (childCount == 0 && state.isPreLayout()) {
        return;
    }
    ensureFlexboxHelper();
    resolveLayoutDirection();
    ensureOrientationHelper();
    ensureLayoutState();
    mFlexboxHelper->ensureMeasureSpecCache(childCount);
    mFlexboxHelper->ensureMeasuredSizeCache(childCount);

    mFlexboxHelper->ensureIndexToFlexLine(childCount);

    mLayoutState->mShouldRecycle = false;

    if (mPendingSavedState != nullptr && mPendingSavedState->hasValidAnchor(childCount)) {
        mPendingScrollPosition = mPendingSavedState->mAnchorPosition;
    }

    if (!mAnchorInfo->mValid || mPendingScrollPosition != RecyclerView::NO_POSITION ||
            mPendingSavedState != nullptr) {
        mAnchorInfo->reset(this);
        updateAnchorInfoForLayout(state, mAnchorInfo);
        mAnchorInfo->mValid = true;
    }

    detachAndScrapAttachedViews(recycler);

    if (mAnchorInfo->mLayoutFromEnd) {
        updateLayoutStateToFillStart(mAnchorInfo, false, true);
    } else {
        updateLayoutStateToFillEnd(mAnchorInfo, false, true);
    }

    updateFlexLines(childCount);

    int startOffset;
    int endOffset;
    int filledToEnd = fill(recycler, *mLayoutState, state, false);
    if (mAnchorInfo->mLayoutFromEnd) {
        startOffset = mLayoutState->mOffset;
        updateLayoutStateToFillEnd(mAnchorInfo, true, false);
        int filledToStart = fill(recycler, *mLayoutState, state, false);
        endOffset = mLayoutState->mOffset;
    } else {
        endOffset = mLayoutState->mOffset;
        updateLayoutStateToFillStart(mAnchorInfo, true, false);
        int filledToStart = fill(recycler, *mLayoutState, state, false);
        startOffset = mLayoutState->mOffset;
    }

    if (getChildCount() > 0) {
        if (mAnchorInfo->mLayoutFromEnd) {
            int fixOffset = fixLayoutEndGap(endOffset, recycler, state, true);
            startOffset += fixOffset;
            fixLayoutStartGap(startOffset, recycler, state, false);
        } else {
            int fixOffset = fixLayoutStartGap(startOffset, recycler, state, true);
            endOffset += fixOffset;
            fixLayoutEndGap(endOffset, recycler, state, false);
        }
    }
}

void FlexboxLayoutManager::onLayoutCompleted(RecyclerView::State& state) {
    LayoutManager::onLayoutCompleted(state);
    mPendingSavedState = nullptr;
    mPendingScrollPosition = RecyclerView::NO_POSITION;
    mPendingScrollPositionOffset = INT_MIN;
    mDirtyPosition = RecyclerView::NO_POSITION;
    mAnchorInfo->reset(this);
    mViewCache.clear();
}

void FlexboxLayoutManager::scrollToPosition(int position) {
    mPendingScrollPosition = position;
    mPendingScrollPositionOffset = INT_MIN;
    if (mPendingSavedState != nullptr) {
        mPendingSavedState->invalidateAnchor();
    }
    requestLayout();
}

void FlexboxLayoutManager::smoothScrollToPosition(RecyclerView& recyclerView, RecyclerView::State& state, int position) {
    LinearSmoothScroller* linearSmoothScroller = new LinearSmoothScroller(recyclerView.getContext());
    linearSmoothScroller->setTargetPosition(position);
    startSmoothScroll(linearSmoothScroller);
}

void FlexboxLayoutManager::scrollToPositionWithOffset(int position, int offset) {
    mPendingScrollPosition = position;
    mPendingScrollPositionOffset = offset;
    requestLayout();
}

int FlexboxLayoutManager::scrollHorizontallyBy(int dx, RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    if (!isMainAxisDirectionHorizontal() || (mFlexWrap == (int)FlexWrap::NOWRAP)) {
        int scrolled = handleScrollingMainOrientation(dx, recycler, state);
        mViewCache.clear();
        return scrolled;
    } else {
        int scrolled = handleScrollingSubOrientation(dx);
        mAnchorInfo->mPerpendicularCoordinate += scrolled;
        mSubOrientationHelper->offsetChildren(-scrolled);
        return scrolled;
    }
}

int FlexboxLayoutManager::scrollVerticallyBy(int dy, RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    if (isMainAxisDirectionHorizontal() ||
            (mFlexWrap == (int)FlexWrap::NOWRAP && !isMainAxisDirectionHorizontal())) {
        int scrolled = handleScrollingMainOrientation(dy, recycler, state);
        mViewCache.clear();
        return scrolled;
    } else {
        int scrolled = handleScrollingSubOrientation(dy);
        mAnchorInfo->mPerpendicularCoordinate += scrolled;
        mSubOrientationHelper->offsetChildren(-scrolled);
        return scrolled;
    }
}

int FlexboxLayoutManager::handleScrollingMainOrientation(int delta, RecyclerView::Recycler& recycler,
        RecyclerView::State& state) {
    if (getChildCount() == 0 || delta == 0) {
        return 0;
    }
    ensureOrientationHelper();
    mLayoutState->mShouldRecycle = true;
    int layoutDirection;
    bool columnAndRtl = !isMainAxisDirectionHorizontal() && mIsRtl;
    if (columnAndRtl) {
        layoutDirection = delta < 0 ? LayoutState::LAYOUT_END : LayoutState::LAYOUT_START;
    } else {
        layoutDirection = delta > 0 ? LayoutState::LAYOUT_END : LayoutState::LAYOUT_START;
    }
    int absDelta = std::abs(delta);

    updateLayoutState(layoutDirection, absDelta);

    int freeScroll = mLayoutState->mScrollingOffset;
    int consumed = freeScroll + fill(recycler, *mLayoutState, state, false);
    if (consumed < 0) {
        return 0;
    }
    int scrolled;
    if (columnAndRtl) {
        scrolled = absDelta > consumed ? -layoutDirection * consumed : delta;
    } else {
        scrolled = absDelta > consumed ? layoutDirection * consumed : delta;
    }
    mOrientationHelper->offsetChildren(-scrolled);
    mLayoutState->mLastScrollDelta = scrolled;
    return scrolled;
}

int FlexboxLayoutManager::handleScrollingSubOrientation(int delta) {
    if (getChildCount() == 0 || delta == 0) {
        return 0;
    }
    ensureOrientationHelper();
    bool isMainAxisHorizontal = isMainAxisDirectionHorizontal();
    int parentLength = isMainAxisHorizontal ? (mParent != nullptr ? mParent->getWidth() : 0) : (mParent != nullptr ? mParent->getHeight() : 0);
    int mainAxisLength = isMainAxisHorizontal ? getWidth() : getHeight();

    bool layoutRtl = (getLayoutDirection() == View::LAYOUT_DIRECTION_RTL);
    if (layoutRtl) {
        int absDelta = std::abs(delta);
        if (delta < 0) {
            delta = std::min(mainAxisLength + mAnchorInfo->mPerpendicularCoordinate - parentLength, absDelta);
            delta = -delta;
        } else {
            delta = mAnchorInfo->mPerpendicularCoordinate + delta > 0
                    ? -mAnchorInfo->mPerpendicularCoordinate
                    : delta;
        }
    } else {
        if (delta > 0) {
            delta = std::min(mainAxisLength - mAnchorInfo->mPerpendicularCoordinate - parentLength, delta);
        } else {
            delta = mAnchorInfo->mPerpendicularCoordinate + delta >= 0 ? delta :
                    -mAnchorInfo->mPerpendicularCoordinate;
        }
    }
    return delta;
}

int FlexboxLayoutManager::fixLayoutStartGap(int startOffset, RecyclerView::Recycler& recycler,
        RecyclerView::State& state, bool canOffsetChildren) {
    int gap;
    int fixOffset;
    if (!isMainAxisDirectionHorizontal() && mIsRtl) {
        gap = mOrientationHelper->getEndAfterPadding() - startOffset;
        if (gap > 0) {
            fixOffset = handleScrollingMainOrientation(-gap, recycler, state);
        } else {
            return 0;
        }
    } else {
        gap = startOffset - mOrientationHelper->getStartAfterPadding();
        if (gap > 0) {
            fixOffset = -handleScrollingMainOrientation(gap, recycler, state);
        } else {
            return 0;
        }
    }
    startOffset += fixOffset;
    if (canOffsetChildren) {
        gap = startOffset - mOrientationHelper->getStartAfterPadding();
        if (gap > 0) {
            mOrientationHelper->offsetChildren(-gap);
            return fixOffset - gap;
        }
    }
    return fixOffset;
}

int FlexboxLayoutManager::fixLayoutEndGap(int endOffset, RecyclerView::Recycler& recycler,
        RecyclerView::State& state, bool canOffsetChildren) {
    int gap;
    bool columnAndRtl = !isMainAxisDirectionHorizontal() && mIsRtl;
    int fixOffset;
    if (columnAndRtl) {
        gap = endOffset - mOrientationHelper->getStartAfterPadding();
        if (gap > 0) {
            fixOffset = handleScrollingMainOrientation(gap, recycler, state);
        } else {
            return 0;
        }
    } else {
        gap = mOrientationHelper->getEndAfterPadding() - endOffset;
        if (gap > 0) {
            fixOffset = -handleScrollingMainOrientation(-gap, recycler, state);
        } else {
            return 0;
        }
    }

    endOffset += fixOffset;
    if (canOffsetChildren) {
        gap = mOrientationHelper->getEndAfterPadding() - endOffset;
        if (gap > 0) {
            mOrientationHelper->offsetChildren(gap);
            return gap + fixOffset;
        }
    }
    return fixOffset;
}

void FlexboxLayoutManager::updateFlexLines(int childCount) {
    int widthMeasureSpec = View::MeasureSpec::makeMeasureSpec(getWidth(), getWidthMode());
    int heightMeasureSpec = View::MeasureSpec::makeMeasureSpec(getHeight(), getHeightMode());
    int width = getWidth();
    int height = getHeight();
    bool isMainSizeChanged;
    int needsToFill;

    if (isMainAxisDirectionHorizontal()) {
        isMainSizeChanged = mLastWidth != INT_MIN && mLastWidth != width;
        needsToFill = mLayoutState->mInfinite
                ? (mContext != nullptr ? mContext->getDisplayMetrics().heightPixels : height)
                : mLayoutState->mAvailable;
    } else {
        isMainSizeChanged = mLastHeight != INT_MIN && mLastHeight != height;
        needsToFill = mLayoutState->mInfinite
                ? (mContext != nullptr ? mContext->getDisplayMetrics().widthPixels : width)
                : mLayoutState->mAvailable;
    }

    mLastWidth = width;
    mLastHeight = height;

    if (mDirtyPosition == RecyclerView::NO_POSITION &&
            (mPendingScrollPosition != RecyclerView::NO_POSITION || isMainSizeChanged)) {
        if (mAnchorInfo->mLayoutFromEnd) {
            return;
        }
        mFlexLines.clear();
        mFlexLinesResult->reset();
        if (isMainAxisDirectionHorizontal()) {
            mFlexboxHelper->calculateHorizontalFlexLinesToIndex(mFlexLinesResult,
                    widthMeasureSpec, heightMeasureSpec,
                    needsToFill, mAnchorInfo->mPosition, &mFlexLines);
        } else {
            mFlexboxHelper->calculateVerticalFlexLinesToIndex(mFlexLinesResult,
                    widthMeasureSpec, heightMeasureSpec,
                    needsToFill, mAnchorInfo->mPosition, &mFlexLines);
        }
        mFlexLines = mFlexLinesResult->mFlexLines;
        mFlexboxHelper->determineMainSize(widthMeasureSpec, heightMeasureSpec);
        mFlexboxHelper->stretchViews();
        mAnchorInfo->mFlexLinePosition =
                mFlexboxHelper->getIndexToFlexLine()[mAnchorInfo->mPosition];
        mLayoutState->mFlexLinePosition = mAnchorInfo->mFlexLinePosition;
    } else {
        int fromIndex = mDirtyPosition != RecyclerView::NO_POSITION ?
                std::min(mDirtyPosition, mAnchorInfo->mPosition) : mAnchorInfo->mPosition;

        mFlexLinesResult->reset();
        if (isMainAxisDirectionHorizontal()) {
            if (!mFlexLines.empty()) {
                mFlexboxHelper->clearFlexLines(mFlexLines, fromIndex);
                mFlexboxHelper->calculateFlexLines(mFlexLinesResult, widthMeasureSpec,
                        heightMeasureSpec, needsToFill, fromIndex, mAnchorInfo->mPosition,
                        &mFlexLines);
            } else {
                mFlexboxHelper->ensureIndexToFlexLine(childCount);
                mFlexboxHelper->calculateHorizontalFlexLines(mFlexLinesResult,
                        widthMeasureSpec, heightMeasureSpec,
                        needsToFill, 0, &mFlexLines);
            }
        } else {
            if (!mFlexLines.empty()) {
                mFlexboxHelper->clearFlexLines(mFlexLines, fromIndex);
                mFlexboxHelper->calculateFlexLines(mFlexLinesResult, heightMeasureSpec,
                        widthMeasureSpec, needsToFill, fromIndex, mAnchorInfo->mPosition,
                        &mFlexLines);
            } else {
                mFlexboxHelper->ensureIndexToFlexLine(childCount);
                mFlexboxHelper->calculateVerticalFlexLines(mFlexLinesResult, widthMeasureSpec,
                        heightMeasureSpec, needsToFill, 0, &mFlexLines);
            }
        }
        mFlexLines = mFlexLinesResult->mFlexLines;
        mFlexboxHelper->determineMainSize(widthMeasureSpec, heightMeasureSpec,
                fromIndex);
        mFlexboxHelper->stretchViews(fromIndex);
    }
}

void FlexboxLayoutManager::recycleChildren(RecyclerView::Recycler& recycler, int startIndex, int endIndex) {
    for (int i = endIndex; i >= startIndex; i--) {
        removeAndRecycleViewAt(i, recycler);
    }
}

int FlexboxLayoutManager::fill(RecyclerView::Recycler& recycler, LayoutState& layoutState, RecyclerView::State& state, bool stopOnFocusable) {
    if (layoutState.mScrollingOffset != LayoutState::SCROLLING_OFFSET_NaN) {
        if (layoutState.mAvailable < 0) {
            layoutState.mScrollingOffset += layoutState.mAvailable;
        }
        recycleByLayoutState(recycler, layoutState);
    }
    int start = layoutState.mAvailable;
    int remainingSpace = layoutState.mAvailable;
    int consumed = 0;
    bool mainAxisHorizontal = isMainAxisDirectionHorizontal();
    while ((remainingSpace > 0 || mLayoutState->mInfinite) &&
            layoutState.hasMore(state, mFlexLines)) {
        FlexLine* flexLine = &mFlexLines[layoutState.mFlexLinePosition];
        layoutState.mPosition = flexLine->getFirstIndex();
        consumed += layoutFlexLine(*flexLine, layoutState, recycler);

        if (!mainAxisHorizontal && mIsRtl) {
            layoutState.mOffset -= flexLine->getCrossSize() * layoutState.mLayoutDirection;
        } else {
            layoutState.mOffset += flexLine->getCrossSize() * layoutState.mLayoutDirection;
        }

        remainingSpace -= flexLine->getCrossSize();
    }
    layoutState.mAvailable -= consumed;
    if (layoutState.mScrollingOffset != LayoutState::SCROLLING_OFFSET_NaN) {
        layoutState.mScrollingOffset += consumed;
        if (layoutState.mAvailable < 0) {
            layoutState.mScrollingOffset += layoutState.mAvailable;
        }
        recycleByLayoutState(recycler, layoutState);
    }
    return start - layoutState.mAvailable;
}

void FlexboxLayoutManager::recycleByLayoutState(RecyclerView::Recycler& recycler, LayoutState& layoutState) {
    if (!layoutState.mShouldRecycle) {
        return;
    }
    if (layoutState.mLayoutDirection == LayoutState::LAYOUT_START) {
        recycleFlexLinesFromEnd(recycler, layoutState);
    } else {
        recycleFlexLinesFromStart(recycler, layoutState);
    }
}

void FlexboxLayoutManager::recycleFlexLinesFromStart(RecyclerView::Recycler& recycler, LayoutState& layoutState) {
    if (layoutState.mScrollingOffset < 0) {
        return;
    }
    const std::vector<int>& indexToFlexLine = mFlexboxHelper->getIndexToFlexLine();
    int childCount = getChildCount();
    if (childCount == 0) {
        return;
    }
    View* firstView = getChildAt(0);
    if (firstView == nullptr) {
        return;
    }
    int currentLineIndex = indexToFlexLine[getPosition(firstView)];
    if (currentLineIndex == RecyclerView::NO_POSITION) {
        return;
    }
    FlexLine* flexLine = &mFlexLines[currentLineIndex];
    int recycleTo = -1;
    for (int i = 0; i < childCount; i++) {
        View* view = getChildAt(i);
        if (view == nullptr) {
            continue;
        }
        if (canViewBeRecycledFromStart(view, layoutState.mScrollingOffset)) {
            if (flexLine->getLastIndex() == getPosition(view)) {
                recycleTo = i;
                if (currentLineIndex >= (int)mFlexLines.size() - 1) {
                    break;
                } else {
                    currentLineIndex += layoutState.mLayoutDirection;
                    flexLine = &mFlexLines[currentLineIndex];
                }
            }
        } else {
            break;
        }
    }
    recycleChildren(recycler, 0, recycleTo);
}

bool FlexboxLayoutManager::canViewBeRecycledFromStart(View* view, int scrollingOffset) {
    if (!isMainAxisDirectionHorizontal() && mIsRtl) {
        return mOrientationHelper->getEnd() -
                mOrientationHelper->getDecoratedStart(view) <= scrollingOffset;
    } else {
        return mOrientationHelper->getDecoratedEnd(view) <= scrollingOffset;
    }
}

void FlexboxLayoutManager::recycleFlexLinesFromEnd(RecyclerView::Recycler& recycler, LayoutState& layoutState) {
    if (layoutState.mScrollingOffset < 0) {
        return;
    }
    const std::vector<int>& indexToFlexLine = mFlexboxHelper->getIndexToFlexLine();
    int childCount = getChildCount();
    if (childCount == 0) {
        return;
    }

    View* lastView = getChildAt(childCount - 1);
    if (lastView == nullptr) {
        return;
    }
    int currentLineIndex = indexToFlexLine[getPosition(lastView)];
    if (currentLineIndex == RecyclerView::NO_POSITION) {
        return;
    }
    int recycleTo = childCount - 1;
    int recycleFrom = childCount;
    FlexLine* flexLine = &mFlexLines[currentLineIndex];
    for (int i = childCount - 1; i >= 0; i--) {
        View* view = getChildAt(i);
        if (view == nullptr) {
            continue;
        }
        if (canViewBeRecycledFromEnd(view, layoutState.mScrollingOffset)) {
            if (flexLine->getFirstIndex() == getPosition(view)) {
                recycleFrom = i;
                if (currentLineIndex <= 0) {
                    break;
                } else {
                    currentLineIndex += layoutState.mLayoutDirection;
                    flexLine = &mFlexLines[currentLineIndex];
                }
            }
        } else {
            break;
        }
    }
    recycleChildren(recycler, recycleFrom, recycleTo);
}

bool FlexboxLayoutManager::canViewBeRecycledFromEnd(View* view, int scrollingOffset) {
    if (!isMainAxisDirectionHorizontal() && mIsRtl) {
        return mOrientationHelper->getDecoratedEnd(view) <= scrollingOffset;
    } else {
        return mOrientationHelper->getDecoratedStart(view) >=
                mOrientationHelper->getEnd() - scrollingOffset;
    }
}

int FlexboxLayoutManager::layoutFlexLine(FlexLine& flexLine, LayoutState& layoutState, RecyclerView::Recycler& recycler) {
    if (isMainAxisDirectionHorizontal()) {
        return layoutFlexLineMainAxisHorizontal(flexLine, layoutState);
    } else {
        return layoutFlexLineMainAxisVertical(flexLine, layoutState);
    }
}

int FlexboxLayoutManager::layoutFlexLineMainAxisHorizontal(FlexLine& flexLine, LayoutState& layoutState) {
    int paddingLeft = getPaddingLeft();
    int paddingRight = getPaddingRight();
    int parentWidth = getWidth();

    int childTop = layoutState.mOffset;
    if (layoutState.mLayoutDirection == LayoutState::LAYOUT_START) {
        childTop = childTop - flexLine.getCrossSize();
    }
    int startPosition = layoutState.mPosition;

    float childLeft;
    float childRight;
    float spaceBetweenItem = 0.f;
    switch (mJustifyContent) {
        case (int)JustifyContent::FLEX_START:
            childLeft = paddingLeft;
            childRight = parentWidth - paddingRight;
            break;
        case (int)JustifyContent::FLEX_END:
            childLeft = parentWidth - flexLine.getMainSize() + paddingRight;
            childRight = flexLine.getMainSize() - paddingLeft;
            break;
        case (int)JustifyContent::CENTER:
            childLeft = paddingLeft + (parentWidth - flexLine.getMainSize()) / 2.f;
            childRight = parentWidth - paddingRight - (parentWidth - flexLine.getMainSize()) / 2.f;
            break;
        case (int)JustifyContent::SPACE_AROUND:
            if (flexLine.getItemCount() != 0) {
                spaceBetweenItem = (parentWidth - flexLine.getMainSize())
                        / (float)flexLine.getItemCount();
            }
            childLeft = paddingLeft + spaceBetweenItem / 2.f;
            childRight = parentWidth - paddingRight - spaceBetweenItem / 2.f;
            break;
        case (int)JustifyContent::SPACE_BETWEEN: {
            childLeft = paddingLeft;
            float denominator = flexLine.getItemCount() != 1 ? flexLine.getItemCount() - 1 : 1.f;
            spaceBetweenItem = (parentWidth - flexLine.getMainSize()) / denominator;
            childRight = parentWidth - paddingRight;
            break;
        }
        case (int)JustifyContent::SPACE_EVENLY:
            if (flexLine.getItemCount() != 0) {
                spaceBetweenItem = (parentWidth - flexLine.getMainSize())
                        / (float)(flexLine.getItemCount() + 1);
            }
            childLeft = paddingLeft + spaceBetweenItem;
            childRight = parentWidth - paddingRight - spaceBetweenItem;
            break;
        default:
            childLeft = paddingLeft;
            childRight = parentWidth - paddingRight;
            break;
    }
    childLeft -= mAnchorInfo->mPerpendicularCoordinate;
    childRight -= mAnchorInfo->mPerpendicularCoordinate;
    spaceBetweenItem = std::max(spaceBetweenItem, 0.f);

    int indexInFlexLine = 0;
    for (int i = startPosition, itemCount = flexLine.getItemCount();
            i < startPosition + itemCount; i++) {
        View* view = getFlexItemAt(i);
        if (view == nullptr) {
            continue;
        }
        Rect TEMP_RECT;
        if (layoutState.mLayoutDirection == LayoutState::LAYOUT_END) {
            calculateItemDecorationsForChild(view, TEMP_RECT);
            addView(view);
        } else {
            calculateItemDecorationsForChild(view, TEMP_RECT);
            addView(view, indexInFlexLine);
            indexInFlexLine++;
        }

        int64_t measureSpec = mFlexboxHelper->getMeasureSpecCache(i);
        int widthSpec = mFlexboxHelper->extractLowerInt(measureSpec);
        int heightSpec = mFlexboxHelper->extractHigherInt(measureSpec);
        LayoutParams* lp = (LayoutParams*)view->getLayoutParams();
        if (shouldMeasureChild(view, widthSpec, heightSpec, lp)) {
            view->measure(widthSpec, heightSpec);
        }

        childLeft += (lp->leftMargin + getLeftDecorationWidth(view));
        childRight -= (lp->rightMargin + getRightDecorationWidth(view));

        int topWithDecoration = childTop + getTopDecorationHeight(view);
        if (mIsRtl) {
            mFlexboxHelper->layoutSingleChildHorizontal(view, flexLine,
                    (int)roundf(childRight) - view->getMeasuredWidth(),
                    topWithDecoration, (int)roundf(childRight),
                    topWithDecoration + view->getMeasuredHeight());
        } else {
            mFlexboxHelper->layoutSingleChildHorizontal(view, flexLine,
                    (int)roundf(childLeft), topWithDecoration,
                    (int)roundf(childLeft) + view->getMeasuredWidth(),
                    topWithDecoration + view->getMeasuredHeight());
        }
        childLeft += (view->getMeasuredWidth() + lp->rightMargin + getRightDecorationWidth(view)
                + spaceBetweenItem);
        childRight -= (view->getMeasuredWidth() + lp->leftMargin + getLeftDecorationWidth(view)
                + spaceBetweenItem);
    }
    layoutState.mFlexLinePosition += mLayoutState->mLayoutDirection;
    return flexLine.getCrossSize();
}

int FlexboxLayoutManager::layoutFlexLineMainAxisVertical(FlexLine& flexLine, LayoutState& layoutState) {
    int paddingTop = getPaddingTop();
    int paddingBottom = getPaddingBottom();
    int parentHeight = getHeight();

    int childLeft = layoutState.mOffset;
    int childRight = layoutState.mOffset;
    if (layoutState.mLayoutDirection == LayoutState::LAYOUT_START) {
        childLeft = childLeft - flexLine.getCrossSize();
        childRight = childRight + flexLine.getCrossSize();
    }
    int startPosition = layoutState.mPosition;

    float childTop;
    float childBottom;
    float spaceBetweenItem = 0.f;
    switch (mJustifyContent) {
        case (int)JustifyContent::FLEX_START:
            childTop = paddingTop;
            childBottom = parentHeight - paddingBottom;
            break;
        case (int)JustifyContent::FLEX_END:
            childTop = parentHeight - flexLine.getMainSize() + paddingBottom;
            childBottom = flexLine.getMainSize() - paddingTop;
            break;
        case (int)JustifyContent::CENTER:
            childTop = paddingTop + (parentHeight - flexLine.getMainSize()) / 2.f;
            childBottom = parentHeight - paddingBottom
                    - (parentHeight - flexLine.getMainSize()) / 2.f;
            break;
        case (int)JustifyContent::SPACE_AROUND:
            if (flexLine.getItemCount() != 0) {
                spaceBetweenItem = (parentHeight - flexLine.getMainSize())
                        / (float)flexLine.getItemCount();
            }
            childTop = paddingTop + spaceBetweenItem / 2.f;
            childBottom = parentHeight - paddingBottom - spaceBetweenItem / 2.f;
            break;
        case (int)JustifyContent::SPACE_BETWEEN: {
            childTop = paddingTop;
            float denominator = flexLine.getItemCount() != 1 ? flexLine.getItemCount() - 1 : 1.f;
            spaceBetweenItem = (parentHeight - flexLine.getMainSize()) / denominator;
            childBottom = parentHeight - paddingBottom;
            break;
        }
        case (int)JustifyContent::SPACE_EVENLY:
            if (flexLine.getItemCount() != 0) {
                spaceBetweenItem = (parentHeight - flexLine.getMainSize())
                        / (float)(flexLine.getItemCount() + 1);
            }
            childTop = paddingTop + spaceBetweenItem;
            childBottom = parentHeight - paddingBottom - spaceBetweenItem;
            break;
        default:
            childTop = paddingTop;
            childBottom = parentHeight - paddingBottom;
            break;
    }
    childTop -= mAnchorInfo->mPerpendicularCoordinate;
    childBottom -= mAnchorInfo->mPerpendicularCoordinate;
    spaceBetweenItem = std::max(spaceBetweenItem, 0.f);

    int indexInFlexLine = 0;
    for (int i = startPosition, itemCount = flexLine.getItemCount();
            i < startPosition + itemCount; i++) {
        View* view = getFlexItemAt(i);
        if (view == nullptr) {
            continue;
        }

        int64_t measureSpec = mFlexboxHelper->getMeasureSpecCache(i);
        int widthSpec = mFlexboxHelper->extractLowerInt(measureSpec);
        int heightSpec = mFlexboxHelper->extractHigherInt(measureSpec);
        LayoutParams* lp = (LayoutParams*)view->getLayoutParams();
        if (shouldMeasureChild(view, widthSpec, heightSpec, lp)) {
            view->measure(widthSpec, heightSpec);
        }

        childTop += (lp->topMargin + getTopDecorationHeight(view));
        childBottom -= (lp->rightMargin + getBottomDecorationHeight(view));
        Rect TEMP_RECT;
        if (layoutState.mLayoutDirection == LayoutState::LAYOUT_END) {
            calculateItemDecorationsForChild(view, TEMP_RECT);
            addView(view);
        } else {
            calculateItemDecorationsForChild(view, TEMP_RECT);
            addView(view, indexInFlexLine);
            indexInFlexLine++;
        }

        int leftWithDecoration = childLeft + getLeftDecorationWidth(view);
        int rightWithDecoration = childRight - getRightDecorationWidth(view);
        if (mIsRtl) {
            if (mFromBottomToTop) {
                mFlexboxHelper->layoutSingleChildVertical(view, flexLine, mIsRtl,
                        rightWithDecoration - view->getMeasuredWidth(),
                        (int)roundf(childBottom) - view->getMeasuredHeight(),
                        rightWithDecoration, (int)roundf(childBottom));
            } else {
                mFlexboxHelper->layoutSingleChildVertical(view, flexLine, mIsRtl,
                        rightWithDecoration - view->getMeasuredWidth(),
                        (int)roundf(childTop), rightWithDecoration,
                        (int)roundf(childTop) + view->getMeasuredHeight());
            }
        } else {
            if (mFromBottomToTop) {
                mFlexboxHelper->layoutSingleChildVertical(view, flexLine, mIsRtl,
                        leftWithDecoration, (int)roundf(childBottom) - view->getMeasuredHeight(),
                        leftWithDecoration + view->getMeasuredWidth(), (int)roundf(childBottom));
            } else {
                mFlexboxHelper->layoutSingleChildVertical(view, flexLine, mIsRtl,
                        leftWithDecoration, (int)roundf(childTop),
                        leftWithDecoration + view->getMeasuredWidth(),
                        (int)roundf(childTop) + view->getMeasuredHeight());
            }
        }
        childTop += (view->getMeasuredHeight() + lp->topMargin + getBottomDecorationHeight(view)
                + spaceBetweenItem);
        childBottom -= (view->getMeasuredHeight() + lp->bottomMargin + getTopDecorationHeight(view)
                + spaceBetweenItem);
    }
    layoutState.mFlexLinePosition += mLayoutState->mLayoutDirection;
    return flexLine.getCrossSize();
}

View* FlexboxLayoutManager::getChildClosestToStart() {
    return getChildAt(0);
}

int FlexboxLayoutManager::computeHorizontalScrollOffset(RecyclerView::State& state) {
    const int scrollOffset = computeScrollOffset(state);
    return scrollOffset;
}

int FlexboxLayoutManager::computeVerticalScrollOffset(RecyclerView::State& state) {
    const int scrollOffset = computeScrollOffset(state);
    return scrollOffset;
}

int FlexboxLayoutManager::computeScrollOffset(RecyclerView::State& state) {
    if (getChildCount() == 0) {
        return 0;
    }
    int allChildrenCount = state.getItemCount();
    View* firstReferenceView = findFirstReferenceChild(allChildrenCount);
    View* lastReferenceView = findLastReferenceChild(allChildrenCount);
    if (state.getItemCount() == 0 || firstReferenceView == nullptr || lastReferenceView == nullptr) {
        return 0;
    }
    const std::vector<int>& indexToFlexLine = mFlexboxHelper->getIndexToFlexLine();
    int minPosition = getPosition(firstReferenceView);
    int maxPosition = getPosition(lastReferenceView);
    int laidOutArea = std::abs(mOrientationHelper->getDecoratedEnd(lastReferenceView) -
            mOrientationHelper->getDecoratedStart(firstReferenceView));
    int firstLinePosition = indexToFlexLine[minPosition];
    if (firstLinePosition == 0 || firstLinePosition == RecyclerView::NO_POSITION) {
        return 0;
    }
    int lastLinePosition = indexToFlexLine[maxPosition];
    int lineRange = lastLinePosition - firstLinePosition + 1;
    float averageSizePerLine = (float) laidOutArea / lineRange;
    // The number of lines before the first line is equal to the value of firstLinePosition
    return (int)roundf(
            firstLinePosition * averageSizePerLine + (mOrientationHelper->getStartAfterPadding()
                    - mOrientationHelper->getDecoratedStart(firstReferenceView)));
}

int FlexboxLayoutManager::computeHorizontalScrollExtent(RecyclerView::State& state) {
    const int scrollExtent = computeScrollExtent(state);
    return scrollExtent;
}

int FlexboxLayoutManager::computeVerticalScrollExtent(RecyclerView::State& state) {
    const int scrollExtent = computeScrollExtent(state);
    return scrollExtent;
}
int FlexboxLayoutManager::computeScrollExtent(RecyclerView::State& state) {
    if (getChildCount() == 0) {
        return 0;
    }
    int allChildrenCount = state.getItemCount();
    ensureOrientationHelper();
    View* firstReferenceView = findFirstReferenceChild(allChildrenCount);
    View* lastReferenceView = findLastReferenceChild(allChildrenCount);
    if (state.getItemCount() == 0 || firstReferenceView == nullptr || lastReferenceView == nullptr) {
        return 0;
    }
    // TODO: Need to consider the reverse pattern when flexWrap == wrap_reverse is implemented
    int extend = mOrientationHelper->getDecoratedEnd(lastReferenceView) -
            mOrientationHelper->getDecoratedStart(firstReferenceView);
    return std::min(mOrientationHelper->getTotalSpace(), extend);
}

int FlexboxLayoutManager::computeHorizontalScrollRange(RecyclerView::State& state) {
    const int scrollRange = computeScrollRange(state);
    return scrollRange;
}

int FlexboxLayoutManager::computeVerticalScrollRange(RecyclerView::State& state) {
    const int scrollRange = computeScrollRange(state);
    return scrollRange;
}

int FlexboxLayoutManager::computeScrollRange(RecyclerView::State& state) {
    if (getChildCount() == 0) {
        return 0;
    }
    int allItemCount = state.getItemCount();
    View* firstReferenceView = findFirstReferenceChild(allItemCount);
    View* lastReferenceView = findLastReferenceChild(allItemCount);
    if (state.getItemCount() == 0 || firstReferenceView == nullptr || lastReferenceView == nullptr) {
        return 0;
    }
    //assert mFlexboxHelper->mIndexToFlexLine != nullptr;
    int firstVisiblePosition = findFirstVisibleItemPosition();
    int lastVisiblePosition = findLastVisibleItemPosition();
    int laidOutArea = std::abs(mOrientationHelper->getDecoratedEnd(lastReferenceView) -
            mOrientationHelper->getDecoratedStart(firstReferenceView));
    int laidOutRange = lastVisiblePosition - firstVisiblePosition + 1;
    return (int) ((float) laidOutArea / laidOutRange * state.getItemCount());
}
bool FlexboxLayoutManager::supportsPredictiveItemAnimations() {
    return true;
}

void FlexboxLayoutManager::onDetachedFromWindow(RecyclerView& view, RecyclerView::Recycler& recycler) {
    if (mRecycleChildrenOnDetach) {
        removeAndRecycleAllViews(recycler);
        recycler.clear();
    }
}

Parcelable* FlexboxLayoutManager::onSaveInstanceState() {
    if (mPendingSavedState != nullptr) {
        return new SavedState(*mPendingSavedState);
    }
    SavedState* savedState = new SavedState();
    if (getChildCount() > 0) {
        View* firstView = getChildClosestToStart();
        savedState->mAnchorPosition = getPosition(firstView);
        savedState->mAnchorOffset = mOrientationHelper->getDecoratedStart(firstView)
                - mOrientationHelper->getStartAfterPadding();
    } else {
        savedState->invalidateAnchor();
    }
    return savedState;
}

void FlexboxLayoutManager::onRestoreInstanceState(Parcelable& state) {
    SavedState* savedState = dynamic_cast<SavedState*>(&state);
    if (savedState != nullptr) {
        mPendingSavedState = savedState;
        requestLayout();
    }
}

bool FlexboxLayoutManager::isLayoutRTL() {
    return mIsRtl;
}

void FlexboxLayoutManager::resolveLayoutDirection() {
    int layoutDirection = getLayoutDirection();
    switch (mFlexDirection) {
        case FlexDirection::ROW:
            mIsRtl = layoutDirection == View::LAYOUT_DIRECTION_RTL;
            mFromBottomToTop = mFlexWrap == (int)FlexWrap::WRAP_REVERSE;
            break;
        case FlexDirection::ROW_REVERSE:
            mIsRtl = layoutDirection != View::LAYOUT_DIRECTION_RTL;
            mFromBottomToTop = mFlexWrap == (int)FlexWrap::WRAP_REVERSE;
            break;
        case FlexDirection::COLUMN:
            mIsRtl = layoutDirection == View::LAYOUT_DIRECTION_RTL;
            if (mFlexWrap == (int)FlexWrap::WRAP_REVERSE) {
                mIsRtl = !mIsRtl;
            }
            mFromBottomToTop = false;
            break;
        case FlexDirection::COLUMN_REVERSE:
            mIsRtl = layoutDirection == View::LAYOUT_DIRECTION_RTL;
            if (mFlexWrap == (int)FlexWrap::WRAP_REVERSE) {
                mIsRtl = !mIsRtl;
            }
            mFromBottomToTop = true;
            break;
        default:
            mIsRtl = false;
            mFromBottomToTop = false;
            break;
    }
}

// FlexContainer interface implementation
int FlexboxLayoutManager::getFlexItemCount() {
    return  mState->getItemCount();
}

View* FlexboxLayoutManager::getFlexItemAt(int index) {
    View* cachedView = mViewCache.get(index);
    if (cachedView != nullptr) {
        return cachedView;
    }
    return mRecycler->getViewForPosition(index);
}

View* FlexboxLayoutManager::getReorderedFlexItemAt(int index) {
    return getFlexItemAt(index);
}

void FlexboxLayoutManager::addView(View* view) {
    RecyclerView::LayoutManager::addView(view);
}

void FlexboxLayoutManager::addView(View* view, int index) {
    RecyclerView::LayoutManager::addView(view, index);
}

void FlexboxLayoutManager::removeAllViews() {
    RecyclerView::LayoutManager::removeAllViews();
}

void FlexboxLayoutManager::removeViewAt(int index) {
    RecyclerView::LayoutManager::removeViewAt(index);
}

int FlexboxLayoutManager::getFlexDirection() {
    return mFlexDirection;
}

void FlexboxLayoutManager::setFlexDirection(int flexDirection) {
    if (mFlexDirection != flexDirection) {
        removeAllViews();
        mFlexDirection = flexDirection;
        delete mOrientationHelper;
        mOrientationHelper = nullptr;
        delete mSubOrientationHelper;
        mSubOrientationHelper = nullptr;
        clearFlexLines();
        requestLayout();
    }
}

int FlexboxLayoutManager::getFlexWrap() {
    return mFlexWrap;
}

void FlexboxLayoutManager::setFlexWrap(int flexWrap) {
    if (flexWrap == (int)FlexWrap::WRAP_REVERSE) {
        LOGE("wrap_reverse is not supported in FlexboxLayoutManager");
        return;
    }
    if (mFlexWrap != flexWrap) {
        if (mFlexWrap == (int)FlexWrap::NOWRAP || flexWrap == (int)FlexWrap::NOWRAP) {
            removeAllViews();
            clearFlexLines();
        }
        mFlexWrap = flexWrap;
        delete mOrientationHelper;
        mOrientationHelper = nullptr;
        delete mSubOrientationHelper;
        mSubOrientationHelper = nullptr;
        requestLayout();
    }
}

int FlexboxLayoutManager::getJustifyContent() {
    return mJustifyContent;
}

void FlexboxLayoutManager::setJustifyContent(int justifyContent) {
    if (mJustifyContent != justifyContent) {
        mJustifyContent = justifyContent;
        requestLayout();
    }
}

int FlexboxLayoutManager::getAlignItems() {
    return mAlignItems;
}

void FlexboxLayoutManager::setAlignItems(int alignItems) {
    if (mAlignItems != alignItems) {
        if (mAlignItems == (int)AlignItems::STRETCH || alignItems == (int)AlignItems::STRETCH) {
            removeAllViews();
            clearFlexLines();
        }
        mAlignItems = alignItems;
        requestLayout();
    }
}

int FlexboxLayoutManager::getAlignContent() {
    return (int)AlignContent::STRETCH;
}

void FlexboxLayoutManager::setAlignContent(int alignContent) {
    LOGE("Setting the alignContent in FlexboxLayoutManager is not supported. "
         "Use FlexboxLayout if you need this attribute.");
}

std::vector<FlexLine> FlexboxLayoutManager::getFlexLines() {
    std::vector<FlexLine> result;
    result.reserve(mFlexLines.size());
    for (auto& flexLine : mFlexLines) {
        if (flexLine.getItemCount() == 0) {
            continue;
        }
        result.push_back(flexLine);
    }
    return result;
}

bool FlexboxLayoutManager::isMainAxisDirectionHorizontal() const {
    return mFlexDirection == (int)FlexDirection::ROW || mFlexDirection == (int)FlexDirection::ROW_REVERSE;
}

int FlexboxLayoutManager::getDecorationLengthMainAxis(View* view, int index, int indexInFlexLine) {
    if (isMainAxisDirectionHorizontal()) {
        return getLeftDecorationWidth(view) + getRightDecorationWidth(view);
    } else {
        return getTopDecorationHeight(view) + getBottomDecorationHeight(view);
    }
}

int FlexboxLayoutManager::getDecorationLengthCrossAxis(View* view) {
    if (isMainAxisDirectionHorizontal()) {
        return getTopDecorationHeight(view) + getBottomDecorationHeight(view);
    } else {
        return getLeftDecorationWidth(view) + getRightDecorationWidth(view);
    }
}

int FlexboxLayoutManager::getPaddingTop() {
    return RecyclerView::LayoutManager::getPaddingTop();
}

int FlexboxLayoutManager::getPaddingLeft() {
    return RecyclerView::LayoutManager::getPaddingLeft();
}

int FlexboxLayoutManager::getPaddingRight() {
    return RecyclerView::LayoutManager::getPaddingRight();
}

int FlexboxLayoutManager::getPaddingBottom() {
    return RecyclerView::LayoutManager::getPaddingBottom();
}

int FlexboxLayoutManager::getPaddingStart() {
    return RecyclerView::LayoutManager::getPaddingStart();
}

int FlexboxLayoutManager::getPaddingEnd() {
    return RecyclerView::LayoutManager::getPaddingEnd();
}

int FlexboxLayoutManager::getChildWidthMeasureSpec(int widthSpec, int padding, int childDimension) {
    return RecyclerView::LayoutManager::getChildMeasureSpec(getWidth(), getWidthMode(),
            padding, childDimension, canScrollHorizontally());
}

int FlexboxLayoutManager::getChildHeightMeasureSpec(int heightSpec, int padding, int childDimension) {
    return RecyclerView::LayoutManager::getChildMeasureSpec(getHeight(), getHeightMode(),
            padding, childDimension, canScrollVertically());
}

int FlexboxLayoutManager::getLargestMainSize() {
    int largest = 0;
    for (auto& line : mFlexLines) {
        largest = std::max(largest, line.getMainSize());
    }
    return largest;
}

int FlexboxLayoutManager::getSumOfCrossSize() {
    int sum = 0;
    for (auto& line : mFlexLines) {
        sum += line.getCrossSize();
    }
    return sum;
}

void FlexboxLayoutManager::onNewFlexItemAdded(View* view, int index, int indexInFlexLine, FlexLine& flexLine) {
    Rect tempRect;
    calculateItemDecorationsForChild(view, tempRect);
    if (isMainAxisDirectionHorizontal()) {
        int decorationWidth = getLeftDecorationWidth(view) + getRightDecorationWidth(view);
        flexLine.setMainSize(flexLine.getMainSize() + decorationWidth);
        flexLine.setDividerLengthInMainSize(flexLine.getDividerLengthInMainSize() + decorationWidth);
    } else {
        int decorationHeight = getTopDecorationHeight(view) + getBottomDecorationHeight(view);
        flexLine.setMainSize(flexLine.getMainSize() + decorationHeight);
        flexLine.setDividerLengthInMainSize(flexLine.getDividerLengthInMainSize() + decorationHeight);
    }
}

void FlexboxLayoutManager::onNewFlexLineAdded(FlexLine& flexLine) {
}

void FlexboxLayoutManager::setFlexLines(const std::vector<FlexLine>& flexLines) {
    mFlexLines = flexLines;
}

int FlexboxLayoutManager::getMaxLine() {
    return mMaxLine;
}

void FlexboxLayoutManager::setMaxLine(int maxLine) {
    if (mMaxLine != maxLine) {
        mMaxLine = maxLine;
        requestLayout();
    }
}

std::vector<FlexLine>& FlexboxLayoutManager::getFlexLinesInternal() {
    return mFlexLines;
}

void FlexboxLayoutManager::updateViewCache(int position, View* view) {
    mViewCache.put(position, view);
}

bool FlexboxLayoutManager::computeScrollVectorForPosition(int targetPosition, PointF& scrollVector) {
    if (getChildCount() == 0) {
        return false;
    }
    View* view = getChildAt(0);
    if (view == nullptr) {
        return false;
    }
    int firstChildPos = getPosition(view);
    int direction = targetPosition < firstChildPos ? -1 : 1;
    if (isMainAxisDirectionHorizontal()) {
        scrollVector.x = 0;
        scrollVector.y = direction;
    } else {
        scrollVector.x = direction;
        scrollVector.y = 0;
    }
    return true;
}

RecyclerView::LayoutParams* FlexboxLayoutManager::generateLayoutParams(Context* c, const AttributeSet& attrs) const {
    return new FlexboxLayoutManager::LayoutParams(c, attrs);
}

bool FlexboxLayoutManager::checkLayoutParams(const RecyclerView::LayoutParams* lp) const {
    return dynamic_cast<const LayoutParams*>(lp) != nullptr;
}

void FlexboxLayoutManager::onAdapterChanged(RecyclerView::Adapter* oldAdapter, RecyclerView::Adapter* newAdapter) {
    removeAllViews();
}

void FlexboxLayoutManager::onItemsAdded(RecyclerView& recyclerView, int positionStart, int itemCount) {
    RecyclerView::LayoutManager::onItemsAdded(recyclerView, positionStart, itemCount);
    updateDirtyPosition(positionStart);
}

void FlexboxLayoutManager::onItemsUpdated(RecyclerView& recyclerView, int positionStart, int itemCount, Object* payload) {
    RecyclerView::LayoutManager::onItemsUpdated(recyclerView, positionStart, itemCount, payload);
    updateDirtyPosition(positionStart);
}

void FlexboxLayoutManager::onItemsUpdated(RecyclerView& recyclerView, int positionStart, int itemCount) {
    RecyclerView::LayoutManager::onItemsUpdated(recyclerView, positionStart, itemCount);
    updateDirtyPosition(positionStart);
}

void FlexboxLayoutManager::onItemsRemoved(RecyclerView& recyclerView, int positionStart, int itemCount) {
    RecyclerView::LayoutManager::onItemsRemoved(recyclerView, positionStart, itemCount);
    updateDirtyPosition(positionStart);
}

void FlexboxLayoutManager::onItemsMoved(RecyclerView& recyclerView, int from, int to, int itemCount) {
    RecyclerView::LayoutManager::onItemsMoved(recyclerView, from, to, itemCount);
    updateDirtyPosition(std::min(from, to));
}

void FlexboxLayoutManager::onAttachedToWindow(RecyclerView& recyclerView) {
    RecyclerView::LayoutManager::onAttachedToWindow(recyclerView);
    mParent = recyclerView.getParent();
}

int FlexboxLayoutManager::findFirstVisibleItemPosition() {
    View* child = findOneVisibleChild(0, getChildCount(), false);
    return child == nullptr ? RecyclerView::NO_POSITION : getPosition(child);
}

int FlexboxLayoutManager::findFirstCompletelyVisibleItemPosition() {
    View* child = findOneVisibleChild(0, getChildCount(), true);
    return child == nullptr ? RecyclerView::NO_POSITION : getPosition(child);
}

int FlexboxLayoutManager::findLastVisibleItemPosition() {
    View* child = findOneVisibleChild(getChildCount() - 1, -1, false);
    return child == nullptr ? RecyclerView::NO_POSITION : getPosition(child);
}

int FlexboxLayoutManager::findLastCompletelyVisibleItemPosition() {
    View* child = findOneVisibleChild(getChildCount() - 1, -1, true);
    return child == nullptr ? RecyclerView::NO_POSITION : getPosition(child);
}

bool FlexboxLayoutManager::getRecycleChildrenOnDetach() {
    return mRecycleChildrenOnDetach;
}

void FlexboxLayoutManager::setRecycleChildrenOnDetach(bool recycleChildrenOnDetach) {
    mRecycleChildrenOnDetach = recycleChildrenOnDetach;
}

void FlexboxLayoutManager::updateDirtyPosition(int positionStart) {
    int lastVisiblePosition = findLastVisibleItemPosition();
    if (positionStart >= lastVisiblePosition) {
        return;
    }
    int childCount = getChildCount();
    mFlexboxHelper->ensureMeasureSpecCache(childCount);
    mFlexboxHelper->ensureMeasuredSizeCache(childCount);
    mFlexboxHelper->ensureIndexToFlexLine(childCount);

    const std::vector<int>& indexToFlexLine = mFlexboxHelper->getIndexToFlexLine();
    if (positionStart >= (int)indexToFlexLine.size()) {
        return;
    }

    mDirtyPosition = positionStart;

    View* firstView = getChildClosestToStart();
    if (firstView == nullptr) {
        return;
    }

    // Assign the pending scroll position and offset so that the first visible position is
    // restored in the next layout.
    ensureOrientationHelper();
    mPendingScrollPosition = getPosition(firstView);
    if (!isMainAxisDirectionHorizontal() && mIsRtl) {
        mPendingScrollPositionOffset = mOrientationHelper->getDecoratedEnd(firstView)
                + mOrientationHelper->getEndPadding();
    } else {
        mPendingScrollPositionOffset = mOrientationHelper->getDecoratedStart(firstView)
                - mOrientationHelper->getStartAfterPadding();
    }
}

View* FlexboxLayoutManager::findOneVisibleChild(int fromIndex, int toIndex, bool completelyVisible) {
    int next = toIndex > fromIndex ? 1 : -1;
    for (int i = fromIndex; i != toIndex; i += next) {
        View* view = getChildAt(i);
        if (isViewVisible(view, completelyVisible)) {
            return view;
        }
    }
    return nullptr;
}

bool FlexboxLayoutManager::isViewVisible(View* view, bool completelyVisible) {
    int left = getPaddingLeft();
    int top = getPaddingTop();
    int right = getWidth() - getPaddingRight();
    int bottom = getHeight() - getPaddingBottom();
    int childLeft = getChildLeft(view);
    int childTop = getChildTop(view);
    int childRight = getChildRight(view);
    int childBottom = getChildBottom(view);

    bool horizontalCompletelyVisible = false;
    bool horizontalPartiallyVisible = false;
    bool verticalCompletelyVisible = false;
    bool verticalPartiallyVisible = false;

    if (left <= childLeft && right >= childRight) {
        horizontalCompletelyVisible = true;
    }
    if (childLeft >= right || childRight >= left) {
        horizontalPartiallyVisible = true;
    }

    if (top <= childTop && bottom >= childBottom) {
        verticalCompletelyVisible = true;
    }
    if (childTop >= bottom || childBottom >= top) {
        verticalPartiallyVisible = true;
    }

    if (completelyVisible) {
        return horizontalCompletelyVisible && verticalCompletelyVisible;
    } else {
        return horizontalPartiallyVisible && verticalPartiallyVisible;
    }
}

int FlexboxLayoutManager::getChildLeft(View* view) {
    ViewGroup::MarginLayoutParams* params = (ViewGroup::MarginLayoutParams*) view->getLayoutParams();
    return getDecoratedLeft(view) - params->leftMargin;
}

int FlexboxLayoutManager::getChildRight(View* view) {
    ViewGroup::MarginLayoutParams* params = (ViewGroup::MarginLayoutParams*) view->getLayoutParams();
    return getDecoratedRight(view) + params->rightMargin;
}

int FlexboxLayoutManager::getChildTop(View* view) {
    ViewGroup::MarginLayoutParams* params = (ViewGroup::MarginLayoutParams*) view->getLayoutParams();
    return getDecoratedTop(view) - params->topMargin;
}

int FlexboxLayoutManager::getChildBottom(View* view) {
    ViewGroup::MarginLayoutParams* params = (ViewGroup::MarginLayoutParams*) view->getLayoutParams();
    return getDecoratedBottom(view) + params->bottomMargin;
}

void FlexboxLayoutManager::clearFlexLines() {
    mFlexLines.clear();
    mAnchorInfo->reset(this);
    mAnchorInfo->mPerpendicularCoordinate = 0;
}

void FlexboxLayoutManager::resolveInfiniteAmount() {
    int crossMode;
    if (isMainAxisDirectionHorizontal()) {
        crossMode = getHeightMode();
    } else {
        crossMode = getWidthMode();
    }
    mLayoutState->mInfinite = crossMode == View::MeasureSpec::UNSPECIFIED
            || crossMode == View::MeasureSpec::AT_MOST;
}

// SavedState implementation
FlexboxLayoutManager::SavedState::SavedState() {}

FlexboxLayoutManager::SavedState::SavedState(Parcel& in) {
    mAnchorPosition = in.readInt();
    mAnchorOffset = in.readInt();
}

FlexboxLayoutManager::SavedState::SavedState(const SavedState& other) {
    mAnchorPosition = other.mAnchorPosition;
    mAnchorOffset = other.mAnchorOffset;
}

bool FlexboxLayoutManager::SavedState::hasValidAnchor(int itemCount) const {
    return mAnchorPosition >= 0 && mAnchorPosition < itemCount;
}

void FlexboxLayoutManager::SavedState::invalidateAnchor() {
    mAnchorPosition = RecyclerView::NO_POSITION;
}

int FlexboxLayoutManager::SavedState::describeContents() {
    return 0;
}

void FlexboxLayoutManager::SavedState::writeToParcel(Parcel& dest, int flags) {
    dest.writeInt(mAnchorPosition);
    dest.writeInt(mAnchorOffset);
}

void FlexboxLayoutManager::updateAnchorInfoForLayout(RecyclerView::State& state, AnchorInfo* anchorInfo) {
    if (updateAnchorFromPendingState(state, anchorInfo, mPendingSavedState)) {
        return;
    }
    if (updateAnchorFromChildren(state, anchorInfo)) {
        return;
    }
    anchorInfo->assignCoordinateFromPadding(this);
    anchorInfo->mPosition = 0;
    anchorInfo->mFlexLinePosition = 0;
}

bool FlexboxLayoutManager::updateAnchorFromPendingState(RecyclerView::State& state, AnchorInfo* anchorInfo, SavedState* savedState) {
    if (state.isPreLayout() || mPendingScrollPosition == RecyclerView::NO_POSITION) {
        return false;
    }
    if (mPendingScrollPosition < 0 || mPendingScrollPosition >= state.getItemCount()) {
        mPendingScrollPosition = RecyclerView::NO_POSITION;
        mPendingScrollPositionOffset = INT_MIN;
        return false;
    }

    anchorInfo->mPosition = mPendingScrollPosition;
    anchorInfo->mFlexLinePosition = mFlexboxHelper->getIndexToFlexLine()[anchorInfo->mPosition];
    if (mPendingSavedState != nullptr && mPendingSavedState->hasValidAnchor(state.getItemCount())) {
        anchorInfo->mCoordinate = mOrientationHelper->getStartAfterPadding() + savedState->mAnchorOffset;
        anchorInfo->mAssignedFromSavedState = true;
        anchorInfo->mFlexLinePosition = RecyclerView::NO_POSITION;
        return true;
    }

    if (mPendingScrollPositionOffset == INT_MIN) {
        View* anchorView = findViewByPosition(mPendingScrollPosition);
        if (anchorView != nullptr) {
            if (mOrientationHelper->getDecoratedMeasurement(anchorView) > mOrientationHelper->getTotalSpace()) {
                anchorInfo->assignCoordinateFromPadding(this);
                return true;
            }
            int startGap = mOrientationHelper->getDecoratedStart(anchorView) - mOrientationHelper->getStartAfterPadding();
            if (startGap < 0) {
                anchorInfo->mCoordinate = mOrientationHelper->getStartAfterPadding();
                anchorInfo->mLayoutFromEnd = false;
                return true;
            }

            int endGap = mOrientationHelper->getEndAfterPadding() - mOrientationHelper->getDecoratedEnd(anchorView);
            if (endGap < 0) {
                anchorInfo->mCoordinate = mOrientationHelper->getEndAfterPadding();
                anchorInfo->mLayoutFromEnd = true;
                return true;
            }
            anchorInfo->mCoordinate = anchorInfo->mLayoutFromEnd ?
                    (mOrientationHelper->getDecoratedEnd(anchorView) + mOrientationHelper->getTotalSpaceChange())
                    : mOrientationHelper->getDecoratedStart(anchorView);
        } else {
            if (getChildCount() > 0) {
                View* view = getChildAt(0);
                if (view != nullptr) {
                    int position = getPosition(view);
                    anchorInfo->mLayoutFromEnd = mPendingScrollPosition < position;
                }
            }
            anchorInfo->assignCoordinateFromPadding(this);
        }
        return true;
    }

    if (!isMainAxisDirectionHorizontal() && mIsRtl) {
        anchorInfo->mCoordinate = mPendingScrollPositionOffset - mOrientationHelper->getEndPadding();
    } else {
        anchorInfo->mCoordinate = mOrientationHelper->getStartAfterPadding() + mPendingScrollPositionOffset;
    }
    return true;
}

bool FlexboxLayoutManager::updateAnchorFromChildren(RecyclerView::State& state, AnchorInfo* anchorInfo) {
    if (getChildCount() == 0) {
        return false;
    }

    View* referenceChild = anchorInfo->mLayoutFromEnd
            ? findLastReferenceChild(state.getItemCount())
            : findFirstReferenceChild(state.getItemCount());
    if (referenceChild != nullptr) {
        anchorInfo->assignFromView(referenceChild, this);
        if (!state.isPreLayout() && supportsPredictiveItemAnimations()) {
            bool notVisible = mOrientationHelper->getDecoratedStart(referenceChild) >= mOrientationHelper->getEndAfterPadding()
                    || mOrientationHelper->getDecoratedEnd(referenceChild) < mOrientationHelper->getStartAfterPadding();
            if (notVisible) {
                anchorInfo->mCoordinate = anchorInfo->mLayoutFromEnd
                        ? mOrientationHelper->getEndAfterPadding()
                        : mOrientationHelper->getStartAfterPadding();
            }
        }
        return true;
    }
    return false;
}

View* FlexboxLayoutManager::findFirstReferenceChild(int itemCount) {
    View* firstFound = findReferenceChild(0, getChildCount(), itemCount);
    if (firstFound == nullptr) {
        return nullptr;
    }
    int firstFoundPosition = getPosition(firstFound);
    int firstFoundLinePosition = mFlexboxHelper->getIndexToFlexLine()[firstFoundPosition];
    if (firstFoundLinePosition == RecyclerView::NO_POSITION) {
        return nullptr;
    }
    FlexLine* firstFoundLine = &mFlexLines[firstFoundLinePosition];
    return findFirstReferenceViewInLine(firstFound, firstFoundLine);
}

View* FlexboxLayoutManager::findLastReferenceChild(int itemCount) {
    View* lastFound = findReferenceChild(getChildCount() - 1, -1, itemCount);
    if (lastFound == nullptr) {
        return nullptr;
    }
    int lastFoundPosition = getPosition(lastFound);
    int lastFoundLinePosition = mFlexboxHelper->getIndexToFlexLine()[lastFoundPosition];
    FlexLine* lastFoundLine = &mFlexLines[lastFoundLinePosition];
    return findLastReferenceViewInLine(lastFound, lastFoundLine);
}

View* FlexboxLayoutManager::findReferenceChild(int start, int end, int itemCount) {
    ensureOrientationHelper();
    ensureLayoutState();
    View* invalidMatch = nullptr;
    View* outOfBoundsMatch = nullptr;
    int boundStart = mOrientationHelper->getStartAfterPadding();
    int boundEnd = mOrientationHelper->getEndAfterPadding();
    int increment = end > start ? 1 : -1;
    for (int i = start; i != end; i += increment) {
        View* child = getChildAt(i);
        if (child == nullptr) {
            continue;
        }
        int position = getPosition(child);
        if (position >= 0 && position < itemCount) {
            RecyclerView::LayoutParams* params =
                    (RecyclerView::LayoutParams*) child->getLayoutParams();
            if (params->isItemRemoved()) {
                if (invalidMatch == nullptr) {
                    invalidMatch = child;
                }
            } else if (mOrientationHelper->getDecoratedStart(child) < boundStart ||
                    mOrientationHelper->getDecoratedEnd(child) > boundEnd) {
                if (outOfBoundsMatch == nullptr) {
                    outOfBoundsMatch = child;
                }
            } else {
                return child;
            }
        }
    }
    return outOfBoundsMatch != nullptr ? outOfBoundsMatch : invalidMatch;
}

// LayoutParams implementation
FlexboxLayoutManager::LayoutParams::LayoutParams(Context* c, const AttributeSet& attrs)
    : RecyclerView::LayoutParams(c, attrs) {
}

FlexboxLayoutManager::LayoutParams::LayoutParams(int width, int height)
    : RecyclerView::LayoutParams(width, height) {
}

FlexboxLayoutManager::LayoutParams::LayoutParams(const ViewGroup::MarginLayoutParams& source)
    : RecyclerView::LayoutParams(source) {
}

FlexboxLayoutManager::LayoutParams::LayoutParams(const RecyclerView::LayoutParams& source)
    : RecyclerView::LayoutParams(source) {
}

int FlexboxLayoutManager::LayoutParams::getWidth() { return width; }
void FlexboxLayoutManager::LayoutParams::setWidth(int w) { width = w; }
int FlexboxLayoutManager::LayoutParams::getHeight() { return height; }
void FlexboxLayoutManager::LayoutParams::setHeight(int h) { height = h; }

int FlexboxLayoutManager::LayoutParams::getOrder() {
    return FlexItem::ORDER_DEFAULT;
}

void FlexboxLayoutManager::LayoutParams::setOrder(int order) {
    // Unlike the FlexboxLayout, the order attribute is not supported, we don't calculate
    // the order attribute because preparing the order attribute requires all view holders
    // to be inflated at least once, which is inefficient if the number of items is large.
    LOGE("Setting the order in FlexboxLayoutManager is not supported. "
         "Use FlexboxLayout if you need to reorder using the attribute.");
}

float FlexboxLayoutManager::LayoutParams::getFlexGrow() { return mFlexGrow; }
void FlexboxLayoutManager::LayoutParams::setFlexGrow(float flexGrow) { mFlexGrow = flexGrow; }
float FlexboxLayoutManager::LayoutParams::getFlexShrink() { return mFlexShrink; }
void FlexboxLayoutManager::LayoutParams::setFlexShrink(float flexShrink) { mFlexShrink = flexShrink; }
int FlexboxLayoutManager::LayoutParams::getAlignSelf() { return mAlignSelf; }
void FlexboxLayoutManager::LayoutParams::setAlignSelf(int alignSelf) { mAlignSelf = alignSelf; }
int FlexboxLayoutManager::LayoutParams::getMinWidth() { return mMinWidth; }
void FlexboxLayoutManager::LayoutParams::setMinWidth(int minWidth) { mMinWidth = minWidth; }
int FlexboxLayoutManager::LayoutParams::getMinHeight() { return mMinHeight; }
void FlexboxLayoutManager::LayoutParams::setMinHeight(int minHeight) { mMinHeight = minHeight; }
int FlexboxLayoutManager::LayoutParams::getMaxWidth() { return mMaxWidth; }
void FlexboxLayoutManager::LayoutParams::setMaxWidth(int maxWidth) { mMaxWidth = maxWidth; }
int FlexboxLayoutManager::LayoutParams::getMaxHeight() { return mMaxHeight; }
void FlexboxLayoutManager::LayoutParams::setMaxHeight(int maxHeight) { mMaxHeight = maxHeight; }
bool FlexboxLayoutManager::LayoutParams::isWrapBefore() { return mWrapBefore; }
void FlexboxLayoutManager::LayoutParams::setWrapBefore(bool wrapBefore) { mWrapBefore = wrapBefore; }
float FlexboxLayoutManager::LayoutParams::getFlexBasisPercent() { return mFlexBasisPercent; }
void FlexboxLayoutManager::LayoutParams::setFlexBasisPercent(float flexBasisPercent) { mFlexBasisPercent = flexBasisPercent; }

int FlexboxLayoutManager::LayoutParams::getMarginLeft() { return leftMargin; }
int FlexboxLayoutManager::LayoutParams::getMarginTop() { return topMargin; }
int FlexboxLayoutManager::LayoutParams::getMarginRight() { return rightMargin; }
int FlexboxLayoutManager::LayoutParams::getMarginBottom() { return bottomMargin; }
int FlexboxLayoutManager::LayoutParams::getMarginStart() { return startMargin; }
int FlexboxLayoutManager::LayoutParams::getMarginEnd() { return endMargin; }

} // namespace cdroid
