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
        if (child->isVirtualLayout()) {
            continue; // VirtualLayout (Flow/Layer) sizes itself via its own measure()
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

        if (!skip && layout->optimizeFor(Optimizer::OPTIMIZATION_DIRECT)) {
            // (child instanceof VirtualLayout) guard omitted — VirtualLayout not ported.
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
    // measureChildren -> updateHierarchy -> solveLinearSystem (first pass), then a VirtualLayout
    // pass (Flow/etc. derive their content size), then the match-constraint convergence loop below
    // (maxIterations=2, TRY→USE_GIVEN_DIMENSIONS, early-exit on convergence). Only the
    // OPTIMIZATION_GRAPH optimize-path (directMeasure / DependencyGraph run-system) remains
    // deferred — it is a performance path (off by default in AndroidX) and does not affect the
    // standard linear-solve correctness exercised here.
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

    // VirtualLayout measure (Android BasicMeasure.java 319-352): the first solve above resolved
    // every 0dp dimension, including a MATCH_CONSTRAINT helper's (Flow). Now build each helper's
    // rows with its size (resolved for 0dp, fixed otherwise), then re-solve so its addToSolver can
    // emit the row constraints it could not emit in the first pass (mChainList was empty then).
    // measureChildren() skips all VirtualLayouts, so we measure every VirtualLayout here — this is
    // the faithful fix for the Flow "max=0" bug (previously Flow measured itself from addToSolver
    // with an unresolved size and fell back to the parent's width).
    bool needSolverPass = false;
    // Measure each VirtualLayout with a mode matching its dimension behaviour: EXACTLY for a
    // resolved/fixed dimension, UNSPECIFIED for WRAP_CONTENT so the helper computes that dimension
    // from its content (e.g. a WRAP-height Flow derives its height from the wrapped rows instead of
    // being forced to the unresolved getHeight()==0).
    auto modeFor = [](ConstraintWidget::DimensionBehaviour b) {
        return (b == ConstraintWidget::DimensionBehaviour::WRAP_CONTENT)
               ? BasicMeasure::UNSPECIFIED : BasicMeasure::EXACTLY;
    };
    for (ConstraintWidget* widget : layout->mChildren) {
        auto* vl = dynamic_cast<VirtualLayout*>(widget);
        if (vl == nullptr) {
            continue;
        }
        vl->measure(modeFor(widget->getHorizontalDimensionBehaviour()), widget->getWidth(),
                    modeFor(widget->getVerticalDimensionBehaviour()), widget->getHeight());
        if (vl->needSolverPass()) {
            needSolverPass = true;
        }
    }
    if (needSolverPass) {
        solveLinearSystem(layout, "VirtualLayout pass", 1, startingWidth, startingHeight);
    }

    // Generic match-constraint convergence loop (Android BasicMeasure.java 355-445): re-measure
    // non-helper 0dp widgets with their solver-resolved size so a content-dependent dimension
    // (e.g. text height under the resolved width) can adapt, then re-solve. Bounded to
    // maxIterations=2; exits early on convergence. Additive over the single pass above — typical
    // widgets (height independent of width) converge in one iteration.
    Measurer* measurer = layout->getMeasurer();
    const int maxIterations = 2;
    for (int j = 0; j < maxIterations && measurer != nullptr; j++) {
        bool isLast = (j == maxIterations - 1);
        int strategy = isLast ? Measure::USE_GIVEN_DIMENSIONS : Measure::TRY_GIVEN_DIMENSIONS;
        bool needPass = false;
        for (ConstraintWidget* widget : mVariableDimensionsWidgets) {
            if (dynamic_cast<VirtualLayout*>(widget) != nullptr) continue;  // VL block owns these
            if (dynamic_cast<HelperWidget*>(widget) != nullptr)   continue;  // Barrier/Group/Guideline
            if (widget->isInVirtualLayout())                      continue;  // Flow measures its own
            if (widget->getVisibility() == ConstraintWidget::GONE) continue;  // GONE: skip (AndroidX 360)

            int preWidth    = widget->getWidth();
            int preHeight   = widget->getHeight();
            int preBaseline = widget->getBaselineDistance();
            if (measure(measurer, widget, strategy)) {
                needPass = true;
            }
            // Independent size/baseline-change checks (AndroidX 398-436): robustness if a measurer
            // forgets measuredNeedsSolverPass — a changed dimension or baseline still forces a re-solve.
            if (widget->getWidth()  != preWidth
                    || widget->getHeight() != preHeight
                    || (widget->hasBaseline() && preBaseline != widget->getBaselineDistance())) {
                needPass = true;
            }
        }
        if (needPass) {
            solveLinearSystem(layout, "match-constraint pass", 2 + j, startingWidth, startingHeight);
        } else {
            break;  // converged
        }
    }
    return 0;
}

} // namespace cdroid
