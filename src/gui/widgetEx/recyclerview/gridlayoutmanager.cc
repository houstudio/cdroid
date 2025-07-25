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
#include <widgetEx/recyclerview/gridlayoutmanager.h>
#include <widgetEx/recyclerview/orientationhelper.h>
#include <core/build.h>
#include <widget/R.h>

namespace cdroid{

GridLayoutManager::GridLayoutManager(Context* context,const AttributeSet& attrs)
   :LinearLayoutManager(context, attrs){
    Properties properties = getProperties(context, attrs,0,0);// defStyleAttr, defStyleRes);
    mSpanSizeLookup = new DefaultSpanSizeLookup();
    setSpanCount(properties.spanCount);
    mPendingSpanCountChange = false;
}

GridLayoutManager::GridLayoutManager(Context* context, int spanCount)
    :LinearLayoutManager(context){
    mSpanSizeLookup = new DefaultSpanSizeLookup();
    setSpanCount(spanCount);
}

GridLayoutManager::GridLayoutManager(Context* context, int spanCount,int orientation, bool reverseLayout)
    :LinearLayoutManager(context, orientation, reverseLayout){
    mSpanSizeLookup = new DefaultSpanSizeLookup();
    setSpanCount(spanCount);
}

GridLayoutManager::~GridLayoutManager(){
    delete mSpanSizeLookup;
}

void GridLayoutManager::setStackFromEnd(bool stackFromEnd) {
    LOGE_IF(stackFromEnd,"GridLayoutManager does not support stack from end. Consider using reverse layout");
    LinearLayoutManager::setStackFromEnd(false);
}

int GridLayoutManager::getRowCountForAccessibility(RecyclerView::Recycler& recycler,
        RecyclerView::State& state) {
    if (mOrientation == HORIZONTAL) {
        return std::min(mSpanCount,getItemCount());
    }
    if (state.getItemCount() < 1) {
        return 0;
    }

    // Row count is one more than the last item's row index.
    return getSpanGroupIndex(recycler, state, state.getItemCount() - 1) + 1;
}

int GridLayoutManager::getColumnCountForAccessibility(RecyclerView::Recycler& recycler,
        RecyclerView::State& state) {
    if (mOrientation == VERTICAL) {
        return std::min(mSpanCount,getItemCount());
    }
    if (state.getItemCount() < 1) {
        return 0;
    }

    // Column count is one more than the last item's column index.
    return getSpanGroupIndex(recycler, state, state.getItemCount() - 1) + 1;
}

void GridLayoutManager::onInitializeAccessibilityNodeInfoForItem(RecyclerView::Recycler& recycler,
        RecyclerView::State& state, View* host, AccessibilityNodeInfo& info) {
    ViewGroup::LayoutParams* lp = host->getLayoutParams();
    if (dynamic_cast<LayoutParams*>(lp)==nullptr) {
        LinearLayoutManager::onInitializeAccessibilityNodeInfoForItem(host, info);
        return;
    }
    LayoutParams* glp = (LayoutParams*) lp;
    int spanGroupIndex = getSpanGroupIndex(recycler, state, glp->getViewLayoutPosition());
    if (mOrientation == HORIZONTAL) {
        info.setCollectionItemInfo(AccessibilityNodeInfo::CollectionItemInfo::obtain(
                glp->getSpanIndex(), glp->getSpanSize(), spanGroupIndex, 1, false, false));
    } else { // VERTICAL
        info.setCollectionItemInfo(AccessibilityNodeInfo::CollectionItemInfo::obtain(
                spanGroupIndex , 1, glp->getSpanIndex(), glp->getSpanSize(),false, false));
    }
}

void GridLayoutManager::onInitializeAccessibilityNodeInfo(RecyclerView::Recycler& recycler,
            RecyclerView::State& state, AccessibilityNodeInfo& info){
    LinearLayoutManager::onInitializeAccessibilityNodeInfo(recycler, state, info);
    // Set the class name so this is treated as a grid. A11y services should identify grids
    // and list via CollectionInfos, but an almost empty grid may be incorrectly identified
    // as a list.
    info.setClassName("GridView");

    if (mRecyclerView->mAdapter != nullptr && mRecyclerView->mAdapter->getItemCount() > 1) {
        //info.addAction(AccessibilityAction::ACTION_SCROLL_IN_DIRECTION);
    }
}

static std::set<int>sSupportedDirectionsForActionScrollInDirection={
    View::FOCUS_LEFT,View::FOCUS_RIGHT,View::FOCUS_UP,View::FOCUS_DOWN
};

bool GridLayoutManager::performAccessibilityAction(int action,Bundle* args){
    // TODO (267511848): when U constants are finalized:
    //  - convert if/else blocks to switch statement
    //  - remove SDK check
    //  - remove the -1 check (this check makes accessibilityActionScrollInDirection
    //  no-op for < 34; see action definition in AccessibilityNodeInfoCompat.java).
    if (action == -1//AccessibilityNodeInfo::AccessibilityAction::ACTION_SCROLL_IN_DIRECTION.getId()
            && action != -1) {
        View* viewWithAccessibilityFocus = findChildWithAccessibilityFocus();
        if (viewWithAccessibilityFocus == nullptr) {
            // TODO(b/268487724#comment2): handle rare cases when the requesting service does
            //  not place accessibility focus on a child. Consider scrolling forward/backward?
            return false;
        }

        // Direction must be specified.
        if (args == nullptr) {
            return false;
        }

        const int direction = -1;//args.getInt( AccessibilityNodeInfo::ACTION_ARGUMENT_DIRECTION_INT, INVALID_POSITION);

        if (!sSupportedDirectionsForActionScrollInDirection.count/*contains*/(direction)) {
            if (_Debug) {
                LOGW("Direction equals which is unsupported when using ACTION_SCROLL_IN_DIRECTION",direction);
            }
            return false;
        }

        RecyclerView::ViewHolder* vh =  mRecyclerView->getChildViewHolder(viewWithAccessibilityFocus);
        if (vh == nullptr) {
            /*if (_Debug) {
                throw new RuntimeException(
                        "viewHolder is null for " + viewWithAccessibilityFocus);
            }*/
            return false;
        }

        int startingAdapterPosition = vh->getAbsoluteAdapterPosition();
        int startingRow = getRowIndex(startingAdapterPosition);
        int startingColumn = getColumnIndex(startingAdapterPosition);

        if (startingRow < 0 || startingColumn < 0) {
            /*if (_Debug) {
                throw new RuntimeException("startingRow equals " + startingRow + ", and "
                        + "startingColumn equals " + startingColumn + ", and neither can be "
                        + "less than 0.");
            }*/
            return false;
        }

        if (hasAccessibilityFocusChanged(startingAdapterPosition)) {
            mRowWithAccessibilityFocus = startingRow;
            mColumnWithAccessibilityFocus = startingColumn;
        }

        int scrollTargetPosition;

        int row = (mRowWithAccessibilityFocus == INVALID_POSITION) ? startingRow : mRowWithAccessibilityFocus;
        int column = (mColumnWithAccessibilityFocus == INVALID_POSITION) ? startingColumn : mColumnWithAccessibilityFocus;

        switch (direction) {
        case View::FOCUS_LEFT:
            scrollTargetPosition = findScrollTargetPositionOnTheLeft(row, column, startingAdapterPosition);
            break;
        case View::FOCUS_RIGHT:
            scrollTargetPosition = findScrollTargetPositionOnTheRight(row, column, startingAdapterPosition);
            break;
        case View::FOCUS_UP:
            scrollTargetPosition = findScrollTargetPositionAbove(row, column, startingAdapterPosition);
            break;
        case View::FOCUS_DOWN:
            scrollTargetPosition = findScrollTargetPositionBelow(row, column, startingAdapterPosition);
            break;
        default:
            return false;
        }

        if (scrollTargetPosition == INVALID_POSITION  && mOrientation == RecyclerView::HORIZONTAL) {
            // TODO (b/268487724): handle RTL.
            // Handle case in grids with horizontal orientation where the scroll target is on
            // a different row.
            if (direction == View::FOCUS_LEFT) {
                scrollTargetPosition = findPositionOfLastItemOnARowAboveForHorizontalGrid(
                        startingRow);
            } else if (direction == View::FOCUS_RIGHT) {
                scrollTargetPosition = findPositionOfFirstItemOnARowBelowForHorizontalGrid(
                        startingRow);
            }
        }

        if (scrollTargetPosition != INVALID_POSITION) {
            scrollToPosition(scrollTargetPosition);
            mPositionTargetedByScrollInDirection = scrollTargetPosition;
            return true;
        }

        return false;
    } else if (action == R::id::accessibilityActionScrollToPosition) {
        int noRow = -1;
        int noColumn = -1;
        if (args != nullptr) {
            int rowArg = -1;//args.getInt(AccessibilityNodeInfo::ACTION_ARGUMENT_ROW_INT, noRow);
            int columnArg = -1;//args.getInt(AccessibilityNodeInfo::ACTION_ARGUMENT_COLUMN_INT, noColumn);

            if (rowArg == noRow || columnArg == noColumn) {
                return false;
            }

            int itemCount = mRecyclerView->mAdapter->getItemCount();

            int position = -1;
            for (int i = 0; i < itemCount; i++) {
                // Corresponds to a column value if the orientation is VERTICAL and a row value
                // if the orientation is HORIZONTAL
                int spanIndex = getSpanIndex(*mRecyclerView->mRecycler, *mRecyclerView->mState, i);

                // Corresponds to a row value if the orientation is VERTICAL and a column value
                // if the orientation is HORIZONTAL
                int spanGroupIndex = getSpanGroupIndex(*mRecyclerView->mRecycler, *mRecyclerView->mState, i);

                if (mOrientation == VERTICAL) {
                    if (spanIndex == columnArg && spanGroupIndex == rowArg) {
                        position = i;
                        break;
                    }
                } else { // horizontal
                    if (spanIndex == rowArg && spanGroupIndex == columnArg) {
                        position = i;
                        break;
                    }
                }
            }

            if (position > -1) {
                scrollToPositionWithOffset(position, 0);
                return true;
            }
            return false;
        }
    }
    return LinearLayoutManager::performAccessibilityAction(action,args);
}

int GridLayoutManager::findScrollTargetPositionOnTheRight(int startingRow, int startingColumn, int startingAdapterPosition){
    int scrollTargetPosition = INVALID_POSITION;
    for (int i = startingAdapterPosition + 1; i < getItemCount(); i++) {
        int currentRow = getRowIndex(i);
        int currentColumn = getColumnIndex(i);

        if (currentRow < 0 || currentColumn < 0) {
            /*if (_Debug) {
                throw new RuntimeException("currentRow equals " + currentRow + ", and "
                        + "currentColumn equals " + currentColumn + ", and neither can be "
                        + "less than 0.");
            }*/
            return INVALID_POSITION;
        }

        if (mOrientation == VERTICAL) {
            /*
             * For grids with vertical orientation...
             * 1   2   3
             * 4   5   5
             * 6   7
             * ... the scroll target may lie on the same or a following row.
             */
            // TODO (b/268487724): handle RTL.
            if ((currentRow == startingRow && currentColumn > startingColumn)
                    || (currentRow > startingRow)) {
                mRowWithAccessibilityFocus = currentRow;
                mColumnWithAccessibilityFocus = currentColumn;
                return i;
            }
        } else { // HORIZONTAL
            /*
             * For grids with horizontal orientation, the scroll target may span multiple
             * rows. For example, in this grid...
             * 1   4   6
             * 2   5   7
             * 3   5   8
             * ... moving from 3 to 5 is considered staying on the "same row" because 5 spans
             *  multiple rows and the row indices for 5 include 3's row.
             */
            if (currentColumn > startingColumn && getRowIndices(i).count/*contains*/(startingRow)) {
                // Note: mRowWithAccessibilityFocus not updated since the scroll target is on
                // the same row.
                mColumnWithAccessibilityFocus = currentColumn;
                return i;
            }
        }
    }

    return scrollTargetPosition;
}

int GridLayoutManager::findScrollTargetPositionOnTheLeft(int startingRow, int startingColumn, int startingAdapterPosition){
    int scrollTargetPosition = INVALID_POSITION;
    for (int i = startingAdapterPosition - 1; i >= 0; i--) {
        int currentRow = getRowIndex(i);
        int currentColumn = getColumnIndex(i);

        if (currentRow < 0 || currentColumn < 0) {
            /*if (_Debug) {
                throw new RuntimeException("currentRow equals " + currentRow + ", and "
                        + "currentColumn equals " + currentColumn + ", and neither can be "
                        + "less than 0.");
            }*/
            return INVALID_POSITION;
        }

        if (mOrientation == VERTICAL) {
            /*
             * For grids with vertical orientation...
             * 1   2   3
             * 4   5   5
             * 6   7
             * ... the scroll target may lie on the same or a preceding row.
             */
            // TODO (b/268487724): handle RTL.
            if ((currentRow == startingRow && currentColumn < startingColumn)
                    || (currentRow < startingRow)) {
                scrollTargetPosition = i;
                mRowWithAccessibilityFocus = currentRow;
                mColumnWithAccessibilityFocus = currentColumn;
                break;
            }
        } else { // HORIZONTAL
            /*
             * For grids with horizontal orientation, the scroll target may span multiple
             * rows. For example, in this grid...
             * 1   4   6
             * 2   5   7
             * 3   5   8
             * ... moving from 8 to 5 or from 7 to 5 is considered staying on the "same row"
             * because the row indices for 5 include 8's and 7's row.
             */
            if (getRowIndices(i).count/*contains*/(startingRow) && currentColumn < startingColumn) {
                // Note: mRowWithAccessibilityFocus not updated since the scroll target is on
                // the same row.
                mColumnWithAccessibilityFocus = currentColumn;
                return i;
            }
        }
    }
    return scrollTargetPosition;
}

int GridLayoutManager::findScrollTargetPositionAbove(int startingRow, int startingColumn, int startingAdapterPosition){
    int scrollTargetPosition = INVALID_POSITION;
    for (int i = startingAdapterPosition - 1; i >= 0; i--) {
        int currentRow = getRowIndex(i);
        int currentColumn = getColumnIndex(i);

        if (currentRow < 0 || currentColumn < 0) {
            /*if (_Debug) {
                throw new RuntimeException("currentRow equals " + currentRow + ", and "
                        + "currentColumn equals " + currentColumn + ", and neither can be "
                        + "less than 0.");
            }*/
            return INVALID_POSITION;
        }

        if (mOrientation == VERTICAL) {
            /*
             * The scroll target may span multiple columns. For example, in this grid...
             * 1   2   3
             * 4   4   5
             * 6   7
             * ... moving from 7 to 4 interprets as staying in second column, and moving from
             * 6 to 4 interprets as staying in the first column.
             */
            if (currentRow < startingRow && getColumnIndices(i).count/*contains*/(startingColumn)) {
                scrollTargetPosition = i;
                mRowWithAccessibilityFocus = currentRow;
                // Note: mColumnWithAccessibilityFocus not updated since the scroll target is on
                // the same column.
                break;
            }
        } else { // HORIZONTAL
            /*
             * The scroll target may span multiple rows. In this grid...
             * 1   4
             * 2   5
             * 2
             * 3
             * ... 2 spans two rows and moving up from 3 to 2 interprets moving to the third
             * row.
             */
            if (currentRow < startingRow && currentColumn == startingColumn) {
                std::set<int> rowIndices = getRowIndices(i);
                scrollTargetPosition = i;
                mRowWithAccessibilityFocus = *rowIndices.rbegin();//Collections.max(rowIndices);
                // Note: mColumnWithAccessibilityFocus not updated since the scroll target is on
                // the same column.
                break;
            }
        }
    }
    return scrollTargetPosition;
}

int GridLayoutManager::findScrollTargetPositionBelow(int startingRow, int startingColumn, int startingAdapterPosition){
    int scrollTargetPosition = INVALID_POSITION;
    for (int i = startingAdapterPosition + 1; i < getItemCount(); i++) {
        int currentRow = getRowIndex(i);
        int currentColumn = getColumnIndex(i);

        if (currentRow < 0 || currentColumn < 0) {
            /*if (_Debug) {
                throw new RuntimeException("currentRow equals " + currentRow + ", and "
                        + "currentColumn equals " + currentColumn + ", and neither can be "
                        + "less than 0.");
            }*/
            return INVALID_POSITION;
        }

        if (mOrientation == VERTICAL) {
            /*
             * The scroll target may span multiple columns. For example, in this grid...
             * 1   2   3
             * 4   4   5
             * 6   7
             * ... moving from 2 to 4 interprets as staying in second column, and moving from
             * 1 to 4 interprets as staying in the first column.
             */
            if ((currentRow > startingRow) && (currentColumn == startingColumn
                    || getColumnIndices(i).count/*contains*/(startingColumn))) {
                scrollTargetPosition = i;
                mRowWithAccessibilityFocus = currentRow;
                break;
            }
        } else { // HORIZONTAL
            /*
             * The scroll target may span multiple rows. In this grid...
             * 1   4
             * 2   5
             * 2
             * 3
             * ... 2 spans two rows and moving down from 1 to 2 interprets moving to the second
             * row.
             */
            if (currentRow > startingRow && currentColumn == startingColumn) {
                scrollTargetPosition = i;
                mRowWithAccessibilityFocus = getRowIndex(i);
                break;
            }
        }
    }
    return scrollTargetPosition;
}

int GridLayoutManager::getRowIndex(int position){
    return mOrientation == VERTICAL ? getSpanGroupIndex(*mRecyclerView->mRecycler, *mRecyclerView->mState, position)
              : getSpanIndex(*mRecyclerView->mRecycler, *mRecyclerView->mState, position);
}

int GridLayoutManager::getColumnIndex(int position){
    return mOrientation == HORIZONTAL ? getSpanGroupIndex(*mRecyclerView->mRecycler, *mRecyclerView->mState, position)
                : getSpanIndex(*mRecyclerView->mRecycler,*mRecyclerView->mState, position);
}

std::set<int> GridLayoutManager::getRowIndices(int position){
    return getRowOrColumnIndices(getRowIndex(position), position);
}

std::set<int> GridLayoutManager::getColumnIndices(int position){
    return getRowOrColumnIndices(getColumnIndex(position), position);
}

std::set<int> GridLayoutManager::getRowOrColumnIndices(int rowOrColumnIndex, int position){
    std::set<int> indices;
    int spanSize = getSpanSize(*mRecyclerView->mRecycler, *mRecyclerView->mState, position);
    for (int i = rowOrColumnIndex;  i <  rowOrColumnIndex + spanSize; i++) {
        indices.insert(i);
    }
    return indices;
}

View* GridLayoutManager::findChildWithAccessibilityFocus(){
    View* child = nullptr;
    // SDK check needed for View#isAccessibilityFocused()
    if (Build::VERSION::SDK_INT >= Build::VERSION_CODES::LOLLIPOP) {
        bool childFound = false;
        int i;
        for (i = 0; i < getChildCount(); i++) {
            View*view = getChildAt(i);
            if (view&&view->isAccessibilityFocused()) {
                childFound = true;
                break;
            }
        }
        if (childFound) {
            child = getChildAt(i);
        }
    }
    return child;
}

bool GridLayoutManager::hasAccessibilityFocusChanged(int adapterPosition){
    return !getRowIndices(adapterPosition).count/*contains*/(mRowWithAccessibilityFocus)
           || !getColumnIndices(adapterPosition).count/*contains*/(mColumnWithAccessibilityFocus);
}

void GridLayoutManager::onLayoutChildren(RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    if (state.isPreLayout()) {
        cachePreLayoutSpanMapping();
    }
    LinearLayoutManager::onLayoutChildren(recycler, state);
    if (_Debug) {
        validateChildOrder();
    }
    clearPreLayoutSpanMappingCache();
}

void GridLayoutManager::onLayoutCompleted(RecyclerView::State& state) {
    LinearLayoutManager::onLayoutCompleted(state);
    mPendingSpanCountChange = false;
    if (mPositionTargetedByScrollInDirection != INVALID_POSITION) {
        View* viewTargetedByScrollInDirection = findViewByPosition(
                mPositionTargetedByScrollInDirection);
        if (viewTargetedByScrollInDirection != nullptr) {
            // Send event after the scroll associated with ACTION_SCROLL_IN_DIRECTION (see
            // performAccessibilityAction()) concludes and layout completes. Accessibility
            // services can listen for this event and change UI state as needed.
            //viewTargetedByScrollInDirection->sendAccessibilityEvent(AccessibilityEvent::TYPE_VIEW_TARGETED_BY_SCROLL);
            mPositionTargetedByScrollInDirection = INVALID_POSITION;
        }
    }
}

void GridLayoutManager::clearPreLayoutSpanMappingCache() {
    mPreLayoutSpanSizeCache.clear();
    mPreLayoutSpanIndexCache.clear();
}

void GridLayoutManager::cachePreLayoutSpanMapping() {
    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        LayoutParams* lp = (LayoutParams*) getChildAt(i)->getLayoutParams();
        const int viewPosition = lp->getViewLayoutPosition();
        mPreLayoutSpanSizeCache.put(viewPosition, lp->getSpanSize());
        mPreLayoutSpanIndexCache.put(viewPosition, lp->getSpanIndex());
    }
}

