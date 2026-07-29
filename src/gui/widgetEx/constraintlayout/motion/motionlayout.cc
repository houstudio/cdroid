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
 */
#include <widgetEx/constraintlayout/motion/motionlayout.h>

#include <porting/cdlog.h>
#include <animation/valueanimator.h>
#include <view/motionevent.h>
#include <view/view.h>
#include <widgetEx/constraintlayout/motion/keyframes.h>
#include <widgetEx/constraintlayout/motion/motionhelper.h>
#include <widgetEx/constraintlayout/motion/viewtransitioncontroller.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyattributes.h>
#include <widgetEx/constraintlayout/core/motion/motionkeycycle.h>
#include <widgetEx/constraintlayout/core/motion/motionkeytimecycle.h>
#include <widgetEx/constraintlayout/core/motion/motionkeytrigger.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyposition.h>
#include <widgetEx/constraintlayout/core/motion/typedvalues.h>
#include <widgetEx/constraintlayout/core/motion/springstopengine.h>
#include <widgetEx/constraintlayout/core/motion/stoplogicengine.h>

DECLARE_WIDGET(MotionLayout)

namespace cdroid {

MotionLayout::MotionLayout(Context* ctx, const AttributeSet& attrs)
    : ConstraintLayout(ctx, attrs) {
    // app:layoutDescription="@xml/..." points at a <MotionScene> resource (bare localname after the
    // XmlPullParser namespace strip). Resolved into a MotionScene on first measure (buildScene).
    mSceneResource = attrs.getString("layoutDescription", "");
}

MotionLayout::MotionLayout(int width, int height)
    : ConstraintLayout(width, height) {}

MotionLayout::~MotionLayout() {
    // Owns the per-child Motion controllers and the transition animator (buildMotions/animator reuse
    // raw pointers; nothing else frees the last set at destruction).
    for (auto& kv : mMotions) delete kv.second;
    mMotions.clear();
    delete mAnimator;
}

// Read a view's current frame + transforms into a MotionWidget (View -> MotionWidget bridge).
void MotionLayout::captureWidgetFrame(MotionWidget& mw, View* v) {
    mw.setBounds(v->getLeft(), v->getTop(), v->getRight(), v->getBottom());
    mw.setAlpha(v->getAlpha());
    mw.setRotationZ(v->getRotation());
    mw.setRotationX(v->getRotationX());
    mw.setRotationY(v->getRotationY());
    mw.setScaleX(v->getScaleX());
    mw.setScaleY(v->getScaleY());
    mw.setPivotX(v->getPivotX());
    mw.setPivotY(v->getPivotY());
    mw.setTranslationX(v->getTranslationX());
    mw.setTranslationY(v->getTranslationY());
}

// Write a MotionWidget's interpolated frame back onto a view. Attached views always get the
// transform applied (so reaching an endpoint resets it to identity — fixes rotation/scale not
// "snapping back" at progress 0/1). Unattached views (test harness) skip identity values, since the
// render-node invalidation path used by the setters is unsafe without an AttachInfo.
void MotionLayout::applyWidgetFrame(View* v, MotionWidget& mw) {
    v->layout(mw.getLeft(), mw.getTop(), mw.getWidth(), mw.getHeight());
    const bool attached = v->isAttachedToWindow();
    auto apply = [&](float val, float identity, void (View::*setter)(float)) {
        if (std::isnan(val)) return;
        if (attached || val != identity) (v->*setter)(val);
    };
    apply(mw.getAlpha(),        1.0f, &View::setAlpha);
    apply(mw.getRotationZ(),    0.0f, &View::setRotation);
    apply(mw.getRotationX(),    0.0f, &View::setRotationX);
    apply(mw.getRotationY(),    0.0f, &View::setRotationY);
    apply(mw.getScaleX(),       1.0f, &View::setScaleX);
    apply(mw.getScaleY(),       1.0f, &View::setScaleY);
    apply(mw.getTranslationX(), 0.0f, &View::setTranslationX);
    apply(mw.getTranslationY(), 0.0f, &View::setTranslationY);
}

void MotionLayout::captureState(ConstraintSet* cs, std::unordered_map<int, MotionWidget>& out) {
    if (cs == nullptr) return;
    cs->applyTo(this);
    mInCapture = true;
    if (mWidthSpec != 0 && mHeightSpec != 0) {
        measure(mWidthSpec, mHeightSpec);
        // CDROID View::layout(l, t, w, h) takes width/height — use the measured size, not
        // getRight()/getBottom() (which are 0 before the parent lays us out).
        layout(0, 0, getMeasuredWidth(), getMeasuredHeight());
    }
    mInCapture = false;
    out.clear();
    const int count = getChildCount();
    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        int id = child->getId();
        if (id == View::NO_ID) continue;
        captureWidgetFrame(out[id], child);
    }
}

