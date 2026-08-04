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
 * Faithful port — see header for the supported feature surface.
 */
#include <widgetEx/constraintlayout/constraintlayout.h>
#include <core/xmlpullparser.h>
#include <widgetEx/constraintlayout/constraintlayoutstates.h>
#include <widgetEx/constraintlayout/sharedvalues.h>

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdio>

#include <widgetEx/constraintlayout/core/widgets/guideline.h>
#include <widgetEx/constraintlayout/core/widgets/barrier.h>

#include <porting/cdlog.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <widget/textview.h>
#include <widgetEx/constraintlayout/helpers/constrainthelper.h>
#include <widgetEx/constraintlayout/helpers/circularflow.h>
#include <widgetEx/constraintlayout/helpers/flow.h>
#include <widgetEx/constraintlayout/helpers/grid.h>
#include <widgetEx/constraintlayout/helpers/layer.h>
#include <widgetEx/constraintlayout/helpers/placeholder.h>

DECLARE_WIDGET(ConstraintLayout)

namespace cdroid {

// out-of-line definition (PARENT_ID is odr-used as a map key)
constexpr int ConstraintLayout::PARENT_ID;

namespace {
// Parse a ratio string like "16:9", "1.5", "W,16:9", "H,3:2" into (ratio, side).
void parseDimensionRatio(const std::string& str, float& ratio, int& side) {
    side = -1; // UNKNOWN
    std::string s = str;
    if (s.size() > 2 && s[1] == ',') {
        if (s[0] == 'W' || s[0] == 'w') side = ConstraintWidget::HORIZONTAL;
        else if (s[0] == 'H' || s[0] == 'h') side = ConstraintWidget::VERTICAL;
        s = s.substr(2);
    }
    size_t colon = s.find(':');
    if (colon != std::string::npos) {
        float num = std::stof(s.substr(0, colon));
        float den = std::stof(s.substr(colon + 1));
        ratio = (den != 0) ? num / den : 0;
    } else {
        ratio = std::stof(s);
    }
}
} // anonymous namespace

// ===========================================================================
// ConstraintLayout::LayoutParams
// ===========================================================================
ConstraintLayout::LayoutParams::LayoutParams(Context* c, const AttributeSet& attrs)
    : MarginLayoutParams(c, attrs) {
    // Anchor targets — accept either a resource id ("parent" -> PARENT_ID=0) or an int.
    leftToLeft   = attrs.getResourceId("layout_constraintLeft_toLeftOf",   UNSET);
    leftToRight  = attrs.getResourceId("layout_constraintLeft_toRightOf",  UNSET);
    rightToLeft  = attrs.getResourceId("layout_constraintRight_toLeftOf",  UNSET);
    rightToRight = attrs.getResourceId("layout_constraintRight_toRightOf", UNSET);
    // Start/End (RTL-aware) — stored raw; resolved to Left/Right at measure time in
    // applyConstraintsFromLayoutParams based on the container's isRtl(). Explicit Left/Right
    // overrides these. Modern layouts emit Start/End, so without reading them a 0dp view
    // constrained only via Start/End gets no horizontal anchor → collapses to 0 width.
    startToStart = attrs.getResourceId("layout_constraintStart_toStartOf", UNSET);
    startToEnd   = attrs.getResourceId("layout_constraintStart_toEndOf",   UNSET);
    endToStart   = attrs.getResourceId("layout_constraintEnd_toStartOf",   UNSET);
    endToEnd     = attrs.getResourceId("layout_constraintEnd_toEndOf",     UNSET);
    topToTop     = attrs.getResourceId("layout_constraintTop_toTopOf",     UNSET);
    topToBottom  = attrs.getResourceId("layout_constraintTop_toBottomOf",  UNSET);
    bottomToTop  = attrs.getResourceId("layout_constraintBottom_toTopOf",  UNSET);
    bottomToBottom = attrs.getResourceId("layout_constraintBottom_toBottomOf", UNSET);

    horizontalBias = attrs.getFloat("layout_constraintHorizontal_bias", 0.5f);
    verticalBias   = attrs.getFloat("layout_constraintVertical_bias",   0.5f);

    constraintTag = attrs.getString("constraintTag", "");

    goneLeftMargin   = attrs.getDimensionPixelSize("layout_goneMarginLeft",   GONE_UNSET);
    goneTopMargin    = attrs.getDimensionPixelSize("layout_goneMarginTop",    GONE_UNSET);
    goneRightMargin  = attrs.getDimensionPixelSize("layout_goneMarginRight",  GONE_UNSET);
    goneBottomMargin = attrs.getDimensionPixelSize("layout_goneMarginBottom", GONE_UNSET);
    // RTL-aware gone margins (resolved to goneLeft/goneRight at measure time per layout direction).
    goneStartMargin  = attrs.getDimensionPixelSize("layout_goneMarginStart", GONE_UNSET);
    goneEndMargin    = attrs.getDimensionPixelSize("layout_goneMarginEnd",   GONE_UNSET);

    // Guideline
    guideBegin   = attrs.getDimensionPixelSize("layout_constraintGuide_begin", UNSET);
    guideEnd     = attrs.getDimensionPixelSize("layout_constraintGuide_end",   UNSET);
    guidePercent = attrs.getFloat("layout_constraintGuide_percent", UNSET_FLOAT);
    guidelineUseRtl = attrs.getBoolean("layout_guidelineUseRtl", true);
    // Orientation: bare key (namespace stripped) + map so "vertical"/"horizontal"
    // resolve (plain getInt treats a leading letter as non-numeric → def).
    // Mirrors LinearLayout; -1 (absent) falls back to HORIZONTAL in validate().
    orientation  = attrs.getInt("orientation", std::unordered_map<std::string,int>{
        {"horizontal", (int)ConstraintWidget::HORIZONTAL},
        {"vertical",   (int)ConstraintWidget::VERTICAL}}, -1);

    // Ratio (parse "16:9", "1.5", "W,16:9", "H,3:2")
    std::string ratioStr = attrs.getString("layout_constraintDimensionRatio", "");
    if (!ratioStr.empty()) {
        parseDimensionRatio(ratioStr, dimensionRatio, dimensionRatioSide);
    }

    // Baseline
    baselineToBaseline = attrs.getResourceId("layout_constraintBaseline_toBaselineOf", UNSET);

    // Circular constraint
    circleConstraint = attrs.getResourceId("layout_constraintCircle", UNSET);
    circleAngle      = attrs.getFloat("layout_constraintCircleAngle", 0);
    circleRadius     = attrs.getDimensionPixelSize("layout_constraintCircleRadius", 0);

    // Chain styles
    static const std::unordered_map<std::string,int> chainStyles = {
        {"spread", ConstraintWidget::CHAIN_SPREAD},
        {"spread_inside", ConstraintWidget::CHAIN_SPREAD_INSIDE},
        {"packed", ConstraintWidget::CHAIN_PACKED}
    };
    horizontalChainStyle = attrs.getInt("layout_constraintHorizontal_chainStyle", chainStyles, ConstraintWidget::CHAIN_SPREAD);
    verticalChainStyle   = attrs.getInt("layout_constraintVertical_chainStyle", chainStyles, ConstraintWidget::CHAIN_SPREAD);

    // chain weights (layout_constraintHorizontal/Vertical_weight).
    horizontalWeight = attrs.getFloat("layout_constraintHorizontal_weight", ConstraintWidget::UNKNOWN);
    verticalWeight   = attrs.getFloat("layout_constraintVertical_weight",   ConstraintWidget::UNKNOWN);

    // match_constraint (0dp) sizing: default spread/wrap/percent + percent value + min/max.
    static const std::unordered_map<std::string,int> matchDefault = {
        {"spread",  ConstraintWidget::MATCH_CONSTRAINT_SPREAD},
        {"wrap",    ConstraintWidget::MATCH_CONSTRAINT_WRAP},
        {"percent", ConstraintWidget::MATCH_CONSTRAINT_PERCENT}
    };
    matchConstraintDefaultWidth  = attrs.getInt("layout_constraintWidth_default",  matchDefault, ConstraintWidget::MATCH_CONSTRAINT_SPREAD);
    matchConstraintDefaultHeight = attrs.getInt("layout_constraintHeight_default", matchDefault, ConstraintWidget::MATCH_CONSTRAINT_SPREAD);
    matchConstraintPercentWidth  = attrs.getFloat("layout_constraintWidth_percent",  1.0f);
    matchConstraintPercentHeight = attrs.getFloat("layout_constraintHeight_percent", 1.0f);
    matchConstraintMinWidth   = attrs.getDimensionPixelSize("layout_constraintWidth_min",  0);
    matchConstraintMaxWidth   = attrs.getDimensionPixelSize("layout_constraintWidth_max",  0);
    matchConstraintMinHeight  = attrs.getDimensionPixelSize("layout_constraintHeight_min", 0);
    matchConstraintMaxHeight  = attrs.getDimensionPixelSize("layout_constraintHeight_max", 0);

    validate();
}

ConstraintLayout::LayoutParams::LayoutParams(int width, int height)
    : MarginLayoutParams(width, height) {
    validate();
}

// FIXED/WRAP_CONTENT -> dimension fixed; MATCH_CONSTRAINT(0dp)/MATCH_PARENT -> variable.
// Guideline: if guideBegin/End/Percent set, replace mWidget with a Guideline.
void ConstraintLayout::LayoutParams::validate() {
    mHorizontalDimensionFixed = (width != 0 && width != LayoutParams::MATCH_PARENT);
    mVerticalDimensionFixed   = (height != 0 && height != LayoutParams::MATCH_PARENT);
    if (guideBegin != UNSET || guideEnd != UNSET || guidePercent != UNSET_FLOAT) {
        mIsGuideline = true;
        mHorizontalDimensionFixed = true;
        mVerticalDimensionFixed = true;
        auto g = std::make_unique<clcore::Guideline>();
        int orient = (orientation == ConstraintWidget::VERTICAL) ? ConstraintWidget::VERTICAL
                     : ConstraintWidget::HORIZONTAL;
        g->setOrientation(orient);
        if (guidePercent != UNSET_FLOAT)      g->setGuidePercent(guidePercent);
        else if (guideBegin != UNSET)         g->setGuideBegin(guideBegin);
        else if (guideEnd != UNSET)           g->setGuideEnd(guideEnd);
        mWidget = std::move(g);
    }
}

// ===========================================================================
// ConstraintLayout
// ===========================================================================
ConstraintLayout::ConstraintLayout(Context* ctx, const AttributeSet& attrs)
    : ViewGroup(ctx, attrs) {
    mLayoutWidget.setMeasurer(asMeasurer());
    mLayoutWidget.setCompanionWidget(this);
    mMinWidth  = attrs.getDimensionPixelSize("android_minWidth", 0);
    mMinHeight = attrs.getDimensionPixelSize("android_minHeight", 0);
    mMaxWidth  = attrs.getDimensionPixelSize("android_maxWidth", INT_MAX);
    mMaxHeight = attrs.getDimensionPixelSize("android_maxHeight", INT_MAX);

    // app:layoutDescription may point at a <StateSet> (adaptive layout) for a ConstraintLayout, or a
    // <MotionScene> for the MotionLayout subclass. Peek the root: build a StateSet only for non-
    // MotionScene roots (MotionLayout builds its own scene from the same attr).
    const std::string layoutDesc = attrs.getString("layoutDescription", "");
    if (!layoutDesc.empty()) {
        XmlPullParser parser(ctx, layoutDesc);
        while (parser.getEventType() != XmlPullParser::START_TAG &&
                parser.getEventType() != XmlPullParser::END_DOCUMENT &&
                parser.getEventType() != XmlPullParser::BAD_DOCUMENT) {
            parser.next();
        }
        if (parser.getEventType() == XmlPullParser::START_TAG && parser.getName() != "MotionScene") {
            mConstraintLayoutStates = std::make_unique<ConstraintLayoutStates>(ctx, this, parser);
        }
    }
}

ConstraintLayout::ConstraintLayout(int width, int height)
    : ViewGroup(width, height) {
    mLayoutWidget.setMeasurer(asMeasurer());
    mLayoutWidget.setCompanionWidget(this);
}

// Defined here (not defaulted in the header) so the unique_ptr<ConstraintLayoutStates> member
// destroys with a complete type.
ConstraintLayout::~ConstraintLayout() = default;

void ConstraintLayout::loadLayoutDescription(const std::string& resource) {
    mConstraintLayoutStates = std::make_unique<ConstraintLayoutStates>(getContext(), this, resource);
}

void ConstraintLayout::setState(int id, int screenWidth, int screenHeight) {
    if (mConstraintLayoutStates != nullptr) {
        mConstraintLayoutStates->updateConstraints(id, (float) screenWidth, (float) screenHeight);
    }
}

SharedValues& ConstraintLayout::getSharedValues() {
    static SharedValues sSharedValues; // process-wide singleton (Meyers)
    return sSharedValues;
}

ViewGroup::LayoutParams* ConstraintLayout::generateLayoutParams(const AttributeSet& attrs) const {
    return new LayoutParams(getContext(), attrs);
}

ViewGroup::LayoutParams* ConstraintLayout::generateDefaultLayoutParams() const {
    return new LayoutParams(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
}

bool ConstraintLayout::checkLayoutParams(const ViewGroup::LayoutParams* p) const {
    return dynamic_cast<const LayoutParams*>(p) != nullptr;
}

ConstraintWidget* ConstraintLayout::getViewWidget(View* view) {
    if (view == this) return &mLayoutWidget;
    // Helper children (Barrier, ...) use their owned core helper widget; Group owns none and
    // falls through to its LayoutParams widget.
    if (auto* helper = dynamic_cast<ConstraintHelper*>(view)) {
        if (HelperWidget* hw = helper->getHelperWidget()) return hw;
    }
    auto* lp = dynamic_cast<LayoutParams*>(view->getLayoutParams());
    return lp ? lp->mWidget.get() : nullptr;
}

void ConstraintLayout::onViewAdded(View* child) {
    if (auto* helper = dynamic_cast<ConstraintHelper*>(child)) {
        helper->validateParams();
        if (auto* lp = dynamic_cast<LayoutParams*>(child->getLayoutParams())) {
            lp->mIsHelper = true;
        }
        // avoid dupes (addView can be called more than once in edge cases)
        if (std::find(mConstraintHelpers.begin(), mConstraintHelpers.end(), helper)
                == mConstraintHelpers.end()) {
            mConstraintHelpers.push_back(helper);
        }
    }
}

void ConstraintLayout::onViewRemoved(View* child) {
    if (auto* helper = dynamic_cast<ConstraintHelper*>(child)) {
        auto it = std::find(mConstraintHelpers.begin(), mConstraintHelpers.end(), helper);
        if (it != mConstraintHelpers.end()) mConstraintHelpers.erase(it);
    }
}

void ConstraintLayout::setChildrenConstraints() {
    const int count = getChildCount();
    for (int i = 0; i < count; i++) {
        ConstraintWidget* widget = getViewWidget(getChildAt(i));
        if (widget) widget->reset();
    }
    mLayoutWidget.removeAllChildren();

    // id -> widget map
    mIdToWidget.clear();
    mIdToWidget[PARENT_ID] = &mLayoutWidget;
    mIdToWidget[getId()]    = &mLayoutWidget;
    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        ConstraintWidget* widget = getViewWidget(child);
        if (widget) mIdToWidget[child->getId()] = widget;
    }

    // Helpers: resolve referenced ids -> ConstraintWidgets and populate their core helper widget
    // before the solver pass (Barrier.addToSolver reads its referenced widgets).
    for (ConstraintHelper* helper : mConstraintHelpers) {
        helper->updatePreLayout(this);
    }
    // Placeholders: resolve their content view (marks its widget as in-placeholder / GONE at origin).
    for (int i = 0; i < count; i++) {
        if (auto* placeholder = dynamic_cast<Placeholder*>(getChildAt(i))) {
            placeholder->updatePreLayout(this);
        }
    }

    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        ConstraintWidget* widget = getViewWidget(child);
        if (!widget) continue;
        auto* lp = static_cast<LayoutParams*>(child->getLayoutParams());
        mLayoutWidget.add(widget);
        applyConstraintsFromLayoutParams(child, widget, lp);
    }
}

