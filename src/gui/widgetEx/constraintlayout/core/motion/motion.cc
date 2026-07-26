/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.Motion.
 */
#include <widgetEx/constraintlayout/core/motion/motion.h>

#include <cmath>

namespace cdroid {

Motion::Motion() {
    mStartMotionPath.mPosition = 0;
    mStartMotionPath.mTime = 0;
    mEndMotionPath.mPosition = 1;
    mEndMotionPath.mTime = 1;
}

float Motion::lerp(float start, float end, float defaultValue, float progress) {
    bool startUnset = std::isnan(start);
    bool endUnset   = std::isnan(end);
    if (startUnset && endUnset) return NAN;
    if (startUnset) start = defaultValue;
    if (endUnset)   end = defaultValue;
    return start + progress * (end - start);
}

void Motion::setView(MotionWidget* view) { mView = view; }

void Motion::setStart(MotionWidget* mw) {
    mStartMotionPath.mTime = 0;
    mStartMotionPath.mPosition = 0;
    mStartMotionPath.setBounds((float) mw->getX(), (float) mw->getY(),
                               (float) mw->getWidth(), (float) mw->getHeight());
    mStartPoint.setState(mw);
}

void Motion::setEnd(MotionWidget* mw) {
    mEndMotionPath.mTime = 1;
    mEndMotionPath.mPosition = 1;
    mEndMotionPath.setBounds((float) mw->getLeft(), (float) mw->getTop(),
                             (float) mw->getWidth(), (float) mw->getHeight());
    mEndPoint.setState(mw);
}

void Motion::setStartState(MotionWidget* mw) { setStart(mw); }
void Motion::setEndState(MotionWidget* mw)   { setEnd(mw); }

void Motion::setup(int /*parentWidth*/, int /*parentHeight*/, float /*transitionDuration*/) {
    // MVP: linear interpolation needs no precomputed tables. The CurveFit[]/keyframe engine is
    // deferred (fidelity TODO).
}

void Motion::buildRect(float p, std::vector<float>& path, int offset) {
    float x = lerp(mStartMotionPath.mX, mEndMotionPath.mX, 0, p);
    float y = lerp(mStartMotionPath.mY, mEndMotionPath.mY, 0, p);
    float w = lerp(mStartMotionPath.mWidth, mEndMotionPath.mWidth, 0, p);
    float h = lerp(mStartMotionPath.mHeight, mEndMotionPath.mHeight, 0, p);
    if (std::isnan(w)) w = mStartMotionPath.mWidth;
    if (std::isnan(h)) h = mStartMotionPath.mHeight;
    // 4 corners (8 floats): (x,y)-(x+w,y)-(x+w,y+h)-(x,y+h)
    path[offset + 0] = x;       path[offset + 1] = y;
    path[offset + 2] = x + w;   path[offset + 3] = y;
    path[offset + 4] = x + w;   path[offset + 5] = y + h;
    path[offset + 6] = x;       path[offset + 7] = y + h;
}

void Motion::interpolate(MotionWidget* child, float progress) {
    // Position + size (linear).
    float x = lerp(mStartMotionPath.mX, mEndMotionPath.mX, 0, progress);
    float y = lerp(mStartMotionPath.mY, mEndMotionPath.mY, 0, progress);
    float w = lerp(mStartMotionPath.mWidth, mEndMotionPath.mWidth, mStartMotionPath.mWidth, progress);
    float h = lerp(mStartMotionPath.mHeight, mEndMotionPath.mHeight, mStartMotionPath.mHeight, progress);
    if (std::isnan(w)) w = mStartMotionPath.mWidth;
    if (std::isnan(h)) h = mStartMotionPath.mHeight;
    child->setBounds((int) x, (int) y, (int) (x + w), (int) (y + h));

    // Visibility / alpha (alpha 0 when the end is invisible, matching WidgetFrame.interpolate).
    float startAlpha = (mStartPoint.mVisibility != MotionWidget::VISIBLE) ? 0 : mStartPoint.mAlpha;
    float endAlpha   = (mEndPoint.mVisibility   != MotionWidget::VISIBLE) ? 0 : mEndPoint.mAlpha;
    if (std::isnan(startAlpha) && !std::isnan(endAlpha)) startAlpha = 1;
    if (!std::isnan(startAlpha) && std::isnan(endAlpha)) endAlpha = 1;
    float alpha = lerp(startAlpha, endAlpha, 1, progress);
    if (!std::isnan(alpha)) child->setAlpha(alpha);

    child->setRotationX(lerp(mStartPoint.mRotationX, mEndPoint.mRotationX, 0, progress));
    child->setRotationY(lerp(mStartPoint.rotationY, mEndPoint.rotationY, 0, progress));
    child->setRotationZ(lerp(mStartPoint.mRotation, mEndPoint.mRotation, 0, progress));
    child->setScaleX(lerp(mStartPoint.mScaleX, mEndPoint.mScaleX, 1, progress));
    child->setScaleY(lerp(mStartPoint.mScaleY, mEndPoint.mScaleY, 1, progress));
    child->setPivotX(lerp(mStartPoint.mPivotX, mEndPoint.mPivotX, 0.5f, progress));
    child->setPivotY(lerp(mStartPoint.mPivotY, mEndPoint.mPivotY, 0.5f, progress));
    child->setTranslationX(lerp(mStartPoint.mTranslationX, mEndPoint.mTranslationX, 0, progress));
    child->setTranslationY(lerp(mStartPoint.mTranslationY, mEndPoint.mTranslationY, 0, progress));
    child->setTranslationZ(lerp(mStartPoint.mTranslationZ, mEndPoint.mTranslationZ, 0, progress));
}

// TypedValues motion-property dispatch — store on this controller for the deferred engine.
bool Motion::setValue(int id, int value) {
    if (id == MotionType::TYPE_PATHMOTION_ARC)       { mPathMotionArc = value; return true; }
    if (id == MotionType::TYPE_DRAW_PATH)            { mDrawPath = value; return true; }
    return false;
}
bool Motion::setValue(int id, float value) {
    if (id == MotionType::TYPE_STAGGER)     { mStagger = value; return true; }
    if (id == MotionType::TYPE_PATH_ROTATE) { mPathRotate = value; return true; }
    return false;
}
bool Motion::setValue(int id, const std::string& value) {
    if (id == MotionType::TYPE_EASING)              { mTransitionEasing = value; return true; }
    if (id == MotionType::TYPE_ANIMATE_RELATIVE_TO) { mAnimateRelativeTo = value; return true; }
    return false;
}
bool Motion::setValue(int /*id*/, bool /*value*/) { return false; }
int  Motion::getId(const std::string& name) { return MotionType::getId(name); }

} // namespace cdroid
