#include <widget/gridview.h>
#include <widget/checkable.h>
#include <widget/R.h>
#include <utils/mathutils.h>
#include <cdlog.h>

namespace cdroid {

DECLARE_WIDGET2(GridView,"cdroid:attr/gridViewStyle")

GridView::GridView(int w,int h):AbsListView(w,h) {
    initGridView();
}

GridView::GridView(Context*ctx,const AttributeSet&atts)
    :AbsListView(ctx,atts) {
    initGridView();
    setHorizontalSpacing(atts.getDimensionPixelOffset("horizontalSpacing",10));
    setVerticalSpacing(atts.getDimensionPixelOffset("verticalSpacing",0));
    int index = atts.getInt("strechMode",std::unordered_map<std::string,int> {
        {"none", NO_STRETCH},
        {"spacingWidth",STRETCH_SPACING},
        {"columnWidth", STRETCH_COLUMN_WIDTH},
        {"spacingWidthUniform",STRETCH_SPACING_UNIFORM}
    },STRETCH_COLUMN_WIDTH);
    if(index>=0)setStretchMode(index);
    const int columnWidth = atts.getDimensionPixelOffset("columnWidth", -1);
    if (columnWidth > 0)
        setColumnWidth(columnWidth);
    const int numColumns = atts.getInt("numColumns", 1);
    setNumColumns(numColumns);
    index = atts.getGravity("gravity", -1);
    if (index >= 0) {
        setGravity(index);
    }
}

void GridView::initGridView() {
    mNumColumns = AUTO_FIT;
    mRequestedNumColumns = 0;
    mRequestedColumnWidth= 0;
    mHorizontalSpacing = 0;
    mVerticalSpacing   = 0;
    mRequestedHorizontalSpacing = 0;
    mStretchMode   = STRETCH_COLUMN_WIDTH;
    mReferenceView = nullptr;
    mReferenceViewInSelectedRow = nullptr;
    mGravity = Gravity::START;
}

Adapter*GridView::getAdapter() {
    return mAdapter;
}

void GridView::setAdapter(Adapter* adapter) {
    if (mAdapter  && mDataSetObserver ) {
        mAdapter->unregisterDataSetObserver(mDataSetObserver);
        delete mDataSetObserver;
        mDataSetObserver = nullptr;
    }

    resetList();
    mRecycler->clear();
    mAdapter = adapter;

    mOldSelectedPosition = INVALID_POSITION;
    mOldSelectedRowId = INVALID_ROW_ID;

    // AbsListView#setAdapter will update choice mode states.
    AbsListView::setAdapter(adapter);

    if (mAdapter ) {
        mOldItemCount = mItemCount;
        mItemCount = mAdapter->getCount();
        mDataChanged = true;
        checkFocus();

        mDataSetObserver = new AdapterDataSetObserver(this);
        mAdapter->registerDataSetObserver(mDataSetObserver);

        mRecycler->setViewTypeCount(mAdapter->getViewTypeCount());

        int position;
        if (mStackFromBottom) {
            position = lookForSelectablePosition(mItemCount - 1, false);
        } else {
            position = lookForSelectablePosition(0, true);
        }
        setSelectedPositionInt(position);
        setNextSelectedPositionInt(position);
        checkSelectionChanged();
    } else {
        checkFocus();
        // Nothing selected
        checkSelectionChanged();
    }
    requestLayout();
}

int GridView::lookForSelectablePosition(int position, bool lookDown) {
    if (mAdapter == nullptr || isInTouchMode()) {
        return INVALID_POSITION;
    }

    if (position < 0 || position >= mItemCount) {
        return INVALID_POSITION;
    }
    return position;
}

void GridView::fillGap(bool down) {
    const int numColumns = mNumColumns;
    const int verticalSpacing = mVerticalSpacing;

    const int count = getChildCount();

    if (down) {
        int paddingTop = 0;
        if ((mGroupFlags & CLIP_TO_PADDING_MASK) == CLIP_TO_PADDING_MASK) {
            paddingTop = getListPaddingTop();
        }
        int startOffset = count > 0 ? getChildAt(count - 1)->getBottom() + verticalSpacing : paddingTop;
        int position = mFirstPosition + count;
        if (mStackFromBottom) {
            position += numColumns - 1;
        }
        fillDown(position, startOffset);
        correctTooHigh(numColumns, verticalSpacing, getChildCount());
    } else {
        int paddingBottom = 0;
        if ((mGroupFlags & CLIP_TO_PADDING_MASK) == CLIP_TO_PADDING_MASK) {
            paddingBottom = getListPaddingBottom();
        }
        int startOffset = count > 0 ? getChildAt(0)->getTop() - verticalSpacing : getHeight() - paddingBottom;
        int position = mFirstPosition;
        if (!mStackFromBottom) {
            position -= numColumns;
        } else {
            position--;
        }
        fillUp(position, startOffset);
        correctTooLow(numColumns, verticalSpacing, getChildCount());
    }
}

View* GridView::fillDown(int pos, int nextTop) {
    View* selectedView = nullptr;

    int listEnd = (mBottom - mTop);
    if ((mGroupFlags & CLIP_TO_PADDING_MASK) == CLIP_TO_PADDING_MASK) {
        listEnd -= mListPadding.height;
    }

    while (nextTop < listEnd && pos < mItemCount) {
        View* temp = makeRow(pos, nextTop, true);
        if (temp != nullptr) {
            selectedView = temp;
        }

        // mReferenceView will change with each call to makeRow()
        // do not cache in a local variable outside of this loop
        nextTop = mReferenceView->getBottom() + mVerticalSpacing;

        pos += mNumColumns;
    }

    setVisibleRangeHint(mFirstPosition, mFirstPosition + getChildCount() - 1);
    return selectedView;
}

View* GridView::makeRow(int startPos, int y, bool flow) {
    const bool bIsLayoutRtl = isLayoutRtl();

    int last , nextLeft;

    if (bIsLayoutRtl) {
        nextLeft = getWidth() - mListPadding.width - mColumnWidth -
                   ((mStretchMode == STRETCH_SPACING_UNIFORM) ? mHorizontalSpacing : 0);
    } else {
        nextLeft = mListPadding.left + ((mStretchMode == STRETCH_SPACING_UNIFORM) ? mHorizontalSpacing : 0);
    }

    if (!mStackFromBottom) {
        last = std::min(startPos + mNumColumns, mItemCount);
    } else {
        last = startPos + 1;
        startPos = std::max(0, startPos - mNumColumns + 1);

        if (last - startPos < mNumColumns) {
            int deltaLeft = (mNumColumns - (last - startPos)) * (mColumnWidth + mHorizontalSpacing);
            nextLeft += (bIsLayoutRtl ? -1 : +1) * deltaLeft;
        }
    }

    View* selectedView = nullptr;

    const bool hasFocus = shouldShowSelector();
    const bool inClick = touchModeDrawsInPressedState();
    const int selectedPosition = mSelectedPosition;

    View* child = nullptr;
    const int nextChildDir = bIsLayoutRtl ? -1 : +1;
    for (int pos = startPos; pos < last; pos++) {
        // is this the selected item?
        bool selected = pos == selectedPosition;
        // does the list view have focus or contain focus

        const int where = flow ? -1 : pos - startPos;
        child = makeAndAddView(pos, y, flow, nextLeft, selected, where);

        nextLeft += nextChildDir * mColumnWidth;
        if (pos < last - 1) {
            nextLeft += nextChildDir * mHorizontalSpacing;
        }
        if (selected && (hasFocus || inClick)) {
            selectedView = child;
        }
    }

    mReferenceView = child;
    if (selectedView != nullptr) {
        mReferenceViewInSelectedRow = mReferenceView;
    }
    return selectedView;
}

View* GridView::fillUp(int pos, int nextBottom) {
    View* selectedView = nullptr;

    int end = 0;
    if ((mGroupFlags & CLIP_TO_PADDING_MASK) == CLIP_TO_PADDING_MASK) {
        end = mListPadding.top;
    }

    while (nextBottom > end && pos >= 0) {
        View* temp = makeRow(pos, nextBottom, false);
        if (temp != nullptr) {
            selectedView = temp;
        }
        nextBottom = mReferenceView->getTop() - mVerticalSpacing;
        mFirstPosition = pos;
        pos -= mNumColumns;
    }

    if (mStackFromBottom) {
        mFirstPosition = std::max(0, pos + 1);
    }

    setVisibleRangeHint(mFirstPosition, mFirstPosition + getChildCount() - 1);
    return selectedView;
}

View* GridView::fillFromTop(int nextTop) {
    mFirstPosition = std::min(mFirstPosition, mSelectedPosition);
    mFirstPosition = std::min(mFirstPosition, mItemCount - 1);
    if (mFirstPosition < 0) {
        mFirstPosition = 0;
    }
    mFirstPosition -= mFirstPosition % mNumColumns;
    return fillDown(mFirstPosition, nextTop);
}

View* GridView::fillFromBottom(int lastPosition, int nextBottom) {
    lastPosition = std::max(lastPosition, mSelectedPosition);
    lastPosition = std::min(lastPosition, mItemCount - 1);

    int invertedPosition = mItemCount - 1 - lastPosition;
    lastPosition = mItemCount - 1 - (invertedPosition - (invertedPosition % mNumColumns));

    return fillUp(lastPosition, nextBottom);
}

View* GridView::fillSelection(int childrenTop, int childrenBottom) {
    int selectedPosition = reconcileSelectedPosition();
    int numColumns = mNumColumns;
    int verticalSpacing = mVerticalSpacing;

    int rowStart;
    int rowEnd = -1;

    if (!mStackFromBottom) {
        rowStart = selectedPosition - (selectedPosition % numColumns);
    } else {
        int invertedSelection = mItemCount - 1 - selectedPosition;

        rowEnd = mItemCount - 1 - (invertedSelection - (invertedSelection % numColumns));
        rowStart = std::max(0, rowEnd - numColumns + 1);
    }

    int fadingEdgeLength = getVerticalFadingEdgeLength();
    int topSelectionPixel = getTopSelectionPixel(childrenTop, fadingEdgeLength, rowStart);

    View* sel = makeRow(mStackFromBottom ? rowEnd : rowStart, topSelectionPixel, true);
    mFirstPosition = rowStart;

    View* referenceView = mReferenceView;

    if (!mStackFromBottom) {
        fillDown(rowStart + numColumns, referenceView->getBottom() + verticalSpacing);
        pinToBottom(childrenBottom);
        fillUp(rowStart - numColumns, referenceView->getTop() - verticalSpacing);
        adjustViewsUpOrDown();
    } else {
        int bottomSelectionPixel = getBottomSelectionPixel(childrenBottom,
                                   fadingEdgeLength, numColumns, rowStart);
        int offset = bottomSelectionPixel - referenceView->getBottom();
        offsetChildrenTopAndBottom(offset);
        fillUp(rowStart - 1, referenceView->getTop() - verticalSpacing);
        pinToTop(childrenTop);
        fillDown(rowEnd + numColumns, referenceView->getBottom() + verticalSpacing);
        adjustViewsUpOrDown();
    }
    return sel;
}

void GridView::pinToTop(int childrenTop) {
    if (mFirstPosition == 0) {
        int top = getChildAt(0)->getTop();
        int offset = childrenTop - top;
        if (offset < 0) {
            offsetChildrenTopAndBottom(offset);
        }
    }
}

void GridView::pinToBottom(int childrenBottom) {
    int count = getChildCount();
    if (mFirstPosition + count == mItemCount) {
        int bottom = getChildAt(count - 1)->getBottom();
        int offset = childrenBottom - bottom;
        if (offset > 0) {
            offsetChildrenTopAndBottom(offset);
        }
    }
}

int GridView::findMotionRow(int y) {
    int childCount = getChildCount();
    if (childCount > 0) {
        int numColumns = mNumColumns;
        if (!mStackFromBottom) {
            for (int i = 0; i < childCount; i += numColumns) {
                if (y <= getChildAt(i)->getBottom()) {
                    return mFirstPosition + i;
                }
            }
        } else {
            for (int i = childCount - 1; i >= 0; i -= numColumns) {
                if (y >= getChildAt(i)->getTop()) {
                    return mFirstPosition + i;
                }
            }
        }
    }
    return INVALID_POSITION;
}

View* GridView::fillSpecific(int position, int top) {
    int numColumns = mNumColumns;

    int motionRowStart;
    int motionRowEnd = -1;

    if (!mStackFromBottom) {
        motionRowStart = position - (position % numColumns);
    } else {
        int invertedSelection = mItemCount - 1 - position;

        motionRowEnd = mItemCount - 1 - (invertedSelection - (invertedSelection % numColumns));
        motionRowStart = std::max(0, motionRowEnd - numColumns + 1);
    }

    View* temp = makeRow(mStackFromBottom ? motionRowEnd : motionRowStart, top, true);

    // Possibly changed again in fillUp if we add rows above this one.
    mFirstPosition = motionRowStart;

    View* referenceView = mReferenceView;
    // We didn't have anything to layout, bail out
    if (referenceView == nullptr) {
        return nullptr;
    }

    int verticalSpacing = mVerticalSpacing;

    View* above,* below;

    if (!mStackFromBottom) {
        above = fillUp(motionRowStart - numColumns, referenceView->getTop() - verticalSpacing);
        adjustViewsUpOrDown();
        below = fillDown(motionRowStart + numColumns, referenceView->getBottom() + verticalSpacing);
        // Check if we have dragged the bottom of the grid too high
        int childCount = getChildCount();
        if (childCount > 0) {
            correctTooHigh(numColumns, verticalSpacing, childCount);
        }
    } else {
        below = fillDown(motionRowEnd + numColumns, referenceView->getBottom() + verticalSpacing);
        adjustViewsUpOrDown();
        above = fillUp(motionRowStart - 1, referenceView->getTop() - verticalSpacing);
        // Check if we have dragged the bottom of the grid too high
        int childCount = getChildCount();
        if (childCount > 0) {
            correctTooLow(numColumns, verticalSpacing, childCount);
        }
    }

    if (temp != nullptr) {
        return temp;
    } else if (above != nullptr) {
        return above;
    } else {
        return below;
    }
}

void GridView::correctTooHigh(int numColumns, int verticalSpacing, int childCount) {
    int lastPosition = mFirstPosition + childCount - 1;
    if (lastPosition == mItemCount - 1 && childCount > 0) {
        // Get the last child ...
        View* lastChild = getChildAt(childCount - 1);

        // ... and its bottom edge
        int lastBottom = lastChild->getBottom();
        // This is bottom of our drawable area
        int end = mBottom-mTop - mListPadding.height;

        // This is how far the bottom edge of the last view is from the bottom of the
        // drawable area
        int bottomOffset = end - lastBottom;

        View* firstChild = getChildAt(0);
        const int firstTop = firstChild->getTop();

        // Make sure we are 1) Too high, and 2) Either there are more rows above the
        // first row or the first row is scrolled off the top of the drawable area
        if (bottomOffset > 0 && (mFirstPosition > 0 || firstTop < mListPadding.top))  {
            if (mFirstPosition == 0) {
                // Don't pull the top too far down
                bottomOffset = std::min(bottomOffset, mListPadding.top - firstTop);
            }

            // Move everything down
            offsetChildrenTopAndBottom(bottomOffset);
            if (mFirstPosition > 0) {
                // Fill the gap that was opened above mFirstPosition with more rows, if possible
                fillUp(mFirstPosition - (mStackFromBottom ? 1 : numColumns),
                       firstChild->getTop() - verticalSpacing);
                // Close up the remaining gap
                adjustViewsUpOrDown();
            }
        }
    }
}

void GridView::correctTooLow(int numColumns, int verticalSpacing, int childCount) {
    if (mFirstPosition == 0 && childCount > 0) {
        // Get the first child ...
        View* firstChild = getChildAt(0);

        // ... and its top edge
        int firstTop = firstChild->getTop();

        // This is top of our drawable area
        int start = mListPadding.top;

        // This is bottom of our drawable area
        int end = mBottom-mTop - mListPadding.height;

        // This is how far the top edge of the first view is from the top of the
        // drawable area
        int topOffset = firstTop - start;
        View* lastChild = getChildAt(childCount - 1);
        int lastBottom = lastChild->getBottom();
        int lastPosition = mFirstPosition + childCount - 1;

        // Make sure we are 1) Too low, and 2) Either there are more rows below the
        // last row or the last row is scrolled off the bottom of the drawable area
        if (topOffset > 0 && (lastPosition < mItemCount - 1 || lastBottom > end))  {
            if (lastPosition == mItemCount - 1 ) {
                // Don't pull the bottom too far up
                topOffset = std::min(topOffset, lastBottom - end);
            }

            // Move everything up
            offsetChildrenTopAndBottom(-topOffset);
            if (lastPosition < mItemCount - 1) {
                // Fill the gap that was opened below the last position with more rows, if
                // possible
                fillDown(lastPosition + (!mStackFromBottom ? 1 : numColumns),
                         lastChild->getBottom() + verticalSpacing);
                // Close up the remaining gap
                adjustViewsUpOrDown();
            }
        }
    }
}

View* GridView::fillFromSelection(int selectedTop, int childrenTop, int childrenBottom) {
    int fadingEdgeLength = getVerticalFadingEdgeLength();
    int selectedPosition = mSelectedPosition;
    int numColumns = mNumColumns;
    int verticalSpacing = mVerticalSpacing;

    int rowStart;
    int rowEnd = -1;

    if (!mStackFromBottom) {
        rowStart = selectedPosition - (selectedPosition % numColumns);
    } else {
        int invertedSelection = mItemCount - 1 - selectedPosition;

        rowEnd = mItemCount - 1 - (invertedSelection - (invertedSelection % numColumns));
        rowStart = std::max(0, rowEnd - numColumns + 1);
    }

    View* sel,*referenceView;

    const int topSelectionPixel = getTopSelectionPixel(childrenTop, fadingEdgeLength, rowStart);
    const int bottomSelectionPixel = getBottomSelectionPixel(childrenBottom, fadingEdgeLength,
                               numColumns, rowStart);

    sel = makeRow(mStackFromBottom ? rowEnd : rowStart, selectedTop, true);
    // Possibly changed again in fillUp if we add rows above this one.
    mFirstPosition = rowStart;

    referenceView = mReferenceView;
    adjustForTopFadingEdge(referenceView, topSelectionPixel, bottomSelectionPixel);
    adjustForBottomFadingEdge(referenceView, topSelectionPixel, bottomSelectionPixel);

    if (!mStackFromBottom) {
        fillUp(rowStart - numColumns, referenceView->getTop() - verticalSpacing);
        adjustViewsUpOrDown();
        fillDown(rowStart + numColumns, referenceView->getBottom() + verticalSpacing);
    } else {
        fillDown(rowEnd + numColumns, referenceView->getBottom() + verticalSpacing);
        adjustViewsUpOrDown();
        fillUp(rowStart - 1, referenceView->getTop() - verticalSpacing);
    }
    return sel;
}

int GridView::getBottomSelectionPixel(int childrenBottom, int fadingEdgeLength,
                                      int numColumns, int rowStart) {
    // Last pixel we can draw the selection into
    int bottomSelectionPixel = childrenBottom;
    if (rowStart + numColumns - 1 < mItemCount - 1) {
        bottomSelectionPixel -= fadingEdgeLength;
    }
    return bottomSelectionPixel;
}
int GridView::getTopSelectionPixel(int childrenTop, int fadingEdgeLength, int rowStart) {
    // first pixel we can draw the selection into
    int topSelectionPixel = childrenTop;
    if (rowStart > 0) {
        topSelectionPixel += fadingEdgeLength;
    }
    return topSelectionPixel;
}

void GridView::adjustForBottomFadingEdge(View* childInSelectedRow,int topSelectionPixel, int bottomSelectionPixel) {
    if (childInSelectedRow->getBottom() > bottomSelectionPixel) {

        // Find space available above the selection into which we can
        // scroll upwards
        int spaceAbove = childInSelectedRow->getTop() - topSelectionPixel;

        // Find space required to bring the bottom of the selected item
        // fully into view
        int spaceBelow = childInSelectedRow->getBottom() - bottomSelectionPixel;
        int offset = std::min(spaceAbove, spaceBelow);

        // Now offset the selected item to get it into view
        offsetChildrenTopAndBottom(-offset);
    }
}

void GridView::adjustForTopFadingEdge(View* childInSelectedRow,int topSelectionPixel, int bottomSelectionPixel) {
    if (childInSelectedRow->getTop() < topSelectionPixel) {
        // Find space required to bring the top of the selected item
        // fully into view
        int spaceAbove = topSelectionPixel - childInSelectedRow->getTop();

        // Find space available below the selection into which we can
        // scroll downwards
        int spaceBelow = bottomSelectionPixel - childInSelectedRow->getBottom();
        int offset = std::min(spaceAbove, spaceBelow);

        // Now offset the selected item to get it into view
        offsetChildrenTopAndBottom(offset);
    }
}

View* GridView::moveSelection(int delta, int childrenTop, int childrenBottom) {
    int fadingEdgeLength = getVerticalFadingEdgeLength();
    int selectedPosition = mSelectedPosition;
    int numColumns = mNumColumns;
    int verticalSpacing = mVerticalSpacing;

    int oldRowStart;
    int rowStart;
    int rowEnd = -1;

    if (!mStackFromBottom) {
        oldRowStart = (selectedPosition - delta) - ((selectedPosition - delta) % numColumns);

        rowStart = selectedPosition - (selectedPosition % numColumns);
    } else {
        int invertedSelection = mItemCount - 1 - selectedPosition;

        rowEnd = mItemCount - 1 - (invertedSelection - (invertedSelection % numColumns));
        rowStart = std::max(0, rowEnd - numColumns + 1);

        invertedSelection = mItemCount - 1 - (selectedPosition - delta);
        oldRowStart = mItemCount - 1 - (invertedSelection - (invertedSelection % numColumns));
        oldRowStart = std::max(0, oldRowStart - numColumns + 1);
    }

    int rowDelta = rowStart - oldRowStart;

    const int topSelectionPixel = getTopSelectionPixel(childrenTop, fadingEdgeLength, rowStart);
    const int bottomSelectionPixel = getBottomSelectionPixel(childrenBottom, fadingEdgeLength,
                               numColumns, rowStart);

    // Possibly changed again in fillUp if we add rows above this one.
    mFirstPosition = rowStart;

    View* sel,*referenceView;

    if (rowDelta > 0) {
        /* Case 1: Scrolling down. */

        const int oldBottom = mReferenceViewInSelectedRow? mReferenceViewInSelectedRow->getBottom():0;

        sel = makeRow(mStackFromBottom ? rowEnd : rowStart, oldBottom + verticalSpacing, true);
        referenceView = mReferenceView;

        adjustForBottomFadingEdge(referenceView, topSelectionPixel, bottomSelectionPixel);
    } else if (rowDelta < 0) {
        /* Case 2: Scrolling up. */
        const int oldTop = mReferenceViewInSelectedRow?mReferenceViewInSelectedRow->getTop():0;

        sel = makeRow(mStackFromBottom ? rowEnd : rowStart, oldTop - verticalSpacing, false);
        referenceView = mReferenceView;

        adjustForTopFadingEdge(referenceView, topSelectionPixel, bottomSelectionPixel);
    } else {
        /*Keep selection where it was */
        const int oldTop = mReferenceViewInSelectedRow ?mReferenceViewInSelectedRow->getTop():0;

        sel = makeRow(mStackFromBottom ? rowEnd : rowStart, oldTop, true);
        referenceView = mReferenceView;
    }

    if (!mStackFromBottom) {
        fillUp(rowStart - numColumns, referenceView->getTop() - verticalSpacing);
        adjustViewsUpOrDown();
        fillDown(rowStart + numColumns, referenceView->getBottom() + verticalSpacing);
    } else {
        fillDown(rowEnd + numColumns, referenceView->getBottom() + verticalSpacing);
        adjustViewsUpOrDown();
        fillUp(rowStart - 1, referenceView->getTop() - verticalSpacing);
    }

    return sel;
}

bool GridView::determineColumns(int availableSpace) {
    int requestedHorizontalSpacing = mRequestedHorizontalSpacing;
    int stretchMode = mStretchMode;
    int requestedColumnWidth = mRequestedColumnWidth;
    bool didNotInitiallyFit = false;

    if (mRequestedNumColumns == AUTO_FIT) {
        if (requestedColumnWidth > 0) {
            // Client told us to pick the number of columns
            mNumColumns = (availableSpace + requestedHorizontalSpacing) /
                          (requestedColumnWidth + requestedHorizontalSpacing);
        } else {
            // Just make up a number if we don't have enough info
            mNumColumns = 2;
        }
    } else {
        // We picked the columns
        mNumColumns = mRequestedNumColumns;
    }

    if (mNumColumns <= 0) {
        mNumColumns = 1;
    }

    switch (stretchMode) {
    case NO_STRETCH:
        // Nobody stretches
        mColumnWidth = requestedColumnWidth;
        mHorizontalSpacing = requestedHorizontalSpacing;
        break;

    default:
        int spaceLeftOver = availableSpace - (mNumColumns * requestedColumnWidth)
                            - ((mNumColumns - 1) * requestedHorizontalSpacing);

        if (spaceLeftOver < 0) {
            didNotInitiallyFit = true;
        }

        switch (stretchMode) {
        case STRETCH_COLUMN_WIDTH:
            // Stretch the columns
            mColumnWidth = requestedColumnWidth + spaceLeftOver / mNumColumns;
            mHorizontalSpacing = requestedHorizontalSpacing;
            break;

        case STRETCH_SPACING:
            // Stretch the spacing between columns
            mColumnWidth = requestedColumnWidth;
            if (mNumColumns > 1) {
                mHorizontalSpacing = requestedHorizontalSpacing
                                     + spaceLeftOver / (mNumColumns - 1);
            } else {
                mHorizontalSpacing = requestedHorizontalSpacing + spaceLeftOver;
            }
            break;

        case STRETCH_SPACING_UNIFORM:
            // Stretch the spacing between columns
            mColumnWidth = requestedColumnWidth;
            if (mNumColumns > 1) {
                mHorizontalSpacing = requestedHorizontalSpacing
                                     + spaceLeftOver / (mNumColumns + 1);
            } else {
                mHorizontalSpacing = requestedHorizontalSpacing + spaceLeftOver;
            }
            break;
        }
        break;
    }
    return didNotInitiallyFit;
}

void GridView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    // Sets up mListPadding
    AbsListView::onMeasure(widthMeasureSpec, heightMeasureSpec);

