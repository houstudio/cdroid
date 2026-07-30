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
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.ConstraintLayout.
 *
 * Faithful port: fixed/wrap_content + match_constraint (0dp) children with left/right/start/end/
 * top/bottom/baseline anchor constraints, bias, chains (spread/spread_inside/packed + weights),
 * ratio, guidelines, helpers (Barrier/Group/Placeholder/Flow/Layer/CircularFlow/MotionEffect/Grid),
 * RTL (start/end + bias mirror + guideline/barrier), and ConstraintSet — all resolved via the core
 * linear solver. The only deferred piece is the OPTIMIZATION_GRAPH analyzer (Direct solver /
 * DependencyGraph) — a performance layer, off by default in AndroidX.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_CONSTRAINT_LAYOUT_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_CONSTRAINT_LAYOUT_H

#include <climits>
#include <memory>
#include <unordered_map>

#include <core/attributeset.h>
#include <widgetEx/constraintlayout/core/widgets/analyzer/basicmeasure.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.h>
#include <view/layoutparams.h>
#include <view/viewgroup.h>

namespace cdroid {

class ConstraintHelper;
class ConstraintLayoutStates;
class SharedValues;

class ConstraintLayout : public ViewGroup, private BasicMeasure::Measurer {
  public:
    class LayoutParams : public ViewGroup::MarginLayoutParams {
      public:
        static constexpr int UNSET      = -1;
        static constexpr int GONE_UNSET = INT_MIN;

        int leftToLeft = UNSET, leftToRight = UNSET;
        int rightToLeft = UNSET, rightToRight = UNSET;
        // RTL-aware Start/End anchors (resolved to Left/Right at measure time via isRtl()).
        int startToStart = UNSET, startToEnd = UNSET;
        int endToStart = UNSET, endToEnd = UNSET;
        int topToTop = UNSET, topToBottom = UNSET;
        int bottomToTop = UNSET, bottomToBottom = UNSET;

        float horizontalBias = 0.5f;
        float verticalBias   = 0.5f;

        std::string constraintTag; // matches a ViewTransition's string motionTarget (regex)

        int goneLeftMargin = GONE_UNSET, goneTopMargin = GONE_UNSET;
        int goneRightMargin = GONE_UNSET, goneBottomMargin = GONE_UNSET;
        // RTL-aware gone margins (resolved to goneLeft/goneRight at measure time via layout direction).
        int goneStartMargin = GONE_UNSET, goneEndMargin = GONE_UNSET;

        bool mHorizontalDimensionFixed = true;
        bool mVerticalDimensionFixed = true;

        // Guideline support (when guideBegin/End/Percent is set, mWidget becomes a Guideline)
        bool mIsGuideline = false;
        // Set for ConstraintHelper children (Barrier/Group/...) — flagged in onViewAdded.
        bool mIsHelper = false;
        // Set on a view that has been pulled into a Placeholder (treated as GONE at its origin).
        bool mIsInPlaceholder = false;
        int guideBegin = UNSET, guideEnd = UNSET;
        float guidePercent = UNSET_FLOAT;
        int orientation = -1;
        // Whether a vertical Guideline mirrors begin/end + percent under RTL (AndroidX default true).
        bool guidelineUseRtl = true;

        // ratio ("layout_constraintDimensionRatio")
        float dimensionRatio = 0;
        int dimensionRatioSide = -1;  // -1=UNKNOWN, 0=HORIZONTAL, 1=VERTICAL

        // baseline ("layout_constraintBaseline_toBaselineOf")
        int baselineToBaseline = UNSET;

        // Circular constraint ("layout_constraintCircle[_angle|_radius]") — positions this view on a
        // circle around the target. Wired to ConstraintWidget::connectCircularConstraint by the bridge.
        int circleConstraint = UNSET;
        float circleAngle = 0;
        int circleRadius = 0;

        // chain styles (layout_constraintHorizontal/Vertical_chainStyle: spread/spread_inside/packed)
        int horizontalChainStyle = ConstraintWidget::CHAIN_SPREAD;
        int verticalChainStyle = ConstraintWidget::CHAIN_SPREAD;
        // chain weights (layout_constraintHorizontal/Vertical_weight) — distribute free space among
        // 0dp (match_constraint) chain elements proportionally. UNKNOWN = unweighted.
        float horizontalWeight = ConstraintWidget::UNKNOWN;
        float verticalWeight = ConstraintWidget::UNKNOWN;

