#ifndef __FLEX_LINE_H__
#define __FLEX_LINE_H__
namespace cdroid{
class FlexLine {
protected:
    int mLeft= INT_MAX;
    int mTop = INT_MAX;
    int mRight = INT_MIN;
    int mBottom = INT_MIN;
    int mMainSize;
    int mDividerLengthInMainSize;
    int mCrossSize;
    int mItemCount;
    int mGoneItemCount;
    int mMaxBaseline;
    int mSumCrossSizeBefore;
    int mFirstIndex;
    int mLastIndex;

    float mTotalFlexGrow;
    float mTotalFlexShrink;

    std::vector<int> mIndicesAlignSelfStretch;

    bool mAnyItemsHaveFlexGrow;
    bool mAnyItemsHaveFlexShrink;
    friend class FlexboxLayout;
    friend class FlexboxHelper;
public:
    FlexLine() {
    }

    int getMainSize() {
        return mMainSize;
    }

    int getCrossSize() {
        return mCrossSize;
    }

    int getItemCount() {
        return mItemCount;
    }

    int getItemCountNotGone() {
        return mItemCount - mGoneItemCount;
    }

    float getTotalFlexGrow() {
        return mTotalFlexGrow;
    }

    float getTotalFlexShrink() {
        return mTotalFlexShrink;
    }

    int getFirstIndex() {
        return mFirstIndex;
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
