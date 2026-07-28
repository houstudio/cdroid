/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.motion.widget.ViewTransition.
 *
 * A <ViewTransition> animates a single target view independently of the MotionLayout's main
 * transition — e.g. a press/focus effect on one view. It is triggered by a touch state
 * (onStateTransition: actionDown/actionUp/actionDownUp), a shared-value change, or programmatically
 * (MotionLayout.fireViewTransition). Its viewTransitionMode selects how it runs:
 *   noState        — a standalone per-view animation driven by its own Motion + KeyFrameSet;
 *   currentState   — apply a constraint delta to the target within the current transition;
 *   allStates      — apply the delta to every ConstraintSet.
 * (currentState/allStates need ConstraintSet delta APIs and are deferred; noState is the MVP.)
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_VIEW_TRANSITION_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_VIEW_TRANSITION_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <widgetEx/constraintlayout/constraintset.h>

namespace cdroid {

class MotionScene;
class KeyFrames;
class Context;
class XmlPullParser;
class View;
class ViewTransitionController;
class MotionLayout;
class Motion;
class Easing;
class MotionEvent;
class ConstraintSet;

class ViewTransition {
  public:
    // What triggers the ViewTransition (onStateTransition attr).
    static constexpr int ONSTATE_ACTION_DOWN        = 1;
    static constexpr int ONSTATE_ACTION_UP          = 2;
    static constexpr int ONSTATE_ACTION_DOWN_UP     = 3;
    static constexpr int ONSTATE_SHARED_VALUE_SET   = 4;
    static constexpr int ONSTATE_SHARED_VALUE_UNSET = 5;

    // How the ViewTransition runs (viewTransitionMode attr).
    static constexpr int VIEWTRANSITIONMODE_CURRENTSTATE = 0;
    static constexpr int VIEWTRANSITIONMODE_ALLSTATES    = 1;
    static constexpr int VIEWTRANSITIONMODE_NOSTATE      = 2;

    static constexpr int UNSET = -1;

    // Caller positions `parser` at the <ViewTransition> START_TAG; this reads its attributes and
    // consumes through </ViewTransition> (parsing nested <KeyFrameSet>).
    ViewTransition(MotionScene& scene, Context* ctx, XmlPullParser& parser);

    int getId() const {
        return mId;
    }
    int getStateTransition() const {
        return mOnStateTransition;
    }
    void setStateTransition(int s) {
        mOnStateTransition = s;
    }
    int getDuration() const {
        return mDuration;
    }
    int getUpDuration() const {
        return mUpDuration;
    }
    int getViewTransitionMode() const {
        return mViewTransitionMode;
    }
    int getPathMotionArc() const {
        return mPathMotionArc;
    }
    const std::string& getInterpolatorString() const {
        return mDefaultInterpolatorString;
    }
    KeyFrames* getKeyFrames() const {
        return mKeyFrames.get();
    }
    // The <Constraint> overrides nested in this <ViewTransition>, as a delta applied to the current
    // (or all) ConstraintSet(s) for the currentState/allStates modes. Empty for noState.
    const ConstraintSet& getConstraintDelta() const {
        return mConstraintDelta;
    }

    // Shared-value trigger (onStateTransition = sharedValueSet/Unset): the VT fires when the shared
    // value registered under mSharedValueID reaches (set) or leaves (unset) mSharedValueTarget.
    int getSharedValueID() const {
        return mSharedValueID;
    }
    int getSharedValue() const {
        return mSharedValueTarget;
    }
    int getSharedValueCurrent() const {
        return mSharedValueCurrent;
    }
    void setSharedValueCurrent(int v) {
        mSharedValueCurrent = v;
    }

    bool isEnabled() const {
        return !mDisabled;
    }
    void setEnabled(bool enable) {
        mDisabled = !enable;
    }

    // True if this ViewTransition applies to `view` (by target id; constraintTag string match is a
    // deferred fidelity).
    bool matchesView(View* view) const;
    // True if this ViewTransition should fire for the given MotionEvent action.
    bool supports(int action) const;