void ConstraintLayout::applyConstraintsFromLayoutParams(View* child, ConstraintWidget* widget,
        LayoutParams* lp) {
    if (lp->mIsGuideline) {
        // Guideline: orientation + begin/end/percent set in validate(). Override the positioning
        // values here with the layout-direction-resolved ones so a vertical Guideline mirrors under
        // RTL when guidelineUseRtl is set (AndroidX lines 1376-1390 read mResolvedGuide*):
        //   percent → 1 - percent, begin ↔ end swap. Horizontal guidelines are unaffected.
        if (auto* g = dynamic_cast<clcore::Guideline*>(widget)) {
            const bool rtlGuide = child->isLayoutRtl() && lp->guidelineUseRtl
                    && lp->orientation == ConstraintWidget::VERTICAL;
            if (lp->guidePercent != LayoutParams::UNSET_FLOAT) {
                g->setGuidePercent(rtlGuide ? 1.0f - lp->guidePercent : lp->guidePercent);
            } else if (lp->guideBegin != LayoutParams::UNSET) {
                if (rtlGuide) g->setGuideEnd(lp->guideBegin);   else g->setGuideBegin(lp->guideBegin);
            } else if (lp->guideEnd != LayoutParams::UNSET) {
                if (rtlGuide) g->setGuideBegin(lp->guideEnd);   else g->setGuideEnd(lp->guideEnd);
            }
        }
        // Guideline::addToSolver (virtual override) handles positioning; skip anchor/dimension logic.
        return;
    }
    widget->setVisibility(child->getVisibility());
    widget->setCompanionWidget(child);
    if (lp->mIsInPlaceholder) {
        // A content view pulled into a Placeholder: its widget is gone at its origin.
        widget->setInPlaceholder(true);
        widget->setVisibility(View::GONE);
    }

    // ConstraintHelper children resolve their RTL-dependent type (Barrier START/END -> LEFT/RIGHT).
    if (auto* helper = dynamic_cast<ConstraintHelper*>(child)) {
        helper->resolveRtl(widget, mLayoutWidget.isRtl());
    }

    auto resolveTarget = [&](int id) -> ConstraintWidget* {
        auto it = mIdToWidget.find(id);
        return (it != mIdToWidget.end()) ? it->second : nullptr;
    };

    // match_parent is modelled as DimensionBehaviour::MATCH_PARENT (see the dimension-behaviour
    // block below) and resolved by Optimizer::checkMatchParent, which pins the widget to the
    // container edges in ConstraintWidgetContainer::layout(). No anchor rewiring is needed here.

    // Resolve effective Left/Right anchors. Per AndroidX LayoutParams.resolveLayoutDirection
    // (ConstraintLayout.java:3834-3922): Start/End take precedence over Left/Right — if ANY Start/End
    // anchor is set, Left/Right is ignored entirely; only when NO Start/End is present do Left/Right
    // apply (the "all-or-nothing" fallback at 3898-3922). Start/End map by the child's resolved
    // layout direction (LTR: Start→Left, End→Right; RTL: Start→Right, End→Left).
    const bool rtl = child->isLayoutRtl();
    const bool startEndDefined = (lp->startToStart != LayoutParams::UNSET)
            || (lp->startToEnd != LayoutParams::UNSET)
            || (lp->endToStart != LayoutParams::UNSET)
            || (lp->endToEnd != LayoutParams::UNSET);
    int leftToLeftE, leftToRightE, rightToLeftE, rightToRightE;
    if (startEndDefined) {
        // Start/End precedence (per direction).
        leftToLeftE  = rtl ? lp->endToEnd     : lp->startToStart;
        leftToRightE = rtl ? lp->endToStart   : lp->startToEnd;
        rightToLeftE = rtl ? lp->startToEnd   : lp->endToStart;
        rightToRightE= rtl ? lp->startToStart : lp->endToEnd;
    } else {
        // No Start/End → fall back to explicit Left/Right.
        leftToLeftE  = lp->leftToLeft;
        leftToRightE = lp->leftToRight;
        rightToLeftE = lp->rightToLeft;
        rightToRightE= lp->rightToRight;
    }
    // Effective gone margins: a Start/End gone margin resolves into Left/Right by direction, falling
    // back to the explicit goneLeft/goneRight when absent (AndroidX lines 3851-3856, 3890-3895).
    int goneLeftE  = lp->goneLeftMargin;
    int goneRightE = lp->goneRightMargin;
    if (rtl) {
        if (lp->goneEndMargin   != LayoutParams::GONE_UNSET) goneLeftE  = lp->goneEndMargin;
        if (lp->goneStartMargin != LayoutParams::GONE_UNSET) goneRightE = lp->goneStartMargin;
    } else {
        if (lp->goneStartMargin != LayoutParams::GONE_UNSET) goneLeftE  = lp->goneStartMargin;
        if (lp->goneEndMargin   != LayoutParams::GONE_UNSET) goneRightE = lp->goneEndMargin;
    }

    // Target anchors are resolved through getAnchor(Type) rather than via the &t->mLeft/mRight/mTop/
    // mBottom members. For an ordinary widget the two are identical, but a Guideline only exposes a
    // single active anchor: getAnchor() redirects TOP/BOTTOM (horizontal guideline) and LEFT/RIGHT
    // (vertical guideline) to that active anchor, and returns null for the off-axis sides. Reaching
    // for the member directly would connect to a Guideline's never-positioned orphan anchor (mBottom/
    // mRight stay 0), so the "to-Bottom"/"to-Right" variants would collapse to the origin. This
    // mirrors AndroidX, which resolves the end anchor with endWidget.getAnchor(endType).
    // Left (leftToLeft preferred over leftToRight)
    if (leftToLeftE != LayoutParams::UNSET || leftToRightE != LayoutParams::UNSET) {
        bool toLeft = (leftToLeftE != LayoutParams::UNSET);
        int tid = toLeft ? leftToLeftE : leftToRightE;
        if (ConstraintWidget* t = resolveTarget(tid)) {
            if (ConstraintAnchor* ta = t->getAnchor(toLeft ? ConstraintAnchor::Type::LEFT
                                                           : ConstraintAnchor::Type::RIGHT)) {
                widget->mLeft.connect(ta, lp->leftMargin, goneLeftE, true);
            }
        }
    }
    // Right
    if (rightToLeftE != LayoutParams::UNSET || rightToRightE != LayoutParams::UNSET) {
        bool toLeft = (rightToLeftE != LayoutParams::UNSET);
        int tid = toLeft ? rightToLeftE : rightToRightE;
        if (ConstraintWidget* t = resolveTarget(tid)) {
            if (ConstraintAnchor* ta = t->getAnchor(toLeft ? ConstraintAnchor::Type::LEFT
                                                           : ConstraintAnchor::Type::RIGHT)) {
                widget->mRight.connect(ta, lp->rightMargin, goneRightE, true);
            }
        }
    }
    // Top
    if (lp->topToTop != LayoutParams::UNSET || lp->topToBottom != LayoutParams::UNSET) {
        bool toTop = (lp->topToTop != LayoutParams::UNSET);
        int tid = toTop ? lp->topToTop : lp->topToBottom;
        if (ConstraintWidget* t = resolveTarget(tid)) {
            if (ConstraintAnchor* ta = t->getAnchor(toTop ? ConstraintAnchor::Type::TOP
                                                          : ConstraintAnchor::Type::BOTTOM)) {
                widget->mTop.connect(ta, lp->topMargin, lp->goneTopMargin, true);
            }
        }
    }
    // Bottom
    if (lp->bottomToTop != LayoutParams::UNSET || lp->bottomToBottom != LayoutParams::UNSET) {
        bool toTop = (lp->bottomToTop != LayoutParams::UNSET);
        int tid = toTop ? lp->bottomToTop : lp->bottomToBottom;
        if (ConstraintWidget* t = resolveTarget(tid)) {
            if (ConstraintAnchor* ta = t->getAnchor(toTop ? ConstraintAnchor::Type::TOP
                                                          : ConstraintAnchor::Type::BOTTOM)) {
                widget->mBottom.connect(ta, lp->bottomMargin, lp->goneBottomMargin, true);
            }
        }
    }

    // Horizontal bias mirrors under RTL when the constraint came from Start/End anchors, since bias
    // is measured from the left anchor and the anchor pair itself is mirrored (AndroidX line 3858).
    widget->mHorizontalBiasPercent = (startEndDefined && rtl) ? 1.0f - lp->horizontalBias
                                                              : lp->horizontalBias;
    widget->mVerticalBiasPercent = lp->verticalBias;

    // Dimension behaviour
    if (lp->mHorizontalDimensionFixed) {
        widget->setHorizontalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
        widget->setWidth(lp->width);
        if (lp->width == LayoutParams::WRAP_CONTENT) {
            widget->setHorizontalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::WRAP_CONTENT);
        }
    } else if (lp->width == LayoutParams::MATCH_PARENT) {
        // match_parent is modelled as MATCH_PARENT (not MATCH_CONSTRAINT). Optimizer::checkMatchParent
        // (run from ConstraintWidgetContainer::layout before addToSolver) pins it to the container
        // edges, resolving it to the full parent width regardless of the widget's own anchors.
        widget->setHorizontalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::MATCH_PARENT);
    } else {
        // MATCH_CONSTRAINT (0dp). Apply the match-style from LayoutParams (Android:
        // setHorizontalMatchStyle). The solver's applyConstraints handles SPREAD/WRAP/PERCENT.
        widget->setHorizontalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT);
        widget->setWidth(0);
        widget->mMatchConstraintDefaultWidth  = lp->matchConstraintDefaultWidth;
        widget->mMatchConstraintMinWidth      = lp->matchConstraintMinWidth;
        widget->mMatchConstraintMaxWidth      = lp->matchConstraintMaxWidth;
        widget->mMatchConstraintPercentWidth  = lp->matchConstraintPercentWidth;
    }
    if (lp->mVerticalDimensionFixed) {
        widget->setVerticalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
        widget->setHeight(lp->height);
        if (lp->height == LayoutParams::WRAP_CONTENT) {
            widget->setVerticalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::WRAP_CONTENT);
        }
    } else if (lp->height == LayoutParams::MATCH_PARENT) {
        // match_parent -> MATCH_PARENT; Optimizer::checkMatchParent pins it to the container edges.
        widget->setVerticalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::MATCH_PARENT);
    } else {
        widget->setVerticalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT);
        widget->setHeight(0);
        widget->mMatchConstraintDefaultHeight = lp->matchConstraintDefaultHeight;
        widget->mMatchConstraintMinHeight     = lp->matchConstraintMinHeight;
        widget->mMatchConstraintMaxHeight     = lp->matchConstraintMaxHeight;
        widget->mMatchConstraintPercentHeight = lp->matchConstraintPercentHeight;
    }

    // Ratio
    if (lp->dimensionRatio > 0) {
        widget->mDimensionRatio = lp->dimensionRatio;
        widget->mDimensionRatioSide = lp->dimensionRatioSide;
    }

    // Chain styles
    widget->mHorizontalChainStyle = lp->horizontalChainStyle;
    widget->mVerticalChainStyle = lp->verticalChainStyle;
    // Chain weights (distribute free space among 0dp chain elements).
    widget->mWeight[ConstraintWidget::HORIZONTAL] = lp->horizontalWeight;
    widget->mWeight[ConstraintWidget::VERTICAL]   = lp->verticalWeight;

    // Baseline constraint (overrides top/bottom). Resolve the target anchor via getAnchor() so a
    // Guideline target (which has no baseline) yields null and the connection is skipped.
    if (lp->baselineToBaseline != LayoutParams::UNSET) {
        if (ConstraintWidget* t = resolveTarget(lp->baselineToBaseline)) {
            if (ConstraintAnchor* ta = t->getAnchor(ConstraintAnchor::Type::BASELINE)) {
                widget->mBaseline.connect(ta, 0, LayoutParams::GONE_UNSET, true);
                widget->setHasBaseline(true);
                t->setHasBaseline(true);
                widget->mTop.reset();
                widget->mBottom.reset();
            }
        }
    }

    // Circular constraint: position this view on a circle around the target (Java: ConstraintLayout
    // lines 1429-1433 translate params.circle* into widget.connectCircularConstraint).
    if (lp->circleConstraint != LayoutParams::UNSET) {
        if (ConstraintWidget* t = resolveTarget(lp->circleConstraint)) {
            widget->connectCircularConstraint(t, lp->circleAngle, lp->circleRadius);
        }
    }
}