    const int widthMode  = MeasureSpec::getMode(widthMeasureSpec);
    const int heightMode = MeasureSpec::getMode(heightMeasureSpec);
    int widthSize  = MeasureSpec::getSize(widthMeasureSpec);
    int heightSize = MeasureSpec::getSize(heightMeasureSpec);

    if (widthMode == MeasureSpec::UNSPECIFIED) {
        if (mColumnWidth > 0) {
            widthSize = mColumnWidth + mListPadding.left + mListPadding.width;
        } else {
            widthSize = mListPadding.left + mListPadding.width;
        }
        widthSize += getVerticalScrollbarWidth();
    }

    const int childWidth = widthSize - mListPadding.left - mListPadding.width;
    const bool didNotInitiallyFit = determineColumns(childWidth);

    int childHeight = 0;
    int childState = 0;

    mItemCount = mAdapter ? mAdapter->getCount():0;
    const int count = mItemCount;
    if (count > 0) {
        View* child = obtainView(0, mIsScrap);
        AbsListView::LayoutParams* p = (AbsListView::LayoutParams*) child->getLayoutParams();
        if (p == nullptr) {
            p = (AbsListView::LayoutParams*) generateDefaultLayoutParams();
            child->setLayoutParams(p);
        }
        p->viewType = mAdapter->getItemViewType(0);
        p->isEnabled= mAdapter->isEnabled(0);
        p->forceAdd = true;

        const int childHeightSpec= getChildMeasureSpec(
                                       MeasureSpec::makeSafeMeasureSpec(MeasureSpec::getSize(heightMeasureSpec),
                                               MeasureSpec::UNSPECIFIED), 0, p->height);
        const int childWidthSpec = getChildMeasureSpec(
                                       MeasureSpec::makeMeasureSpec(mColumnWidth, MeasureSpec::EXACTLY), 0, p->width);
        child->measure(childWidthSpec, childHeightSpec);

        childHeight = child->getMeasuredHeight();
        childState = combineMeasuredStates(childState, child->getMeasuredState());

        if (mRecycler->shouldRecycleViewType(p->viewType)) {
            mRecycler->addScrapView(child, -1);
        }
    }