void GridLayoutManager::onItemsAdded(RecyclerView& recyclerView, int positionStart, int itemCount) {
    mSpanSizeLookup->invalidateSpanIndexCache();
    mSpanSizeLookup->invalidateSpanGroupIndexCache();
}

void GridLayoutManager::onItemsChanged(RecyclerView& recyclerView) {
    mSpanSizeLookup->invalidateSpanIndexCache();
    mSpanSizeLookup->invalidateSpanGroupIndexCache();
}

void GridLayoutManager::onItemsRemoved(RecyclerView& recyclerView, int positionStart, int itemCount) {
    mSpanSizeLookup->invalidateSpanIndexCache();
    mSpanSizeLookup->invalidateSpanGroupIndexCache();
}

void GridLayoutManager::onItemsUpdated(RecyclerView& recyclerView, int positionStart, int itemCount,
        Object* payload) {
    mSpanSizeLookup->invalidateSpanIndexCache();
    mSpanSizeLookup->invalidateSpanGroupIndexCache();
}

void GridLayoutManager::onItemsMoved(RecyclerView& recyclerView, int from, int to, int itemCount) {
    mSpanSizeLookup->invalidateSpanIndexCache();
}

GridLayoutManager::LayoutParams* GridLayoutManager::generateDefaultLayoutParams()const {
    if (mOrientation == HORIZONTAL) {
        return new LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::MATCH_PARENT);
    } else {
        return new LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT);
    }
}

