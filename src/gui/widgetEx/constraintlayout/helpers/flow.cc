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
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.Flow.
 */
#include <widgetEx/constraintlayout/helpers/flow.h>

#include <porting/cdlog.h>
#include <view/view.h>
#include <widget/internal_R.h>
#include <widgetEx/widgetex_styleable.h>

DECLARE_WIDGET2(Flow, "androidx.constraintlayout.helper.widget.Flow");

namespace cdroid {
using namespace cdroid::internal;

static clcore::Flow* asFlow(HelperWidget* hw) {
    return static_cast<clcore::Flow*>(hw);
}

Flow::Flow(Context* ctx):Flow(ctx,nullptr){}

Flow::Flow(Context* ctx,const AttributeSet* attrs):Flow(ctx,attrs,0){}

Flow::Flow(Context* ctx,const AttributeSet* pAttrs,int defStyleAttr)
    : VirtualLayout(ctx, pAttrs, defStyleAttr) {
    mHelperWidget = std::make_unique<clcore::Flow>();
    auto* f = asFlow(mHelperWidget.get());
    // TypedArray reads typed binary AXML values directly (AOSP pattern). AndroidX Flow reuses the
    // platform android:orientation/android:padding (framework attrs resolved by id) and its own
    // flow_* attrs; all live in the ConstraintLayout_Layout styleable. flow_wrapMode/align/style
    // and orientation are enums compiled by aapt2 to their int, so the string→int maps are gone.
    namespace F = R::styleable;
    auto ta = ctx ? ctx->obtainStyledAttributes(pAttrs, R::styleable::ConstraintLayoutLayout) : nullptr;
    if (ta) {
        f->setWrapMode           (ta->getInt(F::ConstraintLayoutLayout_flow_wrapMode, clcore::Flow::WRAP_NONE));
        int orient = ta->getInt(F::ConstraintLayoutLayout_orientation, ConstraintWidget::HORIZONTAL);
        f->setOrientation(orient == ConstraintWidget::VERTICAL ? ConstraintWidget::VERTICAL
                                                               : ConstraintWidget::HORIZONTAL);
        f->setHorizontalAlign    (ta->getInt(F::ConstraintLayoutLayout_flow_horizontalAlign, clcore::Flow::HORIZONTAL_ALIGN_START));
        f->setVerticalAlign      (ta->getInt(F::ConstraintLayoutLayout_flow_verticalAlign,   clcore::Flow::VERTICAL_ALIGN_CENTER));
        f->setHorizontalGap      (ta->getDimensionPixelSize(F::ConstraintLayoutLayout_flow_horizontalGap, 0));
        f->setVerticalGap        (ta->getDimensionPixelSize(F::ConstraintLayoutLayout_flow_verticalGap,   0));
        f->setHorizontalStyle    (ta->getInt(F::ConstraintLayoutLayout_flow_horizontalStyle, ConstraintWidget::UNKNOWN));
        f->setVerticalStyle      (ta->getInt(F::ConstraintLayoutLayout_flow_verticalStyle,   ConstraintWidget::UNKNOWN));
        f->setFirstHorizontalStyle(ta->getInt(F::ConstraintLayoutLayout_flow_firstHorizontalStyle, ConstraintWidget::UNKNOWN));
        f->setFirstVerticalStyle  (ta->getInt(F::ConstraintLayoutLayout_flow_firstVerticalStyle,   ConstraintWidget::UNKNOWN));
        f->setLastHorizontalStyle (ta->getInt(F::ConstraintLayoutLayout_flow_lastHorizontalStyle,  ConstraintWidget::UNKNOWN));
        f->setLastVerticalStyle   (ta->getInt(F::ConstraintLayoutLayout_flow_lastVerticalStyle,    ConstraintWidget::UNKNOWN));
        f->setHorizontalBias     (ta->getFloat(F::ConstraintLayoutLayout_flow_horizontalBias, 0.5f));
        f->setVerticalBias       (ta->getFloat(F::ConstraintLayoutLayout_flow_verticalBias,   0.5f));
        f->setFirstHorizontalBias(ta->getFloat(F::ConstraintLayoutLayout_flow_firstHorizontalBias, 0.5f));
        f->setFirstVerticalBias  (ta->getFloat(F::ConstraintLayoutLayout_flow_firstVerticalBias,   0.5f));
        f->setLastHorizontalBias (ta->getFloat(F::ConstraintLayoutLayout_flow_lastHorizontalBias,  0.5f));
        f->setLastVerticalBias   (ta->getFloat(F::ConstraintLayoutLayout_flow_lastVerticalBias,    0.5f));
        f->setPadding            (ta->getDimensionPixelSize(F::ConstraintLayoutLayout_padding, 0));
        f->setMaxElementsWrap    (ta->getInt(F::ConstraintLayoutLayout_flow_maxElementsWrap, ConstraintWidget::UNKNOWN));
    }
    validateParams();
}

// AndroidX Flow setters each end with requestLayout() (Flow.java:305-373+) — the data only
// takes effect on the next layout pass, so runtime changes must flag the hierarchy: the
// ConstraintLayout capture is dirty-gated and re-runs when a child (this helper) requests
// layout. Without this a runtime call silently no-ops until something else re-lays-out.
void Flow::setWrapMode(int wrapMode)       {
    asFlow(mHelperWidget.get())->setWrapMode(wrapMode);
    requestLayout();
}
void Flow::setMaxElementsWrap(int max)     {
    asFlow(mHelperWidget.get())->setMaxElementsWrap(max);
    requestLayout();
}
void Flow::setOrientation(int orientation) {
    asFlow(mHelperWidget.get())->setOrientation(orientation);
    requestLayout();
}
void Flow::setHorizontalAlign(int align)   {
    asFlow(mHelperWidget.get())->setHorizontalAlign(align);
    requestLayout();
}
void Flow::setVerticalAlign(int align)     {
    asFlow(mHelperWidget.get())->setVerticalAlign(align);
    requestLayout();
}
void Flow::setHorizontalGap(int gap)       {
    asFlow(mHelperWidget.get())->setHorizontalGap(gap);
    requestLayout();
}
void Flow::setVerticalGap(int gap)         {
    asFlow(mHelperWidget.get())->setVerticalGap(gap);
    requestLayout();
}
void Flow::setHorizontalStyle(int style)   {
    asFlow(mHelperWidget.get())->setHorizontalStyle(style);
    requestLayout();
}
void Flow::setVerticalStyle(int style)     {
    asFlow(mHelperWidget.get())->setVerticalStyle(style);
    requestLayout();
}
void Flow::setHorizontalBias(float bias)   {
    asFlow(mHelperWidget.get())->setHorizontalBias(bias);
    requestLayout();
}
void Flow::setVerticalBias(float bias)     {
    asFlow(mHelperWidget.get())->setVerticalBias(bias);
    requestLayout();
}


void Flow::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    // AndroidX Flow.onMeasure (helper Flow.java:166-171): delegate to the 3-arg overload.
    onMeasure(asFlow(mHelperWidget.get()), widthMeasureSpec, heightMeasureSpec);
}

void Flow::onMeasure(clcore::VirtualLayout* layout, int widthMeasureSpec, int heightMeasureSpec) {
    // AndroidX Flow.onMeasure (helper Flow.java:177-192): translate the specs and hand them
    // to the core Flow, then adopt its measured size.
    int widthMode  = View::MeasureSpec::getMode(widthMeasureSpec);
    int widthSize  = View::MeasureSpec::getSize(widthMeasureSpec);
    int heightMode = View::MeasureSpec::getMode(heightMeasureSpec);
    int heightSize = View::MeasureSpec::getSize(heightMeasureSpec);
    if (layout != nullptr) {
        layout->measure(widthMode, widthSize, heightMode, heightSize);
        setMeasuredDimension(layout->getMeasuredWidth(), layout->getMeasuredHeight());
    } else {
        setMeasuredDimension(0, 0);
    }
}
} // namespace cdroid
