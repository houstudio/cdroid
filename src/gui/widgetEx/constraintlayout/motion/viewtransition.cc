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
 * Ported to C++ for CDROID from androidx.constraintlayout.motion.widget.ViewTransition.
 */
#include <widget/internal_R.h>
#include <widgetEx/widgetex_styleable.h>
#include <widgetEx/constraintlayout/motion/viewtransition.h>
#include <widgetEx/constraintlayout/motion/keyframes.h>
#include <widgetEx/constraintlayout/motion/motionscene.h>
#include <widgetEx/constraintlayout/motion/viewtransitioncontroller.h>
#include <widgetEx/constraintlayout/motion/motionlayout.h>
#include <widgetEx/constraintlayout/constraintset.h>

#include <regex>
#include <widgetEx/constraintlayout/core/motion/motion.h>
#include <widgetEx/constraintlayout/core/motion/motionwidget.h>
#include <widgetEx/constraintlayout/core/motion/motionkey.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyattributes.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyposition.h>
#include <widgetEx/constraintlayout/core/motion/motionkeycycle.h>
#include <widgetEx/constraintlayout/core/motion/motionkeytimecycle.h>
#include <widgetEx/constraintlayout/core/motion/motionkeytrigger.h>
#include <widgetEx/constraintlayout/core/motion/easing.h>
#include <widgetEx/constraintlayout/core/motion/typedvalues.h>

#include <core/xmlpullparser.h>
#include <core/systemclock.h>
#include <core/rect.h>
#include <view/motionevent.h>
#include <view/view.h>

#include <limits>
#include <unordered_map>

