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
#ifndef __FLEX_HELPER_H__
#define __FLEX_HELPER_H__
#include <view/viewgroup.h>
#include <widgetEx/flexbox/flexline.h>
namespace cdroid{
class FlexItem;
class FlexContainer;
class CompoundButton;
class FlexboxHelper {
private:
    static constexpr int NO_POSITION = -1;
    static constexpr int INITIAL_CAPACITY = 10;
    static constexpr long MEASURE_SPEC_WIDTH_MASK = 0xffffffffL;
    FlexContainer* mFlexContainer;
    std::vector<bool> mChildrenFrozen;
    std::vector<int> mIndexToFlexLine;

    std::vector<int64_t> mMeasureSpecCache;
    std::vector<int64_t> mMeasuredSizeCache;

    struct Order{
        int index;
        int order;
    };
public:
    struct FlexLinesResult {
        std::vector<FlexLine> mFlexLines;
        int mChildState;
        void reset() {
            mFlexLines.clear();
            mChildState = 0;
        }
    };

    const std::vector<int>& getIndexToFlexLine() const {
        return mIndexToFlexLine;
    }
private:
    std::vector<Order> createOrders(int childCount);
    std::vector<int> sortOrdersIntoReorderedIndices(int childCount, std::vector<Order>& orders,SparseIntArray& orderCache);
    int getPaddingStartMain(bool isMainHorizontal);
    int getPaddingEndMain(bool isMainHorizontal);
    int getPaddingStartCross(bool isMainHorizontal);
    int getPaddingEndCross(bool isMainHorizontal);
    int getViewMeasuredSizeMain(View* view, bool isMainHorizontal);

    int getViewMeasuredSizeCross(View* view, bool isMainHorizontal);
    int getFlexItemSizeMain(FlexItem* flexItem, bool isMainHorizontal);
    int getFlexItemSizeCross(FlexItem* flexItem, bool isMainHorizontal);
    int getFlexItemMarginStartMain(FlexItem* flexItem, bool isMainHorizontal);
    int getFlexItemMarginEndMain(FlexItem* flexItem, bool isMainHorizontal);

    int getFlexItemMarginStartCross(FlexItem* flexItem, bool isMainHorizontal);
    int getFlexItemMarginEndCross(FlexItem* flexItem, bool isMainHorizontal);

    bool isWrapRequired(View* view, int mode, int maxSize, int currentLength,
          int childLength,FlexItem* flexItem, int index, int indexInFlexLine, int flexLinesSize);
    bool isLastFlexItem(int childIndex, int childCount, FlexLine flexLine);
    void addFlexLine(std::vector<FlexLine>& flexLines, FlexLine flexLine, int viewIndex,int usedCrossSizeSoFar);
    void checkSizeConstraints(View* view, int index);
    void evaluateMinimumSizeForCompoundButton(CompoundButton* compoundButton);
    void ensureChildrenFrozen(int size);

    void expandFlexItems(int widthMeasureSpec, int heightMeasureSpec, FlexLine& flexLine,
         int maxMainSize, int paddingAlongMainAxis, bool calledRecursively);
    void shrinkFlexItems(int widthMeasureSpec, int heightMeasureSpec, FlexLine& flexLine,
         int maxMainSize, int paddingAlongMainAxis, bool calledRecursively);

    int getChildWidthMeasureSpecInternal(int widthMeasureSpec,FlexItem* flexItem,int padding);
    int getChildHeightMeasureSpecInternal(int heightMeasureSpec,FlexItem* flexItem,int padding);
    std::vector<FlexLine> constructFlexLinesForAlignContentCenter(std::vector<FlexLine>& flexLines,int size, int totalCrossSize);
    void stretchViewVertically(View* view, int crossSize, int index);
    void stretchViewHorizontally(View* view, int crossSize, int index);
    void updateMeasureCache(int index, int widthMeasureSpec, int heightMeasureSpec,View* view);
public:
    FlexboxHelper(FlexContainer* flexContainer);

    std::vector<int> createReorderedIndices(View* viewBeforeAdded, int indexForViewBeforeAdded,
            ViewGroup::LayoutParams* paramsForViewBeforeAdded,SparseIntArray& orderCache);
    std::vector<int> createReorderedIndices(SparseIntArray& orderCache);

    bool isOrderChangedFromLastMeasurement(const SparseIntArray& orderCache);

    void calculateHorizontalFlexLines(FlexLinesResult* result, int widthMeasureSpec,int heightMeasureSpec);
    void calculateHorizontalFlexLines(FlexLinesResult* result, int widthMeasureSpec,
            int heightMeasureSpec, int needsCalcAmount, int fromIndex,std::vector<FlexLine>* existingLines);
    void calculateHorizontalFlexLinesToIndex(FlexLinesResult* result, int widthMeasureSpec,
            int heightMeasureSpec, int needsCalcAmount, int toIndex, std::vector<FlexLine>* existingLines);

    void calculateVerticalFlexLines(FlexLinesResult* result, int widthMeasureSpec, int heightMeasureSpec);
    void calculateVerticalFlexLines(FlexLinesResult* result, int widthMeasureSpec,
            int heightMeasureSpec, int needsCalcAmount, int fromIndex, std::vector<FlexLine>* existingLines);
    void calculateVerticalFlexLinesToIndex(FlexLinesResult* result, int widthMeasureSpec,
            int heightMeasureSpec, int needsCalcAmount, int toIndex, std::vector<FlexLine>* existingLines);

    void calculateFlexLines(FlexLinesResult* result, int mainMeasureSpec, int crossMeasureSpec,
            int needsCalcAmount, int fromIndex, int toIndex,std::vector<FlexLine>* existingLines);

    void determineMainSize(int widthMeasureSpec, int heightMeasureSpec);
    void determineMainSize(int widthMeasureSpec, int heightMeasureSpec, int fromIndex);
    void determineCrossSize(int widthMeasureSpec, int heightMeasureSpec,int paddingAlongCrossAxis);

    void stretchViews();
    void stretchViews(int fromIndex);

    void layoutSingleChildHorizontal(View* view, FlexLine flexLine, int left, int top, int right, int bottom);

    void layoutSingleChildVertical(View* view, FlexLine flexLine, bool isRtl,int left, int top, int right, int bottom);

    void ensureMeasuredSizeCache(int size);
    void ensureMeasureSpecCache(int size);

    int64_t getMeasureSpecCache(int index) const {
        return mMeasureSpecCache[index];
    }

    int extractLowerInt(int64_t longValue);
    int extractHigherInt(int64_t longValue);

    long makeCombinedLong(int widthMeasureSpec, int heightMeasureSpec);

    void ensureIndexToFlexLine(int size);

    void clearFlexLines(std::vector<FlexLine>& flexLines, int fromFlexItem);
};
}/*endof namespace*/
#endif/*__FLEX_HELPER_H__*/

