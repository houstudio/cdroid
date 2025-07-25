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
#include <widget/listview.h>
#include <widget/checkable.h>
#include <view/focusfinder.h>
#include <core/mathutils.h>
#include <widget/R.h>
#include <porting/cdlog.h>

namespace cdroid {

DECLARE_WIDGET2(ListView,"cdroid:attr/listViewStyle")

ListView::ListView(int w,int h):AbsListView(w,h) {
    const std::string style=LayoutInflater::from(mContext)->getDefaultStyle("ListView");
    AttributeSet attrs=mContext->obtainStyledAttributes(style);
    initListView(attrs);
}

ListView::ListView(Context* context,const AttributeSet& attrs)
    :AbsListView(context,attrs) {
    initListView(attrs);
}

void ListView::initListView(const AttributeSet&attrs) {
    mDividerHeight=0;
    mItemsCanFocus=false;
    mDivider=nullptr;
    mOverScrollHeader=nullptr;
    mOverScrollFooter=nullptr;
    mHeaderDividersEnabled=true;
    mFooterDividersEnabled=true;
    mFocusSelector =nullptr;
    mIsCacheColorOpaque =true;
    mDividerIsOpaque = true;

    Drawable* d = getContext()->getDrawable(attrs.getString("divider"));
    Drawable* osHeader = getContext()->getDrawable(attrs.getString("overScrollHeader"));
    Drawable* osFooter = getContext()->getDrawable(attrs.getString("overScrollFooter"));

    setOverscrollHeader(osHeader);
    setOverscrollHeader(osFooter);
    setDivider(d);
    mHeaderDividersEnabled = attrs.getBoolean("headerDividersEnabled",true);
    mFooterDividersEnabled = attrs.getBoolean("footerDividersEnabled", true);
    setDividerHeight(attrs.getDimensionPixelSize("dividerHeight",0));
}

ListView::~ListView() {
    delete mDivider;
    delete mOverScrollHeader;
    delete mOverScrollFooter;
    delete mFocusSelector;
    for_each(mHeaderViewInfos.begin(),mHeaderViewInfos.end(),[](const FixedViewInfo*f) {
        delete f;
    });
    for_each(mFooterViewInfos.begin(),mFooterViewInfos.end(),[](const FixedViewInfo*f) {
        delete f;
    });
    mHeaderViewInfos.clear();
    mFooterViewInfos.clear();
}

int ListView::getMaxScrollAmount()const {
    return (int) (MAX_SCROLL_FACTOR * (mBottom-mTop));
}

void ListView::adjustViewsUpOrDown() {
    int delta = 0;
    const int childCount = getChildCount();
    if (childCount == 0)return;

    if (!mStackFromBottom) {
        // Uh-oh -- we came up short. Slide all views up to make them align with the top
        const View* child = getChildAt(0);
        delta = child->getTop() - mListPadding.top;
        if (mFirstPosition != 0) {
            // It's OK to have some space above the first item if it is
            // part of the vertical spacing
            delta -= mDividerHeight;
        }
        if (delta < 0) {
            // We only are looking to see if we are too low, not too high
            delta = 0;
        }
    } else {
        // we are too high, slide all views down to align with bottom
        const View* child = getChildAt(childCount - 1);
        delta = child->getBottom() - (getHeight() - mListPadding.height);

        if (mFirstPosition + childCount < mItemCount) {
            // It's OK to have some space below the last item if it is
            // part of the vertical spacing
            delta += mDividerHeight;
        }
        if (delta > 0) delta = 0;
    }

    if (delta != 0) offsetChildrenTopAndBottom(-delta);
}

void ListView::addHeaderView(View* v,void* data, bool isSelectable) {
    if (v->getParent() != nullptr && v->getParent() != this) {
        FATAL("The specified child already has a parent.You must call removeView() on the child's parent first.");
    }
    FixedViewInfo* info=new FixedViewInfo();
    info->view = v;
    info->data = data;
    info->isSelectable = isSelectable;
    mHeaderViewInfos.push_back(info);
    mAreAllItemsSelectable &= isSelectable;

    // Wrap the adapter if it wasn't already wrapped.
    if (mAdapter != nullptr) {
        if (nullptr==dynamic_cast<HeaderViewListAdapter*>(mAdapter)) {
            wrapHeaderListAdapterInternal();
        }

        // In the case of re-adding a header view, or adding one later on,
        // we need to notify the observer.
        if (mDataSetObserver ) mDataSetObserver->onChanged();
    }
}

void ListView::addHeaderView(View* v) {
    addHeaderView(v, nullptr, true);
}

int ListView::getHeaderViewsCount()const {
    return (int)mHeaderViewInfos.size();
}

bool ListView::removeHeaderView(View* v) {
    if (mHeaderViewInfos.size()) {
        bool result = false;
        if (mAdapter != nullptr && ((HeaderViewListAdapter*) mAdapter)->removeHeader(v)) {
            if (mDataSetObserver != nullptr) {
                mDataSetObserver->onChanged();
            }
            result = true;
        }
        removeFixedViewInfo(v, mHeaderViewInfos);
        return result;
    }
    return false;
}

void ListView::removeFixedViewInfo(View* v, std::vector<FixedViewInfo*>& where) {
    const size_t len = where.size();
    for (size_t i = 0; i < len; ++i) {
        FixedViewInfo* info = where[i];
        if (info->view == v) {
            where.erase(where.begin()+i);
            break;
        }
    }
}

void ListView::addFooterView(View* v,void* data, bool isSelectable) {
    if (v->getParent() != nullptr && v->getParent() != this) {
        throw std::runtime_error("The specified child already has a parent.You must call removeView() on the child's parent first.");
    }

    FixedViewInfo* info = new FixedViewInfo();
    info->view = v;
    info->data = data;
    info->isSelectable = isSelectable;
    mFooterViewInfos.push_back(info);
    mAreAllItemsSelectable &= isSelectable;

    // Wrap the adapter if it wasn't already wrapped.
    if (mAdapter != nullptr) {
        if (nullptr==dynamic_cast<HeaderViewListAdapter*>(mAdapter)) {
            wrapHeaderListAdapterInternal();
        }

        // In the case of re-adding a footer view, or adding one later on,
        // we need to notify the observer.
        if (mDataSetObserver)mDataSetObserver->onChanged();
    }
}

void ListView::addFooterView(View* v) {
    addFooterView(v,nullptr,true);
}

int ListView::getFooterViewsCount()const {
    return (int)mFooterViewInfos.size();
}

bool ListView::removeFooterView(View* v) {
    if (mFooterViewInfos.size() > 0) {
        bool result = false;
        if (mAdapter && ((HeaderViewListAdapter*) mAdapter)->removeFooter(v)) {
            if (mDataSetObserver != nullptr) {
                mDataSetObserver->onChanged();
            }
            result = true;
        }
        removeFixedViewInfo(v, mFooterViewInfos);
        return result;
    }
    return false;
}

void ListView::setAdapter(Adapter* adapter) {
    if (mAdapter  && mDataSetObserver ) {
        mAdapter->unregisterDataSetObserver(mDataSetObserver);
        delete mDataSetObserver;
        mDataSetObserver = nullptr;
    }

    resetList();
    mRecycler->clear();
    if (mHeaderViewInfos.size() > 0|| mFooterViewInfos.size() > 0) {
        mAdapter = wrapHeaderListAdapterInternal(mHeaderViewInfos, mFooterViewInfos, adapter);
    } else {
        mAdapter = adapter;
    }

    mOldSelectedPosition = INVALID_POSITION;
    mOldSelectedRowId = INVALID_ROW_ID;

    // AbsListView#setAdapter will update choice mode states.
    AbsListView::setAdapter(adapter);

    if (mAdapter != nullptr) {
        mAreAllItemsSelectable = mAdapter->areAllItemsEnabled();
        mOldItemCount = mItemCount;
        mItemCount = mAdapter->getCount();
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

        if (mItemCount == 0) {
            // Nothing selected
            checkSelectionChanged();
        }
    } else {
        mAreAllItemsSelectable = true;
        checkFocus();
        // Nothing selected
        checkSelectionChanged();
    }
    requestLayout();
}

void ListView::resetList() {
    clearRecycledState(mHeaderViewInfos);
    clearRecycledState(mFooterViewInfos);
    AbsListView::resetList();
    mLayoutMode = LAYOUT_NORMAL;
}

void ListView::clearRecycledState(std::vector<FixedViewInfo*>& infos) {
    for (auto info:infos) { //int i = 0; i < count; i++) {
        View* child = info->view;
        ViewGroup::LayoutParams* params = child->getLayoutParams();
        if (checkLayoutParams(params)) {
            ((LayoutParams*) params)->recycledHeaderFooter = false;
        }
    }
}

bool ListView::showingTopFadingEdge() {
    const int listTop = mScrollY + mListPadding.top;
    return (mFirstPosition > 0) || (getChildAt(0)->getTop() > listTop);
}

bool ListView::showingBottomFadingEdge() {
    int childCount = getChildCount();
    int bottomOfBottomChild = getChildAt(childCount - 1)->getBottom();
    int lastVisiblePosition = mFirstPosition + childCount - 1;

    int listBottom = mScrollY + getHeight() - mListPadding.height;

    return (lastVisiblePosition < mItemCount - 1)|| (bottomOfBottomChild < listBottom);
}

bool ListView::requestChildRectangleOnScreen(View* child, Rect& rect, bool immediate) {
    int rectTopWithinChild = rect.top;

    // offset so rect is in coordinates of the this view
    rect.offset(child->getLeft(), child->getTop());
    rect.offset(-child->getScrollX(), -child->getScrollY());

    const int height = getHeight();
    int listUnfadedTop = getScrollY();
    int listUnfadedBottom = listUnfadedTop + height;
    const int fadingEdge = getVerticalFadingEdgeLength();

    if (showingTopFadingEdge()) {
        // leave room for top fading edge as long as rect isn't at very top
        if ((mSelectedPosition > 0) || (rectTopWithinChild > fadingEdge)) {
            listUnfadedTop += fadingEdge;
        }
    }

    int childCount = getChildCount();
    int bottomOfBottomChild = getChildAt(childCount - 1)->getBottom();

    if (showingBottomFadingEdge()) {
        // leave room for bottom fading edge as long as rect isn't at very bottom
        if ((mSelectedPosition < mItemCount - 1)
                || (rect.bottom() < (bottomOfBottomChild - fadingEdge))) {
            listUnfadedBottom -= fadingEdge;
        }
    }

    int scrollYDelta = 0;

    if (rect.bottom() > listUnfadedBottom && rect.top > listUnfadedTop) {
        // need to MOVE DOWN to get it in view: move down just enough so
        // that the entire rectangle is in view (or at least the first
        // screen size chunk).

        if (rect.height > height) {
            // just enough to get screen size chunk on
            scrollYDelta += (rect.top - listUnfadedTop);
        } else {
            // get entire rect at bottom of screen
            scrollYDelta += (rect.bottom() - listUnfadedBottom);
        }

        // make sure we aren't scrolling beyond the end of our children
        int distanceToBottom = bottomOfBottomChild - listUnfadedBottom;
        scrollYDelta = std::min(scrollYDelta, distanceToBottom);
    } else if (rect.top < listUnfadedTop && rect.bottom() < listUnfadedBottom) {
        // need to MOVE UP to get it in view: move up just enough so that
        // entire rectangle is in view (or at least the first screen
        // size chunk of it).

        if (rect.height > height) {
            // screen size chunk
            scrollYDelta -= (listUnfadedBottom - rect.bottom());
        } else {
            // entire rect at top
            scrollYDelta -= (listUnfadedTop - rect.top);
        }

        // make sure we aren't scrolling any further than the top our children
        const int top = getChildAt(0)->getTop();
        const int deltaToTop = top - listUnfadedTop;
        scrollYDelta = std::max(scrollYDelta, deltaToTop);
    }

    const bool scroll = scrollYDelta != 0;
    if (scroll) {
        scrollListItemsBy(-scrollYDelta);
        positionSelector(INVALID_POSITION, child);
        mSelectedTop = child->getTop();
        invalidate();
    }
    return scroll;
}

void ListView::fillGap(bool down) {
    const int count = getChildCount();
    if (down) {
        int paddingTop = 0;
        if ((mGroupFlags & CLIP_TO_PADDING_MASK) == CLIP_TO_PADDING_MASK) {
            paddingTop = getListPaddingTop();
        }
        const int startOffset = count > 0 ? getChildAt(count - 1)->getBottom() + mDividerHeight :paddingTop;
        fillDown(mFirstPosition + count, startOffset);
        correctTooHigh(getChildCount());
    } else {
        int paddingBottom = 0;
        if ((mGroupFlags & CLIP_TO_PADDING_MASK) == CLIP_TO_PADDING_MASK) {
            paddingBottom = getListPaddingBottom();
        }
        int startOffset = count > 0 ? getChildAt(0)->getTop() - mDividerHeight :getHeight() - paddingBottom;
        fillUp(mFirstPosition - 1, startOffset);
        correctTooLow(getChildCount());
    }
}

View* ListView::fillDown(int pos, int nextTop) {
    View* selectedView = nullptr;

    int listEnd = getHeight();
    if ((mGroupFlags & CLIP_TO_PADDING_MASK) == CLIP_TO_PADDING_MASK) {
        listEnd -= mListPadding.height;
    }

    while (nextTop < listEnd && pos < mItemCount) {
        // is this the selected item?
        bool selected = pos == mSelectedPosition;
        View* child = makeAndAddView(pos, nextTop, true, mListPadding.left, selected);
        nextTop = child->getBottom() + mDividerHeight;
        if (selected) {
            selectedView = child;
        }
        pos++;
    }

    setVisibleRangeHint(mFirstPosition, mFirstPosition + getChildCount() - 1);
    return selectedView;
}

View* ListView::fillUp(int pos, int nextBottom) {
    View* selectedView = nullptr;

    int end = 0;
    if ((mGroupFlags & CLIP_TO_PADDING_MASK) == CLIP_TO_PADDING_MASK) {
        end = mListPadding.top;
    }
    while (nextBottom > end && pos >= 0) {
        // is this the selected item?
        bool selected = pos == mSelectedPosition;
        View* child = makeAndAddView(pos, nextBottom, false, mListPadding.left, selected);
        nextBottom = child->getTop() - mDividerHeight;
        if (selected) {
            selectedView = child;
        }
        pos--;
    }

    mFirstPosition = pos + 1;
    setVisibleRangeHint(mFirstPosition, mFirstPosition + getChildCount() - 1);
    return selectedView;
}

View* ListView::fillFromTop(int nextTop) {
    mFirstPosition = std::min(mFirstPosition, mSelectedPosition);
    mFirstPosition = std::min(mFirstPosition, mItemCount - 1);
    if (mFirstPosition < 0) {
        mFirstPosition = 0;
    }
    return fillDown(mFirstPosition, nextTop);
}

View* ListView::fillFromMiddle(int childrenTop, int childrenBottom) {
    const int height = childrenBottom - childrenTop;

    const int position = reconcileSelectedPosition();

    View* sel = makeAndAddView(position, childrenTop, true,mListPadding.left, true);
    mFirstPosition = position;

    int selHeight = sel->getMeasuredHeight();
    if (selHeight <= height) {
        sel->offsetTopAndBottom((height - selHeight) / 2);
    }

    fillAboveAndBelow(sel, position);

    if (!mStackFromBottom) {
        correctTooHigh(getChildCount());
    } else {
        correctTooLow(getChildCount());
    }
    return sel;
}

void ListView::fillAboveAndBelow(View* sel, int position) {
    if (!mStackFromBottom) {
        fillUp(position - 1, sel->getTop() - mDividerHeight);
        adjustViewsUpOrDown();
        fillDown(position + 1, sel->getBottom() + mDividerHeight);
    } else {
        fillDown(position + 1, sel->getBottom() + mDividerHeight);
        adjustViewsUpOrDown();
        fillUp(position - 1, sel->getTop() - mDividerHeight);
    }
}

View* ListView::fillFromSelection(int selectedTop, int childrenTop, int childrenBottom) {
    const int fadingEdgeLength = getVerticalFadingEdgeLength();
    int selectedPosition = mSelectedPosition;

    int topSelectionPixel = getTopSelectionPixel(childrenTop, fadingEdgeLength,selectedPosition);
    int bottomSelectionPixel = getBottomSelectionPixel(childrenBottom, fadingEdgeLength,selectedPosition);

    View* sel = makeAndAddView(selectedPosition, selectedTop, true, mListPadding.left, true);


    // Some of the newly selected item extends below the bottom of the list
    if (sel->getBottom() > bottomSelectionPixel) {
        // Find space available above the selection into which we can scroll upwards
        int spaceAbove = sel->getTop() - topSelectionPixel;

        // Find space required to bring the bottom of the selected item
        // fully into view
        int spaceBelow = sel->getBottom() - bottomSelectionPixel;
        int offset = std::min(spaceAbove, spaceBelow);

        // Now offset the selected item to get it into view
        sel->offsetTopAndBottom(-offset);
    } else if (sel->getTop() < topSelectionPixel) {
        // Find space required to bring the top of the selected item fully
        // into view
        int spaceAbove = topSelectionPixel - sel->getTop();

        // Find space available below the selection into which we can scroll
        // downwards
        int spaceBelow = bottomSelectionPixel - sel->getBottom();
        int offset = std::min(spaceAbove, spaceBelow);

        // Offset the selected item to get it into view
        sel->offsetTopAndBottom(offset);
    }

    // Fill in views above and below
    fillAboveAndBelow(sel, selectedPosition);

    if (!mStackFromBottom) {
        correctTooHigh(getChildCount());
    } else {
        correctTooLow(getChildCount());
    }
    return sel;
}

int ListView::getBottomSelectionPixel(int childrenBottom, int fadingEdgeLength,int selectedPosition) {
    int bottomSelectionPixel = childrenBottom;
    if (selectedPosition != mItemCount - 1) {
        bottomSelectionPixel -= fadingEdgeLength;
    }
    return bottomSelectionPixel;
}

int ListView::getTopSelectionPixel(int childrenTop, int fadingEdgeLength, int selectedPosition) {
    int topSelectionPixel = childrenTop;
    if (selectedPosition > 0) {
        topSelectionPixel += fadingEdgeLength;
    }
    return topSelectionPixel;
}

View* ListView::moveSelection(View* oldSel, View* newSel, int delta, int childrenTop, int childrenBottom){
    const int fadingEdgeLength = getVerticalFadingEdgeLength();
    int selectedPosition = mSelectedPosition;

    View* sel;

    int topSelectionPixel = getTopSelectionPixel(childrenTop, fadingEdgeLength, selectedPosition);
    int bottomSelectionPixel = getBottomSelectionPixel(childrenTop, fadingEdgeLength,  selectedPosition);
    if (delta > 0) {//Scrolling Down.
        /*
         * Case 1: Scrolling down.
         *     Before           After
         *    |       |        |       |
         *    +-------+        +-------+
         *    |   A   |        |   A   |
         *    |   1   |   =>   +-------+
         *    +-------+        |   B   |
         *    |   B   |        |   2   |
         *    +-------+        +-------+
         *    |       |        |       |
         *
         *    Try to keep the top of the previously selected item where it was.
         *    oldSel = A
         *    sel = B
         */
        // Put oldSel (A) where it belongs
        oldSel = makeAndAddView(selectedPosition - 1, oldSel->getTop(), true, mListPadding.left, false);

        int dividerHeight = mDividerHeight;
        // Now put the new selection (B) below that
        sel = makeAndAddView(selectedPosition, oldSel->getBottom() + dividerHeight, true,  mListPadding.left, true);

        // Some of the newly selected item extends below the bottom of the list
        if (sel->getBottom() > bottomSelectionPixel) {

            // Find space available above the selection into which we can scroll upwards
            int spaceAbove = sel->getTop() - topSelectionPixel;

            // Find space required to bring the bottom of the selected item fully into view
            int spaceBelow = sel->getBottom() - bottomSelectionPixel;

            // Don't scroll more than half the height of the list
            int halfVerticalSpace = (childrenBottom - childrenTop) / 2;
            int offset = std::min(spaceAbove, spaceBelow);
            offset = std::min(offset, halfVerticalSpace);

            // We placed oldSel, so offset that item
            oldSel->offsetTopAndBottom(-offset);
            // Now offset the selected item to get it into view
            sel->offsetTopAndBottom(-offset);
        }

        // Fill in views above and below
        if (!mStackFromBottom) {
            fillUp(mSelectedPosition - 2, sel->getTop() - dividerHeight);
            adjustViewsUpOrDown();
            fillDown(mSelectedPosition + 1, sel->getBottom() + dividerHeight);
        } else {
            fillDown(mSelectedPosition + 1, sel->getBottom() + dividerHeight);
            adjustViewsUpOrDown();
            fillUp(mSelectedPosition - 2, sel->getTop() - dividerHeight);
        }
    } else if (delta < 0) {//Scrolling up.
        /*
        * Case 2: Scrolling up.
        *     Before           After
        *    |       |        |       |
        *    +-------+        +-------+
        *    |   A   |        |   A   |
        *    +-------+   =>   |   1   |
        *    |   B   |        +-------+
        *    |   2   |        |   B   |
        *    +-------+        +-------+
        *    |       |        |       |
        *
        *    Try to keep the top of the item about to become selected where it was.
        *    newSel = A
        *    olSel = B
        */

        if (newSel != nullptr) {
            // Try to position the top of newSel (A) where it was before it was selected
            sel = makeAndAddView(selectedPosition, newSel->getTop(), true, mListPadding.left,true);
        } else {
            // If (A) was not on screen and so did not have a view, position
            // it above the oldSel (B)
            sel = makeAndAddView(selectedPosition, oldSel->getTop(), false, mListPadding.left,true);
        }

        // Some of the newly selected item extends above the top of the list
        if (sel->getTop() < topSelectionPixel) {
            // Find space required to bring the top of the selected item fully into view
            int spaceAbove = topSelectionPixel - sel->getTop();

           // Find space available below the selection into which we can scroll downwards
            int spaceBelow = bottomSelectionPixel - sel->getBottom();

            // Don't scroll more than half the height of the list
            int halfVerticalSpace = (childrenBottom - childrenTop) / 2;
            int offset = std::min(spaceAbove, spaceBelow);
            offset = std::min(offset, halfVerticalSpace);

            // Offset the selected item to get it into view
            sel->offsetTopAndBottom(offset);
        }

        // Fill in views above and below
        fillAboveAndBelow(sel, selectedPosition);
    } else {

        int oldTop = oldSel->getTop();

        // Case 3: Staying still
        sel = makeAndAddView(selectedPosition, oldTop, true, mListPadding.left, true);

        // We're staying still...
        if (oldTop < childrenTop) {
            // ... but the top of the old selection was off screen.
            // (This can happen if the data changes size out from under us)
            int newBottom = sel->getBottom();
            if (newBottom < childrenTop + 20) {
                // Not enough visible -- bring it onscreen
                sel->offsetTopAndBottom(childrenTop - sel->getTop());
            }
        }

        // Fill in views above and below
        fillAboveAndBelow(sel, selectedPosition);
    }

    return sel;
}

ListView::FocusSelector::FocusSelector(ListView*lv) {
    mLV=lv;
}

ListView::FocusSelector&ListView::FocusSelector::setupForSetSelection(int position, int top) {
    mPosition = position;
    mPositionTop = top;
    mAction = STATE_SET_SELECTION;
    return *this;
}

void ListView::FocusSelector::operator()() {
    if (mAction == STATE_SET_SELECTION) {
        mLV->setSelectionFromTop(mPosition, mPositionTop);
        mAction = STATE_WAIT_FOR_LAYOUT;
    } else if (mAction == STATE_REQUEST_FOCUS) {
        int childIndex = mPosition - mLV->mFirstPosition;
        View* child = mLV->getChildAt(childIndex);
        if (child) child->requestFocus();
        mAction = -1;
    }
}

bool ListView::FocusSelector::setupFocusIfValid(int position) {
    if (mAction != STATE_WAIT_FOR_LAYOUT || position != mPosition) {
        return false;
    }
    mAction = STATE_REQUEST_FOCUS;
    return true;
}

void ListView::FocusSelector::onLayoutComplete() {
    if (mAction == STATE_WAIT_FOR_LAYOUT) {
        mAction = -1;
    }
}

void ListView::onDetachedFromWindow() {
    if (mFocusSelector) {
        removeCallbacks(*mFocusSelector);
        delete mFocusSelector;
        mFocusSelector = nullptr;
    }
    AbsListView::onDetachedFromWindow();
}

void ListView::onSizeChanged(int w, int h, int oldw, int oldh) {
    if (getChildCount() > 0) {
        View* focusedChild = getFocusedChild();
        if (focusedChild) {
            int childPosition = mFirstPosition + indexOfChild(focusedChild);
            int childBottom = focusedChild->getBottom();
            int offset = std::max(0, childBottom - (h - mPaddingTop));
            int top = focusedChild->getTop() - offset;

            if (mFocusSelector == nullptr)  mFocusSelector = new FocusSelector(this);

            post(mFocusSelector->setupForSetSelection(childPosition, top));
        }
    }
    AbsListView::onSizeChanged(w, h, oldw, oldh);
}

void ListView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    AbsListView::onMeasure(widthMeasureSpec, heightMeasureSpec);

