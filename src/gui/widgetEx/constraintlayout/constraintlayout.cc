/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.ConstraintLayout.
 * MVP cut — see header.
 */
#include <widgetEx/constraintlayout/constraintlayout.h>

#include <climits>

#include <porting/cdlog.h>
#include <view/view.h>
#include <widget/textview.h>

DECLARE_WIDGET(ConstraintLayout)

namespace cdroid {

// out-of-line definition (PARENT_ID is odr-used as a map key)
constexpr int ConstraintLayout::PARENT_ID;

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

    validate();
}

ConstraintLayout::LayoutParams::LayoutParams(int width, int height)
    : MarginLayoutParams(width, height) {
    validate();
}

// FIXED/WRAP_CONTENT -> dimension fixed; MATCH_CONSTRAINT(0dp)/MATCH_PARENT -> variable.
void ConstraintLayout::LayoutParams::validate() {
    mHorizontalDimensionFixed = (width != 0 && width != LayoutParams::MATCH_PARENT);
    mVerticalDimensionFixed   = (height != 0 && height != LayoutParams::MATCH_PARENT);
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
}

ConstraintLayout::ConstraintLayout(int width, int height)
    : ViewGroup(width, height) {
    mLayoutWidget.setMeasurer(asMeasurer());
    mLayoutWidget.setCompanionWidget(this);
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
    auto* lp = dynamic_cast<LayoutParams*>(view->getLayoutParams());
    return lp ? &lp->mWidget : nullptr;
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
    widget->setVisibility(child->getVisibility());
    widget->setCompanionWidget(child);

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
        // MATCH_CONSTRAINT (0dp) / MATCH_PARENT — MVP: treat as MATCH_CONSTRAINT
        widget->setHorizontalDimensionBehaviour(ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT);
        widget->setWidth(0);
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
    }
}

// --- BasicMeasure::Measurer ---
void ConstraintLayout::measure(ConstraintWidget* widget, BasicMeasure::Measure* m) {
    if (widget->getVisibility() == ConstraintWidget::GONE) {
        m->measuredWidth = 0; m->measuredHeight = 0; m->measuredBaseline = 0;
        m->measuredHasBaseline = false; m->measuredNeedsSolverPass = false;
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
        if (child->getVisibility() == View::GONE) continue;
        auto* lp = static_cast<LayoutParams*>(child->getLayoutParams());
        ConstraintWidget* w = &lp->mWidget;
        // CDROID View::layout(l, t, w, h) takes width/height (not right/bottom).
        child->layout(w->getX(), w->getY(), w->getWidth(), w->getHeight());
    }
}

} // namespace cdroid