// --- BasicMeasure::Measurer ---
void ConstraintLayout::measure(ConstraintWidget* widget, BasicMeasure::Measure* m) {
    if (widget->getVisibility() == ConstraintWidget::GONE) {
        m->measuredWidth = 0;
        m->measuredHeight = 0;
        m->measuredBaseline = 0;
        m->measuredHasBaseline = false;
        m->measuredNeedsSolverPass = false;
        return;
    }
    View* child = static_cast<View*>(widget->getCompanionWidget());
    if (child == nullptr || child->getParent() == nullptr) return;

    auto specFor = [&](ConstraintWidget::DimensionBehaviour b, int dim, bool horizontal) -> int {
        int parentSpec = horizontal ? mWidthSpec : mHeightSpec;
        int padding   = horizontal ? mPaddingWidth : mPaddingHeight;
        if (b == ConstraintWidget::DimensionBehaviour::FIXED) {
            return View::MeasureSpec::makeMeasureSpec(dim, View::MeasureSpec::EXACTLY);
        } else if (b == ConstraintWidget::DimensionBehaviour::WRAP_CONTENT) {
            return ViewGroup::getChildMeasureSpec(parentSpec, padding, LayoutParams::WRAP_CONTENT);
        } else if (b == ConstraintWidget::DimensionBehaviour::MATCH_PARENT) {
            return ViewGroup::getChildMeasureSpec(parentSpec, padding, LayoutParams::MATCH_PARENT);
        }
        // MATCH_CONSTRAINT (0dp). `dim` is widget->getWidth()/Height() — after a solve it is the
        // resolved size; honor the measure strategy so the child's content-dependent dimension
        // (e.g. text height under the resolved width) can adapt, then re-solve if it changed.
        if (m->measureStrategy == BasicMeasure::Measure::TRY_GIVEN_DIMENSIONS
                || m->measureStrategy == BasicMeasure::Measure::USE_GIVEN_DIMENSIONS) {
            return View::MeasureSpec::makeMeasureSpec(dim, View::MeasureSpec::EXACTLY);
        }
        // SELF_DIMENSIONS: not solved yet — measure wrap to seed the solver.
        return ViewGroup::getChildMeasureSpec(parentSpec, padding, LayoutParams::WRAP_CONTENT);
    };

    int wSpec = specFor(m->horizontalBehavior, m->horizontalDimension, true);
    int hSpec = specFor(m->verticalBehavior, m->verticalDimension, false);
    child->measure(wSpec, hSpec);

    int w = child->getMeasuredWidth();
    int h = child->getMeasuredHeight();
    int baseline = child->getBaseline();
    m->measuredWidth = w;
    m->measuredHeight = h;
    m->measuredBaseline = baseline;
    m->measuredHasBaseline = (baseline != -1);
    m->measuredNeedsSolverPass = (w != m->horizontalDimension) || (h != m->verticalDimension);
}