    int widthMode = MeasureSpec::getMode(widthMeasureSpec);
    int heightMode= MeasureSpec::getMode(heightMeasureSpec);
    int widthSize = MeasureSpec::getSize(widthMeasureSpec);
    int heightSize= MeasureSpec::getSize(heightMeasureSpec);

    int childWidth = 0;
    int childHeight = 0;
    int childState = 0;

    mItemCount = mAdapter ?mAdapter->getCount():0;
    if (mItemCount > 0 && (widthMode == MeasureSpec::UNSPECIFIED
                           || heightMode == MeasureSpec::UNSPECIFIED)) {
        View* child = obtainView(0, mIsScrap);

        // Lay out child directly against the parent measure spec so that
        // we can obtain exected minimum width and height.
        measureScrapChild(child, 0, widthMeasureSpec, heightSize);

        childWidth = child->getMeasuredWidth();
        childHeight = child->getMeasuredHeight();
        childState = combineMeasuredStates(childState, child->getMeasuredState());

        if (recycleOnMeasure() && mRecycler->shouldRecycleViewType(
                    ((LayoutParams*) child->getLayoutParams())->viewType)) {
            mRecycler->addScrapView(child, 0);
        }
    }

    if (widthMode == MeasureSpec::UNSPECIFIED) {
        widthSize = mListPadding.left + mListPadding.width + childWidth + getVerticalScrollbarWidth();
    } else {
        widthSize |= (childState & MEASURED_STATE_MASK);
    }

