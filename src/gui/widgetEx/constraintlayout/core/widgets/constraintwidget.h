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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.ConstraintWidget.
 *
 * SKELETON (Stage 2): anchors + geometry + visibility + accessors needed by the solver
 * widget-bridge and ConstraintAnchor. Measure/layout/analyzer/draw methods (the bulk of the
 * 3840-line Java original) are deferred to Stage 3/4.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_CONSTRAINT_WIDGET_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_CONSTRAINT_WIDGET_H

#include <climits>
#include <string>
#include <unordered_map>
#include <vector>

#include <widgetEx/constraintlayout/core/widgets/constraintanchor.h>

namespace cdroid {

class Cache;
class LinearSystem;
class SolverVariable;
class HorizontalWidgetRun; // analyzer (Stage 3) — graph-solver run, set by DependencyGraph
class VerticalWidgetRun;

class ConstraintWidget {
  public:
    // --- match constraint modes ---
    static const int MATCH_CONSTRAINT_SPREAD         = 0;
    static const int MATCH_CONSTRAINT_WRAP           = 1;
    static const int MATCH_CONSTRAINT_PERCENT        = 2;
    static const int MATCH_CONSTRAINT_RATIO          = 3;
    static const int MATCH_CONSTRAINT_RATIO_RESOLVED = 4;

    // sentinel for match-constraint min/max == "use wrapped dimension" (Java: WRAP = -2)
    static const int WRAP = -2;

    static const int UNKNOWN    = -1;
    static const int HORIZONTAL = 0;
    static const int VERTICAL   = 1;
    static const int BOTH       = 2;

    // --- visibility ---
    static const int VISIBLE   = 0;
    static const int INVISIBLE = 4;
    static const int GONE      = 8;

    // --- chain styles ---
    static const int CHAIN_SPREAD       = 0;
    static const int CHAIN_SPREAD_INSIDE = 1;
    static const int CHAIN_PACKED       = 2;

    // --- wrap behavior ---
    static const int WRAP_BEHAVIOR_INCLUDED        = 0; // default
    static const int WRAP_BEHAVIOR_HORIZONTAL_ONLY = 1;
    static const int WRAP_BEHAVIOR_VERTICAL_ONLY   = 2;
    static const int WRAP_BEHAVIOR_SKIPPED         = 3;

    // --- anchor indices ---
    static const int ANCHOR_LEFT    = 0;
    static const int ANCHOR_RIGHT   = 1;
    static const int ANCHOR_TOP     = 2;
    static const int ANCHOR_BOTTOM  = 3;
    static const int ANCHOR_BASELINE = 4;

    // --- dimension behaviour (how width/height are determined) ---
    enum class DimensionBehaviour { FIXED, WRAP_CONTENT, MATCH_CONSTRAINT, MATCH_PARENT };

    // dimension indices (note: equal to HORIZONTAL/VERTICAL)
    static const int DIMENSION_HORIZONTAL = 0;
    static const int DIMENSION_VERTICAL   = 1;

    // resolution hint set by fast-paths (Optimizer.checkMatchParent marks a widget DIRECT)
    static const int DIRECT = 2;

    // default bias for centering constraints (Java: public static float DEFAULT_BIAS = 0.5f)
    static constexpr float DEFAULT_BIAS = 0.5f;

    // All anchors are value members, constructed with `this` as owner (mirrors Java's
    // `new ConstraintAnchor(this, Type.X)` field initializers).
    ConstraintAnchor mLeft    {this, ConstraintAnchor::Type::LEFT};
    ConstraintAnchor mTop     {this, ConstraintAnchor::Type::TOP};
    ConstraintAnchor mRight   {this, ConstraintAnchor::Type::RIGHT};
    ConstraintAnchor mBottom  {this, ConstraintAnchor::Type::BOTTOM};
    ConstraintAnchor mBaseline{this, ConstraintAnchor::Type::BASELINE};
    ConstraintAnchor mCenterX {this, ConstraintAnchor::Type::CENTER_X};
    ConstraintAnchor mCenterY {this, ConstraintAnchor::Type::CENTER_Y};
    ConstraintAnchor mCenter  {this, ConstraintAnchor::Type::CENTER};

    ConstraintAnchor* mListAnchors[6] = {
        &mLeft, &mRight, &mTop, &mBottom, &mBaseline, &mCenter
    };

    ConstraintWidget* mParent = nullptr;

    // --- geometry ---
    int mWidth  = 0;
    int mHeight = 0;
    int mX      = 0;
    int mY      = 0;
    int mBaselineDistance = 0;

    // --- dimension behaviour / resolution (read by the solver driver) ---
    DimensionBehaviour mListDimensionBehaviors[2] = {
        DimensionBehaviour::FIXED, DimensionBehaviour::FIXED
    };
    int mHorizontalResolution = UNKNOWN; // set by fast-path (DIRECT) or solver
    int mVerticalResolution   = UNKNOWN;
    int mMatchConstraintDefaultWidth  = MATCH_CONSTRAINT_SPREAD;
    int mMatchConstraintDefaultHeight = MATCH_CONSTRAINT_SPREAD;