void ConstraintLayout::didMeasures() {
    // No-op: Placeholder/helper post-measure work is driven directly from onMeasure
    // (Placeholder::updatePostMeasure loop + the helper updatePostLayout pass in onLayout).
}

void ConstraintLayout::dispatchDraw(Canvas& canvas) {
    ViewGroup::dispatchDraw(canvas);
    if (debugDraw()) {
        drawDebugOverlays(canvas);
    }
}

// CDROID-only visual debug aid. With View::debugDraw() on ("show layout bounds", VIEW_DEBUG),
// paint the otherwise-invisible helpers: guidelines as full-span lines with their percent/begin/
// end annotation, barriers as a direction line, and each child's anchor connections to its
// targets (modeled on AndroidX's private DEBUG_DRAW_CONSTRAINTS, which ships compiled-out).
void ConstraintLayout::drawDebugOverlays(Canvas& canvas) {
    const int W = getWidth();
    const int H = getHeight();
    char label[32];

    canvas.set_line_width(1);
    canvas.select_font_face("sans",
                            Cairo::ToyFontFace::Slant::NORMAL,
                            Cairo::ToyFontFace::Weight::NORMAL);
    canvas.set_font_size(11);
    // Guidelines and barriers are dashed (Android Studio renders helpers as dotted lines);
    // constraint connection lines are solid, so set_dash({}) is re-applied before that section.
    canvas.set_dash(std::vector<double>{4.0, 3.0}, 0);

    // --- Guidelines: full-span line + percent/begin/end annotation ---
    const int count = getChildCount();
    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        auto* lp = dynamic_cast<LayoutParams*>(child->getLayoutParams());
        if (!lp || !lp->mIsGuideline) continue;
        auto* g = dynamic_cast<clcore::Guideline*>(getViewWidget(child));
        if (g == nullptr) continue;

        const bool vertical = (g->getOrientation() == clcore::Guideline::VERTICAL);
        if (g->getRelativePercent() != -1) {
            snprintf(label, sizeof(label), "G %.0f%%", g->getRelativePercent() * 100.0f);
        } else if (g->getRelativeBegin() != -1) {
            snprintf(label, sizeof(label), "G begin %d", g->getRelativeBegin());
        } else if (g->getRelativeEnd() != -1) {
            snprintf(label, sizeof(label), "G end %d", g->getRelativeEnd());
        } else {
            snprintf(label, sizeof(label), "G");
        }

        // Olive-green dashed full-span line (Android Studio renders helpers as dotted lines).
        // The editor's edge handle is an interaction affordance — not drawn at runtime.
        canvas.set_color(0x99, 0xCC, 0x33, 0xD0);
        if (vertical) {
            const int x = g->getX();
            canvas.move_to(x + 0.5, 0);
            canvas.line_to(x + 0.5, H);
            canvas.stroke();
            canvas.set_color(0x99, 0xCC, 0x33, 0xFF);
            canvas.move_to(x + 6, 12);
            canvas.show_text(label);
        } else {
            const int y = g->getY();
            canvas.move_to(0, y + 0.5);
            canvas.line_to(W, y + 0.5);
            canvas.stroke();
            canvas.set_color(0x99, 0xCC, 0x33, 0xFF);
            canvas.move_to(8, y - 4);
            canvas.show_text(label);
        }
    }

    // --- Barriers: direction line + marker ---
    for (ConstraintHelper* helper : mConstraintHelpers) {
        ConstraintWidget* w = getViewWidget(helper);
        auto* b = dynamic_cast<clcore::Barrier*>(w);
        if (b == nullptr) continue;

        // LEFT/RIGHT barriers run vertically (a column at x); TOP/BOTTOM run horizontally (a row at y).
        const int type = b->getBarrierType();
        const bool vertical = (type == clcore::Barrier::LEFT || type == clcore::Barrier::RIGHT);
        const char marker = vertical ? (type == clcore::Barrier::LEFT ? '<' : '>')
                                     : (type == clcore::Barrier::TOP  ? '^' : 'v');
        snprintf(label, sizeof(label), "B %c", marker);

        // Purple dashed helper line, like a guideline; the direction is drawn as a text marker.
        canvas.set_color(0xAB, 0x47, 0xBC, 0xE0); // purple (AS helper accent)
        if (vertical) {
            const int x = w->getX();
            canvas.move_to(x + 0.5, 0);
            canvas.line_to(x + 0.5, H);
            canvas.stroke();
            canvas.set_color(0xAB, 0x47, 0xBC, 0xFF);
            canvas.move_to(x + 6, H - 4);
            canvas.show_text(label);
        } else {
            const int y = w->getY();
            canvas.move_to(0, y + 0.5);
            canvas.line_to(W, y + 0.5);
            canvas.stroke();
            canvas.set_color(0xAB, 0x47, 0xBC, 0xFF);
            canvas.move_to(W - 24, y - 3);
            canvas.show_text(label);
        }
    }

    // --- Other ConstraintHelpers (Grid/Flow/Layer/CircularFlow): range box + type label ---
    // These are virtual too (AndroidX ConstraintHelper.onDraw is literally "// Nothing"); AS Layout
    // Inspector outlines the helper's solved frame, so we do the same to make its reach visible.
    // Barrier is already drawn as a line above; Group owns no widget (0x0 frame) and is skipped by
    // the extent check. Still in the dashed section (helpers render dotted, like AS).
    for (ConstraintHelper* helper : mConstraintHelpers) {
        ConstraintWidget* w = getViewWidget(helper);
        if (w == nullptr) continue;
        if (dynamic_cast<clcore::Barrier*>(w)) continue;      // already drawn as a line
        const int x = w->getX(), y = w->getY();
        const int ww = w->getWidth(), hh = w->getHeight();
        if (ww <= 0 && hh <= 0) continue;                     // no extent (e.g. Group)
        canvas.set_color(0xE8, 0xA8, 0x2E, 0xD0);             // amber, distinct from guideline/barrier
        canvas.rectangle(x + 0.5, y + 0.5, ww - 1, hh - 1);
        canvas.stroke();
        const char* name = "Helper";
        if (dynamic_cast<Grid*>(helper)) name = "Grid";
        else if (dynamic_cast<Flow*>(helper)) name = "Flow";
        else if (dynamic_cast<Layer*>(helper)) name = "Layer";
        else if (dynamic_cast<CircularFlow*>(helper)) name = "CircularFlow";
        canvas.set_color(0xE8, 0xA8, 0x2E, 0xFF);
        canvas.move_to(x + 4, y - 4);
        canvas.show_text(name);
    }

    // --- Baseline guides: a green line across each child at its text-baseline height ---
    // Pure layout aid (where this view baseline-aligns to others); no editor handles. Solid here.
    canvas.set_dash(std::vector<double>{}, 0);
    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        auto* lp = dynamic_cast<LayoutParams*>(child->getLayoutParams());
        if (!lp || lp->mIsGuideline || lp->mIsHelper) continue;
        if (child->getVisibility() == View::GONE) continue;
        ConstraintWidget* widget = getViewWidget(child);
        if (widget == nullptr) continue;
        if (!widget->hasBaseline() || widget->getBaselineDistance() <= 0) continue;
        const int ox = widget->getX(), oy = widget->getY();
        const int ow = widget->getWidth();
        const int by = oy + widget->getBaselineDistance();
        canvas.set_color(0x4C, 0xAF, 0x50, 0xC0); // green baseline
        canvas.move_to(ox + 0.5, by + 0.5);
        canvas.line_to(ox + ow - 0.5, by + 0.5);
        canvas.stroke();
    }

    // --- Constraint connection lines (child anchor -> target anchor) ---
    // Solid blue with a margin readout; opposing constraints (both sides of an axis connected on a
    // non-stretch dimension) render as a zigzag spring, like the editor.
    auto drawConnection = [&](ConstraintAnchor& a, bool spring) {
        if (!a.isConnected() || a.mTarget == nullptr || a.mOwner == nullptr) return;
        ConstraintWidget* widget = a.mOwner;
        ConstraintWidget* target = a.mTarget->mOwner;
        if (target == nullptr) return;
        const int ox = widget->getX(), oy = widget->getY();
        const int ow = widget->getWidth(), oh = widget->getHeight();
        const int tx = target->getX(), ty = target->getY();
        const int tw = target->getWidth(), th = target->getHeight();
        int x1, y1, x2, y2;
        if (a.mType == ConstraintAnchor::Type::TOP
                || a.mType == ConstraintAnchor::Type::BOTTOM) {
            x1 = ox + ow / 2;
            x2 = tx + tw / 2;
            y1 = (a.mType == ConstraintAnchor::Type::BOTTOM) ? oy + oh : oy;
            y2 = (a.mTarget->mType == ConstraintAnchor::Type::TOP) ? ty : ty + th;
        } else {
            y1 = oy + oh / 2;
            y2 = ty + th / 2;
            x1 = (a.mType == ConstraintAnchor::Type::RIGHT) ? ox + ow : ox;
            x2 = (a.mTarget->mType == ConstraintAnchor::Type::LEFT) ? tx : tx + tw;
        }
        if (spring) {
            // Zigzag "spring" — Android Studio coils opposing constraints.
            const double dx = x2 - x1, dy = y2 - y1;
            const double len = std::sqrt(dx * dx + dy * dy);
            const int teeth = std::max(4, static_cast<int>(len / 8));
            const double ux = dx / len, uy = dy / len, px = -uy, py = ux;
            canvas.move_to(x1 + 0.5, y1 + 0.5);
            for (int k = 1; k < teeth; k++) {
                const double t = static_cast<double>(k) / teeth;
                const double off = (k % 2 == 1) ? 3.0 : -3.0;
                canvas.line_to(x1 + dx * t + px * off, y1 + dy * t + py * off);
            }
            canvas.line_to(x2 + 0.5, y2 + 0.5);
        } else {
            canvas.move_to(x1 + 0.5, y1 + 0.5);
            canvas.line_to(x2 + 0.5, y2 + 0.5);
        }
        canvas.stroke();
        // Margin readout on a white label (Android Studio prints the margin atop each constraint).
        const int margin = a.getMargin();
        if (margin > 0) {
            snprintf(label, sizeof(label), "%d", margin);
            int tw2 = 0, th2 = 0;
            canvas.get_text_size(label, &tw2, &th2);
            const float mx = (x1 + x2) / 2.0f, my = (y1 + y2) / 2.0f;
            canvas.set_color(0xFF, 0xFF, 0xFF, 0xE0);
            canvas.rectangle(mx - tw2 / 2 - 3, my - th2 / 2, tw2 + 6, th2);
            canvas.fill();
            canvas.set_color(0x33, 0x33, 0x33, 0xFF);
            canvas.move_to(mx - tw2 / 2, my + th2 / 2 - 1);
            canvas.show_text(label);
        }
    };
    using DB = ConstraintWidget::DimensionBehaviour;
    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        auto* lp = dynamic_cast<LayoutParams*>(child->getLayoutParams());
        if (!lp || lp->mIsGuideline || lp->mIsHelper) continue;
        ConstraintWidget* widget = getViewWidget(child);
        if (widget == nullptr) continue;
        const bool vOpp = widget->getVerticalDimensionBehaviour() != DB::MATCH_CONSTRAINT
                && widget->mTop.isConnected() && widget->mBottom.isConnected();
        const bool hOpp = widget->getHorizontalDimensionBehaviour() != DB::MATCH_CONSTRAINT
                && widget->mLeft.isConnected() && widget->mRight.isConnected();
        canvas.set_color(0x42, 0x85, 0xF4, 0xD0); // Android Studio constraint blue
        drawConnection(widget->mTop, vOpp);
        drawConnection(widget->mBottom, vOpp);
        drawConnection(widget->mLeft, hOpp);
        drawConnection(widget->mRight, hOpp);
    }
}

void ConstraintLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    // Propagate this view's resolved layout direction to the core container BEFORE building child
    // constraints. AndroidX sets container RTL at the top of onMeasure (ConstraintLayout.java:1852),
    // ahead of updateHierarchy — Start/End anchor resolution, chains (ChainHead) and helpers
    // (Barrier.resolveRtl) all read mLayoutWidget.isRtl() during setChildrenConstraints().
    mLayoutWidget.setRtl(isLayoutRtl());
    setChildrenConstraints();
    resolveSystem(widthMeasureSpec, heightMeasureSpec);
    // Placeholders adopt their content's resolved size post-solve (BasicMeasure's match-constraint
    // convergence loop runs before this; Placeholder itself needs a single post-solve adoption).
    const int count = getChildCount();
    for (int i = 0; i < count; i++) {
        if (auto* placeholder = dynamic_cast<Placeholder*>(getChildAt(i))) {
            placeholder->updatePostMeasure(this);
        }
    }
    // For a WRAP_CONTENT dimension (AT_MOST/UNSPECIFIED spec) the solver pins the container to its
    // desired (AT_MOST max) size; shrink it to the actual content extent of the solved children so a
    // WRAP container sizes to its content. EXACTLY dimensions keep the solver's size. Virtual helpers
    // (Guideline/Barrier/VirtualLayout) carry no visual extent and are skipped — a Flow's referenced
    // children are regular widgets and are included. BasicMeasure's match-constraint convergence loop
    // (solverMeasure) re-measures 0dp children with their resolved size before this, so a WRAP
    // container's content extent is up to date; this shrinks the AT_MOST max down to that extent.
    int measuredW = mLayoutWidget.getWidth();
    int measuredH = mLayoutWidget.getHeight();
    int wMode = View::MeasureSpec::getMode(widthMeasureSpec);
    int hMode = View::MeasureSpec::getMode(heightMeasureSpec);
    if (wMode != View::MeasureSpec::EXACTLY || hMode != View::MeasureSpec::EXACTLY) {
        int maxRight = 0, maxBottom = 0;
        for (ConstraintWidget* w : mLayoutWidget.mChildren) {
            if (w->getVisibility() == ConstraintWidget::GONE) continue;
            if (dynamic_cast<clcore::Guideline*>(w) != nullptr) continue;
            if (w->isBarrier() || w->isVirtualLayout()) continue;
            maxRight  = std::max(maxRight,  w->getX() + w->getWidth());
            maxBottom = std::max(maxBottom, w->getY() + w->getHeight());
        }
        if (wMode != View::MeasureSpec::EXACTLY) measuredW = maxRight;
        if (hMode != View::MeasureSpec::EXACTLY) measuredH = maxBottom;
    }
    resolveMeasuredDimension(widthMeasureSpec, heightMeasureSpec, measuredW, measuredH);
}