GridLayoutManager::LayoutParams* GridLayoutManager::generateLayoutParams(Context* c,const AttributeSet& attrs)const{
    return new LayoutParams(c, attrs);
}

GridLayoutManager::LayoutParams* GridLayoutManager::generateLayoutParams(const ViewGroup::LayoutParams& lp)const{
    if (dynamic_cast<const ViewGroup::MarginLayoutParams*>(&lp)) {
        return new LayoutParams((ViewGroup::MarginLayoutParams&) lp);
    } else {
        return new LayoutParams(lp);
    }
}

bool GridLayoutManager::checkLayoutParams(const RecyclerView::LayoutParams* lp)const {
    return dynamic_cast<const LayoutParams*>(lp);
}

void GridLayoutManager::setSpanSizeLookup(SpanSizeLookup* spanSizeLookup) {
    mSpanSizeLookup = spanSizeLookup;
}

GridLayoutManager::SpanSizeLookup* GridLayoutManager::getSpanSizeLookup() {
    return mSpanSizeLookup;
}

void GridLayoutManager::updateMeasurements() {
    int totalSpace;
    if (getOrientation() == VERTICAL) {
        totalSpace = getWidth() - getPaddingRight() - getPaddingLeft();
    } else {
        totalSpace = getHeight() - getPaddingBottom() - getPaddingTop();
    }
    calculateItemBorders(totalSpace);
}