namespace cdroid {
using namespace cdroid::internal;

ViewTransition::ViewTransition(MotionScene& scene, Context* ctx, XmlPullParser& parser)
    : mScene(scene) {
    while (parser.getEventType() != XmlPullParser::END_DOCUMENT &&
            parser.getEventType() != XmlPullParser::BAD_DOCUMENT) {
        const int eventType = parser.getEventType();
        if (eventType == XmlPullParser::START_TAG) {
            const std::string tag = parser.getName();
            if (tag == "ViewTransition") {
                // TypedArray reads typed binary AXML values directly (AOSP pattern, same as
                // MotionScene::Transition/OnClick/OnSwipe); the default arg covers an absent
                // attr, so no name-based fallback is needed. Enums (onStateTransition,
                // viewTransitionMode, pathMotionArc) are compiled to their int values by aapt2.
                auto ta = ctx->obtainStyledAttributes(parser, R::styleable::ViewTransition);
                if (ta) {
                    namespace VT = R::styleable;
                    mId = (int)ta->getResourceId(VT::ViewTransition_id, UNSET);
                    // motionTarget is reference|string: a @id/... ref resolves to a resource id;
                    // a bare string is a constraintTag regex matched later in matchesView().
                    const int targetId = (int)ta->getResourceId(VT::ViewTransition_motionTarget, UNSET);
                    if (targetId != UNSET && targetId != 0) {
                        mTargetId = targetId;
                    } else {
                        mTargetString = ta->getString(VT::ViewTransition_motionTarget);
                    }
                    mOnStateTransition = ta->getInt(VT::ViewTransition_onStateTransition, mOnStateTransition);
                    mDisabled           = ta->getBoolean(VT::ViewTransition_transitionDisable, mDisabled);
                    mPathMotionArc      = ta->getInt(VT::ViewTransition_pathMotionArc, mPathMotionArc);
                    mDuration           = ta->getInt(VT::ViewTransition_duration, mDuration);
                    mUpDuration         = ta->getInt(VT::ViewTransition_upDuration, mUpDuration);
                    mViewTransitionMode = ta->getInt(VT::ViewTransition_viewTransitionMode, mViewTransitionMode);
                    mDefaultInterpolatorString = ta->getString(VT::ViewTransition_motionInterpolator);
                    mSetsTag     = (int)ta->getResourceId(VT::ViewTransition_setsTag,    mSetsTag);
                    mClearsTag   = (int)ta->getResourceId(VT::ViewTransition_clearsTag,  mClearsTag);
                    mIfTagSet    = (int)ta->getResourceId(VT::ViewTransition_ifTagSet,   mIfTagSet);
                    mIfTagNotSet = (int)ta->getResourceId(VT::ViewTransition_ifTagNotSet,mIfTagNotSet);
                    mSharedValueID     = (int)ta->getResourceId(VT::ViewTransition_SharedValueId, mSharedValueID);
                    mSharedValueTarget = ta->getInt(VT::ViewTransition_SharedValue, mSharedValueTarget);
                }
            } else if (tag == "KeyFrameSet") {
                mKeyFrames = std::make_unique<KeyFrames>(ctx, parser); // consumes through </KeyFrameSet>
            } else if (tag == "Constraint" || tag == "ConstraintOverride") {
                // A per-view override that becomes the delta applied in currentState/allStates mode.
                mConstraintDelta.loadConstraint(ctx, parser); // consumes through </Constraint>
            } else if (tag == "CustomAttribute" || tag == "CustomMethod") {
                // A ViewTransition-level custom attribute: stored on the delta's set-level collection
                // and applied to every target via applyDelta.
                mConstraintDelta.loadCustomAttribute(ctx, parser);
            }
        } else if (eventType == XmlPullParser::END_TAG) {
            if (parser.getName() == "ViewTransition") return;
        }
        parser.next();
    }
}

bool ViewTransition::matchesView(View* view) const {
    if (view == nullptr) return false;
    if (mTargetId == UNSET && mTargetString.empty()) return false;
    if (!checkTags(view)) return false; // ifTagSet / ifTagNotSet gating
    if (view->getId() == mTargetId) return true;
    if (mTargetString.empty()) return false;
    // String motionTarget: match the view's LayoutParams.constraintTag against the regex.
    auto* lp = dynamic_cast<ConstraintLayout::LayoutParams*>(view->getLayoutParams());
    if (lp == nullptr || lp->constraintTag.empty()) return false;
    try {
        return std::regex_match(lp->constraintTag, std::regex(mTargetString));
    } catch (const std::regex_error&) {
        return lp->constraintTag == mTargetString; // invalid pattern -> fall back to exact match
    }
}

bool ViewTransition::checkTags(View* view) const {
    const bool set    = (mIfTagSet == UNSET)    ? true : (view->getTag(mIfTagSet) != nullptr);
    const bool notSet = (mIfTagNotSet == UNSET) ? true : (view->getTag(mIfTagNotSet) == nullptr);
    return set && notSet;
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
                                     int fromId, ConstraintSet* current,
                                     const std::vector<View*>& views) {
    if (mDisabled) return;
    if (mViewTransitionMode == VIEWTRANSITIONMODE_NOSTATE) {
        for (View* v : views) applyIndependentTransition(controller, layout, v);
        return;
    }
    // currentState / allStates: apply the delta as an independent per-view animation (same
    // mechanism as noState above) rather than replacing the main transition. The old approach
    // (setTransition + transitionToEnd) replaced the main start↔end transition with a temporary
    // current→delta'd one, so the OnClick toggle fired a phantom animateTo that interrupted the
    // ViewTransition animation and corrupted progress.
    if (current == nullptr || layout == nullptr) return;

    // allStates additionally persists the delta into EVERY ConstraintSet (except the from-state) so
    // the change survives a later state switch (Android applyTransition 491-506).
    if (mViewTransitionMode == VIEWTRANSITIONMODE_ALLSTATES && !mConstraintDelta.empty()) {
        for (int id : layout->getConstraintSetIds()) {
            if (id == fromId) continue;
            ConstraintSet* cSet = layout->getConstraintSet(id);
            if (cSet == nullptr) continue;
            for (View* v : views) {
                if (v == nullptr) continue;
                mConstraintDelta.applyDelta(cSet->get(v->getId()));
            }
        }
    }

    // androidx animates current -> current+delta by cloning the current set, applying the delta
    // per target, and transitioning to it. CDROID keeps the independent per-view Animate
    // (f6b5548e4 — never replaces the main transition), but the Animate still gets the delta'd
    // endpoint: solve current+delta once via captureState and use the resulting frames as each
    // Motion's end. Capture the start frames BEFORE the solve (it re-layouts the children), and
    // restore the live layout to the current state afterwards so nothing visible moves.
    std::unordered_map<int, MotionWidget> startFrames, endFrames;
    for (View* v : views) {
        if (v == nullptr || v->getId() == View::NO_ID) continue;
        MotionLayout::captureWidgetFrame(startFrames[v->getId()], v);
    }
    if (!mConstraintDelta.empty()) {
        ConstraintSet deltaSet(*current); // deep copy — every member is a value type
        for (View* v : views) {
            if (v == nullptr || v->getId() == View::NO_ID) continue;
            mConstraintDelta.applyDelta(deltaSet.get(v->getId()));
        }
        layout->captureState(&deltaSet, endFrames);
    }
    for (View* v : views) {
        if (v == nullptr) continue;
        const auto s = startFrames.find(v->getId());
        const auto e = endFrames.find(v->getId());
        if (s != startFrames.end() && e != endFrames.end()) {
            applyIndependentTransition(controller, layout, v, s->second, e->second);
        } else {
            applyIndependentTransition(controller, layout, v); // no delta: keyframes only
        }
    }
    if (!endFrames.empty()) { // captureState left the layout at the delta'd set — put it back
        std::unordered_map<int, MotionWidget> restore;
        layout->captureState(current, restore);
    }
}

bool ViewTransition::addAllFrames(Motion* mc) const {
    if (mc == nullptr || mKeyFrames == nullptr) return false;
    for (MotionKey* key : mKeyFrames->getAllKeys()) {
        switch (key->mType) {
        case MotionKeyAttributes::KEY_TYPE: mc->addKey(static_cast<MotionKeyAttributes*>(key)); break;
        case MotionKeyPosition::KEY_TYPE:   mc->addKey(static_cast<MotionKeyPosition*>(key)); break;
        case MotionKeyCycle::KEY_TYPE:      mc->addKey(static_cast<MotionKeyCycle*>(key)); break;
        case MotionKeyTimeCycle::KEY_TYPE: mc->addKey(static_cast<MotionKeyTimeCycle*>(key)); break;
        case MotionKeyTrigger::KEY_TYPE:  mc->addKey(static_cast<MotionKeyTrigger*>(key)); break;
        default: break;
        }
    }
    return true;
}

void ViewTransition::applyTagsToViews(const std::vector<View*>& views) {
    if (mSetsTag == UNSET && mClearsTag == UNSET) return;
    for (View* v : views) {
        if (v == nullptr) continue;
        if (mSetsTag != UNSET)   v->setTag(mSetsTag, (void*) v);
        if (mClearsTag != UNSET) v->setTag(mClearsTag, nullptr);
    }
}

void ViewTransition::applyIndependentTransition(ViewTransitionController* controller,
                                                MotionLayout* layout, View* view) {
    if (view == nullptr || layout == nullptr) return;
    // setBothStates: the Motion's start and end are both the view's current frame, so progress 0 and
    // 1 leave the view untouched; the KeyFrameSet defines the deviation in between.
    MotionWidget mw;
    MotionLayout::captureWidgetFrame(mw, view);
    applyIndependentTransition(controller, layout, view, mw, mw);
}

void ViewTransition::applyIndependentTransition(ViewTransitionController* controller,
                                                MotionLayout* layout, View* view,
                                                MotionWidget& start, MotionWidget& end) {
    if (view == nullptr || layout == nullptr) return;
    // Motion::setStart/setEnd read the widget state into their path points synchronously
    // (nothing is retained), so the caller-owned frames are borrowed directly — MotionWidget's
    // copy is shallow (it owns its WidgetFrame), no by-value copies here.
    Motion* m = new Motion();
    m->setStart(&start);
    m->setEnd(&end);
    if (mKeyFrames) {
        for (MotionKey* key : mKeyFrames->getKeysForView(view->getId())) {
            switch (key->mType) {
            case MotionKeyAttributes::KEY_TYPE: m->addKey(static_cast<MotionKeyAttributes*>(key)); break;
            case MotionKeyPosition::KEY_TYPE:   m->addKey(static_cast<MotionKeyPosition*>(key)); break;
            case MotionKeyCycle::KEY_TYPE:      m->addKey(static_cast<MotionKeyCycle*>(key)); break;
            case MotionKeyTimeCycle::KEY_TYPE: m->addKey(static_cast<MotionKeyTimeCycle*>(key)); break;
            case MotionKeyTrigger::KEY_TYPE:  m->addKey(static_cast<MotionKeyTrigger*>(key)); break;
            default: break;
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
        applyTags();
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
        applyTags();
        mVtController->removeAnimation(this);
    }
    if (mPosition > 0.0f) mVtController->invalidate();
}

void ViewTransition::Animate::applyTags() {
    // Mark the ViewTransition as fired: setTag stores a non-null sentinel (the view itself); clearTag
    // removes a tag. Enables ViewTransition chaining via ifTagSet/ifTagNotSet.
    if (mSetsTag != UNSET)   mView->setTag(mSetsTag, (void*) mView);
    if (mClearsTag != UNSET) mView->setTag(mClearsTag, nullptr);
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
