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
 *
 * SKELETON: owns the LinearSystem + chain-head bookkeeping (read by Chain). The real solver
 * driver — layout()/measure()/addChildrenToSolver()/updateChildrenFromSolver()/directMeasure*()
 * — is heavily coupled to the analyzer (BasicMeasure/DependencyGraph/Direct/Grouping, Stage 3)
 * and is stubbed here. Restore those when the analyzer is ported.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_CONSTRAINT_WIDGET_CONTAINER_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_CONSTRAINT_WIDGET_CONTAINER_H

#include <string>
#include <vector>

#include <widgetEx/constraintlayout/core/linearsystem.h>
#include <widgetEx/constraintlayout/core/metrics.h>
#include <widgetEx/constraintlayout/core/widgets/analyzer/basicmeasure.h>
#include <widgetEx/constraintlayout/core/widgets/chainhead.h>
#include <widgetEx/constraintlayout/core/widgets/optimizer.h>
#include <widgetEx/constraintlayout/core/widgets/widgetcontainer.h>

namespace cdroid {

class ConstraintWidgetContainer : public WidgetContainer {
  public:
    ConstraintWidgetContainer();
    ConstraintWidgetContainer(int x, int y, int width, int height);
    ConstraintWidgetContainer(int width, int height);
    ConstraintWidgetContainer(const std::string& debugName, int width, int height);
    virtual ~ConstraintWidgetContainer();

    // --- measure bridge (called by ConstraintLayout.onMeasure) ---
    using Measurer = BasicMeasure::Measurer;
    Measurer* getMeasurer();
    void setMeasurer(Measurer* measurer);
    // Delegates to mBasicMeasureSolver.solverMeasure (the measure+solve orchestrator).
    long measure(int optimizationLevel, int paddingX, int paddingY,
                 int widthMode, int widthSize, int heightMode, int heightSize,
                 int lastMeasureWidth, int lastMeasureHeight);
    // Static per-widget measure used by the Direct fast-path (Java ConstraintWidgetContainer:522).
    // Distinct from the instance measure() above (different params, static).
    static bool measure(int level, ConstraintWidget* widget, BasicMeasure::Measurer* measurer,
                        BasicMeasure::Measure* measure, int measureStrategy);
    void invalidateGraph();  // MVP: no-op (DependencyGraph deferred)
    void setPass(int pass);  // MVP: no-op

    Metrics* mMetrics = nullptr;

    // --- optimization level ---
    void setOptimizationLevel(int value);
    int  getOptimizationLevel() const;
    bool optimizeFor(int feature) const;

    std::string getType() const override; // "ConstraintLayout"
    LinearSystem& getSystem();

    void setRtl(bool isRtl);
    bool isRtl() const;

    // Register the chain starting at `widget` (called by ConstraintWidget::addToSolver when the
    // widget is a chain head). Owns the created ChainHead (freed on reset/destruction).
    void addChain(ConstraintWidget* widget, int type);

    // Wrap-content min/max tracking (mOptimizeWrapOnResolved path). Stubbed — the wrap
    // optimization (WeakReference<ConstraintAnchor> + addMinWrap solver constraints) is deferred;
    // only matters for WRAP_CONTENT parents, not the fixed-size MVP. No-op for now.
    void addHorizontalWrapMinVariable(ConstraintAnchor* left);
    void addHorizontalWrapMaxVariable(ConstraintAnchor* right);
    void addVerticalWrapMinVariable(ConstraintAnchor* top);
    void addVerticalWrapMaxVariable(ConstraintAnchor* bottom);

    void reset() override;
    void layout() override; // linear-solve driver (≤8 iters, nested-first, chain rebuild); only the OPTIMIZATION_GRAPH fast-paths (Direct/Grouping/graph) are deferred

    // --- chain bookkeeping (read by Chain.applyChainConstraints) ---
    // Java: ChainHead[4] + mHorizontalChainsSize; here a vector + active count.
    std::vector<ChainHead*> mHorizontalChainsArray;
    int mHorizontalChainsSize = 0;
    std::vector<ChainHead*> mVerticalChainsArray;
    int mVerticalChainsSize = 0;

    bool mSkipSolver = false;

  private:
    void clearChains(); // delete owned ChainHead* + reset arrays/sizes

    LinearSystem mSystem;
    BasicMeasure mBasicMeasureSolver;
    Measurer* mMeasurer = nullptr;
    int  mOptimizationLevel = Optimizer::OPTIMIZATION_STANDARD;
    bool mIsRtl = false;
    int  mPass = 0;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_CONSTRAINT_WIDGET_CONTAINER_H
