/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.ConstraintSet.
 */

#include <climits>
#include <cctype>

#include <core/xmlpullparser.h>
#include <porting/cdlog.h>
#include <view/view.h>
#include <widgetEx/constraintlayout/constraintset.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>

namespace cdroid {

namespace {
// Parse a <CustomAttribute> element into a typed CustomAttribute: attributeName + one of
// customColorValue/customIntegerValue/customFloatValue/customStringValue/customBooleanValue.
ConstraintSet::CustomAttribute parseCustomAttribute(const AttributeSet& parser) {
    using CustomAttribute = ConstraintSet::CustomAttribute; // alias: a using-declaration can't import a nested type into block scope
    ConstraintSet::CustomAttribute ca;
    ca.name = parser.getAttributeValue("attributeName");
    std::string v;
    if (!(v = parser.getAttributeValue("customColorValue")).empty()) {
        ca.type = CustomAttribute::COLOR;
        ca.intValue = parser.getColor("customColorValue", 0);
    } else if (!(v = parser.getAttributeValue("customIntegerValue")).empty()) {
        ca.type = CustomAttribute::INTEGER;
        ca.intValue = std::stoi(v);
    } else if (!(v = parser.getAttributeValue("customFloatValue")).empty()) {
        ca.type = CustomAttribute::FLOAT;
        ca.floatValue = std::stof(v);
    } else if (!(v = parser.getAttributeValue("customStringValue")).empty()) {
        ca.type = CustomAttribute::STRING;
        ca.stringValue = v;
    } else if (!(v = parser.getAttributeValue("customBooleanValue")).empty()) {
        ca.type = CustomAttribute::BOOLEAN;
        ca.boolValue = (v == "true" || v == "1");
    }
    return ca;
}

// Registry of named custom-attribute handlers (function-local static avoids init-order issues).
std::unordered_map<std::string, ConstraintSet::CustomAttributeHandler>& customHandlers() {
    static std::unordered_map<std::string, ConstraintSet::CustomAttributeHandler> handlers;
    return handlers;
}

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
    l.leftToLeft = param.leftToLeft;
    l.leftToRight = param.leftToRight;
    l.rightToLeft = param.rightToLeft;
    l.rightToRight = param.rightToRight;
    l.topToTop = param.topToTop;
    l.topToBottom = param.topToBottom;
    l.bottomToTop = param.bottomToTop;
    l.bottomToBottom = param.bottomToBottom;
    l.baselineToBaseline = param.baselineToBaseline;
    l.horizontalBias = param.horizontalBias;
    l.verticalBias = param.verticalBias;
    l.orientation = param.orientation;
    l.guideBegin = param.guideBegin;
    l.guideEnd = param.guideEnd;
    l.guidePercent = param.guidePercent;
    l.mWidth = param.width;
    l.mHeight = param.height;
    l.leftMargin = param.leftMargin;
    l.rightMargin = param.rightMargin;
    l.topMargin = param.topMargin;
    l.bottomMargin = param.bottomMargin;
    l.goneLeftMargin = param.goneLeftMargin;
    l.goneTopMargin = param.goneTopMargin;
    l.goneRightMargin = param.goneRightMargin;
    l.goneBottomMargin = param.goneBottomMargin;
    l.verticalWeight = param.verticalWeight;
    l.horizontalWeight = param.horizontalWeight;
    l.verticalChainStyle = param.verticalChainStyle;
    l.horizontalChainStyle = param.horizontalChainStyle;
    l.widthDefault = param.matchConstraintDefaultWidth;
    l.heightDefault = param.matchConstraintDefaultHeight;
    l.widthMax = param.matchConstraintMaxWidth;
    l.heightMax = param.matchConstraintMaxHeight;
    l.widthMin = param.matchConstraintMinWidth;
    l.heightMin = param.matchConstraintMinHeight;
    l.widthPercent = param.matchConstraintPercentWidth;
    l.heightPercent = param.matchConstraintPercentHeight;
}

void ConstraintSet::Constraint::applyTo(ConstraintLayout::LayoutParams& param) const {
    const Layout& l = layout;
    param.leftToLeft = l.leftToLeft;
    param.leftToRight = l.leftToRight;
    param.rightToLeft = l.rightToLeft;
    param.rightToRight = l.rightToRight;
    param.topToTop = l.topToTop;
    param.topToBottom = l.topToBottom;
    param.bottomToTop = l.bottomToTop;
    param.bottomToBottom = l.bottomToBottom;
    param.baselineToBaseline = l.baselineToBaseline;
    param.horizontalBias = l.horizontalBias;
    param.verticalBias = l.verticalBias;
    param.orientation = l.orientation;
    param.guideBegin = l.guideBegin;
    param.guideEnd = l.guideEnd;
    param.guidePercent = l.guidePercent;
    param.width = l.mWidth;
    param.height = l.mHeight;
    param.leftMargin = l.leftMargin;
    param.rightMargin = l.rightMargin;
    param.topMargin = l.topMargin;
    param.bottomMargin = l.bottomMargin;
    param.goneLeftMargin = l.goneLeftMargin;
    param.goneTopMargin = l.goneTopMargin;
    param.goneRightMargin = l.goneRightMargin;
    param.goneBottomMargin = l.goneBottomMargin;
    param.verticalWeight = l.verticalWeight;
    param.horizontalWeight = l.horizontalWeight;
    param.verticalChainStyle = l.verticalChainStyle;
    param.horizontalChainStyle = l.horizontalChainStyle;
    param.matchConstraintDefaultWidth = l.widthDefault;
    param.matchConstraintDefaultHeight = l.heightDefault;
    param.matchConstraintMaxWidth = l.widthMax;
    param.matchConstraintMaxHeight = l.heightMax;
    param.matchConstraintMinWidth = l.widthMin;
    param.matchConstraintMinHeight = l.heightMin;
    param.matchConstraintPercentWidth = l.widthPercent;
    param.matchConstraintPercentHeight = l.heightPercent;
    if (!l.dimensionRatio.empty()) {
        param.dimensionRatio = parseRatio(l.dimensionRatio);
    }
    if (propertySet.visibility == (int)View::GONE) {
        param.width = 0;
        param.height = 0;
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
        c.propertySet.visibility = view->getVisibility();
        c.propertySet.alpha = view->getAlpha();
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
        view->setVisibility(c.propertySet.visibility);
        // Attached views always get the transform (so identity resets it — e.g. a Motion run that
        // left the view rotated is cleared when the start/end ConstraintSet is re-applied on
        // capture). Unattached views (tests) skip identity: the setter invalidation is unsafe.
        const bool attached = view->isAttachedToWindow();
        auto apply = [&](float val, float identity, void (View::*setter)(float)) {
            if (attached || val != identity) (view->*setter)(val);
        };
        apply(c.propertySet.alpha,      1.0f, &View::setAlpha);
        apply(c.transform.rotation,     0.0f, &View::setRotation);
        apply(c.transform.rotationX,    0.0f, &View::setRotationX);
        apply(c.transform.rotationY,    0.0f, &View::setRotationY);
        apply(c.transform.scaleX,       1.0f, &View::setScaleX);
        apply(c.transform.scaleY,       1.0f, &View::setScaleY);
        apply(c.transform.translationX, 0.0f, &View::setTranslationX);
        apply(c.transform.translationY, 0.0f, &View::setTranslationY);
        // Dispatch parsed <CustomAttribute>s to externally-registered handlers (extension point;
        // the framework binds nothing — no hardcoded attributes, no View modification).
        for (const auto& ca : c.mCustomAttributes) {
            auto it = customHandlers().find(ca.name);
            if (it != customHandlers().end()) it->second(view, ca);
        }
    }
    constraintLayout->requestLayout();
}

void ConstraintSet::mergeFrom(const ConstraintSet& base) {
    // Copy base's per-view constraints; emplace keeps this set's own entries (derived wins).
    for (const auto& kv : base.mConstraints) {
        mConstraints.emplace(kv.first, kv.second);
    }
}

void ConstraintSet::registerCustomAttributeHandler(const std::string& name, CustomAttributeHandler handler) {
    customHandlers()[name] = std::move(handler);
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
    case LEFT:
        l.leftToLeft = UNSET;
        l.leftToRight = UNSET;
        break;
    case RIGHT:
        l.rightToLeft = UNSET;
        l.rightToRight = UNSET;
        break;
    case TOP:
        l.topToTop = UNSET;
        l.topToBottom = UNSET;
        break;
    case BOTTOM:
        l.bottomToTop = UNSET;
        l.bottomToBottom = UNSET;
        break;
    case BASELINE:
        l.baselineToBaseline = UNSET;
        break;
    default:
        break;
    }
}