void ConstraintLayout::resolveSystem(int widthSpec, int heightSpec) {
    int paddingLeft = getPaddingLeft(), paddingRight = getPaddingRight();
    int paddingTop = getPaddingTop(), paddingBottom = getPaddingBottom();
    mWidthSpec = widthSpec;
    mHeightSpec = heightSpec;
    mPaddingWidth = paddingLeft + paddingRight;
    mPaddingHeight = paddingTop + paddingBottom;

    int widthMode = View::MeasureSpec::getMode(widthSpec);
    int heightMode = View::MeasureSpec::getMode(heightSpec);
    int widthSize = View::MeasureSpec::getSize(widthSpec) - mPaddingWidth;
    int heightSize = View::MeasureSpec::getSize(heightSpec) - mPaddingHeight;

    setSelfDimensionBehaviour(widthMode, widthSize, heightMode, heightSize);

    // OPTIMIZATION_GRAPH off (linear solve only). Known gap: padding offset (paddingLeft/paddingTop)
    // is not forwarded to the solver here — the driver sites the container at (0,0); layouts with
    // CL padding don't offset children correctly. TODO: pass padding so children come out parent-relative.
    mLayoutWidget.measure(Optimizer::OPTIMIZATION_NONE,
                          paddingLeft, paddingTop,
                          widthMode, widthSize, heightMode, heightSize,
                          mLayoutWidget.getWidth(), mLayoutWidget.getHeight());
}

