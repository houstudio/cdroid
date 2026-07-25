/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <widgetEx/constraintlayout/core/linear_system.h>
#include <widgetEx/constraintlayout/core/metrics.h>
#include <widgetEx/constraintlayout/core/widgets/analyzer/basic_measure.h>
#include <widgetEx/constraintlayout/core/widgets/chain_head.h>
#include <widgetEx/constraintlayout/core/widgets/optimizer.h>
#include <widgetEx/constraintlayout/core/widgets/widget_container.h>

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
    void layout() override; // STUB — real solver driver deferred (Stage 3 analyzer)

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