void ConstraintSet::connect(int startID, int startSide, int endID, int endSide, int margin) {
    Layout& l = get(startID).layout;
    switch (startSide) {
    case LEFT:
        if (endSide == RIGHT) l.leftToRight = endID;
        else l.leftToLeft = endID;
        l.leftMargin = margin;
        break;
    case RIGHT:
        if (endSide == LEFT) l.rightToLeft = endID;
        else l.rightToRight = endID;
        l.rightMargin = margin;
        break;
    case TOP:
        if (endSide == BOTTOM) l.topToBottom = endID;
        else l.topToTop = endID;
        l.topMargin = margin;
        break;
    case BOTTOM:
        if (endSide == TOP) l.bottomToTop = endID;
        else l.bottomToBottom = endID;
        l.bottomMargin = margin;
        break;
    case BASELINE:
        l.baselineToBaseline = endID;
        break;
    default:
        LOGW("ConstraintSet.connect: unsupported side %d", startSide);
        break;
    }
}

void ConstraintSet::connect(int startID, int startSide, int endID, int endSide) {
    connect(startID, startSide, endID, endSide, 0);
}

void ConstraintSet::constrainWidth(int viewId, int width)  {
    get(viewId).layout.mWidth = width;
}

void ConstraintSet::constrainHeight(int viewId, int height) {
    get(viewId).layout.mHeight = height;
}