void ConstraintLayout::setSelfDimensionBehaviour(int widthMode, int widthSize,
        int heightMode, int heightSize) {
    auto behaviour = [](int mode, int size, int& desired, int minDim, int maxDim, int pad) {
        ConstraintWidget::DimensionBehaviour b = ConstraintWidget::DimensionBehaviour::FIXED;
        desired = 0;
        if (mode == View::MeasureSpec::EXACTLY) {
            desired = std::min(maxDim - pad, size);
        } else if (mode == View::MeasureSpec::AT_MOST) {
            b = ConstraintWidget::DimensionBehaviour::WRAP_CONTENT;
            desired = size;
        } else { // UNSPECIFIED
            b = ConstraintWidget::DimensionBehaviour::WRAP_CONTENT;
        }
        (void)minDim;
        return b;
    };
    int desiredW = 0, desiredH = 0;
    ConstraintWidget::DimensionBehaviour wb = behaviour(widthMode, widthSize, desiredW,
            mMinWidth, mMaxWidth, mPaddingWidth);
    ConstraintWidget::DimensionBehaviour hb = behaviour(heightMode, heightSize, desiredH,
            mMinHeight, mMaxHeight, mPaddingHeight);
    mLayoutWidget.setX(0);
    mLayoutWidget.setY(0);
    mLayoutWidget.setMaxWidth(mMaxWidth - mPaddingWidth);
    mLayoutWidget.setMaxHeight(mMaxHeight - mPaddingHeight);
    mLayoutWidget.setMinWidth(0);
    mLayoutWidget.setMinHeight(0);
    mLayoutWidget.setHorizontalDimensionBehaviour(wb);
    mLayoutWidget.setWidth(desiredW);
    mLayoutWidget.setVerticalDimensionBehaviour(hb);
    mLayoutWidget.setHeight(desiredH);
    mLayoutWidget.setMinWidth(mMinWidth);
    mLayoutWidget.setMinHeight(mMinHeight);
}

