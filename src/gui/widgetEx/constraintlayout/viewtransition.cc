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
#include <widgetEx/constraintlayout/viewtransitioncontroller.h>
#include <widgetEx/constraintlayout/motionlayout.h>
#include <widgetEx/constraintlayout/constraintset.h>
#include <widgetEx/constraintlayout/core/motion/motion.h>
#include <widgetEx/constraintlayout/core/motion/motionwidget.h>
#include <widgetEx/constraintlayout/core/motion/motionkey.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyattributes.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyposition.h>
#include <widgetEx/constraintlayout/core/motion/motionkeycycle.h>
#include <widgetEx/constraintlayout/core/motion/easing.h>
#include <widgetEx/constraintlayout/core/motion/typedvalues.h>

#include <core/xmlpullparser.h>
#include <core/systemclock.h>
#include <core/rect.h>
#include <view/motionevent.h>
#include <view/view.h>

#include <limits>

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
            } else if (tag == "Constraint" || tag == "ConstraintOverride") {
                // A per-view override that becomes the delta applied in currentState/allStates mode.
                mConstraintDelta.loadConstraint(parser); // consumes through </Constraint>
            }
            // CustomAttribute/CustomMethod at the ViewTransition level: deferred.
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

// ===========================================================================
// ViewTransition::applyTransition / applyIndependentTransition / Animate
// ===========================================================================

void ViewTransition::applyTransition(ViewTransitionController* controller, MotionLayout* layout,
                                     int /*fromId*/, ConstraintSet* current,
                                     const std::vector<View*>& views) {
    if (mDisabled) return;
    if (mViewTransitionMode == VIEWTRANSITIONMODE_NOSTATE) {
        for (View* v : views) applyIndependentTransition(controller, layout, v);
        return;
    }
    // currentState / allStates: apply mConstraintDelta to the current ConstraintSet and animate the
    // target views to the resulting state. Android clones the current set, applies the delta, then
    // drives a temporary Transition (start = current, end = delta'd) via transitionToEnd — which is
    // exactly CDROID's setTransition(start, end) + transitionToEnd(). (allStates additionally writes
    // the delta into every ConstraintSet for persistence; that is deferred — both modes animate the
    // current state here.)
    if (current == nullptr || layout == nullptr) return;
    if (mConstraintDelta.empty()) return; // nothing to apply
    ConstraintSet transformed = *current;               // deep copy (map of Constraint)
    for (View* v : views) {
        if (v == nullptr) continue;
        mConstraintDelta.applyDelta(transformed.get(v->getId()));
    }
    layout->setTransition(current, &transformed);
    layout->transitionToEnd();
}

void ViewTransition::applyIndependentTransition(ViewTransitionController* controller,
                                                MotionLayout* layout, View* view) {
    if (view == nullptr || layout == nullptr) return;
    // setBothStates: the Motion's start and end are both the view's current frame, so progress 0 and
    // 1 leave the view untouched; the KeyFrameSet defines the deviation in between.
    MotionWidget mw;
    MotionLayout::captureWidgetFrame(mw, view);
    Motion* m = new Motion();
    m->setStart(&mw);
    m->setEnd(&mw);
    if (mKeyFrames) {
        for (MotionKey* key : mKeyFrames->getKeysForView(view->getId())) {
            switch (key->mType) {
            case MotionKeyAttributes::KEY_TYPE: m->addKey(static_cast<MotionKeyAttributes*>(key)); break;
            case MotionKeyPosition::KEY_TYPE:   m->addKey(static_cast<MotionKeyPosition*>(key)); break;
            case MotionKeyCycle::KEY_TYPE:      m->addKey(static_cast<MotionKeyCycle*>(key)); break;
            default: break; // KeyTimeCycle/KeyTrigger: deferred
            }
        }
    }
    m->setup(layout->getWidth(), layout->getHeight(), (float)mDuration);
    if (mPathMotionArc >= 0) {
        m->setValue(TypedValues::MotionType::TYPE_PATHMOTION_ARC, mPathMotionArc);
    }
    // The Motion borrows the keyframe pointers (its dtor does not delete them); the ViewTransition's
    // KeyFrames outlives this short-lived Animate (owned by the MotionScene/MotionLayout).
    auto easing = Easing::getInterpolator(mDefaultInterpolatorString); // nullptr for empty/linear
    auto animate = std::make_unique<Animate>(controller, m, view, mDuration, mUpDuration,
                                             mOnStateTransition, std::move(easing),
                                             mSetsTag, mClearsTag);
    controller->addAnimation(std::move(animate)); // registers + runs the first frame
}