    if (heightMode == MeasureSpec::UNSPECIFIED) {
        heightSize = mListPadding.top + mListPadding.height + childHeight +
                       getVerticalFadingEdgeLength() * 2;
    }

    if (heightMode == MeasureSpec::AT_MOST) {
        // TODO: after first layout we should maybe start at the first visible position, not 0
        heightSize = measureHeightOfChildren(widthMeasureSpec, 0, NO_POSITION, heightSize, -1);
    }

    setMeasuredDimension(widthSize, heightSize);

    mWidthMeasureSpec = widthMeasureSpec;
}

void ListView::measureScrapChild(View* child, int position, int widthMeasureSpec, int heightHint) {
    LayoutParams* p = (LayoutParams*) child->getLayoutParams();
    if (p == nullptr) {
        p = (AbsListView::LayoutParams*) generateDefaultLayoutParams();
        child->setLayoutParams(p);
    }
    p->viewType = mAdapter->getItemViewType(position);
    p->isEnabled = mAdapter->isEnabled(position);
    p->forceAdd = true;
    int  childWidthSpec,childHeightSpec;
    childWidthSpec = getChildMeasureSpec(widthMeasureSpec,
                         mListPadding.left + mListPadding.width, p->width);
    const int lpHeight = p->height;
    if (lpHeight > 0) {
        childHeightSpec = MeasureSpec::makeMeasureSpec(lpHeight, MeasureSpec::EXACTLY);
    } else {
        childHeightSpec = MeasureSpec::makeSafeMeasureSpec(heightHint, MeasureSpec::UNSPECIFIED);
    }
    child->measure(childWidthSpec, childHeightSpec);

    // Since this view was measured directly aginst the parent measure
    // spec, we must measure it again before reuse.
    child->forceLayout();
}

bool ListView::recycleOnMeasure() {
    return true;
}

int ListView::measureHeightOfChildren(int widthMeasureSpec, int startPosition, int endPosition,
                                      int maxHeight, int disallowPartialChildPosition) {
    if (mAdapter == nullptr) {
        return mListPadding.top + mListPadding.height;
    }

    // Include the padding of the list
    int returnedHeight = mListPadding.top + mListPadding.height;
    const int dividerHeight = mDividerHeight;
    // The previous height value that was less than maxHeight and contained
    // no partial children
    int i,prevHeightWithoutPartialChild = 0;
    View* child;

    // mItemCount - 1 since endPosition parameter is inclusive
    endPosition = (endPosition == NO_POSITION) ? mAdapter->getCount() - 1 : endPosition;
    bool recyle = recycleOnMeasure();
    bool* isScrap = mIsScrap;

    for (i = startPosition; i <= endPosition; ++i) {
        child = obtainView(i, isScrap);

        measureScrapChild(child, i, widthMeasureSpec, maxHeight);

        if (i > 0) {
            // Count the divider for all but one child
            returnedHeight += dividerHeight;
        }

        // Recycle the view before we possibly return from the method
        if (recyle && mRecycler->shouldRecycleViewType(
                    ((LayoutParams*) child->getLayoutParams())->viewType)) {
            mRecycler->addScrapView(child, -1);
        }

        returnedHeight += child->getMeasuredHeight();

        if (returnedHeight >= maxHeight) {
            // We went over, figure out which height to return.  If returnedHeight > maxHeight,
            // then the i'th position did not fit completely.
            LOGV("returnedHeight=%d",returnedHeight);
            return (disallowPartialChildPosition >= 0) // Disallowing is enabled (> -1)
                   && (i > disallowPartialChildPosition) // We've past the min pos
                   && (prevHeightWithoutPartialChild > 0) // We have a prev height
                   && (returnedHeight != maxHeight) // i'th child did not fit completely
                   ? prevHeightWithoutPartialChild : maxHeight;
        }

        if ((disallowPartialChildPosition >= 0) && (i >= disallowPartialChildPosition)) {
            prevHeightWithoutPartialChild = returnedHeight;
        }
    }
    LOGV("returnedHeight=%d",returnedHeight);
    // At this point, we went through the range of children, and they each
    // completely fit, so return the returnedHeight
    return returnedHeight;
}

int ListView::findMotionRow(int y) {
    const int childCount = getChildCount();
    if (childCount > 0) {
        if (!mStackFromBottom) {
            for (int i = 0; i < childCount; i++) {
                View* v = getChildAt(i);
                if (y <= v->getBottom()) {
                    return mFirstPosition + i;
                }
            }
        } else {
            for (int i = childCount - 1; i >= 0; i--) {
                View* v = getChildAt(i);
                if (y >= v->getTop()) {
                    return mFirstPosition + i;
                }
            }
        }
    }
    return INVALID_POSITION;
}

View* ListView::fillSpecific(int position, int top) {
    bool tempIsSelected = position == mSelectedPosition;
    View* temp = makeAndAddView(position, top, true, mListPadding.left, tempIsSelected);
    // Possibly changed again in fillUp if we add rows above this one.
    mFirstPosition = position;
    View* above,* below;

    const int dividerHeight = mDividerHeight;
    if (!mStackFromBottom) {
        above = fillUp(position - 1, temp->getTop() - dividerHeight);
        // This will correct for the top of the first view not touching the top of the list
        adjustViewsUpOrDown();
        below = fillDown(position + 1, temp->getBottom() + dividerHeight);
        int childCount = getChildCount();
        if (childCount > 0) {
            correctTooHigh(childCount);
        }
    } else {
        below = fillDown(position + 1, temp->getBottom() + dividerHeight);
        // This will correct for the bottom of the last view not touching the bottom of the list
        adjustViewsUpOrDown();
        above = fillUp(position - 1, temp->getTop() - dividerHeight);
        int childCount = getChildCount();
        if (childCount > 0) {
            correctTooLow(childCount);
        }
    }

    if (tempIsSelected) {
        return temp;
    } else if (above != nullptr) {
        return above;
    } else {
        return below;
    }
}

void ListView::correctTooHigh(int childCount) {
    const int lastPosition = mFirstPosition + childCount - 1;
    if (lastPosition == mItemCount - 1 && childCount > 0) {
        // Get the last child ...
        const View* last = getChildAt(childCount - 1);
        // ... and its bottom edge
        const int lastBottom = last->getBottom();
        // This is bottom of our drawable area
        const int listEnd = getHeight()- mListPadding.height;
        // This is how far the bottom edge of the last view is from the bottom of the
        // drawable area
        int bottomOffset = listEnd - lastBottom;
        const View* first = getChildAt(0);
        const int firstTop = first->getTop();

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
                // Fill the gap that was opened above mFirstPosition with more rows, if
                // possible
                fillUp(mFirstPosition - 1, first->getTop() - mDividerHeight);
                // Close up the remaining gap
                adjustViewsUpOrDown();
            }

        }
    }
}

void ListView::correctTooLow(int childCount) {
    if (mFirstPosition != 0 || childCount == 0) return;
    // Get the first child ...
    const View* first = getChildAt(0);
    // ... and its top edge
    const int firstTop = first->getTop();
    // This is top of our drawable area
    const int listStart = mListPadding.top;
    // This is bottom of our drawable area
    const int listEnd = getHeight()- mListPadding.height;
    // This is how far the top edge of the first view is from the top of the
    // drawable area
    int topOffset = firstTop - listStart;
    const View* last = getChildAt(childCount - 1);
    const int lastBottom = last->getBottom();
    const int lastPosition = mFirstPosition + childCount - 1;

    // Make sure we are 1) Too low, and 2) Either there are more rows below the
    // last row or the last row is scrolled off the bottom of the drawable area
    if (topOffset > 0) {
        if (lastPosition < mItemCount - 1 || lastBottom > listEnd)  {
            if (lastPosition == mItemCount - 1) {
                // Don't pull the bottom too far up
                topOffset = std::min(topOffset, lastBottom - listEnd);
            }
            // Move everything up
            offsetChildrenTopAndBottom(-topOffset);
            if (lastPosition < mItemCount - 1) {
                // Fill the gap that was opened below the last position with more rows, if
                // possible
                fillDown(lastPosition + 1, last->getBottom() + mDividerHeight);
                // Close up the remaining gap
                adjustViewsUpOrDown();
            }
        } else if (lastPosition == mItemCount - 1) {
            adjustViewsUpOrDown();
        }
    }
}