void MotionLayout::buildMotions() {
    for (auto& kv : mMotions) delete kv.second;
    mMotions.clear();
    for (auto& sw : mStartWidgets) {
        int id = sw.first;
        auto ew = mEndWidgets.find(id);
        if (ew == mEndWidgets.end()) continue;
        Motion* m = new Motion();
        m->setStart(&sw.second);
        m->setEnd(&ew->second);
        // Parent dimensions feed TYPE_SCREEN KeyPositions (frames placed relative to the parent).
        m->setup(getWidth(), getHeight(), (float) mTransitionDuration);
        if (mSceneArcMode >= 0) {
            m->setValue(TypedValues::MotionType::TYPE_PATHMOTION_ARC, mSceneArcMode);
        }
        if (mTriggerListener) {
            m->setTriggerListener([this, id](const std::string& name, float pos) {
                mTriggerListener(id, name, pos);
            });
        }
        mMotions[id] = m;
    }
}

void MotionLayout::captureAndBuild() {
    captureState(mStartSet, mStartWidgets);
    captureState(mEndSet, mEndWidgets);
    buildMotions();
    // MotionHelper decorators (e.g. MotionEffect) insert keyframes into the freshly-built Motion
    // controllers now that their start/end positions are known (AndroidX MotionLayout calls each
    // decorator's onPreSetup during the controller-build path).
    if (!mMotions.empty()) {
        const int n = getChildCount();
        for (int i = 0; i < n; i++) {
            View* child = getChildAt(i);
            auto* decorator = dynamic_cast<MotionHelper*>(child);
            if (decorator != nullptr && decorator->isDecorator()) {
                decorator->onPreSetup(this, mMotions);
            }
        }
    }
    // The scene's KeyFrames go onto the freshly-built Motion controllers (borrowed pointers; the
    // MotionScene/KeyFrames outlive the controllers — both owned by this MotionLayout).
    if (mKeyFramesToApply) applyKeyFramesToMotions(mKeyFramesToApply);
    mCaptured = !mMotions.empty();
    if (mCaptured) applyMotion();
}

void MotionLayout::setTransition(ConstraintSet* start, ConstraintSet* end) {
    // Deep-copy the ConstraintSets — callers often pass stack-local objects that won't survive
    // the deferred capture (onMeasure runs long after setTransition returns).
    mOwnedStart = std::make_shared<ConstraintSet>(*start);
    mOwnedEnd   = std::make_shared<ConstraintSet>(*end);
    mStartSet = mOwnedStart.get();
    mEndSet   = mOwnedEnd.get();
    // The capture needs a real measure spec; if we haven't been laid out yet, defer to onMeasure.
    if (mWidthSpec != 0 && getWidth() > 0) {
        mCapturePending = false;
        captureAndBuild();
    } else {
        mCapturePending = true;
    }
}

void MotionLayout::setProgress(float progress) {
    mProgress = progress;
    if (progress <= 0.0f) mCurrentState = mBeginState;
    else if (progress >= 1.0f) mCurrentState = mEndState;
    else mCurrentState = -1; // in motion
    if (mCaptured) applyMotion();
    // On reaching an endpoint, fire any autoTransition whose mode matches this state.
    if (mCurrentState != -1 && mScene && !mInAutoTransition) {
        mInAutoTransition = true;
        mScene->autoTransition(this, mCurrentState);
        mInAutoTransition = false;
    }
}

