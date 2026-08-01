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
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.ConstraintSet.
 */
#include <climits>
#include <cctype>

#include <core/xmlpullparser.h>
#include <porting/cdlog.h>
#include <view/layoutinflater.h>
#include <view/view.h>
#include <widgetEx/constraintlayout/constraintset.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>

namespace cdroid {

namespace {

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
    Constraint& c = mConstraints[id]; // default-constructs a Constraint if absent
    c.mViewId = id; // a constraint retrieved by id knows its id (applyDelta/mergeFrom rely on this)
    return c;
}

ConstraintSet::CustomAttribute ConstraintSet::parseCustomAttribute(const AttributeSet& parser) {
    using CustomAttribute = ConstraintSet::CustomAttribute;
    CustomAttribute ca;
    ca.name = parser.getAttributeValue("attributeName");
    // Presence is detected via getAttributeValue (the hasValue equivalent); the
    // value is then read with the matching typed getter, which resolves @dimen/
    // @color/@string refs and handles numeric formats instead of parsing inline.
    if (!parser.getAttributeValue("customColorValue").empty()) {
        ca.type = CustomAttribute::COLOR;
        ca.intValue = parser.getColor("customColorValue", 0);
    } else if (!parser.getAttributeValue("customIntegerValue").empty()) {
        ca.type = CustomAttribute::INTEGER;
        ca.intValue = parser.getInt("customIntegerValue", 0);
    } else if (!parser.getAttributeValue("customFloatValue").empty()) {
        ca.type = CustomAttribute::FLOAT;
        ca.floatValue = parser.getFloat("customFloatValue", 0.f);
    } else if (!parser.getAttributeValue("customStringValue").empty()) {
        ca.type = CustomAttribute::STRING;
        ca.stringValue = parser.getString("customStringValue");
    } else if (!parser.getAttributeValue("customBooleanValue").empty()) {
        ca.type = CustomAttribute::BOOLEAN;
        ca.boolValue = parser.getBoolean("customBooleanValue", false);
    }
    return ca;
}

void ConstraintSet::loadCustomAttribute(const AttributeSet& parser) {
    mCustomAttributes.push_back(parseCustomAttribute(parser));
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

void ConstraintSet::clone(Context* context, const std::string& resource) {
    // Inflate the layout offscreen (no parent) and clone its children's LayoutParams, then discard
    // the inflated tree (we only needed its constraints). Supports StateSet `constraints="@layout/.."`.
    View* root = LayoutInflater::from(context)->inflate(resource, nullptr, false);
    auto* cl = dynamic_cast<ConstraintLayout*>(root);
    if (cl != nullptr) {
        clone(cl);
    }
    delete root;
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
        // VISIBILITY_MODE_IGNORE: do not touch the view's visibility — a helper (Carousel) is
        // driving it directly and applyTo must not reset it (e.g. a pool view it hid would pop back).
        if (c.propertySet.mVisibilityMode != VISIBILITY_MODE_IGNORE) {
            view->setVisibility(c.propertySet.visibility);
        }
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
    case START:
        if (endSide == END) l.startToEnd = endID;
        else l.startToStart = endID;
        l.startMargin = margin;
        break;
    case END:
        if (endSide == START) l.endToStart = endID;
        else l.endToEnd = endID;
        l.endMargin = margin;
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
    case START:
        l.startMargin = value;
        break;
    case END:
        l.endMargin = value;
        break;
    case BASELINE:
        l.baselineMargin = value;
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

// --- centering (built on connect + bias; faithful to AndroidX ConstraintSet) ---
void ConstraintSet::center(int centerID, int firstID, int firstSide, int firstMargin,
                           int secondId, int secondSide, int secondMargin, float bias) {
    if (firstSide == LEFT || firstSide == RIGHT) {
        connect(centerID, LEFT, firstID, firstSide, firstMargin);
        connect(centerID, RIGHT, secondId, secondSide, secondMargin);
        get(centerID).layout.horizontalBias = bias;
    } else if (firstSide == START || firstSide == END) {
        connect(centerID, START, firstID, firstSide, firstMargin);
        connect(centerID, END, secondId, secondSide, secondMargin);
        get(centerID).layout.horizontalBias = bias;
    } else {
        connect(centerID, TOP, firstID, firstSide, firstMargin);
        connect(centerID, BOTTOM, secondId, secondSide, secondMargin);
        get(centerID).layout.verticalBias = bias;
    }
}

void ConstraintSet::centerHorizontally(int centerID, int leftId, int leftSide, int leftMargin,
                                       int rightId, int rightSide, int rightMargin, float bias) {
    connect(centerID, LEFT, leftId, leftSide, leftMargin);
    connect(centerID, RIGHT, rightId, rightSide, rightMargin);
    get(centerID).layout.horizontalBias = bias;
}

void ConstraintSet::centerHorizontally(int viewId, int toView) {
    connect(viewId, LEFT, toView, LEFT, 0);
    connect(viewId, RIGHT, toView, RIGHT, 0);
    get(viewId).layout.horizontalBias = 0.5f;
}

void ConstraintSet::centerHorizontallyRtl(int centerID, int startId, int startSide, int startMargin,
                                          int endId, int endSide, int endMargin, float bias) {
    connect(centerID, START, startId, startSide, startMargin);
    connect(centerID, END, endId, endSide, endMargin);
    get(centerID).layout.horizontalBias = bias;
}

void ConstraintSet::centerVertically(int centerID, int topId, int topSide, int topMargin,
                                     int bottomId, int bottomSide, int bottomMargin, float bias) {
    connect(centerID, TOP, topId, topSide, topMargin);
    connect(centerID, BOTTOM, bottomId, bottomSide, bottomMargin);
    get(centerID).layout.verticalBias = bias;
}

void ConstraintSet::centerVertically(int viewId, int toView) {
    connect(viewId, TOP, toView, TOP, 0);
    connect(viewId, BOTTOM, toView, BOTTOM, 0);
    get(viewId).layout.verticalBias = 0.5f;
}

// --- match-constraint sizing / bias / weight / chain style ---
void ConstraintSet::constrainDefaultWidth(int viewId, int width)  { get(viewId).layout.widthDefault = width; }
void ConstraintSet::constrainDefaultHeight(int viewId, int height){ get(viewId).layout.heightDefault = height; }
void ConstraintSet::constrainMaxWidth(int viewId, int width)      { get(viewId).layout.widthMax = width; }
void ConstraintSet::constrainMaxHeight(int viewId, int height)    { get(viewId).layout.heightMax = height; }
void ConstraintSet::constrainMinWidth(int viewId, int width)      { get(viewId).layout.widthMin = width; }
void ConstraintSet::constrainMinHeight(int viewId, int height)    { get(viewId).layout.heightMin = height; }
void ConstraintSet::constrainPercentWidth(int viewId, float percent)  { get(viewId).layout.widthPercent = percent; }
void ConstraintSet::constrainPercentHeight(int viewId, float percent){ get(viewId).layout.heightPercent = percent; }
void ConstraintSet::constrainedWidth(int viewId, bool constrained)   { get(viewId).layout.constrainedWidth = constrained; }
void ConstraintSet::constrainedHeight(int viewId, bool constrained)  { get(viewId).layout.constrainedHeight = constrained; }
void ConstraintSet::setHorizontalBias(int viewId, float bias)     { get(viewId).layout.horizontalBias = bias; }
void ConstraintSet::setVerticalBias(int viewId, float bias)       { get(viewId).layout.verticalBias = bias; }
void ConstraintSet::setHorizontalWeight(int viewId, float weight) { get(viewId).layout.horizontalWeight = weight; }
void ConstraintSet::setVerticalWeight(int viewId, float weight)   { get(viewId).layout.verticalWeight = weight; }
void ConstraintSet::setHorizontalChainStyle(int viewId, int style){ get(viewId).layout.horizontalChainStyle = style; }
void ConstraintSet::setVerticalChainStyle(int viewId, int style)  { get(viewId).layout.verticalChainStyle = style; }

void ConstraintSet::setGoneMargin(int viewId, int anchor, int value) {
    Layout& l = get(viewId).layout;
    switch (anchor) {
    case LEFT:    l.goneLeftMargin = value; break;
    case RIGHT:   l.goneRightMargin = value; break;
    case TOP:     l.goneTopMargin = value; break;
    case BOTTOM:  l.goneBottomMargin = value; break;
    case START:   l.goneStartMargin = value; break;
    case END:     l.goneEndMargin = value; break;
    case BASELINE:l.goneBaselineMargin = value; break;
    default: break;
    }
}

void ConstraintSet::constrainCircle(int viewId, int id, int radius, float angle) {
    Layout& l = get(viewId).layout;
    l.circleConstraint = id;
    l.circleRadius = radius;
    l.circleAngle = angle;
}

// --- guideline / barrier / helper ---
void ConstraintSet::setGuidelineBegin(int guidelineID, int margin) {
    Layout& l = get(guidelineID).layout;
    l.guideBegin = margin;
    l.guideEnd = -1;
    l.guidePercent = -1.0f;
}
void ConstraintSet::setGuidelineEnd(int guidelineID, int margin) {
    Layout& l = get(guidelineID).layout;
    l.guideEnd = margin;
    l.guideBegin = -1;
    l.guidePercent = -1.0f;
}
void ConstraintSet::setGuidelinePercent(int guidelineID, float percent) {
    Layout& l = get(guidelineID).layout;
    l.guidePercent = percent;
    l.guideBegin = -1;
    l.guideEnd = -1;
}
void ConstraintSet::create(int guidelineID, int orientation) {
    Layout& l = get(guidelineID).layout;
    l.mIsGuideline = true;
    l.orientation = orientation;
}
void ConstraintSet::createBarrier(int id, int direction, int margin, const std::vector<int>& referenced) {
    Layout& l = get(id).layout;
    l.mHelperType = BARRIER_TYPE;
    l.mBarrierDirection = direction;
    l.mBarrierMargin = margin;
    l.mIsGuideline = false;
    l.mReferenceIds = referenced;
}
void ConstraintSet::setReferencedIds(int viewId, const std::vector<int>& ids) {
    get(viewId).layout.mReferenceIds = ids;
}
void ConstraintSet::setBarrierType(int viewId, int type) {
    get(viewId).layout.mBarrierDirection = type;
}

// --- transform / property extras ---
void ConstraintSet::setTranslationZ(int viewId, float translationZ) { get(viewId).transform.translationZ = translationZ; }
void ConstraintSet::setTransformPivotX(int viewId, float pivotX)    { get(viewId).transform.transformPivotX = pivotX; }
void ConstraintSet::setTransformPivotY(int viewId, float pivotY)    { get(viewId).transform.transformPivotY = pivotY; }
void ConstraintSet::setTransformPivot(int viewId, int target)       { get(viewId).transform.transformPivotTarget = target; }
void ConstraintSet::setElevation(int viewId, float elevation) {
    Transform& t = get(viewId).transform;
    t.elevation = elevation;
    t.applyElevation = true;
}
void ConstraintSet::setApplyElevation(int viewId, bool apply) { get(viewId).transform.applyElevation = apply; }
void ConstraintSet::setVisibilityMode(int viewId, int mode)   { get(viewId).propertySet.mVisibilityMode = mode; }
void ConstraintSet::setProgress(int viewId, float progress)   { get(viewId).propertySet.mProgress = progress; }
void ConstraintSet::setEditorAbsoluteX(int viewId, int value) { get(viewId).layout.editorAbsoluteX = value; }
void ConstraintSet::setEditorAbsoluteY(int viewId, int value) { get(viewId).layout.editorAbsoluteY = value; }
void ConstraintSet::setLayoutWrapBehavior(int viewId, int value) { get(viewId).layout.mWrapBehavior = value; }

// --- chain creation (built on connect + weight/style; faithful to AndroidX) ---
void ConstraintSet::createVerticalChain(int topId, int topSide, int bottomId, int bottomSide,
                                        const std::vector<int>& chainIds,
                                        const std::vector<float>& weights, int style) {
    if (chainIds.size() < 2) return;
    if (!weights.empty()) get(chainIds[0]).layout.verticalWeight = weights[0];
    get(chainIds[0]).layout.verticalChainStyle = style;
    connect(chainIds[0], TOP, topId, topSide, 0);
    for (size_t i = 1; i < chainIds.size(); i++) {
        connect(chainIds[i], TOP, chainIds[i - 1], BOTTOM, 0);
        connect(chainIds[i - 1], BOTTOM, chainIds[i], TOP, 0);
        if (!weights.empty()) get(chainIds[i]).layout.verticalWeight = weights[i];
    }
    connect(chainIds.back(), BOTTOM, bottomId, bottomSide, 0);
}

void ConstraintSet::createHorizontalChain(int leftId, int leftSide, int rightId, int rightSide,
                                          const std::vector<int>& chainIds,
                                          const std::vector<float>& weights, int style) {
    if (chainIds.size() < 2) return;
    if (!weights.empty()) get(chainIds[0]).layout.horizontalWeight = weights[0];
    get(chainIds[0]).layout.horizontalChainStyle = style;
    connect(chainIds[0], LEFT, leftId, leftSide, -1);
    for (size_t i = 1; i < chainIds.size(); i++) {
        connect(chainIds[i], LEFT, chainIds[i - 1], RIGHT, -1);
        connect(chainIds[i - 1], RIGHT, chainIds[i], LEFT, -1);
        if (!weights.empty()) get(chainIds[i]).layout.horizontalWeight = weights[i];
    }
    connect(chainIds.back(), RIGHT, rightId, rightSide, -1);
}

void ConstraintSet::createHorizontalChainRtl(int startId, int startSide, int endId, int endSide,
                                             const std::vector<int>& chainIds,
                                             const std::vector<float>& weights, int style) {
    if (chainIds.size() < 2) return;
    if (!weights.empty()) get(chainIds[0]).layout.horizontalWeight = weights[0];
    get(chainIds[0]).layout.horizontalChainStyle = style;
    connect(chainIds[0], START, startId, startSide, -1);
    for (size_t i = 1; i < chainIds.size(); i++) {
        connect(chainIds[i], START, chainIds[i - 1], END, -1);
        connect(chainIds[i - 1], END, chainIds[i], START, -1);
        if (!weights.empty()) get(chainIds[i]).layout.horizontalWeight = weights[i];
    }
    connect(chainIds.back(), END, endId, endSide, -1);
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
    // Record every attribute this element actually authored by iterating AttributeSet directly, so
    // applyDelta overlays precisely (only the authored fields) instead of guessing by default-difference.
    // copyAuthoredField maps the known field names; structural attrs (id/motionTarget) are recorded
    // too but ignored by copyAuthoredField's no-op default.
    a.forEachAttribute([&](const std::string& name, const std::string&) {
        mAuthored.insert(name);
    });

    Layout& l = layout;
    Transform& t = transform;
    PropertySet& p = propertySet;
    Motion& m = motion;

    // --- id + anchor targets (resolve "parent"/"@id/x" -> int via Context) ---
    // <Constraint> uses android:id; <ConstraintOverride> (ViewTransition delta) uses motionTarget.
    mViewId      = a.getResourceId("id", mViewId);
    if (mViewId == View::NO_ID) mViewId = a.getResourceId("motionTarget", mViewId);
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
                loadConstraint(parser); // consumes through the element's END_TAG
            }
        } else if (eventType == XmlPullParser::END_TAG) {
            if (toLower(parser.getName()) == "constraintset") {
                return; // consumed the whole ConstraintSet; leave parser at this END_TAG
            }
        }
        parser.next();
    }
}