    // --- chain state (populated by ChainHead.define(); read by Chain.applyChainConstraints) ---
    ConstraintWidget* mListNextMatchConstraintsWidget[2] = {nullptr, nullptr};
    ConstraintWidget* mNextChainWidget[2]                = {nullptr, nullptr};
    int   mResolvedMatchConstraintDefault[2] = {0, 0};
    float mWeight[2]                = {UNKNOWN, UNKNOWN};
    int   mMatchConstraintMinWidth  = 0;
    int   mMatchConstraintMaxWidth  = 0;
    int   mMatchConstraintMinHeight = 0;
    int   mMatchConstraintMaxHeight = 0;
    float mDimensionRatio = 0;

    // --- chain style / bias / barrier membership ---
    int   mHorizontalChainStyle  = CHAIN_SPREAD;
    int   mVerticalChainStyle    = CHAIN_SPREAD;
    float mHorizontalBiasPercent = DEFAULT_BIAS;
    float mVerticalBiasPercent   = DEFAULT_BIAS;
    bool  mIsInBarrier[2]        = {false, false};
    bool  mInPlaceholder         = false;
    bool  mIsInVirtualLayout     = false;
    bool  mMeasureRequested      = true;

    // --- solver-population state (read/written by addToSolver / applyConstraints) ---
    bool  mAnimated                = false;
    bool  mOptimizeWrapOnResolved  = true;
    int   mWrapBehaviorInParent    = WRAP_BEHAVIOR_INCLUDED;
    bool  isTerminalWidget[2]      = {true, true};
    int   mMaxDimension[2]         = {INT_MAX, INT_MAX};
    float mMatchConstraintPercentWidth  = 1;
    float mMatchConstraintPercentHeight = 1;
    int   mDimensionRatioSide      = UNKNOWN;       // Java: protected
    int   mResolvedDimensionRatioSide = UNKNOWN;
    float mResolvedDimensionRatio  = 1.0f;
    bool  mResolvedHasRatio        = false;
    float mCircleConstraintAngle   = 0;   // circular constraint angle (radians offset) for center connections

    // --- analyzer runs (OPTIMIZATION_GRAPH path; created by DependencyGraph, null otherwise) ---
    HorizontalWidgetRun* mHorizontalRun = nullptr;
    VerticalWidgetRun*   mVerticalRun   = nullptr;

    // back-pointer to the host View (set by the widget layer, e.g. ConstraintLayout) — stored as
    // void* to avoid a header cycle; cast to View* at the call site.
    void* mCompanionWidget = nullptr;
    void setCompanionWidget(void* companion) {
        mCompanionWidget = companion;
    }
    void* getCompanionWidget() {
        return mCompanionWidget;
    }

    // --- offset (relative to the root container) ---
    int mOffsetX = 0;
    int mOffsetY = 0;

    // --- minimum size (applied in setHorizontal/VerticalDimension) ---
    int mMinWidth  = 0;
    int mMinHeight = 0;

    ConstraintWidget();
    ConstraintWidget(const std::string& debugName);
    ConstraintWidget(int width, int height);
    ConstraintWidget(const std::string& debugName, int width, int height);
    ConstraintWidget(int x, int y, int width, int height);
    ConstraintWidget(const std::string& debugName, int x, int y, int width, int height);

    virtual ~ConstraintWidget();

    // --- anchors ---
    virtual ConstraintAnchor* getAnchor(ConstraintAnchor::Type anchorType);
    const std::vector<ConstraintAnchor*>& getAnchors() const;
    virtual void resetSolverVariables(Cache* cache);

    // --- visibility ---
    int  getVisibility() const;
    void setVisibility(int visibility);

    // --- parent / root ---
    ConstraintWidget* getParent() const;
    void setParent(ConstraintWidget* widget);
    bool isRoot() const;

    // --- baseline ---
    bool hasBaseline() const;
    void setHasBaseline(bool hasBaseline);

    // --- debug name / type ---
    std::string getDebugName() const;
    void setDebugName(const std::string& name);
    virtual std::string getType() const;
    void setType(const std::string& type);

    // --- barrier / resolution hooks (overridden by Guideline/Barrier/...) ---
    virtual bool allowedInBarrier() const;
    virtual bool isBarrier() const; // Barrier (Stage 5) overrides to true
    virtual bool isVirtualLayout() const; // VirtualLayout (Flow/Layer) overrides to true
    bool isInBarrier(int orientation) const;
    void setIsInBarrier(int orientation, bool value);
    // Pulled into a Placeholder (the content view's widget is flagged so it is treated as gone at
    // its origin while the placeholder carries its size/position).
    bool isInPlaceholder() const;
    void setInPlaceholder(bool inPlaceholder);

    // --- chain membership / dependency queries (used by addToSolver) ---
    bool hasDependencies() const;
    bool isInHorizontalChain() const;
    bool isInVerticalChain() const;
    virtual bool isResolvedHorizontally() const;
    virtual bool isResolvedVertically() const;