void ListView::layoutChildren() {
    const bool blockLayoutRequests = mBlockLayoutRequests;
    if(mBlockLayoutRequests)return;

    mBlockLayoutRequests = true;

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

    int index = 0;
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
    default:
        // Remember the previously selected view
        index = mSelectedPosition - mFirstPosition;
        if (index >= 0 && index < childCount) {
            oldSel = getChildAt(index);
        }

        // Remember the previous first child
        oldFirst = getChildAt(0);

        if (mNextSelectedPosition >= 0) {
            delta = mNextSelectedPosition - mSelectedPosition;
        }

        // Caution: newSel might be null
        newSel = getChildAt(index + delta);
    }


    bool dataChanged = mDataChanged;
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
    } else if (mItemCount != mAdapter->getCount()) {
        LOGE("The content of the adapter has changed but "
             "ListView did not receive a notification. Make sure the content of "
             "your adapter is not modified from a background thread, but only from "
             "the UI thread. Make sure your adapter calls notifyDataSetChanged() "
             "when its content changes. [in ListView( %p) with Adapter( %p )]",getId(),mAdapter);
    }

    setSelectedPositionInt(mNextSelectedPosition);

    //AccessibilityNodeInfo accessibilityFocusLayoutRestoreNode = null;
    View* accessibilityFocusLayoutRestoreView = nullptr;
    int accessibilityFocusPosition = INVALID_POSITION;

    // Remember which child, if any, had accessibility focus. This must
    // occur before recycling any views, since that will clear
    // accessibility focus.
    /*ViewRootImpl viewRootImpl = getViewRootImpl();
    if (viewRootImpl != nullptr) {
        View focusHost = viewRootImpl.getAccessibilityFocusedHost();
        if (focusHost != nullptr) {
            View* focusChild = getAccessibilityFocusedChild(focusHost);
            if (focusChild != nullptr) {
                if (!dataChanged || isDirectChildHeaderOrFooter(focusChild)
                        || (focusChild.hasTransientState() && mAdapterHasStableIds)) {
                    // The views won't be changing, so try to maintain
                    // focus on the current host and virtual view.
                    accessibilityFocusLayoutRestoreView = focusHost;
                    accessibilityFocusLayoutRestoreNode = viewRootImpl
                                                          .getAccessibilityFocusedVirtualView();
                }

                // If all else fails, maintain focus at the same
                // position.
                accessibilityFocusPosition = getPositionForView(focusChild);
            }
        }
    }*/

    View* focusLayoutRestoreDirectChild = nullptr;
    View* focusLayoutRestoreView = nullptr;

    // Take focus back to us temporarily to avoid the eventual call to
    // clear focus when removing the focused child below from messing
    // things up when ViewAncestor assigns focus back to someone else.
    View* focusedChild = getFocusedChild();
    if (focusedChild != nullptr) {
        // TODO: in some cases focusedChild.getParent() == null

        // We can remember the focused view to restore after re-layout
        // if the data hasn't changed, or if the focused position is a
        // header or footer.
        if (!dataChanged || isDirectChildHeaderOrFooter(focusedChild)
                || focusedChild->hasTransientState() || mAdapterHasStableIds) {
            focusLayoutRestoreDirectChild = focusedChild;
            // Remember the specific view that had focus.
            focusLayoutRestoreView = findFocus();
            if (focusLayoutRestoreView != nullptr) {
                // Tell it we are going to mess with it.
                focusLayoutRestoreView->dispatchStartTemporaryDetach();
            }
        }
        requestFocus();
    }

    // Pull all children into the RecycleBin.
    // These views will be reused if possible
    const int firstPosition = mFirstPosition;
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
        if (newSel != nullptr) {
            sel = fillFromSelection(newSel->getTop(), childrenTop, childrenBottom);
        } else {
            sel = fillFromMiddle(childrenTop, childrenBottom);
        }
        break;
    case LAYOUT_SYNC:
        sel = fillSpecific(mSyncPosition, mSpecificTop);
        break;
    case LAYOUT_FORCE_BOTTOM:
        sel = fillUp(mItemCount - 1, childrenBottom);
        adjustViewsUpOrDown();
        break;
    case LAYOUT_FORCE_TOP:
        mFirstPosition = 0;
        sel = fillFromTop(childrenTop);
        adjustViewsUpOrDown();
        break;
    case LAYOUT_SPECIFIC: {
        int selectedPosition = reconcileSelectedPosition();
        sel = fillSpecific(selectedPosition, mSpecificTop);
        /**
         * When ListView is resized, FocusSelector requests an async selection for the
         * previously focused item to make sure it is still visible. If the item is not
         * selectable, it won't regain focus so instead we call FocusSelector
         * to directly request focus on the view after it is visible.
         */
        if (sel == nullptr && mFocusSelector != nullptr) {
            if(mFocusSelector->setupFocusIfValid(selectedPosition))
                post(*mFocusSelector);
        }
    }
    break;
    case LAYOUT_MOVE_SELECTION:
        sel = moveSelection(oldSel, newSel, delta, childrenTop, childrenBottom);
        break;
    default:
        if (childCount == 0) {
            if (!mStackFromBottom) {
                int position = lookForSelectablePosition(0, true);
                setSelectedPositionInt(position);
                sel = fillFromTop(childrenTop);
            } else {
                int position = lookForSelectablePosition(mItemCount - 1, false);
                setSelectedPositionInt(position);
                sel = fillUp(mItemCount - 1, childrenBottom);
            }
        } else {
            if (mSelectedPosition >= 0 && mSelectedPosition < mItemCount) {
                sel = fillSpecific(mSelectedPosition,
                                   oldSel == nullptr ? childrenTop : oldSel->getTop());
            } else if (mFirstPosition < mItemCount) {
                sel = fillSpecific(mFirstPosition,
                                   oldFirst == nullptr ? childrenTop : oldFirst->getTop());
            } else {
                sel = fillSpecific(0, childrenTop);
            }
        }
        break;
    }

    // Flush any cached views that did not get reused above
    mRecycler->scrapActiveViews();

    // remove any header/footer that has been temp detached and not re-attached
    removeUnusedFixedViews(mHeaderViewInfos);
    removeUnusedFixedViews(mFooterViewInfos);

    if (sel != nullptr) {
        // The current selected item should get focus if items are
        // focusable.
        if (mItemsCanFocus && hasFocus() && !sel->hasFocus()) {
            const bool focusWasTaken = (sel == focusLayoutRestoreDirectChild &&
                                        focusLayoutRestoreView != nullptr &&
                                        focusLayoutRestoreView->requestFocus()) || sel->requestFocus();
            if (!focusWasTaken) {
                // Selected item didn't take focus, but we still want to
                // make sure something else outside of the selected view
                // has focus.
                View* focused = getFocusedChild();
                if (focused != nullptr) {
                    focused->clearFocus();
                }
                positionSelector(INVALID_POSITION, sel);
            } else {
                sel->setSelected(false);
                mSelectorRect.setEmpty();
            }
        } else {
            positionSelector(INVALID_POSITION, sel);
        }
        mSelectedTop = sel->getTop();
    } else {
        const bool inTouchMode = mTouchMode == TOUCH_MODE_TAP || mTouchMode == TOUCH_MODE_DONE_WAITING;
        if (inTouchMode) {
            // If the user's finger is down, select the motion position.
            View* child = getChildAt(mMotionPosition - mFirstPosition);
            if (child != nullptr) {
                positionSelector(mMotionPosition, child);
            }
        } else if (mSelectorPosition != INVALID_POSITION) {
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

        // Even if there is not selected position, we may need to
        // restore focus (i.e. something focusable in touch mode).
        if (hasFocus() && focusLayoutRestoreView != nullptr) {
            focusLayoutRestoreView->requestFocus();
        }
    }

    // Attempt to restore accessibility focus, if necessary.
    /*if (viewRootImpl != nullptr) {
        View* newAccessibilityFocusedView = viewRootImpl.getAccessibilityFocusedHost();
        if (newAccessibilityFocusedView == nullptr) {
            if (accessibilityFocusLayoutRestoreView != nullptr
                    && accessibilityFocusLayoutRestoreView.isAttachedToWindow()) {
                AccessibilityNodeProvider provider =
                    accessibilityFocusLayoutRestoreView.getAccessibilityNodeProvider();
                if (accessibilityFocusLayoutRestoreNode != nullptr && provider != nullptr) {
                    int virtualViewId = AccessibilityNodeInfo.getVirtualDescendantId(
                                                  accessibilityFocusLayoutRestoreNode.getSourceNodeId());
                    provider.performAction(virtualViewId,
                                           AccessibilityNodeInfo.ACTION_ACCESSIBILITY_FOCUS, nullptr);
                } else {
                    accessibilityFocusLayoutRestoreView.requestAccessibilityFocus();
                }
            } else if (accessibilityFocusPosition != INVALID_POSITION) {
                // Bound the position within the visible children.
                int position = MathUtils::constrain(
                                         accessibilityFocusPosition - mFirstPosition, 0,
                                         getChildCount() - 1);
                View* restoreView = getChildAt(position);
                if (restoreView != nullptr) {
                    restoreView->requestAccessibilityFocus();
                }
            }
        }
    }*/

    // Tell focus view we are done mucking with it, if it is still in
    // our view hierarchy.
    if (focusLayoutRestoreView != nullptr/*&& focusLayoutRestoreView.getWindowToken() != nullptr*/) {
        focusLayoutRestoreView->dispatchFinishTemporaryDetach();
    }

    mLayoutMode = LAYOUT_NORMAL;
    mDataChanged = false;
    if (mPositionScrollAfterLayout != nullptr) {
        post(mPositionScrollAfterLayout);
        mPositionScrollAfterLayout = nullptr;
    }
    mNeedSync = false;
    setNextSelectedPositionInt(mSelectedPosition);

    updateScrollIndicators();

    if (mItemCount > 0)checkSelectionChanged();

    invokeOnItemScrollListener();
    if (mFocusSelector) mFocusSelector->onLayoutComplete();
    if (!blockLayoutRequests) mBlockLayoutRequests =false;
}

bool ListView::trackMotionScroll(int deltaY, int incrementalDeltaY) {
    const bool result = AbsListView::trackMotionScroll(deltaY, incrementalDeltaY);
    removeUnusedFixedViews(mHeaderViewInfos);
    removeUnusedFixedViews(mFooterViewInfos);
    return result;
}

void ListView::removeUnusedFixedViews(std::vector<FixedViewInfo*>& infoList) {
    for (int i = int(infoList.size() - 1); i >= 0; i--) {
        FixedViewInfo* fixedViewInfo = infoList[i];
        View* view = fixedViewInfo->view;
        LayoutParams* lp = (LayoutParams*) view->getLayoutParams();
        if (view->getParent() == nullptr && lp != nullptr && lp->recycledHeaderFooter) {
            removeDetachedView(view, false);
            lp->recycledHeaderFooter = false;
        }
    }
}

bool ListView::isDirectChildHeaderOrFooter(View* child) {
    for (auto h:mHeaderViewInfos) {
        if (child == h->view) {
            return true;
        }
    }
    for (auto f:mFooterViewInfos) {
        if (child == f->view) {
            return true;
        }
    }
    return false;
}

View* ListView::makeAndAddView(int position, int y, bool flow, int childrenLeft,bool selected) {
    if (!mDataChanged) {
        // Try to use an existing view for this position.
        View* activeView = mRecycler->getActiveView(position);
        if (activeView != nullptr) {
            // Found it. We're reusing an existing child, so it just needs
            // to be positioned like a scrap view.
            setupChild(activeView, position, y, flow, childrenLeft, selected, true);
            return activeView;
        }
    }

    // Make a new view for this position, or convert an unused view if
    // possible.
    View* child = obtainView(position, mIsScrap);

    // This needs to be positioned and measured.
    setupChild(child, position, y, flow, childrenLeft, selected, mIsScrap[0]);
    return child;
}

void ListView::setupChild(View* child, int position, int y, bool flowDown, int childrenLeft,
                          bool selected, bool isAttachedToWindow) {

    const bool isSelected = selected && shouldShowSelector();
    const bool updateChildSelected = isSelected != child->isSelected();
    const bool isPressed = mTouchMode > TOUCH_MODE_DOWN && mTouchMode < TOUCH_MODE_SCROLL && mMotionPosition == position;
    const bool updateChildPressed = isPressed != child->isPressed();
    const bool needToMeasure = !isAttachedToWindow || updateChildSelected || child->isLayoutRequested();

    // Respect layout params that are already in the view. Otherwise make
    // some up...
    AbsListView::LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
    if (lp == nullptr) lp = (LayoutParams*) generateDefaultLayoutParams();

    lp->viewType = mAdapter->getItemViewType(position);
    lp->isEnabled = mAdapter->isEnabled(position);

    // Set up view state before attaching the view, since we may need to
    // rely on the jumpDrawablesToCurrentState() call that occurs as part
    // of view attachment.
    if (updateChildSelected)child->setSelected(isSelected);
    if (updateChildPressed) child->setPressed(isPressed);

    if ((mChoiceMode != CHOICE_MODE_NONE) && (mCheckStates!=nullptr)) {
        if (dynamic_cast<Checkable*>(child)) {
            ((Checkable*)child)->setChecked(mCheckStates->get(position,false));
        } else {
            child->setActivated(mCheckStates->get(position,false));
        }
    }

    if ((isAttachedToWindow && !lp->forceAdd) || (lp->recycledHeaderFooter
            && lp->viewType == AdapterView::ITEM_VIEW_TYPE_HEADER_OR_FOOTER)) {
        attachViewToParent(child, flowDown ? -1 : 0, lp);
        // If the view was previously attached for a different position,
        // then manually jump the drawables.
        if (isAttachedToWindow
                && (((AbsListView::LayoutParams*) child->getLayoutParams())->scrappedFromPosition)
                != position) {
            child->jumpDrawablesToCurrentState();
        }
    } else {
        lp->forceAdd = false;
        if (lp->viewType == AdapterView::ITEM_VIEW_TYPE_HEADER_OR_FOOTER) {
            lp->recycledHeaderFooter = true;
        }
        addViewInLayout(child, flowDown ? -1 : 0, lp, true);
        // add view in layout will reset the RTL properties. We have to re-resolve them
        child->resolveRtlPropertiesIfNeeded();
    }

    if (needToMeasure) {
        int childWidthSpec = ViewGroup::getChildMeasureSpec(mWidthMeasureSpec,mListPadding.left + mListPadding.width, lp->width);
        int lpHeight = lp->height;
        int childHeightSpec;
        if (lpHeight > 0) {
            childHeightSpec = MeasureSpec::makeMeasureSpec(lpHeight, MeasureSpec::EXACTLY);
        } else {
            childHeightSpec = MeasureSpec::makeSafeMeasureSpec(getMeasuredHeight(),MeasureSpec::UNSPECIFIED);
        }
        child->measure(childWidthSpec, childHeightSpec);
    } else {
        cleanupLayoutState(child);
    }

    const int w = child->getMeasuredWidth();
    const int h = child->getMeasuredHeight();
    const int childTop = flowDown ? y : y - h;

    if (needToMeasure) {
        child->layout(childrenLeft, childTop, w, h);
    } else {
        child->offsetLeftAndRight(childrenLeft - child->getLeft());
        child->offsetTopAndBottom(childTop - child->getTop());
    }
    if (mCachingStarted && !child->isDrawingCacheEnabled())
        child->setDrawingCacheEnabled(true);
}