ViewTransition::Animate::Animate(ViewTransitionController* controller, Motion* mc, View* view,
                                 int duration, int upDuration, int mode,
                                 std::unique_ptr<Easing> interpolator, int setTag, int clearTag)
    : mVtController(controller)
    , mMC(mc)
    , mView(view)
    , mDuration(duration)
    , mUpDuration(upDuration)
    , mInterpolator(std::move(interpolator))
    , mSetsTag(setTag)
    , mClearsTag(clearTag) {
    mStart = SystemClock::uptimeMillis();
    mLastRender = mStart;
    if (mode == ONSTATE_ACTION_DOWN_UP) mHoldAt100 = true; // hold pressed state until released
    mDpositionDt = (mDuration == 0) ? std::numeric_limits<float>::max() : 1.0f / mDuration;
}

ViewTransition::Animate::~Animate() {
    delete mMC; // owns the per-view Motion (the borrowed keyframes are not freed here)
}

void ViewTransition::Animate::mutate() {
    const int64_t now = SystemClock::uptimeMillis();
    const long elapsed = (long)(now - mLastRender);
    mLastRender = now;
    stepMutate(elapsed);
}

void ViewTransition::Animate::stepMutate(long elapsedMs) {
    if (mReverse) mutateReverse(elapsedMs);
    else          mutateForward(elapsedMs);
}

void ViewTransition::Animate::mutateForward(long elapsedMs) {
    mPosition += (float)elapsedMs * mDpositionDt; // elapsedMs·(1/durationMs) → 0..1 over `duration`
    if (mPosition >= 1.0f) mPosition = 1.0f;

    const float ipos = mInterpolator ? (float)mInterpolator->get(mPosition) : mPosition;
    MotionWidget temp;
    mMC->interpolate(&temp, ipos);
    MotionLayout::applyWidgetFrame(mView, temp);

    if (mPosition >= 1.0f) {
        // TODO: apply setsTag/clearsTag onto mView once wired (most ViewTransitions leave them UNSET).
        if (!mHoldAt100) mVtController->removeAnimation(this);
    }
    if (mPosition < 1.0f) mVtController->invalidate();
}

void ViewTransition::Animate::mutateReverse(long elapsedMs) {
    mPosition -= (float)elapsedMs * mDpositionDt;
    if (mPosition < 0.0f) mPosition = 0.0f;

    const float ipos = mInterpolator ? (float)mInterpolator->get(mPosition) : mPosition;
    MotionWidget temp;
    mMC->interpolate(&temp, ipos);
    MotionLayout::applyWidgetFrame(mView, temp);

    if (mPosition <= 0.0f) {
        mVtController->removeAnimation(this);
    }
    if (mPosition > 0.0f) mVtController->invalidate();
}

void ViewTransition::Animate::reverse(bool dir) {
    mReverse = dir;
    if (mReverse && mUpDuration != UNSET) {
        mDpositionDt = (mUpDuration == 0) ? std::numeric_limits<float>::max() : 1.0f / mUpDuration;
    }
    mVtController->invalidate();
    mLastRender = SystemClock::uptimeMillis();
}

void ViewTransition::Animate::reactTo(int action, float x, float y) {
    if (action == MotionEvent::ACTION_UP) {
        if (!mReverse) reverse(true); // release: animate back to the rest state
        return;
    }
    if (action == MotionEvent::ACTION_MOVE) {
        Rect rec;
        mView->getHitRect(rec);
        if (!rec.contains((int)x, (int)y)) {
            if (!mReverse) reverse(true); // finger slid off the target: release
        }
    }
}

} // namespace cdroid
