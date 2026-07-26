/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.motion.widget.MotionLayout.
 *
 * A ConstraintLayout that animates its children between two constraint states. setTransition()
 * captures each child's frame in the start and end ConstraintSets (apply → measure → layout →
 * read), building a per-child Motion controller; setProgress(p) then drives every Motion and
 * writes the interpolated position + transforms back onto the views.
 *
 * MVP: programmatic start/end ConstraintSets + linear interpolation (the Motion engine's MVP).
 * Deferred: MotionScene/Transition JSON loading (core.state), keyframes, arc/eased paths, and the
 * animation/touch decoupling (transitionTo* with duration + Choreographer pacing).
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_MOTION_LAYOUT_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_MOTION_LAYOUT_H

#include <unordered_map>

#include <widgetEx/constraintlayout/constraintlayout.h>
#include <widgetEx/constraintlayout/constraintset.h>
#include <widgetEx/constraintlayout/core/motion/motion.h>
#include <widgetEx/constraintlayout/core/motion/motionwidget.h>

namespace cdroid {

class ValueAnimator;
class MotionKeyAttributes;
class MotionKeyPosition;

class MotionLayout : public ConstraintLayout {
public:
    MotionLayout(Context* ctx, const AttributeSet& attrs);
    explicit MotionLayout(int width, int height);

    // Capture each child's frame in the start/end ConstraintSets and build per-child Motion.
    void setTransition(ConstraintSet* start, ConstraintSet* end);

    // Drive every child's Motion to `progress` in [0,1] and apply the interpolated state.
    void setProgress(float progress);
    float getProgress() const { return mProgress; }

    // Animate to the start/end state over the transition duration (driven by a ValueAnimator).
    void transitionToStart();
    void transitionToEnd();
    // Instant jump (no animation).
    void setProgressInstant(float progress) { setProgress(progress); }
    void setTransitionDuration(int64_t durationMs) { mTransitionDuration = durationMs; }
    int64_t getTransitionDuration() const { return mTransitionDuration; }

    // Per-child keyframes (must be called after setTransition). The MotionLayout stores the key
    // and forwards it to the child's Motion controller.
    void addKeyAttributes(int viewId, MotionKeyAttributes* key);
    void addKeyPosition(int viewId, MotionKeyPosition* key);
    // Set the easing curve on all (or one) child Motion.
    void setTransitionEasing(const std::string& easing);
    void setTransitionEasing(int viewId, const std::string& easing);

protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
    void onLayout(bool changed, int l, int t, int r, int b) override;

private:
    // Animate mProgress to `target` over mTransitionDuration using a ValueAnimator.
    void animateTo(float target);
    // Actually run the start/end capture + build motions (called once we have a real size).
    void captureAndBuild();

    // Apply `cs`, force a measure+layout pass, then read each child's frame into `out`.
    void captureState(ConstraintSet* cs, std::unordered_map<int, MotionWidget>& out);
    // Build (or rebuild) the per-child Motion controllers from the captured start/end widgets.
    void buildMotions();
    // Apply the interpolated state at mProgress to every child view.
    void applyMotion();

    ConstraintSet* mStartSet = nullptr;
    ConstraintSet* mEndSet = nullptr;
    std::unordered_map<int, Motion*> mMotions;
    std::unordered_map<int, MotionWidget> mStartWidgets;
    std::unordered_map<int, MotionWidget> mEndWidgets;
    float mProgress = 0.0f;
    int64_t mTransitionDuration = 400;
    ValueAnimator* mAnimator = nullptr;
    bool mCaptured = false;
    bool mCapturePending = false; // setTransition called before the layout had a size
    bool mInCapture = false; // guard against re-entrant measure/layout during capture
    int mWidthSpec = 0;
    int mHeightSpec = 0;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_MOTION_LAYOUT_H
