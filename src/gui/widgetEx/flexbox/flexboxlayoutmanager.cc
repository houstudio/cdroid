#include <widgetEx/flexbox/flexboxlayoutmanager.h>
#include <view/viewgroup.h>
#include <algorithm>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include <widgetEx/recyclerview/orientationhelper.h>
#include <widgetEx/recyclerview/layoutstate.h>

namespace cdroid {

FlexboxLayoutManager::FlexboxLayoutManager(Context* context) {
    init();
}

FlexboxLayoutManager::FlexboxLayoutManager(Context* context, int flexDirection)
    : mFlexDirection(flexDirection) {
    init();
}

FlexboxLayoutManager::FlexboxLayoutManager(Context* context, int flexDirection, int flexWrap)
    : mFlexDirection(flexDirection), mFlexWrap(flexWrap) {
    init();
}

FlexboxLayoutManager::FlexboxLayoutManager(Context* context, const AttributeSet& attrs) {
    mFlexDirection = attrs.getInt("flexDirection", (int)FlexDirection::ROW);
    mFlexWrap = attrs.getInt("flexWrap", (int)FlexWrap::NOWRAP);
    mJustifyContent = attrs.getInt("justifyContent", (int)JustifyContent::FLEX_START);
    mAlignItems = attrs.getInt("alignItems", (int)AlignItems::FLEX_START);
    mAlignContent = attrs.getInt("alignContent", (int)AlignContent::FLEX_START);
    init();
}

void FlexboxLayoutManager::init() {
    mFlexLinesResult = new FlexboxHelper::FlexLinesResult();
    mOrientationHelper = OrientationHelper::createOrientationHelper(this, 
        isMainAxisDirectionHorizontal() ? RecyclerView::HORIZONTAL : RecyclerView::VERTICAL);
    ensureFlexboxHelper();
    mAnchorInfo = new AnchorInfo();
}

FlexboxLayoutManager::~FlexboxLayoutManager() {
    delete mFlexLinesResult;
    delete mFlexboxHelper;
    delete mOrientationHelper;
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
        // resolveInfiniteAmount() - will be implemented later if needed
    } else {
        mLayoutState->mInfinite = false;
    }
    if (!isMainAxisDirectionHorizontal() && mIsRtl) {
        mLayoutState->mAvailable = anchorInfo->mCoordinate - getPaddingRight();
    } else {
        mLayoutState->mAvailable = mOrientationHelper->getEnd() - anchorInfo->mCoordinate;
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
        // resolveInfiniteAmount() - will be implemented later if needed
    } else {
        mLayoutState->mInfinite = false;
    }
    if (!isMainAxisDirectionHorizontal() && mIsRtl) {
        mLayoutState->mAvailable = getWidth() - anchorInfo->mCoordinate - mOrientationHelper->getStartAfterPadding();
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
    return new RecyclerView::LayoutParams(RecyclerView::LayoutParams::WRAP_CONTENT,
                                         RecyclerView::LayoutParams::WRAP_CONTENT);
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
    int childCount = state.getItemCount();
    if (childCount == 0 && state.isPreLayout()) {
        return;
    }
    mRecycler = &recycler;
    mState = &state;
    ensureFlexboxHelper();
    resolveLayoutDirection();
    ensureOrientationHelper();
    ensureLayoutState();
    
    mFlexboxHelper->ensureIndexToFlexLine(childCount);

    mLayoutState->mShouldRecycle = false;

    if (mPendingSavedState != nullptr && mPendingSavedState->hasValidAnchor(childCount)) {
        mPendingScrollPosition = mPendingSavedState->mAnchorPosition;
    }

    if (!mAnchorInfo->mValid || mPendingScrollPosition != RecyclerView::NO_POSITION ||
            mPendingSavedState != nullptr) {
        mAnchorInfo->reset();
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
    mAnchorInfo->reset();
    mViewCache.clear();
}

void FlexboxLayoutManager::scrollToPosition(int position) {
    mPendingScrollPosition = position;
    mPendingScrollPositionOffset = INT_MIN;
    requestLayout();
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

    if (mIsRtl) {
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
    int widthMeasureSpec = View::MeasureSpec::makeMeasureSpec(getWidth(), View::MeasureSpec::EXACTLY);
    int heightMeasureSpec = View::MeasureSpec::makeMeasureSpec(getHeight(), View::MeasureSpec::EXACTLY);
    int width = getWidth();
    int height = getHeight();
    bool isMainSizeChanged;
    int needsToFill;

    if (isMainAxisDirectionHorizontal()) {
        isMainSizeChanged = mLastWidth != INT_MIN && mLastWidth != width;
        needsToFill = mLayoutState->mInfinite ? height : mLayoutState->mAvailable;
    } else {
        isMainSizeChanged = mLastHeight != INT_MIN && mLastHeight != height;
        needsToFill = mLayoutState->mInfinite ? width : mLayoutState->mAvailable;
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
            layoutState.hasMore(state)) {
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
                    (int)(childRight - view->getMeasuredWidth()),
                    topWithDecoration, (int)childRight,
                    topWithDecoration + view->getMeasuredHeight());
        } else {
            mFlexboxHelper->layoutSingleChildHorizontal(view, flexLine,
                    (int)childLeft, topWithDecoration,
                    (int)childLeft + view->getMeasuredWidth(),
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
        childBottom -= (lp->bottomMargin + getBottomDecorationHeight(view));
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
                        (int)(childBottom - view->getMeasuredHeight()),
                        rightWithDecoration, (int)childBottom);
            } else {
                mFlexboxHelper->layoutSingleChildVertical(view, flexLine, mIsRtl,
                        rightWithDecoration - view->getMeasuredWidth(),
                        (int)childTop, rightWithDecoration,
                        (int)childTop + view->getMeasuredHeight());
            }
        } else {
            if (mFromBottomToTop) {
                mFlexboxHelper->layoutSingleChildVertical(view, flexLine, mIsRtl,
                        leftWithDecoration, (int)(childBottom - view->getMeasuredHeight()),
                        leftWithDecoration + view->getMeasuredWidth(), (int)childBottom);
            } else {
                mFlexboxHelper->layoutSingleChildVertical(view, flexLine, mIsRtl,
                        leftWithDecoration, (int)childTop,
                        leftWithDecoration + view->getMeasuredWidth(),
                        (int)childTop + view->getMeasuredHeight());
            }
        }
        childTop += (view->getMeasuredHeight() + lp->bottomMargin + getBottomDecorationHeight(view)
                + spaceBetweenItem);
        childBottom -= (view->getMeasuredHeight() + lp->topMargin + getTopDecorationHeight(view)
                + spaceBetweenItem);
    }
    layoutState.mFlexLinePosition += mLayoutState->mLayoutDirection;
    return flexLine.getCrossSize();
}

