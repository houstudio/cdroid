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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.ConstraintWidgetContainer.
 * SKELETON (Stage 2) — see header.
 */
#include <widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.h>
#include <widgetEx/constraintlayout/core/widgets/chain.h>
#include <widgetEx/constraintlayout/core/widgets/helperwidget.h>

namespace cdroid {

ConstraintWidgetContainer::ConstraintWidgetContainer()
    : mBasicMeasureSolver(this) {
}

ConstraintWidgetContainer::ConstraintWidgetContainer(int x, int y, int width, int height)
    : WidgetContainer(x, y, width, height)
    , mBasicMeasureSolver(this) {
}

ConstraintWidgetContainer::ConstraintWidgetContainer(int width, int height)
    : WidgetContainer(width, height)
    , mBasicMeasureSolver(this) {
}

ConstraintWidgetContainer::ConstraintWidgetContainer(const std::string& debugName,
        int width, int height)
    : WidgetContainer(width, height)
    , mBasicMeasureSolver(this) {
    setDebugName(debugName);
}

ConstraintWidgetContainer::~ConstraintWidgetContainer() {
    clearChains();
}

BasicMeasure::Measurer* ConstraintWidgetContainer::getMeasurer() {
    return mMeasurer;
}

void ConstraintWidgetContainer::setMeasurer(Measurer* measurer) {
    mMeasurer = measurer;
}

long ConstraintWidgetContainer::measure(int optimizationLevel, int paddingX, int paddingY,
                                        int widthMode, int widthSize,
                                        int heightMode, int heightSize,
                                        int lastMeasureWidth, int lastMeasureHeight) {
    return mBasicMeasureSolver.solverMeasure(this, optimizationLevel, paddingX, paddingY,
            widthMode, widthSize, heightMode, heightSize,
            lastMeasureWidth, lastMeasureHeight);
}

// Static per-widget measure used by the Direct fast-path. Ported verbatim from
// androidx ConstraintWidgetContainer.measure (Java:522); DEBUG prints omitted.
bool ConstraintWidgetContainer::measure(int /*level*/, ConstraintWidget* widget,
        BasicMeasure::Measurer* measurer, BasicMeasure::Measure* measure, int measureStrategy) {
    if (measurer == nullptr) {
        return false;
    }
    if (widget->getVisibility() == ConstraintWidget::GONE
            || widget->isGuideline() || widget->isBarrier()) {
        measure->measuredWidth = 0;
        measure->measuredHeight = 0;
        return false;
    }

    measure->horizontalBehavior = widget->getHorizontalDimensionBehaviour();
    measure->verticalBehavior = widget->getVerticalDimensionBehaviour();
    measure->horizontalDimension = widget->getWidth();
    measure->verticalDimension = widget->getHeight();
    measure->measuredNeedsSolverPass = false;
    measure->measureStrategy = measureStrategy;

    bool horizontalMatchConstraints =
            (measure->horizontalBehavior == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT);
    bool verticalMatchConstraints =
            (measure->verticalBehavior == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT);

    bool horizontalUseRatio = horizontalMatchConstraints && widget->mDimensionRatio > 0;
    bool verticalUseRatio = verticalMatchConstraints && widget->mDimensionRatio > 0;

    if (horizontalMatchConstraints && widget->hasDanglingDimension(ConstraintWidget::HORIZONTAL)
            && widget->mMatchConstraintDefaultWidth == ConstraintWidget::MATCH_CONSTRAINT_SPREAD
            && !horizontalUseRatio) {
        horizontalMatchConstraints = false;
        measure->horizontalBehavior = ConstraintWidget::DimensionBehaviour::WRAP_CONTENT;
        if (verticalMatchConstraints
                && widget->mMatchConstraintDefaultHeight == ConstraintWidget::MATCH_CONSTRAINT_SPREAD) {
            // if match x match, size would be zero.
            measure->horizontalBehavior = ConstraintWidget::DimensionBehaviour::FIXED;
        }
    }

    if (verticalMatchConstraints && widget->hasDanglingDimension(ConstraintWidget::VERTICAL)
            && widget->mMatchConstraintDefaultHeight == ConstraintWidget::MATCH_CONSTRAINT_SPREAD
            && !verticalUseRatio) {
        verticalMatchConstraints = false;
        measure->verticalBehavior = ConstraintWidget::DimensionBehaviour::WRAP_CONTENT;
        if (horizontalMatchConstraints
                && widget->mMatchConstraintDefaultWidth == ConstraintWidget::MATCH_CONSTRAINT_SPREAD) {
            // if match x match, size would be zero.
            measure->verticalBehavior = ConstraintWidget::DimensionBehaviour::FIXED;
        }
    }

    if (widget->isResolvedHorizontally()) {
        horizontalMatchConstraints = false;
        measure->horizontalBehavior = ConstraintWidget::DimensionBehaviour::FIXED;
    }
    if (widget->isResolvedVertically()) {
        verticalMatchConstraints = false;
        measure->verticalBehavior = ConstraintWidget::DimensionBehaviour::FIXED;
    }

    if (horizontalUseRatio) {
        if (widget->mResolvedMatchConstraintDefault[ConstraintWidget::HORIZONTAL]
                == ConstraintWidget::MATCH_CONSTRAINT_RATIO_RESOLVED) {
            measure->horizontalBehavior = ConstraintWidget::DimensionBehaviour::FIXED;
        } else if (!verticalMatchConstraints) {
            // let's measure here
            int measuredHeight;
            if (measure->verticalBehavior == ConstraintWidget::DimensionBehaviour::FIXED) {
                measuredHeight = measure->verticalDimension;
            } else {
                measure->horizontalBehavior = ConstraintWidget::DimensionBehaviour::WRAP_CONTENT;
                measurer->measure(widget, measure);
                measuredHeight = measure->measuredHeight;
            }
            measure->horizontalBehavior = ConstraintWidget::DimensionBehaviour::FIXED;
            // getDimensionRatio() is expressed in WxH format, so multiply.
            measure->horizontalDimension = (int) (widget->getDimensionRatio() * measuredHeight);
        }
    }
    if (verticalUseRatio) {
        if (widget->mResolvedMatchConstraintDefault[ConstraintWidget::VERTICAL]
                == ConstraintWidget::MATCH_CONSTRAINT_RATIO_RESOLVED) {
            measure->verticalBehavior = ConstraintWidget::DimensionBehaviour::FIXED;
        } else if (!horizontalMatchConstraints) {
            // let's measure here
            int measuredWidth;
            if (measure->horizontalBehavior == ConstraintWidget::DimensionBehaviour::FIXED) {
                measuredWidth = measure->horizontalDimension;
            } else {
                measure->verticalBehavior = ConstraintWidget::DimensionBehaviour::WRAP_CONTENT;
                measurer->measure(widget, measure);
                measuredWidth = measure->measuredWidth;
            }
            measure->verticalBehavior = ConstraintWidget::DimensionBehaviour::FIXED;
            if (widget->mDimensionRatioSide == -1) {
                // getDimensionRatio() is WxH, so divide.
                measure->verticalDimension = (int) (measuredWidth / widget->getDimensionRatio());
            } else {
                // ratio was reverted, so multiply.
                measure->verticalDimension = (int) (widget->getDimensionRatio() * measuredWidth);
            }
        }
    }

    measurer->measure(widget, measure);
    widget->setWidth(measure->measuredWidth);
    widget->setHeight(measure->measuredHeight);
    widget->setHasBaseline(measure->measuredHasBaseline);
    widget->setBaselineDistance(measure->measuredBaseline);
    measure->measureStrategy = BasicMeasure::Measure::SELF_DIMENSIONS;
    return measure->measuredNeedsSolverPass;
}

void ConstraintWidgetContainer::invalidateGraph() {
    // MVP no-op — the DependencyGraph (run-system) is deferred.
}

void ConstraintWidgetContainer::setPass(int /*pass*/) {
    // MVP no-op (mPass unused by the linear-solve driver).
}

void ConstraintWidgetContainer::clearChains() {
    for (ChainHead* head : mHorizontalChainsArray) {
        delete head;
    }
    for (ChainHead* head : mVerticalChainsArray) {
        delete head;
    }
    mHorizontalChainsArray.clear();
    mVerticalChainsArray.clear();
    mHorizontalChainsSize = 0;
    mVerticalChainsSize = 0;
}

void ConstraintWidgetContainer::addChain(ConstraintWidget* widget, int type) {
    if (type == ConstraintWidget::HORIZONTAL) {
        mHorizontalChainsArray.push_back(new ChainHead(widget, ConstraintWidget::HORIZONTAL, mIsRtl));
        mHorizontalChainsSize = (int) mHorizontalChainsArray.size();
    } else if (type == ConstraintWidget::VERTICAL) {
        mVerticalChainsArray.push_back(new ChainHead(widget, ConstraintWidget::VERTICAL, mIsRtl));
        mVerticalChainsSize = (int) mVerticalChainsArray.size();
    }
}

// DEFERRED(wrap-opt): wrap-content min/max variable tracking. Java tracks these via
// WeakReference<ConstraintAnchor> + addMinWrap/addMaxWrap (which add solver constraints to bound
// the parent's wrap size). Only relevant for WRAP_CONTENT parents; the fixed-size MVP doesn't
// exercise it. Restore when wrap-content optimization is ported.
void ConstraintWidgetContainer::addHorizontalWrapMinVariable(ConstraintAnchor* /*left*/) {}
void ConstraintWidgetContainer::addHorizontalWrapMaxVariable(ConstraintAnchor* /*right*/) {}
void ConstraintWidgetContainer::addVerticalWrapMinVariable(ConstraintAnchor* /*top*/) {}
void ConstraintWidgetContainer::addVerticalWrapMaxVariable(ConstraintAnchor* /*bottom*/) {}

void ConstraintWidgetContainer::setOptimizationLevel(int value) {
    mOptimizationLevel = value;
}

int ConstraintWidgetContainer::getOptimizationLevel() const {
    return mOptimizationLevel;
}

bool ConstraintWidgetContainer::optimizeFor(int feature) const {
    return (mOptimizationLevel & feature) == feature;
}

std::string ConstraintWidgetContainer::getType() const {
    return "ConstraintLayout";
}

LinearSystem& ConstraintWidgetContainer::getSystem() {
    return mSystem;
}

void ConstraintWidgetContainer::setRtl(bool isRtl) {
    mIsRtl = isRtl;
}

bool ConstraintWidgetContainer::isRtl() const {
    return mIsRtl;
}

void ConstraintWidgetContainer::reset() {
    clearChains();
    mSkipSolver = false;
    WidgetContainer::reset();
}

void ConstraintWidgetContainer::layout() {
    // Linear-solve driver (the faithful core of Java layout()). Iterates ≤MAX_ITERATIONS: reset the
    // system, rebuild chains, add every child to the solver (helpers first), solve, read back. The
    // match-constraint re-measure convergence lives in BasicMeasure::solverMeasure (which wraps this);
    // the onMeasure shrink handles WRAP_CONTENT. Only the OPTIMIZATION_GRAPH fast-paths — Direct
    // solver (OPTIMIZATION_DIRECT), Grouping (OPTIMIZATION_GROUPING), the DependencyGraph optimizer,
    // and wrap-content min/max variable tracking — remain deferred; they are a performance layer
    // (off by default in AndroidX) and do not affect the linear-solve correctness here.
    mX = 0;
    mY = 0;
    const int count = (int) mChildren.size();

    // Layout nested containers first.
    for (int i = 0; i < count; i++) {
        if (auto* c = dynamic_cast<WidgetContainer*>(mChildren[i])) {
            c->layout();
        }
    }

    constexpr int MAX_ITERATIONS = 8;
    int countSolve = 0;
    bool needsSolving = true;
    while (needsSolving && countSolve < MAX_ITERATIONS) {
        countSolve++;
        mSystem.reset();
        clearChains(); // rebuilt by child addToSolver -> isChainHead -> addChain

        // Pin the root container to (0,0) + its size.
        mSystem.addEquality(mSystem.createObjectVariable(&mLeft), 0);
        mSystem.addEquality(mSystem.createObjectVariable(&mRight), mWidth);
        mSystem.addEquality(mSystem.createObjectVariable(&mTop), 0);
        mSystem.addEquality(mSystem.createObjectVariable(&mBottom), mHeight);

        // Add every child to the solver. HelperWidget children (Flow/Barrier/...) run FIRST so they
        // can wire their referenced children's anchor targets before those children solve.
        for (int i = 0; i < count; i++) {
            if (dynamic_cast<HelperWidget*>(mChildren[i]) != nullptr) {
                mChildren[i]->addToSolver(&mSystem, /*optimize=*/false);
            }
        }
        for (int i = 0; i < count; i++) {
            if (dynamic_cast<HelperWidget*>(mChildren[i]) == nullptr) {
                mChildren[i]->addToSolver(&mSystem, /*optimize=*/false);
            }
        }

        // Apply chain constraints (built above via addChain).
        Chain::applyChainConstraints(this, &mSystem, nullptr, ConstraintWidget::HORIZONTAL);
        Chain::applyChainConstraints(this, &mSystem, nullptr, ConstraintWidget::VERTICAL);

        mSystem.minimize();

        // Read back positions.
        updateFromSolver(&mSystem, /*optimize=*/false);
        for (int i = 0; i < count; i++) {
            mChildren[i]->updateFromSolver(&mSystem, /*optimize=*/false);
        }
        needsSolving = false; // per-iteration flag (BasicMeasure drives the outer match-constraint loop)
    }
}

} // namespace cdroid
