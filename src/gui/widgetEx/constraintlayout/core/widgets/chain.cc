/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.Chain.
 */
#include <widgetEx/constraintlayout/core/widgets/chain.h>

#include <algorithm>

#include <widgetEx/constraintlayout/core/arrayrow.h>
#include <widgetEx/constraintlayout/core/linearsystem.h>
#include <widgetEx/constraintlayout/core/solvervariable.h>
#include <widgetEx/constraintlayout/core/widgets/chainhead.h>
#include <widgetEx/constraintlayout/core/widgets/constraintanchor.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.h>

namespace cdroid {

void Chain::applyChainConstraints(ConstraintWidgetContainer* constraintWidgetContainer,
                                  LinearSystem* system,
                                  std::vector<ConstraintWidget*>* widgets,
                                  int orientation) {
    int offset = 0;
    int chainsSize = 0;
    std::vector<ChainHead*>* chainsArray = nullptr;
    if (orientation == ConstraintWidget::HORIZONTAL) {
        offset = 0;
        chainsSize = constraintWidgetContainer->mHorizontalChainsSize;
        chainsArray = &constraintWidgetContainer->mHorizontalChainsArray;
    } else {
        offset = 2;
        chainsSize = constraintWidgetContainer->mVerticalChainsSize;
        chainsArray = &constraintWidgetContainer->mVerticalChainsArray;
    }

    for (int i = 0; i < chainsSize; i++) {
        ChainHead* first = (*chainsArray)[i];
        // define() here: values may not be correctly initialized otherwise (they are set in
        // ConstraintWidget.addToSolver(), which is still stubbed).
        first->define();
        if (widgets == nullptr
                || std::find(widgets->begin(), widgets->end(), first->mFirst) != widgets->end()) {
            applyChainConstraints(constraintWidgetContainer, system, orientation, offset, first);
        }
    }
}

void Chain::applyChainConstraints(ConstraintWidgetContainer* container, LinearSystem* system,
                                  int orientation, int offset, ChainHead* chainHead) {
    ConstraintWidget* first = chainHead->mFirst;
    ConstraintWidget* last = chainHead->mLast;
    ConstraintWidget* firstVisibleWidget = chainHead->mFirstVisibleWidget;
    ConstraintWidget* lastVisibleWidget = chainHead->mLastVisibleWidget;
    ConstraintWidget* head = chainHead->mHead;

    ConstraintWidget* widget = first;
    ConstraintWidget* next = nullptr;
    bool done = false;

    float totalWeights = chainHead->mTotalWeight;
    (void)chainHead->mFirstMatchConstraintWidget;     // firstMatchConstraintsWidget (unused)
    (void)chainHead->mLastMatchConstraintWidget;      // previousMatchConstraintsWidget (unused)

    bool isWrapContent = container->mListDimensionBehaviors[orientation]
            == ConstraintWidget::DimensionBehaviour::WRAP_CONTENT;
    bool isChainSpread = false;
    bool isChainSpreadInside = false;
    bool isChainPacked = false;

    if (orientation == ConstraintWidget::HORIZONTAL) {
        isChainSpread = head->mHorizontalChainStyle == ConstraintWidget::CHAIN_SPREAD;
        isChainSpreadInside =
                head->mHorizontalChainStyle == ConstraintWidget::CHAIN_SPREAD_INSIDE;
        isChainPacked = head->mHorizontalChainStyle == ConstraintWidget::CHAIN_PACKED;
    } else {
        isChainSpread = head->mVerticalChainStyle == ConstraintWidget::CHAIN_SPREAD;
        isChainSpreadInside = head->mVerticalChainStyle == ConstraintWidget::CHAIN_SPREAD_INSIDE;
        isChainPacked = head->mVerticalChainStyle == ConstraintWidget::CHAIN_PACKED;
    }

    // USE_CHAIN_OPTIMIZATION (Direct.solveChain) branch omitted — flag is false.

    // This traversal sets up basic ordering constraints and builds the match-constraint list.
    while (!done) {
        ConstraintAnchor* begin = widget->mListAnchors[offset];

        int strength = SolverVariable::STRENGTH_HIGHEST;
        if (isChainPacked) {
            strength = SolverVariable::STRENGTH_LOW;
        }
        int margin = begin->getMargin();
        bool isSpreadOnly = widget->mListDimensionBehaviors[orientation]
                == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                && widget->mResolvedMatchConstraintDefault[orientation]
                        == ConstraintWidget::MATCH_CONSTRAINT_SPREAD;

        if (begin->mTarget != nullptr && widget != first) {
            margin += begin->mTarget->getMargin();
        }

        if (isChainPacked && widget != first && widget != firstVisibleWidget) {
            strength = SolverVariable::STRENGTH_FIXED;
        }

        if (begin->mTarget != nullptr) {
            if (widget == firstVisibleWidget) {
                system->addGreaterThan(begin->mSolverVariable, begin->mTarget->mSolverVariable,
                        margin, SolverVariable::STRENGTH_BARRIER);
            } else {
                system->addGreaterThan(begin->mSolverVariable, begin->mTarget->mSolverVariable,
                        margin, SolverVariable::STRENGTH_FIXED);
            }
            if (isSpreadOnly && !isChainPacked) {
                strength = SolverVariable::STRENGTH_EQUALITY;
            }
            if (widget == firstVisibleWidget && isChainPacked
                    && widget->isInBarrier(orientation)) {
                strength = SolverVariable::STRENGTH_EQUALITY;
            }
            system->addEquality(begin->mSolverVariable, begin->mTarget->mSolverVariable, margin,
                    strength);
        }

        if (isWrapContent) {
            if (widget->getVisibility() != ConstraintWidget::GONE
                    && widget->mListDimensionBehaviors[orientation]
                    == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT) {
                system->addGreaterThan(widget->mListAnchors[offset + 1]->mSolverVariable,
                        widget->mListAnchors[offset]->mSolverVariable, 0,
                        SolverVariable::STRENGTH_EQUALITY);
            }
            system->addGreaterThan(widget->mListAnchors[offset]->mSolverVariable,
                    container->mListAnchors[offset]->mSolverVariable,
                    0, SolverVariable::STRENGTH_FIXED);
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

    // Make sure we have constraints for the last anchors / targets
    if (lastVisibleWidget != nullptr && last->mListAnchors[offset + 1]->mTarget != nullptr) {
        ConstraintAnchor* end = lastVisibleWidget->mListAnchors[offset + 1];
        bool isSpreadOnly = lastVisibleWidget->mListDimensionBehaviors[orientation]
                == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                && lastVisibleWidget->mResolvedMatchConstraintDefault[orientation]
                        == ConstraintWidget::MATCH_CONSTRAINT_SPREAD;
        if (isSpreadOnly && !isChainPacked && end->mTarget->mOwner == container) {
            system->addEquality(end->mSolverVariable, end->mTarget->mSolverVariable,
                    -end->getMargin(), SolverVariable::STRENGTH_EQUALITY);
        } else if (isChainPacked && end->mTarget->mOwner == container) {
            system->addEquality(end->mSolverVariable, end->mTarget->mSolverVariable,
                    -end->getMargin(), SolverVariable::STRENGTH_HIGHEST);
        }
        system->addLowerThan(end->mSolverVariable,
                last->mListAnchors[offset + 1]->mTarget->mSolverVariable, -end->getMargin(),
                SolverVariable::STRENGTH_BARRIER);
    }

    // ... and make sure the root end is constrained in wrap content.
    if (isWrapContent) {
        system->addGreaterThan(container->mListAnchors[offset + 1]->mSolverVariable,
                last->mListAnchors[offset + 1]->mSolverVariable,
                last->mListAnchors[offset + 1]->getMargin(), SolverVariable::STRENGTH_FIXED);
    }

    // Now apply the centering / spreading for matched-constraint widgets
    std::vector<ConstraintWidget*>& listMatchConstraints = chainHead->mWeightedMatchConstraintsWidgets;
    const int count = (int) listMatchConstraints.size();
    if (count > 1) {
        ConstraintWidget* lastMatch = nullptr;
        float lastWeight = 0;

        if (chainHead->mHasUndefinedWeights && !chainHead->mHasComplexMatchWeights) {
            totalWeights = chainHead->mWidgetsMatchCount;
        }

        for (int i = 0; i < count; i++) {
            ConstraintWidget* match = listMatchConstraints[i];
            float currentWeight = match->mWeight[orientation];

            if (currentWeight < 0) {
                if (chainHead->mHasComplexMatchWeights) {
                    system->addEquality(match->mListAnchors[offset + 1]->mSolverVariable,
                            match->mListAnchors[offset]->mSolverVariable,
                            0, SolverVariable::STRENGTH_HIGHEST);
                    continue;
                }
                currentWeight = 1;
            }
            if (currentWeight == 0) {
                system->addEquality(match->mListAnchors[offset + 1]->mSolverVariable,
                        match->mListAnchors[offset]->mSolverVariable,
                        0, SolverVariable::STRENGTH_FIXED);
                continue;
            }

            if (lastMatch != nullptr) {
                SolverVariable* begin = lastMatch->mListAnchors[offset]->mSolverVariable;
                SolverVariable* end = lastMatch->mListAnchors[offset + 1]->mSolverVariable;
                SolverVariable* nextBegin = match->mListAnchors[offset]->mSolverVariable;
                SolverVariable* nextEnd = match->mListAnchors[offset + 1]->mSolverVariable;
                ArrayRow* row = system->createRow();
                row->createRowEqualMatchDimensions(lastWeight, totalWeights, currentWeight,
                        begin, end, nextBegin, nextEnd);
                system->addConstraint(row);
            }

            lastMatch = match;
            lastWeight = currentWeight;
        }
    }

    // DEBUG variable-name print block omitted (DEBUG = false).

    // Finally, apply the specific rules for the different chain types.
    if (firstVisibleWidget != nullptr
            && (firstVisibleWidget == lastVisibleWidget || isChainPacked)) {
        ConstraintAnchor* begin = first->mListAnchors[offset];
        ConstraintAnchor* end = last->mListAnchors[offset + 1];
        SolverVariable* beginTarget = begin->mTarget != nullptr
                ? begin->mTarget->mSolverVariable : nullptr;
        SolverVariable* endTarget = end->mTarget != nullptr ? end->mTarget->mSolverVariable : nullptr;
        begin = firstVisibleWidget->mListAnchors[offset];
        if (lastVisibleWidget != nullptr) {
            end = lastVisibleWidget->mListAnchors[offset + 1];
        }
        if (beginTarget != nullptr && endTarget != nullptr) {
            float bias = 0.5f;
            if (orientation == ConstraintWidget::HORIZONTAL) {
                bias = head->mHorizontalBiasPercent;
            } else {
                bias = head->mVerticalBiasPercent;
            }
            int beginMargin = begin->getMargin();
            int endMargin = end->getMargin();
            system->addCentering(begin->mSolverVariable, beginTarget,
                    beginMargin, bias, endTarget, end->mSolverVariable,
                    endMargin, SolverVariable::STRENGTH_CENTERING);
        }
    } else if (isChainSpread && firstVisibleWidget != nullptr) {
        // for chain spread, add equal dimensions in between *visible* widgets
        widget = firstVisibleWidget;
        ConstraintWidget* previousVisibleWidget = firstVisibleWidget;
        bool applyFixedEquality = chainHead->mWidgetsMatchCount > 0
                && (chainHead->mWidgetsCount == chainHead->mWidgetsMatchCount);
        while (widget != nullptr) {
            next = widget->mNextChainWidget[orientation];
            while (next != nullptr && next->getVisibility() == ConstraintWidget::GONE) {
                next = next->mNextChainWidget[orientation];
            }
            if (next != nullptr || widget == lastVisibleWidget) {
                ConstraintAnchor* beginAnchor = widget->mListAnchors[offset];
                SolverVariable* begin = beginAnchor->mSolverVariable;
                SolverVariable* beginTarget = beginAnchor->mTarget != nullptr
                        ? beginAnchor->mTarget->mSolverVariable : nullptr;
                if (previousVisibleWidget != widget) {
                    beginTarget =
                            previousVisibleWidget->mListAnchors[offset + 1]->mSolverVariable;
                } else if (widget == firstVisibleWidget) {
                    beginTarget = first->mListAnchors[offset]->mTarget != nullptr
                            ? first->mListAnchors[offset]->mTarget->mSolverVariable : nullptr;
                }

                ConstraintAnchor* beginNextAnchor = nullptr;
                SolverVariable* beginNext = nullptr;
                int beginMargin = beginAnchor->getMargin();
                int nextMargin = widget->mListAnchors[offset + 1]->getMargin();

                if (next != nullptr) {
                    beginNextAnchor = next->mListAnchors[offset];
                    beginNext = beginNextAnchor->mSolverVariable;
                } else {
                    beginNextAnchor = last->mListAnchors[offset + 1]->mTarget;
                    if (beginNextAnchor != nullptr) {
                        beginNext = beginNextAnchor->mSolverVariable;
                    }
                }
                SolverVariable* beginNextTarget = widget->mListAnchors[offset + 1]->mSolverVariable;

                if (beginNextAnchor != nullptr) {
                    nextMargin += beginNextAnchor->getMargin();
                }
                beginMargin += previousVisibleWidget->mListAnchors[offset + 1]->getMargin();
                if (begin != nullptr && beginTarget != nullptr
                        && beginNext != nullptr && beginNextTarget != nullptr) {
                    int margin1 = beginMargin;
                    if (widget == firstVisibleWidget) {
                        margin1 = firstVisibleWidget->mListAnchors[offset]->getMargin();
                    }
                    int margin2 = nextMargin;
                    if (widget == lastVisibleWidget) {
                        margin2 = lastVisibleWidget->mListAnchors[offset + 1]->getMargin();
                    }
                    int strength = SolverVariable::STRENGTH_EQUALITY;
                    if (applyFixedEquality) {
                        strength = SolverVariable::STRENGTH_FIXED;
                    }
                    system->addCentering(begin, beginTarget, margin1, 0.5f,
                            beginNext, beginNextTarget, margin2,
                            strength);
                }
            }
            if (widget->getVisibility() != ConstraintWidget::GONE) {
                previousVisibleWidget = widget;
            }
            widget = next;
        }
    } else if (isChainSpreadInside && firstVisibleWidget != nullptr) {
        // for chain spread inside, add equal dimensions in between *visible* widgets
        widget = firstVisibleWidget;
        ConstraintWidget* previousVisibleWidget = firstVisibleWidget;
        bool applyFixedEquality = chainHead->mWidgetsMatchCount > 0
                && (chainHead->mWidgetsCount == chainHead->mWidgetsMatchCount);
        while (widget != nullptr) {
            next = widget->mNextChainWidget[orientation];
            while (next != nullptr && next->getVisibility() == ConstraintWidget::GONE) {
                next = next->mNextChainWidget[orientation];
            }
            if (widget != firstVisibleWidget && widget != lastVisibleWidget && next != nullptr) {
                if (next == lastVisibleWidget) {
                    next = nullptr;
                }
                ConstraintAnchor* beginAnchor = widget->mListAnchors[offset];
                SolverVariable* begin = beginAnchor->mSolverVariable;
                SolverVariable* beginTarget = beginAnchor->mTarget != nullptr
                        ? beginAnchor->mTarget->mSolverVariable : nullptr;
                beginTarget = previousVisibleWidget->mListAnchors[offset + 1]->mSolverVariable;
                ConstraintAnchor* beginNextAnchor = nullptr;
                SolverVariable* beginNext = nullptr;
                SolverVariable* beginNextTarget = nullptr;
                int beginMargin = beginAnchor->getMargin();
                int nextMargin = widget->mListAnchors[offset + 1]->getMargin();

                if (next != nullptr) {
                    beginNextAnchor = next->mListAnchors[offset];
                    beginNext = beginNextAnchor->mSolverVariable;
                    beginNextTarget = beginNextAnchor->mTarget != nullptr
                            ? beginNextAnchor->mTarget->mSolverVariable : nullptr;
                } else {
                    beginNextAnchor = lastVisibleWidget->mListAnchors[offset];
                    if (beginNextAnchor != nullptr) {
                        beginNext = beginNextAnchor->mSolverVariable;
                    }
                    beginNextTarget = widget->mListAnchors[offset + 1]->mSolverVariable;
                }

                if (beginNextAnchor != nullptr) {
                    nextMargin += beginNextAnchor->getMargin();
                }
                beginMargin += previousVisibleWidget->mListAnchors[offset + 1]->getMargin();
                int strength = SolverVariable::STRENGTH_HIGHEST;
                if (applyFixedEquality) {
                    strength = SolverVariable::STRENGTH_FIXED;
                }
                if (begin != nullptr && beginTarget != nullptr
                        && beginNext != nullptr && beginNextTarget != nullptr) {
                    system->addCentering(begin, beginTarget, beginMargin, 0.5f,
                            beginNext, beginNextTarget, nextMargin,
                            strength);
                }
            }
            if (widget->getVisibility() != ConstraintWidget::GONE) {
                previousVisibleWidget = widget;
            }
            widget = next;
        }
        ConstraintAnchor* begin = firstVisibleWidget->mListAnchors[offset];
        ConstraintAnchor* beginTarget = first->mListAnchors[offset]->mTarget;
        ConstraintAnchor* end = lastVisibleWidget->mListAnchors[offset + 1];
        ConstraintAnchor* endTarget = lastVisibleWidget->mListAnchors[offset + 1]->mTarget;
        int endPointsStrength = SolverVariable::STRENGTH_EQUALITY;
        if (beginTarget != nullptr) {
            if (firstVisibleWidget != lastVisibleWidget) {
                system->addEquality(begin->mSolverVariable, beginTarget->mSolverVariable,
                        begin->getMargin(), endPointsStrength);
            } else if (endTarget != nullptr) {
                system->addCentering(begin->mSolverVariable, beginTarget->mSolverVariable,
                        begin->getMargin(), 0.5f, end->mSolverVariable, endTarget->mSolverVariable,
                        end->getMargin(), endPointsStrength);
            }
        }
        if (endTarget != nullptr && (firstVisibleWidget != lastVisibleWidget)) {
            system->addEquality(end->mSolverVariable,
                    endTarget->mSolverVariable, -end->getMargin(), endPointsStrength);
        }
    }

    // final centering, necessary if the chain is larger than the available space...
    if ((isChainSpread || isChainSpreadInside) && firstVisibleWidget != nullptr
            && firstVisibleWidget != lastVisibleWidget) {
        ConstraintAnchor* begin = firstVisibleWidget->mListAnchors[offset];
        if (lastVisibleWidget == nullptr) {
            lastVisibleWidget = firstVisibleWidget;
        }
        ConstraintAnchor* end = lastVisibleWidget->mListAnchors[offset + 1];
        SolverVariable* beginTarget =
                begin->mTarget != nullptr ? begin->mTarget->mSolverVariable : nullptr;
        SolverVariable* endTarget = end->mTarget != nullptr ? end->mTarget->mSolverVariable : nullptr;
        if (last != lastVisibleWidget) {
            ConstraintAnchor* realEnd = last->mListAnchors[offset + 1];
            endTarget = realEnd->mTarget != nullptr ? realEnd->mTarget->mSolverVariable : nullptr;
        }
        if (firstVisibleWidget == lastVisibleWidget) {
            begin = firstVisibleWidget->mListAnchors[offset];
            end = firstVisibleWidget->mListAnchors[offset + 1];
        }
        if (beginTarget != nullptr && endTarget != nullptr) {
            float bias = 0.5f;
            int beginMargin = begin->getMargin();
            int endMargin = lastVisibleWidget->mListAnchors[offset + 1]->getMargin();
            system->addCentering(begin->mSolverVariable, beginTarget, beginMargin,
                    bias, endTarget, end->mSolverVariable, endMargin,
                    SolverVariable::STRENGTH_EQUALITY);
        }
    }
}

} // namespace cdroid