View* FlexboxLayoutManager::getChildClosestToStart() {
    return getChildAt(0);
}

int FlexboxLayoutManager::computeHorizontalScrollOffset(RecyclerView::State& state) {
   int scrollOffset = computeScrollOffset(state);
    LOGD("computeHorizontalScrollOffset: " + scrollOffset);
    return scrollOffset;
}

int FlexboxLayoutManager::computeVerticalScrollOffset(RecyclerView::State& state) {
    int scrollOffset = computeScrollOffset(state);
    LOGD("computeVerticalScrollOffset: " + scrollOffset);
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
    //assert mFlexboxHelper->mIndexToFlexLine != nullptr;
    int minPosition = getPosition(firstReferenceView);
    int maxPosition = getPosition(lastReferenceView);
    int laidOutArea = std::abs(mOrientationHelper->getDecoratedEnd(lastReferenceView) -
            mOrientationHelper->getDecoratedStart(firstReferenceView));
    int firstLinePosition = mFlexboxHelper->mIndexToFlexLine[minPosition];
    if (firstLinePosition == 0 || firstLinePosition == NO_POSITION) {
        return 0;
    }
    int lastLinePosition = mFlexboxHelper->mIndexToFlexLine[maxPosition];
    int lineRange = lastLinePosition - firstLinePosition + 1;
    float averageSizePerLine = (float) laidOutArea / lineRange;
    // The number of lines before the first line is equal to the value of firstLinePosition
    return roundf(
            firstLinePosition * averageSizePerLine + (mOrientationHelper->getAfterPadding()
                    - mOrientationHelper->getDecoratedStart(firstReferenceView)));
}

int FlexboxLayoutManager::computeHorizontalScrollExtent(RecyclerView::State& state) {
    int scrollExtent = computeScrollExtent(state);
    LOGD("computeHorizontalScrollExtent: " + scrollExtent);
    return scrollExtent;
}

int FlexboxLayoutManager::computeVerticalScrollExtent(RecyclerView::State& state) {
    int scrollExtent = computeScrollExtent(state);
    LOGD("computeVerticalScrollExtent: " + scrollExtent);
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
    int scrollRange = computeScrollRange(state);
    LOGD("computeHorizontalScrollRange: " + scrollRange);
    return scrollRange;
}