void MotionLayout::animateTo(float target) {
    if (!mCaptured) {
        setProgress(target);
        return;
    }
    const float start = mProgress;
    if (mAnimator != nullptr) {
        mAnimator->cancel();
        delete mAnimator;
    }
    mAnimator = ValueAnimator::ofFloat({start, target});
    mAnimator->setDuration(mTransitionDuration);
    mAnimator->addUpdateListener([this, start, target](ValueAnimator& a) {
        float f = a.getAnimatedFraction();
        mProgress = start + (target - start) * f;
        if (mProgress <= 0.0f) mCurrentState = mBeginState;
        else if (mProgress >= 1.0f) mCurrentState = mEndState;
        else mCurrentState = -1;
        applyMotion();
    });
    mAnimator->start();
}

void MotionLayout::animateToWithSpring(float target, float startVelocity,
                                       float mass, float stiffness, float damping, float stopThreshold, int boundary) {
    if (!mCaptured) {
        setProgress(target);
        return;
    }
    if (mAnimator != nullptr) {
        mAnimator->cancel();
        delete mAnimator;
    }
    mSpringEngine = std::make_unique<SpringStopEngine>();
    mSpringEngine->springConfig(mProgress, target, startVelocity, mass, stiffness, damping,
                                stopThreshold, boundary);
    constexpr float kDurationSec = 2.0f; // generous upper bound; the spring ends itself via isStopped
    mAnimator = ValueAnimator::ofFloat({0.0f, 1.0f});
    mAnimator->setDuration((int64_t)(kDurationSec * 1000));
    SpringStopEngine* engine = mSpringEngine.get();
    mAnimator->addUpdateListener([this, engine, target, kDurationSec](ValueAnimator& a) {
        const float time = a.getAnimatedFraction() * kDurationSec;
        mProgress = engine->getInterpolation(time);
        if (mProgress <= 0.0f) mCurrentState = mBeginState;
        else if (mProgress >= 1.0f) mCurrentState = mEndState;
        else mCurrentState = -1;
        applyMotion();
        if (engine->isStopped()) {
            mProgress = target;
            if (target <= 0.0f) mCurrentState = mBeginState;
            else if (target >= 1.0f) mCurrentState = mEndState;
            if (mCaptured) applyMotion();
            a.cancel();
            if (mCurrentState != -1 && mScene && !mInAutoTransition) {
                mInAutoTransition = true;
                mScene->autoTransition(this, mCurrentState);
                mInAutoTransition = false;
            }
        }
    });
    mAnimator->start();
}

void MotionLayout::animateToWithStopLogic(float target, float startVelocity,
        float maxAcceleration, float maxVelocity) {
    if (!mCaptured) {
        setProgress(target);
        return;
    }
    if (mAnimator != nullptr) {
        mAnimator->cancel();
        delete mAnimator;
    }
    mStopEngine = std::make_unique<StopLogicEngine>();
    // The transition duration (seconds) bounds the profile's max time.
    const float maxTimeSec = mTransitionDuration / 1000.0f;
    mStopEngine->config(mProgress, target, startVelocity, maxTimeSec, maxAcceleration, maxVelocity);
    mAnimator = ValueAnimator::ofFloat({0.0f, 1.0f});
    mAnimator->setDuration(mTransitionDuration); // upper bound; the engine ends itself via isStopped
    StopLogicEngine* engine = mStopEngine.get();
    mAnimator->addUpdateListener([this, engine, target, maxTimeSec](ValueAnimator& a) {
        const float time = a.getAnimatedFraction() * maxTimeSec;
        mProgress = engine->getInterpolation(time);
        if (mProgress <= 0.0f) mCurrentState = mBeginState;
        else if (mProgress >= 1.0f) mCurrentState = mEndState;
        else mCurrentState = -1;
        applyMotion();
        if (engine->isStopped()) {
            mProgress = target;
            if (target <= 0.0f) mCurrentState = mBeginState;
            else if (target >= 1.0f) mCurrentState = mEndState;
            if (mCaptured) applyMotion();
            a.cancel();
            if (mCurrentState != -1 && mScene && !mInAutoTransition) {
                mInAutoTransition = true;
                mScene->autoTransition(this, mCurrentState);
                mInAutoTransition = false;
            }
        }
    });
    mAnimator->start();
}