void ConstraintSet::loadConstraint(XmlPullParser& parser) {
    // `parser` is at the <Constraint>/<ConstraintOverride>/<Guideline>/<Barrier> START_TAG. Read its
    // own attributes, then its nested sub-elements, and consume through the matching END_TAG.
    Constraint current;
    const std::string openTag = parser.getName();
    current.fillFromAttributeList(parser); // parser IS-A AttributeSet
    if (openTag == "Guideline") {
        current.layout.mIsGuideline = true;
        current.layout.mApply = true;
    } else if (openTag == "Barrier") {
        current.layout.mHelperType = BARRIER_TYPE;
    }
    auto toLower = [](std::string s) {
        for (auto& c : s) c = (char)std::tolower((unsigned char)c);
        return s;
    };
    const std::string openLower = toLower(openTag);

    while (parser.getEventType() != XmlPullParser::END_DOCUMENT &&
            parser.getEventType() != XmlPullParser::BAD_DOCUMENT) {
        parser.next();
        const int eventType = parser.getEventType();
        if (eventType == XmlPullParser::START_TAG) {
            const std::string tag = parser.getName();
            if (tag == "PropertySet" || tag == "Transform" ||
                    tag == "Layout" || tag == "Motion") {
                current.fillFromAttributeList(parser); // nested sub-element dispatches into sub-structs
            } else if (tag == "CustomAttribute" || tag == "CustomMethod") {
                current.mCustomAttributes.push_back(parseCustomAttribute(parser));
            }
        } else if (eventType == XmlPullParser::END_TAG) {
            if (toLower(parser.getName()) == openLower) break; // matching close of this constraint
        }
    }
    mConstraints[current.mViewId] = current;
}

