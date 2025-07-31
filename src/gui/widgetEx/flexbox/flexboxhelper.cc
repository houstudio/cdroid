#include <widgetEx/flexbox/flexitem.h>
#include <widgetEx/flexbox/flexwrap.h>
#include <widgetEx/flexbox/aligndefs.h>
#include <widgetEx/flexbox/flexboxhelper.h>
#include <widgetEx/flexbox/flexcontainer.h>
#include <widgetEx/flexbox/flexdirection.h>
namespace cdroid{

FlexboxHelper::FlexboxHelper(FlexContainer* flexContainer) {
    mFlexContainer = flexContainer;
}

std::vector<int> FlexboxHelper::createReorderedIndices(View* viewBeforeAdded, int indexForViewBeforeAdded,
        ViewGroup::LayoutParams* paramsForViewBeforeAdded, SparseIntArray& orderCache) {
    int childCount = mFlexContainer->getFlexItemCount();
    std::vector<Order> orders = createOrders(childCount);
    Order orderForViewToBeAdded;// = new Order();
    if (viewBeforeAdded != nullptr && dynamic_cast<FlexItem*>(paramsForViewBeforeAdded)) {
        orderForViewToBeAdded.order = ((FlexItem*)paramsForViewBeforeAdded)->getOrder();
    } else {
        orderForViewToBeAdded.order = FlexItem::ORDER_DEFAULT;
    }

    if ((indexForViewBeforeAdded == -1) || (indexForViewBeforeAdded == childCount)) {
        orderForViewToBeAdded.index = childCount;
    } else if (indexForViewBeforeAdded < mFlexContainer->getFlexItemCount()) {
        orderForViewToBeAdded.index = indexForViewBeforeAdded;
        for (int i = indexForViewBeforeAdded; i < childCount; i++) {
            orders.at(i).index++;
        }
    } else {
        // This path is not expected since OutOfBoundException will be thrown in the ViewGroup
        // But setting the index for fail-safe
        orderForViewToBeAdded.index = childCount;
    }
    orders.push_back(orderForViewToBeAdded);

    return sortOrdersIntoReorderedIndices(childCount + 1, orders, orderCache);
}

std::vector<int> FlexboxHelper::createReorderedIndices(SparseIntArray& orderCache) {
    int childCount = mFlexContainer->getFlexItemCount();
    std::vector<Order> orders = createOrders(childCount);
    return sortOrdersIntoReorderedIndices(childCount, orders, orderCache);
}

std::vector<FlexboxHelper::Order> FlexboxHelper::createOrders(int childCount) {
    std::vector<Order> orders;
    for (int i = 0; i < childCount; i++) {
        View* child = mFlexContainer->getFlexItemAt(i);
        FlexItem* flexItem = (FlexItem*) child->getLayoutParams();
        Order order;// = new Order();
        order.order = flexItem->getOrder();
        order.index = i;
        orders.push_back(order);
    }
    return orders;
}

bool FlexboxHelper::isOrderChangedFromLastMeasurement(const SparseIntArray& orderCache) {
    const int childCount = mFlexContainer->getFlexItemCount();
    if (orderCache.size() != childCount) {
        return true;
    }
    for (int i = 0; i < childCount; i++) {
        View* view = mFlexContainer->getFlexItemAt(i);
        if (view == nullptr) {
            continue;
        }
        FlexItem* flexItem = (FlexItem*) view->getLayoutParams();
        if (flexItem->getOrder() != orderCache.get(i)) {
            return true;
        }
    }
    return false;
}

std::vector<int> FlexboxHelper::sortOrdersIntoReorderedIndices(int childCount, std::vector<Order>& orders,SparseIntArray& orderCache) {
    std::sort(orders.begin(),orders.end(),[](const Order& a, const Order&b ){
        if(a.order!=b.order)return a.order-b.order;
        return a.index-b.index;
    });
    orderCache.clear();
    std::vector<int>reorderedIndices(childCount);
    int i = 0;
    for (Order order : orders) {
        reorderedIndices[i] = order.index;
        orderCache.append(order.index, order.order);
        i++;
    }
    return reorderedIndices;
}

void FlexboxHelper::calculateHorizontalFlexLines(FlexLinesResult* result, int widthMeasureSpec, int heightMeasureSpec) {
    calculateFlexLines(result, widthMeasureSpec, heightMeasureSpec, INT_MAX, 0, (int)NO_POSITION, nullptr);
}

void FlexboxHelper::calculateHorizontalFlexLines(FlexLinesResult* result, int widthMeasureSpec,
        int heightMeasureSpec, int needsCalcAmount, int fromIndex,std::vector<FlexLine>* existingLines) {
    calculateFlexLines(result, widthMeasureSpec, heightMeasureSpec, needsCalcAmount,
            fromIndex, (int)NO_POSITION, existingLines);
}

void FlexboxHelper::calculateHorizontalFlexLinesToIndex(FlexLinesResult* result, int widthMeasureSpec,
        int heightMeasureSpec, int needsCalcAmount, int toIndex, std::vector<FlexLine>* existingLines) {
    calculateFlexLines(result, widthMeasureSpec, heightMeasureSpec, needsCalcAmount,
            0, toIndex, existingLines);
}

void FlexboxHelper::calculateVerticalFlexLines(FlexLinesResult* result, int widthMeasureSpec, int heightMeasureSpec) {
    calculateFlexLines(result, heightMeasureSpec, widthMeasureSpec, INT_MAX,0, (int)NO_POSITION, nullptr);
}

void FlexboxHelper::calculateVerticalFlexLines(FlexLinesResult* result, int widthMeasureSpec,
        int heightMeasureSpec, int needsCalcAmount, int fromIndex,std::vector<FlexLine>* existingLines) {
    calculateFlexLines(result, heightMeasureSpec, widthMeasureSpec, needsCalcAmount,fromIndex, (int)NO_POSITION, existingLines);
}

void FlexboxHelper::calculateVerticalFlexLinesToIndex(FlexLinesResult* result, int widthMeasureSpec,
        int heightMeasureSpec, int needsCalcAmount, int toIndex, std::vector<FlexLine>* existingLines) {
    calculateFlexLines(result, heightMeasureSpec, widthMeasureSpec, needsCalcAmount, 0, toIndex, existingLines);
}

void FlexboxHelper::calculateFlexLines(FlexLinesResult* result, int mainMeasureSpec,
        int crossMeasureSpec, int needsCalcAmount, int fromIndex, int toIndex,std::vector<FlexLine>* existingLines) {

    const bool isMainHorizontal = mFlexContainer->isMainAxisDirectionHorizontal();

    const int mainMode = View::MeasureSpec::getMode(mainMeasureSpec);
    const int mainSize = View::MeasureSpec::getSize(mainMeasureSpec);

    int childState = 0;

    std::vector<FlexLine> flexLines;
    if (existingLines!= nullptr) {
        flexLines = *existingLines;
    }

    result->mFlexLines = flexLines;

    bool reachedToIndex = (toIndex == (int)NO_POSITION);

    int mainPaddingStart = getPaddingStartMain(isMainHorizontal);
    int mainPaddingEnd = getPaddingEndMain(isMainHorizontal);
    int crossPaddingStart = getPaddingStartCross(isMainHorizontal);
    int crossPaddingEnd = getPaddingEndCross(isMainHorizontal);

    int largestSizeInCross = INT_MIN;

    // The amount of cross size calculated in this method call.
    int sumCrossSize = 0;

    // The index of the view in the flex line.
    int indexInFlexLine = 0;

    FlexLine flexLine;// = new FlexLine();
    flexLine.mFirstIndex = fromIndex;
    flexLine.mMainSize = mainPaddingStart + mainPaddingEnd;

    int childCount = mFlexContainer->getFlexItemCount();
    for (int i = fromIndex; i < childCount; i++) {
        View* child = mFlexContainer->getReorderedFlexItemAt(i);

        if (child == nullptr) {
            if (isLastFlexItem(i, childCount, flexLine)) {
                addFlexLine(flexLines, flexLine, i, sumCrossSize);
            }
            continue;
        } else if (child->getVisibility() == View::GONE) {
            flexLine.mGoneItemCount++;
            flexLine.mItemCount++;
            if (isLastFlexItem(i, childCount, flexLine)) {
                addFlexLine(flexLines, flexLine, i, sumCrossSize);
            }
            continue;
        }

        FlexItem* flexItem = (FlexItem*) child->getLayoutParams();

        if (flexItem->getAlignSelf() == (int)AlignItems::STRETCH) {
            flexLine.mIndicesAlignSelfStretch.push_back(i);
        }

        int childMainSize = getFlexItemSizeMain(flexItem, isMainHorizontal);

        if (flexItem->getFlexBasisPercent() != FlexItem::FLEX_BASIS_PERCENT_DEFAULT
                && mainMode == View::MeasureSpec::EXACTLY) {
            childMainSize = std::round(mainSize * flexItem->getFlexBasisPercent());
            // Use the dimension from the layout if the mainMode is not
            // MeasureSpec.EXACTLY even if any fraction value is set to
            // layout_flexBasisPercent.
        }

        int childMainMeasureSpec;
        int childCrossMeasureSpec;
        if (isMainHorizontal) {
            childMainMeasureSpec = mFlexContainer->getChildWidthMeasureSpec(mainMeasureSpec,
                    mainPaddingStart + mainPaddingEnd + getFlexItemMarginStartMain(flexItem, true) +
                            getFlexItemMarginEndMain(flexItem, true), childMainSize);
            childCrossMeasureSpec = mFlexContainer->getChildHeightMeasureSpec(crossMeasureSpec,
                    crossPaddingStart + crossPaddingEnd + getFlexItemMarginStartCross(flexItem, true) +
                            getFlexItemMarginEndCross(flexItem, true) + sumCrossSize, getFlexItemSizeCross(flexItem, true));
            child->measure(childMainMeasureSpec, childCrossMeasureSpec);
            updateMeasureCache(i, childMainMeasureSpec, childCrossMeasureSpec, child);
        } else {
            childCrossMeasureSpec = mFlexContainer->getChildWidthMeasureSpec(crossMeasureSpec,
                    crossPaddingStart + crossPaddingEnd + getFlexItemMarginStartCross(flexItem, false) +
                            getFlexItemMarginEndCross(flexItem, false) + sumCrossSize,
                    getFlexItemSizeCross(flexItem, false));
            childMainMeasureSpec = mFlexContainer->getChildHeightMeasureSpec(mainMeasureSpec,
                    mainPaddingStart + mainPaddingEnd + getFlexItemMarginStartMain(flexItem, false) +
                            getFlexItemMarginEndMain(flexItem, false), childMainSize);
            child->measure(childCrossMeasureSpec, childMainMeasureSpec);
            updateMeasureCache(i, childCrossMeasureSpec, childMainMeasureSpec, child);
        }
        mFlexContainer->updateViewCache(i, child);

        // Check the size constraint after the first measurement for the child
        // To prevent the child's width/height violate the size constraints imposed by the
        // {@link FlexItem#getMinWidth()}, {@link FlexItem#getMinHeight()},
        // {@link FlexItem#getMaxWidth()} and {@link FlexItem#getMaxHeight()} attributes.
        // E.g. When the child's layout_width is wrap_content the measured width may be
        // less than the min width after the first measurement.
        checkSizeConstraints(child, i);

        childState = (childState|child->getMeasuredState());//View::combineMeasuredStates( childState, child->getMeasuredState());

        if (isWrapRequired(child, mainMode, mainSize, flexLine.mMainSize,
                getViewMeasuredSizeMain(child, isMainHorizontal)
                        + getFlexItemMarginStartMain(flexItem, isMainHorizontal) +
                        getFlexItemMarginEndMain(flexItem, isMainHorizontal),
                flexItem, i, indexInFlexLine, flexLines.size())) {
            if (flexLine.getItemCountNotGone() > 0) {
                addFlexLine(flexLines, flexLine, i > 0 ? i - 1 : 0, sumCrossSize);
                sumCrossSize += flexLine.mCrossSize;
            }

            if (isMainHorizontal) {
                if (flexItem->getHeight() == ViewGroup::LayoutParams::MATCH_PARENT) {
                    // This case takes care of the corner case where the cross size of the
                    // child is affected by the just added flex line.
                    // E.g. when the child's layout_height is set to match_parent, the height
                    // of that child needs to be determined taking the total cross size used
                    // so far into account. In that case, the height of the child needs to be
                    // measured again note that we don't need to judge if the wrapping occurs
                    // because it doesn't change the size along the main axis.
                    childCrossMeasureSpec = mFlexContainer->getChildHeightMeasureSpec( crossMeasureSpec,
                            mFlexContainer->getPaddingTop() + mFlexContainer->getPaddingBottom()
                                    + flexItem->getMarginTop() + flexItem->getMarginBottom() + sumCrossSize, flexItem->getHeight());
                    child->measure(childMainMeasureSpec, childCrossMeasureSpec);
                    checkSizeConstraints(child, i);
                }
            } else {
                if (flexItem->getWidth() == ViewGroup::LayoutParams::MATCH_PARENT) {
                    // This case takes care of the corner case where the cross size of the
                    // child is affected by the just added flex line.
                    // E.g. when the child's layout_width is set to match_parent, the width
                    // of that child needs to be determined taking the total cross size used
                    // so far into account. In that case, the width of the child needs to be
                    // measured again note that we don't need to judge if the wrapping occurs
                    // because it doesn't change the size along the main axis.
                    childCrossMeasureSpec = mFlexContainer->getChildWidthMeasureSpec(crossMeasureSpec,
                            mFlexContainer->getPaddingLeft() + mFlexContainer->getPaddingRight()
                                    + flexItem->getMarginLeft() + flexItem->getMarginRight() + sumCrossSize, flexItem->getWidth());
                    child->measure(childCrossMeasureSpec, childMainMeasureSpec);
                    checkSizeConstraints(child, i);
                }
            }

            //flexLine = new FlexLine();
            flexLine.mItemCount = 1;
            flexLine.mMainSize = mainPaddingStart + mainPaddingEnd;
            flexLine.mFirstIndex = i;
            indexInFlexLine = 0;
            largestSizeInCross = INT_MIN;
        } else {
            flexLine.mItemCount++;
            indexInFlexLine++;
        }
        flexLine.mAnyItemsHaveFlexGrow |= flexItem->getFlexGrow() != FlexItem::FLEX_GROW_DEFAULT;
        flexLine.mAnyItemsHaveFlexShrink |= flexItem->getFlexShrink() != FlexItem::FLEX_SHRINK_NOT_SET;

        if (!mIndexToFlexLine.empty()) {
            mIndexToFlexLine[i] = flexLines.size();
        }
        flexLine.mMainSize += getViewMeasuredSizeMain(child, isMainHorizontal)
                + getFlexItemMarginStartMain(flexItem, isMainHorizontal) +
                getFlexItemMarginEndMain(flexItem, isMainHorizontal);
        flexLine.mTotalFlexGrow += flexItem->getFlexGrow();
        flexLine.mTotalFlexShrink += flexItem->getFlexShrink();

        mFlexContainer->onNewFlexItemAdded(child, i, indexInFlexLine, flexLine);

        largestSizeInCross = std::max(largestSizeInCross,
                getViewMeasuredSizeCross(child, isMainHorizontal) +
                        getFlexItemMarginStartCross(flexItem, isMainHorizontal) +
                        getFlexItemMarginEndCross(flexItem, isMainHorizontal) +
                        mFlexContainer->getDecorationLengthCrossAxis(child));
        // Temporarily set the cross axis length as the largest child in the flexLine
        // Expand along the cross axis depending on the mAlignContent property if needed
        // later
        flexLine.mCrossSize = std::max(flexLine.mCrossSize, largestSizeInCross);

        if (isMainHorizontal) {
            if (mFlexContainer->getFlexWrap() != FlexWrap::WRAP_REVERSE) {
                flexLine.mMaxBaseline = std::max(flexLine.mMaxBaseline,
                        child->getBaseline() + flexItem->getMarginTop());
            } else {
                // if the flex wrap property is WRAP_REVERSE, calculate the
                // baseline as the distance from the cross end and the baseline
                // since the cross size calculation is based on the distance from the cross end
                flexLine.mMaxBaseline = std::max(flexLine.mMaxBaseline,
                        child->getMeasuredHeight() - child->getBaseline()
                                + flexItem->getMarginBottom());
            }
        }

        if (isLastFlexItem(i, childCount, flexLine)) {
            addFlexLine(flexLines, flexLine, i, sumCrossSize);
            sumCrossSize += flexLine.mCrossSize;
        }

        if ((toIndex != (int)NO_POSITION) && (flexLines.size() > 0)
                && (flexLines.at(flexLines.size() - 1).mLastIndex >= toIndex)
                && (i >= toIndex) && !reachedToIndex) {
            // Calculated to include a flex line which includes the flex item having the
            // toIndex.
            // Let the sumCrossSize start from the negative value of the last flex line's
            // cross size because otherwise flex lines aren't calculated enough to fill the
            // visible area.
            sumCrossSize = -flexLine.getCrossSize();
            reachedToIndex = true;
        }
        if (sumCrossSize > needsCalcAmount && reachedToIndex) {
            // Stop the calculation if the sum of cross size calculated reached to the point
            // beyond the needsCalcAmount value to avoid unneeded calculation in a
            // RecyclerView.
            // To be precise, the decoration length may be added to the sumCrossSize,
            // but we omit adding the decoration length because even without the decorator
            // length, it's guaranteed that calculation is done at least beyond the
            // needsCalcAmount
            break;
        }
    }

    result->mChildState = childState;
}

int FlexboxHelper::getPaddingStartMain(bool isMainHorizontal) {
    if (isMainHorizontal) {
        return mFlexContainer->getPaddingStart();
    }

    return mFlexContainer->getPaddingTop();
}

int FlexboxHelper::getPaddingEndMain(bool isMainHorizontal) {
    if (isMainHorizontal) {
        return mFlexContainer->getPaddingEnd();
    }

    return mFlexContainer->getPaddingBottom();
}

int FlexboxHelper::getPaddingStartCross(bool isMainHorizontal) {
    if (isMainHorizontal) {
        return mFlexContainer->getPaddingTop();
    }

    return mFlexContainer->getPaddingStart();
}

int FlexboxHelper::getPaddingEndCross(bool isMainHorizontal) {
    if (isMainHorizontal) {
        return mFlexContainer->getPaddingBottom();
    }

    return mFlexContainer->getPaddingEnd();
}

int FlexboxHelper::getViewMeasuredSizeMain(View* view, bool isMainHorizontal) {
    if (isMainHorizontal) {
        return view->getMeasuredWidth();
    }

    return view->getMeasuredHeight();
}

int FlexboxHelper::getViewMeasuredSizeCross(View* view, bool isMainHorizontal) {
    if (isMainHorizontal) {
        return view->getMeasuredHeight();
    }

    return view->getMeasuredWidth();
}

int FlexboxHelper::getFlexItemSizeMain(FlexItem* flexItem, bool isMainHorizontal) {
    if (isMainHorizontal) {
        return flexItem->getWidth();
    }

    return flexItem->getHeight();
}

int FlexboxHelper::getFlexItemSizeCross(FlexItem* flexItem, bool isMainHorizontal) {
    if (isMainHorizontal) {
        return flexItem->getHeight();
    }

    return flexItem->getWidth();
}

int FlexboxHelper::getFlexItemMarginStartMain(FlexItem* flexItem, bool isMainHorizontal) {
    if (isMainHorizontal) {
        return flexItem->getMarginLeft();
    }

    return flexItem->getMarginTop();
}

int FlexboxHelper::getFlexItemMarginEndMain(FlexItem* flexItem, bool isMainHorizontal) {
    if (isMainHorizontal) {
        return flexItem->getMarginRight();
    }

    return flexItem->getMarginBottom();
}

int FlexboxHelper::getFlexItemMarginStartCross(FlexItem* flexItem, bool isMainHorizontal) {
    if (isMainHorizontal) {
        return flexItem->getMarginTop();
    }

    return flexItem->getMarginLeft();
}

int FlexboxHelper::getFlexItemMarginEndCross(FlexItem* flexItem, bool isMainHorizontal) {
    if (isMainHorizontal) {
        return flexItem->getMarginBottom();
    }

    return flexItem->getMarginRight();
}

bool FlexboxHelper::isWrapRequired(View* view, int mode, int maxSize, int currentLength,
        int childLength, FlexItem* flexItem, int index, int indexInFlexLine, int flexLinesSize) {
    if (mFlexContainer->getFlexWrap() == FlexWrap::NOWRAP) {
        return false;
    }
    if (flexItem->isWrapBefore()) {
        return true;
    }
    if (mode == View::MeasureSpec::UNSPECIFIED) {
        return false;
    }
    int maxLine = mFlexContainer->getMaxLine();
    // Judge the condition by adding 1 to the current flexLinesSize because the flex line
    // being computed isn't added to the flexLinesSize.
    if (maxLine != FlexContainer::NOT_SET && maxLine <= flexLinesSize + 1) {
        return false;
    }
    int decorationLength = mFlexContainer->getDecorationLengthMainAxis(view, index, indexInFlexLine);
    if (decorationLength > 0) {
        childLength += decorationLength;
    }
    return maxSize < currentLength + childLength;
}

bool FlexboxHelper::isLastFlexItem(int childIndex, int childCount, FlexLine flexLine) {
    return childIndex == childCount - 1 && flexLine.getItemCountNotGone() != 0;
}

void FlexboxHelper::addFlexLine(std::vector<FlexLine>& flexLines, FlexLine flexLine, int viewIndex,
        int usedCrossSizeSoFar) {
    flexLine.mSumCrossSizeBefore = usedCrossSizeSoFar;
    mFlexContainer->onNewFlexLineAdded(flexLine);
    flexLine.mLastIndex = viewIndex;
    flexLines.push_back(flexLine);
}

void FlexboxHelper::checkSizeConstraints(View* view, int index) {
    bool needsMeasure = false;
    FlexItem* flexItem = (FlexItem*) view->getLayoutParams();
    int childWidth = view->getMeasuredWidth();
    int childHeight = view->getMeasuredHeight();

    if (childWidth < flexItem->getMinWidth()) {
        needsMeasure = true;
        childWidth = flexItem->getMinWidth();
    } else if (childWidth > flexItem->getMaxWidth()) {
        needsMeasure = true;
        childWidth = flexItem->getMaxWidth();
    }

    if (childHeight < flexItem->getMinHeight()) {
        needsMeasure = true;
        childHeight = flexItem->getMinHeight();
    } else if (childHeight > flexItem->getMaxHeight()) {
        needsMeasure = true;
        childHeight = flexItem->getMaxHeight();
    }
    if (needsMeasure) {
        const int widthSpec = View::MeasureSpec::makeMeasureSpec(childWidth, View::MeasureSpec::EXACTLY);
        const int heightSpec = View::MeasureSpec::makeMeasureSpec(childHeight, View::MeasureSpec::EXACTLY);
        view->measure(widthSpec, heightSpec);
        updateMeasureCache(index, widthSpec, heightSpec, view);
        mFlexContainer->updateViewCache(index, view);
    }
}

void FlexboxHelper::determineMainSize(int widthMeasureSpec, int heightMeasureSpec) {
    determineMainSize(widthMeasureSpec, heightMeasureSpec, 0);
}

void FlexboxHelper::determineMainSize(int widthMeasureSpec, int heightMeasureSpec, int fromIndex) {
    ensureChildrenFrozen(mFlexContainer->getFlexItemCount());
    if (fromIndex >= mFlexContainer->getFlexItemCount()) {
        return;
    }
    int mainSize , paddingAlongMainAxis;
    int widthMode,widthSize,heightMode,heightSize;
    int flexDirection = mFlexContainer->getFlexDirection();
    switch ((FlexDirection)mFlexContainer->getFlexDirection()) {
    case FlexDirection::ROW: // Intentional fall through
    case FlexDirection::ROW_REVERSE:
        widthMode = View::MeasureSpec::getMode(widthMeasureSpec);
        widthSize = View::MeasureSpec::getSize(widthMeasureSpec);
        if (widthMode == View::MeasureSpec::EXACTLY) {
            mainSize = widthSize;
        } else {
            mainSize = mFlexContainer->getLargestMainSize();
        }
        paddingAlongMainAxis = mFlexContainer->getPaddingLeft()
                + mFlexContainer->getPaddingRight();
        break;
    case FlexDirection::COLUMN: // Intentional fall through
    case FlexDirection::COLUMN_REVERSE:
        heightMode = View::MeasureSpec::getMode(heightMeasureSpec);
        heightSize = View::MeasureSpec::getSize(heightMeasureSpec);
        if (heightMode == View::MeasureSpec::EXACTLY) {
            mainSize = heightSize;
        } else {
            mainSize = mFlexContainer->getLargestMainSize();
        }
        paddingAlongMainAxis = mFlexContainer->getPaddingTop()
                + mFlexContainer->getPaddingBottom();
        break;
    default:
        LOGE("Invalid flex direction:%d" , flexDirection);
    }

    int flexLineIndex = 0;
    if (!mIndexToFlexLine.empty()) {
        flexLineIndex = mIndexToFlexLine[fromIndex];
    }
    std::vector<FlexLine> flexLines = mFlexContainer->getFlexLinesInternal();
    for (int i = flexLineIndex, size = flexLines.size(); i < size; i++) {
        FlexLine flexLine = flexLines.at(i);
        if (flexLine.mMainSize < mainSize && flexLine.mAnyItemsHaveFlexGrow) {
            expandFlexItems(widthMeasureSpec, heightMeasureSpec, flexLine,
                    mainSize, paddingAlongMainAxis, false);
        } else if (flexLine.mMainSize > mainSize && flexLine.mAnyItemsHaveFlexShrink) {
            shrinkFlexItems(widthMeasureSpec, heightMeasureSpec, flexLine,
                    mainSize, paddingAlongMainAxis, false);
        }
    }
}

void FlexboxHelper::ensureChildrenFrozen(int size) {
    if (mChildrenFrozen.size() < size) {
        const int newCapacity = mChildrenFrozen.size() * 2;
        mChildrenFrozen.resize(newCapacity >= size ? newCapacity : size);// = new bool[newCapacity >= size ? newCapacity : size];
    } else {
        std::fill(mChildrenFrozen.begin(),mChildrenFrozen.end(), false);
    }
}

void FlexboxHelper::expandFlexItems(int widthMeasureSpec, int heightMeasureSpec, FlexLine flexLine,
        int maxMainSize, int paddingAlongMainAxis, bool calledRecursively) {
    if (flexLine.mTotalFlexGrow <= 0 || maxMainSize < flexLine.mMainSize) {
        return;
    }
    int sizeBeforeExpand = flexLine.mMainSize;
    bool needsReexpand = false;
    float unitSpace = (maxMainSize - flexLine.mMainSize) / flexLine.mTotalFlexGrow;
    flexLine.mMainSize = paddingAlongMainAxis + flexLine.mDividerLengthInMainSize;

    // Setting the cross size of the flex line as the temporal value since the cross size of
    // each flex item may be changed from the initial calculation
    // (in the measureHorizontal/measureVertical method) even this method is part of the main
    // size determination.
    // E.g. If a TextView's layout_width is set to 0dp, layout_height is set to wrap_content,
    // and layout_flexGrow is set to 1, the TextView is trying to expand to the vertical
    // direction to enclose its content (in the measureHorizontal method), but
    // the width will be expanded in this method. In that case, the height needs to be measured
    // again with the expanded width.
    int largestCrossSize = 0;
    if (!calledRecursively) {
        flexLine.mCrossSize = INT_MIN;
    }
    float accumulatedRoundError = 0;
    for (int i = 0; i < flexLine.mItemCount; i++) {
        int index = flexLine.mFirstIndex + i;
        View* child = mFlexContainer->getReorderedFlexItemAt(index);
        if (child == nullptr || child->getVisibility() == View::GONE) {
            continue;
        }
        FlexItem* flexItem = (FlexItem*) child->getLayoutParams();
        int flexDirection = mFlexContainer->getFlexDirection();
        if (flexDirection == FlexDirection::ROW || flexDirection == FlexDirection::ROW_REVERSE) {
            // The direction of the main axis is horizontal

            int childMeasuredWidth = child->getMeasuredWidth();
            if (!mMeasuredSizeCache.empty()) {
                // Retrieve the measured width from the cache because there
                // are some cases that the view is re-created from the last measure, thus
                // View#getMeasuredWidth returns 0.
                // E.g. if the flex container is FlexboxLayoutManager, the case happens
                // frequently
                childMeasuredWidth = extractLowerInt(mMeasuredSizeCache[index]);
            }
            int childMeasuredHeight = child->getMeasuredHeight();
            if (!mMeasuredSizeCache.empty()) {
                // Extract the measured height from the cache
                childMeasuredHeight = extractHigherInt(mMeasuredSizeCache[index]);
            }
            if (!mChildrenFrozen[index] && flexItem->getFlexGrow() > 0.f) {
                float rawCalculatedWidth = childMeasuredWidth + unitSpace * flexItem->getFlexGrow();
                if (i == flexLine.mItemCount - 1) {
                    rawCalculatedWidth += accumulatedRoundError;
                    accumulatedRoundError = 0;
                }
                int newWidth = std::round(rawCalculatedWidth);
                if (newWidth > flexItem->getMaxWidth()) {
                    // This means the child can't expand beyond the value of the mMaxWidth
                    // attribute.
                    // To adjust the flex line length to the size of maxMainSize, remaining
                    // positive free space needs to be re-distributed to other flex items
                    // (children views). In that case, invoke this method again with the same
                    // fromIndex.
                    needsReexpand = true;
                    newWidth = flexItem->getMaxWidth();
                    mChildrenFrozen[index] = true;
                    flexLine.mTotalFlexGrow -= flexItem->getFlexGrow();
                } else {
                    accumulatedRoundError += (rawCalculatedWidth - newWidth);
                    if (accumulatedRoundError > 1.0) {
                        newWidth += 1;
                        accumulatedRoundError -= 1.0;
                    } else if (accumulatedRoundError < -1.0) {
                        newWidth -= 1;
                        accumulatedRoundError += 1.0;
                    }
                }
                const int childHeightMeasureSpec = getChildHeightMeasureSpecInternal(
                        heightMeasureSpec, flexItem, flexLine.mSumCrossSizeBefore);
                const int childWidthMeasureSpec = View::MeasureSpec::makeMeasureSpec(newWidth,
                        View::MeasureSpec::EXACTLY);
                child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
                childMeasuredWidth = child->getMeasuredWidth();
                childMeasuredHeight = child->getMeasuredHeight();
                updateMeasureCache(index, childWidthMeasureSpec, childHeightMeasureSpec,
                        child);
                mFlexContainer->updateViewCache(index, child);
            }
            largestCrossSize = std::max(largestCrossSize, childMeasuredHeight
                    + flexItem->getMarginTop() + flexItem->getMarginBottom()
                    + mFlexContainer->getDecorationLengthCrossAxis(child));
            flexLine.mMainSize += childMeasuredWidth + flexItem->getMarginLeft()
                    + flexItem->getMarginRight();
        } else {
            // The direction of the main axis is vertical

            int childMeasuredHeight = child->getMeasuredHeight();
            if (!mMeasuredSizeCache.empty()) {
                // Retrieve the measured height from the cache because there
                // are some cases that the view is re-created from the last measure, thus
                // View#getMeasuredHeight returns 0.
                // E.g. if the flex container is FlexboxLayoutManager, that case happens
                // frequently
                childMeasuredHeight = extractHigherInt(mMeasuredSizeCache[index]);
            }
            int childMeasuredWidth = child->getMeasuredWidth();
            if (!mMeasuredSizeCache.empty()) {
                // Extract the measured width from the cache
                childMeasuredWidth = extractLowerInt(mMeasuredSizeCache[index]);
            }
            if (!mChildrenFrozen[index] && flexItem->getFlexGrow() > 0.f) {
                float rawCalculatedHeight = childMeasuredHeight + unitSpace * flexItem->getFlexGrow();
                if (i == flexLine.mItemCount - 1) {
                    rawCalculatedHeight += accumulatedRoundError;
                    accumulatedRoundError = 0;
                }
                int newHeight = std::round(rawCalculatedHeight);
                if (newHeight > flexItem->getMaxHeight()) {
                    // This means the child can't expand beyond the value of the mMaxHeight
                    // attribute.
                    // To adjust the flex line length to the size of maxMainSize, remaining
                    // positive free space needs to be re-distributed to other flex items
                    // (children views). In that case, invoke this method again with the same
                    // fromIndex.
                    needsReexpand = true;
                    newHeight = flexItem->getMaxHeight();
                    mChildrenFrozen[index] = true;
                    flexLine.mTotalFlexGrow -= flexItem->getFlexGrow();
                } else {
                    accumulatedRoundError += (rawCalculatedHeight - newHeight);
                    if (accumulatedRoundError > 1.0) {
                        newHeight += 1;
                        accumulatedRoundError -= 1.0;
                    } else if (accumulatedRoundError < -1.0) {
                        newHeight -= 1;
                        accumulatedRoundError += 1.0;
                    }
                }
                const int childWidthMeasureSpec = getChildWidthMeasureSpecInternal(widthMeasureSpec,
                        flexItem, flexLine.mSumCrossSizeBefore);
                const int childHeightMeasureSpec = View::MeasureSpec::makeMeasureSpec(newHeight,
                        View::MeasureSpec::EXACTLY);
                child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
                childMeasuredWidth = child->getMeasuredWidth();
                childMeasuredHeight = child->getMeasuredHeight();
                updateMeasureCache(index, childWidthMeasureSpec, childHeightMeasureSpec,child);
                mFlexContainer->updateViewCache(index, child);
            }
            largestCrossSize = std::max(largestCrossSize, childMeasuredWidth
                    + flexItem->getMarginLeft() + flexItem->getMarginRight()
                    + mFlexContainer->getDecorationLengthCrossAxis(child));
            flexLine.mMainSize += childMeasuredHeight + flexItem->getMarginTop()
                    + flexItem->getMarginBottom();
        }
        flexLine.mCrossSize = std::max(flexLine.mCrossSize, largestCrossSize);
    }

    if (needsReexpand && sizeBeforeExpand != flexLine.mMainSize) {
        // Re-invoke the method with the same flex line to distribute the positive free space
        // that wasn't fully distributed (because of maximum length constraint)
        expandFlexItems(widthMeasureSpec, heightMeasureSpec, flexLine, maxMainSize,
                paddingAlongMainAxis, true);
    }
}

void FlexboxHelper::shrinkFlexItems(int widthMeasureSpec, int heightMeasureSpec, FlexLine flexLine,
        int maxMainSize, int paddingAlongMainAxis, bool calledRecursively) {
    int sizeBeforeShrink = flexLine.mMainSize;
    if (flexLine.mTotalFlexShrink <= 0 || maxMainSize > flexLine.mMainSize) {
        return;
    }
    bool needsReshrink = false;
    float unitShrink = (flexLine.mMainSize - maxMainSize) / flexLine.mTotalFlexShrink;
    float accumulatedRoundError = 0;
    flexLine.mMainSize = paddingAlongMainAxis + flexLine.mDividerLengthInMainSize;

    // Setting the cross size of the flex line as the temporal value since the cross size of
    // each flex item may be changed from the initial calculation
    // (in the measureHorizontal/measureVertical method) even this method is part of the main
    // size determination.
    // E.g. If a TextView's layout_width is set to 0dp, layout_height is set to wrap_content,
    // and layout_flexGrow is set to 1, the TextView is trying to expand to the vertical
    // direction to enclose its content (in the measureHorizontal method), but
    // the width will be expanded in this method. In that case, the height needs to be measured
    // again with the expanded width.
    int largestCrossSize = 0;
    if (!calledRecursively) {
        flexLine.mCrossSize = INT_MIN;
    }
    for (int i = 0; i < flexLine.mItemCount; i++) {
        int index = flexLine.mFirstIndex + i;
        View* child = mFlexContainer->getReorderedFlexItemAt(index);
        if (child == nullptr || child->getVisibility() == View::GONE) {
            continue;
        }
        FlexItem* flexItem = (FlexItem*) child->getLayoutParams();
        int flexDirection = mFlexContainer->getFlexDirection();
        if (flexDirection == FlexDirection::ROW || flexDirection == FlexDirection::ROW_REVERSE) {
            // The direction of main axis is horizontal

            int childMeasuredWidth = child->getMeasuredWidth();
            if (!mMeasuredSizeCache.empty()) {
                // Retrieve the measured width from the cache because there
                // are some cases that the view is re-created from the last measure, thus
                // View#getMeasuredWidth returns 0.
                // E.g. if the flex container is FlexboxLayoutManager, the case happens
                // frequently
                childMeasuredWidth = extractLowerInt(mMeasuredSizeCache[index]);
            }
            int childMeasuredHeight = child->getMeasuredHeight();
            if (!mMeasuredSizeCache.empty()){// != nullptr) {
                // Extract the measured height from the cache
                childMeasuredHeight = extractHigherInt(mMeasuredSizeCache[index]);
            }
            if (!mChildrenFrozen[index] && flexItem->getFlexShrink() > 0.f) {
                float rawCalculatedWidth = childMeasuredWidth - unitShrink * flexItem->getFlexShrink();
                if (i == flexLine.mItemCount - 1) {
                    rawCalculatedWidth += accumulatedRoundError;
                    accumulatedRoundError = 0;
                }
                int newWidth = std::round(rawCalculatedWidth);
                if (newWidth < flexItem->getMinWidth()) {
                    // This means the child doesn't have enough space to distribute the negative
                    // free space. To adjust the flex line length down to the maxMainSize,
                    // remaining
                    // negative free space needs to be re-distributed to other flex items
                    // (children views). In that case, invoke this method again with the same
                    // fromIndex.
                    needsReshrink = true;
                    newWidth = flexItem->getMinWidth();
                    mChildrenFrozen[index] = true;
                    flexLine.mTotalFlexShrink -= flexItem->getFlexShrink();
                } else {
                    accumulatedRoundError += (rawCalculatedWidth - newWidth);
                    if (accumulatedRoundError > 1.0) {
                        newWidth += 1;
                        accumulatedRoundError -= 1;
                    } else if (accumulatedRoundError < -1.0) {
                        newWidth -= 1;
                        accumulatedRoundError += 1;
                    }
                }
                const int childHeightMeasureSpec = getChildHeightMeasureSpecInternal(
                        heightMeasureSpec, flexItem, flexLine.mSumCrossSizeBefore);
                const int childWidthMeasureSpec = View::MeasureSpec::makeMeasureSpec(newWidth, View::MeasureSpec::EXACTLY);
                child->measure(childWidthMeasureSpec, childHeightMeasureSpec);

                childMeasuredWidth = child->getMeasuredWidth();
                childMeasuredHeight = child->getMeasuredHeight();
                updateMeasureCache(index, childWidthMeasureSpec, childHeightMeasureSpec,child);
                mFlexContainer->updateViewCache(index, child);
            }
            largestCrossSize = std::max(largestCrossSize, childMeasuredHeight +
                    flexItem->getMarginTop() + flexItem->getMarginBottom() +
                    mFlexContainer->getDecorationLengthCrossAxis(child));
            flexLine.mMainSize += childMeasuredWidth + flexItem->getMarginLeft() + flexItem->getMarginRight();
        } else {
            // The direction of main axis is vertical

            int childMeasuredHeight = child->getMeasuredHeight();
            if (!mMeasuredSizeCache.empty()) {
                // Retrieve the measured height from the cache because there
                // are some cases that the view is re-created from the last measure, thus
                // View#getMeasuredHeight returns 0.
                // E.g. if the flex container is FlexboxLayoutManager, that case happens
                // frequently
                childMeasuredHeight = extractHigherInt(mMeasuredSizeCache[index]);
            }
            int childMeasuredWidth = child->getMeasuredWidth();
            if (!mMeasuredSizeCache.empty()) {
                // Extract the measured width from the cache
                childMeasuredWidth = extractLowerInt(mMeasuredSizeCache[index]);
            }
            if (!mChildrenFrozen[index] && flexItem->getFlexShrink() > 0.f) {
                float rawCalculatedHeight = childMeasuredHeight
                        - unitShrink * flexItem->getFlexShrink();
                if (i == flexLine.mItemCount - 1) {
                    rawCalculatedHeight += accumulatedRoundError;
                    accumulatedRoundError = 0;
                }
                int newHeight = std::round(rawCalculatedHeight);
                if (newHeight < flexItem->getMinHeight()) {
                    // Need to invoke this method again like the case flex direction is vertical
                    needsReshrink = true;
                    newHeight = flexItem->getMinHeight();
                    mChildrenFrozen[index] = true;
                    flexLine.mTotalFlexShrink -= flexItem->getFlexShrink();
                } else {
                    accumulatedRoundError += (rawCalculatedHeight - newHeight);
                    if (accumulatedRoundError > 1.0) {
                        newHeight += 1;
                        accumulatedRoundError -= 1;
                    } else if (accumulatedRoundError < -1.0) {
                        newHeight -= 1;
                        accumulatedRoundError += 1;
                    }
                }
                const int childWidthMeasureSpec = getChildWidthMeasureSpecInternal(widthMeasureSpec,flexItem, flexLine.mSumCrossSizeBefore);
                const int childHeightMeasureSpec = View::MeasureSpec::makeMeasureSpec(newHeight, View::MeasureSpec::EXACTLY);
                child->measure(childWidthMeasureSpec, childHeightMeasureSpec);

                childMeasuredWidth = child->getMeasuredWidth();
                childMeasuredHeight = child->getMeasuredHeight();
                updateMeasureCache(index, childWidthMeasureSpec, childHeightMeasureSpec,child);
                mFlexContainer->updateViewCache(index, child);
            }
            largestCrossSize = std::max(largestCrossSize, childMeasuredWidth +
                    flexItem->getMarginLeft() + flexItem->getMarginRight() +
                    mFlexContainer->getDecorationLengthCrossAxis(child));
            flexLine.mMainSize += childMeasuredHeight + flexItem->getMarginTop()
                    + flexItem->getMarginBottom();
        }
        flexLine.mCrossSize = std::max(flexLine.mCrossSize, largestCrossSize);
    }

    if (needsReshrink && sizeBeforeShrink != flexLine.mMainSize) {
        // Re-invoke the method with the same fromIndex to distribute the negative free space
        // that wasn't fully distributed (because some views length were not enough)
        shrinkFlexItems(widthMeasureSpec, heightMeasureSpec, flexLine,
                maxMainSize, paddingAlongMainAxis, true);
    }
}

int FlexboxHelper::getChildWidthMeasureSpecInternal(int widthMeasureSpec,FlexItem* flexItem,int padding) {
    int childWidthMeasureSpec = mFlexContainer->getChildWidthMeasureSpec(widthMeasureSpec,
            mFlexContainer->getPaddingLeft() + mFlexContainer->getPaddingRight() +
                    flexItem->getMarginLeft() + flexItem->getMarginRight() + padding,
            flexItem->getWidth());
    const int childWidth = View::MeasureSpec::getSize(childWidthMeasureSpec);
    if (childWidth > flexItem->getMaxWidth()) {
        childWidthMeasureSpec = View::MeasureSpec::makeMeasureSpec(flexItem->getMaxWidth(),
                View::MeasureSpec::getMode(childWidthMeasureSpec));
    } else if (childWidth < flexItem->getMinWidth()) {
        childWidthMeasureSpec = View::MeasureSpec::makeMeasureSpec(flexItem->getMinWidth(),
                View::MeasureSpec::getMode(childWidthMeasureSpec));
    }
    return childWidthMeasureSpec;
}

int FlexboxHelper::getChildHeightMeasureSpecInternal(int heightMeasureSpec,FlexItem* flexItem,int padding) {
    int childHeightMeasureSpec = mFlexContainer->getChildHeightMeasureSpec(heightMeasureSpec,
            mFlexContainer->getPaddingTop() + mFlexContainer->getPaddingBottom()
                    + flexItem->getMarginTop() + flexItem->getMarginBottom() + padding,
            flexItem->getHeight());
    const int childHeight = View::MeasureSpec::getSize(childHeightMeasureSpec);
    if (childHeight > flexItem->getMaxHeight()) {
        childHeightMeasureSpec = View::MeasureSpec::makeMeasureSpec(flexItem->getMaxHeight(),
                View::MeasureSpec::getMode(childHeightMeasureSpec));
    } else if (childHeight < flexItem->getMinHeight()) {
        childHeightMeasureSpec = View::MeasureSpec::makeMeasureSpec(flexItem->getMinHeight(),
                View::MeasureSpec::getMode(childHeightMeasureSpec));
    }
    return childHeightMeasureSpec;
}

void FlexboxHelper::determineCrossSize(int widthMeasureSpec, int heightMeasureSpec,int paddingAlongCrossAxis) {
    // The MeasureSpec mode along the cross axis
    int mode;
    // The MeasureSpec size along the cross axis
    int size;
    FlexDirection flexDirection = (FlexDirection)mFlexContainer->getFlexDirection();
    switch (flexDirection) {
    case FlexDirection::ROW: // Intentional fall through
    case FlexDirection::ROW_REVERSE:
        mode = View::MeasureSpec::getMode(heightMeasureSpec);
        size = View::MeasureSpec::getSize(heightMeasureSpec);
        break;
    case FlexDirection::COLUMN: // Intentional fall through
    case FlexDirection::COLUMN_REVERSE:
        mode = View::MeasureSpec::getMode(widthMeasureSpec);
        size = View::MeasureSpec::getSize(widthMeasureSpec);
        break;
    default:
        LOGE("Invalid flex direction:%d" , flexDirection);
    }
    std::vector<FlexLine> flexLines = mFlexContainer->getFlexLinesInternal();
    if (mode == View::MeasureSpec::EXACTLY) {
        int totalCrossSize = mFlexContainer->getSumOfCrossSize() + paddingAlongCrossAxis;
        if (flexLines.size() == 1) {
            flexLines.at(0).mCrossSize = size - paddingAlongCrossAxis;
            // alignContent property is valid only if the Flexbox has at least two lines
        } else if (flexLines.size() >= 2) {
            switch ((AlignContent)mFlexContainer->getAlignContent()) {
            case AlignContent::STRETCH: {
                if (totalCrossSize >= size) {
                    break;
                }
                float freeSpaceUnit = (size - totalCrossSize) / (float) flexLines.size();
                float accumulatedError = 0;
                for (int i = 0, flexLinesSize = flexLines.size(); i < flexLinesSize; i++) {
                    FlexLine flexLine = flexLines.at(i);
                    float newCrossSizeAsFloat = flexLine.mCrossSize + freeSpaceUnit;
                    if (i == flexLines.size() - 1) {
                        newCrossSizeAsFloat += accumulatedError;
                        accumulatedError = 0;
                    }
                    int newCrossSize = std::round(newCrossSizeAsFloat);
                    accumulatedError += (newCrossSizeAsFloat - newCrossSize);
                    if (accumulatedError > 1) {
                        newCrossSize += 1;
                        accumulatedError -= 1;
                    } else if (accumulatedError < -1) {
                        newCrossSize -= 1;
                        accumulatedError += 1;
                    }
                    flexLine.mCrossSize = newCrossSize;
                }
                break;
            }
            case AlignContent::SPACE_AROUND: {
                if (totalCrossSize >= size) {
                    // If the size of the content is larger than the flex container, the
                    // Flex lines should be aligned center like ALIGN_CONTENT_CENTER
                    mFlexContainer->setFlexLines(
                            constructFlexLinesForAlignContentCenter(flexLines, size,
                                    totalCrossSize));
                    break;
                }
                // The value of free space along the cross axis which needs to be put on top
                // and below the bottom of each flex line.
                int spaceTopAndBottom = size - totalCrossSize;
                // The number of spaces along the cross axis
                int numberOfSpaces = flexLines.size() * 2;
                spaceTopAndBottom = spaceTopAndBottom / numberOfSpaces;
                std::vector<FlexLine> newFlexLines;// = new ArrayList<>();
                FlexLine dummySpaceFlexLine;// = new FlexLine();
                dummySpaceFlexLine.mCrossSize = spaceTopAndBottom;
                for (FlexLine flexLine : flexLines) {
                    newFlexLines.push_back(dummySpaceFlexLine);
                    newFlexLines.push_back(flexLine);
                    newFlexLines.push_back(dummySpaceFlexLine);
                }
                mFlexContainer->setFlexLines(newFlexLines);
                break;
            }
            case AlignContent::SPACE_BETWEEN: {
                if (totalCrossSize >= size) {
                    break;
                }
                // The value of free space along the cross axis between each flex line.
                float spaceBetweenFlexLine = size - totalCrossSize;
                int numberOfSpaces = flexLines.size() - 1;
                spaceBetweenFlexLine = spaceBetweenFlexLine / (float) numberOfSpaces;
                float accumulatedError = 0;
                std::vector<FlexLine> newFlexLines;// = new ArrayList<>();
                for (int i = 0, flexLineSize = flexLines.size(); i < flexLineSize; i++) {
                    FlexLine& flexLine = flexLines.at(i);
                    newFlexLines.push_back(flexLine);

                    if (i != flexLines.size() - 1) {
                        FlexLine dummySpaceFlexLine;// = new FlexLine();
                        if (i == flexLines.size() - 2) {
                            // The last dummy space block in the flex container.
                            // Adjust the cross size by the accumulated error.
                            dummySpaceFlexLine.mCrossSize = std::round(spaceBetweenFlexLine + accumulatedError);
                            accumulatedError = 0;
                        } else {
                            dummySpaceFlexLine.mCrossSize = std::round(spaceBetweenFlexLine);
                        }
                        accumulatedError += (spaceBetweenFlexLine
                                - dummySpaceFlexLine.mCrossSize);
                        if (accumulatedError > 1) {
                            dummySpaceFlexLine.mCrossSize += 1;
                            accumulatedError -= 1;
                        } else if (accumulatedError < -1) {
                            dummySpaceFlexLine.mCrossSize -= 1;
                            accumulatedError += 1;
                        }
                        newFlexLines.push_back(dummySpaceFlexLine);
                    }
                }
                mFlexContainer->setFlexLines(newFlexLines);
                break;
            }
            case AlignContent::CENTER: {
                mFlexContainer->setFlexLines(
                        constructFlexLinesForAlignContentCenter(flexLines, size,
                                totalCrossSize));
                break;
            }
            case AlignContent::FLEX_END: {
                int spaceTop = size - totalCrossSize;
                FlexLine dummySpaceFlexLine;// = new FlexLine();
                dummySpaceFlexLine.mCrossSize = spaceTop;
                flexLines.insert(flexLines.begin(), dummySpaceFlexLine);
                break;
            }
            case AlignContent::FLEX_START:
                // No op. Just to cover the available switch statement options
                break;
            }
        }
    }
}

std::vector<FlexLine> FlexboxHelper::constructFlexLinesForAlignContentCenter(std::vector<FlexLine>& flexLines,
        int size, int totalCrossSize) {
    int spaceAboveAndBottom = size - totalCrossSize;
    spaceAboveAndBottom = spaceAboveAndBottom / 2;
    std::vector<FlexLine> newFlexLines;// = new ArrayList<>();
    FlexLine dummySpaceFlexLine;// = new FlexLine();
    dummySpaceFlexLine.mCrossSize = spaceAboveAndBottom;
    for (int i = 0, flexLineSize = flexLines.size(); i < flexLineSize; i++) {
        if (i == 0) {
            newFlexLines.push_back(dummySpaceFlexLine);
        }
        FlexLine flexLine = flexLines.at(i);
        newFlexLines.push_back(flexLine);
        if (i == flexLines.size() - 1) {
            newFlexLines.push_back(dummySpaceFlexLine);
        }
    }
    return newFlexLines;
}

void FlexboxHelper::stretchViews() {
    stretchViews(0);
}

void FlexboxHelper::stretchViews(int fromIndex) {
    if (fromIndex >= mFlexContainer->getFlexItemCount()) {
        return;
    }
    int flexDirection = mFlexContainer->getFlexDirection();
    if (mFlexContainer->getAlignItems() == (int)AlignItems::STRETCH) {
        int flexLineIndex = 0;
        if (!mIndexToFlexLine.empty()) {
            flexLineIndex = mIndexToFlexLine[fromIndex];
        }
        std::vector<FlexLine> flexLines = mFlexContainer->getFlexLinesInternal();
        for (int i = flexLineIndex, size = flexLines.size(); i < size; i++) {
            FlexLine flexLine = flexLines.at(i);
            for (int j = 0, itemCount = flexLine.mItemCount; j < itemCount; j++) {
                int viewIndex = flexLine.mFirstIndex + j;
                if (j >= mFlexContainer->getFlexItemCount()) {
                    continue;
                }
                View* view = mFlexContainer->getReorderedFlexItemAt(viewIndex);
                if (view == nullptr || view->getVisibility() == View::GONE) {
                    continue;
                }
                FlexItem* flexItem = (FlexItem*) view->getLayoutParams();
                if (flexItem->getAlignSelf() != (int)AlignSelf::AUTO &&
                        flexItem->getAlignSelf() != (int)AlignItems::STRETCH) {
                    continue;
                }
                switch (flexDirection) {
                case FlexDirection::ROW: // Intentional fall through
                case FlexDirection::ROW_REVERSE:
                    stretchViewVertically(view, flexLine.mCrossSize, viewIndex);
                    break;
                case FlexDirection::COLUMN:
                case FlexDirection::COLUMN_REVERSE:
                    stretchViewHorizontally(view, flexLine.mCrossSize, viewIndex);
                    break;
                default:
                    LOGE("Invalid flex direction:%d" , flexDirection);
                }
            }
        }
    } else {
        for (FlexLine flexLine : mFlexContainer->getFlexLinesInternal()) {
            for (int index : flexLine.mIndicesAlignSelfStretch) {
                View* view = mFlexContainer->getReorderedFlexItemAt(index);
                switch (flexDirection) {
                case FlexDirection::ROW: // Intentional fall through
                case FlexDirection::ROW_REVERSE:
                    stretchViewVertically(view, flexLine.mCrossSize, index);
                    break;
                case FlexDirection::COLUMN:
                case FlexDirection::COLUMN_REVERSE:
                    stretchViewHorizontally(view, flexLine.mCrossSize, index);
                    break;
                default:
                    LOGE("Invalid flex direction:%d" , flexDirection);
                }
            }
        }
    }
}

void FlexboxHelper::stretchViewVertically(View* view, int crossSize, int index) {
    FlexItem* flexItem = (FlexItem*) view->getLayoutParams();
    int newHeight = crossSize - flexItem->getMarginTop() - flexItem->getMarginBottom() -
            mFlexContainer->getDecorationLengthCrossAxis(view);
    newHeight = std::max(newHeight, flexItem->getMinHeight());
    newHeight = std::min(newHeight, flexItem->getMaxHeight());
    int childWidthSpec;
    int measuredWidth;
    if (!mMeasuredSizeCache.empty()) {
        // Retrieve the measured height from the cache because there
        // are some cases that the view is re-created from the last measure, thus
        // View#getMeasuredHeight returns 0.
        // E.g. if the flex container is FlexboxLayoutManager, that case happens
        // frequently
        measuredWidth = extractLowerInt(mMeasuredSizeCache[index]);
    } else {
        measuredWidth = view->getMeasuredWidth();
    }
    childWidthSpec = MeasureSpec::makeMeasureSpec(measuredWidth,MeasureSpec::EXACTLY);

    int childHeightSpec = MeasureSpec::makeMeasureSpec(newHeight, MeasureSpec::EXACTLY);
    view->measure(childWidthSpec, childHeightSpec);

    updateMeasureCache(index, childWidthSpec, childHeightSpec, view);
    mFlexContainer->updateViewCache(index, view);
}

void FlexboxHelper::stretchViewHorizontally(View* view, int crossSize, int index) {
    FlexItem* flexItem = (FlexItem*) view->getLayoutParams();
    int newWidth = crossSize - flexItem->getMarginLeft() - flexItem->getMarginRight()
            - mFlexContainer->getDecorationLengthCrossAxis(view);
    newWidth = std::max(newWidth, flexItem->getMinWidth());
    newWidth = std::min(newWidth, flexItem->getMaxWidth());
    int childHeightSpec;
    int measuredHeight;
    if (!mMeasuredSizeCache.empty()) {
        // Retrieve the measured height from the cache because there
        // are some cases that the view is re-created from the last measure, thus
        // View#getMeasuredHeight returns 0.
        // E.g. if the flex container is FlexboxLayoutManager, that case happens
        // frequently
        measuredHeight = extractHigherInt(mMeasuredSizeCache[index]);
    } else {
        measuredHeight = view->getMeasuredHeight();
    }
    childHeightSpec = MeasureSpec::makeMeasureSpec(measuredHeight,MeasureSpec::EXACTLY);
    int childWidthSpec = MeasureSpec::makeMeasureSpec(newWidth, MeasureSpec::EXACTLY);
    view->measure(childWidthSpec, childHeightSpec);

    updateMeasureCache(index, childWidthSpec, childHeightSpec, view);
    mFlexContainer->updateViewCache(index, view);
}

void FlexboxHelper::layoutSingleChildHorizontal(View* view, FlexLine flexLine, int left, int top, int width, int height) {
    FlexItem* flexItem = (FlexItem*) view->getLayoutParams();
    int alignItems = mFlexContainer->getAlignItems();
    if (flexItem->getAlignSelf() != (int)AlignSelf::AUTO) {
        // Expecting the values for alignItems and mAlignSelf match except for ALIGN_SELF_AUTO.
        // Assigning the mAlignSelf value as alignItems should work.
        alignItems = flexItem->getAlignSelf();
    }
    int crossSize = flexLine.mCrossSize;
    switch ((AlignItems)alignItems) {
    case AlignItems::FLEX_START: // Intentional fall through
    case AlignItems::STRETCH:
        if (mFlexContainer->getFlexWrap() != (int)FlexWrap::WRAP_REVERSE) {
            view->layout(left, top + flexItem->getMarginTop(), width,height);
        } else {
            view->layout(left, top - flexItem->getMarginBottom(), width,height);
        }
        break;
    case AlignItems::BASELINE:
        if (mFlexContainer->getFlexWrap() != (int)FlexWrap::WRAP_REVERSE) {
            int marginTop = flexLine.mMaxBaseline - view->getBaseline();
            marginTop = std::max(marginTop, flexItem->getMarginTop());
            view->layout(left, top + marginTop, width, height);
        } else {
            int marginBottom = flexLine.mMaxBaseline - view->getMeasuredHeight() + view->getBaseline();
            marginBottom = std::max(marginBottom, flexItem->getMarginBottom());
            view->layout(left, top - marginBottom, width, height);
        }
        break;
    case AlignItems::FLEX_END:
        if (mFlexContainer->getFlexWrap() != (int)FlexWrap::WRAP_REVERSE) {
            view->layout(left,top + crossSize - view->getMeasuredHeight() - flexItem->getMarginBottom(),width,height);
                    //right, top + crossSize - flexItem->getMarginBottom());
        } else {
            // If the flexWrap == WRAP_REVERSE, the direction of the
            // flexEnd is flipped (from top to bottom).
            view->layout(left, top - crossSize + view->getMeasuredHeight() + flexItem->getMarginTop(), width, height);
        }
        break;
    case AlignItems::CENTER:
        int topFromCrossAxis = (crossSize - view->getMeasuredHeight()
                + flexItem->getMarginTop() - flexItem->getMarginBottom()) / 2;
        if (mFlexContainer->getFlexWrap() != (int)FlexWrap::WRAP_REVERSE) {
            view->layout(left, top + topFromCrossAxis, width, view->getMeasuredHeight());
        } else {
            view->layout(left, top - topFromCrossAxis, width, view->getMeasuredHeight());
        }
        break;
    }
}

void FlexboxHelper::layoutSingleChildVertical(View* view, FlexLine flexLine, bool isRtl,
        int left, int top, int width, int height) {
    FlexItem* flexItem = (FlexItem*) view->getLayoutParams();
    int alignItems = mFlexContainer->getAlignItems();
    if (flexItem->getAlignSelf() != (int)AlignSelf::AUTO) {
        // Expecting the values for alignItems and mAlignSelf match except for ALIGN_SELF_AUTO.
        // Assigning the mAlignSelf value as alignItems should work.
        alignItems = flexItem->getAlignSelf();
    }
    int crossSize = flexLine.mCrossSize;
    switch ((AlignItems)alignItems) {
    case AlignItems::FLEX_START: // Intentional fall through
    case AlignItems::STRETCH: // Intentional fall through
    case AlignItems::BASELINE:
        if (!isRtl) {
            view->layout(left + flexItem->getMarginLeft(), top,width,height);//right + flexItem->getMarginLeft(), bottom);
        } else {
            view->layout(left - flexItem->getMarginRight(), top,width,height);//right - flexItem->getMarginRight(), bottom);
        }
        break;
    case AlignItems::FLEX_END:
        if (!isRtl) {
            view->layout(left + crossSize - view->getMeasuredWidth() - flexItem->getMarginRight(), top,width,height);
                    //right + crossSize - view->getMeasuredWidth() - flexItem->getMarginRight(), bottom);
        } else {
            // If the flexWrap == WRAP_REVERSE, the direction of the
            // flexEnd is flipped (from left to right).
            view->layout(left - crossSize + view->getMeasuredWidth() + flexItem->getMarginLeft(), top,width,height);
                    //right - crossSize + view->getMeasuredWidth() + flexItem->getMarginLeft(), bottom);
        }
        break;
    case AlignItems::CENTER:
        ViewGroup::MarginLayoutParams* lp = (ViewGroup::MarginLayoutParams*)view->getLayoutParams();
        int leftFromCrossAxis = (crossSize - view->getMeasuredWidth()
                + lp->getMarginStart() - lp->getMarginEnd()) / 2;
        if (!isRtl) {
            view->layout(left + leftFromCrossAxis, top, width,height);//right + leftFromCrossAxis, bottom);
        } else {
            view->layout(left - leftFromCrossAxis, top, width,height);//right - leftFromCrossAxis, bottom);
        }
        break;
    }
}

void FlexboxHelper::ensureMeasuredSizeCache(int size) {
    if (mMeasuredSizeCache.size() < size) {
        const size_t newCapacity = std::max(mMeasuredSizeCache.size() * 2,size_t(size));
        mMeasuredSizeCache.resize(newCapacity);
    }
}

void FlexboxHelper::ensureMeasureSpecCache(int size) {
    if (mMeasureSpecCache.size() < size) {
        const size_t newCapacity = std::max(mMeasureSpecCache.size() * 2,size_t(size));
        mMeasureSpecCache.resize(newCapacity);
    }
}

int FlexboxHelper::extractLowerInt(int64_t longValue) {
    return (int) longValue;
}

int FlexboxHelper::extractHigherInt(int64_t longValue) {
    return (int) (longValue >> 32);
}

long FlexboxHelper::makeCombinedLong(int widthMeasureSpec, int heightMeasureSpec) {
    // Suppress sign extension for the low bytes
    return (int64_t) heightMeasureSpec << 32 | (int64_t) widthMeasureSpec & MEASURE_SPEC_WIDTH_MASK;
}

void FlexboxHelper::updateMeasureCache(int index, int widthMeasureSpec, int heightMeasureSpec,View* view) {
    if (!mMeasureSpecCache.empty()) {
        mMeasureSpecCache[index] = makeCombinedLong(widthMeasureSpec,heightMeasureSpec);
    }
    if (!mMeasuredSizeCache.empty()) {
        mMeasuredSizeCache[index] = makeCombinedLong(
                view->getMeasuredWidth(),view->getMeasuredHeight());
    }
}

void FlexboxHelper::ensureIndexToFlexLine(int size) {
    if (mIndexToFlexLine.size() < size) {
        const size_t newCapacity = std::max(mIndexToFlexLine.size() * 2,size_t(size));
        mIndexToFlexLine.resize(newCapacity);
    }
}

void FlexboxHelper::clearFlexLines(std::vector<FlexLine>& flexLines, int fromFlexItem) {

    int fromFlexLine = mIndexToFlexLine[fromFlexItem];
    if (fromFlexLine == (int)NO_POSITION) {
        fromFlexLine = 0;
    }

    // Deleting from the last to avoid unneeded copy it happens when deleting the middle of the
    // item in the ArrayList
    for (int i = flexLines.size() - 1; i >= fromFlexLine; i--) {
        flexLines.erase(flexLines.begin()+i);
    }

    int fillTo = mIndexToFlexLine.size() - 1;
    if (fromFlexItem > fillTo) {
        std::fill(mIndexToFlexLine.begin(),mIndexToFlexLine.end(), (int)NO_POSITION);
    } else {
        std::fill(mIndexToFlexLine.begin()+ fromFlexItem, mIndexToFlexLine.begin()+fillTo, (int)NO_POSITION);
    }

    fillTo = mMeasureSpecCache.size() - 1;
    if (fromFlexItem > fillTo) {
        std::fill(mMeasureSpecCache.begin(),mMeasureSpecCache.end(), 0);
    } else {
        std::fill(mMeasureSpecCache.begin()+fromFlexItem, mMeasureSpecCache.begin()+fillTo, 0);
    }
}

}/*endof namespace*/
