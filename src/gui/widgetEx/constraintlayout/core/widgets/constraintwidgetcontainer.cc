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
#include <widgetEx/constraintlayout/core/widgets/barrier.h>
#include <widgetEx/constraintlayout/core/widgets/chain.h>
#include <widgetEx/constraintlayout/core/widgets/helperwidget.h>
#include <widgetEx/constraintlayout/core/widgets/optimizer.h>
#include <widgetEx/constraintlayout/core/widgets/virtuallayout.h>

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
}

void ConstraintWidgetContainer::addChain(ConstraintWidget* widget, int type) {
    if (type == ConstraintWidget::HORIZONTAL) {
        mHorizontalChainsArray.push_back(new ChainHead(widget, ConstraintWidget::HORIZONTAL, mIsRtl));
    } else if (type == ConstraintWidget::VERTICAL) {
        mVerticalChainsArray.push_back(new ChainHead(widget, ConstraintWidget::VERTICAL, mIsRtl));
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
    // Clear final values our own anchors may carry from a previous layout pass (a FIXED dimension
    // finalizes mRight/mBottom for the chain fast-path). Without this, switching a dimension to
    // WRAP_CONTENT between layouts leaves it finalized — and createObjectVariable() then returns
    // the stale final value as a constant instead of a free variable, so the wrap constraints can
    // no longer size the container (multi-scenario wrap tests regress). FIXED dimensions are
    // re-finalized inside the solve loop below.
    resetFinalResolution();
    const int count = (int) mChildren.size();

    // Behaviours as handed to us; restored at the exit if a wrap override changed
    // them (ConstraintWidgetContainer.java:684-686, restore at :1001-1004).
    const DimensionBehaviour originalHorizontal = mListDimensionBehaviors[DIMENSION_HORIZONTAL];
    const DimensionBehaviour originalVertical = mListDimensionBehaviors[VERTICAL];
    bool wrapOverride = false;

    // Layout nested containers first.
    for (int i = 0; i < count; i++) {
        if (auto* c = dynamic_cast<WidgetContainer*>(mChildren[i])) {
            c->layout();
        }
    }

    // AndroidX pre-pass sizes (ConstraintWidgetContainer.java:682-687): what the measure
    // pass handed us; layout override 3 compares against these to flag measured-too-small.
    const int preW = std::max(0, getWidth());
    const int preH = std::max(0, getHeight());

    mWidthMeasuredTooSmall = false;
    mHeightMeasuredTooSmall = false;

    constexpr int MAX_ITERATIONS = 8;
    int countSolve = 0;
    bool needsSolving = true;
    while (needsSolving && countSolve < MAX_ITERATIONS) {
        countSolve++;
        mSystem.reset();
        clearChains(); // rebuilt by child addToSolver -> isChainHead -> addChain

        // AndroidX (java:851-855): register the container's and every child's anchor
        // variables ahead of the solve.
        createObjectVariables(&mSystem);
        for (int i = 0; i < count; i++) {
            mChildren[i]->createObjectVariables(&mSystem);
        }

        // AndroidX registers the container's anchors (java:851), then addChildrenToSolver
        // adds the container ITSELF to the solve first (java:334) — its applyConstraints
        // brings the WRAP_CONTENT min/max constraints. CDROID keeps the hand-pinned origin
        // instead (the root's WRAP shrink is driven by the per-child wrap constraints from
        // ConstraintWidget::applyConstraints; the container's own wrap pass interacts with
        // the BasicMeasure match-constraint convergence in ways upstream does not exercise
        // the same way — WrapContainerWithMatchConstraintMax is the sentinel). Revisit with
        // the measured-too-small reporting hookup below.
        mSystem.addEquality(mSystem.createObjectVariable(&mLeft), 0);
        mSystem.addEquality(mSystem.createObjectVariable(&mTop), 0);
        const bool wrapH = (mListDimensionBehaviors[DIMENSION_HORIZONTAL]
                == DimensionBehaviour::WRAP_CONTENT);
        const bool wrapV = (mListDimensionBehaviors[VERTICAL]
                == DimensionBehaviour::WRAP_CONTENT);
        if (!wrapH) {
            mSystem.addEquality(mSystem.createObjectVariable(&mRight), mWidth);
        }
        if (!wrapV) {
            mSystem.addEquality(mSystem.createObjectVariable(&mBottom), mHeight);
        }

        // AndroidX addChildrenToSolver (java:334-448): barriers mark, addFirst widgets
        // (Guidelines) resolve, virtual layouts dependency-order, nested containers join at
        // WRAP->FIXED (java:414-431 — the old helpers-first split flattened a nested wrap
        // container's freshly-solved size back to zero), and the chains apply.
        needsSolving = addChildrenToSolverInner(&mSystem);
        if (needsSolving) {
            mSystem.minimize();
        }

        if (needsSolving) {
            needsSolving = updateChildrenFromSolver(&mSystem);
        } else {
            updateFromSolver(&mSystem, /*optimize=*/false);
            for (int i = 0; i < count; i++) {
                mChildren[i]->updateFromSolver(&mSystem, /*optimize=*/false);
            }
            needsSolving = false;
        }

        // AndroidX "layout override 2" (java:931-953, unconditional there): the solved size
        // respects mMinWidth/mMinHeight — clamp, flip to FIXED and re-solve so the chains
        // spread their MATCH_CONSTRAINT elements across the enforced size. (The former
        // custom variant re-derived the trigger from the anchor variables; with the
        // container itself in the solve the plain width/height comparison is faithful.)
        int width = std::max(mMinWidth, getWidth());
        if (width > getWidth()) {
            setWidth(width);
            mListDimensionBehaviors[DIMENSION_HORIZONTAL] = DimensionBehaviour::FIXED;
            wrapOverride = true;
            needsSolving = true;
        }
        int height = std::max(mMinHeight, getHeight());
        if (height > getHeight()) {
            setHeight(height);
            mListDimensionBehaviors[VERTICAL] = DimensionBehaviour::FIXED;
            wrapOverride = true;
            needsSolving = true;
        }

        // AndroidX "layout override 3" (java:955-986): a WRAP dimension that solved LARGER
        // than the measure-pass size reports measured-too-small and re-solves pinned to the
        // measured size — the caller learns the content did not fit.
        if (!wrapOverride) {
            if (mListDimensionBehaviors[DIMENSION_HORIZONTAL] == DimensionBehaviour::WRAP_CONTENT
                    && preW > 0 && getWidth() > preW) {
                mWidthMeasuredTooSmall = true;
                wrapOverride = true;
                mListDimensionBehaviors[DIMENSION_HORIZONTAL] = DimensionBehaviour::FIXED;
                setWidth(preW);
                needsSolving = true;
            }
            if (mListDimensionBehaviors[VERTICAL] == DimensionBehaviour::WRAP_CONTENT
                    && preH > 0 && getHeight() > preH) {
                mHeightMeasuredTooSmall = true;
                wrapOverride = true;
                mListDimensionBehaviors[VERTICAL] = DimensionBehaviour::FIXED;
                setHeight(preH);
                needsSolving = true;
            }
        }

        if (countSolve > MAX_ITERATIONS) {
            needsSolving = false;
        }
    }

    // AndroidX restores the caller's behaviours after a wrap override — the container
    // is laid out at the enforced size but keeps reporting WRAP_CONTENT to its caller
    // (ConstraintWidgetContainer.java:1001-1004).
    if (wrapOverride) {
        mListDimensionBehaviors[DIMENSION_HORIZONTAL] = originalHorizontal;
        mListDimensionBehaviors[VERTICAL] = originalVertical;
    }
}


// AndroidX addChildrenToSolver (ConstraintWidgetContainer.java:334-448). USE_DEPENDENCY_ORDERING
// is false upstream, so the plain ordered branch runs (java:412-438).
bool ConstraintWidgetContainer::addChildrenToSolverInner(LinearSystem* system) {
    const int count = (int) mChildren.size();
    bool hasBarriers = false;
    for (int i = 0; i < count; i++) {
        ConstraintWidget* widget = mChildren[i];
        widget->setIsInBarrier(HORIZONTAL, false);
        widget->setIsInBarrier(VERTICAL, false);
        if (dynamic_cast<clcore::Barrier*>(widget) != nullptr) hasBarriers = true;
    }
    if (hasBarriers) {
        for (int i = 0; i < count; i++) {
            if (auto* barrier = dynamic_cast<clcore::Barrier*>(mChildren[i])) {
                barrier->markWidgets();
            }
        }
    }

    // addFirst widgets: Guidelines solve immediately; virtual layouts collect for
    // dependency ordering (a layout referencing another layout solves first, java:356-391).
    std::vector<ConstraintWidget*> widgetsToAdd;
    for (int i = 0; i < count; i++) {
        ConstraintWidget* widget = mChildren[i];
        if (widget->addFirst()) {
            if (dynamic_cast<VirtualLayout*>(widget) != nullptr) {
                widgetsToAdd.push_back(widget);
            } else {
                widget->addToSolver(system, /*optimize=*/false);
            }
        }
    }
    while (!widgetsToAdd.empty()) {
        const int numLayouts = (int) widgetsToAdd.size();
        bool progressed = false;
        for (auto it = widgetsToAdd.begin(); it != widgetsToAdd.end(); ++it) {
            auto* layout = dynamic_cast<VirtualLayout*>(*it);
            if (layout->contains(widgetsToAdd)) {
                layout->addToSolver(system, /*optimize=*/false);
                widgetsToAdd.erase(it);
                progressed = true;
                break;
            }
        }
        if (!progressed) {
            // no dependency found among the rest — add them all (java:384-390)
            for (ConstraintWidget* widget : widgetsToAdd) {
                widget->addToSolver(system, /*optimize=*/false);
            }
            widgetsToAdd.clear();
        }
    }

    for (int i = 0; i < count; i++) {
        ConstraintWidget* widget = mChildren[i];
        if (auto* nested = dynamic_cast<ConstraintWidgetContainer*>(widget)) {
            // java:414-431: a nested container joins the parent solve at its own resolved
            // size — WRAP_CONTENT temporarily becomes FIXED around its addToSolver, then
            // the caller's behaviour is restored. Without the flip the wrap branch zeroes
            // the size the child's own layout() just resolved.
            const DimensionBehaviour horizontalBehaviour =
                    nested->mListDimensionBehaviors[DIMENSION_HORIZONTAL];
            const DimensionBehaviour verticalBehaviour =
                    nested->mListDimensionBehaviors[VERTICAL];
            if (horizontalBehaviour == DimensionBehaviour::WRAP_CONTENT) {
                nested->setHorizontalDimensionBehaviour(DimensionBehaviour::FIXED);
            }
            if (verticalBehaviour == DimensionBehaviour::WRAP_CONTENT) {
                nested->setVerticalDimensionBehaviour(DimensionBehaviour::FIXED);
            }
            nested->addToSolver(system, /*optimize=*/false);
            if (horizontalBehaviour == DimensionBehaviour::WRAP_CONTENT) {
                nested->setHorizontalDimensionBehaviour(horizontalBehaviour);
            }
            if (verticalBehaviour == DimensionBehaviour::WRAP_CONTENT) {
                nested->setVerticalDimensionBehaviour(verticalBehaviour);
            }
        } else {
            Optimizer::checkMatchParent(this, system, widget);
            if (!widget->addFirst()) {
                widget->addToSolver(system, /*optimize=*/false);
            }
        }
    }

    if (!mHorizontalChainsArray.empty()) {
        Chain::applyChainConstraints(this, system, nullptr, HORIZONTAL);
    }
    if (!mVerticalChainsArray.empty()) {
        Chain::applyChainConstraints(this, system, nullptr, VERTICAL);
    }
    return true;
}

// AndroidX updateChildrenFromSolver (ConstraintWidgetContainer.java:455-469).
bool ConstraintWidgetContainer::updateChildrenFromSolver(LinearSystem* system) {
    updateFromSolver(system, /*optimize=*/false);
    const int count = (int) mChildren.size();
    bool hasOverride = false;
    for (int i = 0; i < count; i++) {
        ConstraintWidget* widget = mChildren[i];
        widget->updateFromSolver(system, /*optimize=*/false);
        if (widget->hasDimensionOverride()) {
            hasOverride = true;
        }
    }
    return hasOverride;
}

} // namespace cdroid