        // match_constraint (0dp) sizing: layout_constraintWidth_default (spread=0/wrap=1/percent=2)
        // + percent + min/max. Wired into the widget's mMatchConstraint* fields.
        int matchConstraintDefaultWidth  = ConstraintWidget::MATCH_CONSTRAINT_SPREAD;
        int matchConstraintDefaultHeight = ConstraintWidget::MATCH_CONSTRAINT_SPREAD;
        float matchConstraintPercentWidth  = 1.0f;
        float matchConstraintPercentHeight = 1.0f;
        int matchConstraintMinWidth   = 0;
        int matchConstraintMaxWidth   = 0;
        int matchConstraintMinHeight  = 0;
        int matchConstraintMaxHeight  = 0;

        // The per-child solver model (owned; a Guideline for guideline children). Pointer so the
        // concrete type can be swapped to clcore::Guideline in validate().
        std::unique_ptr<ConstraintWidget> mWidget = std::make_unique<ConstraintWidget>();

        LayoutParams(Context* c, const AttributeSet& attrs);
        LayoutParams(int width, int height);
        void validate();

        static constexpr float UNSET_FLOAT = -1.0f;
    };

    ConstraintLayout(Context* ctx, const AttributeSet& attrs);
    ConstraintLayout(int width, int height);
    ~ConstraintLayout() override;
    static constexpr int PARENT_ID = 0;

    // The Measurer::measure(Widget*,Measure*) override would hide View::measure(int,int);
    // bring View::measure back into scope so external callers can measure this view normally.
    using View::measure;

    // Measurer is privately inherited (its measure(Widget*,Measure*) would clash with
    // View::measure(int,int)); this exposes the Measurer subobject for the solver bridge.
    BasicMeasure::Measurer* asMeasurer() {
        return this;
    }

    // Resolve a child View to its solver model widget. Helper children (Barrier, ...) return their
    // owned core helper widget; Guideline children return their Guideline; others their LayoutParams
    // widget. Exposed so ConstraintHelper.updatePreLayout can map referenced views to widgets.
    ConstraintWidget* getViewWidget(View* view);
    // The container's own solver model (the root ConstraintWidgetContainer).
    ConstraintWidgetContainer& getLayoutWidget() {
        return mLayoutWidget;
    }

    // Adaptive layouts (<StateSet>): load a state-set resource so setState can swap ConstraintSets
    // when the layout's size or a logical state changes. (MotionLayout overrides layoutDescription
    // for <MotionScene> and does not use this.)
    void loadLayoutDescription(const std::string& resource);
    // Apply the ConstraintSet selected by (id, screenWidth, screenHeight) from the loaded StateSet.
    void setState(int id, int screenWidth, int screenHeight);

    // Process-wide registry of shared integer values (for ViewTransition sharedValue triggers).
    static SharedValues& getSharedValues();

  protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
    void onLayout(bool changed, int l, int t, int w, int h) override;
    void onViewAdded(View* child) override;
    void onViewRemoved(View* child) override;
    ViewGroup::LayoutParams* generateLayoutParams(const AttributeSet& attrs) const override;
    ViewGroup::LayoutParams* generateDefaultLayoutParams() const override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p) const override;

  public:
    // --- BasicMeasure::Measurer (bridge to View measurement) ---
    void measure(ConstraintWidget* widget, BasicMeasure::Measure* measure) override;
    void didMeasures() override;

  private:
    ConstraintWidgetContainer mLayoutWidget;
    std::unordered_map<int, ConstraintWidget*> mIdToWidget; // id -> widget (PARENT_ID/own id -> mLayoutWidget)
    std::vector<ConstraintHelper*> mConstraintHelpers; // Barrier/Group/... children
    std::unique_ptr<ConstraintLayoutStates> mConstraintLayoutStates; // <StateSet> adaptive layout
    int mMinWidth = 0;
    int mMaxWidth = INT_MAX;
    int mMinHeight = 0;
    int mMaxHeight = INT_MAX;

    // captured during resolveSystem for the Measurer
    int mWidthSpec = 0;
    int mHeightSpec = 0;
    int mPaddingWidth = 0;
    int mPaddingHeight = 0;

    void setChildrenConstraints();
    void applyConstraintsFromLayoutParams(View* child, ConstraintWidget* widget, LayoutParams* lp);
    void resolveSystem(int widthSpec, int heightSpec);
    void setSelfDimensionBehaviour(int widthMode, int widthSize, int heightMode, int heightSize);
    void resolveMeasuredDimension(int widthSpec, int heightSpec, int measuredWidth, int measuredHeight);
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_CONSTRAINT_LAYOUT_H