bool ListView::canAnimate()const {
    return AbsListView::canAnimate() && mItemCount > 0;
}

void ListView::setSelection(int position) {
    setSelectionFromTop(position, 0);
}

void ListView::setSelectionInt(int position) {
    bool awakeScrollbars = false;
    setNextSelectedPositionInt(position);

    int selectedPosition = mSelectedPosition;
    if (selectedPosition >= 0) {
        if (position == selectedPosition - 1) {
            awakeScrollbars = true;
        } else if (position == selectedPosition + 1) {
            awakeScrollbars = true;
        }
    }

    if (mPositionScroller) mPositionScroller->stop();

    layoutChildren();

    if (awakeScrollbars) awakenScrollBars();
}

int ListView::lookForSelectablePosition(int position, bool lookDown) {
    Adapter* adapter = mAdapter;
    if (adapter == nullptr || isInTouchMode()) {
        return INVALID_POSITION;
    }

    const int count = adapter->getCount();
    if (!mAreAllItemsSelectable) {
        if (lookDown) {
            position = std::max(0, position);
            while (position < count && !adapter->isEnabled(position)) {
                position++;
            }
        } else {
            position = std::min(position, count - 1);
            while (position >= 0 && !adapter->isEnabled(position)) {
                position--;
            }
        }
    }

    if (position < 0 || position >= count) {
        return INVALID_POSITION;
    }

    return position;
}

int ListView::lookForSelectablePositionAfter(int current, int position, bool lookDown) {
    if (mAdapter == nullptr || isInTouchMode()) {
        return INVALID_POSITION;
    }

    // First check after the starting position in the specified direction.
    const int after = lookForSelectablePosition(position, lookDown);
    if (after != INVALID_POSITION) {
        return after;
    }

    // Then check between the starting position and the current position.
    const int count = mAdapter->getCount();
    current = MathUtils::constrain(current, -1, count - 1);
    if (lookDown) {
        position = std::min(position - 1, count - 1);
        while ((position > current) && !mAdapter->isEnabled(position)) {
            position--;
        }
        if (position <= current) return INVALID_POSITION;
    } else {
        position = std::max(0, position + 1);
        while ((position < current) && !mAdapter->isEnabled(position)) {
            position++;
        }
        if (position >= current) return INVALID_POSITION;
    }
    return position;
}

void ListView::setSelectionAfterHeaderView() {
    const int count = getHeaderViewsCount();
    if (count > 0) {
        mNextSelectedPosition = 0;
        return;
    }

    if (mAdapter) {
        setSelection(count);
    } else {
        mNextSelectedPosition = count;
        mLayoutMode = LAYOUT_SET_SELECTION;
    }
}

bool ListView::dispatchKeyEvent(KeyEvent& event) {
    bool handled = AbsListView::dispatchKeyEvent(event);
    if (!handled) {// If we didn't handle it...
        View* focused = getFocusedChild();
        if (focused && event.getAction() == KeyEvent::ACTION_DOWN) {
            // ... and our focused child didn't handle it
            // ... give it to ourselves so we can scroll if necessary
            handled = onKeyDown(event.getKeyCode(), event);
        }
    }
    return handled;
}

bool ListView::onKeyDown(int keyCode,KeyEvent& event) {
    return commonKey(keyCode, 1, event);
}

bool ListView::onKeyMultiple(int keyCode, int repeatCount, KeyEvent& event) {
    return commonKey(keyCode, repeatCount, event);
}