int FlexboxLayoutManager::computeVerticalScrollRange(RecyclerView::State& state) {
    int scrollRange = computeScrollRange(state);
    LOGD("computeVerticalScrollRange: " + scrollRange);
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
    } else {
        detachAndScrapAttachedViews(recycler);
    }
}

Parcelable* FlexboxLayoutManager::onSaveInstanceState() {
    SavedState* state = new SavedState();
    state->mAnchorPosition = mAnchorPosition;
    state->mAnchorOffset = mAnchorOffset;
    state->mAnchorLayoutFromEnd = mAnchorLayoutFromEnd;
    return state;
}

void FlexboxLayoutManager::onRestoreInstanceState(Parcelable& state) {
    SavedState* savedState = static_cast<SavedState*>(&state);
    mAnchorPosition = savedState->mAnchorPosition;
    mAnchorOffset = savedState->mAnchorOffset;
    mAnchorLayoutFromEnd = savedState->mAnchorLayoutFromEnd;
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
    return findViewByPosition(index);
}

View* FlexboxLayoutManager::getReorderedFlexItemAt(int index) {
    if (index >= 0 && index < mReorderedIndices.size()) {
        return findViewByPosition(mReorderedIndices[index]);
    }
    return nullptr;
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
        mFlexDirection = flexDirection;
        requestLayout();
    }
}

int FlexboxLayoutManager::getFlexWrap() {
    return mFlexWrap;
}

void FlexboxLayoutManager::setFlexWrap(int flexWrap) {
    if (mFlexWrap != flexWrap) {
        mFlexWrap = flexWrap;
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
        mAlignItems = alignItems;
        requestLayout();
    }
}

int FlexboxLayoutManager::getAlignContent() {
    return mAlignContent;
}

void FlexboxLayoutManager::setAlignContent(int alignContent) {
    if (mAlignContent != alignContent) {
        mAlignContent = alignContent;
        requestLayout();
    }
}

std::vector<FlexLine> FlexboxLayoutManager::getFlexLines() {
    return mFlexLines;
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
    return ViewGroup::getChildMeasureSpec(widthSpec, padding, childDimension);
}

int FlexboxLayoutManager::getChildHeightMeasureSpec(int heightSpec, int padding, int childDimension) {
    return ViewGroup::getChildMeasureSpec(heightSpec, padding, childDimension);
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

std::vector<FlexLine> FlexboxLayoutManager::getFlexLinesInternal() {
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
        scrollVector.x = direction;
        scrollVector.y = 0;
    } else {
        scrollVector.x = 0;
        scrollVector.y = direction;
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
    int childLeft = getDecoratedLeft(view);
    int childTop = getDecoratedTop(view);
    int childRight = getDecoratedRight(view);
    int childBottom = getDecoratedBottom(view);

    bool horizontalCompletelyVisible = false;
    bool horizontalPartiallyVisible = false;
    bool verticalCompletelyVisible = false;
    bool verticalPartiallyVisible = false;

    if (left <= childLeft && right >= childRight) {
        horizontalCompletelyVisible = true;
    }
    if (childLeft < right && childRight > left) {
        horizontalPartiallyVisible = true;
    }

    if (top <= childTop && bottom >= childBottom) {
        verticalCompletelyVisible = true;
    }
    if (childTop < bottom && childBottom > top) {
        verticalPartiallyVisible = true;
    }

    if (completelyVisible) {
        return horizontalCompletelyVisible && verticalCompletelyVisible;
    } else {
        return horizontalPartiallyVisible && verticalPartiallyVisible;
    }
}

// SavedState implementation
FlexboxLayoutManager::SavedState::SavedState() {}

FlexboxLayoutManager::SavedState::SavedState(Parcel& in) {
    mAnchorPosition = in.readInt();
    mAnchorOffset = in.readInt();
    mAnchorLayoutFromEnd = in.readInt() != 0;
}

FlexboxLayoutManager::SavedState::SavedState(const SavedState& other) {
    mAnchorPosition = other.mAnchorPosition;
    mAnchorOffset = other.mAnchorOffset;
    mAnchorLayoutFromEnd = other.mAnchorLayoutFromEnd;
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
    dest.writeInt(mAnchorLayoutFromEnd ? 1 : 0);
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
    int increment = end > start ? 1 : -1;
    for (int i = start; i != end; i += increment) {
        View* child = getChildAt(i);
        if (child == nullptr) {
            continue;
        }
        int position = getPosition(child);
        if (position == RecyclerView::NO_POSITION) {
            continue;
        }
        if (position < 0 || position >= itemCount) {
            outOfBoundsMatch = child;
            continue;
        }
        return child;
    }
    return invalidMatch != nullptr ? invalidMatch : outOfBoundsMatch;
}

} // namespace cdroid