void ConstraintSet::setVisibility(int viewId, int visibility) {
    get(viewId).propertySet.visibility = visibility;
}

void ConstraintSet::setMargin(int viewId, int anchor, int value) {
    Layout& l = get(viewId).layout;
    switch (anchor) {
    case LEFT:
        l.leftMargin = value;
        break;
    case RIGHT:
        l.rightMargin = value;
        break;
    case TOP:
        l.topMargin = value;
        break;
    case BOTTOM:
        l.bottomMargin = value;
        break;
    default:
        break;
    }
}

void ConstraintSet::setDimensionRatio(int viewId, const std::string& ratio) {
    get(viewId).layout.dimensionRatio = ratio;
}

void ConstraintSet::setAlpha(int viewId, float alpha) {
    get(viewId).propertySet.alpha = alpha;
}

void ConstraintSet::setRotation(int viewId, float rotation) {
    get(viewId).transform.rotation = rotation;
}

void ConstraintSet::setRotationX(int viewId, float rotationX) {
    get(viewId).transform.rotationX = rotationX;
}

void ConstraintSet::setRotationY(int viewId, float rotationY) {
    get(viewId).transform.rotationY = rotationY;
}

void ConstraintSet::setScaleX(int viewId, float scaleX) {
    get(viewId).transform.scaleX = scaleX;
}

void ConstraintSet::setScaleY(int viewId, float scaleY) {
    get(viewId).transform.scaleY = scaleY;
}

void ConstraintSet::setTranslationX(int viewId, float translationX) {
    get(viewId).transform.translationX = translationX;
}

void ConstraintSet::setTranslationY(int viewId, float translationY) {
    get(viewId).transform.translationY = translationY;
}

// ===========================================================================
// XML loading — Constraint::fillFromAttributeList (Java populateConstraint) +
// ConstraintSet::load(Context, XmlPullParser).
//
// CDROID uses a name-keyed AttributeSet (the XmlPullParser IS-A AttributeSet), so the Java
// TypedArray + styleable-int switch becomes a sequence of name lookups: each attribute is read by
// name with the field's current value as default, so unset attributes are no-ops (equivalent to
// Java iterating only the present attrs).
// ===========================================================================