// Copy the single field named `name` (a bare XML attribute name) from `src` onto `dst`. This is the
// precise per-attribute overlay (Android ConstraintSet.Delta + setDeltaValue): only authored fields
// are touched, so a delta that sets a field to its default value IS applied (the old default-difference
// guess skipped those).
static void copyAuthoredField(const std::string& name,
                              const ConstraintSet::Constraint& src,
                              ConstraintSet::Constraint& dst) {
    // --- Layout ---
    if (name == "layout_constraintLeft_toLeftOf")        dst.layout.leftToLeft = src.layout.leftToLeft;
    else if (name == "layout_constraintLeft_toRightOf")  dst.layout.leftToRight = src.layout.leftToRight;
    else if (name == "layout_constraintRight_toLeftOf")  dst.layout.rightToLeft = src.layout.rightToLeft;
    else if (name == "layout_constraintRight_toRightOf") dst.layout.rightToRight = src.layout.rightToRight;
    else if (name == "layout_constraintTop_toTopOf")     dst.layout.topToTop = src.layout.topToTop;
    else if (name == "layout_constraintTop_toBottomOf")  dst.layout.topToBottom = src.layout.topToBottom;
    else if (name == "layout_constraintBottom_toTopOf")  dst.layout.bottomToTop = src.layout.bottomToTop;
    else if (name == "layout_constraintBottom_toBottomOf") dst.layout.bottomToBottom = src.layout.bottomToBottom;
    else if (name == "layout_constraintBaseline_toBaselineOf") dst.layout.baselineToBaseline = src.layout.baselineToBaseline;
    else if (name == "layout_constraintBaseline_toTopOf") dst.layout.baselineToTop = src.layout.baselineToTop;
    else if (name == "layout_constraintBaseline_toBottomOf") dst.layout.baselineToBottom = src.layout.baselineToBottom;
    else if (name == "layout_constraintStart_toStartOf") dst.layout.startToStart = src.layout.startToStart;
    else if (name == "layout_constraintStart_toEndOf")   dst.layout.startToEnd = src.layout.startToEnd;
    else if (name == "layout_constraintEnd_toStartOf")   dst.layout.endToStart = src.layout.endToStart;
    else if (name == "layout_constraintEnd_toEndOf")     dst.layout.endToEnd = src.layout.endToEnd;
    else if (name == "layout_constraintCircle")          dst.layout.circleConstraint = src.layout.circleConstraint;
    else if (name == "layout_constraintGuide_begin")     dst.layout.guideBegin = src.layout.guideBegin;
    else if (name == "layout_constraintGuide_end")       dst.layout.guideEnd = src.layout.guideEnd;
    else if (name == "layout_constraintGuide_percent")   dst.layout.guidePercent = src.layout.guidePercent;
    else if (name == "layout_editor_absoluteX")          dst.layout.editorAbsoluteX = src.layout.editorAbsoluteX;
    else if (name == "layout_editor_absoluteY")          dst.layout.editorAbsoluteY = src.layout.editorAbsoluteY;
    else if (name == "orientation")                      dst.layout.orientation = src.layout.orientation;
    else if (name == "layout_marginLeft")                dst.layout.leftMargin = src.layout.leftMargin;
    else if (name == "layout_marginRight")               dst.layout.rightMargin = src.layout.rightMargin;
    else if (name == "layout_marginTop")                 dst.layout.topMargin = src.layout.topMargin;
    else if (name == "layout_marginBottom")              dst.layout.bottomMargin = src.layout.bottomMargin;
    else if (name == "layout_marginStart")               dst.layout.startMargin = src.layout.startMargin;
    else if (name == "layout_marginEnd")                 dst.layout.endMargin = src.layout.endMargin;
    else if (name == "layout_goneMarginLeft")            dst.layout.goneLeftMargin = src.layout.goneLeftMargin;
    else if (name == "layout_goneMarginTop")             dst.layout.goneTopMargin = src.layout.goneTopMargin;
    else if (name == "layout_goneMarginRight")           dst.layout.goneRightMargin = src.layout.goneRightMargin;
    else if (name == "layout_goneMarginBottom")          dst.layout.goneBottomMargin = src.layout.goneBottomMargin;
    else if (name == "layout_goneMarginStart")           dst.layout.goneStartMargin = src.layout.goneStartMargin;
    else if (name == "layout_goneMarginEnd")             dst.layout.goneEndMargin = src.layout.goneEndMargin;
    else if (name == "layout_constraintHorizontal_bias") dst.layout.horizontalBias = src.layout.horizontalBias;
    else if (name == "layout_constraintVertical_bias")   dst.layout.verticalBias = src.layout.verticalBias;
    else if (name == "layout_constraintHorizontal_weight") dst.layout.horizontalWeight = src.layout.horizontalWeight;
    else if (name == "layout_constraintVertical_weight") dst.layout.verticalWeight = src.layout.verticalWeight;
    else if (name == "layout_constraintHorizontal_chainStyle") dst.layout.horizontalChainStyle = src.layout.horizontalChainStyle;
    else if (name == "layout_constraintVertical_chainStyle") dst.layout.verticalChainStyle = src.layout.verticalChainStyle;
    else if (name == "layout_constraintDimensionRatio")  dst.layout.dimensionRatio = src.layout.dimensionRatio;
    else if (name == "layout_width")                     dst.layout.mWidth = src.layout.mWidth;
    else if (name == "layout_height")                    dst.layout.mHeight = src.layout.mHeight;
    else if (name == "layout_constraintWidth_default")   dst.layout.widthDefault = src.layout.widthDefault;
    else if (name == "layout_constraintHeight_default")  dst.layout.heightDefault = src.layout.heightDefault;
    else if (name == "layout_constraintWidth_percent")   dst.layout.widthPercent = src.layout.widthPercent;
    else if (name == "layout_constraintHeight_percent")  dst.layout.heightPercent = src.layout.heightPercent;
    else if (name == "layout_constraintWidth_min")       dst.layout.widthMin = src.layout.widthMin;
    else if (name == "layout_constraintWidth_max")       dst.layout.widthMax = src.layout.widthMax;
    else if (name == "layout_constraintHeight_min")      dst.layout.heightMin = src.layout.heightMin;
    else if (name == "layout_constraintHeight_max")      dst.layout.heightMax = src.layout.heightMax;
    else if (name == "layout_constrainedWidth")          dst.layout.constrainedWidth = src.layout.constrainedWidth;
    else if (name == "layout_constrainedHeight")         dst.layout.constrainedHeight = src.layout.constrainedHeight;
    else if (name == "layout_wrapBehaviorInParent")      dst.layout.mWrapBehavior = src.layout.mWrapBehavior;
    else if (name == "layout_constraintCircleRadius")    dst.layout.circleRadius = src.layout.circleRadius;
    else if (name == "layout_constraintCircleAngle")     dst.layout.circleAngle = src.layout.circleAngle;
    else if (name == "barrierDirection")                 dst.layout.mBarrierDirection = src.layout.mBarrierDirection;
    else if (name == "barrierMargin")                    dst.layout.mBarrierMargin = src.layout.mBarrierMargin;
    else if (name == "barrierAllowsGoneWidgets")         dst.layout.mBarrierAllowsGoneWidgets = src.layout.mBarrierAllowsGoneWidgets;
    else if (name == "constraint_referenced_ids")        dst.layout.mReferenceIdString = src.layout.mReferenceIdString;
    else if (name == "layout_constraintTag")             dst.layout.constraintTag = src.layout.constraintTag;
    // --- PropertySet ---
    else if (name == "visibility")      dst.propertySet.visibility = src.propertySet.visibility;
    else if (name == "alpha")           dst.propertySet.alpha = src.propertySet.alpha;
    else if (name == "motionProgress")  dst.propertySet.mProgress = src.propertySet.mProgress;
    else if (name == "visibilityMode")  dst.propertySet.mVisibilityMode = src.propertySet.mVisibilityMode;
    // --- Transform ---
    else if (name == "rotation")          dst.transform.rotation = src.transform.rotation;
    else if (name == "rotationX")         dst.transform.rotationX = src.transform.rotationX;
    else if (name == "rotationY")         dst.transform.rotationY = src.transform.rotationY;
    else if (name == "scaleX")            dst.transform.scaleX = src.transform.scaleX;
    else if (name == "scaleY")            dst.transform.scaleY = src.transform.scaleY;
    else if (name == "translationX")      dst.transform.translationX = src.transform.translationX;
    else if (name == "translationY")      dst.transform.translationY = src.transform.translationY;
    else if (name == "translationZ")      dst.transform.translationZ = src.transform.translationZ;
    else if (name == "transformPivotX")   dst.transform.transformPivotX = src.transform.transformPivotX;
    else if (name == "transformPivotY")   dst.transform.transformPivotY = src.transform.transformPivotY;
    else if (name == "transformPivotTarget") dst.transform.transformPivotTarget = src.transform.transformPivotTarget;
    else if (name == "elevation") { dst.transform.elevation = src.transform.elevation; dst.transform.applyElevation = true; }
    // --- Motion ---
    else if (name == "animateRelativeTo")   dst.motion.mAnimateRelativeTo = src.motion.mAnimateRelativeTo;
    else if (name == "transitionEasing")    dst.motion.mTransitionEasing = src.motion.mTransitionEasing;
    else if (name == "pathMotionArc")       dst.motion.mPathMotionArc = src.motion.mPathMotionArc;
    else if (name == "transitionPathRotate") dst.motion.mPathRotate = src.motion.mPathRotate;
    else if (name == "motionStagger")       dst.motion.mMotionStagger = src.motion.mMotionStagger;
    else if (name == "drawPath")            dst.motion.mDrawPath = src.motion.mDrawPath;
    else if (name == "quantizeMotionSteps") dst.motion.mQuantizeMotionSteps = src.motion.mQuantizeMotionSteps;
    else if (name == "quantizeMotionPhase") dst.motion.mQuantizeMotionPhase = src.motion.mQuantizeMotionPhase;
    // Unknown/structural names (id, motionTarget, xmlns) are intentionally ignored.
}

void ConstraintSet::applyDelta(Constraint& target) const {
    // Precise per-attribute overlay: copy only the fields the delta's <ConstraintOverride> actually
    // authored (recorded in mAuthored during fillFromAttributeList). This matches Android's sparse
    // Delta (ConstraintSet.Delta + setDeltaValue) — a delta setting a field to its default value IS
    // applied, and an unrelated field is never clobbered. Custom attributes are appended.
    auto it = mConstraints.find(target.mViewId);
    if (it != mConstraints.end()) {
        const Constraint& d = it->second;
        for (const auto& name : d.mAuthored) copyAuthoredField(name, d, target);
        for (const auto& ca : d.mCustomAttributes) target.mCustomAttributes.push_back(ca);
    }
    // Set-level customs (a ViewTransition's direct <CustomAttribute> children) apply to every target.
    for (const auto& ca : mCustomAttributes) target.mCustomAttributes.push_back(ca);
}

} // namespace cdroid