void GridLayoutManager::setMeasuredDimension(Rect& childrenBounds, int wSpec, int hSpec) {
    if (mCachedBorders.empty()){// == null) {
	LinearLayoutManager::setMeasuredDimension(childrenBounds, wSpec, hSpec);
    }
    int width, height;
    const int horizontalPadding = getPaddingLeft() + getPaddingRight();
    const int verticalPadding = getPaddingTop() + getPaddingBottom();
    if (mOrientation == VERTICAL) {
        const int usedHeight = childrenBounds.height + verticalPadding;
        height = chooseSize(hSpec, usedHeight, getMinimumHeight());
        width = chooseSize(wSpec, mCachedBorders[mCachedBorders.size() - 1] + horizontalPadding,
                getMinimumWidth());
    } else {
        const int usedWidth = childrenBounds.width + horizontalPadding;
        width = chooseSize(wSpec, usedWidth, getMinimumWidth());
        height = chooseSize(hSpec, mCachedBorders[mCachedBorders.size() - 1] + verticalPadding,
                getMinimumHeight());
    }
    LinearLayoutManager::setMeasuredDimension(width, height);
}

int GridLayoutManager::findPositionOfLastItemOnARowAboveForHorizontalGrid(int startingRow){
    if (startingRow < 0) {
        /*if (_Debug) {
            throw new RuntimeException(
                    "startingRow equals " + startingRow + ". It cannot be less than zero");
        }*/
        return INVALID_POSITION;
    }

    if (mOrientation == VERTICAL) {
        // This only handles cases of grids with horizontal orientation.
        /*if (_Debug) {
            Log.w(TAG, "You should not "
                    + "use findPositionOfLastItemOnARowAboveForHorizontalGrid(...) with grids "
                    + "with VERTICAL orientation");
        }*/
        return INVALID_POSITION;
    }

    // Map where the keys are row numbers and values are the adapter positions of the last
    // item in each row. This map is used to locate a scroll target on a previous row in grids
    // with horizontal orientation. In this example...
    // 1   4   7
    // 2   5   8
    // 3   6
    // ... the generated map - {2 -> 5, 1 -> 7, 0 -> 6} - can be used to scroll from,
    // say, "2" (adapter position 1) in the second row to "7" (adapter position 6) in the
    // preceding row.
    //
    // Sometimes cells span multiple rows. In this example:
    // 1   4   7
    // 2   5   7
    // 3   6   8
    // ... the generated map - {0 -> 6, 1 -> 6, 2 -> 7} - can be used to scroll left from,
    // say, "3" (adapter position 2) in the third row to "7" (adapter position 6) on the
    // second row, and then to "5" (adapter position 4).
    std::map<int,int> rowToLastItemPositionMap;
    for (int position = 0; position < getItemCount(); position++) {
        std::set<int> rows = getRowIndices(position);
        for (int row: rows) {
            if (row < 0) {
                /*if (DEBUG) {
                    throw new RuntimeException(
                            "row equals " + row + ". It cannot be less than zero");
                }*/
                return INVALID_POSITION;
            }
            rowToLastItemPositionMap.insert({row, position});
        }
    }

    for (auto it : rowToLastItemPositionMap/*.keySet()*/) {
        if (it.first/*row*/ < startingRow) {
            const int scrollTargetPosition = it.second;//rowToLastItemPositionMap.get(row);
            mRowWithAccessibilityFocus = it.first/*row*/;
            mColumnWithAccessibilityFocus = getColumnIndex(scrollTargetPosition);
            return scrollTargetPosition;
        }
    }
    return INVALID_POSITION;
}

int GridLayoutManager::findPositionOfFirstItemOnARowBelowForHorizontalGrid(int startingRow){
    if (startingRow < 0) {
        /*if (DEBUG) {
            throw new RuntimeException(
                    "startingRow equals " + startingRow + ". It cannot be less than zero");
        }*/
        return INVALID_POSITION;
    }

    if (mOrientation == VERTICAL) {
        // This only handles cases of grids with horizontal orientation.
        /*if (_Debug) {
            LOGW"You should not "
                    + "use findPositionOfFirstItemOnARowBelowForHorizontalGrid(...) with grids "
                    + "with VERTICAL orientation");
        }*/
        return INVALID_POSITION;
    }

    // Map where the keys are row numbers and values are the adapter positions of the first
    // item in each row. This map is used to locate a scroll target on a following row in grids
    // with horizontal orientation. In this example:
    // 1   4   7
    // 2   5   8
    // 3   6
    // ... the generated map - {0 -> 0, 1 -> 1, 2 -> 2} - can be used to scroll from, say,
    // "7" (adapter position 6) in the first row to "2" (adapter position 1) in the next row.
    // Sometimes cells span multiple rows. In this example:
    // 1   3   6
    // 1   4   7
    // 2   5   8
    // ... the generated map - {0 -> 0, 1 -> 0, 2 -> 1} - can be used to scroll right from,
    // say, "6" (adapter position 5) in the first row to "1" (adapter position 0) on the
    // second row, and then to "4" (adapter position 3).
    std::map<int,int> rowToFirstItemPositionMap;
    for (int position = 0; position < getItemCount(); position++) {
        std::set<int> rows = getRowIndices(position);
        for (int row : rows) {
            if (row < 0) {
                /*if (DEBUG) {
                    throw new RuntimeException(
                            "row equals " + row + ". It cannot be less than zero");
                }*/
                return INVALID_POSITION;
            }
            // We only care about the first item on each row.
            if (rowToFirstItemPositionMap.find(row)==rowToFirstItemPositionMap.end()){
                rowToFirstItemPositionMap.insert({row, position});
            }
        }
    }

    for (auto it : rowToFirstItemPositionMap) {
        if (it.first/*row*/ > startingRow) {
            const int scrollTargetPosition = it.second;//rowToFirstItemPositionMap.get(row);
            mRowWithAccessibilityFocus = it.first/*row*/;
            mColumnWithAccessibilityFocus = 0;
            return scrollTargetPosition;
        }
    }
    return INVALID_POSITION;
}

void GridLayoutManager::calculateItemBorders(int totalSpace) {
    //mCachedBorders = calculateItemBorders(mCachedBorders, mSpanCount, totalSpace);
    calculateItemBorders(mCachedBorders, mSpanCount, totalSpace);
}

int* GridLayoutManager::calculateItemBorders(std::vector<int>& cachedBorders, int spanCount, int totalSpace) {
    if (cachedBorders.empty() || cachedBorders.size() != spanCount + 1
            || cachedBorders[cachedBorders.size() - 1] != totalSpace) {
        cachedBorders.resize(spanCount +1);// = new int[spanCount + 1];
    }
    cachedBorders[0] = 0;
    int sizePerSpan = totalSpace / spanCount;
    int sizePerSpanRemainder = totalSpace % spanCount;
    int consumedPixels = 0;
    int additionalSize = 0;
    for (int i = 1; i <= spanCount; i++) {
        int itemSize = sizePerSpan;
        additionalSize += sizePerSpanRemainder;
        if (additionalSize > 0 && (spanCount - additionalSize) < sizePerSpanRemainder) {
            itemSize += 1;
            additionalSize -= spanCount;
        }
        consumedPixels += itemSize;
        cachedBorders[i] = consumedPixels;
    }
    return cachedBorders.data();
}

int GridLayoutManager::getSpaceForSpanRange(int startSpan, int spanSize) {
    if (mOrientation == VERTICAL && isLayoutRTL()) {
        return mCachedBorders[mSpanCount - startSpan]
                - mCachedBorders[mSpanCount - startSpan - spanSize];
    } else {
        return mCachedBorders[startSpan + spanSize] - mCachedBorders[startSpan];
    }
}

void GridLayoutManager::onAnchorReady(RecyclerView::Recycler& recycler, RecyclerView::State& state,
                   AnchorInfo& anchorInfo, int itemDirection) {
    LinearLayoutManager::onAnchorReady(recycler, state, anchorInfo, itemDirection);
    updateMeasurements();
    if (state.getItemCount() > 0 && !state.isPreLayout()) {
        ensureAnchorIsInCorrectSpan(recycler, state, anchorInfo, itemDirection);
    }
    ensureViewSet();
}

void GridLayoutManager::ensureViewSet() {
    if (mSet.empty() || mSet.size() != mSpanCount) {
        mSet.resize(mSpanCount);// = new View[mSpanCount];
    }
}

int GridLayoutManager::scrollHorizontallyBy(int dx, RecyclerView::Recycler& recycler,
        RecyclerView::State& state) {
    updateMeasurements();
    ensureViewSet();
    return LinearLayoutManager::scrollHorizontallyBy(dx, recycler, state);
}

