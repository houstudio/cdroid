/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.Flow.
 */
#include <widgetEx/constraintlayout/flow.h>

#include <porting/cdlog.h>
#include <view/view.h>

DECLARE_WIDGET(Flow)

namespace cdroid {

namespace {
const std::unordered_map<std::string, int> kWrapMode = {
    {"none", clcore::Flow::WRAP_NONE}, {"chain", clcore::Flow::WRAP_CHAIN},
    {"aligned", clcore::Flow::WRAP_ALIGNED}, {"chain_new", clcore::Flow::WRAP_CHAIN_NEW}
};
const std::unordered_map<std::string, int> kHorizontalAlign = {
    {"start", clcore::Flow::HORIZONTAL_ALIGN_START},
    {"end",   clcore::Flow::HORIZONTAL_ALIGN_END},
    {"center", clcore::Flow::HORIZONTAL_ALIGN_CENTER}
};
const std::unordered_map<std::string, int> kVerticalAlign = {
    {"top", clcore::Flow::VERTICAL_ALIGN_TOP},
    {"bottom", clcore::Flow::VERTICAL_ALIGN_BOTTOM},
    {"center", clcore::Flow::VERTICAL_ALIGN_CENTER},
    {"baseline", clcore::Flow::VERTICAL_ALIGN_BASELINE}
};
const std::unordered_map<std::string, int> kChainStyle = {
    {"spread", ConstraintWidget::CHAIN_SPREAD},
    {"spread_inside", ConstraintWidget::CHAIN_SPREAD_INSIDE},
    {"packed", ConstraintWidget::CHAIN_PACKED}
};
} // namespace

static clcore::Flow* asFlow(HelperWidget* hw) {
    return static_cast<clcore::Flow*>(hw);
}

Flow::Flow(Context* ctx, const AttributeSet& attrs)
    : ConstraintHelper(ctx, attrs) {
    mHelperWidget = std::make_unique<clcore::Flow>();
    auto* f = asFlow(mHelperWidget.get());
    f->setWrapMode(attrs.getInt("wrapMode", kWrapMode, clcore::Flow::WRAP_NONE));
    int orient = attrs.getInt("android:orientation", ConstraintWidget::HORIZONTAL);
    f->setOrientation(orient == ConstraintWidget::VERTICAL ? ConstraintWidget::VERTICAL
                                                           : ConstraintWidget::HORIZONTAL);
    f->setHorizontalAlign(attrs.getInt("horizontalAlign", kHorizontalAlign, clcore::Flow::HORIZONTAL_ALIGN_START));
    f->setVerticalAlign(attrs.getInt("verticalAlign", kVerticalAlign, clcore::Flow::VERTICAL_ALIGN_CENTER));
    f->setHorizontalGap(attrs.getDimensionPixelSize("horizontalGap", 0));
    f->setVerticalGap(attrs.getDimensionPixelSize("verticalGap", 0));
    f->setHorizontalStyle(attrs.getInt("horizontalStyle", kChainStyle, ConstraintWidget::UNKNOWN));
    f->setVerticalStyle(attrs.getInt("verticalStyle", kChainStyle, ConstraintWidget::UNKNOWN));
    f->setFirstHorizontalStyle(attrs.getInt("firstHorizontalStyle", kChainStyle, ConstraintWidget::UNKNOWN));
    f->setFirstVerticalStyle(attrs.getInt("firstVerticalStyle", kChainStyle, ConstraintWidget::UNKNOWN));
    f->setLastHorizontalStyle(attrs.getInt("lastHorizontalStyle", kChainStyle, ConstraintWidget::UNKNOWN));
    f->setLastVerticalStyle(attrs.getInt("lastVerticalStyle", kChainStyle, ConstraintWidget::UNKNOWN));
    f->setHorizontalBias(attrs.getFloat("horizontalBias", 0.5f));
    f->setVerticalBias(attrs.getFloat("verticalBias", 0.5f));
    f->setFirstHorizontalBias(attrs.getFloat("firstHorizontalBias", 0.5f));
    f->setFirstVerticalBias(attrs.getFloat("firstVerticalBias", 0.5f));
    f->setLastHorizontalBias(attrs.getFloat("lastHorizontalBias", 0.5f));
    f->setLastVerticalBias(attrs.getFloat("lastVerticalBias", 0.5f));
    f->setPadding(attrs.getDimensionPixelSize("padding", 0));
    f->setMaxElementsWrap(attrs.getInt("maxElementsWrap", ConstraintWidget::UNKNOWN));
    validateParams();
}

Flow::Flow(int width, int height)
    : ConstraintHelper(width, height) {
    mHelperWidget = std::make_unique<clcore::Flow>();
    validateParams();
}

void Flow::setWrapMode(int wrapMode)       { asFlow(mHelperWidget.get())->setWrapMode(wrapMode); }
void Flow::setOrientation(int orientation) { asFlow(mHelperWidget.get())->setOrientation(orientation); }
void Flow::setHorizontalAlign(int align)   { asFlow(mHelperWidget.get())->setHorizontalAlign(align); }
void Flow::setVerticalAlign(int align)     { asFlow(mHelperWidget.get())->setVerticalAlign(align); }
void Flow::setHorizontalGap(int gap)       { asFlow(mHelperWidget.get())->setHorizontalGap(gap); }
void Flow::setVerticalGap(int gap)         { asFlow(mHelperWidget.get())->setVerticalGap(gap); }
void Flow::setHorizontalStyle(int style)   { asFlow(mHelperWidget.get())->setHorizontalStyle(style); }
void Flow::setVerticalStyle(int style)     { asFlow(mHelperWidget.get())->setVerticalStyle(style); }
void Flow::setHorizontalBias(float bias)   { asFlow(mHelperWidget.get())->setHorizontalBias(bias); }
void Flow::setVerticalBias(float bias)     { asFlow(mHelperWidget.get())->setVerticalBias(bias); }

} // namespace cdroid