    if (heightMode == MeasureSpec::UNSPECIFIED) {
        heightSize = mListPadding.top + mListPadding.height + childHeight +
                     getVerticalFadingEdgeLength() * 2;
    }

    if (heightMode == MeasureSpec::AT_MOST) {
        int ourSize =  mListPadding.top + mListPadding.height;
        for (int i = 0; i < count; i += mNumColumns) {
            ourSize += childHeight;
            if (i + mNumColumns < count) {
                ourSize += mVerticalSpacing;
            }
            if (ourSize >= heightSize) {
                ourSize = heightSize;
                break;
            }
        }
        heightSize = ourSize;
    }

    if (widthMode == MeasureSpec::AT_MOST && mRequestedNumColumns != AUTO_FIT) {
        int ourSize = (mRequestedNumColumns*mColumnWidth)
                      + ((mRequestedNumColumns-1)*mHorizontalSpacing)
                      + mListPadding.left + mListPadding.width;
        if (ourSize > widthSize || didNotInitiallyFit) {
            widthSize |= MEASURED_STATE_TOO_SMALL;
        }
    }

    setMeasuredDimension(widthSize, heightSize);
    mWidthMeasureSpec = widthMeasureSpec;
}

void GridView::layoutChildren() {
    bool blockLayoutRequests = mBlockLayoutRequests;
    if (!blockLayoutRequests) {
        mBlockLayoutRequests = true;
    }

    AbsListView::layoutChildren();

    invalidate();

    if (mAdapter == nullptr) {
        resetList();
        invokeOnItemScrollListener();
        mBlockLayoutRequests=false;
        return;
    }

    const int childrenTop = mListPadding.top;
    const int childrenBottom = getHeight() - mListPadding.height;

    const int childCount = getChildCount();
    int index;
    int delta = 0;

    View* sel;
    View* oldSel = nullptr;
    View* oldFirst = nullptr;
    View* newSel = nullptr;

    // Remember stuff we will need down below
    switch (mLayoutMode) {
    case LAYOUT_SET_SELECTION:
        index = mNextSelectedPosition - mFirstPosition;
        if (index >= 0 && index < childCount) {
            newSel = getChildAt(index);
        }
        break;
    case LAYOUT_FORCE_TOP:
    case LAYOUT_FORCE_BOTTOM:
    case LAYOUT_SPECIFIC:
    case LAYOUT_SYNC:
        break;
    case LAYOUT_MOVE_SELECTION:
        if (mNextSelectedPosition >= 0) {
            delta = mNextSelectedPosition - mSelectedPosition;
        }
        break;
    default:
        // Remember the previously selected view
        index = mSelectedPosition - mFirstPosition;
        if ((index >= 0) && (index < childCount)) {
            oldSel = getChildAt(index);
        }

        // Remember the previous first child
        oldFirst = getChildAt(0);
    }

    const bool dataChanged = mDataChanged;
    if (dataChanged) {
        handleDataChanged();
    }

    // Handle the empty set by removing all views that are visible
    // and calling it a day
    if (mItemCount == 0) {
        resetList();
        invokeOnItemScrollListener();
        mBlockLayoutRequests=false;
        return;
    }

    setSelectedPositionInt(mNextSelectedPosition);

    AccessibilityNodeInfo* accessibilityFocusLayoutRestoreNode = nullptr;
    View* accessibilityFocusLayoutRestoreView = nullptr;
    int accessibilityFocusPosition = INVALID_POSITION;

    // Remember which child, if any, had accessibility focus. This must
    // occur before recycling any views, since that will clear
    // accessibility focus.
    ViewGroup* viewRootImpl = getRootView();
    if (viewRootImpl != nullptr) {
        View* focusHost = viewRootImpl->getAccessibilityFocusedHost();
        if (focusHost != nullptr) {
            View* focusChild = getAccessibilityFocusedChild(focusHost);
            if (focusChild != nullptr) {
                if (!dataChanged || focusChild->hasTransientState()|| mAdapterHasStableIds) {
                    // The views won't be changing, so try to maintain
                    // focus on the current host and virtual view.
                    accessibilityFocusLayoutRestoreView = focusHost;
                    accessibilityFocusLayoutRestoreNode = viewRootImpl->getAccessibilityFocusedVirtualView();
                }

                // Try to maintain focus at the same position.
                accessibilityFocusPosition = getPositionForView(focusChild);
            }
        }
    }

    // Pull all children into the RecycleBin.
    // These views will be reused if possible
    int firstPosition = mFirstPosition;

    if (dataChanged) {
        for (int i = 0; i < childCount; i++) {
            mRecycler->addScrapView(getChildAt(i), firstPosition+i);
        }
    } else {
        mRecycler->fillActiveViews(childCount, firstPosition);
    }

    // Clear out old views
    detachAllViewsFromParent();
    mRecycler->removeSkippedScrap();

    switch (mLayoutMode) {
    case LAYOUT_SET_SELECTION:
        if (newSel ) {
            sel = fillFromSelection(newSel->getTop(), childrenTop, childrenBottom);
        } else {
            sel = fillSelection(childrenTop, childrenBottom);
        }
        break;
    case LAYOUT_FORCE_TOP:
        mFirstPosition = 0;
        sel = fillFromTop(childrenTop);
        adjustViewsUpOrDown();
        break;
    case LAYOUT_FORCE_BOTTOM:
        sel = fillUp(mItemCount - 1, childrenBottom);
        adjustViewsUpOrDown();
        break;
    case LAYOUT_SPECIFIC:
        sel = fillSpecific(mSelectedPosition, mSpecificTop);
        break;
    case LAYOUT_SYNC:
        sel = fillSpecific(mSyncPosition, mSpecificTop);
        break;
    case LAYOUT_MOVE_SELECTION:
        // Move the selection relative to its old position
        sel = moveSelection(delta, childrenTop, childrenBottom);
        break;
    default:
        if (childCount == 0) {
            if (!mStackFromBottom) {
                setSelectedPositionInt(mAdapter == nullptr || isInTouchMode() ?
                                       INVALID_POSITION : 0);
                sel = fillFromTop(childrenTop);
            } else {
                const int last = mItemCount - 1;
                setSelectedPositionInt(mAdapter == nullptr || isInTouchMode() ?
                                       INVALID_POSITION : last);
                sel = fillFromBottom(last, childrenBottom);
            }
        } else {
            if (mSelectedPosition >= 0 && mSelectedPosition < mItemCount) {
                sel = fillSpecific(mSelectedPosition, oldSel == nullptr ?
                                   childrenTop : oldSel->getTop());
            } else if (mFirstPosition < mItemCount)  {
                sel = fillSpecific(mFirstPosition, oldFirst == nullptr ?
                                   childrenTop : oldFirst->getTop());
            } else {
                sel = fillSpecific(0, childrenTop);
            }
        }
        break;
    }

    // Flush any cached views that did not get reused above
    mRecycler->scrapActiveViews();

    if (sel != nullptr) {
        positionSelector(INVALID_POSITION, sel);
        mSelectedTop = sel->getTop();
    } else {
        bool inTouchMode = mTouchMode > TOUCH_MODE_DOWN
                           && mTouchMode < TOUCH_MODE_SCROLL;
        if (inTouchMode) {
            // If the user's finger is down, select the motion position.
            View* child = getChildAt(mMotionPosition - mFirstPosition);
            if (child != nullptr) {
                positionSelector(mMotionPosition, child);
            }
        } else if (mSelectedPosition != INVALID_POSITION) {
            // If we had previously positioned the selector somewhere,
            // put it back there. It might not match up with the data,
            // but it's transitioning out so it's not a big deal.
            View* child = getChildAt(mSelectorPosition - mFirstPosition);
            if (child != nullptr) {
                positionSelector(mSelectorPosition, child);
            }
        } else {
            // Otherwise, clear selection.
            mSelectedTop = 0;
            mSelectorRect.setEmpty();
        }
    }

    // Attempt to restore accessibility focus, if necessary.
    if (viewRootImpl != nullptr) {
        View* newAccessibilityFocusedView = viewRootImpl->getAccessibilityFocusedHost();
        if (newAccessibilityFocusedView == nullptr) {
            if (accessibilityFocusLayoutRestoreView != nullptr
                    && accessibilityFocusLayoutRestoreView->isAttachedToWindow()) {
                AccessibilityNodeProvider* provider = accessibilityFocusLayoutRestoreView->getAccessibilityNodeProvider();
                if (accessibilityFocusLayoutRestoreNode && provider) {
                    int virtualViewId = AccessibilityNodeInfo::getVirtualDescendantId(
                                            accessibilityFocusLayoutRestoreNode->getSourceNodeId());
                    provider->performAction(virtualViewId, AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS, nullptr);
                } else {
                    accessibilityFocusLayoutRestoreView->requestAccessibilityFocus();
                }
            } else if (accessibilityFocusPosition != INVALID_POSITION) {
                // Bound the position within the visible children.
                const int position = MathUtils::constrain(accessibilityFocusPosition - mFirstPosition, 0, getChildCount() - 1);
                View* restoreView = getChildAt(position);
                if (restoreView ) restoreView->requestAccessibilityFocus();
            }
        }
    }

    mLayoutMode = LAYOUT_NORMAL;
    mDataChanged = false;
    /*if (mPositionScrollAfterLayout != nullptr) {
        post(mPositionScrollAfterLayout);
        mPositionScrollAfterLayout = nullptr;
    }*/
    mNeedSync = false;
    setNextSelectedPositionInt(mSelectedPosition);

    updateScrollIndicators();

    if (mItemCount > 0) {
        checkSelectionChanged();
    }

    invokeOnItemScrollListener();

    mBlockLayoutRequests = false;
}

