// Shared helpers for the androidx.constraintlayout.core test ports. Pure inline functions in
// namespace clport — they wrap CDROID's C++ core API so each ported TEST reads close to the
// AndroidX Java original. Nothing here touches the core library.
//
// API mapping (AndroidX Java -> CDROID C++ via these helpers):
//   a.connect(LEFT, root, RIGHT, m)           -> connect(a, Side::LEFT, root, Side::RIGHT, m)
//   a.setHorizontalChainStyle(CHAIN_SPREAD)   -> setHorizontalChainStyle(a, CHAIN_SPREAD)
//   a.setHorizontalWeight(1)                  -> setHorizontalWeight(a, 1)
//   b.setHorizontalMatchStyle(type,min,max,v) -> setHorizontalMatchStyle(b, type, min, max, v)
//   b.setDimensionRatio("16:9")               -> setDimensionRatio(b, "16:9")  (pure "W:H" only)
//   a.getLeft()/getRight()/getTop()/getBottom()-> getLeft(a)/...  (CDROID core only has getX/Y/W/H)
#pragma once
#include <cstdlib>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.h>
#include <widgetEx/constraintlayout/core/widgets/guideline.h>

namespace clport {

using cdroid::ConstraintAnchor;
using cdroid::ConstraintWidget;
using cdroid::ConstraintWidgetContainer;
using cdroid::clcore::Guideline;

// Mirrors androidx.constraintlayout.core.widgets.ConstraintAnchor.Type
enum class Side { LEFT, TOP, RIGHT, BOTTOM, BASELINE, CENTER_X, CENTER_Y, CENTER };

inline ConstraintAnchor::Type toType(Side s) {
    switch (s) {
        case Side::LEFT:     return ConstraintAnchor::Type::LEFT;
        case Side::TOP:      return ConstraintAnchor::Type::TOP;
        case Side::RIGHT:    return ConstraintAnchor::Type::RIGHT;
        case Side::BOTTOM:   return ConstraintAnchor::Type::BOTTOM;
        case Side::BASELINE: return ConstraintAnchor::Type::BASELINE;
        case Side::CENTER_X: return ConstraintAnchor::Type::CENTER_X;
        case Side::CENTER_Y: return ConstraintAnchor::Type::CENTER_Y;
        case Side::CENTER:   return ConstraintAnchor::Type::CENTER;
    }
    return ConstraintAnchor::Type::LEFT;
}

// Resolve via getAnchor(Type) — virtual, so Guideline's override returns its active anchor
// (a vertical guideline's LEFT/RIGHT both resolve to the guideline line). Direct mLeft access
// would miss that and leave Guideline-anchored widgets stuck at 0.
inline ConstraintAnchor& anchor(ConstraintWidget& w, Side s) {
    return *w.getAnchor(toType(s));
}

// a.connect(ConstraintAnchor.Type.LEFT, root, ConstraintAnchor.Type.LEFT [, margin])
inline void connect(ConstraintWidget& from, Side fa, ConstraintWidget& target, Side ta, int margin = 0) {
    anchor(from, fa).connect(anchor(target, ta), margin);
}

inline void setHorizontalChainStyle(ConstraintWidget& w, int style) { w.mHorizontalChainStyle = style; }
inline void setVerticalChainStyle  (ConstraintWidget& w, int style) { w.mVerticalChainStyle   = style; }

inline void setHorizontalWeight(ConstraintWidget& w, float weight) { w.mWeight[ConstraintWidget::HORIZONTAL] = weight; }
inline void setVerticalWeight  (ConstraintWidget& w, float weight) { w.mWeight[ConstraintWidget::VERTICAL]   = weight; }

// b.setHorizontalMatchStyle(type, min, max, value). `value` is percent (0..1) for PERCENT;
// ignored for SPREAD/WRAP; for RATIO the ratio is set separately via setDimensionRatio().
inline void setHorizontalMatchStyle(ConstraintWidget& w, int type, int mn, int mx, float value) {
    w.mMatchConstraintDefaultWidth = type;
    w.setMinWidth(mn); w.setMaxWidth(mx);
    if (type == ConstraintWidget::MATCH_CONSTRAINT_PERCENT) w.mMatchConstraintPercentWidth = value;
}
inline void setVerticalMatchStyle(ConstraintWidget& w, int type, int mn, int mx, float value) {
    w.mMatchConstraintDefaultHeight = type;
    w.setMinHeight(mn); w.setMaxHeight(mx);
    if (type == ConstraintWidget::MATCH_CONSTRAINT_PERCENT) w.mMatchConstraintPercentHeight = value;
}

// b.setDimensionRatio("16:9") — parses pure "W:H" only (the forms the chain tests use). Does NOT
// support "W,16:9"/"H,16:9" prefix or lone-float forms. Returns false on parse failure.
inline bool setDimensionRatio(ConstraintWidget& w, const char* ratioStr) {
    if (ratioStr == nullptr || *ratioStr == '\0') {
        w.mDimensionRatio = 0; w.mDimensionRatioSide = ConstraintWidget::UNKNOWN; return true;
    }
    const char* colon = ratioStr;
    while (*colon && *colon != ':') ++colon;
    if (*colon != ':') return false;
    float num   = std::strtof(ratioStr, nullptr);
    float denom = std::strtof(colon + 1, nullptr);
    if (denom == 0) return false;
    w.mDimensionRatio = num / denom;
    w.mDimensionRatioSide = ConstraintWidget::UNKNOWN; // let solver pick side (Java default)
    return true;
}
// b.setDimensionRatio(0, 0) clears the ratio (Java overload used in ChainTest.testPackChain).
inline void setDimensionRatio(ConstraintWidget& w, int /*numerator*/, int /*denominator*/) {
    w.mDimensionRatio = 0; w.mDimensionRatioSide = ConstraintWidget::UNKNOWN;
}

// CDROID core only exposes getX/getY/getWidth/getHeight; AndroidX tests use getLeft/Right/Top/Bottom.
inline int getLeft  (ConstraintWidget& w) { return w.getX(); }
inline int getTop   (ConstraintWidget& w) { return w.getY(); }
inline int getRight (ConstraintWidget& w) { return w.getX() + w.getWidth(); }
inline int getBottom(ConstraintWidget& w) { return w.getY() + w.getHeight(); }

} // namespace clport
