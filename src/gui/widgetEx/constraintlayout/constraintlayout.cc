/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.ConstraintLayout.
 * MVP cut — see header.
 */
#include <widgetEx/constraintlayout/constraintlayout.h>
#include <core/xmlpullparser.h>
#include <widgetEx/constraintlayout/constraintlayoutstates.h>

#include <algorithm>
#include <climits>

#include <widgetEx/constraintlayout/core/widgets/guideline.h>

#include <porting/cdlog.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <widget/textview.h>
#include <widgetEx/constraintlayout/constrainthelper.h>
#include <widgetEx/constraintlayout/placeholder.h>

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
    topToTop     = attrs.getResourceId("layout_constraintTop_toTopOf",     UNSET);
    topToBottom  = attrs.getResourceId("layout_constraintTop_toBottomOf",  UNSET);
    bottomToTop  = attrs.getResourceId("layout_constraintBottom_toTopOf",  UNSET);
    bottomToBottom = attrs.getResourceId("layout_constraintBottom_toBottomOf", UNSET);

    horizontalBias = attrs.getFloat("layout_constraintHorizontal_bias", 0.5f);
    verticalBias   = attrs.getFloat("layout_constraintVertical_bias",   0.5f);

    goneLeftMargin   = attrs.getDimensionPixelSize("layout_goneMarginLeft",   GONE_UNSET);
    goneTopMargin    = attrs.getDimensionPixelSize("layout_goneMarginTop",    GONE_UNSET);
    goneRightMargin  = attrs.getDimensionPixelSize("layout_goneMarginRight",  GONE_UNSET);
    goneBottomMargin = attrs.getDimensionPixelSize("layout_goneMarginBottom", GONE_UNSET);

    // Guideline
    guideBegin   = attrs.getDimensionPixelSize("layout_constraintGuide_begin", UNSET);
    guideEnd     = attrs.getDimensionPixelSize("layout_constraintGuide_end",   UNSET);
    guidePercent = attrs.getFloat("layout_constraintGuide_percent", UNSET_FLOAT);
    orientation  = attrs.getInt("android:orientation", -1);

    // Ratio (parse "16:9", "1.5", "W,16:9", "H,3:2")
    std::string ratioStr = attrs.getString("layout_constraintDimensionRatio", "");
    if (!ratioStr.empty()) {
        parseDimensionRatio(ratioStr, dimensionRatio, dimensionRatioSide);
    }

    // Baseline
    baselineToBaseline = attrs.getResourceId("layout_constraintBaseline_toBaselineOf", UNSET);

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
        // Guideline: orientation + begin/end/percent set in validate(). Guideline::addToSolver
        // (virtual override) handles positioning; skip normal anchor/dimension logic.
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

    // Left (leftToLeft preferred over leftToRight)
    if (lp->leftToLeft != LayoutParams::UNSET || lp->leftToRight != LayoutParams::UNSET) {
        bool toLeft = (lp->leftToLeft != LayoutParams::UNSET);
        int tid = toLeft ? lp->leftToLeft : lp->leftToRight;
        if (ConstraintWidget* t = resolveTarget(tid)) {
            widget->mLeft.connect(toLeft ? &t->mLeft : &t->mRight,
                                  lp->leftMargin, lp->goneLeftMargin, true);
        }
    }
    // Right
    if (lp->rightToLeft != LayoutParams::UNSET || lp->rightToRight != LayoutParams::UNSET) {
        bool toLeft = (lp->rightToLeft != LayoutParams::UNSET);
        int tid = toLeft ? lp->rightToLeft : lp->rightToRight;
        if (ConstraintWidget* t = resolveTarget(tid)) {
            widget->mRight.connect(toLeft ? &t->mLeft : &t->mRight,
                                   lp->rightMargin, lp->goneRightMargin, true);
        }
    }
    // Top
    if (lp->topToTop != LayoutParams::UNSET || lp->topToBottom != LayoutParams::UNSET) {
        bool toTop = (lp->topToTop != LayoutParams::UNSET);
        int tid = toTop ? lp->topToTop : lp->topToBottom;
        if (ConstraintWidget* t = resolveTarget(tid)) {
            widget->mTop.connect(toTop ? &t->mTop : &t->mBottom,
                                 lp->topMargin, lp->goneTopMargin, true);
        }
    }
    // Bottom
    if (lp->bottomToTop != LayoutParams::UNSET || lp->bottomToBottom != LayoutParams::UNSET) {
        bool toTop = (lp->bottomToTop != LayoutParams::UNSET);
        int tid = toTop ? lp->bottomToTop : lp->bottomToBottom;
        if (ConstraintWidget* t = resolveTarget(tid)) {
            widget->mBottom.connect(toTop ? &t->mTop : &t->mBottom,
                                    lp->bottomMargin, lp->goneBottomMargin, true);
        }
    }

    widget->mHorizontalBiasPercent = lp->horizontalBias;
    widget->mVerticalBiasPercent = lp->verticalBias;

    // Dimension behaviour
    if (lp->mHorizontalDimensionFixed) {
        widget->setHorizontalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::FIXED);
        widget->setWidth(lp->width);
        if (lp->width == LayoutParams::WRAP_CONTENT) {
            widget->setHorizontalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::WRAP_CONTENT);
        }
    } else {
        // MATCH_CONSTRAINT (0dp) / MATCH_PARENT. Apply the match-style from LayoutParams
        // (Android: setHorizontalMatchStyle). The solver's applyConstraints handles SPREAD/WRAP/PERCENT.
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

    // Baseline constraint (overrides top/bottom)
    if (lp->baselineToBaseline != LayoutParams::UNSET) {
        if (ConstraintWidget* t = resolveTarget(lp->baselineToBaseline)) {
            widget->mBaseline.connect(&t->mBaseline, 0, LayoutParams::GONE_UNSET, true);
            widget->setHasBaseline(true);
            t->setHasBaseline(true);
            widget->mTop.reset();
            widget->mBottom.reset();
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
        // MATCH_CONSTRAINT (0dp) — MVP: measure as wrap to get a base size.
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
    // MVP: no Placeholder/ConstraintHelper post-measure hooks.
}

void ConstraintLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    setChildrenConstraints();
    resolveSystem(widthMeasureSpec, heightMeasureSpec);
    // Placeholders adopt their content's resolved size post-solve (single pass; the Java re-measure
    // loop is deferred, so this runs once after the linear solve).
    const int count = getChildCount();
    for (int i = 0; i < count; i++) {
        if (auto* placeholder = dynamic_cast<Placeholder*>(getChildAt(i))) {
            placeholder->updatePostMeasure(this);
        }
    }
    resolveMeasuredDimension(widthMeasureSpec, heightMeasureSpec,
                             mLayoutWidget.getWidth(), mLayoutWidget.getHeight());
}

void ConstraintLayout::resolveSystem(int widthSpec, int heightSpec) {
    mWidthSpec = widthSpec;
    mHeightSpec = heightSpec;
    int paddingLeft = getPaddingLeft(), paddingRight = getPaddingRight();
    int paddingTop = getPaddingTop(), paddingBottom = getPaddingBottom();
    mPaddingWidth = paddingLeft + paddingRight;
    mPaddingHeight = paddingTop + paddingBottom;

    int widthMode = View::MeasureSpec::getMode(widthSpec);
    int heightMode = View::MeasureSpec::getMode(heightSpec);
    int widthSize = View::MeasureSpec::getSize(widthSpec) - mPaddingWidth;
    int heightSize = View::MeasureSpec::getSize(heightSpec) - mPaddingHeight;

    setSelfDimensionBehaviour(widthMode, widthSize, heightMode, heightSize);

    // MVP: optimization off (linear solve only). padding offset (paddingLeft/paddingTop) is not
    // forwarded to the solver here — the MVP driver sites the container at (0,0); samples use no
    // padding. TODO: pass padding so children come out parent-relative.
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

void ConstraintLayout::onLayout(bool /*changed*/, int /*l*/, int /*t*/, int /*r*/, int /*b*/) {
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