// Enum-name -> int maps for the enum-valued attributes (mirrors attrs.xml enum values).
static const std::unordered_map<std::string,int> kChainStyles = {
    {"spread", ConstraintWidget::CHAIN_SPREAD},
    {"spread_inside", ConstraintWidget::CHAIN_SPREAD_INSIDE},
    {"packed", ConstraintWidget::CHAIN_PACKED}
};
static const std::unordered_map<std::string,int> kMatchDefault = {
    {"spread",  ConstraintWidget::MATCH_CONSTRAINT_SPREAD},
    {"wrap",    ConstraintWidget::MATCH_CONSTRAINT_WRAP},
    {"percent", ConstraintWidget::MATCH_CONSTRAINT_PERCENT}
};
static const std::unordered_map<std::string,int> kVisibility = {
    {"visible",   0}, {"invisible", 4}, {"gone", 8}
};
static const std::unordered_map<std::string,int> kPathMotionArc = {
    {"none", 0}, {"startVertical", 1}, {"startHorizontal", 2},
    {"flip", 3}, {"below", 4}, {"above", 5}
};
static const std::unordered_map<std::string,int> kBarrierDirection = {
    {"left", 0}, {"right", 1}, {"top", 2}, {"bottom", 3}, {"start", 5}, {"end", 6}
};
static const std::unordered_map<std::string,int> kWrapBehavior = {
    {"included", 0}, {"horizontal_only", 1}, {"vertical_only", 2}, {"skipped", 3}
};
static const std::unordered_map<std::string,int> kVisibilityMode = {
    {"normal", 0}, {"ignore", 1}
};

