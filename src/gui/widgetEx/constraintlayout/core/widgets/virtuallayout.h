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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.VirtualLayout.
 *
 * Base for virtual layouts (Flow, Layer): a HelperWidget that owns padding + a measure hook and
 * can measure its referenced children through the container's Measurer. Concrete layouts (Flow)
 * override measure() to compute their wrapped size, then generate per-child constraints in
 * addToSolver().
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_VIRTUAL_LAYOUT_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_VIRTUAL_LAYOUT_H

#include <widgetEx/constraintlayout/core/widgets/analyzer/basicmeasure.h>
#include <widgetEx/constraintlayout/core/widgets/helperwidget.h>

namespace cdroid {

class VirtualLayout : public HelperWidget {
  public:
    VirtualLayout();
    ~VirtualLayout() override;

    bool isVirtualLayout() const override {
        return true;
    }

    // --- padding ---
    void setPadding(int value);
    void setPaddingStart(int value);
    void setPaddingEnd(int value);
    void setPaddingLeft(int value);
    void setPaddingRight(int value);
    void setPaddingTop(int value);
    void setPaddingBottom(int value);
    void applyRtl(bool isRtl);
    int  getPaddingTop() const;
    int  getPaddingBottom() const;
    int  getPaddingLeft() const;
    int  getPaddingRight() const;

    // --- solver callback ---
    void needsCallbackFromSolver(bool value);
    bool needSolverPass() const;

    // --- measure (overridden by Flow) ---
    virtual void measure(int widthMode, int widthSize, int heightMode, int heightSize);
    void updateConstraints(ConstraintWidgetContainer* container) override; // captureWidgets

    int  getMeasuredWidth() const;
    int  getMeasuredHeight() const;
    void setMeasure(int width, int height);

    std::string getType() const override; // "VirtualLayout"

  protected:
    // Measure every referenced child via the container's Measurer (skip Guideline; treat
    // match_constraint as wrap unless it is fully solver-determined). Returns false if no measurer.
    bool measureChildren();
    // Measure a single widget with the given behaviours/dimensions, writing back width/height/baseline.
    void measure(ConstraintWidget* widget,
                 ConstraintWidget::DimensionBehaviour horizontalBehavior, int horizontalDimension,
                 ConstraintWidget::DimensionBehaviour verticalBehavior, int verticalDimension);
    // Mark every referenced widget as participating in a virtual layout.
    void captureWidgets();

    BasicMeasure::Measure mMeasure;

  private:
    int mPaddingTop = 0;
    int mPaddingBottom = 0;
    int mPaddingLeft = 0;
    int mPaddingRight = 0;
    int mPaddingStart = 0;
    int mPaddingEnd = 0;
    int mResolvedPaddingLeft = 0;
    int mResolvedPaddingRight = 0;

    bool mNeedsCallFromSolver = false;
    int  mMeasuredWidth = 0;
    int  mMeasuredHeight = 0;

    BasicMeasure::Measurer* mMeasurer = nullptr;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_VIRTUAL_LAYOUT_H
