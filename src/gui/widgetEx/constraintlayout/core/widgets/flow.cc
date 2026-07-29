/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.Flow.
 */
#include <widgetEx/constraintlayout/core/widgets/flow.h>

#include <algorithm>
#include <climits>
#include <porting/cdlog.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.h>

namespace cdroid::clcore {

// out-of-line definitions (odr-used as map values in the widget layer)
const int Flow::HORIZONTAL_ALIGN_START;
const int Flow::HORIZONTAL_ALIGN_END;
const int Flow::HORIZONTAL_ALIGN_CENTER;
const int Flow::VERTICAL_ALIGN_TOP;
const int Flow::VERTICAL_ALIGN_BOTTOM;
const int Flow::VERTICAL_ALIGN_CENTER;
const int Flow::VERTICAL_ALIGN_BASELINE;
const int Flow::WRAP_NONE;
const int Flow::WRAP_CHAIN;
const int Flow::WRAP_ALIGNED;
const int Flow::WRAP_CHAIN_NEW;

Flow::Flow() {
    setType("Flow");
}

void Flow::copy(ConstraintWidget* src,
                std::unordered_map<ConstraintWidget*, ConstraintWidget*>& map) {
    VirtualLayout::copy(src, map);
    // Flow's config fields are not exercised by the copy path in the MVP; faithful TODO.
}

std::string Flow::getType() const {
    return "Flow";
}

// --- setters ---
void Flow::setOrientation(int value) {
    mOrientation = value;
}
void Flow::setHorizontalStyle(int value) {
    mHorizontalStyle = value;
}
void Flow::setVerticalStyle(int value) {
    mVerticalStyle = value;
}
void Flow::setFirstHorizontalStyle(int value) {
    mFirstHorizontalStyle = value;
}
void Flow::setFirstVerticalStyle(int value) {
    mFirstVerticalStyle = value;
}
void Flow::setLastHorizontalStyle(int value) {
    mLastHorizontalStyle = value;
}
void Flow::setLastVerticalStyle(int value) {
    mLastVerticalStyle = value;
}
void Flow::setHorizontalBias(float value) {
    mHorizontalBias = value;
}
void Flow::setVerticalBias(float value) {
    mVerticalBias = value;
}
void Flow::setFirstHorizontalBias(float value) {
    mFirstHorizontalBias = value;
}
void Flow::setFirstVerticalBias(float value) {
    mFirstVerticalBias = value;
}
void Flow::setLastHorizontalBias(float value) {
    mLastHorizontalBias = value;
}
void Flow::setLastVerticalBias(float value) {
    mLastVerticalBias = value;
}
void Flow::setHorizontalAlign(int value) {
    mHorizontalAlign = value;
}
void Flow::setVerticalAlign(int value) {
    mVerticalAlign = value;
}
void Flow::setWrapMode(int value) {
    mWrapMode = value;
}
void Flow::setHorizontalGap(int value) {
    mHorizontalGap = value;
}
void Flow::setVerticalGap(int value) {
    mVerticalGap = value;
}
void Flow::setMaxElementsWrap(int value) {
    mMaxElementsWrap = value;
}

int Flow::getWidgetWidth(ConstraintWidget* widget, int max) {
    if (widget == nullptr) return 0;
    if (widget->getHorizontalDimensionBehaviour() == DimensionBehaviour::MATCH_CONSTRAINT) {
        if (widget->mMatchConstraintDefaultWidth == MATCH_CONSTRAINT_SPREAD) {
            return 0;
        } else if (widget->mMatchConstraintDefaultWidth == MATCH_CONSTRAINT_PERCENT) {
            int value = (int) (widget->mMatchConstraintPercentWidth * max);
            if (value != widget->getWidth()) {
                widget->setMeasureRequested(true);
                measure(widget, DimensionBehaviour::FIXED, value,
                        widget->getVerticalDimensionBehaviour(), widget->getHeight());
            }
            return value;
        } else if (widget->mMatchConstraintDefaultWidth == MATCH_CONSTRAINT_WRAP) {
            return widget->getWidth();
        } else if (widget->mMatchConstraintDefaultWidth == MATCH_CONSTRAINT_RATIO) {
            return (int) (widget->getHeight() * widget->mDimensionRatio + 0.5f);
        }
    }
    return widget->getWidth();
}

int Flow::getWidgetHeight(ConstraintWidget* widget, int max) {
    if (widget == nullptr) return 0;
    if (widget->getVerticalDimensionBehaviour() == DimensionBehaviour::MATCH_CONSTRAINT) {
        if (widget->mMatchConstraintDefaultHeight == MATCH_CONSTRAINT_SPREAD) {
            return 0;
        } else if (widget->mMatchConstraintDefaultHeight == MATCH_CONSTRAINT_PERCENT) {
            int value = (int) (widget->mMatchConstraintPercentHeight * max);
            if (value != widget->getHeight()) {
                widget->setMeasureRequested(true);
                measure(widget, widget->getHorizontalDimensionBehaviour(), widget->getWidth(),
                        DimensionBehaviour::FIXED, value);
            }
            return value;
        } else if (widget->mMatchConstraintDefaultHeight == MATCH_CONSTRAINT_WRAP) {
            return widget->getHeight();
        } else if (widget->mMatchConstraintDefaultHeight == MATCH_CONSTRAINT_RATIO) {
            return (int) (widget->getWidth() * widget->mDimensionRatio + 0.5f);
        }
    }
    return widget->getHeight();
}

void Flow::measure(int widthMode, int widthSize, int heightMode, int heightSize) {
    if (!mWidgets.empty() && !measureChildren()) {
        setMeasure(0, 0);
        needsCallbackFromSolver(false);
        return;
    }
    int paddingLeft = getPaddingLeft(), paddingRight = getPaddingRight();
    int paddingTop = getPaddingTop(), paddingBottom = getPaddingBottom();

    std::vector<int> measured = {0, 0};
    int max;
    if (mOrientation == VERTICAL) {
        // Wrap direction is height; WRAP_CONTENT (UNSPECIFIED) → unbounded, single column.
        max = (heightMode == BasicMeasure::UNSPECIFIED) ? INT_MAX
              : heightSize - paddingTop - paddingBottom;
    } else {
        // Wrap direction is width; WRAP_CONTENT (UNSPECIFIED) → unbounded, single row.
        max = (widthMode == BasicMeasure::UNSPECIFIED) ? INT_MAX
              : widthSize - paddingLeft - paddingRight;
    }

    if (mHorizontalStyle == UNKNOWN) mHorizontalStyle = CHAIN_SPREAD;
    if (mVerticalStyle == UNKNOWN)   mVerticalStyle = CHAIN_SPREAD;

    std::vector<ConstraintWidget*>& widgets = mWidgets;
    int gone = 0;
    for (ConstraintWidget* w : widgets) {
        if (w->getVisibility() == GONE) gone++;
    }
    int count = (int) widgets.size();
    if (gone > 0) {
        std::vector<ConstraintWidget*> visible;
        visible.reserve(widgets.size() - gone);
        for (ConstraintWidget* w : widgets) {
            if (w->getVisibility() != GONE) visible.push_back(w);
        }
        mDisplayedWidgets = visible;
        count = (int) visible.size();
    } else {
        mDisplayedWidgets = widgets;
    }
    mDisplayedWidgetsCount = count;

    switch (mWrapMode) {
    case WRAP_ALIGNED:
        measureAligned(mDisplayedWidgets, count, mOrientation, max, measured);
        break;
    case WRAP_CHAIN:
        measureChainWrap(mDisplayedWidgets, count, mOrientation, max, measured);
        break;
    case WRAP_NONE:
        measureNoWrap(mDisplayedWidgets, count, mOrientation, max, measured);
        break;
    case WRAP_CHAIN_NEW:
        measureChainWrap_new(mDisplayedWidgets, count, mOrientation, max, measured);
        break;
    default:
        measureChainWrap(mDisplayedWidgets, count, mOrientation, max, measured);
        break;
    }

    int width = measured[HORIZONTAL] + paddingLeft + paddingRight;
    int height = measured[VERTICAL] + paddingTop + paddingBottom;
    int measuredWidth = 0, measuredHeight = 0;
    if (widthMode == BasicMeasure::EXACTLY) measuredWidth = widthSize;
    else if (widthMode == BasicMeasure::AT_MOST) measuredWidth = std::min(width, widthSize);
    else measuredWidth = width;
    if (heightMode == BasicMeasure::EXACTLY) measuredHeight = heightSize;
    else if (heightMode == BasicMeasure::AT_MOST) measuredHeight = std::min(height, heightSize);
    else measuredHeight = height;

    setMeasure(measuredWidth, measuredHeight);
    setWidth(measuredWidth);
    setHeight(measuredHeight);
    needsCallbackFromSolver(!mWidgets.empty());
}

void Flow::measureChainWrap(std::vector<ConstraintWidget*>& widgets, int count, int orientation,
                            int max, std::vector<int>& measured) {
    if (count == 0) return;
    mChainList.clear();
    WidgetsList list(this, orientation, &mLeft, &mTop, &mRight, &mBottom, max);
    mChainList.push_back(list);
    WidgetsList* currentList = &mChainList.back();

    int nbMatchConstraintsWidgets = 0;
    if (orientation == HORIZONTAL) {
        int width = 0;
        for (int i = 0; i < count; i++) {
            ConstraintWidget* widget = widgets[i];
            int w = getWidgetWidth(widget, max);
            if (widget->getHorizontalDimensionBehaviour() == DimensionBehaviour::MATCH_CONSTRAINT) {
                nbMatchConstraintsWidgets++;
            }
            bool doWrap = (width == max || (width + mHorizontalGap + w) > max) && currentList->mBiggest != nullptr;
            if (!doWrap && i > 0 && mMaxElementsWrap > 0 && (i % mMaxElementsWrap == 0)) doWrap = true;
            if (doWrap) {
                width = w;
                mChainList.push_back(WidgetsList(this, orientation, &mLeft, &mTop, &mRight, &mBottom, max));
                mChainList.back().setStartIndex(i);
                currentList = &mChainList.back();
            } else {
                width = (i > 0) ? (width + mHorizontalGap + w) : w;
            }
            currentList->add(widget);
        }
    } else {
        int height = 0;
        for (int i = 0; i < count; i++) {
            ConstraintWidget* widget = widgets[i];
            int h = getWidgetHeight(widget, max);
            if (widget->getVerticalDimensionBehaviour() == DimensionBehaviour::MATCH_CONSTRAINT) {
                nbMatchConstraintsWidgets++;
            }
            bool doWrap = (height == max || (height + mVerticalGap + h) > max) && currentList->mBiggest != nullptr;
            if (!doWrap && i > 0 && mMaxElementsWrap > 0 && (i % mMaxElementsWrap == 0)) doWrap = true;
            if (doWrap) {
                height = h;
                mChainList.push_back(WidgetsList(this, orientation, &mLeft, &mTop, &mRight, &mBottom, max));
                mChainList.back().setStartIndex(i);
                currentList = &mChainList.back();
            } else {
                height = (i > 0) ? (height + mVerticalGap + h) : h;
            }
            currentList->add(widget);
        }
    }

    const int listCount = (int) mChainList.size();
    ConstraintAnchor* left = &mLeft;
    ConstraintAnchor* top = &mTop;
    ConstraintAnchor* right = &mRight;
    ConstraintAnchor* bottom = &mBottom;
    int paddingLeft = getPaddingLeft(), paddingTop = getPaddingTop();
    int paddingRight = getPaddingRight(), paddingBottom = getPaddingBottom();
    int maxWidth = 0, maxHeight = 0;

    bool needInternalMeasure = getHorizontalDimensionBehaviour() == DimensionBehaviour::WRAP_CONTENT
                               || getVerticalDimensionBehaviour() == DimensionBehaviour::WRAP_CONTENT;
    if (nbMatchConstraintsWidgets > 0 && needInternalMeasure) {
        for (int i = 0; i < listCount; i++) {
            if (orientation == HORIZONTAL) {
                mChainList[i].measureMatchConstraints(max - mChainList[i].getWidth());
            } else {
                mChainList[i].measureMatchConstraints(max - mChainList[i].getHeight());
            }
        }
    }

    for (int i = 0; i < listCount; i++) {
        WidgetsList& current = mChainList[i];
        if (orientation == HORIZONTAL) {
            if (i < listCount - 1) {
                bottom = &mChainList[i + 1].mBiggest->mTop;
                paddingBottom = 0;
            } else {
                bottom = &mBottom;
                paddingBottom = getPaddingBottom();
            }
            ConstraintAnchor* currentBottom = &current.mBiggest->mBottom;
            current.setup(orientation, left, top, right, bottom,
                          paddingLeft, paddingTop, paddingRight, paddingBottom, max);
            top = currentBottom;
            paddingTop = 0;
            maxWidth = std::max(maxWidth, current.getWidth());
            maxHeight += current.getHeight();
            if (i > 0) maxHeight += mVerticalGap;
        } else {
            if (i < listCount - 1) {
                right = &mChainList[i + 1].mBiggest->mLeft;
                paddingRight = 0;
            } else {
                right = &mRight;
                paddingRight = getPaddingRight();
            }
            ConstraintAnchor* currentRight = &current.mBiggest->mRight;
            current.setup(orientation, left, top, right, bottom,
                          paddingLeft, paddingTop, paddingRight, paddingBottom, max);
            left = currentRight;
            paddingLeft = 0;
            maxWidth += current.getWidth();
            maxHeight = std::max(maxHeight, current.getHeight());
            if (i > 0) maxWidth += mHorizontalGap;
        }
    }
    measured[HORIZONTAL] = maxWidth;
    measured[VERTICAL] = maxHeight;
}

void Flow::measureNoWrap(std::vector<ConstraintWidget*>& /*widgets*/, int /*count*/, int /*orientation*/,
                         int /*max*/, std::vector<int>& /*measured*/) {
    // TODO: faithful single-row (no-wrap) measure. Falls back to chain-wrap behavior is incorrect
    // for WRAP_NONE; deferred until the exotic wrap modes are ported.
    LOGW("Flow: WRAP_NONE measure not yet implemented");
}

void Flow::measureAligned(std::vector<ConstraintWidget*>& /*widgets*/, int /*count*/, int /*orientation*/,
                          int /*max*/, std::vector<int>& /*measured*/) {
    LOGW("Flow: WRAP_ALIGNED measure not yet implemented");
}

void Flow::measureChainWrap_new(std::vector<ConstraintWidget*>& /*widgets*/, int /*count*/, int /*orientation*/,
                                int /*max*/, std::vector<int>& /*measured*/) {
    LOGW("Flow: WRAP_CHAIN_NEW measure not yet implemented");
}

void Flow::addToSolver(LinearSystem* system, bool optimize) {
    // The row layout (mChainList) is built by Flow.measure(), which the container's BasicMeasure
    // VirtualLayout block calls AFTER the solver resolved this Flow's 0dp size (see
    // BasicMeasure::solverMeasure). Here we only emit the row constraints for the already-built
    // mChainList — matching Android's core Flow.addToSolver (constraint emission, not measure).
    VirtualLayout::addToSolver(system, optimize);

    bool isInRtl = false;
    if (auto* container = dynamic_cast<ConstraintWidgetContainer*>(getParent())) {
        isInRtl = container->isRtl();
    }
    switch (mWrapMode) {
    case WRAP_CHAIN:
    case WRAP_CHAIN_NEW: {
        const int count = (int) mChainList.size();
        for (int i = 0; i < count; i++) {
            mChainList[i].createConstraints(isInRtl, i, i == count - 1);
        }
        break;
    }
    case WRAP_NONE: {
        if (!mChainList.empty()) {
            mChainList[0].createConstraints(isInRtl, 0, true);
        }
        break;
    }
    case WRAP_ALIGNED: {
        // TODO: createAlignedConstraints (grid-aligned layout).
        const int count = (int) mChainList.size();
        for (int i = 0; i < count; i++) {
            mChainList[i].createConstraints(isInRtl, i, i == count - 1);
        }
        break;
    }
    }
    needsCallbackFromSolver(false);
}

// ===========================================================================
// Flow::WidgetsList
// ===========================================================================
Flow::WidgetsList::WidgetsList(Flow* flow, int orientation,
                               ConstraintAnchor* left, ConstraintAnchor* top,
                               ConstraintAnchor* right, ConstraintAnchor* bottom, int max)
    : mLeft(left), mTop(top), mRight(right), mBottom(bottom),
      mPaddingLeft(flow->getPaddingLeft()), mPaddingTop(flow->getPaddingTop()),
      mPaddingRight(flow->getPaddingRight()), mPaddingBottom(flow->getPaddingBottom()),
      mMax(max), mFlow(flow), mOrientation(orientation) {}

void Flow::WidgetsList::setup(int orientation, ConstraintAnchor* left, ConstraintAnchor* top,
                              ConstraintAnchor* right, ConstraintAnchor* bottom,
                              int paddingLeft, int paddingTop, int paddingRight, int paddingBottom, int max) {
    mOrientation = orientation;
    mLeft = left;
    mTop = top;
    mRight = right;
    mBottom = bottom;
    mPaddingLeft = paddingLeft;
    mPaddingTop = paddingTop;
    mPaddingRight = paddingRight;
    mPaddingBottom = paddingBottom;
    mMax = max;
}

void Flow::WidgetsList::clear() {
    mBiggestDimension = 0;
    mBiggest = nullptr;
    mWidth = 0;
    mHeight = 0;
    mStartIndex = 0;
    mCount = 0;
    mNbMatchConstraintsWidgets = 0;
}
void Flow::WidgetsList::setStartIndex(int value) {
    mStartIndex = value;
}
int  Flow::WidgetsList::getWidth() const {
    return (mOrientation == HORIZONTAL) ? (mWidth - mFlow->mHorizontalGap) : mWidth;
}
int  Flow::WidgetsList::getHeight() const {
    return (mOrientation == VERTICAL) ? (mHeight - mFlow->mVerticalGap) : mHeight;
}

void Flow::WidgetsList::add(ConstraintWidget* widget) {
    if (mOrientation == HORIZONTAL) {
        int width = mFlow->getWidgetWidth(widget, mMax);
        if (widget->getHorizontalDimensionBehaviour() == DimensionBehaviour::MATCH_CONSTRAINT) {
            mNbMatchConstraintsWidgets++;
            width = 0;
        }
        int gap = (widget->getVisibility() == GONE) ? 0 : mFlow->mHorizontalGap;
        mWidth += width + gap;
        int height = mFlow->getWidgetHeight(widget, mMax);
        if (mBiggest == nullptr || mBiggestDimension < height) {
            mBiggest = widget;
            mBiggestDimension = height;
            mHeight = height;
        }
    } else {
        int width = mFlow->getWidgetWidth(widget, mMax);
        int height = mFlow->getWidgetHeight(widget, mMax);
        if (widget->getVerticalDimensionBehaviour() == DimensionBehaviour::MATCH_CONSTRAINT) {
            mNbMatchConstraintsWidgets++;
            height = 0;
        }
        int gap = (widget->getVisibility() == GONE) ? 0 : mFlow->mVerticalGap;
        mHeight += height + gap;
        if (mBiggest == nullptr || mBiggestDimension < width) {
            mBiggest = widget;
            mBiggestDimension = width;
            mWidth = width;
        }
    }
    mCount++;
}

void Flow::WidgetsList::createConstraints(bool isInRtl, int chainIndex, bool isLastChain) {
    const int count = mCount;
    for (int i = 0; i < count; i++) {
        if (mStartIndex + i >= mFlow->mDisplayedWidgetsCount) break;
        ConstraintWidget* widget = mFlow->mDisplayedWidgets[mStartIndex + i];
        if (widget != nullptr) widget->resetAnchors();
    }
    if (count == 0 || mBiggest == nullptr) return;

    bool singleChain = isLastChain && chainIndex == 0;
    int firstVisible = -1, lastVisible = -1;
    for (int i = 0; i < count; i++) {
        int index = isInRtl ? (count - 1 - i) : i;
        if (mStartIndex + index >= mFlow->mDisplayedWidgetsCount) break;
        ConstraintWidget* widget = mFlow->mDisplayedWidgets[mStartIndex + index];
        if (widget != nullptr && widget->getVisibility() == VISIBLE) {
            if (firstVisible == -1) firstVisible = i;
            lastVisible = i;
        }
    }

    ConstraintWidget* previous = nullptr;
    if (mOrientation == HORIZONTAL) {
        ConstraintWidget* verticalWidget = mBiggest;
        verticalWidget->mVerticalChainStyle = mFlow->mVerticalStyle;
        int padding = mPaddingTop;
        if (chainIndex > 0) padding += mFlow->mVerticalGap;
        verticalWidget->mTop.connect(mTop, padding);
        if (isLastChain) verticalWidget->mBottom.connect(mBottom, mPaddingBottom);
        if (chainIndex > 0) {
            ConstraintAnchor& rowBottom = mTop->getOwner()->mBottom;
            rowBottom.connect(verticalWidget->mTop, 0);
        }
        ConstraintWidget* baselineVerticalWidget = verticalWidget;
        if (mFlow->mVerticalAlign == VERTICAL_ALIGN_BASELINE && !verticalWidget->hasBaseline()) {
            for (int i = 0; i < count; i++) {
                int index = isInRtl ? (count - 1 - i) : i;
                if (mStartIndex + index >= mFlow->mDisplayedWidgetsCount) break;
                ConstraintWidget* widget = mFlow->mDisplayedWidgets[mStartIndex + index];
                if (widget->hasBaseline()) {
                    baselineVerticalWidget = widget;
                    break;
                }
            }
        }
        for (int i = 0; i < count; i++) {
            int index = isInRtl ? (count - 1 - i) : i;
            if (mStartIndex + index >= mFlow->mDisplayedWidgetsCount) break;
            ConstraintWidget* widget = mFlow->mDisplayedWidgets[mStartIndex + index];
            if (widget == nullptr) continue;
            if (i == 0) widget->connect(widget->mLeft, mLeft, mPaddingLeft);
            if (index == 0) {
                int style = mFlow->mHorizontalStyle;
                float bias = isInRtl ? (1 - mFlow->mHorizontalBias) : mFlow->mHorizontalBias;
                if (mStartIndex == 0 && mFlow->mFirstHorizontalStyle != UNKNOWN) {
                    style = mFlow->mFirstHorizontalStyle;
                    bias = isInRtl ? (1 - mFlow->mFirstHorizontalBias) : mFlow->mFirstHorizontalBias;
                } else if (isLastChain && mFlow->mLastHorizontalStyle != UNKNOWN) {
                    style = mFlow->mLastHorizontalStyle;
                    bias = isInRtl ? (1 - mFlow->mLastHorizontalBias) : mFlow->mLastHorizontalBias;
                }
                widget->mHorizontalChainStyle = style;
                widget->mHorizontalBiasPercent = bias;
            }
            if (i == count - 1) widget->connect(widget->mRight, mRight, mPaddingRight);
            if (previous != nullptr) {
                widget->mLeft.connect(previous->mRight, mFlow->mHorizontalGap);
                if (i == firstVisible) widget->mLeft.setGoneMargin(mPaddingLeft);
                previous->mRight.connect(widget->mLeft, 0);
                if (i == lastVisible + 1) previous->mRight.setGoneMargin(mPaddingRight);
            }
            if (widget != verticalWidget) {
                if (mFlow->mVerticalAlign == VERTICAL_ALIGN_BASELINE
                        && baselineVerticalWidget->hasBaseline()
                        && widget != baselineVerticalWidget && widget->hasBaseline()) {
                    widget->mBaseline.connect(baselineVerticalWidget->mBaseline, 0);
                } else {
                    switch (mFlow->mVerticalAlign) {
                    case VERTICAL_ALIGN_TOP:
                        widget->mTop.connect(verticalWidget->mTop, 0);
                        break;
                    case VERTICAL_ALIGN_BOTTOM:
                        widget->mBottom.connect(verticalWidget->mBottom, 0);
                        break;
                    case VERTICAL_ALIGN_CENTER:
                    default:
                        if (singleChain) {
                            widget->mTop.connect(mTop, mPaddingTop);
                            widget->mBottom.connect(mBottom, mPaddingBottom);
                        } else {
                            widget->mTop.connect(verticalWidget->mTop, 0);
                            widget->mBottom.connect(verticalWidget->mBottom, 0);
                        }
                        break;
                    }
                }
            }
            previous = widget;
        }
    } else {
        ConstraintWidget* horizontalWidget = mBiggest;
        horizontalWidget->mHorizontalChainStyle = mFlow->mHorizontalStyle;
        int padding = mPaddingLeft;
        if (chainIndex > 0) padding += mFlow->mHorizontalGap;
        if (isInRtl) {
            horizontalWidget->mRight.connect(mRight, padding);
            if (isLastChain) horizontalWidget->mLeft.connect(mLeft, mPaddingRight);
            if (chainIndex > 0) {
                ConstraintAnchor& l = mRight->getOwner()->mLeft;
                l.connect(horizontalWidget->mRight, 0);
            }
        } else {
            horizontalWidget->mLeft.connect(mLeft, padding);
            if (isLastChain) horizontalWidget->mRight.connect(mRight, mPaddingRight);
            if (chainIndex > 0) {
                ConstraintAnchor& r = mLeft->getOwner()->mRight;
                r.connect(horizontalWidget->mLeft, 0);
            }
        }
        for (int i = 0; i < count; i++) {
            if (mStartIndex + i >= mFlow->mDisplayedWidgetsCount) break;
            ConstraintWidget* widget = mFlow->mDisplayedWidgets[mStartIndex + i];
            if (widget == nullptr) continue;
            if (i == 0) {
                widget->connect(widget->mTop, mTop, mPaddingTop);
                int style = mFlow->mVerticalStyle;
                float bias = mFlow->mVerticalBias;
                if (mStartIndex == 0 && mFlow->mFirstVerticalStyle != UNKNOWN) {
                    style = mFlow->mFirstVerticalStyle;
                    bias = mFlow->mFirstVerticalBias;
                } else if (isLastChain && mFlow->mLastVerticalStyle != UNKNOWN) {
                    style = mFlow->mLastVerticalStyle;
                    bias = mFlow->mLastVerticalBias;
                }
                widget->mVerticalChainStyle = style;
                widget->mVerticalBiasPercent = bias;
            }
            if (i == count - 1) widget->connect(widget->mBottom, mBottom, mPaddingBottom);
            if (previous != nullptr) {
                widget->mTop.connect(previous->mBottom, mFlow->mVerticalGap);
                if (i == firstVisible) widget->mTop.setGoneMargin(mPaddingTop);
                previous->mBottom.connect(widget->mTop, 0);
                if (i == lastVisible + 1) previous->mBottom.setGoneMargin(mPaddingBottom);
            }
            if (widget != horizontalWidget) {
                int align = mFlow->mHorizontalAlign;
                if (isInRtl) {
                    switch (align) {
                    case HORIZONTAL_ALIGN_START:
                        widget->mRight.connect(horizontalWidget->mRight, 0);
                        break;
                    case HORIZONTAL_ALIGN_CENTER:
                        widget->mLeft.connect(horizontalWidget->mLeft, 0);
                        widget->mRight.connect(horizontalWidget->mRight, 0);
                        break;
                    case HORIZONTAL_ALIGN_END:
                        widget->mLeft.connect(horizontalWidget->mLeft, 0);
                        break;
                    }
                } else {
                    switch (align) {
                    case HORIZONTAL_ALIGN_START:
                        widget->mLeft.connect(horizontalWidget->mLeft, 0);
                        break;
                    case HORIZONTAL_ALIGN_CENTER:
                        if (singleChain) {
                            widget->mLeft.connect(mLeft, mPaddingLeft);
                            widget->mRight.connect(mRight, mPaddingRight);
                        } else {
                            widget->mLeft.connect(horizontalWidget->mLeft, 0);
                            widget->mRight.connect(horizontalWidget->mRight, 0);
                        }
                        break;
                    case HORIZONTAL_ALIGN_END:
                        widget->mRight.connect(horizontalWidget->mRight, 0);
                        break;
                    }
                }
            }
            previous = widget;
        }
    }
}

void Flow::WidgetsList::measureMatchConstraints(int /*availableSpace*/) {
    // TODO: distribute available space across this row's match_constraint widgets.
}

void Flow::WidgetsList::recomputeDimensions() {
    // TODO: recompute row width/height after match-constraint distribution (wrap-content Flow).
}

} // namespace cdroid::clcore
