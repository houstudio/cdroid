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
#ifndef __FLEX_LINE_H__
#define __FLEX_LINE_H__
namespace cdroid{
class FlexLine {
protected:
    int mLeft= INT_MAX;
    int mTop = INT_MAX;
    int mRight = INT_MIN;
    int mBottom = INT_MIN;
    int mMainSize = 0;
    int mDividerLengthInMainSize = 0;
    int mCrossSize = 0;
    int mItemCount = 0;
    int mGoneItemCount = 0;
    int mMaxBaseline = 0;
    int mSumCrossSizeBefore = 0;
    int mFirstIndex = 0;
    int mLastIndex = 0;

    float mTotalFlexGrow = 0.f;
    float mTotalFlexShrink = 0.f;

    std::vector<int> mIndicesAlignSelfStretch;

    bool mAnyItemsHaveFlexGrow = false;
    bool mAnyItemsHaveFlexShrink = false;
    friend class FlexboxLayout;
    friend class FlexboxHelper;
public:
    FlexLine() {
    }

    int getMainSize() const{
        return mMainSize;
    }

    int getCrossSize() const{
        return mCrossSize;
    }

    int getItemCount() const{
        return mItemCount;
    }

    int getItemCountNotGone() const{
        return mItemCount - mGoneItemCount;
    }

    float getTotalFlexGrow() const{
        return mTotalFlexGrow;
    }

    float getTotalFlexShrink() const{
        return mTotalFlexShrink;
    }

    int getFirstIndex() const{
        return mFirstIndex;
    }

    void setMainSize(int mainSize) {
        mMainSize = mainSize;
    }

    void setDividerLengthInMainSize(int dividerLengthInMainSize) {
        mDividerLengthInMainSize = dividerLengthInMainSize;
    }

    int getDividerLengthInMainSize() const{
        return mDividerLengthInMainSize;
    }

    int getLastIndex() const{
        return mLastIndex;
    }

    void updatePositionFromView(View* view, int leftDecoration, int topDecoration,
            int rightDecoration, int bottomDecoration) {
#if 0
        FlexItem* flexItem = (FlexItem*) view->getLayoutParams();
        mLeft = std::min(mLeft, view->getLeft() - flexItem->getMarginLeft() - leftDecoration);
        mTop = std::min(mTop, view->getTop() - flexItem->getMarginTop() - topDecoration);
        mRight = std::max(mRight, view->getRight() + flexItem->getMarginRight() + rightDecoration);
        mBottom = std::max(mBottom, view->getBottom() + flexItem->getMarginBottom() + bottomDecoration);
#endif
    }
};
}/*endof namespace*/
#endif/*__FLEX_LINE_H__*/