int GridLayoutManager::scrollVerticallyBy(int dy, RecyclerView::Recycler& recycler,
        RecyclerView::State& state) {
    updateMeasurements();
    ensureViewSet();
    return LinearLayoutManager::scrollVerticallyBy(dy, recycler, state);
}

void GridLayoutManager::ensureAnchorIsInCorrectSpan(RecyclerView::Recycler& recycler,
        RecyclerView::State& state, AnchorInfo& anchorInfo, int itemDirection) {
    const bool layingOutInPrimaryDirection =
            itemDirection == LayoutState::ITEM_DIRECTION_TAIL;
    int span = getSpanIndex(recycler, state, anchorInfo.mPosition);
    if (layingOutInPrimaryDirection) {
        // choose span 0
        while (span > 0 && anchorInfo.mPosition > 0) {
            anchorInfo.mPosition--;
            span = getSpanIndex(recycler, state, anchorInfo.mPosition);
        }
    } else {
        // choose the max span we can get. hopefully last one
        const int indexLimit = state.getItemCount() - 1;
        int pos = anchorInfo.mPosition;
        int bestSpan = span;
        while (pos < indexLimit) {
            int next = getSpanIndex(recycler, state, pos + 1);
            if (next > bestSpan) {
                pos += 1;
                bestSpan = next;
            } else {
                break;
            }
        }
        anchorInfo.mPosition = pos;
    }
}

