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
    {"spread", (int)ConstraintWidget::CHAIN_SPREAD},
    {"spread_inside", (int)ConstraintWidget::CHAIN_SPREAD_INSIDE},
    {"packed", (int)ConstraintWidget::CHAIN_PACKED}
};
// android:orientation is a platform enum (horizontal/vertical), so it needs a
// string→int map — getInt(key, def) alone can't parse "vertical" (alpha-prefixed
// values fall back to def). Mirrors LinearLayout/GridLayout/RadioGroup.
const std::unordered_map<std::string, int> kOrientation = {
    {"horizontal", (int)ConstraintWidget::HORIZONTAL},
    {"vertical", (int)ConstraintWidget::VERTICAL}
};
} // namespace

static clcore::Flow* asFlow(HelperWidget* hw) {
    return static_cast<clcore::Flow*>(hw);
}

Flow::Flow(Context* ctx, const AttributeSet& attrs)
    : ConstraintHelper(ctx, attrs) {
    mHelperWidget = std::make_unique<clcore::Flow>();
    auto* f = asFlow(mHelperWidget.get());
    // AndroidX Flow styleable (ConstraintLayout_flow) reuses the platform
    // android:orientation/android:padding and prefixes its own attrs with "flow_".
    // CDROID's AttributeSet (expat namespace-aware, ' ' separator) stores every
    // namespaced attr by its bare local name, so the lookup keys are the local
    // names: "orientation"/"padding" and "flow_*". (A bare "android:..." key would
    // never match — LinearLayout/GridLayout read "orientation" the same way.)
    f->setWrapMode(attrs.getInt("flow_wrapMode", kWrapMode, clcore::Flow::WRAP_NONE));
    int orient = attrs.getInt("orientation", kOrientation, ConstraintWidget::HORIZONTAL);
    f->setOrientation(orient == ConstraintWidget::VERTICAL ? ConstraintWidget::VERTICAL
                      : ConstraintWidget::HORIZONTAL);
    f->setHorizontalAlign(attrs.getInt("flow_horizontalAlign", kHorizontalAlign, clcore::Flow::HORIZONTAL_ALIGN_START));
    f->setVerticalAlign(attrs.getInt("flow_verticalAlign", kVerticalAlign, clcore::Flow::VERTICAL_ALIGN_CENTER));
    f->setHorizontalGap(attrs.getDimensionPixelSize("flow_horizontalGap", 0));
    f->setVerticalGap(attrs.getDimensionPixelSize("flow_verticalGap", 0));
    f->setHorizontalStyle(attrs.getInt("flow_horizontalStyle", kChainStyle, ConstraintWidget::UNKNOWN));
    f->setVerticalStyle(attrs.getInt("flow_verticalStyle", kChainStyle, ConstraintWidget::UNKNOWN));
    f->setFirstHorizontalStyle(attrs.getInt("flow_firstHorizontalStyle", kChainStyle, ConstraintWidget::UNKNOWN));
    f->setFirstVerticalStyle(attrs.getInt("flow_firstVerticalStyle", kChainStyle, ConstraintWidget::UNKNOWN));
    f->setLastHorizontalStyle(attrs.getInt("flow_lastHorizontalStyle", kChainStyle, ConstraintWidget::UNKNOWN));
    f->setLastVerticalStyle(attrs.getInt("flow_lastVerticalStyle", kChainStyle, ConstraintWidget::UNKNOWN));
    f->setHorizontalBias(attrs.getFloat("flow_horizontalBias", 0.5f));
    f->setVerticalBias(attrs.getFloat("flow_verticalBias", 0.5f));
    f->setFirstHorizontalBias(attrs.getFloat("flow_firstHorizontalBias", 0.5f));
    f->setFirstVerticalBias(attrs.getFloat("flow_firstVerticalBias", 0.5f));
    f->setLastHorizontalBias(attrs.getFloat("flow_lastHorizontalBias", 0.5f));
    f->setLastVerticalBias(attrs.getFloat("flow_lastVerticalBias", 0.5f));
    f->setPadding(attrs.getDimensionPixelSize("padding", 0));
    f->setMaxElementsWrap(attrs.getInt("flow_maxElementsWrap", ConstraintWidget::UNKNOWN));
    validateParams();
}

Flow::Flow(int width, int height)
    : ConstraintHelper(width, height) {
    mHelperWidget = std::make_unique<clcore::Flow>();
    validateParams();
}

void Flow::setWrapMode(int wrapMode)       {
    asFlow(mHelperWidget.get())->setWrapMode(wrapMode);
}
void Flow::setMaxElementsWrap(int max)     {
    asFlow(mHelperWidget.get())->setMaxElementsWrap(max);
}
void Flow::setOrientation(int orientation) {
    asFlow(mHelperWidget.get())->setOrientation(orientation);
}
void Flow::setHorizontalAlign(int align)   {
    asFlow(mHelperWidget.get())->setHorizontalAlign(align);
}
void Flow::setVerticalAlign(int align)     {
    asFlow(mHelperWidget.get())->setVerticalAlign(align);
}
void Flow::setHorizontalGap(int gap)       {
    asFlow(mHelperWidget.get())->setHorizontalGap(gap);
}
void Flow::setVerticalGap(int gap)         {
    asFlow(mHelperWidget.get())->setVerticalGap(gap);
}
void Flow::setHorizontalStyle(int style)   {
    asFlow(mHelperWidget.get())->setHorizontalStyle(style);
}
void Flow::setVerticalStyle(int style)     {
    asFlow(mHelperWidget.get())->setVerticalStyle(style);
}
void Flow::setHorizontalBias(float bias)   {
    asFlow(mHelperWidget.get())->setHorizontalBias(bias);
}
void Flow::setVerticalBias(float bias)     {
    asFlow(mHelperWidget.get())->setVerticalBias(bias);
}

} // namespace cdroid