void MotionLayout::transitionToStart() {
    animateTo(0.0f);
}
void MotionLayout::transitionToEnd()   {
    animateTo(1.0f);
}

void MotionLayout::transitionToEnd(const std::function<void()>& onEnd) {
    transitionToEnd();
    if (onEnd && mAnimator != nullptr) {
        Animator::AnimatorListener listener;
        listener.onAnimationEnd = [onEnd](Animator&, bool) { onEnd(); };
        mAnimator->addListener(listener);
    }
}

void MotionLayout::setTransition(int transitionId) {
    if (mScene == nullptr) return;
    auto* t = mScene->getTransitionById(transitionId);
    if (t == nullptr) return;
    mScene->setCurrentTransition(t);
    if (mAnimator != nullptr) mAnimator->cancel();
    applyTransition(t);
    setProgress(0.0f); // MVP: rest at the new transition's start
}

void MotionLayout::setTransition(int startId, int endId) {
    if (mScene == nullptr) return;
    auto* t = mScene->findTransition(startId, endId);
    if (t == nullptr) return;
    mScene->setCurrentTransition(t);
    if (mAnimator != nullptr) mAnimator->cancel();
    applyTransition(t);
    setProgress(0.0f);
}

void MotionLayout::transitionToState(int stateId) {
    if (mScene == nullptr) return;
    if (mCurrentState == stateId) return;        // already there
    if (mBeginState == stateId) {
        animateTo(0.0f);    // back along the current transition
        return;
    }
    if (mEndState == stateId)   {
        animateTo(1.0f);    // forward along the current transition
        return;
    }
    auto* t = mScene->findTransition(mCurrentState, stateId);
    if (t == nullptr) return;                    // MVP: no fallback transition construction
    mScene->setCurrentTransition(t);
    if (mAnimator != nullptr) mAnimator->cancel();
    applyTransition(t);
    animateTo(1.0f); // animate from the new start (=currentState) to the target end
}

void MotionLayout::applyTransitionForAuto(MotionScene::Transition* t, bool toEnd, bool jump) {
    if (mScene == nullptr || t == nullptr) return;
    mScene->setCurrentTransition(t);
    if (mAnimator != nullptr) mAnimator->cancel();
    applyTransition(t);
    if (jump) setProgress(toEnd ? 1.0f : 0.0f);
    else if (toEnd) animateTo(1.0f);
    else            animateTo(0.0f);
}

void MotionLayout::addKeyAttributes(int viewId, MotionKeyAttributes* key) {
    auto it = mMotions.find(viewId);
    if (it != mMotions.end()) it->second->addKey(key);
}

void MotionLayout::addKeyPosition(int viewId, MotionKeyPosition* key) {
    auto it = mMotions.find(viewId);
    if (it != mMotions.end()) it->second->addKey(key);
}

void MotionLayout::setTransitionEasing(const std::string& easing) {
    for (auto& kv : mMotions) {
        kv.second->setValue(TypedValues::MotionType::TYPE_EASING, easing);
    }
}

void MotionLayout::setTransitionEasing(int viewId, const std::string& easing) {
    auto it = mMotions.find(viewId);
    if (it != mMotions.end()) it->second->setValue(TypedValues::MotionType::TYPE_EASING, easing);
}

void MotionLayout::applyMotion() {
    const int count = getChildCount();
    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        int id = child->getId();
        auto it = mMotions.find(id);
        if (it == mMotions.end()) continue;
        MotionWidget temp;
        it->second->interpolate(&temp, mProgress);
        applyWidgetFrame(child, temp);
    }
    invalidate();
}

void MotionLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    mWidthSpec = widthMeasureSpec;
    mHeightSpec = heightMeasureSpec;
    // Build the MotionScene from layoutDescription on the first measure (idempotent). setTransition
    // then defers the capture to after ConstraintLayout::onMeasure gives us a measured size.
    if (!mSceneBuilt && !mSceneResource.empty()) buildScene();
    ConstraintLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);
    // If setTransition was called before we had a size, run the capture now that we do.
    // Use getMeasuredWidth (set by measure) — getWidth() is 0 until onLayout runs.
    if (mCapturePending && !mInCapture && getMeasuredWidth() > 0) {
        mCapturePending = false;
        captureAndBuild();
    }
}