View* GridView::makeAndAddView(int position, int y, bool flow, int childrenLeft,bool selected, int where) {
    if (!mDataChanged) {
        // Try to use an existing view for this position
        View* activeView = mRecycler->getActiveView(position);
        if (activeView != nullptr) {
            // Found it -- we're using an existing child
            // This just needs to be positioned
            setupChild(activeView, position, y, flow, childrenLeft, selected, true, where);
            return activeView;
        }
    }

    // Make a new view for this position, or convert an unused view if possible.
    View* child = obtainView(position, mIsScrap);

    // This needs to be positioned and measured.
    setupChild(child, position, y, flow, childrenLeft, selected, mIsScrap[0], where);
    return child;
}

void GridView::setupChild(View* child, int position, int y, bool flowDown, int childrenLeft,
                          bool selected, bool isAttachedToWindow, int where) {
    const bool isSelected = selected && shouldShowSelector();
    const bool updateChildSelected = isSelected != child->isSelected();
    const bool isPressed = (mTouchMode > TOUCH_MODE_DOWN) && (mTouchMode < TOUCH_MODE_SCROLL)
                           && (mMotionPosition == position);
    const bool updateChildPressed = isPressed != child->isPressed();
    const bool needToMeasure = !isAttachedToWindow || updateChildSelected
                               || child->isLayoutRequested();

    // Respect layout params that are already in the view. Otherwise make
    // some up...
    AbsListView::LayoutParams* p = (AbsListView::LayoutParams*) child->getLayoutParams();
    if (p == nullptr) {
        p = (AbsListView::LayoutParams*) generateDefaultLayoutParams();
    }
    p->viewType = mAdapter->getItemViewType(position);
    p->isEnabled= mAdapter->isEnabled(position);

    // Set up view state before attaching the view, since we may need to
    // rely on the jumpDrawablesToCurrentState() call that occurs as part
    // of view attachment.
    if (updateChildSelected) {
        child->setSelected(isSelected);
        if (isSelected) {
            requestFocus();
        }
    }

    if (updateChildPressed) {
        child->setPressed(isPressed);
    }

    if ((mChoiceMode != CHOICE_MODE_NONE) && (mCheckStates!=nullptr)) {
        if (dynamic_cast<Checkable*>(child)) {
            ((Checkable*) child)->setChecked(mCheckStates->get(position));
        } else {
            child->setActivated(mCheckStates->get(position));
        }
    }

    if (isAttachedToWindow && !p->forceAdd) {
        attachViewToParent(child, where, p);

        // If the view isn't attached, or if it's attached but for a different
        // position, then jump the drawables.
        if (!isAttachedToWindow
                || (((AbsListView::LayoutParams*) child->getLayoutParams())->scrappedFromPosition)
                != position) {
            child->jumpDrawablesToCurrentState();
        }
    } else {
        p->forceAdd = false;
        addViewInLayout(child, where, p, true);
    }

    if (needToMeasure) {
        const int childHeightSpec = ViewGroup::getChildMeasureSpec(MeasureSpec::makeMeasureSpec(0, MeasureSpec::UNSPECIFIED), 0, p->height);
        const int childWidthSpec = ViewGroup::getChildMeasureSpec(MeasureSpec::makeMeasureSpec(mColumnWidth, MeasureSpec::EXACTLY), 0, p->width);
        child->measure(childWidthSpec, childHeightSpec);
    } else {
        cleanupLayoutState(child);
    }

    const int w = child->getMeasuredWidth();
    const int h = child->getMeasuredHeight();

    int childLeft;
    int childTop = flowDown ? y : y - h;

    const int layoutDirection = getLayoutDirection();
    const int absoluteGravity = Gravity::getAbsoluteGravity(mGravity, layoutDirection);
    switch (absoluteGravity & Gravity::HORIZONTAL_GRAVITY_MASK) {
    case Gravity::LEFT:
        childLeft = childrenLeft;
        break;
    case Gravity::CENTER_HORIZONTAL:
        childLeft = childrenLeft + ((mColumnWidth - w) / 2);
        break;
    case Gravity::RIGHT:
        childLeft = childrenLeft + mColumnWidth - w;
        break;
    default:
        childLeft = childrenLeft;
        break;
    }

    if (needToMeasure) {
        child->layout(childLeft, childTop, w, h);
    } else {
        child->offsetLeftAndRight(childLeft - child->getLeft());
        child->offsetTopAndBottom(childTop - child->getTop());
    }

    if (mCachingStarted && !child->isDrawingCacheEnabled()) {
        child->setDrawingCacheEnabled(true);
    }
}

