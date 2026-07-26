/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.motion.widget.MotionLayout.
 */
#include <widgetEx/constraintlayout/motionlayout.h>

#include <porting/cdlog.h>
#include <animation/valueanimator.h>
#include <view/view.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyattributes.h>
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

// Write a MotionWidget's interpolated frame back onto a view. Transforms are applied only when
// non-identity — the render-node setters are unsafe on unattached views and identity is a no-op.
void applyFrom(View* v, MotionWidget& mw) {
    v->layout(mw.getLeft(), mw.getTop(), mw.getWidth(), mw.getHeight());
    float a = mw.getAlpha();
    if (!std::isnan(a) && a != 1.0f) v->setAlpha(a);
    float rz = mw.getRotationZ();
    if (!std::isnan(rz) && rz != 0.0f) v->setRotation(rz);
    float rx = mw.getRotationX();
    if (!std::isnan(rx) && rx != 0.0f) v->setRotationX(rx);
    float ry = mw.getRotationY();
    if (!std::isnan(ry) && ry != 0.0f) v->setRotationY(ry);
    float sx = mw.getScaleX();
    if (!std::isnan(sx) && sx != 1.0f) v->setScaleX(sx);
    float sy = mw.getScaleY();
    if (!std::isnan(sy) && sy != 1.0f) v->setScaleY(sy);
    float tx = mw.getTranslationX();
    if (!std::isnan(tx) && tx != 0.0f) v->setTranslationX(tx);
    float ty = mw.getTranslationY();
    if (!std::isnan(ty) && ty != 0.0f) v->setTranslationY(ty);
}
} // namespace

MotionLayout::MotionLayout(Context* ctx, const AttributeSet& attrs)
    : ConstraintLayout(ctx, attrs) {}

MotionLayout::MotionLayout(int width, int height)
    : ConstraintLayout(width, height) {}

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
        mMotions[id] = m;
    }
}

void MotionLayout::captureAndBuild() {
            (void*)mStartSet, (void*)mEndSet, getMeasuredWidth(), getMeasuredHeight());
    captureState(mStartSet, mStartWidgets);
    captureState(mEndSet, mEndWidgets);
    buildMotions();
    mCaptured = !mMotions.empty();
            (int)mMotions.size(), (int)mCaptured);
    if (mCaptured) applyMotion();
}

void MotionLayout::setTransition(ConstraintSet* start, ConstraintSet* end) {
    mStartSet = start;
    mEndSet = end;
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

} // namespace cdroid
