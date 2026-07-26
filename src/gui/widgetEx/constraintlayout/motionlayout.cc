/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.motion.widget.MotionLayout.
 */
#include <widgetEx/constraintlayout/motionlayout.h>

#include <porting/cdlog.h>
#include <view/view.h>

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
        layout(getLeft(), getTop(), getRight(), getBottom());
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

void MotionLayout::setTransition(ConstraintSet* start, ConstraintSet* end) {
    mStartSet = start;
    mEndSet = end;
    // Need measure specs to capture; if not yet measured, derive exactly from our size.
    if (mWidthSpec == 0) {
        mWidthSpec = View::MeasureSpec::makeMeasureSpec(getWidth(), View::MeasureSpec::EXACTLY);
        mHeightSpec = View::MeasureSpec::makeMeasureSpec(getHeight(), View::MeasureSpec::EXACTLY);
    }
    captureState(start, mStartWidgets);
    captureState(end, mEndWidgets);
    buildMotions();
    mCaptured = !mMotions.empty();
    if (mCaptured) applyMotion();
}

void MotionLayout::setProgress(float progress) {
    mProgress = progress;
    if (mCaptured) applyMotion();
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
    // During normal (non-capture) measure, if a transition is captured, apply the current progress
    // so the laid-out children reflect the interpolated state.
    if (!mInCapture && mCaptured) {
        applyMotion();
    }
}

} // namespace cdroid
