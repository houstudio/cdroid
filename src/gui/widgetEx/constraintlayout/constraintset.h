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
 *
 * Programmatic capture + application of a ConstraintLayout's constraints. clone() snapshots a
 * layout's LayoutParams into a per-id Constraint model; applyTo() writes a Constraint set back onto
 * a layout's LayoutParams (then requestLayout). The full typed setter API (connect incl. START/END,
 * center/centerHorizontally/Vertically, constrain* sizing/bias/weight/chain, setGuideline*,
 * create/createBarrier, create*Chain, transform/property setters) mirrors AndroidX; XML load via an
 * expat XmlPullParser and clone(Context, resource) round out the surface.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_CONSTRAINT_SET_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_CONSTRAINT_SET_H

#include <cmath>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <widgetEx/constraintlayout/constraintlayout.h>

namespace cdroid {

class XmlPullParser; // expat-backed pull parser (src/gui/core/xmlpullparser.h); IS-A AttributeSet

class ConstraintSet {
  public:
    // Anchor/side constants used by connect()/setMargin() (Android ConstraintSet values).
    static const int LEFT    = 3;
    static const int RIGHT   = 4;
    static const int TOP     = 1;
    static const int BOTTOM  = 2;
    static const int BASELINE = 5;
    static const int START   = 6;
    static const int END     = 7;
    static const int PARENT  = 0;
    // visibilityMode values (Java: ConstraintSet.VISIBILITY_MODE_*). IGNORE makes applyTo() leave
    // the view's visibility untouched — used by helpers (e.g. Carousel) that drive pool-view
    // visibility directly, so capture/apply does not clobber it.
    static constexpr int VISIBILITY_MODE_NORMAL = 0;
    static constexpr int VISIBILITY_MODE_IGNORE = 1;

    // Layout.mHelperType value marking a Barrier (Java: ConstraintSet.BARRIER_TYPE / Helper type).
    static const int BARRIER_TYPE = 1;

    // The per-view layout model (Java: ConstraintSet.Layout). Holds every constraint field; maps
    // bidirectionally to ConstraintLayout::LayoutParams via Constraint::applyTo / fillFrom.
    struct Layout {
        int mWidth = 0;
        int mHeight = 0;
        int guideBegin = -1, guideEnd = -1;
        float guidePercent = -1.0f;
        int leftToLeft = -1, leftToRight = -1, rightToLeft = -1, rightToRight = -1;
        int topToTop = -1, topToBottom = -1, bottomToTop = -1, bottomToBottom = -1;
        int baselineToBaseline = -1, baselineToTop = -1, baselineToBottom = -1;
        int startToStart = -1, startToEnd = -1, endToStart = -1, endToEnd = -1;
        int editorAbsoluteX = 0, editorAbsoluteY = 0;
        float horizontalBias = 0.5f, verticalBias = 0.5f;
        std::string dimensionRatio; // empty = none
        int orientation = -1;
        int leftMargin = 0, rightMargin = 0, topMargin = 0, bottomMargin = 0;
        int startMargin = INT_MIN, endMargin = INT_MIN, baselineMargin = INT_MIN;
        int goneLeftMargin = INT_MIN, goneTopMargin = INT_MIN;
        int goneRightMargin = INT_MIN, goneBottomMargin = INT_MIN;
        int goneStartMargin = INT_MIN, goneEndMargin = INT_MIN, goneBaselineMargin = INT_MIN;
        float verticalWeight = -1, horizontalWeight = -1;
        int horizontalChainStyle = 0 /*CHAIN_SPREAD*/, verticalChainStyle = 0;
        int widthDefault = 0 /*MATCH_CONSTRAINT_SPREAD*/, heightDefault = 0;
        int widthMax = 0, heightMax = 0, widthMin = 0, heightMin = 0;
        float widthPercent = 1, heightPercent = 1;
        int circleConstraint = -1, circleRadius = 0;
        float circleAngle = 0;
        bool constrainedWidth = false, constrainedHeight = false;
        int mWrapBehavior = 0;        // WRAP_BEHAVIOR_INCLUDED=0
        int mBarrierDirection = -1, mBarrierMargin = 0;
        bool mBarrierAllowsGoneWidgets = true;
        bool guidelineUseRtl = false;
        std::string mReferenceIdString; // comma-separated names; resolved to mReferenceIds lazily
        std::vector<int> mReferenceIds;
        std::string constraintTag;
        bool mIsGuideline = false;
        int mHelperType = 0;           // 0 = none, BARRIER_TYPE = barrier
        bool mApply = false;
    };

    // View transforms (Java: ConstraintSet.Transform). Applied to the View (not LayoutParams).
    struct Transform {
        float rotation = 0, rotationX = 0, rotationY = 0;
        float scaleX = 1, scaleY = 1;
        float translationX = 0, translationY = 0, translationZ = 0;
        float transformPivotX = 0, transformPivotY = 0;
        int   transformPivotTarget = -1;
        bool  applyElevation = false;
        float elevation = 0;
        bool mApply = false;
    };

