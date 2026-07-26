/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.ConstraintSet.
 */
#include <widgetEx/constraintlayout/constraintset.h>

#include <climits>

#include <porting/cdlog.h>
#include <view/view.h>

namespace cdroid {

namespace {
// Parse a ratio string ("16:9", "1.5", "W,16:9") into a float ratio (0 if unparseable).
float parseRatio(const std::string& s) {
    if (s.empty()) return 0;
    std::string str = s;
    if (str.size() > 2 && str[1] == ',') str = str.substr(2); // strip "W,"/"H," side hint
    size_t colon = str.find(':');
    if (colon != std::string::npos) {
        float num = std::stof(str.substr(0, colon));
        float den = std::stof(str.substr(colon + 1));
        return (den != 0) ? num / den : 0;
    }
    return std::stof(str);
}
} // namespace

// ===========================================================================
// Constraint
// ===========================================================================
void ConstraintSet::Constraint::fillFrom(int viewId, const ConstraintLayout::LayoutParams& param) {
    mViewId = viewId;
    Layout& l = layout;
    l.leftToLeft = param.leftToLeft;     l.leftToRight = param.leftToRight;
    l.rightToLeft = param.rightToLeft;   l.rightToRight = param.rightToRight;
    l.topToTop = param.topToTop;         l.topToBottom = param.topToBottom;
    l.bottomToTop = param.bottomToTop;   l.bottomToBottom = param.bottomToBottom;
    l.baselineToBaseline = param.baselineToBaseline;
    l.horizontalBias = param.horizontalBias; l.verticalBias = param.verticalBias;
    l.orientation = param.orientation;
    l.guideBegin = param.guideBegin; l.guideEnd = param.guideEnd; l.guidePercent = param.guidePercent;
    l.mWidth = param.width; l.mHeight = param.height;
    l.leftMargin = param.leftMargin; l.rightMargin = param.rightMargin;
    l.topMargin = param.topMargin;   l.bottomMargin = param.bottomMargin;
    l.goneLeftMargin = param.goneLeftMargin; l.goneTopMargin = param.goneTopMargin;
    l.goneRightMargin = param.goneRightMargin; l.goneBottomMargin = param.goneBottomMargin;
    l.verticalWeight = param.verticalWeight; l.horizontalWeight = param.horizontalWeight;
    l.verticalChainStyle = param.verticalChainStyle; l.horizontalChainStyle = param.horizontalChainStyle;
    l.widthDefault = param.matchConstraintDefaultWidth; l.heightDefault = param.matchConstraintDefaultHeight;
    l.widthMax = param.matchConstraintMaxWidth; l.heightMax = param.matchConstraintMaxHeight;
    l.widthMin = param.matchConstraintMinWidth; l.heightMin = param.matchConstraintMinHeight;
    l.widthPercent = param.matchConstraintPercentWidth; l.heightPercent = param.matchConstraintPercentHeight;
}

void ConstraintSet::Constraint::applyTo(ConstraintLayout::LayoutParams& param) const {
    const Layout& l = layout;
    param.leftToLeft = l.leftToLeft;     param.leftToRight = l.leftToRight;
    param.rightToLeft = l.rightToLeft;   param.rightToRight = l.rightToRight;
    param.topToTop = l.topToTop;         param.topToBottom = l.topToBottom;
    param.bottomToTop = l.bottomToTop;   param.bottomToBottom = l.bottomToBottom;
    param.baselineToBaseline = l.baselineToBaseline;
    param.horizontalBias = l.horizontalBias; param.verticalBias = l.verticalBias;
    param.orientation = l.orientation;
    param.guideBegin = l.guideBegin; param.guideEnd = l.guideEnd; param.guidePercent = l.guidePercent;
    param.width = l.mWidth; param.height = l.mHeight;
    param.leftMargin = l.leftMargin; param.rightMargin = l.rightMargin;
    param.topMargin = l.topMargin;   param.bottomMargin = l.bottomMargin;
    param.goneLeftMargin = l.goneLeftMargin; param.goneTopMargin = l.goneTopMargin;
    param.goneRightMargin = l.goneRightMargin; param.goneBottomMargin = l.goneBottomMargin;
    param.verticalWeight = l.verticalWeight; param.horizontalWeight = l.horizontalWeight;
    param.verticalChainStyle = l.verticalChainStyle; param.horizontalChainStyle = l.horizontalChainStyle;
    param.matchConstraintDefaultWidth = l.widthDefault; param.matchConstraintDefaultHeight = l.heightDefault;
    param.matchConstraintMaxWidth = l.widthMax; param.matchConstraintMaxHeight = l.heightMax;
    param.matchConstraintMinWidth = l.widthMin; param.matchConstraintMinHeight = l.heightMin;
    param.matchConstraintPercentWidth = l.widthPercent; param.matchConstraintPercentHeight = l.heightPercent;
    if (!l.dimensionRatio.empty()) {
        param.dimensionRatio = parseRatio(l.dimensionRatio);
    }
    if (l.visibility == (int)View::GONE) {
        param.width = 0; param.height = 0;
    }
    param.validate();
}

// ===========================================================================
// ConstraintSet
// ===========================================================================
ConstraintSet::Constraint& ConstraintSet::get(int id) {
    return mConstraints[id]; // default-constructs a Constraint if absent
}

void ConstraintSet::clone(ConstraintLayout* constraintLayout) {
    mConstraints.clear();
    const int count = constraintLayout->getChildCount();
    for (int i = 0; i < count; i++) {
        View* view = constraintLayout->getChildAt(i);
        int id = view->getId();
        auto* param = dynamic_cast<ConstraintLayout::LayoutParams*>(view->getLayoutParams());
        if (param == nullptr) continue;
        Constraint& c = mConstraints[id];
        c.fillFrom(id, *param);
        c.layout.visibility = view->getVisibility();
        c.layout.alpha = view->getAlpha();
        c.transform.rotation = view->getRotation();
        c.transform.rotationX = view->getRotationX();
        c.transform.rotationY = view->getRotationY();
        c.transform.scaleX = view->getScaleX();
        c.transform.scaleY = view->getScaleY();
        c.transform.translationX = view->getTranslationX();
        c.transform.translationY = view->getTranslationY();
    }
}

void ConstraintSet::applyTo(ConstraintLayout* constraintLayout) {
    const int count = constraintLayout->getChildCount();
    for (int i = 0; i < count; i++) {
        View* view = constraintLayout->getChildAt(i);
        int id = view->getId();
        auto it = mConstraints.find(id);
        if (it == mConstraints.end()) continue;
        auto* param = dynamic_cast<ConstraintLayout::LayoutParams*>(view->getLayoutParams());
        if (param == nullptr) continue;
        it->second.applyTo(*param);
        view->setLayoutParams(param);
        const Constraint& c = it->second;
        view->setVisibility(c.layout.visibility);
        // Apply view transforms only when non-identity — the render-node invalidation path used by
        // the setters is unsafe on unattached views, and re-applying identity is a no-op anyway.
        if (c.layout.alpha != 1.0f) view->setAlpha(c.layout.alpha);
        if (c.transform.rotation != 0)      view->setRotation(c.transform.rotation);
        if (c.transform.rotationX != 0)     view->setRotationX(c.transform.rotationX);
        if (c.transform.rotationY != 0)     view->setRotationY(c.transform.rotationY);
        if (c.transform.scaleX != 1)        view->setScaleX(c.transform.scaleX);
        if (c.transform.scaleY != 1)        view->setScaleY(c.transform.scaleY);
        if (c.transform.translationX != 0)  view->setTranslationX(c.transform.translationX);
        if (c.transform.translationY != 0)  view->setTranslationY(c.transform.translationY);
    }
    constraintLayout->requestLayout();
}

void ConstraintSet::clear(int viewId) {
    mConstraints.erase(viewId);
}

void ConstraintSet::clear(int viewId, int anchor) {
    auto it = mConstraints.find(viewId);
    if (it == mConstraints.end()) return;
    Layout& l = it->second.layout;
    const int UNSET = -1;
    switch (anchor) {
        case LEFT:   l.leftToLeft = UNSET; l.leftToRight = UNSET; break;
        case RIGHT:  l.rightToLeft = UNSET; l.rightToRight = UNSET; break;
        case TOP:    l.topToTop = UNSET; l.topToBottom = UNSET; break;
        case BOTTOM: l.bottomToTop = UNSET; l.bottomToBottom = UNSET; break;
        case BASELINE: l.baselineToBaseline = UNSET; break;
        default: break;
    }
}

void ConstraintSet::connect(int startID, int startSide, int endID, int endSide, int margin) {
    Layout& l = get(startID).layout;
    switch (startSide) {
        case LEFT:
            if (endSide == RIGHT) l.leftToRight = endID; else l.leftToLeft = endID;
            l.leftMargin = margin; break;
        case RIGHT:
            if (endSide == LEFT) l.rightToLeft = endID; else l.rightToRight = endID;
            l.rightMargin = margin; break;
        case TOP:
            if (endSide == BOTTOM) l.topToBottom = endID; else l.topToTop = endID;
            l.topMargin = margin; break;
        case BOTTOM:
            if (endSide == TOP) l.bottomToTop = endID; else l.bottomToBottom = endID;
            l.bottomMargin = margin; break;
        case BASELINE: l.baselineToBaseline = endID; break;
        default:
            LOGW("ConstraintSet.connect: unsupported side %d", startSide);
            break;
    }
}

void ConstraintSet::connect(int startID, int startSide, int endID, int endSide) {
    connect(startID, startSide, endID, endSide, 0);
}

void ConstraintSet::constrainWidth(int viewId, int width)  { get(viewId).layout.mWidth = width; }
void ConstraintSet::constrainHeight(int viewId, int height){ get(viewId).layout.mHeight = height; }
void ConstraintSet::setVisibility(int viewId, int visibility) { get(viewId).layout.visibility = visibility; }

void ConstraintSet::setMargin(int viewId, int anchor, int value) {
    Layout& l = get(viewId).layout;
    switch (anchor) {
        case LEFT:   l.leftMargin = value; break;
        case RIGHT:  l.rightMargin = value; break;
        case TOP:    l.topMargin = value; break;
        case BOTTOM: l.bottomMargin = value; break;
        default: break;
    }
}

void ConstraintSet::setDimensionRatio(int viewId, const std::string& ratio) {
    get(viewId).layout.dimensionRatio = ratio;
}

void ConstraintSet::setAlpha(int viewId, float alpha) { get(viewId).layout.alpha = alpha; }
void ConstraintSet::setRotation(int viewId, float rotation) { get(viewId).transform.rotation = rotation; }
void ConstraintSet::setRotationX(int viewId, float rotationX) { get(viewId).transform.rotationX = rotationX; }
void ConstraintSet::setRotationY(int viewId, float rotationY) { get(viewId).transform.rotationY = rotationY; }
void ConstraintSet::setScaleX(int viewId, float scaleX) { get(viewId).transform.scaleX = scaleX; }
void ConstraintSet::setScaleY(int viewId, float scaleY) { get(viewId).transform.scaleY = scaleY; }
void ConstraintSet::setTranslationX(int viewId, float translationX) { get(viewId).transform.translationX = translationX; }
void ConstraintSet::setTranslationY(int viewId, float translationY) { get(viewId).transform.translationY = translationY; }

} // namespace cdroid