void MotionLayout::onLayout(bool changed, int l, int t, int w, int h) {
    ConstraintLayout::onLayout(changed, l, t, w, h);
    // After the solver lays out children, override their positions with the interpolated motion
    // state. Without this the solver's layout (from the last-applied ConstraintSet) would clobber
    // the motion positions.
    if (mCaptured && !mInCapture) {
        applyMotion();
    }
}

// ===========================================================================
// MotionScene (pure-XML) path
// ===========================================================================
void MotionLayout::applyTransition(MotionScene::Transition* t) {
    if (t == nullptr) return;
    ConstraintSet* start = mScene->getConstraintSet(t->getStartId());
    ConstraintSet* end   = mScene->getConstraintSet(t->getEndId());
    if (start == nullptr || end == nullptr) return;
    setTransitionDuration(t->getDuration());
    if (!t->getInterpolatorString().empty()) setTransitionEasing(t->getInterpolatorString());
    mKeyFramesToApply = t->getKeyFrames();
    mSceneArcMode = t->getPathMotionArc();
    mBeginState = t->getStartId();
    mEndState = t->getEndId();
    setTransition(start, end); // the ConstraintSet* overload: captures + builds the per-child Motion
    wireOnClicks(t);
    mTouchResponse = (t->getOnSwipe() != nullptr)
                     ? std::make_unique<TouchResponse>(this, *t->getOnSwipe()) : nullptr;
    // The layout rests at the start (mProgress 0) after a fresh transition; reflect that in
    // mCurrentState so ViewTransition touch dispatch (which bails while currentState == -1) works.
    if (mProgress <= 0.0f) mCurrentState = mBeginState;
    else if (mProgress >= 1.0f) mCurrentState = mEndState;
}

void MotionLayout::buildScene() {
    if (mSceneBuilt || mSceneResource.empty()) return;
    mSceneBuilt = true; // set first so a parse failure doesn't retry every measure
    mScene = std::make_unique<MotionScene>(getContext(), this, mSceneResource);
    applyTransition(mScene->getCurrentTransition());
}

void MotionLayout::getAnchorDpDt(int anchorId, float pos, float locationX, float locationY,
                                 float out[2]) {
    out[0] = out[1] = 0.0f;
    auto it = mMotions.find(anchorId);
    if (it != mMotions.end()) {
        it->second->getDpDt(pos, locationX, locationY, out);
    }
}

// ---- ViewTransition delegators (forward to the scene, which owns the controller) ----
void MotionLayout::viewTransition(int viewTransitionId, const std::vector<View*>& views) {
    if (mScene) mScene->viewTransition(viewTransitionId, views);
}

void MotionLayout::enableViewTransition(int viewTransitionId, bool enable) {
    if (mScene) mScene->enableViewTransition(viewTransitionId, enable);
}

bool MotionLayout::isViewTransitionEnabled(int viewTransitionId) const {
    return mScene && mScene->isViewTransitionEnabled(viewTransitionId);
}

bool MotionLayout::applyViewTransition(int viewTransitionId, Motion* mc) {
    return mScene && mScene->applyViewTransition(viewTransitionId, mc);
}

void MotionLayout::setTriggerListener(const TriggerListener& listener) {
    mTriggerListener = listener;
    for (auto& kv : mMotions) {
        const int viewId = kv.first;
        if (listener) {
            kv.second->setTriggerListener(
                [listener, viewId](const std::string& name, float pos) { listener(viewId, name, pos); });
        } else {
            kv.second->setTriggerListener(nullptr);
        }
    }
}

void MotionLayout::setSharedValue(int key, int value) {
    ConstraintLayout::getSharedValues().fireNewValue(key, value);
}

ConstraintSet* MotionLayout::getConstraintSet(int stateId) const {
    return mScene ? mScene->getConstraintSet(stateId) : nullptr;
}

std::vector<int> MotionLayout::getConstraintSetIds() const {
    return mScene ? mScene->getConstraintSetIds() : std::vector<int>{};
}