    // Per-view property set (Java: ConstraintSet.PropertySet). visibility/alpha live here.
    struct PropertySet {
        int visibility = 0 /*View::VISIBLE*/;
        float alpha = 1.0f;
        int mVisibilityMode = 0;
        float mProgress = NAN; // NAN = unset
        bool mApply = false;
    };

    // Per-view motion config (Java: ConstraintSet.Motion). Drives the Motion controller.
    struct Motion {
        int mAnimateRelativeTo = -1;
        int mAnimateCircleAngleTo = -1;
        std::string mTransitionEasing;   // empty = default (named easing or spline string)
        int mPathMotionArc = -1;         // -1 none, else arc mode
        float mPathRotate = 0;
        float mMotionStagger = NAN;
        int mDrawPath = 0;
        int mQuantizeMotionSteps = 0;
        float mQuantizeMotionPhase = 0;
        int mQuantizeInterpolatorType = 0;
        std::string mQuantizeInterpolatorString;
        int mQuantizeInterpolatorID = -1;
        bool mApply = false;
    };

    // A parsed <CustomAttribute> (Java: ConstraintSet + ConstraintAttribute). CDROID has no
    // reflection, so applyTo handles only a whitelisted set of common attributes (textColor,
    // progress — direct-int setters); others are stored but not applied (TODO).
    struct CustomAttribute {
        enum Type { INTEGER, COLOR, FLOAT, STRING, BOOLEAN };
        std::string name;
        Type type = INTEGER;
        int intValue = 0;       // INTEGER or COLOR (ARGB)
        float floatValue = 0;
        std::string stringValue;
        bool boolValue = false;
    };

    // External extension point: handlers for named <CustomAttribute>s, registered by app/widget
    // code. applyTo() dispatches each parsed <CustomAttribute> to a registered handler — the
    // framework binds nothing itself (no hardcoded attribute, no View modification, no reflection).
    using CustomAttributeHandler = std::function<void(View*, const CustomAttribute&)>;
    static void registerCustomAttributeHandler(const std::string& name, CustomAttributeHandler handler);

    // Parse a <CustomAttribute> element (attributeName + one of customColorValue/customIntegerValue/
    // customFloatValue/customStringValue/customBooleanValue) at `parser`'s current START_TAG.
    static CustomAttribute parseCustomAttribute(const AttributeSet& parser);
    // Parse a <CustomAttribute> at `parser` and append it to this set's set-level custom collection.
    void loadCustomAttribute(const AttributeSet& parser);

    // Java: ConstraintSet.Constraint — one per referenced view id.
    struct Constraint {
        Layout layout;
        Transform transform;
        PropertySet propertySet;
        Motion motion;
        std::vector<CustomAttribute> mCustomAttributes;
        int mViewId = -1;
        // The XML attribute names this Constraint actually authored (populated by
        // fillFromAttributeList). applyDelta copies only these fields onto a target, so a delta
        // setting a field to its default value IS applied (precise), unlike a default-difference guess.
        std::unordered_set<std::string> mAuthored;

        void fillFrom(int viewId, const ConstraintLayout::LayoutParams& param);
        void applyTo(ConstraintLayout::LayoutParams& param) const;
        // Read every attribute on the current START_TAG (a <Constraint>/<Layout>/<Transform>/
        // <PropertySet>/<Motion> element) into the matching sub-struct. (Java: populateConstraint.)
        void fillFromAttributeList(const AttributeSet& attrs);
    };

    // --- core API ---
    void clone(ConstraintLayout* constraintLayout);
    // Inflate a layout resource (offscreen) and clone its constraints — supports a StateSet's
    // `constraints="@layout/foo"` reference. `resource` is the resource path (e.g. "layout/foo").
    void clone(Context* context, const std::string& resource);
    // Parse a <ConstraintSet> XML resource. `parser` is positioned at the <ConstraintSet> START_TAG;
    // returns after consuming the matching END_TAG. (Java: ConstraintSet.load(Context, XmlPullParser).)
    void load(Context* context, XmlPullParser& parser);
    // Parse a single <Constraint>/<ConstraintOverride>/<Guideline>/<Barrier> block: `parser` is at the
    // element's START_TAG; this reads its attributes + nested <PropertySet>/<Transform>/<Layout>/
    // <Motion>/<CustomAttribute> children and consumes through the matching END_TAG. Shared by load()
    // and ViewTransition (which builds its mConstraintDelta from <Constraint> children).
    void loadConstraint(XmlPullParser& parser);
    // Overlay this set's constraint for target.mViewId onto `target` — each authored sub-struct
    // (Layout/Transform/PropertySet/Motion, gated by mApply) replaces the target's; custom attributes
    // are appended. (Java: ConstraintSet.applyDelta(Constraint).)
    void applyDelta(Constraint& target) const;
    void applyTo(ConstraintLayout* constraintLayout);
    Constraint& get(int id);          // creates an entry if absent
    const Constraint* find(int id) const { // nullptr if absent (read-only lookup, no insert)
        auto it = mConstraints.find(id);
        return it == mConstraints.end() ? nullptr : &it->second;
    }
    bool contains(int id) const {
        return mConstraints.find(id) != mConstraints.end();
    }
    bool empty() const {
        return mConstraints.empty();
    }
    // Copy base's per-view constraints into this set; entries already present here are kept
    // (derived overrides base). Used by MotionScene for <ConstraintSet deriveConstraintsFrom=...>.
    void mergeFrom(const ConstraintSet& base);
    void clear(int viewId);
    void clear(int viewId, int anchor);