void GridView::setSelection(int position) {
    if (!isInTouchMode()) {
        setNextSelectedPositionInt(position);
    } else {
        mResurrectToPosition = position;
    }
    mLayoutMode = LAYOUT_SET_SELECTION;
    if (mPositionScroller) mPositionScroller->stop();
    requestLayout();
}

void GridView::setSelectionInt(int position) {
    const int previousSelectedPosition = mNextSelectedPosition;

    if (mPositionScroller) mPositionScroller->stop();

    setNextSelectedPositionInt(position);
    layoutChildren();

    const int next = mStackFromBottom ? mItemCount - 1  - mNextSelectedPosition : mNextSelectedPosition;
    const int previous = mStackFromBottom ? mItemCount - 1 - previousSelectedPosition : previousSelectedPosition;

    int nextRow = next / mNumColumns;
    int previousRow = previous / mNumColumns;

    if (nextRow != previousRow) {
        awakenScrollBars();
    }
}

bool GridView::dispatchKeyEvent(KeyEvent& event) {
    bool handled = AbsListView::dispatchKeyEvent(event);
    if (!handled) {// If we didn't handle it...
        View* focused = getFocusedChild();
        if (focused && (event.getAction() == KeyEvent::ACTION_DOWN)) {
            // ... and our focused child didn't handle it
            // ... give it to ourselves so we can scroll if necessary
            handled = onKeyDown(event.getKeyCode(), event);
        }
        handled = commonKey(event.getKeyCode(), 1,event);
    }
    return handled;
}