ViewTransitionController* MotionLayout::getViewTransitionController() const {
    return mScene ? mScene->getViewTransitionController() : nullptr;
}

void MotionLayout::setScene(std::unique_ptr<MotionScene> scene) {
    mScene = std::move(scene);
    mSceneBuilt = true; // do not rebuild from layoutDescription on the next measure
}

void MotionLayout::applyKeyFramesToMotions(KeyFrames* kf) {
    if (kf == nullptr) return;
    for (auto& kv : mMotions) {
        Motion* m = kv.second;
        for (MotionKey* key : kf->getKeysForView(kv.first)) {
            switch (key->mType) {
            case MotionKeyAttributes::KEY_TYPE:
                m->addKey(static_cast<MotionKeyAttributes*>(key));
                break;
            case MotionKeyPosition::KEY_TYPE:
                m->addKey(static_cast<MotionKeyPosition*>(key));
                break;
            case MotionKeyCycle::KEY_TYPE:
                m->addKey(static_cast<MotionKeyCycle*>(key));
                break;
            case MotionKeyTimeCycle::KEY_TYPE:
                m->addKey(static_cast<MotionKeyTimeCycle*>(key)); // MVP: overlaid like a Cycle wave
                break;
            case MotionKeyTrigger::KEY_TYPE:
                m->addKey(static_cast<MotionKeyTrigger*>(key)); // fires via Motion::setTriggerListener
                break;
            default:
                break;
            }
        }
    }
}

void MotionLayout::wireOnClicks(MotionScene::Transition* t) {
    for (const auto& oc : t->getOnClicks()) {
        View* target = findViewById(oc.targetId);
        if (target == nullptr) continue;
        const int action = oc.clickAction;
        MotionLayout* self = this;
        target->setOnClickListener([self, action](View&) {
            switch (action) {
            case MotionScene::Transition::FLAG_TRANSITION_TO_END:
                self->transitionToEnd();
                break;
            case MotionScene::Transition::FLAG_TRANSITION_TO_START:
                self->transitionToStart();
                break;
            case MotionScene::Transition::FLAG_JUMP_TO_END:
                self->setProgress(1.0f);
                break;
            case MotionScene::Transition::FLAG_JUMP_TO_START:
                self->setProgress(0.0f);
                break;
            default: { // toggle: at the end state (or past halfway) -> start, else -> end
                const bool back = self->getCurrentState() == self->mEndState
                                  || self->getProgress() >= 0.5f;
                back ? self->transitionToStart() : self->transitionToEnd();
                break;
            }
            }
        });
    }
}

// OnSwipe drag-to-progress is delegated to TouchResponse (anchor geometry + auto-complete). We
// intercept only once the drag exceeds touch slop, so taps still reach <OnClick> children. Touches
// are also forwarded to the ViewTransitionController (fires <ViewTransition> press/release effects).
bool MotionLayout::onInterceptTouchEvent(MotionEvent& evt) {
    if (mScene) {
        if (auto* c = mScene->getViewTransitionController()) c->touchEvent(evt);
    }
    if (mTouchResponse == nullptr) return ConstraintLayout::onInterceptTouchEvent(evt);
    const int action = evt.getActionMasked();
    if (action == MotionEvent::ACTION_DOWN) {
        mTouchResponse->onDown(evt);
    } else if (action == MotionEvent::ACTION_MOVE) {
        if (mTouchResponse->dragSlopExceeded(evt)) return true;
    }
    return false;
}

bool MotionLayout::onTouchEvent(MotionEvent& evt) {
    if (mTouchResponse == nullptr) return ConstraintLayout::onTouchEvent(evt);
    const int action = evt.getActionMasked();
    if (action == MotionEvent::ACTION_DOWN) {
        mTouchResponse->onDown(evt);
        return true;
    }
    if (action == MotionEvent::ACTION_MOVE) {
        mTouchResponse->onMove(evt);
        return true;
    }
    if (action == MotionEvent::ACTION_UP || action == MotionEvent::ACTION_CANCEL) {
        mTouchResponse->onUp(evt);
        return true;
    }
    return true;
}

} // namespace cdroid
