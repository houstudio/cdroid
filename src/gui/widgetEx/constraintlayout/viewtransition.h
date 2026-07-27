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

#include <memory>
#include <string>

namespace cdroid {

class MotionScene;
class KeyFrames;
class Context;
class XmlPullParser;
class View;

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

  private:
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
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_VIEW_TRANSITION_H
