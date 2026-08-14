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
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.Barrier.
 */
#include <widgetEx/constraintlayout/helpers/barrier.h>

#include <porting/cdlog.h>
#include <view/view.h>
#include <widget/internal_R.h>
#include <widgetEx/widgetex_styleable.h>

DECLARE_WIDGET(Barrier)

namespace cdroid {
using namespace cdroid::internal;

// out-of-line definitions (odr-used as runtime args / defaults)
constexpr int Barrier::LEFT;
constexpr int Barrier::TOP;
constexpr int Barrier::RIGHT;
constexpr int Barrier::BOTTOM;
constexpr int Barrier::START;
constexpr int Barrier::END;

Barrier::Barrier(Context* ctx,const AttributeSet& attrs):Barrier(ctx,&attrs,0){
}

Barrier::Barrier(Context* ctx,const AttributeSet* pAttrs,int defStyleAttr)
    : ConstraintHelper(ctx, pAttrs, defStyleAttr) {
    const AttributeSet& attrs = *pAttrs;
    setVisibility(View::GONE);
    mHelperWidget = std::make_unique<clcore::Barrier>();

    // TypedArray reads typed binary AXML values directly (AOSP pattern). barrierDirection is an
    // enum compiled by aapt2 to its int (left/right/top/bottom/start/end == the Barrier::* enum),
    // so no name-based string→int map is needed.
    auto ta = ctx ? ctx->obtainStyledAttributes(attrs, R::styleable::ConstraintLayoutLayout) : nullptr;
    int dir = LEFT;
    bool allowsGone = true;
    int margin = 0;
    if (ta) {
        dir = ta->getInt(R::styleable::ConstraintLayoutLayout_barrierDirection, LEFT);
        allowsGone = ta->getBoolean(R::styleable::ConstraintLayoutLayout_barrierAllowsGoneWidgets, true);
        margin = ta->getDimensionPixelSize(R::styleable::ConstraintLayoutLayout_barrierMargin, 0);
    }
    setType(dir);
    static_cast<clcore::Barrier*>(mHelperWidget.get())->setAllowsGoneWidget(allowsGone);
    static_cast<clcore::Barrier*>(mHelperWidget.get())->setMargin(margin);

    // Default to LTR here (START->LEFT, END->RIGHT); the bridge re-resolves via resolveRtl() with
    // the container's real direction at measure time (START->RIGHT/END->LEFT under RTL).
    updateType(mHelperWidget.get(), mIndicatedType, /*isRtl=*/false);
    validateParams();
}

Barrier::Barrier(int width, int height)
    : ConstraintHelper(width, height) {
    setVisibility(View::GONE);
    mHelperWidget = std::make_unique<clcore::Barrier>();
    setType(LEFT);
    updateType(mHelperWidget.get(), mIndicatedType, /*isRtl=*/false);
    validateParams();
}

int Barrier::getType() const {
    return mIndicatedType;
}

void Barrier::setType(int type) {
    mIndicatedType = type;
    updateType(mHelperWidget.get(), mIndicatedType, /*isRtl=*/false);
}

void Barrier::updateType(ConstraintWidget* widget, int type, bool isRtl) {
    mResolvedType = type;
    // RTL resolution: START/END are direction-relative. LTR: START→LEFT, END→RIGHT; under RTL they
    // swap (faithful to AndroidX Barrier, gated on the container's resolved layout direction).
    if (mIndicatedType == START) {
        mResolvedType = isRtl ? RIGHT : LEFT;
    } else if (mIndicatedType == END) {
        mResolvedType = isRtl ? LEFT : RIGHT;
    }
    if (auto* barrier = dynamic_cast<clcore::Barrier*>(widget)) {
        barrier->setBarrierType(mResolvedType);
    }
}

void Barrier::resolveRtl(ConstraintWidget* widget, bool isRtl) {
    updateType(widget, mIndicatedType, isRtl);
}

bool Barrier::getAllowsGoneWidget() const {
    return static_cast<clcore::Barrier*>(mHelperWidget.get())->getAllowsGoneWidget();
}

void Barrier::setAllowsGoneWidget(bool supportGone) {
    static_cast<clcore::Barrier*>(mHelperWidget.get())->setAllowsGoneWidget(supportGone);
}

int Barrier::getMargin() const {
    return static_cast<clcore::Barrier*>(mHelperWidget.get())->getMargin();
}

void Barrier::setMargin(int margin) {
    static_cast<clcore::Barrier*>(mHelperWidget.get())->setMargin(margin);
}

void Barrier::setDpMargin(int margin) {
    // TODO: multiply by display density. CDROID embedded targets are often px-oriented; treat as px.
    setMargin(margin);
}

} // namespace cdroid