    // --- common setters ---
    void connect(int startID, int startSide, int endID, int endSide, int margin);
    void connect(int startID, int startSide, int endID, int endSide);
    void constrainWidth(int viewId, int width);
    void constrainHeight(int viewId, int height);
    void setVisibility(int viewId, int visibility);
    void setMargin(int viewId, int anchor, int value);
    void setDimensionRatio(int viewId, const std::string& ratio);
    // View-transform setters (capture start/end state for transitions).
    void setAlpha(int viewId, float alpha);
    void setRotation(int viewId, float rotation);
    void setRotationX(int viewId, float rotationX);
    void setRotationY(int viewId, float rotationY);
    void setScaleX(int viewId, float scaleX);
    void setScaleY(int viewId, float scaleY);
    void setTranslationX(int viewId, float translationX);
    void setTranslationY(int viewId, float translationY);

    // --- centering (built on connect + bias) ---
    void center(int centerID, int firstID, int firstSide, int firstMargin,
                int secondId, int secondSide, int secondMargin, float bias);
    void centerHorizontally(int centerID, int leftId, int leftSide, int leftMargin,
                            int rightId, int rightSide, int rightMargin, float bias);
    void centerHorizontally(int viewId, int toView);  // simple: both sides to toView, bias 0.5
    void centerHorizontallyRtl(int centerID, int startId, int startSide, int startMargin,
                               int endId, int endSide, int endMargin, float bias);
    void centerVertically(int centerID, int topId, int topSide, int topMargin,
                          int bottomId, int bottomSide, int bottomMargin, float bias);
    void centerVertically(int viewId, int toView);

    // --- match-constraint sizing / bias / weight / chain style ---
    void constrainDefaultWidth(int viewId, int width);
    void constrainDefaultHeight(int viewId, int height);
    void constrainMaxWidth(int viewId, int width);
    void constrainMaxHeight(int viewId, int height);
    void constrainMinWidth(int viewId, int width);
    void constrainMinHeight(int viewId, int height);
    void constrainPercentWidth(int viewId, float percent);
    void constrainPercentHeight(int viewId, float percent);
    void constrainedWidth(int viewId, bool constrained);
    void constrainedHeight(int viewId, bool constrained);
    void setHorizontalBias(int viewId, float bias);
    void setVerticalBias(int viewId, float bias);
    void setHorizontalWeight(int viewId, float weight);
    void setVerticalWeight(int viewId, float weight);
    void setHorizontalChainStyle(int viewId, int style);
    void setVerticalChainStyle(int viewId, int style);
    void setGoneMargin(int viewId, int anchor, int value);
    void constrainCircle(int viewId, int id, int radius, float angle);

    // --- guideline / barrier / helper ---
    void setGuidelineBegin(int guidelineID, int margin);
    void setGuidelineEnd(int guidelineID, int margin);
    void setGuidelinePercent(int guidelineID, float percent);
    void create(int guidelineID, int orientation);
    void createBarrier(int id, int direction, int margin, const std::vector<int>& referenced);
    void setReferencedIds(int viewId, const std::vector<int>& ids);
    void setBarrierType(int viewId, int type);

    // --- transform / property extras ---
    void setTranslationZ(int viewId, float translationZ);
    void setTransformPivotX(int viewId, float pivotX);
    void setTransformPivotY(int viewId, float pivotY);
    void setTransformPivot(int viewId, int target);
    void setElevation(int viewId, float elevation);
    void setApplyElevation(int viewId, bool apply);
    void setVisibilityMode(int viewId, int mode);
    void setProgress(int viewId, float progress);
    void setEditorAbsoluteX(int viewId, int value);
    void setEditorAbsoluteY(int viewId, int value);
    void setLayoutWrapBehavior(int viewId, int value);

    // --- chain creation (built on connect + weight/style) ---
    void createVerticalChain(int topId, int topSide, int bottomId, int bottomSide,
                             const std::vector<int>& chainIds,
                             const std::vector<float>& weights, int style);
    void createHorizontalChain(int leftId, int leftSide, int rightId, int rightSide,
                               const std::vector<int>& chainIds,
                               const std::vector<float>& weights, int style);
    void createHorizontalChainRtl(int startId, int startSide, int endId, int endSide,
                                  const std::vector<int>& chainIds,
                                  const std::vector<float>& weights, int style);

  private:
    std::unordered_map<int, Constraint> mConstraints;
    // Set-level <CustomAttribute>s (a ViewTransition's direct <CustomAttribute> children) — applied
    // to every target via applyDelta, mirroring Android's ConstraintSet.mCustomConstraints.
    std::vector<CustomAttribute> mCustomAttributes;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_CONSTRAINT_SET_H