bool GridView::onKeyDown(int keyCode, KeyEvent& event) {
    return commonKey(keyCode, 1, event);
}

bool GridView::onKeyMultiple(int keyCode, int repeatCount, KeyEvent& event) {
    return commonKey(keyCode, repeatCount, event);
}

bool GridView::onKeyUp(int keyCode, KeyEvent& event) {
    return commonKey(keyCode, 1, event);
}

bool GridView::commonKey(int keyCode, int count, KeyEvent& event) {
    if (mAdapter == nullptr) {
        return false;
    }

    if (mDataChanged) {
        layoutChildren();
    }

    bool handled = false;
    const int action = event.getAction();
    if (KeyEvent::isConfirmKey(keyCode)
            && event.hasNoModifiers() && action != KeyEvent::ACTION_UP) {
        handled = resurrectSelectionIfNeeded();
        if (!handled && event.getRepeatCount() == 0 && getChildCount() > 0) {
            keyPressed();
            handled = true;
        }
    }

    if (!handled && action != KeyEvent::ACTION_UP) {
        switch (keyCode) {
        case KeyEvent::KEYCODE_DPAD_LEFT:
            if (event.hasNoModifiers()) {
                handled = resurrectSelectionIfNeeded() || arrowScroll(FOCUS_LEFT);
            }
            break;

        case KeyEvent::KEYCODE_DPAD_RIGHT:
            if (event.hasNoModifiers()) {
                handled = resurrectSelectionIfNeeded() || arrowScroll(FOCUS_RIGHT);
            }
            break;

        case KeyEvent::KEYCODE_DPAD_UP:
            if (event.hasNoModifiers()) {
                handled = resurrectSelectionIfNeeded() || arrowScroll(FOCUS_UP);
            } else if (event.hasModifiers(KeyEvent::META_ALT_ON)) {
                handled = resurrectSelectionIfNeeded() || fullScroll(FOCUS_UP);
            }
            break;

        case KeyEvent::KEYCODE_DPAD_DOWN:
            if (event.hasNoModifiers()) {
                handled = resurrectSelectionIfNeeded() || arrowScroll(FOCUS_DOWN);
            } else if (event.hasModifiers(KeyEvent::META_ALT_ON)) {
                handled = resurrectSelectionIfNeeded() || fullScroll(FOCUS_DOWN);
            }
            break;

        case KeyEvent::KEYCODE_PAGE_UP:
            if (event.hasNoModifiers()) {
                handled = resurrectSelectionIfNeeded() || pageScroll(FOCUS_UP);
            } else if (event.hasModifiers(KeyEvent::META_ALT_ON)) {
                handled = resurrectSelectionIfNeeded() || fullScroll(FOCUS_UP);
            }
            break;

        case KeyEvent::KEYCODE_PAGE_DOWN:
            if (event.hasNoModifiers()) {
                handled = resurrectSelectionIfNeeded() || pageScroll(FOCUS_DOWN);
            } else if (event.hasModifiers(KeyEvent::META_ALT_ON)) {
                handled = resurrectSelectionIfNeeded() || fullScroll(FOCUS_DOWN);
            }
            break;

        case KeyEvent::KEYCODE_MOVE_HOME:
            if (event.hasNoModifiers()) {
                handled = resurrectSelectionIfNeeded() || fullScroll(FOCUS_UP);
            }
            break;

        case KeyEvent::KEYCODE_MOVE_END:
            if (event.hasNoModifiers()) {
                handled = resurrectSelectionIfNeeded() || fullScroll(FOCUS_DOWN);
            }
            break;

        case KeyEvent::KEYCODE_TAB:
            // TODO: Sometimes it is useful to be able to TAB through the items in
            //     a GridView sequentially.  Unfortunately this can create an
            //     asymmetry in TAB navigation order unless the list selection
            //     always reverts to the top or bottom when receiving TAB focus from
            //     another widget.
            if (event.hasNoModifiers()) {
                handled = resurrectSelectionIfNeeded() || sequenceScroll(FOCUS_FORWARD);
            } else if (event.hasModifiers(KeyEvent::META_SHIFT_ON)) {
                handled = resurrectSelectionIfNeeded() || sequenceScroll(FOCUS_BACKWARD);
            }
            break;
        }
    }

    if (handled) {
        return true;
    }

    //if (sendToTextFilter(keyCode, count, event)) return true;

    switch (action) {
    case KeyEvent::ACTION_DOWN:    return AbsListView::onKeyDown(keyCode, event);
    case KeyEvent::ACTION_UP:      return AbsListView::onKeyUp(keyCode, event);
    case KeyEvent::ACTION_MULTIPLE:return AbsListView::onKeyMultiple(keyCode, count, event);
    default: return false;
    }
}

