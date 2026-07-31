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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1300  USA
 *********************************************************************************/

/*
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.analyzer.Direct.
 *
 * Translates the chain fast-path verbatim. Java field accesses on ConstraintWidget/ConstraintAnchor
 * map to CDROID's public members; `instanceof ConstraintWidgetContainer` -> dynamic_cast,
 * `instanceof Guideline` -> isGuideline(). mListAnchors[] is a pointer array in C++ (use ->).
 * DEBUG/FULL_DEBUG print blocks and the sHcount/sVcount counters are omitted (house style).
 * EARLY_TERMINATION (true) and APPLY_MATCH_PARENT (false) are preserved as file-local flags.
 */
#include <widgetEx/constraintlayout/core/widgets/analyzer/direct.h>

#include <algorithm>

#include <widgetEx/constraintlayout/core/linearsystem.h>
#include <widgetEx/constraintlayout/core/widgets/chainhead.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.h>

namespace cdroid {
namespace {
// Java Direct static finals (lines 45-47). DEBUG prints are dropped, so DEBUG itself is unused.
constexpr bool EARLY_TERMINATION  = true;
constexpr bool APPLY_MATCH_PARENT = false;
} // namespace

// Java Direct.canMeasure (815). Whether both dimensions are knowable by a single measure pass.
bool Direct::canMeasure(int /*level*/, ConstraintWidget* layout) {
    ConstraintWidget::DimensionBehaviour horizontalBehaviour = layout->getHorizontalDimensionBehaviour();
    ConstraintWidget::DimensionBehaviour verticalBehaviour = layout->getVerticalDimensionBehaviour();
    ConstraintWidgetContainer* parent = dynamic_cast<ConstraintWidgetContainer*>(layout->getParent());
    bool isParentHorizontalFixed = parent != nullptr
            && parent->getHorizontalDimensionBehaviour() == ConstraintWidget::DimensionBehaviour::FIXED;
    bool isParentVerticalFixed = parent != nullptr
            && parent->getVerticalDimensionBehaviour() == ConstraintWidget::DimensionBehaviour::FIXED;
    bool isHorizontalFixed = horizontalBehaviour == ConstraintWidget::DimensionBehaviour::FIXED
            || layout->isResolvedHorizontally()
            || (APPLY_MATCH_PARENT && horizontalBehaviour == ConstraintWidget::DimensionBehaviour::MATCH_PARENT
                    && isParentHorizontalFixed)
            || horizontalBehaviour == ConstraintWidget::DimensionBehaviour::WRAP_CONTENT
            || (horizontalBehaviour == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    && layout->mMatchConstraintDefaultWidth == ConstraintWidget::MATCH_CONSTRAINT_SPREAD
                    && layout->mDimensionRatio == 0
                    && layout->hasDanglingDimension(ConstraintWidget::HORIZONTAL))
            || (horizontalBehaviour == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    && layout->mMatchConstraintDefaultWidth == ConstraintWidget::MATCH_CONSTRAINT_WRAP
                    && layout->hasResolvedTargets(ConstraintWidget::HORIZONTAL, layout->getWidth()));
    bool isVerticalFixed = verticalBehaviour == ConstraintWidget::DimensionBehaviour::FIXED
            || layout->isResolvedVertically()
            || (APPLY_MATCH_PARENT && verticalBehaviour == ConstraintWidget::DimensionBehaviour::MATCH_PARENT
                    && isParentVerticalFixed)
            || verticalBehaviour == ConstraintWidget::DimensionBehaviour::WRAP_CONTENT
            || (verticalBehaviour == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    && layout->mMatchConstraintDefaultHeight == ConstraintWidget::MATCH_CONSTRAINT_SPREAD
                    && layout->mDimensionRatio == 0
                    && layout->hasDanglingDimension(ConstraintWidget::VERTICAL))
            || (verticalBehaviour == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    && layout->mMatchConstraintDefaultHeight == ConstraintWidget::MATCH_CONSTRAINT_WRAP
                    && layout->hasResolvedTargets(ConstraintWidget::VERTICAL, layout->getHeight()));
    if (layout->mDimensionRatio > 0 && (isHorizontalFixed || isVerticalFixed)) {
        return true;
    }
    return isHorizontalFixed && isVerticalFixed;
}

// Java Direct.horizontalSolvingPass (315). Walks the widget's horizontal dependencies, resolving
// those that can be placed directly. left/right use the direct member anchors (== getAnchor(LEFT/RIGHT)).
void Direct::horizontalSolvingPass(int level, ConstraintWidget* layout,
        BasicMeasure::Measurer* measurer, bool isRtl) {
    if (EARLY_TERMINATION && layout->isHorizontalSolvingPassDone()) {
        return;
    }

    if (dynamic_cast<ConstraintWidgetContainer*>(layout) == nullptr
            && layout->isMeasureRequested() && canMeasure(level + 1, layout)) {
        BasicMeasure::Measure measure;
        ConstraintWidgetContainer::measure(level + 1, layout, measurer, &measure,
                BasicMeasure::Measure::SELF_DIMENSIONS);
    }

    ConstraintAnchor* left = &layout->mLeft;
    ConstraintAnchor* right = &layout->mRight;
    int l = left->getFinalValue();
    int r = right->getFinalValue();

    if (left->getDependents() != nullptr && left->hasFinalValue()) {
        for (ConstraintAnchor* first : *left->getDependents()) {
            ConstraintWidget* widget = first->mOwner;
            int x1 = 0;
            int x2 = 0;
            bool canMeasureWidget = canMeasure(level + 1, widget);
            if (widget->isMeasureRequested() && canMeasureWidget) {
                BasicMeasure::Measure measure;
                ConstraintWidgetContainer::measure(level + 1, widget, measurer, &measure,
                        BasicMeasure::Measure::SELF_DIMENSIONS);
            }

            bool bothConnected = (first == &widget->mLeft && widget->mRight.mTarget != nullptr
                    && widget->mRight.mTarget->hasFinalValue())
                    || (first == &widget->mRight && widget->mLeft.mTarget != nullptr
                    && widget->mLeft.mTarget->hasFinalValue());
            if (widget->getHorizontalDimensionBehaviour() != ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    || canMeasureWidget) {
                if (widget->isMeasureRequested()) {
                    // Widget needs to be measured; bail.
                    continue;
                }
                if (first == &widget->mLeft && widget->mRight.mTarget == nullptr) {
                    x1 = l + widget->mLeft.getMargin();
                    x2 = x1 + widget->getWidth();
                    widget->setFinalHorizontal(x1, x2);
                    horizontalSolvingPass(level + 1, widget, measurer, isRtl);
                } else if (first == &widget->mRight && widget->mLeft.mTarget == nullptr) {
                    x2 = l - widget->mRight.getMargin();
                    x1 = x2 - widget->getWidth();
                    widget->setFinalHorizontal(x1, x2);
                    horizontalSolvingPass(level + 1, widget, measurer, isRtl);
                } else if (bothConnected && !widget->isInHorizontalChain()) {
                    solveHorizontalCenterConstraints(level + 1, measurer, widget, isRtl);
                } else if (APPLY_MATCH_PARENT
                        && widget->getHorizontalDimensionBehaviour() == ConstraintWidget::DimensionBehaviour::MATCH_PARENT) {
                    widget->setFinalHorizontal(0, widget->getWidth());
                    horizontalSolvingPass(level + 1, widget, measurer, isRtl);
                }
            } else if (widget->getHorizontalDimensionBehaviour() == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    && widget->mMatchConstraintMaxWidth >= 0
                    && widget->mMatchConstraintMinWidth >= 0
                    && (widget->getVisibility() == ConstraintWidget::GONE
                    || (widget->mMatchConstraintDefaultWidth == ConstraintWidget::MATCH_CONSTRAINT_SPREAD
                    && widget->getDimensionRatio() == 0))
                    && !widget->isInHorizontalChain() && !widget->isInVirtualLayout()) {
                if (bothConnected && !widget->isInHorizontalChain()) {
                    solveHorizontalMatchConstraint(level + 1, layout, measurer, widget, isRtl);
                }
            }
        }
    }
    if (layout->isGuideline()) {
        return;
    }
    if (right->getDependents() != nullptr && right->hasFinalValue()) {
        for (ConstraintAnchor* first : *right->getDependents()) {
            ConstraintWidget* widget = first->mOwner;
            bool canMeasureWidget = canMeasure(level + 1, widget);
            if (widget->isMeasureRequested() && canMeasureWidget) {
                BasicMeasure::Measure measure;
                ConstraintWidgetContainer::measure(level + 1, widget, measurer, &measure,
                        BasicMeasure::Measure::SELF_DIMENSIONS);
            }

            int x1 = 0;
            int x2 = 0;
            bool bothConnected = (first == &widget->mLeft && widget->mRight.mTarget != nullptr
                    && widget->mRight.mTarget->hasFinalValue())
                    || (first == &widget->mRight && widget->mLeft.mTarget != nullptr
                    && widget->mLeft.mTarget->hasFinalValue());
            if (widget->getHorizontalDimensionBehaviour() != ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    || canMeasureWidget) {
                if (widget->isMeasureRequested()) {
                    continue;
                }
                if (first == &widget->mLeft && widget->mRight.mTarget == nullptr) {
                    x1 = r + widget->mLeft.getMargin();
                    x2 = x1 + widget->getWidth();
                    widget->setFinalHorizontal(x1, x2);
                    horizontalSolvingPass(level + 1, widget, measurer, isRtl);
                } else if (first == &widget->mRight && widget->mLeft.mTarget == nullptr) {
                    x2 = r - widget->mRight.getMargin();
                    x1 = x2 - widget->getWidth();
                    widget->setFinalHorizontal(x1, x2);
                    horizontalSolvingPass(level + 1, widget, measurer, isRtl);
                } else if (bothConnected && !widget->isInHorizontalChain()) {
                    solveHorizontalCenterConstraints(level + 1, measurer, widget, isRtl);
                }
            } else if (widget->getHorizontalDimensionBehaviour() == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    && widget->mMatchConstraintMaxWidth >= 0
                    && widget->mMatchConstraintMinWidth >= 0
                    && (widget->getVisibility() == ConstraintWidget::GONE
                    || (widget->mMatchConstraintDefaultWidth == ConstraintWidget::MATCH_CONSTRAINT_SPREAD
                    && widget->getDimensionRatio() == 0))
                    && !widget->isInHorizontalChain() && !widget->isInVirtualLayout()) {
                if (bothConnected && !widget->isInHorizontalChain()) {
                    solveHorizontalMatchConstraint(level + 1, layout, measurer, widget, isRtl);
                }
            }
        }
    }
    layout->markHorizontalSolvingPassDone();
}

// Java Direct.verticalSolvingPass (468). Mirrors horizontalSolvingPass, plus a baseline-dependents
// pass (612-639).
void Direct::verticalSolvingPass(int level, ConstraintWidget* layout,
        BasicMeasure::Measurer* measurer) {
    if (EARLY_TERMINATION && layout->isVerticalSolvingPassDone()) {
        return;
    }

    if (dynamic_cast<ConstraintWidgetContainer*>(layout) == nullptr
            && layout->isMeasureRequested() && canMeasure(level + 1, layout)) {
        BasicMeasure::Measure measure;
        ConstraintWidgetContainer::measure(level + 1, layout, measurer, &measure,
                BasicMeasure::Measure::SELF_DIMENSIONS);
    }

    ConstraintAnchor* top = &layout->mTop;
    ConstraintAnchor* bottom = &layout->mBottom;
    int t = top->getFinalValue();
    int b = bottom->getFinalValue();

    if (top->getDependents() != nullptr && top->hasFinalValue()) {
        for (ConstraintAnchor* first : *top->getDependents()) {
            ConstraintWidget* widget = first->mOwner;
            int y1 = 0;
            int y2 = 0;
            bool canMeasureWidget = canMeasure(level + 1, widget);
            if (widget->isMeasureRequested() && canMeasureWidget) {
                BasicMeasure::Measure measure;
                ConstraintWidgetContainer::measure(level + 1, widget, measurer, &measure,
                        BasicMeasure::Measure::SELF_DIMENSIONS);
            }

            bool bothConnected = (first == &widget->mTop && widget->mBottom.mTarget != nullptr
                    && widget->mBottom.mTarget->hasFinalValue())
                    || (first == &widget->mBottom && widget->mTop.mTarget != nullptr
                    && widget->mTop.mTarget->hasFinalValue());
            if (widget->getVerticalDimensionBehaviour() != ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    || canMeasureWidget) {
                if (widget->isMeasureRequested()) {
                    continue;
                }
                if (first == &widget->mTop && widget->mBottom.mTarget == nullptr) {
                    y1 = t + widget->mTop.getMargin();
                    y2 = y1 + widget->getHeight();
                    widget->setFinalVertical(y1, y2);
                    verticalSolvingPass(level + 1, widget, measurer);
                } else if (first == &widget->mBottom && widget->mTop.mTarget == nullptr) {
                    y2 = t - widget->mBottom.getMargin();
                    y1 = y2 - widget->getHeight();
                    widget->setFinalVertical(y1, y2);
                    verticalSolvingPass(level + 1, widget, measurer);
                } else if (bothConnected && !widget->isInVerticalChain()) {
                    solveVerticalCenterConstraints(level + 1, measurer, widget);
                } else if (APPLY_MATCH_PARENT
                        && widget->getVerticalDimensionBehaviour() == ConstraintWidget::DimensionBehaviour::MATCH_PARENT) {
                    widget->setFinalVertical(0, widget->getHeight());
                    verticalSolvingPass(level + 1, widget, measurer);
                }
            } else if (widget->getVerticalDimensionBehaviour() == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    && widget->mMatchConstraintMaxHeight >= 0
                    && widget->mMatchConstraintMinHeight >= 0
                    && (widget->getVisibility() == ConstraintWidget::GONE
                    || (widget->mMatchConstraintDefaultHeight == ConstraintWidget::MATCH_CONSTRAINT_SPREAD
                    && widget->getDimensionRatio() == 0))
                    && !widget->isInVerticalChain() && !widget->isInVirtualLayout()) {
                if (bothConnected && !widget->isInVerticalChain()) {
                    solveVerticalMatchConstraint(level + 1, layout, measurer, widget);
                }
            }
        }
    }
    if (layout->isGuideline()) {
        return;
    }
    if (bottom->getDependents() != nullptr && bottom->hasFinalValue()) {
        for (ConstraintAnchor* first : *bottom->getDependents()) {
            ConstraintWidget* widget = first->mOwner;
            bool canMeasureWidget = canMeasure(level + 1, widget);
            if (widget->isMeasureRequested() && canMeasureWidget) {
                BasicMeasure::Measure measure;
                ConstraintWidgetContainer::measure(level + 1, widget, measurer, &measure,
                        BasicMeasure::Measure::SELF_DIMENSIONS);
            }

            int y1 = 0;
            int y2 = 0;
            bool bothConnected = (first == &widget->mTop && widget->mBottom.mTarget != nullptr
                    && widget->mBottom.mTarget->hasFinalValue())
                    || (first == &widget->mBottom && widget->mTop.mTarget != nullptr
                    && widget->mTop.mTarget->hasFinalValue());
            if (widget->getVerticalDimensionBehaviour() != ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    || canMeasureWidget) {
                if (widget->isMeasureRequested()) {
                    continue;
                }
                if (first == &widget->mTop && widget->mBottom.mTarget == nullptr) {
                    y1 = b + widget->mTop.getMargin();
                    y2 = y1 + widget->getHeight();
                    widget->setFinalVertical(y1, y2);
                    verticalSolvingPass(level + 1, widget, measurer);
                } else if (first == &widget->mBottom && widget->mTop.mTarget == nullptr) {
                    y2 = b - widget->mBottom.getMargin();
                    y1 = y2 - widget->getHeight();
                    widget->setFinalVertical(y1, y2);
                    verticalSolvingPass(level + 1, widget, measurer);
                } else if (bothConnected && !widget->isInVerticalChain()) {
                    solveVerticalCenterConstraints(level + 1, measurer, widget);
                }
            } else if (widget->getVerticalDimensionBehaviour() == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    && widget->mMatchConstraintMaxHeight >= 0
                    && widget->mMatchConstraintMinHeight >= 0
                    && (widget->getVisibility() == ConstraintWidget::GONE
                    || (widget->mMatchConstraintDefaultHeight == ConstraintWidget::MATCH_CONSTRAINT_SPREAD
                    && widget->getDimensionRatio() == 0))
                    && !widget->isInVerticalChain() && !widget->isInVirtualLayout()) {
                if (bothConnected && !widget->isInVerticalChain()) {
                    solveVerticalMatchConstraint(level + 1, layout, measurer, widget);
                }
            }
        }
    }

    ConstraintAnchor* baseline = &layout->mBaseline;
    if (baseline->getDependents() != nullptr && baseline->hasFinalValue()) {
        int baselineValue = baseline->getFinalValue();
        for (ConstraintAnchor* first : *baseline->getDependents()) {
            ConstraintWidget* widget = first->mOwner;
            bool canMeasureWidget = canMeasure(level + 1, widget);
            if (widget->isMeasureRequested() && canMeasureWidget) {
                BasicMeasure::Measure measure;
                ConstraintWidgetContainer::measure(level + 1, widget, measurer, &measure,
                        BasicMeasure::Measure::SELF_DIMENSIONS);
            }
            if (widget->getVerticalDimensionBehaviour() != ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    || canMeasureWidget) {
                if (widget->isMeasureRequested()) {
                    continue;
                }
                if (first == &widget->mBaseline) {
                    widget->setFinalBaseline(baselineValue + first->getMargin());
                    verticalSolvingPass(level + 1, widget, measurer);
                }
            }
        }
    }
    layout->markVerticalSolvingPassDone();
}

// Java Direct.solveHorizontalCenterConstraints (646).
void Direct::solveHorizontalCenterConstraints(int level, BasicMeasure::Measurer* measurer,
        ConstraintWidget* widget, bool isRtl) {
    int x1;
    int x2;
    float bias = widget->getHorizontalBiasPercent();
    int start = widget->mLeft.mTarget->getFinalValue();
    int end = widget->mRight.mTarget->getFinalValue();
    int s1 = start + widget->mLeft.getMargin();
    int s2 = end - widget->mRight.getMargin();
    if (start == end) {
        bias = 0.5f;
        s1 = start;
        s2 = end;
    }
    int width = widget->getWidth();
    int distance = s2 - s1 - width;
    if (s1 > s2) {
        distance = s1 - s2 - width;
    }
    int d1;
    if (distance > 0) {
        d1 = (int) (0.5f + bias * distance);
    } else {
        d1 = (int) (bias * distance);
    }
    x1 = s1 + d1;
    x2 = x1 + width;
    if (s1 > s2) {
        x1 = s1 + d1;
        x2 = x1 - width;
    }
    widget->setFinalHorizontal(x1, x2);
    horizontalSolvingPass(level + 1, widget, measurer, isRtl);
}

// Java Direct.solveVerticalCenterConstraints (687).
void Direct::solveVerticalCenterConstraints(int level, BasicMeasure::Measurer* measurer,
        ConstraintWidget* widget) {
    int y1;
    int y2;
    float bias = widget->getVerticalBiasPercent();
    int start = widget->mTop.mTarget->getFinalValue();
    int end = widget->mBottom.mTarget->getFinalValue();
    int s1 = start + widget->mTop.getMargin();
    int s2 = end - widget->mBottom.getMargin();
    if (start == end) {
        bias = 0.5f;
        s1 = start;
        s2 = end;
    }
    int height = widget->getHeight();
    int distance = s2 - s1 - height;
    if (s1 > s2) {
        distance = s1 - s2 - height;
    }
    int d1;
    if (distance > 0) {
        d1 = (int) (0.5f + bias * distance);
    } else {
        d1 = (int) (bias * distance);
    }
    y1 = s1 + d1;
    y2 = y1 + height;
    if (s1 > s2) {
        y1 = s1 - d1;
        y2 = y1 - height;
    }
    widget->setFinalVertical(y1, y2);
    verticalSolvingPass(level + 1, widget, measurer);
}

// Java Direct.solveHorizontalMatchConstraint (727).
void Direct::solveHorizontalMatchConstraint(int level, ConstraintWidget* layout,
        BasicMeasure::Measurer* measurer, ConstraintWidget* widget, bool isRtl) {
    int x1;
    int x2;
    float bias = widget->getHorizontalBiasPercent();
    int s1 = widget->mLeft.mTarget->getFinalValue() + widget->mLeft.getMargin();
    int s2 = widget->mRight.mTarget->getFinalValue() - widget->mRight.getMargin();
    if (s2 >= s1) {
        int width = widget->getWidth();
        if (widget->getVisibility() != ConstraintWidget::GONE) {
            if (widget->mMatchConstraintDefaultWidth == ConstraintWidget::MATCH_CONSTRAINT_PERCENT) {
                int parentWidth = 0;
                if (dynamic_cast<ConstraintWidgetContainer*>(layout) != nullptr) {
                    parentWidth = layout->getWidth();
                } else {
                    parentWidth = layout->getParent()->getWidth();
                }
                width = (int) (0.5f * widget->getHorizontalBiasPercent() * parentWidth);
            } else if (widget->mMatchConstraintDefaultWidth == ConstraintWidget::MATCH_CONSTRAINT_SPREAD) {
                width = s2 - s1;
            }
            width = std::max(widget->mMatchConstraintMinWidth, width);
            if (widget->mMatchConstraintMaxWidth > 0) {
                width = std::min(widget->mMatchConstraintMaxWidth, width);
            }
        }
        int distance = s2 - s1 - width;
        int d1 = (int) (0.5f + bias * distance);
        x1 = s1 + d1;
        x2 = x1 + width;
        widget->setFinalHorizontal(x1, x2);
        horizontalSolvingPass(level + 1, widget, measurer, isRtl);
    }
}

// Java Direct.solveVerticalMatchConstraint (770).
void Direct::solveVerticalMatchConstraint(int level, ConstraintWidget* layout,
        BasicMeasure::Measurer* measurer, ConstraintWidget* widget) {
    int y1;
    int y2;
    float bias = widget->getVerticalBiasPercent();
    int s1 = widget->mTop.mTarget->getFinalValue() + widget->mTop.getMargin();
    int s2 = widget->mBottom.mTarget->getFinalValue() - widget->mBottom.getMargin();
    if (s2 >= s1) {
        int height = widget->getHeight();
        if (widget->getVisibility() != ConstraintWidget::GONE) {
            if (widget->mMatchConstraintDefaultHeight == ConstraintWidget::MATCH_CONSTRAINT_PERCENT) {
                int parentHeight = 0;
                if (dynamic_cast<ConstraintWidgetContainer*>(layout) != nullptr) {
                    parentHeight = layout->getHeight();
                } else {
                    parentHeight = layout->getParent()->getHeight();
                }
                height = (int) (0.5f * bias * parentHeight);
            } else if (widget->mMatchConstraintDefaultHeight == ConstraintWidget::MATCH_CONSTRAINT_SPREAD) {
                height = s2 - s1;
            }
            height = std::max(widget->mMatchConstraintMinHeight, height);
            if (widget->mMatchConstraintMaxHeight > 0) {
                height = std::min(widget->mMatchConstraintMaxHeight, height);
            }
        }
        int distance = s2 - s1 - height;
        int d1 = (int) (0.5f + bias * distance);
        y1 = s1 + d1;
        y2 = y1 + height;
        widget->setFinalVertical(y1, y2);
        verticalSolvingPass(level + 1, widget, measurer);
    }
}

// Java Direct.solveChain (869). Try to directly resolve the whole chain; return true if fully
// resolved (caller returns), false to fall back to Cassowary.
bool Direct::solveChain(ConstraintWidgetContainer* container, LinearSystem* system,
        int orientation, int offset, ChainHead* chainHead,
        bool isChainSpread, bool isChainSpreadInside, bool isChainPacked) {
    if (isChainPacked) {
        return false;
    }
    if (orientation == ConstraintWidget::HORIZONTAL) {
        if (!container->isResolvedHorizontally()) {
            return false;
        }
    } else {
        if (!container->isResolvedVertically()) {
            return false;
        }
    }
    int level = 0; // nested level (debug only)
    bool isRtl = container->isRtl();

    ConstraintWidget* first = chainHead->getFirst();
    ConstraintWidget* last = chainHead->getLast();
    ConstraintWidget* firstVisibleWidget = chainHead->getFirstVisibleWidget();
    ConstraintWidget* lastVisibleWidget = chainHead->getLastVisibleWidget();
    ConstraintWidget* head = chainHead->getHead();

    ConstraintWidget* widget = first;
    ConstraintWidget* next = nullptr;
    bool done = false;

    ConstraintAnchor* begin = first->mListAnchors[offset];
    ConstraintAnchor* end = last->mListAnchors[offset + 1];
    if (begin->mTarget == nullptr || end->mTarget == nullptr) {
        return false;
    }
    if (!begin->mTarget->hasFinalValue() || !end->mTarget->hasFinalValue()) {
        return false;
    }

    if (firstVisibleWidget == nullptr || lastVisibleWidget == nullptr) {
        return false;
    }

    int startPoint = begin->mTarget->getFinalValue()
            + firstVisibleWidget->mListAnchors[offset]->getMargin();
    int endPoint = end->mTarget->getFinalValue()
            - lastVisibleWidget->mListAnchors[offset + 1]->getMargin();

    int distance = endPoint - startPoint;
    if (distance <= 0) {
        return false;
    }
    int totalSize = 0;
    BasicMeasure::Measure measure;

    int numWidgets = 0;
    int numVisibleWidgets = 0;

    while (!done) {
        bool canMeasureWidget = canMeasure(level + 1, widget);
        if (!canMeasureWidget) {
            return false;
        }
        if (widget->mListDimensionBehaviors[orientation]
                == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT) {
            return false;
        }

        if (widget->isMeasureRequested()) {
            ConstraintWidgetContainer::measure(level + 1, widget, container->getMeasurer(),
                    &measure, BasicMeasure::Measure::SELF_DIMENSIONS);
        }

        totalSize += widget->mListAnchors[offset]->getMargin();
        if (orientation == ConstraintWidget::HORIZONTAL) {
            totalSize += widget->getWidth();
        } else {
            totalSize += widget->getHeight();
        }
        totalSize += widget->mListAnchors[offset + 1]->getMargin();

        numWidgets++;
        if (widget->getVisibility() != ConstraintWidget::GONE) {
            numVisibleWidgets++;
        }

        // go to the next widget
        ConstraintAnchor* nextAnchor = widget->mListAnchors[offset + 1]->mTarget;
        if (nextAnchor != nullptr) {
            next = nextAnchor->mOwner;
            if (next->mListAnchors[offset]->mTarget == nullptr
                    || next->mListAnchors[offset]->mTarget->mOwner != widget) {
                next = nullptr;
            }
        } else {
            next = nullptr;
        }
        if (next != nullptr) {
            widget = next;
        } else {
            done = true;
        }
    }

    if (numVisibleWidgets == 0) {
        return false;
    }
    if (numVisibleWidgets != numWidgets) {
        return false;
    }
    if (distance < totalSize) {
        return false;
    }

    int gap = distance - totalSize;
    if (isChainSpread) {
        gap = gap / (numVisibleWidgets + 1);
    } else if (isChainSpreadInside) {
        if (numVisibleWidgets > 2) {
            gap = gap / numVisibleWidgets - 1;
        }
    }

    if (numVisibleWidgets == 1) {
        float bias;
        if (orientation == ConstraintWidget::HORIZONTAL) {
            bias = head->getHorizontalBiasPercent();
        } else {
            bias = head->getVerticalBiasPercent();
        }
        int p1 = (int) (0.5f + startPoint + gap * bias);
        if (orientation == ConstraintWidget::HORIZONTAL) {
            firstVisibleWidget->setFinalHorizontal(p1, p1 + firstVisibleWidget->getWidth());
        } else {
            firstVisibleWidget->setFinalVertical(p1, p1 + firstVisibleWidget->getHeight());
        }
        horizontalSolvingPass(level + 1, firstVisibleWidget, container->getMeasurer(), isRtl);
        return true;
    }

    if (isChainSpread) {
        done = false;
        int current = startPoint + gap;
        widget = first;
        while (!done) {
            if (widget->getVisibility() == ConstraintWidget::GONE) {
                if (orientation == ConstraintWidget::HORIZONTAL) {
                    widget->setFinalHorizontal(current, current);
                    horizontalSolvingPass(level + 1, widget, container->getMeasurer(), isRtl);
                } else {
                    widget->setFinalVertical(current, current);
                    verticalSolvingPass(level + 1, widget, container->getMeasurer());
                }
            } else {
                current += widget->mListAnchors[offset]->getMargin();
                if (orientation == ConstraintWidget::HORIZONTAL) {
                    widget->setFinalHorizontal(current, current + widget->getWidth());
                    horizontalSolvingPass(level + 1, widget, container->getMeasurer(), isRtl);
                    current += widget->getWidth();
                } else {
                    widget->setFinalVertical(current, current + widget->getHeight());
                    verticalSolvingPass(level + 1, widget, container->getMeasurer());
                    current += widget->getHeight();
                }
                current += widget->mListAnchors[offset + 1]->getMargin();
                current += gap;
            }

            widget->addToSolver(system, false);

            // go to the next widget
            ConstraintAnchor* nextAnchor = widget->mListAnchors[offset + 1]->mTarget;
            if (nextAnchor != nullptr) {
                next = nextAnchor->mOwner;
                if (next->mListAnchors[offset]->mTarget == nullptr
                        || next->mListAnchors[offset]->mTarget->mOwner != widget) {
                    next = nullptr;
                }
            } else {
                next = nullptr;
            }
            if (next != nullptr) {
                widget = next;
            } else {
                done = true;
            }
        }
    } else if (isChainSpreadInside) {
        if (numVisibleWidgets == 2) {
            if (orientation == ConstraintWidget::HORIZONTAL) {
                firstVisibleWidget->setFinalHorizontal(startPoint,
                        startPoint + firstVisibleWidget->getWidth());
                lastVisibleWidget->setFinalHorizontal(endPoint - lastVisibleWidget->getWidth(),
                        endPoint);
                horizontalSolvingPass(level + 1, firstVisibleWidget, container->getMeasurer(), isRtl);
                horizontalSolvingPass(level + 1, lastVisibleWidget, container->getMeasurer(), isRtl);
            } else {
                firstVisibleWidget->setFinalVertical(startPoint,
                        startPoint + firstVisibleWidget->getHeight());
                lastVisibleWidget->setFinalVertical(endPoint - lastVisibleWidget->getHeight(),
                        endPoint);
                verticalSolvingPass(level + 1, firstVisibleWidget, container->getMeasurer());
                verticalSolvingPass(level + 1, lastVisibleWidget, container->getMeasurer());
            }
            return true;
        }
        return false;
    }
    return true;
}

} // namespace cdroid
