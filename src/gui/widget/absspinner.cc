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
#include <widget/absspinner.h>
#include <sparsearray.h>
#include <cdlog.h>

namespace cdroid{

AbsSpinner::RecycleBin::RecycleBin(AbsSpinner*abs){
    ABS=abs;
}
void AbsSpinner::RecycleBin::put(int position, View* v) {
    mScrapHeap.put(position, v);
}

View* AbsSpinner::RecycleBin::get(int position) {
    View* result = mScrapHeap.get(position);
    if (result) mScrapHeap.remove(position);
    return result;
}

void AbsSpinner::RecycleBin::clear() {
    int count = mScrapHeap.size();
    for (int i = 0; i < count; i++) {
        View* view = mScrapHeap.valueAt(i);
        if (view)ABS->removeDetachedView(view, true);
    }
    mScrapHeap.clear();
}    

AbsSpinner::AbsSpinner(int w,int h):AdapterView(w,h){
    initAbsSpinner();
}

AbsSpinner::AbsSpinner(Context*ctx,const AttributeSet&atts)
  :AdapterView(ctx,atts){
    initAbsSpinner();
}

AbsSpinner::~AbsSpinner(){
    delete mRecycler;
}

void AbsSpinner::initAbsSpinner() {
    mSelectionLeftPadding=0;
    mSelectionTopPadding=0;
    mSelectionRightPadding=0;
    mSelectionBottomPadding=0;
    mHeightMeasureSpec=0;
    mWidthMeasureSpec =0;
    setFocusable(true);
    setWillNotDraw(false);
    mSpinnerPadding.setEmpty();
    mRecycler=new RecycleBin(this);
}

int AbsSpinner::getCount(){
    return mItemCount;
}

void AbsSpinner::setAdapter(Adapter* adapter) {
    if (mAdapter) {
        mAdapter->unregisterDataSetObserver(mDataSetObserver);
        resetList();
    }
    mAdapter=adapter;
    mOldSelectedPosition = INVALID_POSITION;
    mOldSelectedRowId = INVALID_ROW_ID;

    if (mAdapter != nullptr) {
        mOldItemCount = mItemCount;
        mItemCount = mAdapter->getCount();
        checkFocus();
        mDataSetObserver = new AdapterDataSetObserver(this);
        mAdapter->registerDataSetObserver(mDataSetObserver);
        int position = mItemCount > 0 ? 0 : INVALID_POSITION;

        setSelectedPositionInt(position);
        setNextSelectedPositionInt(position);

        if (mItemCount == 0) {
            // Nothing selected
            checkSelectionChanged();
        }

   } else {
        checkFocus();
        resetList();
        // Nothing selected
        checkSelectionChanged();
   }
   requestLayout();
}

void AbsSpinner::resetList() {
    mDataChanged = false;
    mNeedSync = false;

    removeAllViewsInLayout();
    mOldSelectedPosition = INVALID_POSITION;
    mOldSelectedRowId = INVALID_ROW_ID;

    setSelectedPositionInt(INVALID_POSITION);
    setNextSelectedPositionInt(INVALID_POSITION);
    invalidate(true);
}
int AbsSpinner::getChildHeight(View* child) {
    return child->getMeasuredHeight();
}

int AbsSpinner::getChildWidth(View* child) {
    return child->getMeasuredWidth();
}

void AbsSpinner::recycleAllViews() {
    int childCount = getChildCount();
    int position = mFirstPosition;
    // All views go in recycler
    for (int i = 0; i < childCount; i++) {
        View* v = getChildAt(i);
        int index = position + i;
        mRecycler->put(index, v);
    }
}

void AbsSpinner::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    int widthSize;
    int heightSize;
    const int widthMode = MeasureSpec::getMode(widthMeasureSpec);

    mSpinnerPadding.left = mPaddingLeft > mSelectionLeftPadding ? mPaddingLeft
            : mSelectionLeftPadding;
    mSpinnerPadding.top  = mPaddingTop > mSelectionTopPadding ? mPaddingTop
            : mSelectionTopPadding;
    mSpinnerPadding.width = mPaddingRight > mSelectionRightPadding ? mPaddingRight
            : mSelectionRightPadding;
    mSpinnerPadding.height = mPaddingBottom > mSelectionBottomPadding ? mPaddingBottom
            : mSelectionBottomPadding;

