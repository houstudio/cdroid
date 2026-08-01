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
 * Ported to C++ for CDROID from androidx.constraintlayout.motion.widget.MotionLayout.
 *
 * A ConstraintLayout that animates its children between two constraint states. setTransition()
 * captures each child's frame in the start and end ConstraintSets (apply → measure → layout →
 * read), building a per-child Motion controller; setProgress(p) then drives every Motion and
 * writes the interpolated position + transforms back onto the views.
 *
 * Ported: programmatic setTransition(start,end) + XML MotionScene, per-child keyframes
 * (KeyAttributes/Position/Cycle/TimeCycle/Trigger), spline + arc path interpolation, OnSwipe/
 * TouchResponse (spring/velocity), transitionToState with duration + Choreographer pacing, and the
 * multi-transition state machine. (Only the JSON/Compose core.state loading path is absent — CDROID
 * has no Compose/JSON layouts.)
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_MOTION_LAYOUT_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_MOTION_LAYOUT_H

#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>

#include <widgetEx/constraintlayout/constraintlayout.h>
#include <widgetEx/constraintlayout/constraintset.h>
#include <widgetEx/constraintlayout/motion/motionscene.h>
#include <widgetEx/constraintlayout/motion/touchresponse.h>
#include <core/callbackbase.h> // Runnable (for the post() wrapper)
#include <widgetEx/constraintlayout/core/motion/motion.h>
#include <view/choreographer.h> // FrameCallback — drives the spring by real frame time
#include <widgetEx/constraintlayout/core/motion/motionwidget.h>

namespace cdroid {

class ValueAnimator;
class MotionEvent;
class SpringStopEngine;
class StopLogicEngine;
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
    // Used by MotionScene::autoTransition: switch to `t` and animate/jump to its end (or start).
    void applyTransitionForAuto(MotionScene::Transition* t, bool toEnd, bool jump);
    int getCurrentState() const {
        return mCurrentState;
    }

    // Drive every child's Motion to `progress` in [0,1] and apply the interpolated state.
    void setProgress(float progress);
    float getProgress() const {
        return mProgress;
    }

    // Animate to the start/end state over the transition duration (driven by a ValueAnimator).
    void transitionToStart();
    void transitionToEnd();
    // transitionToEnd with a callback invoked once the animation reaches the end (used by
    // ViewTransition delta modes to set/clear tags on completion).
    void transitionToEnd(const std::function<void()>& onEnd);
    // Spring-settle to `target` (0 or 1) carrying `startVelocity` (progress/sec). Uses the OnSwipe
    // spring parameters; the spring stops itself via its energy threshold.
    void animateToWithSpring(float target, float startVelocity,
                             float mass, float stiffness, float damping,
                             float stopThreshold, int boundary);
    // Continuous-velocity settle (OnSwipe autoCompleteMode=continuousVelocity): a velocity-profile
    // engine carries the release momentum and decelerates to rest at `target` within the transition
    // duration, bounded by maxAcceleration/maxVelocity (progress units).
    void animateToWithStopLogic(float target, float startVelocity,
                                float maxAcceleration, float maxVelocity);
    // Instant jump (no animation).
    void setProgressInstant(float progress) {
        setProgress(progress);
    }
    void setTransitionDuration(int64_t durationMs) {
        mTransitionDuration = durationMs;
    }
    int64_t getTransitionDuration() const {
        return mTransitionDuration;
    }

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

    // ---- ViewTransition (per-view independent animations) ----
    // Programmatic fire (Android MotionLayout.viewTransition): animate `views` per the ViewTransition
    // registered under `viewTransitionId` in the scene.
    void viewTransition(int viewTransitionId, const std::vector<View*>& views);
    void enableViewTransition(int viewTransitionId, bool enable);
    bool isViewTransitionEnabled(int viewTransitionId) const;
    // Merge a ViewTransition's keyframes into a main-transition Motion (Android applyViewTransition).
    bool applyViewTransition(int viewTransitionId, Motion* mc);
    // Receiver for <KeyTrigger> fires during the main transition: (viewId, triggerName, progress).
    // The host interprets `triggerName` (Android calls the named method on the receiver view via
    // reflection; CDROID delivers the name here). Applied to every per-child Motion (current + rebuilt).
    using TriggerListener = std::function<void(int viewId, const std::string& triggerName, float position)>;
    void setTriggerListener(const TriggerListener& listener);
    // Fire a shared value (key, value) — notifies SharedValues listeners (e.g. sharedValueSet/Unset
    // ViewTransitions). (Java: MotionLayout.setSharedValue.)
    void setSharedValue(int key, int value);
    // ConstraintSet registered under `stateId` in the scene (used by the ViewTransition delta modes).
    ConstraintSet* getConstraintSet(int stateId) const;
    std::vector<int> getConstraintSetIds() const; // all ConstraintSet ids (ViewTransition allStates)
    ViewTransitionController* getViewTransitionController() const;

