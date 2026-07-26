/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.ConstraintSet.
 *
 * Programmatic capture + application of a ConstraintLayout's constraints. clone() snapshots a
 * layout's LayoutParams into a per-id Constraint model; applyTo() writes a Constraint set back onto
 * a layout's LayoutParams (then requestLayout). This is the core programmatic API; the XML
 * load(Context, int) parser and the long tail of typed setters are deferred.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_CONSTRAINT_SET_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_CONSTRAINT_SET_H

#include <cmath>
#include <string>
#include <unordered_map>
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

    // Java: ConstraintSet.Constraint — one per referenced view id.
    struct Constraint {
        Layout layout;
        Transform transform;
        PropertySet propertySet;
        Motion motion;
        int mViewId = -1;

        void fillFrom(int viewId, const ConstraintLayout::LayoutParams& param);
        void applyTo(ConstraintLayout::LayoutParams& param) const;
        // Read every attribute on the current START_TAG (a <Constraint>/<Layout>/<Transform>/
        // <PropertySet>/<Motion> element) into the matching sub-struct. (Java: populateConstraint.)
        void fillFromAttributeList(const AttributeSet& attrs);
    };

    // --- core API ---
    void clone(ConstraintLayout* constraintLayout);
    // Parse a <ConstraintSet> XML resource. `parser` is positioned at the <ConstraintSet> START_TAG;
    // returns after consuming the matching END_TAG. (Java: ConstraintSet.load(Context, XmlPullParser).)
    void load(Context* context, XmlPullParser& parser);
    void applyTo(ConstraintLayout* constraintLayout);
    Constraint& get(int id);          // creates an entry if absent
    bool contains(int id) const { return mConstraints.find(id) != mConstraints.end(); }
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

private:
    std::unordered_map<int, Constraint> mConstraints;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_CONSTRAINT_SET_H