    // --- final-resolution fast-path (Barrier/Guideline pre-resolution) ---
    // Set the widget's horizontal anchors to their final values (skipped if already resolved).
    // Java: setFinalHorizontal(int x1, int x2). Used by Barrier.allSolved() once referenced
    // widgets are all resolved, so the barrier pins itself without going through the solver.
    void setFinalHorizontal(int x1, int x2);
    void setFinalVertical(int y1, int y2);
    void setFinalBaseline(int baselineValue);
    // Clear final-resolution state on every anchor (Java: resetFinalResolution).
    void resetFinalResolution();

    // --- solver lifecycle (called by ConstraintWidgetContainer driver) ---
    // Base implementations are stubbed; Guideline overrides with real bodies. The full base
    // addToSolver/updateFromSolver depend on Chain (Stage 2) + analyzer runs (Stage 3).
    virtual void addToSolver(LinearSystem* system, bool optimize);
    virtual void updateFromSolver(LinearSystem* system, bool optimize);
    virtual void copy(ConstraintWidget* src,
                      std::unordered_map<ConstraintWidget*, ConstraintWidget*>& map);

    // Resolve which side a dimension ratio applies to (called by addToSolver when ratio set).
    void setupDimensionRatio(bool hParentWrapContent, bool vParentWrapContent,
                             bool horizontalDimensionFixed, bool verticalDimensionFixed);

    // --- geometry accessors ---
    int getX() const;
    int getY() const;
    int getWidth() const;
    int getHeight() const;
    int getLength(int orientation) const;
    int getMinWidth() const;
    int getMinHeight() const;
    void setMinWidth(int minWidth);
    void setMinHeight(int minHeight);
    void setMaxWidth(int maxWidth);
    void setMaxHeight(int maxHeight);
    void setBaselineDistance(int baselineDistance);
    int getBaselineDistance() const { return mBaselineDistance; }
    float getDimensionRatio() const;
    bool isInVirtualLayout() const;
    void setInVirtualLayout(bool inVirtualLayout);
    // Connect one of this widget's anchors to a target anchor (Java: connect(from, to, margin)).
    void connect(ConstraintAnchor& from, ConstraintAnchor* to, int margin);
    // Position this widget on a circle of `radius` around `target` at `angle` degrees (Java:
    // connectCircularConstraint). Connects CENTER→CENTER with radius as margin and stores the angle,
    // which LinearSystem::addCenterPoint consumes (constraintwidget.cc) to emit the center constraint.
    void connectCircularConstraint(ConstraintWidget* target, float angle, int radius);
    // Reset every anchor's connection (Java: resetAnchors).
    void resetAnchors();
    // Re-measure request flag (Flow uses it for percent/match-constraint children).
    void setMeasureRequested(bool measureRequested);
    bool isMeasureRequested() const;
    void setX(int x);
    void setY(int y);
    void setWidth(int width);
    void setHeight(int height);
    virtual void setOffset(int x, int y);

    // --- dimension setters (called by the solver fast-path / read-back) ---
    void setHorizontalDimension(int left, int right);
    void setVerticalDimension(int top, int bottom);
    void setFrame(int left, int top, int right, int bottom);

    // --- dimension behaviour accessors ---
    DimensionBehaviour getHorizontalDimensionBehaviour() const;
    DimensionBehaviour getVerticalDimensionBehaviour() const;
    DimensionBehaviour getDimensionBehaviour(int orientation) const;
    void setHorizontalDimensionBehaviour(DimensionBehaviour behaviour);
    void setVerticalDimensionBehaviour(DimensionBehaviour behaviour);
    void setDimensionBehaviour(int orientation, DimensionBehaviour behaviour);

    // --- reset (full; called between solving passes) ---
    virtual void reset();

  protected:
    std::vector<ConstraintAnchor*> mAnchors;

  private:
    int          mVisibility = VISIBLE;
    bool         mHasBaseline = false;
    bool         mResolvedHorizontal = false;
    bool         mResolvedVertical = false;
    std::string  mDebugName;
    std::string  mType;
    int          mWidthOverride  = -1; // one-shot dimension override consumed in applyConstraints
    int          mHeightOverride = -1;

    void addAnchors();
    bool isChainHead(int orientation) const;

    // Apply one dimension's constraints into the system (called by addToSolver).
    void applyConstraints(LinearSystem* system, bool isHorizontal,
                          bool parentWrapContent, bool oppositeParentWrapContent,
                          bool isTerminal, SolverVariable* parentMin, SolverVariable* parentMax,
                          DimensionBehaviour dimensionBehaviour, bool wrapContent,
                          ConstraintAnchor* beginAnchor, ConstraintAnchor* endAnchor,
                          int beginPosition, int dimension, int minDimension,
                          int maxDimension, float bias, bool useRatio,
                          bool oppositeVariable, bool inChain,
                          bool oppositeInChain, bool inBarrier,
                          int matchConstraintDefault, int oppositeMatchConstraintDefault,
                          int matchMinDimension, int matchMaxDimension,
                          float matchPercentDimension, bool applyPosition);
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_CONSTRAINT_WIDGET_H