void ConstraintSet::Constraint::fillFromAttributeList(const AttributeSet& a) {
    // Any parsed attribute marks these sub-structs as authored (Java sets mApply on each present attr).
    layout.mApply = transform.mApply = propertySet.mApply = motion.mApply = true;

    Layout& l = layout;
    Transform& t = transform;
    PropertySet& p = propertySet;
    Motion& m = motion;

    // --- id + anchor targets (resolve "parent"/"@id/x" -> int via Context) ---
    mViewId      = a.getResourceId("id", mViewId);
    l.leftToLeft   = a.getResourceId("layout_constraintLeft_toLeftOf",   l.leftToLeft);
    l.leftToRight  = a.getResourceId("layout_constraintLeft_toRightOf",  l.leftToRight);
    l.rightToLeft  = a.getResourceId("layout_constraintRight_toLeftOf",  l.rightToLeft);
    l.rightToRight = a.getResourceId("layout_constraintRight_toRightOf", l.rightToRight);
    l.topToTop     = a.getResourceId("layout_constraintTop_toTopOf",     l.topToTop);
    l.topToBottom  = a.getResourceId("layout_constraintTop_toBottomOf",  l.topToBottom);
    l.bottomToTop  = a.getResourceId("layout_constraintBottom_toTopOf",  l.bottomToTop);
    l.bottomToBottom = a.getResourceId("layout_constraintBottom_toBottomOf", l.bottomToBottom);
    l.baselineToBaseline = a.getResourceId("layout_constraintBaseline_toBaselineOf", l.baselineToBaseline);
    l.baselineToTop    = a.getResourceId("layout_constraintBaseline_toTopOf",    l.baselineToTop);
    l.baselineToBottom = a.getResourceId("layout_constraintBaseline_toBottomOf", l.baselineToBottom);
    l.startToStart = a.getResourceId("layout_constraintStart_toStartOf", l.startToStart);
    l.startToEnd   = a.getResourceId("layout_constraintStart_toEndOf",   l.startToEnd);
    l.endToStart   = a.getResourceId("layout_constraintEnd_toStartOf",   l.endToStart);
    l.endToEnd     = a.getResourceId("layout_constraintEnd_toEndOf",     l.endToEnd);
    l.circleConstraint = a.getResourceId("layout_constraintCircle", l.circleConstraint);

    // --- guideline / editor absolute ---
    l.guideBegin   = a.getDimensionPixelOffset("layout_constraintGuide_begin", l.guideBegin);
    l.guideEnd     = a.getDimensionPixelOffset("layout_constraintGuide_end",   l.guideEnd);
    l.guidePercent = a.getFloat("layout_constraintGuide_percent", l.guidePercent);
    l.editorAbsoluteX = a.getDimensionPixelOffset("layout_editor_absoluteX", l.editorAbsoluteX);
    l.editorAbsoluteY = a.getDimensionPixelOffset("layout_editor_absoluteY", l.editorAbsoluteY);
    l.orientation     = a.getInt("orientation", l.orientation);

    // --- margins ---
    l.leftMargin   = a.getDimensionPixelSize("layout_marginLeft",  l.leftMargin);
    l.rightMargin  = a.getDimensionPixelSize("layout_marginRight", l.rightMargin);
    l.topMargin    = a.getDimensionPixelSize("layout_marginTop",   l.topMargin);
    l.bottomMargin = a.getDimensionPixelSize("layout_marginBottom",l.bottomMargin);
    l.startMargin  = a.getDimensionPixelSize("layout_marginStart", l.startMargin);
    l.endMargin    = a.getDimensionPixelSize("layout_marginEnd",   l.endMargin);
    l.goneLeftMargin   = a.getDimensionPixelSize("layout_goneMarginLeft",   l.goneLeftMargin);
    l.goneTopMargin    = a.getDimensionPixelSize("layout_goneMarginTop",    l.goneTopMargin);
    l.goneRightMargin  = a.getDimensionPixelSize("layout_goneMarginRight",  l.goneRightMargin);
    l.goneBottomMargin = a.getDimensionPixelSize("layout_goneMarginBottom", l.goneBottomMargin);
    l.goneStartMargin  = a.getDimensionPixelSize("layout_goneMarginStart",  l.goneStartMargin);
    l.goneEndMargin    = a.getDimensionPixelSize("layout_goneMarginEnd",    l.goneEndMargin);

    // --- bias / chain / weight / ratio ---
    l.horizontalBias = a.getFloat("layout_constraintHorizontal_bias", l.horizontalBias);
    l.verticalBias   = a.getFloat("layout_constraintVertical_bias",   l.verticalBias);
    l.horizontalWeight = a.getFloat("layout_constraintHorizontal_weight", l.horizontalWeight);
    l.verticalWeight   = a.getFloat("layout_constraintVertical_weight",   l.verticalWeight);
    l.horizontalChainStyle = a.getInt("layout_constraintHorizontal_chainStyle", kChainStyles, l.horizontalChainStyle);
    l.verticalChainStyle   = a.getInt("layout_constraintVertical_chainStyle",   kChainStyles, l.verticalChainStyle);
    l.dimensionRatio = a.getString("layout_constraintDimensionRatio", l.dimensionRatio);

    // --- dimensions / match_constraint ---
    l.mWidth  = a.getLayoutDimension("layout_width",  l.mWidth);
    l.mHeight = a.getLayoutDimension("layout_height", l.mHeight);
    l.widthDefault  = a.getInt("layout_constraintWidth_default",  kMatchDefault, l.widthDefault);
    l.heightDefault = a.getInt("layout_constraintHeight_default", kMatchDefault, l.heightDefault);
    l.widthPercent  = a.getFloat("layout_constraintWidth_percent",  l.widthPercent);
    l.heightPercent = a.getFloat("layout_constraintHeight_percent", l.heightPercent);
    l.widthMin  = a.getDimensionPixelSize("layout_constraintWidth_min",  l.widthMin);
    l.widthMax  = a.getDimensionPixelSize("layout_constraintWidth_max",  l.widthMax);
    l.heightMin = a.getDimensionPixelSize("layout_constraintHeight_min", l.heightMin);
    l.heightMax = a.getDimensionPixelSize("layout_constraintHeight_max", l.heightMax);
    l.constrainedWidth  = a.getBoolean("layout_constrainedWidth",  l.constrainedWidth);
    l.constrainedHeight = a.getBoolean("layout_constrainedHeight", l.constrainedHeight);
    l.mWrapBehavior = a.getInt("layout_wrapBehaviorInParent", kWrapBehavior, l.mWrapBehavior);

    // --- circle / barrier / helper ---
    l.circleRadius = a.getDimensionPixelSize("layout_constraintCircleRadius", l.circleRadius);
    l.circleAngle  = a.getFloat("layout_constraintCircleAngle", l.circleAngle);
    l.mBarrierDirection    = a.getInt("barrierDirection", kBarrierDirection, l.mBarrierDirection);
    l.mBarrierMargin       = a.getDimensionPixelSize("barrierMargin", l.mBarrierMargin);
    l.mBarrierAllowsGoneWidgets = a.getBoolean("barrierAllowsGoneWidgets", l.mBarrierAllowsGoneWidgets);
    l.mReferenceIdString   = a.getString("constraint_referenced_ids", l.mReferenceIdString);
    l.constraintTag        = a.getString("layout_constraintTag", l.constraintTag);

    // --- property set (visibility / alpha / progress) ---
    p.visibility = a.getInt("visibility", kVisibility, p.visibility);
    p.alpha      = a.getFloat("alpha", p.alpha);
    p.mProgress  = a.getFloat("motionProgress", p.mProgress);
    p.mVisibilityMode = a.getInt("visibilityMode", kVisibilityMode, p.mVisibilityMode);

    // --- transforms ---
    t.rotation    = a.getFloat("rotation",    t.rotation);
    t.rotationX   = a.getFloat("rotationX",   t.rotationX);
    t.rotationY   = a.getFloat("rotationY",   t.rotationY);
    t.scaleX      = a.getFloat("scaleX",      t.scaleX);
    t.scaleY      = a.getFloat("scaleY",      t.scaleY);
    t.translationX = a.getDimension("translationX", t.translationX);
    t.translationY = a.getDimension("translationY", t.translationY);
    t.translationZ = a.getDimension("translationZ", t.translationZ);
    t.transformPivotX = a.getDimension("transformPivotX", t.transformPivotX);
    t.transformPivotY = a.getDimension("transformPivotY", t.transformPivotY);
    t.transformPivotTarget = a.getResourceId("transformPivotTarget", t.transformPivotTarget);
    {
        float elev = a.getDimension("elevation", t.elevation);
        if (a.hasAttribute("elevation")) {
            t.applyElevation = true;
            t.elevation = elev;
        }
    }

    // --- motion ---
    m.mAnimateRelativeTo = a.getResourceId("animateRelativeTo", m.mAnimateRelativeTo);
    m.mTransitionEasing  = a.getString("transitionEasing", m.mTransitionEasing);
    m.mPathMotionArc     = a.getInt("pathMotionArc", kPathMotionArc, m.mPathMotionArc);
    m.mPathRotate        = a.getFloat("transitionPathRotate", m.mPathRotate);
    m.mMotionStagger     = a.getFloat("motionStagger", m.mMotionStagger);
    m.mDrawPath          = a.getInt("drawPath", m.mDrawPath);
    m.mQuantizeMotionSteps = a.getInt("quantizeMotionSteps", m.mQuantizeMotionSteps);
    m.mQuantizeMotionPhase = a.getFloat("quantizeMotionPhase", m.mQuantizeMotionPhase);
}