void ConstraintLayout::resolveMeasuredDimension(int widthSpec, int heightSpec,
        int measuredWidth, int measuredHeight) {
    int androidW = measuredWidth + mPaddingWidth;
    int androidH = measuredHeight + mPaddingHeight;
    int resolvedW = View::resolveSize(androidW, widthSpec);
    int resolvedH = View::resolveSize(androidH, heightSpec);
    resolvedW = std::min(resolvedW, mMaxWidth);
    resolvedH = std::min(resolvedH, mMaxHeight);
    setMeasuredDimension(resolvedW, resolvedH);
}

void ConstraintLayout::onLayout(bool /*changed*/, int l, int t, int width, int height) {
    const int count = getChildCount();
    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        auto* lp = dynamic_cast<LayoutParams*>(child->getLayoutParams());
        if (!lp || lp->mIsGuideline) continue; // guidelines don't render
        if (lp->mIsInPlaceholder) continue;    // positioned by its Placeholder, not here
        ConstraintWidget* w = getViewWidget(child);
        if (w == nullptr) continue;
        // The solver works in content coordinates (the container is sized to width-padding); offset
        // child positions by the padding so they land inside the padded frame.
        int x = w->getX() + getPaddingLeft();
        int y = w->getY() + getPaddingTop();
        int wWidth = w->getWidth(), wHeight = w->getHeight();

        if (auto* placeholder = dynamic_cast<Placeholder*>(child)) {
            // The placeholder's content view is drawn at the placeholder's frame.
            View* content = placeholder->getContent();
            if (content != nullptr) {
                content->setVisibility(View::VISIBLE);
                content->layout(x, y, wWidth, wHeight);
            }
            continue;
        }
        if (child->getVisibility() == View::GONE) continue;
        // CDROID View::layout(l, t, w, h) takes width/height (not right/bottom).
        child->layout(x, y, wWidth, wHeight);
    }
    // Helpers post-layout hook (Group zeroes its own widget here; Placeholder swaps content).
    for (ConstraintHelper* helper : mConstraintHelpers) {
        helper->updatePostLayout(this);
    }
}

} // namespace cdroid