bool GridView::pageScroll(int direction) {
    int nextPage = -1;

    if (direction == FOCUS_UP) {
        nextPage = std::max(0, mSelectedPosition - getChildCount());
    } else if (direction == FOCUS_DOWN) {
        nextPage = std::min(mItemCount - 1, mSelectedPosition + getChildCount());
    }
    if (nextPage >= 0) {
        setSelectionInt(nextPage);
        invokeOnItemScrollListener();
        awakenScrollBars();
        return true;
    }
    return false;
}

bool GridView::fullScroll(int direction) {
    bool moved = false;
    if (direction == FOCUS_UP) {
        mLayoutMode = LAYOUT_SET_SELECTION;
        setSelectionInt(0);
        invokeOnItemScrollListener();
        moved = true;
    } else if (direction == FOCUS_DOWN) {
        mLayoutMode = LAYOUT_SET_SELECTION;
        setSelectionInt(mItemCount - 1);
        invokeOnItemScrollListener();
        moved = true;
    }

    if (moved) awakenScrollBars();
    return moved;
}

bool GridView::arrowScroll(int direction) {
    int selectedPosition = mSelectedPosition;
    int numColumns = mNumColumns;

    int startOfRowPos;
    int endOfRowPos;

    bool moved = false;

    if (!mStackFromBottom) {
        startOfRowPos= (selectedPosition / numColumns) * numColumns;
        endOfRowPos  = std::min(startOfRowPos + numColumns - 1, mItemCount - 1);
    } else {
        int invertedSelection = mItemCount - 1 - selectedPosition;
        endOfRowPos  = mItemCount - 1 - (invertedSelection / numColumns) * numColumns;
        startOfRowPos= std::max(0, endOfRowPos - numColumns + 1);
    }

    switch (direction) {
    case FOCUS_UP:
        if (startOfRowPos > 0) {
            mLayoutMode = LAYOUT_MOVE_SELECTION;
            setSelectionInt(std::max(0, selectedPosition - numColumns));
            moved = true;
        }
        break;
    case FOCUS_DOWN:
        if (endOfRowPos < mItemCount - 1) {
            mLayoutMode = LAYOUT_MOVE_SELECTION;
            setSelectionInt(std::min(selectedPosition + numColumns, mItemCount - 1));
            moved = true;
        }
        break;
    }

    const bool islayoutRtl = isLayoutRtl();
    if (selectedPosition > startOfRowPos && ((direction == FOCUS_LEFT && !islayoutRtl) ||
            (direction == FOCUS_RIGHT && islayoutRtl))) {
        mLayoutMode = LAYOUT_MOVE_SELECTION;
        setSelectionInt(std::max(0, selectedPosition - 1));
        moved = true;
    } else if (selectedPosition < endOfRowPos && ((direction == FOCUS_LEFT && islayoutRtl) ||
               (direction == FOCUS_RIGHT && !islayoutRtl))) {
        mLayoutMode = LAYOUT_MOVE_SELECTION;
        setSelectionInt(std::min(selectedPosition + 1, mItemCount - 1));
        moved = true;
    }

    if (moved) {
        playSoundEffect(SoundEffectConstants::getContantForFocusDirection(direction));
        invokeOnItemScrollListener();
    }
    if (moved) awakenScrollBars();

    return moved;
}

bool GridView::sequenceScroll(int direction) {
    int selectedPosition = mSelectedPosition;
    int numColumns = mNumColumns;
    int count = mItemCount;

    int startOfRow;
    int endOfRow;
    if (!mStackFromBottom) {
        startOfRow= (selectedPosition / numColumns) * numColumns;
        endOfRow  = std::min(startOfRow + numColumns - 1, count - 1);
    } else {
        int invertedSelection = count - 1 - selectedPosition;
        endOfRow  = count - 1 - (invertedSelection / numColumns) * numColumns;
        startOfRow= std::max(0, endOfRow - numColumns + 1);
    }

    bool moved = false;
    bool showScroll = false;
    switch (direction) {
    case FOCUS_FORWARD:
        if (selectedPosition < count - 1) {
            // Move to the next item.
            mLayoutMode = LAYOUT_MOVE_SELECTION;
            setSelectionInt(selectedPosition + 1);
            moved = true;
            // Show the scrollbar only if changing rows.
            showScroll = selectedPosition == endOfRow;
        }
        break;

    case FOCUS_BACKWARD:
        if (selectedPosition > 0) {
            // Move to the previous item.
            mLayoutMode = LAYOUT_MOVE_SELECTION;
            setSelectionInt(selectedPosition - 1);
            moved = true;
            // Show the scrollbar only if changing rows.
            showScroll = selectedPosition == startOfRow;
        }
        break;
    }

    if (moved) {
        playSoundEffect(SoundEffectConstants::getContantForFocusDirection(direction));
        invokeOnItemScrollListener();
    }

    if (showScroll) awakenScrollBars();

    return moved;
}

