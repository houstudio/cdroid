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

/*
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.analyzer.BasicMeasure.
 */
#include <widgetEx/constraintlayout/core/widgets/analyzer/basicmeasure.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.h>
#include <widgetEx/constraintlayout/core/widgets/guideline.h>
#include <widgetEx/constraintlayout/core/widgets/virtuallayout.h>

namespace cdroid {

int BasicMeasure::Measure::SELF_DIMENSIONS       = 0;
int BasicMeasure::Measure::TRY_GIVEN_DIMENSIONS  = 1;
int BasicMeasure::Measure::USE_GIVEN_DIMENSIONS  = 2;

BasicMeasure::BasicMeasure(ConstraintWidgetContainer* constraintWidgetContainer)
    : mConstraintWidgetContainer(constraintWidgetContainer) {
}

void BasicMeasure::updateHierarchy(ConstraintWidgetContainer* layout) {
    mVariableDimensionsWidgets.clear();
    const int childCount = (int) layout->mChildren.size();
    for (int i = 0; i < childCount; i++) {
        ConstraintWidget* widget = layout->mChildren[i];
        if (widget->getHorizontalDimensionBehaviour()
                == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                || widget->getVerticalDimensionBehaviour()
                == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT) {
            mVariableDimensionsWidgets.push_back(widget);
        }
    }
    layout->invalidateGraph(); // MVP: no-op (DependencyGraph deferred)
}

void BasicMeasure::measureChildren(ConstraintWidgetContainer* layout) {
    const int childCount = (int) layout->mChildren.size();
    bool optimize = layout->optimizeFor(Optimizer::OPTIMIZATION_GRAPH);
    Measurer* measurer = layout->getMeasurer();
    for (int i = 0; i < childCount; i++) {
        ConstraintWidget* child = layout->mChildren[i];
        if (dynamic_cast<clcore::Guideline*>(child) != nullptr) {
            continue;
        }
        if (child->isBarrier()) {
            continue;
        }
        if (child->isInVirtualLayout()) {
            continue;
        }

        if (optimize && child->mHorizontalRun != nullptr && child->mVerticalRun != nullptr) {
            // DEFERRED(graph): Java also checks mHorizontalRun->mDimension.resolved &&
            // mVerticalRun->mDimension.resolved (needs HorizontalWidgetRun complete). OPTIMIZATION_GRAPH
            // is off by default so this branch is dead; the resolved check is restored with the run system.
            continue;
        }

        ConstraintWidget::DimensionBehaviour widthBehavior =
                child->getDimensionBehaviour(ConstraintWidget::HORIZONTAL);
        ConstraintWidget::DimensionBehaviour heightBehavior =
                child->getDimensionBehaviour(ConstraintWidget::VERTICAL);

        bool skip = widthBehavior == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                && child->mMatchConstraintDefaultWidth != ConstraintWidget::MATCH_CONSTRAINT_WRAP
                && heightBehavior == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                && child->mMatchConstraintDefaultHeight != ConstraintWidget::MATCH_CONSTRAINT_WRAP;

        if (!skip && layout->optimizeFor(Optimizer::OPTIMIZATION_DIRECT)
                && !child->isVirtualLayout()) {
            if (widthBehavior == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    && child->mMatchConstraintDefaultWidth == ConstraintWidget::MATCH_CONSTRAINT_SPREAD
                    && heightBehavior != ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    && !child->isInHorizontalChain()) {
                skip = true;
            }
            if (heightBehavior == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    && child->mMatchConstraintDefaultHeight == ConstraintWidget::MATCH_CONSTRAINT_SPREAD
                    && widthBehavior != ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    && !child->isInHorizontalChain()) {
                skip = true;
            }
            if ((widthBehavior == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    || heightBehavior == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT)
                    && child->mDimensionRatio > 0) {
                skip = true;
            }
        }

        if (skip) {
            continue;
        }

        measure(measurer, child, Measure::SELF_DIMENSIONS);
    }
    if (measurer != nullptr) measurer->didMeasures();
}

bool BasicMeasure::measure(Measurer* measurer, ConstraintWidget* widget, int measureStrategy) {
    mMeasure.horizontalBehavior = widget->getHorizontalDimensionBehaviour();
    mMeasure.verticalBehavior   = widget->getVerticalDimensionBehaviour();
    mMeasure.horizontalDimension = widget->getWidth();
    mMeasure.verticalDimension   = widget->getHeight();
    mMeasure.measuredNeedsSolverPass = false;
    mMeasure.measureStrategy = measureStrategy;

    bool horizontalMatchConstraints = (mMeasure.horizontalBehavior
            == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT);
    bool verticalMatchConstraints = (mMeasure.verticalBehavior
            == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT);
    bool horizontalUseRatio = horizontalMatchConstraints && widget->mDimensionRatio > 0;
    bool verticalUseRatio = verticalMatchConstraints && widget->mDimensionRatio > 0;

    if (horizontalUseRatio) {
        if (widget->mResolvedMatchConstraintDefault[ConstraintWidget::HORIZONTAL]
                == ConstraintWidget::MATCH_CONSTRAINT_RATIO_RESOLVED) {
            mMeasure.horizontalBehavior = ConstraintWidget::DimensionBehaviour::FIXED;
        }
    }
    if (verticalUseRatio) {
        if (widget->mResolvedMatchConstraintDefault[ConstraintWidget::VERTICAL]
                == ConstraintWidget::MATCH_CONSTRAINT_RATIO_RESOLVED) {
            mMeasure.verticalBehavior = ConstraintWidget::DimensionBehaviour::FIXED;
        }
    }

    if (measurer != nullptr) measurer->measure(widget, &mMeasure);
    widget->setWidth(mMeasure.measuredWidth);
    widget->setHeight(mMeasure.measuredHeight);
    widget->setHasBaseline(mMeasure.measuredHasBaseline);
    widget->setBaselineDistance(mMeasure.measuredBaseline);
    mMeasure.measureStrategy = Measure::SELF_DIMENSIONS;
    return mMeasure.measuredNeedsSolverPass;
}

void BasicMeasure::solveLinearSystem(ConstraintWidgetContainer* layout, const char* /*reason*/,
                                     int pass, int w, int h) {
    int minWidth = layout->getMinWidth();
    int minHeight = layout->getMinHeight();
    layout->setMinWidth(0);
    layout->setMinHeight(0);
    layout->setWidth(w);
    layout->setHeight(h);
    layout->setMinWidth(minWidth);
    layout->setMinHeight(minHeight);
    layout->setPass(pass);
    layout->layout();
}

long BasicMeasure::solverMeasure(ConstraintWidgetContainer* layout, int /*optimizationLevel*/,
                                 int /*paddingX*/, int /*paddingY*/,
                                 int /*widthMode*/, int /*widthSize*/,
                                 int /*heightMode*/, int /*heightSize*/,
                                 int /*lastMeasureWidth*/, int /*lastMeasureHeight*/) {
    // measureChildren -> updateHierarchy -> solveLinearSystem (first pass), then the
    // size-dependent block (Android BasicMeasure.java 305-445): VirtualLayouts first
    // (TRY_GIVEN_DIMENSIONS through the shared Measurer — the B4 mode vocabulary: the core
    // Flow consumes the same mode encoding BasicMeasure declares), then the match-constraint
    // convergence loop (maxIterations=2, TRY→USE_GIVEN_DIMENSIONS, early-exit on convergence).
    // Only the OPTIMIZATION_GRAPH optimize-path (directMeasure / DependencyGraph run-system)
    // remains deferred — it is a performance path (off by default in AndroidX) and does not
    // affect the standard linear-solve correctness exercised here.
    Measurer* measurer = layout->getMeasurer();
    const int childCount = (int) layout->mChildren.size();
    int startingWidth = layout->getWidth();
    int startingHeight = layout->getHeight();

    bool allSolved = false;
    (void) allSolved;

    if (childCount > 0) {
        measureChildren(layout);
    }
    updateHierarchy(layout);

    if (childCount > 0) {
        solveLinearSystem(layout, "First pass", 0, startingWidth, startingHeight);
    }

    const int sizeDependentWidgetsCount = (int) mVariableDimensionsWidgets.size();
    if (sizeDependentWidgetsCount > 0) {
        bool needSolverPass = false;
        const bool containerWrapWidth =
                layout->getHorizontalDimensionBehaviour()
                        == ConstraintWidget::DimensionBehaviour::WRAP_CONTENT;
        const bool containerWrapHeight =
                layout->getVerticalDimensionBehaviour()
                        == ConstraintWidget::DimensionBehaviour::WRAP_CONTENT;
        int minWidth = std::max(layout->getWidth(),
                                mConstraintWidgetContainer->getMinWidth());
        int minHeight = std::max(layout->getHeight(),
                                 mConstraintWidgetContainer->getMinHeight());

        ////////////////////////////////////////////////////////////////////////////////////
        // Let's first apply sizes for VirtualLayouts if any (Android BasicMeasure.java 316-352)
        ////////////////////////////////////////////////////////////////////////////////////
        for (int i = 0; i < sizeDependentWidgetsCount; i++) {
            ConstraintWidget* widget = mVariableDimensionsWidgets[i];
            auto* virtualLayout = dynamic_cast<clcore::VirtualLayout*>(widget);
            if (virtualLayout == nullptr) {
                continue;
            }
            const int preWidth = widget->getWidth();
            const int preHeight = widget->getHeight();
            needSolverPass |= measure(measurer, widget, Measure::TRY_GIVEN_DIMENSIONS);
            const int measuredWidth = widget->getWidth();
            const int measuredHeight = widget->getHeight();
            if (measuredWidth != preWidth) {
                widget->setWidth(measuredWidth);
                if (containerWrapWidth && widget->getRight() > minWidth) {
                    const int w = widget->getRight()
                            + widget->getAnchor(ConstraintAnchor::Type::RIGHT)->getMargin();
                    minWidth = std::max(minWidth, w);
                }
                needSolverPass = true;
            }
            if (measuredHeight != preHeight) {
                widget->setHeight(measuredHeight);
                if (containerWrapHeight && widget->getBottom() > minHeight) {
                    const int h = widget->getBottom()
                            + widget->getAnchor(ConstraintAnchor::Type::BOTTOM)->getMargin();
                    minHeight = std::max(minHeight, h);
                }
                needSolverPass = true;
            }
            needSolverPass |= virtualLayout->needSolverPass();
        }
        ////////////////////////////////////////////////////////////////////////////////////

        const int maxIterations = 2;
        for (int j = 0; j < maxIterations; j++) {
            for (int i = 0; i < sizeDependentWidgetsCount; i++) {
                ConstraintWidget* widget = mVariableDimensionsWidgets[i];
                if ((dynamic_cast<HelperWidget*>(widget) != nullptr
                            && dynamic_cast<clcore::VirtualLayout*>(widget) == nullptr)
                        || dynamic_cast<clcore::Guideline*>(widget) != nullptr) {
                    continue;  // Barrier/Group/Guideline — no self measurement
                }
                if (widget->getVisibility() == ConstraintWidget::GONE) {
                    continue;  // GONE: skip (AndroidX 363)
                }
                // optimize && runs-resolved skip (AndroidX 366-369) rides with the run system.
                if (dynamic_cast<clcore::VirtualLayout*>(widget) != nullptr) {
                    continue;  // the VL block above owns these
                }

                const int preWidth = widget->getWidth();
                const int preHeight = widget->getHeight();
                const int preBaselineDistance = widget->getBaselineDistance();

                const int measureStrategy = (j == maxIterations - 1)
                        ? Measure::USE_GIVEN_DIMENSIONS : Measure::TRY_GIVEN_DIMENSIONS;
                needSolverPass |= measure(measurer, widget, measureStrategy);

                const int measuredWidth = widget->getWidth();
                const int measuredHeight = widget->getHeight();
                if (measuredWidth != preWidth) {
                    widget->setWidth(measuredWidth);
                    if (containerWrapWidth && widget->getRight() > minWidth) {
                        const int w = widget->getRight()
                                + widget->getAnchor(ConstraintAnchor::Type::RIGHT)->getMargin();
                        minWidth = std::max(minWidth, w);
                    }
                    needSolverPass = true;
                }
                if (measuredHeight != preHeight) {
                    widget->setHeight(measuredHeight);
                    if (containerWrapHeight && widget->getBottom() > minHeight) {
                        const int h = widget->getBottom()
                                + widget->getAnchor(ConstraintAnchor::Type::BOTTOM)->getMargin();
                        minHeight = std::max(minHeight, h);
                    }
                    needSolverPass = true;
                }
                // A baseline move alone can invalidate the solve (AndroidX 427-436).
                if (widget->hasBaseline() && preBaselineDistance != widget->getBaselineDistance()) {
                    needSolverPass = true;
                }
            }
            if (needSolverPass) {
                solveLinearSystem(layout, "intermediate pass", 1 + j, startingWidth, startingHeight);
                needSolverPass = false;
            } else {
                break;  // converged
            }
        }
    }
    return 0;
}

} // namespace cdroid