View* GridLayoutManager::findReferenceChild(RecyclerView::Recycler& recycler, RecyclerView::State& state,
        bool layoutFromEnd, bool traverseChildrenInReverseOrder) {
    int start = 0;
    int end = getChildCount();
    int diff = 1;
    if (traverseChildrenInReverseOrder) {
        start = getChildCount() - 1;
        end = -1;
        diff = -1;
    }

    int itemCount = state.getItemCount();

    ensureLayoutState();
    View* invalidMatch = nullptr;
    View* outOfBoundsMatch = nullptr;

    const int boundsStart = mOrientationHelper->getStartAfterPadding();
    const int boundsEnd = mOrientationHelper->getEndAfterPadding();

    for (int i = start; i != end; i += diff) {
        View* view = getChildAt(i);
        const int position = getPosition(view);
        if (position >= 0 && position < itemCount) {
            const int span = getSpanIndex(recycler, state, position);
            if (span != 0) {
                continue;
            }
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

int GridLayoutManager::getSpanGroupIndex(RecyclerView::Recycler& recycler, RecyclerView::State& state,
        int viewPosition) {
    if (!state.isPreLayout()) {
        return mSpanSizeLookup->getSpanGroupIndex(viewPosition, mSpanCount);
    }
    const int adapterPosition = recycler.convertPreLayoutPositionToPostLayout(viewPosition);
    if (adapterPosition == -1) {
        if (_Debug) {
            LOGD("Cannot find span group index for position %d",viewPosition);
        }
        LOGW("Cannot find span size for pre layout position. ",viewPosition);
        return 0;
    }
    return mSpanSizeLookup->getSpanGroupIndex(adapterPosition, mSpanCount);
}

int GridLayoutManager::getSpanIndex(RecyclerView::Recycler& recycler, RecyclerView::State& state, int pos) {
    if (!state.isPreLayout()) {
        return mSpanSizeLookup->getCachedSpanIndex(pos, mSpanCount);
    }
    const int cached = mPreLayoutSpanIndexCache.get(pos, -1);
    if (cached != -1) {
        return cached;
    }
    const int adapterPosition = recycler.convertPreLayoutPositionToPostLayout(pos);
    if (adapterPosition == -1) {
        if (_Debug) {
            LOGD("Cannot find span index for pre layout position. It is"
                   " not cached, not in the adapter. Pos:%d",pos);
        }
        LOGW("Cannot find span size for pre layout position. It is"
               " not cached, not in the adapter. Pos:%d",pos);
        return 0;
    }
    return mSpanSizeLookup->getCachedSpanIndex(adapterPosition, mSpanCount);
}

int GridLayoutManager::getSpanSize(RecyclerView::Recycler& recycler, RecyclerView::State& state, int pos) {
    if (!state.isPreLayout()) {
        return mSpanSizeLookup->getSpanSize(pos);
    }
    const int cached = mPreLayoutSpanSizeCache.get(pos, -1);
    if (cached != -1) {
        return cached;
    }
    const int adapterPosition = recycler.convertPreLayoutPositionToPostLayout(pos);
    if (adapterPosition == -1) {
        if (_Debug) {
            LOGD("Cannot find span size for pre layout position. It is"
                 " not cached, not in the adapter. Pos:%d",pos);
        }
        LOGW("Cannot find span size for pre layout position. It is"
               " not cached, not in the adapter. Pos:",pos);
        return 1;
    }
    return mSpanSizeLookup->getSpanSize(adapterPosition);
}

void GridLayoutManager::collectPrefetchPositionsForLayoutState(RecyclerView::State& state, LayoutState& layoutState,
        LayoutPrefetchRegistry& layoutPrefetchRegistry) {
    int remainingSpan = mSpanCount;
    int count = 0;
    while (count < mSpanCount && layoutState.hasMore(state) && remainingSpan > 0) {
        const int pos = layoutState.mCurrentPosition;
        layoutPrefetchRegistry.addPosition(pos, std::max(0, layoutState.mScrollingOffset));
        const int spanSize = mSpanSizeLookup->getSpanSize(pos);
        remainingSpan -= spanSize;
        layoutState.mCurrentPosition += layoutState.mItemDirection;
        count++;
    }
}

void GridLayoutManager::layoutChunk(RecyclerView::Recycler& recycler, RecyclerView::State& state,
        LayoutState& layoutState, LayoutChunkResult& result) {
    const int otherDirSpecMode = mOrientationHelper->getModeInOther();
    const bool flexibleInOtherDir = otherDirSpecMode != MeasureSpec::EXACTLY;
    const int currentOtherDirSize = getChildCount() > 0 ? mCachedBorders[mSpanCount] : 0;
    // if grid layout's dimensions are not specified, let the new row change the measurements
    // This is not perfect since we not covering all rows but still solves an important case
    // where they may have a header row which should be laid out according to children.
    if (flexibleInOtherDir) {
        updateMeasurements(); //  reset measurements
    }
    const bool layingOutInPrimaryDirection =
            layoutState.mItemDirection == LayoutState::ITEM_DIRECTION_TAIL;
    int count = 0;
    int consumedSpanCount = 0;
    int remainingSpan = mSpanCount;
    if (!layingOutInPrimaryDirection) {
        int itemSpanIndex = getSpanIndex(recycler, state, layoutState.mCurrentPosition);
        int itemSpanSize = getSpanSize(recycler, state, layoutState.mCurrentPosition);
        remainingSpan = itemSpanIndex + itemSpanSize;
    }
    while (count < mSpanCount && layoutState.hasMore(state) && remainingSpan > 0) {
        int pos = layoutState.mCurrentPosition;
        const int spanSize = getSpanSize(recycler, state, pos);
        if (spanSize > mSpanCount) {
            FATAL("Item at position %d requires %d spans but GridLayoutManager has only %d spans.",pos,spanSize,mSpanCount);
        }
        remainingSpan -= spanSize;
        if (remainingSpan < 0) {
            break; // item did not fit into this row or column
        }
        View* view = layoutState.next(recycler);
        if (view == nullptr) {
            break;
        }
        consumedSpanCount += spanSize;
        mSet[count] = view;
        count++;
    }

    if (count == 0) {
        result.mFinished = true;
        return;
    }

    int maxSize = 0;
    float maxSizeInOther = 0; // use a float to get size per span

    // we should assign spans before item decor offsets are calculated
    assignSpans(recycler, state, count, consumedSpanCount, layingOutInPrimaryDirection);
    for (int i = 0; i < count; i++) {
        View* view = mSet[i];
        if (layoutState.mScrapList.empty()){// == null) {
            if (layingOutInPrimaryDirection) {
                addView(view);
            } else {
                addView(view, 0);
            }
        } else {
            if (layingOutInPrimaryDirection) {
                addDisappearingView(view);
            } else {
                addDisappearingView(view, 0);
            }
        }
        calculateItemDecorationsForChild(view, mDecorInsets);

        measureChild(view, otherDirSpecMode, false);
        const int size = mOrientationHelper->getDecoratedMeasurement(view);
        if (size > maxSize) {
            maxSize = size;
        }
        const LayoutParams* lp = (LayoutParams*) view->getLayoutParams();
        const float otherSize = 1.f * mOrientationHelper->getDecoratedMeasurementInOther(view)
                / lp->mSpanSize;
        if (otherSize > maxSizeInOther) {
            maxSizeInOther = otherSize;
        }
    }
    if (flexibleInOtherDir) {
        // re-distribute columns
        guessMeasurement(maxSizeInOther, currentOtherDirSize);
        // now we should re-measure any item that was match parent.
        maxSize = 0;
        for (int i = 0; i < count; i++) {
            View* view = mSet[i];
            measureChild(view, MeasureSpec::EXACTLY, true);
            const int size = mOrientationHelper->getDecoratedMeasurement(view);
            if (size > maxSize) {
                maxSize = size;
            }
        }
    }

    // Views that did not measure the maxSize has to be re-measured
    // We will stop doing this once we introduce Gravity in the GLM layout params
    for (int i = 0; i < count; i++) {
        View* view = mSet[i];
        if (mOrientationHelper->getDecoratedMeasurement(view) != maxSize) {
            LayoutParams* lp = (LayoutParams*) view->getLayoutParams();
            Rect decorInsets = lp->mDecorInsets;
            const int verticalInsets = decorInsets.top + decorInsets.height
                    + lp->topMargin + lp->bottomMargin;
            const int horizontalInsets = decorInsets.left + decorInsets.width
                    + lp->leftMargin + lp->rightMargin;
            const int totalSpaceInOther = getSpaceForSpanRange(lp->mSpanIndex, lp->mSpanSize);
            int wSpec, hSpec;
            if (mOrientation == VERTICAL) {
                wSpec = getChildMeasureSpec(totalSpaceInOther, MeasureSpec::EXACTLY,
                        horizontalInsets, lp->width, false);
                hSpec = MeasureSpec::makeMeasureSpec(maxSize - verticalInsets,
                        MeasureSpec::EXACTLY);
            } else {
                wSpec = MeasureSpec::makeMeasureSpec(maxSize - horizontalInsets,
                        MeasureSpec::EXACTLY);
                hSpec = getChildMeasureSpec(totalSpaceInOther, MeasureSpec::EXACTLY,
                        verticalInsets, lp->height, false);
            }
            measureChildWithDecorationsAndMargin(view, wSpec, hSpec, true);
        }
    }

    result.mConsumed = maxSize;

    int left = 0, right = 0, top = 0, bottom = 0;
    if (mOrientation == VERTICAL) {
        if (layoutState.mLayoutDirection == LayoutState::LAYOUT_START) {
            bottom = layoutState.mOffset;
            top = bottom - maxSize;
        } else {
            top = layoutState.mOffset;
            bottom = top + maxSize;
        }
    } else {
        if (layoutState.mLayoutDirection == LayoutState::LAYOUT_START) {
            right = layoutState.mOffset;
            left = right - maxSize;
        } else {
            left = layoutState.mOffset;
            right = left + maxSize;
        }
    }
    for (int i = 0; i < count; i++) {
        View* view = mSet[i];
        LayoutParams* params = (LayoutParams*) view->getLayoutParams();
        if (mOrientation == VERTICAL) {
            if (isLayoutRTL()) {
                right = getPaddingLeft() + mCachedBorders[mSpanCount - params->mSpanIndex];
                left = right - mOrientationHelper->getDecoratedMeasurementInOther(view);
            } else {
                left = getPaddingLeft() + mCachedBorders[params->mSpanIndex];
                right = left + mOrientationHelper->getDecoratedMeasurementInOther(view);
            }
        } else {
            top = getPaddingTop() + mCachedBorders[params->mSpanIndex];
            bottom = top + mOrientationHelper->getDecoratedMeasurementInOther(view);
        }
        // We calculate everything with View's bounding box (which includes decor and margins)
        // To calculate correct layout position, we subtract margins.
        layoutDecoratedWithMargins(view, left, top, right-left, bottom-top);
        LOGD_IF(_Debug,"laid out child at position %d,with*%d,%d,%d,%d) span:%d, spanSize:%d",
	    getPosition(view),(left + params->leftMargin),(top + params->topMargin),(right - params->rightMargin),
	    (bottom - params->bottomMargin), params->mSpanIndex , params->mSpanSize);
        // Consume the available space if the view is not removed OR changed
        if (params->isItemRemoved() || params->isItemChanged()) {
            result.mIgnoreConsumed = true;
        }
        result.mFocusable |= view->hasFocusable();
    }
    for(auto& m:mSet)m=nullptr;//Arrays.fill(mSet, null);
}

void GridLayoutManager::measureChild(View* view, int otherDirParentSpecMode, bool alreadyMeasured) {
    const LayoutParams* lp = (const LayoutParams*) view->getLayoutParams();
    const Rect& decorInsets = lp->mDecorInsets;
    const int verticalInsets = decorInsets.top + decorInsets.height
            + lp->topMargin + lp->bottomMargin;
    const int horizontalInsets = decorInsets.left + decorInsets.width
            + lp->leftMargin + lp->rightMargin;
    const int availableSpaceInOther = getSpaceForSpanRange(lp->mSpanIndex, lp->mSpanSize);
    int wSpec, hSpec;
    if (mOrientation == VERTICAL) {
        wSpec = getChildMeasureSpec(availableSpaceInOther, otherDirParentSpecMode,
                horizontalInsets, lp->width, false);
        hSpec = getChildMeasureSpec(mOrientationHelper->getTotalSpace(), getHeightMode(),
                verticalInsets, lp->height, true);
    } else {
        hSpec = getChildMeasureSpec(availableSpaceInOther, otherDirParentSpecMode,
                verticalInsets, lp->height, false);
        wSpec = getChildMeasureSpec(mOrientationHelper->getTotalSpace(), getWidthMode(),
                horizontalInsets, lp->width, true);
    }
    measureChildWithDecorationsAndMargin(view, wSpec, hSpec, alreadyMeasured);
}

void GridLayoutManager::guessMeasurement(float maxSizeInOther, int currentOtherDirSize) {
    const int contentSize = std::round(maxSizeInOther * mSpanCount);
    // always re-calculate because borders were stretched during the fill
    calculateItemBorders(std::max(contentSize, currentOtherDirSize));
}

void GridLayoutManager::measureChildWithDecorationsAndMargin(View* child, int widthSpec, int heightSpec,
        bool alreadyMeasured) {
    RecyclerView::LayoutParams* lp = (RecyclerView::LayoutParams*) child->getLayoutParams();
    bool measure;
    if (alreadyMeasured) {
        measure = shouldReMeasureChild(child, widthSpec, heightSpec, lp);
    } else {
        measure = shouldMeasureChild(child, widthSpec, heightSpec, lp);
    }
    if (measure) {
        child->measure(widthSpec, heightSpec);
    }
}

void GridLayoutManager::assignSpans(RecyclerView::Recycler& recycler, RecyclerView::State& state, int count,
        int consumedSpanCount, bool layingOutInPrimaryDirection) {
    // spans are always assigned from 0 to N no matter if it is RTL or not.
    // RTL is used only when positioning the view.
    int span, start, end, diff;
    // make sure we traverse from min position to max position
    if (layingOutInPrimaryDirection) {
        start = 0;
        end = count;
        diff = 1;
    } else {
        start = count - 1;
        end = -1;
        diff = -1;
    }
    span = 0;
    for (int i = start; i != end; i += diff) {
        View* view = mSet[i];
        LayoutParams* params = (LayoutParams*) view->getLayoutParams();
        params->mSpanSize = getSpanSize(recycler, state, getPosition(view));
        params->mSpanIndex = span;
        span += params->mSpanSize;
    }
}

int GridLayoutManager::getSpanCount() {
    return mSpanCount;
}

void GridLayoutManager::setSpanCount(int spanCount) {
    if (spanCount == mSpanCount) {
        return;
    }
    mPendingSpanCountChange = true;
    if (spanCount < 1) {
        FATAL("Span count should be at least 1. Provided %d",spanCount);
    }
    mSpanCount = spanCount;
    mSpanSizeLookup->invalidateSpanIndexCache();
    requestLayout();
}


View* GridLayoutManager::onFocusSearchFailed(View* focused, int focusDirection,
            RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    View* prevFocusedChild = findContainingItemView(focused);
    if (prevFocusedChild == nullptr) {
        return nullptr;
    }
    LayoutParams* lp = (LayoutParams*) prevFocusedChild->getLayoutParams();
    const int prevSpanStart = lp->mSpanIndex;
    const int prevSpanEnd = lp->mSpanIndex + lp->mSpanSize;
    View* view = LinearLayoutManager::onFocusSearchFailed(focused, focusDirection, recycler, state);
    if (view == nullptr) {
        return nullptr;
    }
    // LinearLayoutManager finds the last child. What we want is the child which has the same
    // spanIndex.
    const int layoutDir = convertFocusDirectionToLayoutDirection(focusDirection);
    const bool ascend = (layoutDir == LayoutState::LAYOUT_END) != mShouldReverseLayout;
    int start, inc, limit;
    if (ascend) {
        start = getChildCount() - 1;
        inc = -1;
        limit = -1;
    } else {
        start = 0;
        inc = 1;
        limit = getChildCount();
    }
    const bool preferLastSpan = mOrientation == VERTICAL && isLayoutRTL();

    // The focusable candidate to be picked if no perfect focusable candidate is found.
    // The best focusable candidate is the one with the highest amount of span overlap with
    // the currently focused view.
    View* focusableWeakCandidate = nullptr; // somewhat matches but not strong
    int focusableWeakCandidateSpanIndex = -1;
    int focusableWeakCandidateOverlap = 0; // how many spans overlap

    // The unfocusable candidate to become visible on the screen next, if no perfect or
    // weak focusable candidates are found to receive focus next.
    // We are only interested in partially visible unfocusable views. These are views that are
    // not fully visible, that is either partially overlapping, or out-of-bounds and right below
    // or above RV's padded bounded area. The best unfocusable candidate is the one with the
    // highest amount of span overlap with the currently focused view.
    View* unfocusableWeakCandidate = nullptr; // somewhat matches but not strong
    int unfocusableWeakCandidateSpanIndex = -1;
    int unfocusableWeakCandidateOverlap = 0; // how many spans overlap

    // The span group index of the start child. This indicates the span group index of the
    // next focusable item to receive focus, if a focusable item within the same span group
    // exists. Any focusable item beyond this group index are not relevant since they
    // were already stored in the layout before onFocusSearchFailed call and were not picked
    // by the focusSearch algorithm.
    int focusableSpanGroupIndex = getSpanGroupIndex(recycler, state, start);
    for (int i = start; i != limit; i += inc) {
        int spanGroupIndex = getSpanGroupIndex(recycler, state, i);
        View* candidate = getChildAt(i);
        if (candidate == prevFocusedChild) {
            break;
        }

        if (candidate->hasFocusable() && spanGroupIndex != focusableSpanGroupIndex) {
            // We are past the allowable span group index for the next focusable item.
            // The search only continues if no focusable weak candidates have been found up
            // until this point, in order to find the best unfocusable candidate to become
            // visible on the screen next.
            if (focusableWeakCandidate != nullptr) {
                break;
            }
            continue;
        }

        const LayoutParams* candidateLp = (LayoutParams*) candidate->getLayoutParams();
        const int candidateStart = candidateLp->mSpanIndex;
        const int candidateEnd = candidateLp->mSpanIndex + candidateLp->mSpanSize;
        if (candidate->hasFocusable() && candidateStart == prevSpanStart
                && candidateEnd == prevSpanEnd) {
            return candidate; // perfect match
        }
        bool assignAsWeek = false;
        if ((candidate->hasFocusable() && focusableWeakCandidate == nullptr)
                || (!candidate->hasFocusable() && unfocusableWeakCandidate == nullptr)) {
            assignAsWeek = true;
        } else {
            int maxStart = std::max(candidateStart, prevSpanStart);
            int minEnd = std::min(candidateEnd, prevSpanEnd);
            int overlap = minEnd - maxStart;
            if (candidate->hasFocusable()) {
                if (overlap > focusableWeakCandidateOverlap) {
                    assignAsWeek = true;
                } else if (overlap == focusableWeakCandidateOverlap
                        && preferLastSpan == (candidateStart
                        > focusableWeakCandidateSpanIndex)) {
                    assignAsWeek = true;
                }
            } else if (focusableWeakCandidate == nullptr
                    && isViewPartiallyVisible(candidate, false, true)) {
                if (overlap > unfocusableWeakCandidateOverlap) {
                    assignAsWeek = true;
                } else if (overlap == unfocusableWeakCandidateOverlap
                        && preferLastSpan == (candidateStart
                                > unfocusableWeakCandidateSpanIndex)) {
                    assignAsWeek = true;
                }
            }
        }

        if (assignAsWeek) {
            if (candidate->hasFocusable()) {
                focusableWeakCandidate = candidate;
                focusableWeakCandidateSpanIndex = candidateLp->mSpanIndex;
                focusableWeakCandidateOverlap = std::min(candidateEnd, prevSpanEnd)
                        - std::max(candidateStart, prevSpanStart);
            } else {
                unfocusableWeakCandidate = candidate;
                unfocusableWeakCandidateSpanIndex = candidateLp->mSpanIndex;
                unfocusableWeakCandidateOverlap = std::min(candidateEnd, prevSpanEnd)
                        - std::max(candidateStart, prevSpanStart);
            }
        }
    }
    return (focusableWeakCandidate != nullptr) ? focusableWeakCandidate : unfocusableWeakCandidate;
}

bool GridLayoutManager::supportsPredictiveItemAnimations() {
    return mPendingSavedState == nullptr && !mPendingSpanCountChange;
}

int GridLayoutManager::computeHorizontalScrollRange(RecyclerView::State& state) {
    if (mUsingSpansToEstimateScrollBarDimensions) {
        return computeScrollRangeWithSpanInfo(state);
    } else {
        return LinearLayoutManager::computeHorizontalScrollRange(state);
    }
}

int GridLayoutManager::computeVerticalScrollRange(RecyclerView::State& state) {
    if (mUsingSpansToEstimateScrollBarDimensions) {
        return computeScrollRangeWithSpanInfo(state);
    } else {
        return LinearLayoutManager::computeVerticalScrollRange(state);
    }
}

int GridLayoutManager::computeHorizontalScrollOffset(RecyclerView::State& state) {
    if (mUsingSpansToEstimateScrollBarDimensions) {
        return computeScrollOffsetWithSpanInfo(state);
    } else {
        return LinearLayoutManager::computeHorizontalScrollOffset(state);
    }
}

int GridLayoutManager::computeVerticalScrollOffset(RecyclerView::State& state) {
    if (mUsingSpansToEstimateScrollBarDimensions) {
        return computeScrollOffsetWithSpanInfo(state);
    } else {
        return LinearLayoutManager::computeVerticalScrollOffset(state);
    }
}

void GridLayoutManager::setUsingSpansToEstimateScrollbarDimensions(bool useSpansToEstimateScrollBarDimensions) {
    mUsingSpansToEstimateScrollBarDimensions = useSpansToEstimateScrollBarDimensions;
}

bool GridLayoutManager::isUsingSpansToEstimateScrollbarDimensions() const{
    return mUsingSpansToEstimateScrollBarDimensions;
}

int GridLayoutManager::computeScrollRangeWithSpanInfo(RecyclerView::State& state) {
    if (getChildCount() == 0 || state.getItemCount() == 0) {
        return 0;
    }
    ensureLayoutState();

    View* startChild = findFirstVisibleChildClosestToStart(!isSmoothScrollbarEnabled(), true);
    View* endChild = findFirstVisibleChildClosestToEnd(!isSmoothScrollbarEnabled(), true);

    if (startChild == nullptr || endChild == nullptr) {
        return 0;
    }
    if (!isSmoothScrollbarEnabled()) {
        return mSpanSizeLookup->getCachedSpanGroupIndex(
                state.getItemCount() - 1, mSpanCount) + 1;
    }

    // smooth scrollbar enabled. try to estimate better.
    const int laidOutArea = mOrientationHelper->getDecoratedEnd(endChild)
            - mOrientationHelper->getDecoratedStart(startChild);

    const int firstVisibleSpan =
            mSpanSizeLookup->getCachedSpanGroupIndex(getPosition(startChild), mSpanCount);
    const int lastVisibleSpan = mSpanSizeLookup->getCachedSpanGroupIndex(getPosition(endChild),
            mSpanCount);
    const int totalSpans = mSpanSizeLookup->getCachedSpanGroupIndex(state.getItemCount() - 1,
            mSpanCount) + 1;
    const int laidOutSpans = lastVisibleSpan - firstVisibleSpan + 1;

    // estimate a size for full list.
    return (int) (((float) laidOutArea / laidOutSpans) * totalSpans);
}

int GridLayoutManager::computeScrollOffsetWithSpanInfo(RecyclerView::State& state) {
    if (getChildCount() == 0 || state.getItemCount() == 0) {
        return 0;
    }
    ensureLayoutState();

    bool smoothScrollEnabled = isSmoothScrollbarEnabled();
    View* startChild = findFirstVisibleChildClosestToStart(!smoothScrollEnabled, true);
    View* endChild = findFirstVisibleChildClosestToEnd(!smoothScrollEnabled, true);
    if (startChild == nullptr || endChild == nullptr) {
        return 0;
    }
    int startChildSpan = mSpanSizeLookup->getCachedSpanGroupIndex(getPosition(startChild),
            mSpanCount);
    int endChildSpan = mSpanSizeLookup->getCachedSpanGroupIndex(getPosition(endChild),
            mSpanCount);

    const int minSpan = std::min(startChildSpan, endChildSpan);
    const int maxSpan = std::max(startChildSpan, endChildSpan);
    const int totalSpans = mSpanSizeLookup->getCachedSpanGroupIndex(state.getItemCount() - 1,
            mSpanCount) + 1;

    const int spansBefore = mShouldReverseLayout
            ? std::max(0, totalSpans - maxSpan - 1)
            : std::max(0, minSpan);
    if (!smoothScrollEnabled) {
        return spansBefore;
    }
    const int laidOutArea = std::abs(mOrientationHelper->getDecoratedEnd(endChild)
            - mOrientationHelper->getDecoratedStart(startChild));

    const int firstVisibleSpan =
            mSpanSizeLookup->getCachedSpanGroupIndex(getPosition(startChild), mSpanCount);
    const int lastVisibleSpan = mSpanSizeLookup->getCachedSpanGroupIndex(getPosition(endChild),
            mSpanCount);
    const int laidOutSpans = lastVisibleSpan - firstVisibleSpan + 1;
    const float avgSizePerSpan = (float) laidOutArea / laidOutSpans;

    return std::round(spansBefore * avgSizePerSpan + (mOrientationHelper->getStartAfterPadding()
        - mOrientationHelper->getDecoratedStart(startChild)));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GridLayoutManager::SpanSizeLookup::setSpanIndexCacheEnabled(bool cacheSpanIndices) {
    if (!cacheSpanIndices) {
        mSpanGroupIndexCache.clear();
    }
    mCacheSpanIndices = cacheSpanIndices;
}

void GridLayoutManager::SpanSizeLookup::setSpanGroupIndexCacheEnabled(bool cacheSpanGroupIndices)  {
    if (!cacheSpanGroupIndices) {
        mSpanGroupIndexCache.clear();
    }
    mCacheSpanGroupIndices = cacheSpanGroupIndices;
}

void GridLayoutManager::SpanSizeLookup::invalidateSpanIndexCache() {
    mSpanIndexCache.clear();
}

void GridLayoutManager::SpanSizeLookup::invalidateSpanGroupIndexCache(){
    mSpanGroupIndexCache.clear();
}

bool GridLayoutManager::SpanSizeLookup::isSpanIndexCacheEnabled() {
    return mCacheSpanIndices;
}

bool GridLayoutManager::SpanSizeLookup::isSpanGroupIndexCacheEnabled() {
    return mCacheSpanGroupIndices;
}

int GridLayoutManager::SpanSizeLookup::getCachedSpanIndex(int position, int spanCount) {
    if (!mCacheSpanIndices) {
        return getSpanIndex(position, spanCount);
    }
    const int existing = mSpanIndexCache.get(position, -1);
    if (existing != -1) {
        return existing;
    }
    const int value = getSpanIndex(position, spanCount);
    mSpanIndexCache.put(position, value);
    return value;
}

int GridLayoutManager::SpanSizeLookup::getCachedSpanGroupIndex(int position, int spanCount) {
    if (!mCacheSpanGroupIndices) {
        return getSpanGroupIndex(position, spanCount);
    }
    const int existing = mSpanGroupIndexCache.get(position, -1);
    if (existing != -1) {
        return existing;
    }
    const int value = getSpanGroupIndex(position, spanCount);
    mSpanGroupIndexCache.put(position, value);
    return value;
}

int GridLayoutManager::SpanSizeLookup::getSpanIndex(int position, int spanCount) {
    int positionSpanSize = getSpanSize(position);
    if (positionSpanSize == spanCount) {
        return 0; // quick return for full-span items
    }
    int span = 0;
    int startPos = 0;
    // If caching is enabled, try to jump
    if (mCacheSpanIndices && mSpanIndexCache.size() > 0) {
        int prevKey = findFirstKeyLessThan(mSpanIndexCache,position);
        if (prevKey >= 0) {
            span = mSpanIndexCache.get(prevKey) + getSpanSize(prevKey);
            startPos = prevKey + 1;
        }
    }
    for (int i = startPos; i < position; i++) {
        int size = getSpanSize(i);
        span += size;
        if (span == spanCount) {
            span = 0;
        } else if (span > spanCount) {
            // did not fit, moving to next row / column
            span = size;
        }
    }
    if (span + positionSpanSize <= spanCount) {
        return span;
    }
    return 0;
}

int GridLayoutManager::SpanSizeLookup::findFirstKeyLessThan(SparseIntArray&cache,int position) {
    int lo = 0;
    int hi = cache.size() - 1;

    while (lo <= hi) {
        int mid = (lo + hi) >> 1;
        int midVal = cache.keyAt(mid);
        if (midVal < position) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    int index = lo - 1;
    if (index >= 0 && index < cache.size()) {
        return cache.keyAt(index);
    }
    return -1;
}

int GridLayoutManager::SpanSizeLookup::getSpanGroupIndex(int adapterPosition, int spanCount) {
    int span  = 0;
    int group = 0;
    int start = 0;
    if(mCacheSpanGroupIndices){
        // This finds the first non empty cached group cache key.
        int prevKey = findFirstKeyLessThan(mSpanGroupIndexCache, adapterPosition);
        if (prevKey != -1) {
            group = mSpanGroupIndexCache.get(prevKey);
            start = prevKey + 1;
            span = getCachedSpanIndex(prevKey, spanCount) + getSpanSize(prevKey);
            if (span == spanCount) {
                span = 0;
                group++;
            }
        }
    }
    const int positionSpanSize = getSpanSize(adapterPosition);
    for (int i = start; i < adapterPosition; i++) {
        int size = getSpanSize(i);
        span += size;
        if (span == spanCount) {
            span = 0;
            group++;
        } else if (span > spanCount) {
            // did not fit, moving to next row / column
            span = size;
            group++;
        }
    }
    if (span + positionSpanSize > spanCount) {
        group++;
    }
    return group;
}

int GridLayoutManager::DefaultSpanSizeLookup::getSpanSize(int position) {
    return 1;
}

int GridLayoutManager::DefaultSpanSizeLookup::getSpanIndex(int position, int spanCount) {
    return position % spanCount;
}

GridLayoutManager::LayoutParams::LayoutParams(Context* c, const AttributeSet& attrs)
    :RecyclerView::LayoutParams(c, attrs){
}

GridLayoutManager::LayoutParams::LayoutParams(int width, int height)
    :RecyclerView::LayoutParams(width, height){
}

GridLayoutManager::LayoutParams::LayoutParams(const ViewGroup::MarginLayoutParams& source)
    :RecyclerView::LayoutParams(source){
}

GridLayoutManager::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& source)
    :RecyclerView::LayoutParams(source){
}

GridLayoutManager::LayoutParams::LayoutParams(const RecyclerView::LayoutParams& source)
    :RecyclerView::LayoutParams(source){
}

int GridLayoutManager::LayoutParams::getSpanIndex()const{
    return mSpanIndex;
}

int GridLayoutManager::LayoutParams::getSpanSize()const{
    return mSpanSize;
}

}/*endof namespace*/