    // Dispatch entry (Android ViewTransition.applyTransition). For noState it drives a standalone
    // per-view animation; currentState/allStates (apply a ConstraintSet delta) are deferred.
    void applyTransition(ViewTransitionController* controller, MotionLayout* layout,
                         int fromId, ConstraintSet* current, const std::vector<View*>& views);

    // The noState animation driver (Android ViewTransition.Animate). A standalone per-view animation
    // advanced by wall-clock time: mutate() steps it forward (or in reverse once the finger lifts),
    // writing each interpolated frame onto the target view. Self-registers with the controller and
    // self-removes when it reaches its end (or, for actionDownUp, holds at 100% until released).
    class Animate {
      public:
        Animate(ViewTransitionController* controller, Motion* mc, View* view,
                int duration, int upDuration, int mode,
                std::unique_ptr<Easing> interpolator, int setTag, int clearTag);
        ~Animate();

        // Advance one frame using the elapsed wall-clock since the last render.
        void mutate();
        // Advance one frame by a forced elapsed time (ms). Used by the controller's frame tick and
        // by tests; mutate() just reads the clock and delegates here.
        void stepMutate(long elapsedMs);
        // React to a touch event: ACTION_UP (or leaving the view's hit rect) reverses a forward
        // animation (the actionDownUp press-release effect).
        void reactTo(int action, float x, float y);

        bool mRemove = false; // flagged by removeAnimation; the controller erases flagged entries

        // Fields mirror Android ViewTransition.Animate (package-private there); public so the
        // controller can drive/animate them.
        ViewTransitionController* mVtController;
        Motion* mMC;                 // owned (built in applyIndependentTransition); freed in ~Animate
        View* mView;                 // the animated view; each frame is written onto it
        int mDuration;
        int mUpDuration;
        std::unique_ptr<Easing> mInterpolator;
        int mSetsTag;
        int mClearsTag;
        bool mReverse = false;
        float mPosition = 0.0f;
        float mDpositionDt;
        int64_t mStart = 0;        // wall-clock (ms) at construction
        int64_t mLastRender = 0;   // wall-clock (ms) at the last mutate
        bool mHoldAt100 = false;

      private:
        void mutateForward(long elapsedMs);
        void mutateReverse(long elapsedMs);
        void reverse(bool dir);
        void applyTags(); // set/clear the setsTag/clearsTag keyed tags on the target view
    };

  private:
    friend class ViewTransitionController;
    // noState: build a standalone per-view Motion (start==end==current frame, keyframes drive it)
    // and start an Animate on the controller.
    void applyIndependentTransition(ViewTransitionController* controller, MotionLayout* layout,
                                    View* view);
    // ifTagSet/ifTagNotSet gating: true unless a required tag is missing / a forbidden tag is present.
    bool checkTags(View* view) const;
    // Set/clear the setsTag/clearsTag keyed tags on each view (delta-mode completion callback).
    void applyTagsToViews(const std::vector<View*>& views);
    MotionScene& mScene;
    int mId = UNSET;
    int mTargetId = UNSET;
    std::string mTargetString;          // constraintTag regex (string form of motionTarget)
    int mOnStateTransition = UNSET;
    bool mDisabled = false;
    int mPathMotionArc = 0;
    int mViewTransitionMode = VIEWTRANSITIONMODE_CURRENTSTATE;
    int mDuration = UNSET;
    int mUpDuration = UNSET;
    std::string mDefaultInterpolatorString;
    int mSetsTag = UNSET;
    int mClearsTag = UNSET;
    int mIfTagSet = UNSET;
    int mIfTagNotSet = UNSET;
    int mSharedValueID = UNSET;
    int mSharedValueTarget = UNSET;
    int mSharedValueCurrent = UNSET;
    std::unique_ptr<KeyFrames> mKeyFrames;
    ConstraintSet mConstraintDelta; // <Constraint> overrides (currentState/allStates delta mode)
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_VIEW_TRANSITION_H
