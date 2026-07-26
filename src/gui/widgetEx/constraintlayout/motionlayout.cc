/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.motion.widget.MotionLayout.
 */
#include <widgetEx/constraintlayout/motionlayout.h>

#include <porting/cdlog.h>
#include <animation/valueanimator.h>
#include <view/motionevent.h>
#include <view/view.h>
#include <widgetEx/constraintlayout/keyframes.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyattributes.h>
#include <widgetEx/constraintlayout/core/motion/motionkeycycle.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyposition.h>
#include <widgetEx/constraintlayout/core/motion/typedvalues.h>

DECLARE_WIDGET(MotionLayout)

namespace cdroid {

namespace {
// Read a view's current frame + transforms into a MotionWidget (View -> MotionWidget bridge).
void captureInto(MotionWidget& mw, View* v) {
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
void applyFrom(View* v, MotionWidget& mw) {
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
} // namespace

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
        captureInto(out[id], child);
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
        if (mSceneArcMode >= 0) {
            m->setValue(TypedValues::MotionType::TYPE_PATHMOTION_ARC, mSceneArcMode);
        }
        mMotions[id] = m;
    }
}

void MotionLayout::captureAndBuild() {
    captureState(mStartSet, mStartWidgets);
    captureState(mEndSet, mEndWidgets);
    buildMotions();
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
    if (mCaptured) applyMotion();
}

void MotionLayout::animateTo(float target) {
    if (!mCaptured) { setProgress(target); return; }
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
        applyMotion();
    });
    mAnimator->start();
}

void MotionLayout::transitionToStart() { animateTo(0.0f); }
void MotionLayout::transitionToEnd()   { animateTo(1.0f); }

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
        applyFrom(child, temp);
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

void MotionLayout::onLayout(bool changed, int l, int t, int r, int b) {
    ConstraintLayout::onLayout(changed, l, t, r, b);
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
void MotionLayout::buildScene() {
    if (mSceneBuilt || mSceneResource.empty()) return;
    mSceneBuilt = true; // set first so a parse failure doesn't retry every measure
    mScene = std::make_unique<MotionScene>(getContext(), this, mSceneResource);
    auto* t = mScene->getCurrentTransition();
    if (t == nullptr) return;

    ConstraintSet* start = mScene->getConstraintSet(t->getStartId());
    ConstraintSet* end   = mScene->getConstraintSet(t->getEndId());
    if (start != nullptr && end != nullptr) {
        setTransitionDuration(t->getDuration());
        if (!t->getInterpolatorString().empty()) {
            setTransitionEasing(t->getInterpolatorString());
        }
        mKeyFramesToApply = t->getKeyFrames();
        mSceneArcMode = t->getPathMotionArc();
        setTransition(start, end); // captures now if we have a size, else defers to onMeasure
    }
    wireOnClicks(t);
    if (t->getOnSwipe() != nullptr) {
        mTouchResponse = std::make_unique<TouchResponse>(this, *t->getOnSwipe());
    }
}

void MotionLayout::getAnchorDpDt(int anchorId, float pos, float locationX, float locationY,
                                 float out[2]) {
    out[0] = out[1] = 0.0f;
    auto it = mMotions.find(anchorId);
    if (it != mMotions.end()) {
        it->second->getDpDt(pos, locationX, locationY, out);
    }
}

void MotionLayout::applyKeyFramesToMotions(KeyFrames* kf) {
    if (kf == nullptr) return;
    for (auto& kv : mMotions) {
        Motion* m = kv.second;
        for (MotionKey* key : kf->getKeysForView(kv.first)) {
            switch (key->mType) {
                case MotionKeyAttributes::KEY_TYPE:
                    m->addKey(static_cast<MotionKeyAttributes*>(key)); break;
                case MotionKeyPosition::KEY_TYPE:
                    m->addKey(static_cast<MotionKeyPosition*>(key)); break;
                case MotionKeyCycle::KEY_TYPE:
                    m->addKey(static_cast<MotionKeyCycle*>(key)); break;
                default: break; // KeyTimeCycle/KeyTrigger: no Motion addKey yet (deferred)
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
                case MotionScene::Transition::FLAG_TRANSITION_TO_END:   self->transitionToEnd();   break;
                case MotionScene::Transition::FLAG_TRANSITION_TO_START: self->transitionToStart(); break;
                case MotionScene::Transition::FLAG_JUMP_TO_END:         self->setProgress(1.0f);   break;
                case MotionScene::Transition::FLAG_JUMP_TO_START:       self->setProgress(0.0f);   break;
                default: // toggle
                    self->getProgress() < 0.5f ? self->transitionToEnd() : self->transitionToStart();
                    break;
            }
        });
    }
}

// OnSwipe drag-to-progress is delegated to TouchResponse (anchor geometry + auto-complete). We
// intercept only once the drag exceeds touch slop, so taps still reach <OnClick> children.
bool MotionLayout::onInterceptTouchEvent(MotionEvent& evt) {
    if (mTouchResponse == nullptr) return ConstraintLayout::onInterceptTouchEvent(evt);
    const int action = evt.getActionMasked();
    if (action == MotionEvent::ACTION_DOWN) {
        mTouchResponse->onDown(evt.getX(), evt.getY());
    } else if (action == MotionEvent::ACTION_MOVE) {
        if (mTouchResponse->dragSlopExceeded(evt.getX(), evt.getY())) return true;
    }
    return false;
}

bool MotionLayout::onTouchEvent(MotionEvent& evt) {
    if (mTouchResponse == nullptr) return ConstraintLayout::onTouchEvent(evt);
    const int action = evt.getActionMasked();
    if (action == MotionEvent::ACTION_DOWN) {
        mTouchResponse->onDown(evt.getX(), evt.getY());
        return true;
    }
    if (action == MotionEvent::ACTION_MOVE) {
        mTouchResponse->onMove(evt.getX(), evt.getY());
        return true;
    }
    if (action == MotionEvent::ACTION_UP || action == MotionEvent::ACTION_CANCEL) {
        mTouchResponse->onUp(evt.getX(), evt.getY());
        return true;
    }
    return true;
}

} // namespace cdroid