void ConstraintSet::load(Context* /*context*/, XmlPullParser& parser) {
    // Caller positions `parser` at the <ConstraintSet> START_TAG. We consume through its END_TAG.
    Constraint current;
    bool hasCurrent = false;
    auto toLower = [](std::string s) {
        for (auto& c : s) c = (char)std::tolower((unsigned char)c);
        return s;
    };

    // Terminate on END_DOCUMENT (clean) or BAD_DOCUMENT (expat parse error) — otherwise a malformed
    // resource would spin forever, since next() short-circuits on BAD_DOCUMENT.
    while (parser.getEventType() != XmlPullParser::END_DOCUMENT &&
            parser.getEventType() != XmlPullParser::BAD_DOCUMENT) {
        const int eventType = parser.getEventType();
        if (eventType == XmlPullParser::START_TAG) {
            const std::string tag = parser.getName();
            if (tag == "Constraint" || tag == "ConstraintOverride" ||
                    tag == "Guideline"   || tag == "Barrier") {
                current = Constraint{};
                hasCurrent = true;
                current.fillFromAttributeList(parser); // parser IS-A AttributeSet
                if (tag == "Guideline") {
                    current.layout.mIsGuideline = true;
                    current.layout.mApply = true;
                }
                if (tag == "Barrier")   {
                    current.layout.mHelperType = BARRIER_TYPE;
                }
            } else if (hasCurrent && (tag == "PropertySet" || tag == "Transform" ||
                                      tag == "Layout" || tag == "Motion")) {
                // Nested sub-element: same attribute names dispatch into the same sub-structs.
                current.fillFromAttributeList(parser);
            } else if (hasCurrent && (tag == "CustomAttribute" || tag == "CustomMethod")) {
                current.mCustomAttributes.push_back(parseCustomAttribute(parser));
            }
        } else if (eventType == XmlPullParser::END_TAG) {
            const std::string low = toLower(parser.getName());
            if (low == "constraintset") {
                return; // consumed the whole ConstraintSet; leave parser at this END_TAG
            }
            if (hasCurrent && (low == "constraint" || low == "constraintoverride" ||
                               low == "guideline"   || low == "barrier")) {
                mConstraints[current.mViewId] = current;
                hasCurrent = false;
            }
        }
        parser.next();
    }
}

} // namespace cdroid
