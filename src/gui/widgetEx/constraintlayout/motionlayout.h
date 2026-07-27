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

#include <memory>
#include <string>
#include <unordered_map>

#include <widgetEx/constraintlayout/constraintlayout.h>
#include <widgetEx/constraintlayout/constraintset.h>
#include <widgetEx/constraintlayout/motionscene.h>
#include <widgetEx/constraintlayout/touchresponse.h>
#include <widgetEx/constraintlayout/core/motion/motion.h>
#include <widgetEx/constraintlayout/core/motion/motionwidget.h>

namespace cdroid {

class ValueAnimator;
class MotionEvent;
class SpringStopEngine;
class MotionKeyAttributes;
class MotionKeyPosition;
class KeyFrames;

class MotionLayout : public ConstraintLayout {
public:
    MotionLayout(Context* ctx, const AttributeSet& attrs);
    explicit MotionLayout(int width, int height);
    ~MotionLayout() override;

    // Capture each child's frame in the start/end ConstraintSets and build per-child Motion.
    void setTransition(ConstraintSet* start, ConstraintSet* end);
    // Multi-transition state machine: switch by <Transition android:id> or by endpoint ConstraintSet
    // ids, or animate from the current state to a target state.
    void setTransition(int transitionId);
    void setTransition(int startId, int endId);
    void transitionToState(int stateId);
    int getCurrentState() const { return mCurrentState; }

    // Drive every child's Motion to `progress` in [0,1] and apply the interpolated state.
    void setProgress(float progress);
    float getProgress() const { return mProgress; }

    // Animate to the start/end state over the transition duration (driven by a ValueAnimator).
    void transitionToStart();
    void transitionToEnd();
    // Spring-settle to `target` (0 or 1) carrying `startVelocity` (progress/sec). Uses the OnSwipe
    // spring parameters; the spring stops itself via its energy threshold.
    void animateToWithSpring(float target, float startVelocity,
                             float mass, float stiffness, float damping,
                             float stopThreshold, int boundary);
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

    // Pixels-per-progress of the anchor point (locationX,locationY) on view `anchorId` at `pos`.
    // Used by TouchResponse to map drag deltas to progress (the anchor's travel is the drag range).
    void getAnchorDpDt(int anchorId, float pos, float locationX, float locationY, float out[2]);

protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
    void onLayout(bool changed, int l, int t, int r, int b) override;
    // Drag-to-progress when the scene's current transition has an <OnSwipe>. Intercepted once the
    // drag exceeds touch slop (so taps still reach <OnClick> children); auto-completes on release.
    bool onInterceptTouchEvent(MotionEvent& evt) override;
    bool onTouchEvent(MotionEvent& evt) override;

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

    // --- MotionScene (pure-XML) path ---
    // Parse mSceneResource into a MotionScene and wire its current transition: setTransition with
    // the scene's start/end ConstraintSets, capture the KeyFrames for post-capture application, and
    // install OnClick handlers. Idempotent (guarded by mSceneBuilt).
    void applyTransition(MotionScene::Transition* t); // shared by buildScene + setTransition(id)
    void buildScene();
    // Feed the scene's KeyFrames into the freshly-built per-child Motion controllers (borrowed).
    void applyKeyFramesToMotions(KeyFrames* kf);
    // Wire a transition's <OnClick> targets to toggle/transition-to-end/start the layout.
    void wireOnClicks(MotionScene::Transition* t);

    ConstraintSet* mStartSet = nullptr;
    ConstraintSet* mEndSet = nullptr;
    std::shared_ptr<ConstraintSet> mOwnedStart;
    std::shared_ptr<ConstraintSet> mOwnedEnd;
    std::unordered_map<int, Motion*> mMotions;
    std::unordered_map<int, MotionWidget> mStartWidgets;
    std::unordered_map<int, MotionWidget> mEndWidgets;
    int mCurrentState = -1; // ConstraintSet id at rest (-1 = UNSET / in motion)
    int mBeginState = -1;   // current transition's start ConstraintSet id
    int mEndState = -1;     // current transition's end ConstraintSet id
    float mProgress = 0.0f;
    int64_t mTransitionDuration = 400;
    ValueAnimator* mAnimator = nullptr;
    std::unique_ptr<SpringStopEngine> mSpringEngine;
    bool mCaptured = false;
    bool mCapturePending = false; // setTransition called before the layout had a size
    bool mInCapture = false; // guard against re-entrant measure/layout during capture
    int mWidthSpec = 0;
    int mHeightSpec = 0;

    // Pure-XML MotionScene path (app:layoutDescription="@xml/...").
    std::unique_ptr<MotionScene> mScene;
    std::string mSceneResource;       // resource path of the <MotionScene> XML
    KeyFrames* mKeyFramesToApply = nullptr; // borrowed from mScene's current transition; applied post-capture
    int mSceneArcMode = -1;            // Transition-level pathMotionArc, propagated to each Motion
    bool mSceneBuilt = false;

    // OnSwipe drag-to-progress (null when the scene has no <OnSwipe>).
    std::unique_ptr<TouchResponse> mTouchResponse;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_MOTION_LAYOUT_H
