/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.analyzer.BasicMeasure.
 */
#include <widgetEx/constraintlayout/core/widgets/analyzer/basicmeasure.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.h>
#include <widgetEx/constraintlayout/core/widgets/guideline.h>

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
    // MVP path: OPTIMIZATION_GRAPH optimize-path (directMeasure / DependencyGraph) and the
    // match-constraint re-measure loop (BasicMeasure 305-446) are deferred. This implements
    // measureChildren -> updateHierarchy -> solveLinearSystem, which handles fixed-dimension
    // children (the MVP sample). match_constraint children measure with their default size.
    // TODO(analyzer): restore optimize path + size-dependent iteration.
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
    return 0;
}

} // namespace cdroid