    if (mDataChanged) {
        handleDataChanged();
    }

    int preferredHeight = 0;
    int preferredWidth = 0;
    bool needsMeasuring = true;

    const int selectedPosition = getSelectedItemPosition();
    if (selectedPosition >= 0 && mAdapter != nullptr && selectedPosition < mAdapter->getCount()) {
        // Try looking in the recycler. (Maybe we were measured once already)
        View* view = mRecycler->get(selectedPosition);
        if (view == nullptr) {
            // Make a new one
            view = mAdapter->getView(selectedPosition, nullptr, this);
            if (view->getImportantForAccessibility() == IMPORTANT_FOR_ACCESSIBILITY_AUTO) {
                view->setImportantForAccessibility(IMPORTANT_FOR_ACCESSIBILITY_YES);
            }
        }
        if (view) {
            // Put in recycler for re-measuring and/or layout
            mRecycler->put(selectedPosition,view);
            if (view->getLayoutParams() == nullptr) {
                mBlockLayoutRequests = true;
                view->setLayoutParams(generateDefaultLayoutParams());
                mBlockLayoutRequests = false;
            }
            measureChild(view, widthMeasureSpec, heightMeasureSpec);
            preferredHeight = getChildHeight(view) + mSpinnerPadding.top + mSpinnerPadding.height;
            preferredWidth = getChildWidth(view) + mSpinnerPadding.left + mSpinnerPadding.width;
            needsMeasuring = false;
        }
    }
    if (needsMeasuring) {
         // No views -- just use padding
         preferredHeight = mSpinnerPadding.top + mSpinnerPadding.height;
         if (widthMode == MeasureSpec::UNSPECIFIED) {
              preferredWidth = mSpinnerPadding.left + mSpinnerPadding.width;
         }
    }

    preferredHeight= std::max(preferredHeight, getSuggestedMinimumHeight());
    preferredWidth = std::max(preferredWidth, getSuggestedMinimumWidth());

    heightSize = resolveSizeAndState(preferredHeight, heightMeasureSpec, 0);
    widthSize = resolveSizeAndState(preferredWidth, widthMeasureSpec, 0);

    setMeasuredDimension(widthSize, heightSize);
    mHeightMeasureSpec = heightMeasureSpec;
    mWidthMeasureSpec = widthMeasureSpec;
}

void AbsSpinner::setSelection(int position, bool animate){
     bool shouldAnimate = animate && mFirstPosition <= position &&
                position <= mFirstPosition + getChildCount() - 1;
    setSelectionInt(position, shouldAnimate);
}

void AbsSpinner::setSelection(int position) {
    setNextSelectedPositionInt(position);
    requestLayout();
    invalidate();
}

void AbsSpinner::setSelectionInt(int position, bool animate){
    if (position != mOldSelectedPosition) {
        mBlockLayoutRequests = true;
        int delta  = position - mSelectedPosition;
        setNextSelectedPositionInt(position);
        layout(delta, animate);
        mBlockLayoutRequests = false;
    }
}

View* AbsSpinner::getSelectedView() {
    if (mItemCount > 0 && mSelectedPosition >= 0) {
        return getChildAt(mSelectedPosition - mFirstPosition);
    } else {
        return nullptr;
    }
}

int AbsSpinner::pointToPosition(int x, int y) {
    const int count = getChildCount();
    for (int i = count - 1; i >= 0; i--) {
        Rect frame;
        View* child = getChildAt(i);
        if (child->getVisibility() == View::VISIBLE) {
            child->getHitRect(frame);
            if (frame.contains(x, y)) {
                return mFirstPosition + i;
            }
        }
    }
    return INVALID_POSITION;
}

void AbsSpinner::requestLayout() {
    if (!mBlockLayoutRequests) {
        AdapterView::requestLayout();
    }
}
}