    // ---- TransitionListener (AndroidX MotionLayout.TransitionListener, nested) ----
    // Nested under MotionLayout (faithful to AndroidX) but expressed as an EventSet + CallbackBase
    // callback object (CDROID style, like Animator::AnimatorListener): assign lambdas to the events
    // you need, then addTransitionListener(it). A helper like Carousel holds one and registers it.
    class TransitionListener : public EventSet {
      public:
        CallbackBase<void, MotionLayout*, int, int, float> onTransitionChange;
        CallbackBase<void, MotionLayout*, int>             onTransitionCompleted;
    };
    // Subscribe to transition lifecycle events. Notified from every completion path (animateTo /
    // spring / stopLogic / setProgress) so a helper like Carousel can advance its state on completion.
    void addTransitionListener(const TransitionListener& listener) {
        mTransitionListeners.push_back(listener);
    }
    // mMotionLayout->post(r) resolves to the public View::post (handler-backed, regular queue) —
    // the same View.post the AndroidX Carousel posts its update runnable through. We intentionally
    // do NOT override post() here. (View::postUpdate is postAtFrontOfQueue — a different semantic
    // Android's Carousel does not use.)
    // Touch-up settle modes (AndroidX MotionLayout.TOUCH_UP_*).
    static constexpr int TOUCH_UP_STOP = 0;
    static constexpr int TOUCH_UP_AUTOCOMPLETE_TO_START = 1;
    static constexpr int TOUCH_UP_AUTOCOMPLETE_TO_END = 2;
    static constexpr int TOUCH_UP_DECELERATE_AND_COMPLETE = 3;
    // Drive the transition to `progress` (0 or 1) carrying `velocity` (progress/sec), per mode.
    void touchAnimateTo(int touchUpMode, float progress, float velocity);
    // Current transition velocity (progress/sec) from the active engine, else 0.
    float getVelocity() const;
    // Re-capture start/end states and rebuild every per-child Motion controller.
    void rebuildScene();
    // MotionScene forwarders (defined-transition scan / lookup by id).
    std::vector<MotionScene::Transition*> getDefinedTransitions() const;
    MotionScene::Transition* getTransition(int transitionId) const;
    // transitionToState with an explicit per-transition duration override (Carousel uses it).
    void transitionToState(int stateId, int64_t durationMs);

    // Inject a MotionScene programmatically (instead of via app:layoutDescription). Marks the scene
    // built so the onMeasure path does not rebuild it; the caller wires the current transition.
    void setScene(std::unique_ptr<MotionScene> scene);

    // View <-> MotionWidget frame bridge (shared with ViewTransition's noState driver).
    static void captureWidgetFrame(MotionWidget& out, View* v);
    static void applyWidgetFrame(View* v, MotionWidget& mw);

  protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
    void onLayout(bool changed, int l, int t, int w, int h) override;
    // Drag-to-progress when the scene's current transition has an <OnSwipe>. Intercepted once the
    // drag exceeds touch slop (so taps still reach <OnClick> children); auto-completes on release.
    bool onInterceptTouchEvent(MotionEvent& evt) override;
    bool onTouchEvent(MotionEvent& evt) override;

  private:
    // Fire TransitionListener callbacks. change() every frame/setProgress; completed() when an
    // animation reaches an endpoint (caller guards on mCurrentState != -1).
    void fireTransitionChange(int startId, int endId, float progress);
    void fireTransitionCompleted(int currentId);
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

    // Cancel the spring frame-callback loop (remove the posted callback + drop the engine). Called
    // whenever a new animation or transition supersedes a running spring, and from the dtor.
    void stopSpringAnimation();

    // Direction-based transition selection (androidx MotionScene.bestTransitionFor /
    // processTouchEvent): while the layout RESTS at a state (currentState != -1) shared by several
    // transitions (e.g. a bidirectional Carousel's rest state), switch to the one whose <OnSwipe>
    // dragDirection best matches the gesture, and seed its fresh TouchResponse at the touch point.
    // Like Android, picking stops once the drag leaves the endpoint (currentState becomes -1).
    // No-op for single-transition layouts (best == current, no switch).
    void pickTransitionForDrag(const MotionEvent& evt);

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
    // The spring is driven by a Choreographer frame callback (real frame time), not a ValueAnimator.
    // mSpringFrameCallback re-posts itself each frame until isStopped(); mSpringStartNanos is seeded
    // on the first frame. Mirrors androidx MotionLayout sampling mInterpolator at (frameTime - start).
    Choreographer::FrameCallback mSpringFrameCallback;
    int64_t mSpringStartNanos = -1;
    std::unique_ptr<StopLogicEngine> mStopEngine;
    bool mCaptured = false;
    bool mInAutoTransition = false; // guards autoTransition re-entry
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
    // Last touch position (set on DOWN, updated each MOVE) — the per-move drag delta for
    // pickTransitionForDrag, mirroring MotionScene.mLastTouchX/Y in processTouchEvent.
    float mLastTouchX = 0, mLastTouchY = 0;
    TriggerListener mTriggerListener; // host receiver for <KeyTrigger> fires (applied per Motion)
    std::vector<TransitionListener> mTransitionListeners; // Carousel etc. (AndroidX TransitionListener)
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_MOTION_LAYOUT_H
