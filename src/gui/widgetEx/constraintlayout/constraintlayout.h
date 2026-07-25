/*
 * Copyright (C) 2016 The Android Open Source Project
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
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.ConstraintLayout.
 *
 * MVP cut: fixed/wrap_content children with left/right/top/bottom anchor constraints + bias,
 * resolved via the core linear solver. Deferred: match_constraint (0dp) / ratio / chains /
 * baseline / guideline / helpers (Barrier/Group/Placeholder/VirtualLayout) / RTL / ConstraintSet /
 * optimizer fast-paths.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_CONSTRAINT_LAYOUT_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_CONSTRAINT_LAYOUT_H

#include <climits>
#include <unordered_map>

#include <core/attributeset.h>
#include <widgetEx/constraintlayout/core/widgets/analyzer/basicmeasure.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.h>
#include <view/layoutparams.h>
#include <view/viewgroup.h>

namespace cdroid {

class ConstraintLayout : public ViewGroup, private BasicMeasure::Measurer {
public:
    class LayoutParams : public ViewGroup::MarginLayoutParams {
    public:
        static constexpr int UNSET      = -1;
        static constexpr int GONE_UNSET = INT_MIN;

        int leftToLeft = UNSET, leftToRight = UNSET;
        int rightToLeft = UNSET, rightToRight = UNSET;
        int topToTop = UNSET, topToBottom = UNSET;
        int bottomToTop = UNSET, bottomToBottom = UNSET;

        float horizontalBias = 0.5f;
        float verticalBias   = 0.5f;

        int goneLeftMargin = GONE_UNSET, goneTopMargin = GONE_UNSET;
        int goneRightMargin = GONE_UNSET, goneBottomMargin = GONE_UNSET;

        bool mHorizontalDimensionFixed = true;
        bool mVerticalDimensionFixed = true;

        ConstraintWidget mWidget; // per-child solver model (anchors own this widget)

        LayoutParams(Context* c, const AttributeSet& attrs);
        LayoutParams(int width, int height);
        void validate();
    };

    ConstraintLayout(Context* ctx, const AttributeSet& attrs);
    ConstraintLayout(int width, int height);
    static constexpr int PARENT_ID = 0;

    // The Measurer::measure(Widget*,Measure*) override would hide View::measure(int,int);
    // bring View::measure back into scope so external callers can measure this view normally.
    using View::measure;

    // Measurer is privately inherited (its measure(Widget*,Measure*) would clash with
    // View::measure(int,int)); this exposes the Measurer subobject for the solver bridge.
    BasicMeasure::Measurer* asMeasurer() { return this; }

protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
    void onLayout(bool changed, int l, int t, int r, int b) override;
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
    int mMinWidth = 0;
    int mMaxWidth = INT_MAX;
    int mMinHeight = 0;
    int mMaxHeight = INT_MAX;

    // captured during resolveSystem for the Measurer
    int mWidthSpec = 0;
    int mHeightSpec = 0;
    int mPaddingWidth = 0;
    int mPaddingHeight = 0;

    ConstraintWidget* getViewWidget(View* view);
    void setChildrenConstraints();
    void applyConstraintsFromLayoutParams(View* child, ConstraintWidget* widget, LayoutParams* lp);
    void resolveSystem(int widthSpec, int heightSpec);
    void setSelfDimensionBehaviour(int widthMode, int widthSize, int heightMode, int heightSize);
    void resolveMeasuredDimension(int widthSpec, int heightSpec, int measuredWidth, int measuredHeight);
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_CONSTRAINT_LAYOUT_H