void GridView::setGravity(int gravity) {
    if (mGravity != gravity) {
        mGravity = gravity;
        requestLayoutIfNecessary();
    }
}

int GridView::getGravity()const {
    return mGravity;
}

void GridView::setHorizontalSpacing(int horizontalSpacing) {
    if (horizontalSpacing != mRequestedHorizontalSpacing) {
        mRequestedHorizontalSpacing = horizontalSpacing;
        requestLayoutIfNecessary();
    }
}

int GridView::getHorizontalSpacing()const {
    return mHorizontalSpacing;
}

int GridView::getRequestedHorizontalSpacing() const {
    return mRequestedHorizontalSpacing;
}

void GridView::setVerticalSpacing(int verticalSpacing) {
    if (verticalSpacing != mVerticalSpacing) {
        mVerticalSpacing = verticalSpacing;
        requestLayoutIfNecessary();
    }
}

int GridView::getVerticalSpacing()const {
    return mVerticalSpacing;
}

void GridView::setStretchMode( int stretchMode) {
    if (stretchMode != mStretchMode) {
        mStretchMode = stretchMode;
        requestLayoutIfNecessary();
    }
}

int GridView::getStretchMode()const {
    return mStretchMode;
}

void GridView::setColumnWidth(int columnWidth) {
    if (columnWidth != mRequestedColumnWidth) {
        mRequestedColumnWidth = columnWidth;
        requestLayoutIfNecessary();
    }
}

int GridView::getColumnWidth()const {
    return mColumnWidth;
}

int GridView::getRequestedColumnWidth() const {
    return mRequestedColumnWidth;
}

void GridView::setNumColumns(int numColumns) {
    if (numColumns != mRequestedNumColumns) {
        mRequestedNumColumns = numColumns;
        requestLayoutIfNecessary();
    }
}

int GridView::getNumColumns()const {
    return mNumColumns;
}

void GridView::adjustViewsUpOrDown(){
    const int childCount = getChildCount();
    int delta;
    View* child;

    if (childCount == 0)
        return;
    if (!mStackFromBottom) {
        // Uh-oh -- we came up short. Slide all views up to make them
        // align with the top
        child = getChildAt(0);
        delta = child->getTop() - mListPadding.top;
        if (mFirstPosition != 0) {
            // It's OK to have some space above the first item if it is
            // part of the vertical spacing
            delta -= mVerticalSpacing;
        }
        if (delta < 0) {
            // We only are looking to see if we are too low, not too high
            delta = 0;
        }
    } else {
        // we are too high, slide all views down to align with bottom
        child = getChildAt(childCount - 1);
        delta = child->getBottom() - (getHeight() - mListPadding.height);

        if (mFirstPosition + childCount < mItemCount) {
            // It's OK to have some space below the last item if it is
            // part of the vertical spacing
            delta += mVerticalSpacing;
        }
        if (delta > 0) {
            // We only are looking to see if we are too high, not too low
            delta = 0;
        }
    }
    if (delta != 0) {
        offsetChildrenTopAndBottom(-delta);
    }
}

int GridView::computeVerticalScrollExtent() {
    const int count = getChildCount();
    if (count == 0) return 0;
    const int rowCount = (count + mNumColumns - 1) / mNumColumns;
    int extent = rowCount * 100;
    const View* view = getChildAt(0);
    const int top = view->getTop();
    int height = view->getHeight();
    if (height > 0) {
        extent += (top * 100) / height;
    }

    view = getChildAt(count - 1);
    const int bottom = view->getBottom();
    height = view->getHeight();
    if (height > 0) {
        extent -= ((bottom - getHeight()) * 100) / height;
    }
    return extent;
}

int GridView::computeVerticalScrollOffset() {
    if (mFirstPosition >= 0 && getChildCount() > 0) {
        const View* view = getChildAt(0);
        const int top = view->getTop();
        const int height = view->getHeight();
        if (height > 0) {
            const int rowCount = (mItemCount + mNumColumns - 1) / mNumColumns;
            // In case of stackFromBottom the calculation of whichRow needs
            // to take into account that counting from the top the first row
            // might not be entirely filled.
            const int oddItemsOnFirstRow = isStackFromBottom() ? ((rowCount * mNumColumns) - mItemCount) : 0;
            const int whichRow = (mFirstPosition + oddItemsOnFirstRow) / mNumColumns;
            return std::max(whichRow * 100 - (top * 100) / height +
                    (int) ((float) mScrollY / getHeight() * rowCount * 100), 0);
        }
    }
    return 0;
}

int GridView::computeVerticalScrollRange() {
    // TODO: Account for vertical spacing too
    const int rowCount = (mItemCount + mNumColumns - 1) / mNumColumns;
    int result = std::max(rowCount * 100, 0);
    if (mScrollY != 0) {
        // Compensate for overscroll
        result += std::abs((int) ((float) mScrollY / getHeight() * rowCount * 100));
    }
    return result;
}

std::string GridView::getAccessibilityClassName()const{
    return "GridView";
}

void GridView::onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info){
     AbsListView::onInitializeAccessibilityNodeInfoInternal(info);

     const int columnsCount = getNumColumns();
     const int rowsCount = getCount() / columnsCount;
     const int selectionMode = getSelectionModeForAccessibility();
     AccessibilityNodeInfo::CollectionInfo* collectionInfo = AccessibilityNodeInfo::CollectionInfo::obtain(
             rowsCount, columnsCount, false, selectionMode);
     info.setCollectionInfo(collectionInfo);

     if (columnsCount > 0 || rowsCount > 0) {
         info.addAction(AccessibilityNodeInfo::AccessibilityAction::ACTION_SCROLL_TO_POSITION.getId());
     }
}

bool GridView::performAccessibilityActionInternal(int action, Bundle* arguments){
    if (AbsListView::performAccessibilityActionInternal(action, arguments)) {
         return true;
    }

    switch (action) {
        case R::id::accessibilityActionScrollToPosition: {
            // GridView only supports scrolling in one direction, so we can
            // ignore the column argument.
            const int numColumns = getNumColumns();
            const int row = 0;LOGD("TODO");//arguments.getInt(AccessibilityNodeInfo::ACTION_ARGUMENT_ROW_INT, -1);
            const int position = std::min(row * numColumns, getCount() - 1);
            if (row >= 0) {
                // The accessibility service gets data asynchronously, so
                // we'll be a little lenient by clamping the last position.
                smoothScrollToPosition(position);
                return true;
            }
        } break;
    }

    return false;
}

void GridView::onInitializeAccessibilityNodeInfoForItem(View* view, int position, AccessibilityNodeInfo&info){
    AbsListView::onInitializeAccessibilityNodeInfoForItem(view, position, info);

     const int count = getCount();
     const int columnsCount = getNumColumns();
     const int rowsCount = count / columnsCount;

     int row, column;
     if (!mStackFromBottom) {
         column = position % columnsCount;
         row = position / columnsCount;
     } else {
         const int invertedIndex = count - 1 - position;

         column = columnsCount - 1 - (invertedIndex % columnsCount);
         row = rowsCount - 1 - invertedIndex / columnsCount;
     }

     LayoutParams* lp = (LayoutParams*) view->getLayoutParams();
     bool isHeading = lp  && (lp->viewType == ITEM_VIEW_TYPE_HEADER_OR_FOOTER);
     bool isSelected = isItemChecked(position);
     AccessibilityNodeInfo::CollectionItemInfo* itemInfo = AccessibilityNodeInfo::CollectionItemInfo::obtain(
             row, 1, column, 1, isHeading, isSelected);
     info.setCollectionItemInfo(itemInfo);
}

}//namespace
