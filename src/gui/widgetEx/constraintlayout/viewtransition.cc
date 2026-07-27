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
 */
#include <widgetEx/constraintlayout/viewtransition.h>
#include <widgetEx/constraintlayout/keyframes.h>
#include <widgetEx/constraintlayout/motionscene.h>

#include <core/xmlpullparser.h>
#include <view/motionevent.h>
#include <view/view.h>

namespace cdroid {

namespace {
const std::unordered_map<std::string, int> kOnState = {
    {"actionDown",      (int) ViewTransition::ONSTATE_ACTION_DOWN},
    {"actionUp",        (int) ViewTransition::ONSTATE_ACTION_UP},
    {"actionDownUp",    (int) ViewTransition::ONSTATE_ACTION_DOWN_UP},
    {"sharedValueSet",  (int) ViewTransition::ONSTATE_SHARED_VALUE_SET},
    {"sharedValueUnset",(int) ViewTransition::ONSTATE_SHARED_VALUE_UNSET}
};
const std::unordered_map<std::string, int> kViewTransitionMode = {
    {"currentState", (int) ViewTransition::VIEWTRANSITIONMODE_CURRENTSTATE},
    {"allStates",    (int) ViewTransition::VIEWTRANSITIONMODE_ALLSTATES},
    {"noState",      (int) ViewTransition::VIEWTRANSITIONMODE_NOSTATE}
};
} // namespace

ViewTransition::ViewTransition(MotionScene& scene, Context* ctx, XmlPullParser& parser)
    : mScene(scene) {
    while (parser.getEventType() != XmlPullParser::END_DOCUMENT &&
            parser.getEventType() != XmlPullParser::BAD_DOCUMENT) {
        const int eventType = parser.getEventType();
        if (eventType == XmlPullParser::START_TAG) {
            const std::string tag = parser.getName();
            if (tag == "ViewTransition") {
                mId = mScene.getId(parser.getAttributeValue("id"));
                const int targetId = parser.getResourceId("motionTarget", UNSET);
                if (targetId != UNSET && targetId != 0) {
                    mTargetId = targetId;
                } else {
                    mTargetString = parser.getAttributeValue("motionTarget");
                }
                mOnStateTransition = parser.getInt("onStateTransition", kOnState, mOnStateTransition);
                mDisabled           = parser.getBoolean("transitionDisable", mDisabled);
                mPathMotionArc      = parser.getInt("pathMotionArc", mPathMotionArc);
                mDuration           = parser.getInt("duration", mDuration);
                mUpDuration         = parser.getInt("upDuration", mUpDuration);
                mViewTransitionMode = parser.getInt("viewTransitionMode", kViewTransitionMode,
                                                    mViewTransitionMode);
                mDefaultInterpolatorString = parser.getAttributeValue("motionInterpolator");
                mSetsTag    = parser.getResourceId("setsTag",    mSetsTag);
                mClearsTag  = parser.getResourceId("clearsTag",  mClearsTag);
                mIfTagSet   = parser.getResourceId("ifTagSet",   mIfTagSet);
                mIfTagNotSet= parser.getResourceId("ifTagNotSet",mIfTagNotSet);
                mSharedValueID     = parser.getResourceId("SharedValueId", mSharedValueID);
                mSharedValueTarget = parser.getInt("SharedValue", mSharedValueTarget);
            } else if (tag == "KeyFrameSet") {
                mKeyFrames = std::make_unique<KeyFrames>(ctx, parser); // consumes through </KeyFrameSet>
            }
            // ConstraintOverride / CustomAttribute / CustomMethod: deferred (need ConstraintSet delta APIs).
        } else if (eventType == XmlPullParser::END_TAG) {
            if (parser.getName() == "ViewTransition") return;
        }
        parser.next();
    }
}

bool ViewTransition::matchesView(View* view) const {
    if (view == nullptr) return false;
    if (mTargetId == UNSET && mTargetString.empty()) return false;
    // TODO: ifTagSet/ifTagNotSet gating + constraintTag regex match against mTargetString.
    return view->getId() == mTargetId;
}

bool ViewTransition::supports(int action) const {
    if (mOnStateTransition == ONSTATE_ACTION_DOWN)    return action == MotionEvent::ACTION_DOWN;
    if (mOnStateTransition == ONSTATE_ACTION_UP)      return action == MotionEvent::ACTION_UP;
    if (mOnStateTransition == ONSTATE_ACTION_DOWN_UP) return action == MotionEvent::ACTION_DOWN;
    return false;
}

} // namespace cdroid