bool ListView::commonKey(int keyCode, int count, KeyEvent& event) {
    if (mAdapter == nullptr || !isAttachedToWindow()) {
        return false;
    }

    if (mDataChanged) layoutChildren();

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

    LOGV("%s action=%d handled=%d",KeyEvent::getLabel(keyCode),action,handled);
    if (!handled && action != KeyEvent::ACTION_UP) {
        switch (keyCode) {
        case KeyEvent::KEYCODE_DPAD_UP:
            if (event.hasNoModifiers()) {
                handled = resurrectSelectionIfNeeded();
                if (!handled) {
                    while (count-- > 0) {
                        if (arrowScroll(FOCUS_UP)) {
                            handled = true;
                        } else {
                            break;
                        }
                    }
                }
            } else if (event.hasModifiers(KeyEvent::META_ALT_ON)) {
                handled = resurrectSelectionIfNeeded() || fullScroll(FOCUS_UP);
            }
            break;

        case KeyEvent::KEYCODE_DPAD_DOWN:
            if (event.hasNoModifiers()) {
                handled = resurrectSelectionIfNeeded();
                if (!handled) {
                    while (count-- > 0) {
                        if (arrowScroll(FOCUS_DOWN)) {
                            handled = true;
                        } else {
                            break;
                        }
                    }
                }
            } else if (event.hasModifiers(KeyEvent::META_ALT_ON)) {
                handled = resurrectSelectionIfNeeded() || fullScroll(FOCUS_DOWN);
            }
            break;

        case KeyEvent::KEYCODE_DPAD_LEFT:
            if (event.hasNoModifiers()) {
                handled = handleHorizontalFocusWithinListItem(FOCUS_LEFT);
            }
            break;

        case KeyEvent::KEYCODE_DPAD_RIGHT:
            if (event.hasNoModifiers()) {
                handled = handleHorizontalFocusWithinListItem(View::FOCUS_RIGHT);
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
            // This creates an asymmetry in TAB navigation order. At some
            // point in the future we may decide that it's preferable to
            // force the list selection to the top or bottom when receiving
            // TAB focus from another widget, but for now this is adequate.
            if (event.hasNoModifiers()) {
                handled = resurrectSelectionIfNeeded() || arrowScroll(FOCUS_DOWN);
            } else if (event.hasModifiers(KeyEvent::META_SHIFT_ON)) {
                handled = resurrectSelectionIfNeeded() || arrowScroll(FOCUS_UP);
            }
            break;
        }
    }

    if (handled) {
        return true;
    }

    //if (sendToTextFilter(keyCode, count, event)) return true;

    switch (action) {
    case KeyEvent::ACTION_DOWN: return AbsListView::onKeyDown(keyCode, event);
    case KeyEvent::ACTION_UP:   return AbsListView::onKeyUp(keyCode, event);
    case KeyEvent::ACTION_MULTIPLE:
        return AbsListView::onKeyMultiple(keyCode, count, event);
    default: return false;// shouldn't happen
    }
}

bool ListView::pageScroll(int direction) {
    int nextPage;
    bool down;

    if (direction == FOCUS_UP) {
        nextPage = std::max(0, mSelectedPosition - getChildCount() - 1);
        down = false;
    } else if (direction == FOCUS_DOWN) {
        nextPage = std::min(mItemCount - 1, mSelectedPosition + getChildCount() - 1);
        down = true;
    } else {
        return false;
    }

    if (nextPage >= 0) {
        int position = lookForSelectablePositionAfter(mSelectedPosition, nextPage, down);
        if (position >= 0) {
            mLayoutMode = LAYOUT_SPECIFIC;
            mSpecificTop = mPaddingTop + getVerticalFadingEdgeLength();
            if (down && (position > (mItemCount - getChildCount()))) {
                mLayoutMode = LAYOUT_FORCE_BOTTOM;
            }

            if (!down && (position < getChildCount())) {
                mLayoutMode = LAYOUT_FORCE_TOP;
            }

            setSelectionInt(position);
            invokeOnItemScrollListener();
            if (!awakenScrollBars()) {
                invalidate();
            }

            return true;
        }
    }
    return false;
}

bool ListView::fullScroll(int direction) {
    bool moved = false;
    if (direction == FOCUS_UP) {
        if (mSelectedPosition != 0) {
            int position = lookForSelectablePositionAfter(mSelectedPosition, 0, true);
            if (position >= 0) {
                mLayoutMode = LAYOUT_FORCE_TOP;
                setSelectionInt(position);
                invokeOnItemScrollListener();
            }
            moved = true;
        }
    } else if (direction == FOCUS_DOWN) {
        int lastItem = (mItemCount - 1);
        if (mSelectedPosition < lastItem) {
            int position = lookForSelectablePositionAfter(mSelectedPosition, lastItem, false);
            if (position >= 0) {
                mLayoutMode = LAYOUT_FORCE_BOTTOM;
                setSelectionInt(position);
                invokeOnItemScrollListener();
            }
            moved = true;
        }
    }

    if (moved && !awakenScrollBars()) {
        awakenScrollBars();
        invalidate();
    }
    return moved;
}

/**
* To avoid horizontal focus searches changing the selected item, we
* manually focus search within the selected item (as applicable), and
* prevent focus from jumping to something within another item.
* @param direction one of {View.FOCUS_LEFT, View.FOCUS_RIGHT}
* @return Whether this consumes the key event.
*/
bool ListView::handleHorizontalFocusWithinListItem(int direction) {
    if (direction != View::FOCUS_LEFT && direction != View::FOCUS_RIGHT)  {
        LOGD("direction must be one of{View.FOCUS_LEFT, View.FOCUS_RIGHT}");
    }

    int numChildren = getChildCount();
    if (mItemsCanFocus && numChildren > 0 && mSelectedPosition != INVALID_POSITION) {
        View* selectedView = getSelectedView();
        if (selectedView && selectedView->hasFocus() &&
                dynamic_cast<ViewGroup*>(selectedView)) {

            View* currentFocus = selectedView->findFocus();
            View* nextFocus = FocusFinder::getInstance().findNextFocus(
                       (ViewGroup*) selectedView, currentFocus, direction);
            if (nextFocus != nullptr) {
                // do the math to get interesting rect in next focus' coordinates
                Rect focusedRect ;
                if (currentFocus != nullptr) {
                    currentFocus->getFocusedRect(focusedRect);
                    offsetDescendantRectToMyCoords(currentFocus, focusedRect);
                    offsetRectIntoDescendantCoords(nextFocus, focusedRect);
                } else {
                    focusedRect.set(0,0,0,0);
                }
                if (nextFocus->requestFocus(direction, focusedRect.empty()?nullptr:&focusedRect)) {
                    return true;
                }
            }
            // we are blocking the key from being handled (by returning true)
            // if the global result is going to be some other view within this
            // list.  this is to acheive the overall goal of having
            // horizontal d-pad navigation remain in the current item.
            View* globalNextFocus = FocusFinder::getInstance().findNextFocus(
                       (ViewGroup*) getRootView(), currentFocus, direction);
            if (globalNextFocus != nullptr) {
                return isViewAncestorOf(globalNextFocus, this);
            }
        }
    }
    return false;
}


bool  ListView::arrowScroll(int direction) {
    mInLayout = true;
    const bool handled=arrowScrollImpl(direction);
    if(handled)
        playSoundEffect(SoundEffectConstants::getContantForFocusDirection(direction));
    mInLayout=false;
    return handled;
}

int ListView::nextSelectedPositionForDirection(View* selectedView, int selectedPos, int direction) {
    int nextSelected;

    if (direction == View::FOCUS_DOWN) {
        int listBottom = getHeight() - mListPadding.height;
        if (selectedView && selectedView->getBottom() <= listBottom) {
            nextSelected = selectedPos != INVALID_POSITION && selectedPos >= mFirstPosition ?
                           selectedPos + 1 :mFirstPosition;
        } else {
            return INVALID_POSITION;
        }
    } else {
        int listTop = mListPadding.top;
        if (selectedView && selectedView->getTop() >= listTop) {
            int lastPos = mFirstPosition + getChildCount() - 1;
            nextSelected = selectedPos != INVALID_POSITION && selectedPos <= lastPos ?
                           selectedPos - 1 :lastPos;
        } else {
            return INVALID_POSITION;
        }
    }

    if (nextSelected < 0 || nextSelected >= mAdapter->getCount()) {
        return INVALID_POSITION;
    }
    return lookForSelectablePosition(nextSelected, direction == View::FOCUS_DOWN);
}

bool  ListView::arrowScrollImpl(int direction) {
    if (getChildCount() <= 0) {
        return false;
    }

    View* selectedView = getSelectedView();
    int selectedPos = mSelectedPosition;

    int nextSelectedPosition = nextSelectedPositionForDirection(selectedView, selectedPos, direction);
    int amtToScroll = amountToScroll(direction, nextSelectedPosition);

    // if we are moving focus, we may OVERRIDE the default behavior
    ArrowScrollFocusResult* focusResult = mItemsCanFocus ? arrowScrollFocused(direction) : nullptr;
    if (focusResult != nullptr) {
        nextSelectedPosition = focusResult->getSelectedPosition();
        amtToScroll = focusResult->getAmountToScroll();
    }

    bool needToRedraw = focusResult != nullptr;
    if (nextSelectedPosition != INVALID_POSITION) {
        handleNewSelectionChange(selectedView, direction, nextSelectedPosition, focusResult != nullptr);
        setSelectedPositionInt(nextSelectedPosition);
        setNextSelectedPositionInt(nextSelectedPosition);
        selectedView = getSelectedView();
        selectedPos = nextSelectedPosition;
        if (mItemsCanFocus && focusResult == nullptr) {
            // there was no new view found to take focus, make sure we
            // don't leave focus with the old selection
            View* focused = getFocusedChild();
            if (focused != nullptr) {
                focused->clearFocus();
            }
        }
        needToRedraw = true;
        checkSelectionChanged();
    }

    if (amtToScroll > 0) {
        scrollListItemsBy((direction == View::FOCUS_UP) ? amtToScroll : -amtToScroll);
        needToRedraw = true;
    }

    // if we didn't find a new focusable, make sure any existing focused
    // item that was panned off screen gives up focus.
    if (mItemsCanFocus && (focusResult == nullptr)
            && selectedView != nullptr && selectedView->hasFocus()) {
        View* focused = selectedView->findFocus();
        if (focused != nullptr) {
            if (!isViewAncestorOf(focused, this) || distanceToView(focused) > 0) {
                focused->clearFocus();
            }
        }
    }

    // if  the current selection is panned off, we need to remove the selection
    if (nextSelectedPosition == INVALID_POSITION && selectedView != nullptr
            && !isViewAncestorOf(selectedView, this)) {
        selectedView = nullptr;
        hideSelector();

        // but we don't want to set the ressurect position (that would make subsequent
        // unhandled key events bring back the item we just scrolled off!)
        mResurrectToPosition = INVALID_POSITION;
    }

    if (needToRedraw) {
        if (selectedView != nullptr) {
            positionSelectorLikeFocus(selectedPos, selectedView);
            mSelectedTop = selectedView->getTop();
        }
        if (!awakenScrollBars()) {
            invalidate();
        }
        invokeOnItemScrollListener();
        return true;
    }

    return false;
}

void ListView::handleNewSelectionChange(View* selectedView, int direction, int newSelectedPosition, bool newFocusAssigned) {
    LOGE_IF(newSelectedPosition == INVALID_POSITION,"newSelectedPosition %d needs to be valid",newSelectedPosition);

    // whether or not we are moving down or up, we want to preserve the
    // top of whatever view is on top:
    // - moving down: the view that had selection
    // - moving up: the view that is getting selection
    View* topView;
    View* bottomView;
    int topViewIndex, bottomViewIndex;
    bool topSelected = false;
    const int selectedIndex = mSelectedPosition - mFirstPosition;
    const int nextSelectedIndex = newSelectedPosition - mFirstPosition;
    if (direction == View::FOCUS_UP) {
        topViewIndex = nextSelectedIndex;
        bottomViewIndex = selectedIndex;
        topView = getChildAt(topViewIndex);
        bottomView = selectedView;
        topSelected = true;
    } else {
        topViewIndex = selectedIndex;
        bottomViewIndex = nextSelectedIndex;
        topView = selectedView;
        bottomView = getChildAt(bottomViewIndex);
    }

    const int numChildren = getChildCount();

    // start with top view: is it changing size?
    if (topView != nullptr) {
        topView->setSelected(!newFocusAssigned && topSelected);
        measureAndAdjustDown(topView, topViewIndex, numChildren);
    }

    // is the bottom view changing size?
    if (bottomView != nullptr) {
        bottomView->setSelected(!newFocusAssigned && !topSelected);
        measureAndAdjustDown(bottomView, bottomViewIndex, numChildren);
    }
}

void ListView::measureAndAdjustDown(View* child, int childIndex, int numChildren) {
    int oldHeight = child->getHeight();
    measureItem(child);
    if (child->getMeasuredHeight() != oldHeight) {
        // lay out the view, preserving its top
        relayoutMeasuredItem(child);

        // adjust views below appropriately
        int heightDelta = child->getMeasuredHeight() - oldHeight;
        for (int i = childIndex + 1; i < numChildren; i++) {
            getChildAt(i)->offsetTopAndBottom(heightDelta);
        }
    }
}

void ListView::measureItem(View* child) {
    ViewGroup::LayoutParams* lp = child->getLayoutParams();
    if (lp == nullptr) {
        lp = generateDefaultLayoutParams();//new LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT);
    }
    int childWidthSpec,childHeightSpec;
    childWidthSpec = getChildMeasureSpec(mWidthMeasureSpec,
            mListPadding.left + mListPadding.width, lp->width);
    int lpHeight = lp->height;
    if (lpHeight > 0) {
        childHeightSpec = MeasureSpec::makeMeasureSpec(lpHeight, MeasureSpec::EXACTLY);
    } else {
        childHeightSpec = MeasureSpec::makeSafeMeasureSpec(getMeasuredHeight(),MeasureSpec::UNSPECIFIED);
    }
    child->measure(childWidthSpec, childHeightSpec);
}

void ListView::relayoutMeasuredItem(View* child) {
    const int w = child->getMeasuredWidth();
    const int h = child->getMeasuredHeight();
    const int childLeft = mListPadding.left;
    const int childTop = child->getTop();
    child->layout(childLeft, childTop, w, h);
}

int ListView::getArrowScrollPreviewLength() {
    return std::max((int)MIN_SCROLL_PREVIEW_PIXELS, getVerticalFadingEdgeLength());
}

int ListView::amountToScroll(int direction, int nextSelectedPosition) {
    const int listBottom = getHeight() - mListPadding.height;
    const int listTop = mListPadding.top;

    int numChildren = getChildCount();

    if (direction == View::FOCUS_DOWN) {
        int indexToMakeVisible = numChildren - 1;
        if (nextSelectedPosition != INVALID_POSITION) {
            indexToMakeVisible = nextSelectedPosition - mFirstPosition;
        }
        while (numChildren <= indexToMakeVisible) {
            // Child to view is not attached yet.
            addViewBelow(getChildAt(numChildren - 1), mFirstPosition + numChildren - 1);
            numChildren++;
        }
        int positionToMakeVisible = mFirstPosition + indexToMakeVisible;
        View* viewToMakeVisible = getChildAt(indexToMakeVisible);

        int goalBottom = listBottom;
        if (positionToMakeVisible < mItemCount - 1) {
            goalBottom -= getArrowScrollPreviewLength();
        }

        if (viewToMakeVisible->getBottom() <= goalBottom) {
            // item is fully visible.
            return 0;
        }

        if (nextSelectedPosition != INVALID_POSITION
                && (goalBottom - viewToMakeVisible->getTop()) >= getMaxScrollAmount()) {
            // item already has enough of it visible, changing selection is good enough
            return 0;
        }

        int amtToScroll = (viewToMakeVisible->getBottom() - goalBottom);

        if ((mFirstPosition + numChildren) == mItemCount) {
            // last is last in list -> make sure we don't scroll past it
            int max = getChildAt(numChildren - 1)->getBottom() - listBottom;
            amtToScroll = std::min(amtToScroll, max);
        }

        return std::min(amtToScroll, getMaxScrollAmount());
    } else {
        int indexToMakeVisible = 0;
        if (nextSelectedPosition != INVALID_POSITION) {
            indexToMakeVisible = nextSelectedPosition - mFirstPosition;
        }
        while (indexToMakeVisible < 0) {
            // Child to view is not attached yet.
            addViewAbove(getChildAt(0), mFirstPosition);
            mFirstPosition--;
            indexToMakeVisible = nextSelectedPosition - mFirstPosition;
        }
        int positionToMakeVisible = mFirstPosition + indexToMakeVisible;
        View* viewToMakeVisible = getChildAt(indexToMakeVisible);
        int goalTop = listTop;
        if (positionToMakeVisible > 0) {
            goalTop += getArrowScrollPreviewLength();
        }
        if (viewToMakeVisible->getTop() >= goalTop) {
            // item is fully visible.
            return 0;
        }
        if (nextSelectedPosition != INVALID_POSITION &&
                (viewToMakeVisible->getBottom() - goalTop) >= getMaxScrollAmount()) {
            // item already has enough of it visible, changing selection is good enough
            return 0;
        }

        int amtToScroll = (goalTop - viewToMakeVisible->getTop());
        if (mFirstPosition == 0) {
            // first is first in list -> make sure we don't scroll past it
            int max = listTop - getChildAt(0)->getTop();
            amtToScroll = std::min(amtToScroll,  max);
        }
        return std::min(amtToScroll, getMaxScrollAmount());
    }
}

int ListView::lookForSelectablePositionOnScreen(int direction) {
    int firstPosition = mFirstPosition;
    if (direction == View::FOCUS_DOWN) {
        int startPos = (mSelectedPosition != INVALID_POSITION) ? mSelectedPosition + 1 :firstPosition;
        if (startPos >= mAdapter->getCount()) {
            return INVALID_POSITION;
        }
        if (startPos < firstPosition) {
            startPos = firstPosition;
        }

        int lastVisiblePos = getLastVisiblePosition();
        for (int pos = startPos; pos <= lastVisiblePos; pos++) {
            if (mAdapter->isEnabled(pos)
                    && getChildAt(pos - firstPosition)->getVisibility() == View::VISIBLE) {
                return pos;
            }
        }
    } else {
        int last = firstPosition + getChildCount() - 1;
        int startPos = (mSelectedPosition != INVALID_POSITION) ?
                       mSelectedPosition - 1 :firstPosition + getChildCount() - 1;
        if (startPos < 0 || startPos >= mAdapter->getCount()) {
            return INVALID_POSITION;
        }
        if (startPos > last) {
            startPos = last;
        }

        for (int pos = startPos; pos >= firstPosition; pos--) {
            if (mAdapter->isEnabled(pos)
                    && getChildAt(pos - firstPosition)->getVisibility() == View::VISIBLE) {
                return pos;
            }
        }
    }
    return INVALID_POSITION;
}

ListView::ArrowScrollFocusResult* ListView::arrowScrollFocused(int direction) {
    View* selectedView = getSelectedView();
    View* newFocus=nullptr;
    Rect mTempRect;
    if (selectedView != nullptr && selectedView->hasFocus()) {
        View* oldFocus = selectedView->findFocus();
        newFocus = FocusFinder::getInstance().findNextFocus(this, oldFocus, direction);
    } else {
        if (direction == View::FOCUS_DOWN) {
            bool topFadingEdgeShowing = (mFirstPosition > 0);
            int listTop = mListPadding.top + (topFadingEdgeShowing ? getArrowScrollPreviewLength() : 0);
            int ySearchPoint =(selectedView != nullptr  && selectedView->getTop() > listTop) ?
                              selectedView->getTop() : listTop;
            mTempRect.set(0, ySearchPoint, 0, ySearchPoint);
        } else {
            bool bottomFadingEdgeShowing = (mFirstPosition + getChildCount() - 1) < mItemCount;
            int listBottom = getHeight() - mListPadding.height -
                             (bottomFadingEdgeShowing ? getArrowScrollPreviewLength() : 0);
            int ySearchPoint = (selectedView != nullptr && selectedView->getBottom() < listBottom) ?
                               selectedView->getBottom() : listBottom;
            mTempRect.set(0, ySearchPoint, 0, ySearchPoint);
        }
        newFocus = FocusFinder::getInstance().findNextFocusFromRect(this, &mTempRect, direction);
    }

    if (newFocus != nullptr) {
        int posOfNewFocus = positionOfNewFocus(newFocus);

        // if the focus change is in a different new position, make sure
        // we aren't jumping over another selectable position
        if (mSelectedPosition != INVALID_POSITION && posOfNewFocus != mSelectedPosition) {
            int selectablePosition = lookForSelectablePositionOnScreen(direction);
            if (selectablePosition != INVALID_POSITION &&
                    ((direction == View::FOCUS_DOWN && selectablePosition < posOfNewFocus) ||
                     (direction == View::FOCUS_UP && selectablePosition > posOfNewFocus))) {
                return nullptr;
            }
        }

        int focusScroll = amountToScrollToNewFocus(direction, newFocus, posOfNewFocus);

        int maxScrollAmount = getMaxScrollAmount();
        if (focusScroll < maxScrollAmount) {
            // not moving too far, safe to give next view focus
            newFocus->requestFocus(direction);
            mArrowScrollFocusResult.populate(posOfNewFocus, focusScroll);
            return &mArrowScrollFocusResult;
        } else if (distanceToView(newFocus) < maxScrollAmount) {
            // Case to consider:
            // too far to get entire next focusable on screen, but by going
            // max scroll amount, we are getting it at least partially in view,
            // so give it focus and scroll the max ammount.
            newFocus->requestFocus(direction);
            mArrowScrollFocusResult.populate(posOfNewFocus, maxScrollAmount);
            return &mArrowScrollFocusResult;
        }
    }
    return nullptr;
}

int ListView::positionOfNewFocus(View* newFocus) {
    int numChildren = getChildCount();
    for (int i = 0; i < numChildren; i++) {
        View* child = getChildAt(i);
        if (isViewAncestorOf(newFocus, child)) {
            return mFirstPosition + i;
        }
    }
    LOGE("newFocus is not a child of any of the children of the list!");
    return 0;
}

bool ListView::isViewAncestorOf(View* child, View* parent) {
    if (child == parent) return true;

    ViewGroup* theParent = child->getParent();
    return theParent&&isViewAncestorOf(theParent, parent);
}

int ListView::amountToScrollToNewFocus(int direction, View* newFocus, int positionOfNewFocus) {
    int amountToScroll = 0;
    Rect mTempRect;
    newFocus->getDrawingRect(mTempRect);
    offsetDescendantRectToMyCoords(newFocus, mTempRect);
    if (direction == View::FOCUS_UP) {
        if (mTempRect.top < mListPadding.top) {
            amountToScroll = mListPadding.top - mTempRect.top;
            if (positionOfNewFocus > 0) {
                amountToScroll += getArrowScrollPreviewLength();
            }
        }
    } else {
        int listBottom = getHeight() - mListPadding.height;
        if (mTempRect.bottom() > listBottom) {
            amountToScroll = mTempRect.bottom() - listBottom;
            if (positionOfNewFocus < mItemCount - 1) {
                amountToScroll += getArrowScrollPreviewLength();
            }
        }
    }
    return amountToScroll;
}

int ListView::distanceToView(View* descendant) {
    int distance = 0;
    Rect tmpRect;
    descendant->getDrawingRect(tmpRect);
    offsetDescendantRectToMyCoords(descendant, tmpRect);
    int listBottom = mBottom-mTop - mListPadding.height;
    if (tmpRect.bottom() < mListPadding.top) {
        distance = mListPadding.top - tmpRect.bottom();
    } else if (tmpRect.top > listBottom) {
        distance = tmpRect.top - listBottom;
    }
    return distance;
}

void ListView::scrollListItemsBy(int amount) {
    const int oldX = mScrollX;
    const int oldY = mScrollY;
    offsetChildrenTopAndBottom(amount);

    const int listBottom = mBottom-mTop - mListPadding.height;
    const int listTop = mListPadding.top;

    if (amount < 0) {
        // shifted items up

        // may need to pan views into the bottom space
        int numChildren = getChildCount();
        View* last = getChildAt(numChildren - 1);
        while (last->getBottom() < listBottom) {
            int lastVisiblePosition = mFirstPosition + numChildren - 1;
            if (lastVisiblePosition < mItemCount - 1) {
                last = addViewBelow(last, lastVisiblePosition);
                numChildren++;
            } else {
                break;
            }
        }

        // may have brought in the last child of the list that is skinnier
        // than the fading edge, thereby leaving space at the end.  need
        // to shift back
        if (last->getBottom() < listBottom) {
            offsetChildrenTopAndBottom(listBottom - last->getBottom());
        }

        // top views may be panned off screen
        View* first = getChildAt(0);
        while (first->getBottom() < listTop) {
            LayoutParams* layoutParams = (LayoutParams*) first->getLayoutParams();
            if (mRecycler->shouldRecycleViewType(layoutParams->viewType)) {
                mRecycler->addScrapView(first, mFirstPosition);
            }
            detachViewFromParent(first);
            first = getChildAt(0);
            mFirstPosition++;
        }
    } else {
        // shifted items down
        View* first = getChildAt(0);

        // may need to pan views into top
        while ((first->getTop() > listTop) && (mFirstPosition > 0)) {
            first = addViewAbove(first, mFirstPosition);
            mFirstPosition--;
        }

        // may have brought the very first child of the list in too far and
        // need to shift it back
        if (first->getTop() > listTop) {
            offsetChildrenTopAndBottom(listTop - first->getTop());
        }

        int lastIndex = getChildCount() - 1;
        View* last = getChildAt(lastIndex);

        // bottom view may be panned off screen
        while (last->getTop() > listBottom) {
            LayoutParams* layoutParams = (LayoutParams*) last->getLayoutParams();
            if (mRecycler->shouldRecycleViewType(layoutParams->viewType)) {
                mRecycler->addScrapView(last, mFirstPosition+lastIndex);
            }
            detachViewFromParent(last);
            last = getChildAt(--lastIndex);
        }
    }
    mRecycler->fullyDetachScrapViews();
    removeUnusedFixedViews(mHeaderViewInfos);
    removeUnusedFixedViews(mFooterViewInfos);
    onScrollChanged(mScrollX,mScrollY,oldX,oldY);
}

View* ListView::addViewAbove(View* theView, int position) {
    const int abovePosition = position - 1;
    View* view = obtainView(abovePosition, mIsScrap);
    const int edgeOfNewChild = theView->getTop() - mDividerHeight;
    setupChild(view, abovePosition, edgeOfNewChild, false, mListPadding.left,
               false, mIsScrap[0]);
    return view;
}

View* ListView::addViewBelow(View* theView, int position) {
    const int belowPosition = position + 1;
    View* view = obtainView(belowPosition, mIsScrap);
    const int edgeOfNewChild = theView->getBottom() + mDividerHeight;
    setupChild(view, belowPosition, edgeOfNewChild, true, mListPadding.left,
               false, mIsScrap[0]);
    return view;
}

void ListView::setItemsCanFocus(bool itemsCanFocus) {
    mItemsCanFocus = itemsCanFocus;
    if (!itemsCanFocus) {
        setDescendantFocusability(ViewGroup::FOCUS_BLOCK_DESCENDANTS);
    }
}

bool ListView::getItemsCanFocus()const {
    return mItemsCanFocus;
}

bool ListView::isOpaque()const {
    const bool retValue = (mCachingActive && mIsCacheColorOpaque && mDividerIsOpaque
                     && hasOpaqueScrollbars()) || AbsListView::isOpaque();
    if (retValue) {
        // only return true if the list items cover the entire area of the view
        const int listTop = mListPadding.top;
        const View* first = getChildAt(0);
        if (first == nullptr || first->getTop() > listTop) {
            return false;
        }
        int listBottom = getHeight() - mListPadding.height;
        const View* last = getChildAt(getChildCount() - 1);
        if (last == nullptr || last->getBottom() < listBottom) {
            return false;
        }
    }
    return retValue;
}

void ListView::setCacheColorHint(int color) {
    const bool opaque = (color >> 24) == 0xFF;
    mIsCacheColorOpaque = opaque;
    if (opaque) {
        /*if (mDividerPaint == null) {
            mDividerPaint = new Paint();
        }
        mDividerPaint.setColor(color);*/
    }
    AbsListView::setCacheColorHint(color);
}


void ListView::drawOverscrollHeader(Canvas&canvas, Drawable* drawable,Rect& bounds) {
    const int height = drawable->getMinimumHeight();

    canvas.save();
    canvas.rectangle(bounds);
    canvas.clip();

    int span = bounds.height;//bottom - bounds.top;
    if (span < height) {
        bounds.top = bounds.bottom() - height;
    }
    drawable->setBounds(bounds);
    drawable->draw(canvas);

    canvas.restore();
}

void ListView::drawOverscrollFooter(Canvas&canvas, Drawable* drawable,Rect& bounds) {
    const int height = drawable->getMinimumHeight();

    canvas.save();
    canvas.rectangle(bounds);
    canvas.clip();
    int span = bounds.height;//bottom - bounds.top;
    if (span < height) {
        bounds.height = height;
    }
    drawable->setBounds(bounds);
    drawable->draw(canvas);

    canvas.restore();
}

void ListView::dispatchDraw(Canvas&canvas) {
    //if (mCachingStarted) mCachingActive = true;

    // Draw the dividers
    int dividerHeight = mDividerHeight;
    Drawable* overscrollHeader = mOverScrollHeader;
    Drawable* overscrollFooter = mOverScrollFooter;
    bool drawDividers = dividerHeight > 0 && mDivider != nullptr;

    if (drawDividers || mOverScrollHeader || mOverScrollFooter) {
        // Only modify the top and bottom in the loop, we set the left and right here
        Rect bounds;
        bounds.left = mPaddingLeft;
        bounds.width = getWidth() - mPaddingRight - mPaddingLeft;

        int count = getChildCount();
        int headerCount = getHeaderViewsCount();
        int itemCount = mItemCount;
        int footerLimit = int(itemCount - mFooterViewInfos.size());
        bool headerDividers = mHeaderDividersEnabled;
        bool footerDividers = mFooterDividersEnabled;
        int first = mFirstPosition;
        bool areAllItemsSelectable = mAreAllItemsSelectable;
        Adapter* adapter = mAdapter;
        // If the list is opaque *and* the background is not, we want to
        // fill a rect where the dividers would be for non-selectable items
        // If the list is opaque and the background is also opaque, we don't
        // need to draw anything since the background will do it for us
        bool fillForMissingDividers = isOpaque() && !AbsListView::isOpaque();

        int effectivePaddingTop = 0;
        int effectivePaddingBottom = 0;
        if ((mGroupFlags & CLIP_TO_PADDING_MASK) == CLIP_TO_PADDING_MASK) {
            effectivePaddingTop = mListPadding.top;
            effectivePaddingBottom = mListPadding.height;
        }

        int listBottom = getWidth() - effectivePaddingBottom + mScrollY;
        if (!mStackFromBottom) {
            int bottom = 0;

            // Draw top divider or header for overscroll
            if (count > 0 && mScrollY < 0) {
                if (mOverScrollHeader) {
                    bounds.top = 0;
                    bounds.height = mScrollY;
                    drawOverscrollHeader(canvas, overscrollHeader, bounds);
                } else if (drawDividers) {
                    bounds.top = 0;
                    bounds.height = dividerHeight;
                    drawDivider(canvas, bounds, -1);
                }
            }

            for (int i = 0; i < count; i++) {
                int itemIndex = (first + i);
                bool isHeader = (itemIndex < headerCount);
                bool isFooter = (itemIndex >= footerLimit);
                if ((headerDividers || !isHeader) && (footerDividers || !isFooter)) {
                    View* child = getChildAt(i);
                    bottom = child->getBottom();
                    bool isLastItem = (i == (count - 1));

                    if (drawDividers && (bottom < listBottom)
                            && !(mOverScrollFooter && isLastItem)) {
                        int nextIndex = (itemIndex + 1);
                        // Draw dividers between enabled items, headers
                        // and/or footers when enabled and requested, and
                        // after the last enabled item.
                        if (adapter->isEnabled(itemIndex) && (headerDividers || !isHeader && (nextIndex >= headerCount)) 
			    && (isLastItem || adapter->isEnabled(nextIndex) && (footerDividers || !isFooter && (nextIndex < footerLimit)))) {
                            bounds.top = bottom;
                            bounds.height = dividerHeight;
                            drawDivider(canvas, bounds, i);
                        } else if (fillForMissingDividers) {
                            bounds.top = bottom;
                            bounds.height = dividerHeight;
                            canvas.set_color(getCacheColorHint());
                            canvas.rectangle(bounds);
                            canvas.fill();
                        }
                    }
                }
            }

            int overFooterBottom = getBottom() + mScrollY;
            if (mOverScrollFooter && first + count == itemCount &&
                    overFooterBottom > bottom) {
                bounds.top = bottom;
                bounds.height = overFooterBottom-bottom;
                drawOverscrollFooter(canvas, overscrollFooter, bounds);
            }
        } else {
            int top;

            if (count > 0 && mOverScrollHeader) {
                bounds.top = mScrollY;
                bounds.height = getChildAt(0)->getTop() - mScrollY;
                drawOverscrollHeader(canvas, overscrollHeader, bounds);
            }

            int start = mOverScrollHeader ? 1 : 0;
            for (int i = start; i < count; i++) {
                int itemIndex = (first + i);
                bool isHeader = (itemIndex < headerCount);
                bool isFooter = (itemIndex >= footerLimit);
                if ((headerDividers || !isHeader) && (footerDividers || !isFooter)) {
                    View* child = getChildAt(i);
                    top = child->getTop();
                    if (drawDividers && (top > effectivePaddingTop)) {
                        bool isFirstItem = (i == start);
                        int previousIndex = (itemIndex - 1);
                        // Draw dividers between enabled items, headers
                        // and/or footers when enabled and requested, and
                        // before the first enabled item.
                        if ( adapter->isEnabled(itemIndex) && (headerDividers || !isHeader && (previousIndex >= headerCount))
						    && (isFirstItem ||adapter->isEnabled(previousIndex)
							     && (footerDividers || !isFooter && (previousIndex < footerLimit)))) {
                            bounds.top = top - dividerHeight;
                            bounds.height = dividerHeight;
                            // Give the method the child ABOVE the divider,
                            // so we subtract one from our child position.
                            // Give -1 when there is no child above the
                            // divider.
                            drawDivider(canvas, bounds, i - 1);
                        } else if (fillForMissingDividers) {
                            bounds.top = top - dividerHeight;
                            bounds.height = dividerHeight;
                            canvas.set_color(getCacheColorHint());
                            canvas.rectangle(bounds);
                            canvas.fill();
                        }
                    }
                }
            }

            if (count > 0 && mScrollY > 0) {
                if (mOverScrollFooter) {
                    int absListBottom = getBottom();
                    bounds.top = absListBottom;
                    bounds.height =  mScrollY;
                    drawOverscrollFooter(canvas, overscrollFooter, bounds);
                } else if (drawDividers) {
                    bounds.top = listBottom;
                    bounds.height = dividerHeight;
                    drawDivider(canvas, bounds, -1);
                }
            }
        }
    }

    // Draw the indicators (these should be drawn above the dividers) and children
    AbsListView::dispatchDraw(canvas);
}

bool ListView::drawChild(Canvas& canvas, View* child, int64_t drawingTime) {
    bool more = AbsListView::drawChild(canvas, child, drawingTime);
    if (mCachingActive /*&& child->mCachingFailed*/) {
        mCachingActive = false;
    }
    return more;
}

void ListView::drawDivider(Canvas&canvas,const Rect&bounds, int childIndex) {
    mDivider->setBounds(bounds);
    mDivider->draw(canvas);
}

Drawable*ListView::getDevider() {
    return mDivider;
}

void ListView::setDivider(Drawable* divider) {
    if (divider != nullptr) {
        mDividerHeight = divider->getIntrinsicHeight();
    } else {
        mDividerHeight = 0;
    }
    if(mDivider!= divider)
        delete mDivider;
    mDivider = divider;
    mDividerIsOpaque = divider == nullptr || divider->getOpacity() == PixelFormat::OPAQUE;
    if(mAdapter && mItemCount)requestLayout();
    invalidate(true);
}

void ListView::setDividerHeight(int h) {
    mDividerHeight=h;
}

int ListView::getDividerHeight()const {
    return mDividerHeight;
}

void ListView::setHeaderDividersEnabled(bool headerDividersEnabled) {
    mHeaderDividersEnabled = headerDividersEnabled;
    invalidate(true);
}

bool ListView::areHeaderDividersEnabled()const {
    return mHeaderDividersEnabled;
}

void ListView::setFooterDividersEnabled(bool footerDividersEnabled) {
    mFooterDividersEnabled = footerDividersEnabled;
    invalidate(true);
}

bool ListView::areFooterDividersEnabled()const {
    return mFooterDividersEnabled;
}

void ListView::setOverscrollHeader(Drawable* header) {
    mOverScrollHeader = header;
    if (mScrollY < 0) {
        invalidate(true);
    }
}

Drawable* ListView::getOverscrollHeader()const {
    return mOverScrollHeader;
}

void ListView::setOverscrollFooter(Drawable* footer) {
    mOverScrollFooter = footer;
    invalidate(true);
}

Drawable* ListView::getOverscrollFooter()const {
    return mOverScrollFooter;
}

void ListView::onFocusChanged(bool gainFocus, int direction,Rect* previouslyFocusedRect) {
    AbsListView::onFocusChanged(gainFocus, direction, previouslyFocusedRect);

    ListAdapter* adapter = mAdapter;
    int closetChildIndex = -1;
    int closestChildTop = 0;
    if (adapter && gainFocus && previouslyFocusedRect ) {
        previouslyFocusedRect->offset(mScrollX, mScrollY);

        // Don't cache the result of getChildCount or mFirstPosition here,
        // it could change in layoutChildren.
        if (adapter->getCount() < getChildCount() + mFirstPosition) {
            mLayoutMode = LAYOUT_NORMAL;
            layoutChildren();
        }

        // figure out which item should be selected based on previously
        // focused rect
        Rect otherRect;
        int minDistance = INT_MAX;
        const int childCount = getChildCount();

        for (int i = 0; i < childCount; i++) {
            // only consider selectable views
            if (!adapter->isEnabled(mFirstPosition + i)) {
                continue;
            }

            const View* other = getChildAt(i);
            other->getDrawingRect(otherRect);
            offsetDescendantRectToMyCoords(other, otherRect);
            int distance = getDistance(*previouslyFocusedRect, otherRect, direction);

            if (distance < minDistance) {
                minDistance = distance;
                closetChildIndex = i;
                closestChildTop = other->getTop();
            }
        }
    }

    if (closetChildIndex >= 0) {
        setSelectionFromTop(closetChildIndex + mFirstPosition, closestChildTop);
    } else {
        requestLayout();
    }
}

void ListView::onFinishInflate() {
    AbsListView::onFinishInflate();

    const int count = getChildCount();
    if (count > 0) {
        for (int i = 0; i < count; ++i) {
            addHeaderView(getChildAt(i));
        }
        removeAllViews();
    }
}

View* ListView::findViewTraversal(int id) {
    // First look in our children, then in any header and footer views that
    // may be scrolled off.
    View* v = AbsListView::findViewTraversal(id);
    if (v == nullptr) {
        v = findViewInHeadersOrFooters(mHeaderViewInfos, id);
        if (v != nullptr) {
            return v;
        }
        v = findViewInHeadersOrFooters(mFooterViewInfos, id);
        if (v != nullptr) {
            return v;
        }
    }
    return v;
}

View* ListView::findViewInHeadersOrFooters(const std::vector<FixedViewInfo*>& where, int id) {
    // Look in the passed in list of headers or footers for the view.
    const size_t len = where.size();
    for (size_t i = 0; i < len; i++) {
        View*v = where.at(i)->view;
        //if (!v.isRootNamespace()) {
            v = v->findViewById(id);
            if (v != nullptr) {
                return v;
            }
        //}
    }
    return nullptr;
}

View* ListView::findViewByPredicateTraversal(const Predicate<View*>&predicate,View* childToSkip) {
    View* v = AbsListView::findViewByPredicateTraversal(predicate, childToSkip);
    if (v == nullptr) {
        v = findViewByPredicateInHeadersOrFooters(mHeaderViewInfos, predicate, childToSkip);
        if (v)return v;
        v = findViewByPredicateInHeadersOrFooters(mFooterViewInfos, predicate, childToSkip);
        if (v)return v;
    }
    return v;
}

View* ListView::findViewByPredicateInHeadersOrFooters(const std::vector<FixedViewInfo*>&where,
        const Predicate<View*>&predicate, View* childToSkip) {
    const size_t len = where.size();
    View* v;

    for (size_t i = 0; i < len; i++) {
        v = where.at(i)->view;

        if (v != childToSkip && !v->isRootNamespace()) {
            v = v->findViewByPredicate(predicate);
            if (v) return v;
        }
    }
    return nullptr;
}

View* ListView::findViewWithTagInHeadersOrFooters(std::vector<FixedViewInfo*>& where, void* tag) {
    // Look in the passed in list of headers or footers for the view with the tag.
    const size_t len = where.size();

    for (size_t i = 0; i < len; i++) {
        View* v = where.at(i)->view;

        if (!v->isRootNamespace()) {
            v = v->findViewWithTag(tag);
            if (v)return v;
        }
    }
    return nullptr;
}

int ListView::getHeightForPosition(int position) {
    int height = AbsListView::getHeightForPosition(position);
    if (shouldAdjustHeightForDivider(position)) {
        return height + mDividerHeight;
    }
    return height;
}

bool ListView::shouldAdjustHeightForDivider(int itemIndex) {
    const bool drawDividers = mDividerHeight > 0 && mDivider != nullptr;

    if (drawDividers) {
        bool fillForMissingDividers = isOpaque() && !AbsListView::isOpaque();
        const int itemCount = mItemCount;
        const int headerCount = getHeaderViewsCount();
        const int footerLimit = int(itemCount - mFooterViewInfos.size());
        const bool isHeader = (itemIndex < headerCount);
        const bool isFooter = (itemIndex >= footerLimit);
        const bool headerDividers = mHeaderDividersEnabled;
        const bool footerDividers = mFooterDividersEnabled;
        if ((headerDividers || !isHeader) && (footerDividers || !isFooter)) {
            if (!mStackFromBottom) {
                bool isLastItem = (itemIndex == (itemCount - 1));
                if (!mOverScrollHeader || !isLastItem) {
                    int nextIndex = itemIndex + 1;
                    /* Draw dividers between enabled items, headers and/or footers
                     *when enabled and requested, and after the last enabled item.*/
                    if (mAdapter->isEnabled(itemIndex) && (headerDividers || !isHeader && (nextIndex >= headerCount))
                            && (isLastItem || mAdapter->isEnabled(nextIndex)
                            && (footerDividers || !isFooter && (nextIndex < footerLimit)))) {
                        return true;
                    } else if (fillForMissingDividers) {
                        return true;
                    }
                }
            } else {
                const int start = mOverScrollHeader ? 1 : 0;
                const bool isFirstItem = (itemIndex == start);
                if (!isFirstItem) {
                    const int previousIndex = (itemIndex - 1);
                    // Draw dividers between enabled items, headers
                    // and/or footers when enabled and requested, and
                    // before the first enabled item.
                    if (mAdapter->isEnabled(itemIndex) && (headerDividers || !isHeader && (previousIndex >= headerCount))
                            && (isFirstItem  || mAdapter->isEnabled(previousIndex)
                                && (footerDividers || !isFooter && (previousIndex < footerLimit)))) {
                        return true;
                    } else if (fillForMissingDividers) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

std::string ListView::getAccessibilityClassName()const{
    return "ListView";
}

void ListView::onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info){
    AbsListView::onInitializeAccessibilityNodeInfoInternal(info);

    const int rowsCount = getCount();
    const int selectionMode = getSelectionModeForAccessibility();
    AccessibilityNodeInfo::CollectionInfo* collectionInfo = AccessibilityNodeInfo::CollectionInfo::obtain(
            rowsCount, 1, false, selectionMode);
    info.setCollectionInfo(collectionInfo);

    if (rowsCount > 0) {
        info.addAction(AccessibilityNodeInfo::AccessibilityAction::ACTION_SCROLL_TO_POSITION.getId());
    }
}

bool ListView::performAccessibilityActionInternal(int action, Bundle* arguments){
   if (AbsListView::performAccessibilityActionInternal(action, arguments)) {
        return true;
    }

    switch (action) {
    case R::id::accessibilityActionScrollToPosition: {
        const  int row = 0;//TODO arguments.getInt(AccessibilityNodeInfo::ACTION_ARGUMENT_ROW_INT, -1);
        const int position = std::min(row, getCount() - 1);
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

void ListView::onInitializeAccessibilityNodeInfoForItem(View* view, int position, AccessibilityNodeInfo& info){
    AbsListView::onInitializeAccessibilityNodeInfoForItem(view, position, info);

    const LayoutParams* lp = (LayoutParams*) view->getLayoutParams();
    const bool isHeading = lp && lp->viewType == ITEM_VIEW_TYPE_HEADER_OR_FOOTER;
    const bool isSelected = isItemChecked(position);
    AccessibilityNodeInfo::CollectionItemInfo* itemInfo = AccessibilityNodeInfo::CollectionItemInfo::obtain(
            position, 1, 0, 1, isHeading, isSelected);
    info.setCollectionItemInfo(itemInfo);
}

HeaderViewListAdapter* ListView::wrapHeaderListAdapterInternal(
    const std::vector<FixedViewInfo*>& headerViewInfos,
    const std::vector<FixedViewInfo*>& footerViewInfos,Adapter* adapter) {
    return new HeaderViewListAdapter(headerViewInfos, footerViewInfos, adapter);
}

void ListView::wrapHeaderListAdapterInternal() {
    mAdapter = wrapHeaderListAdapterInternal(mHeaderViewInfos, mFooterViewInfos, mAdapter);
}

}//namespace

