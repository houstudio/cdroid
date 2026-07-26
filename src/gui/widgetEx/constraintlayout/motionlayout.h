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

class MotionLayout : public ConstraintLayout {
public:
    MotionLayout(Context* ctx, const AttributeSet& attrs);
    explicit MotionLayout(int width, int height);

    // Capture each child's frame in the start/end ConstraintSets and build per-child Motion.
    void setTransition(ConstraintSet* start, ConstraintSet* end);

    // Drive every child's Motion to `progress` in [0,1] and apply the interpolated state.
    void setProgress(float progress);
    float getProgress() const { return mProgress; }

    void transitionToStart() { setProgress(0.0f); }
    void transitionToEnd()   { setProgress(1.0f); }

protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;

private:
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
    bool mCaptured = false;
    bool mInCapture = false; // guard against re-entrant measure/layout during capture
    int mWidthSpec = 0;
    int mHeightSpec = 0;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_MOTION_LAYOUT_H
