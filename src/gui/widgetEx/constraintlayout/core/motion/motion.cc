/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.Motion.
 */
#include <widgetEx/constraintlayout/core/motion/motion.h>

#include <algorithm>
#include <cmath>
#include <utility>

#include <widgetEx/constraintlayout/core/motion/motionkeyattributes.h>
#include <widgetEx/constraintlayout/core/motion/motionkeyposition.h>
#include <widgetEx/constraintlayout/core/motion/motionkeycycle.h>

namespace cdroid {

Motion::Motion() {
    mStartMotionPath.mPosition = 0;
    mStartMotionPath.mTime = 0;
    mEndMotionPath.mPosition = 1;
    mEndMotionPath.mTime = 1;
}

Motion::~Motion() {
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
    mPathDirty = true;
}

void Motion::setEnd(MotionWidget* mw) {
    mEndMotionPath.mTime = 1;
    mEndMotionPath.mPosition = 1;
    mEndMotionPath.setBounds((float) mw->getLeft(), (float) mw->getTop(),
                             (float) mw->getWidth(), (float) mw->getHeight());
    mEndPoint.setState(mw);
    mPathDirty = true;
}

void Motion::setStartState(MotionWidget* mw) { setStart(mw); }
void Motion::setEndState(MotionWidget* mw)   { setEnd(mw); }

void Motion::setup(int /*parentWidth*/, int /*parentHeight*/, float /*transitionDuration*/) {
    // MVP: linear interpolation needs no precomputed tables. The CurveFit[]/keyframe engine is
    // deferred (fidelity TODO).
    buildEasing();
}

void Motion::buildEasing() {
    if (!mEasingDirty) return;
    mEasingDirty = false;
    mEasing = Easing::getInterpolator(mTransitionEasing); // empty -> nullptr (linear)
}

float Motion::eased(float progress) const {
    if (mEasing == nullptr) return progress;
    return (float) mEasing->get(progress);
}

void Motion::buildPath() {
    if (!mPathDirty) return;
    mPathDirty = false;
    mPositionCurveFit.reset();
    mArcCurveFit.reset();
    const float sX = mStartMotionPath.mX, sY = mStartMotionPath.mY;
    const float sW = mStartMotionPath.mWidth, sH = mStartMotionPath.mHeight;
    const float eX = mEndMotionPath.mX, eY = mEndMotionPath.mY;
    const float eW = mEndMotionPath.mWidth, eH = mEndMotionPath.mHeight;

    // pathMotionArc: the widget travels a quarter-ellipse arc from start to end (no keyframes).
    // x,y come from the arc; w,h interpolate linearly (ArcCurveFit is 2-D).
    if (mPathMotionArc >= 0 && mPositionKeys.empty()) {
        std::vector<int> arcModes = { mPathMotionArc };
        std::vector<double> time = { 0.0, 1.0 };
        std::vector<std::vector<double>> y = { { (double) sX, (double) sY },
                                               { (double) eX, (double) eY } };
        mArcCurveFit = CurveFit::getArc(arcModes, time, y);
        return;
    }

    // Otherwise: monotonic spline through start + KeyPosition control points + end (x,y,w,h).
    const float sCx = sX + sW / 2.0f, sCy = sY + sH / 2.0f;
    const float eCx = eX + eW / 2.0f, eCy = eY + eH / 2.0f;
    const float pvx = eCx - sCx, pvy = eCy - sCy;
    const float dW = eW - sW, dH = eH - sH;

    std::vector<double> time;
    std::vector<std::vector<double>> y;
    auto addPt = [&](double p, float x, float yy, float w, float h) {
        time.push_back(p);
        y.push_back({(double) x, (double) yy, (double) w, (double) h});
    };
    addPt(0.0, sX, sY, sW, sH);
    for (auto* k : mPositionKeys) {
        const float pos = k->mFramePosition / 100.0f;
        if (pos <= 0.0f || pos >= 1.0f) continue;
        const float sw   = std::isnan(k->mPercentWidth)  ? pos : k->mPercentWidth;
        const float sh   = std::isnan(k->mPercentHeight) ? pos : k->mPercentHeight;
        const float dxdx = std::isnan(k->mPercentX)      ? pos : k->mPercentX;
        const float dydx = std::isnan(k->mAltPercentY)   ? 0.0f : k->mAltPercentY;
        const float dydy = std::isnan(k->mPercentY)      ? pos : k->mPercentY;
        const float dxdy = std::isnan(k->mAltPercentX)   ? 0.0f : k->mAltPercentX;
        const float pw = sW + dW * sw;
        const float ph = sH + dH * sh;
        const float cx = sCx + pvx * dxdx + pvy * dxdy;
        const float cy = sCy + pvy * dydy + pvx * dydx;
        addPt(pos, cx - pw / 2.0f, cy - ph / 2.0f, pw, ph);
    }
    addPt(1.0, eX, eY, eW, eH);
    if (time.size() >= 2) {
        mPositionCurveFit = CurveFit::get(CurveFit::SPLINE, time, y);
    }
}

void Motion::addKey(MotionKeyAttributes* key) {
    mAttributeKeys.push_back(key);
    std::sort(mAttributeKeys.begin(), mAttributeKeys.end(),
              [](MotionKeyAttributes* a, MotionKeyAttributes* b) {
                  return a->mFramePosition < b->mFramePosition;
              });
}

void Motion::addKey(MotionKeyPosition* key) {
    mPositionKeys.push_back(key);
    std::sort(mPositionKeys.begin(), mPositionKeys.end(),
              [](MotionKeyPosition* a, MotionKeyPosition* b) {
                  return a->mFramePosition < b->mFramePosition;
              });
    mPathDirty = true; // position keyframes changed -> rebuild the path CurveFit
}

void Motion::addKey(MotionKeyCycle* key) {
    mCycleKeys.push_back(key);
}

float Motion::keyframed(float progress, float startVal, float endVal, float defaultValue,
                        float (*get)(const MotionKeyAttributes*)) const {
    float s = std::isnan(startVal) ? defaultValue : startVal;
    float e = std::isnan(endVal)   ? defaultValue : endVal;
    std::vector<std::pair<float, float>> pts;
    pts.emplace_back(0.0f, s);
    for (auto* k : mAttributeKeys) {
        float v = get(k);
        float p = k->mFramePosition / 100.0f;
        if (!std::isnan(v) && p > 0.0f && p < 1.0f) pts.emplace_back(p, v);
    }
    pts.emplace_back(1.0f, e);
    if (progress <= pts.front().first) return pts.front().second;
    if (progress >= pts.back().first) return pts.back().second;
    for (size_t i = 0; i + 1 < pts.size(); i++) {
        if (progress >= pts[i].first && progress <= pts[i + 1].first) {
            float span = pts[i + 1].first - pts[i].first;
            float f = (span != 0) ? (progress - pts[i].first) / span : 0.0f;
            return pts[i].second + f * (pts[i + 1].second - pts[i].second);
        }
    }
    return e;
}

void Motion::buildRect(float p, std::vector<float>& path, int offset) {
    buildEasing();
    p = eased(p);
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
    buildEasing();
    progress = eased(progress);

    // Position + size: quarter-ellipse arc (pathMotionArc), spline through KeyPositions, or lerp.
    buildPath();
    float x, y, w, h;
    if (mArcCurveFit) {
        x = (float) mArcCurveFit->getPos(progress, 0);
        y = (float) mArcCurveFit->getPos(progress, 1);
        w = lerp(mStartMotionPath.mWidth, mEndMotionPath.mWidth, mStartMotionPath.mWidth, progress);
        h = lerp(mStartMotionPath.mHeight, mEndMotionPath.mHeight, mStartMotionPath.mHeight, progress);
        if (std::isnan(w)) w = mStartMotionPath.mWidth;
        if (std::isnan(h)) h = mStartMotionPath.mHeight;
    } else if (mPositionCurveFit) {
        x = (float) mPositionCurveFit->getPos(progress, 0);
        y = (float) mPositionCurveFit->getPos(progress, 1);
        w = (float) mPositionCurveFit->getPos(progress, 2);
        h = (float) mPositionCurveFit->getPos(progress, 3);
    } else {
        x = lerp(mStartMotionPath.mX, mEndMotionPath.mX, 0, progress);
        y = lerp(mStartMotionPath.mY, mEndMotionPath.mY, 0, progress);
        w = lerp(mStartMotionPath.mWidth, mEndMotionPath.mWidth, mStartMotionPath.mWidth, progress);
        h = lerp(mStartMotionPath.mHeight, mEndMotionPath.mHeight, mStartMotionPath.mHeight, progress);
        if (std::isnan(w)) w = mStartMotionPath.mWidth;
        if (std::isnan(h)) h = mStartMotionPath.mHeight;
    }
    child->setBounds((int) x, (int) y, (int) (x + w), (int) (y + h));

    // Visibility / alpha (alpha 0 when the end is invisible, matching WidgetFrame.interpolate).
    float startAlpha = (mStartPoint.mVisibility != MotionWidget::VISIBLE) ? 0 : mStartPoint.mAlpha;
    float endAlpha   = (mEndPoint.mVisibility   != MotionWidget::VISIBLE) ? 0 : mEndPoint.mAlpha;
    if (std::isnan(startAlpha) && !std::isnan(endAlpha)) startAlpha = 1;
    if (!std::isnan(startAlpha) && std::isnan(endAlpha)) endAlpha = 1;
    float alpha = keyframed(progress, startAlpha, endAlpha, 1,
                            [](const MotionKeyAttributes* k) { return k->mAlpha; });
    // Cycle overlay: superimpose wave oscillations on the base alpha.
    for (auto* c : mCycleKeys) {
        if (!std::isnan(c->mAlpha) && !std::isnan(c->mWavePeriod)) {
            float wave = std::sin(2.0 * M_PI * progress * c->mWavePeriod + c->mWavePhase);
            alpha += wave * c->mAlpha + c->mWaveOffset;
        }
    }
    if (!std::isnan(alpha)) child->setAlpha(alpha);

    child->setRotationX(keyframed(progress, mStartPoint.mRotationX, mEndPoint.mRotationX, 0,
        [](const MotionKeyAttributes* k) { return k->mRotationX; }));
    child->setRotationY(keyframed(progress, mStartPoint.rotationY, mEndPoint.rotationY, 0,
        [](const MotionKeyAttributes* k) { return k->mRotationY; }));
    child->setRotationZ(keyframed(progress, mStartPoint.mRotation, mEndPoint.mRotation, 0,
        [](const MotionKeyAttributes* k) { return k->mRotation; }));
    child->setScaleX(keyframed(progress, mStartPoint.mScaleX, mEndPoint.mScaleX, 1,
        [](const MotionKeyAttributes* k) { return k->mScaleX; }));
    child->setScaleY(keyframed(progress, mStartPoint.mScaleY, mEndPoint.mScaleY, 1,
        [](const MotionKeyAttributes* k) { return k->mScaleY; }));
    child->setPivotX(keyframed(progress, mStartPoint.mPivotX, mEndPoint.mPivotX, 0.5f,
        [](const MotionKeyAttributes* k) { return k->mPivotX; }));
    child->setPivotY(keyframed(progress, mStartPoint.mPivotY, mEndPoint.mPivotY, 0.5f,
        [](const MotionKeyAttributes* k) { return k->mPivotY; }));
    child->setTranslationX(keyframed(progress, mStartPoint.mTranslationX, mEndPoint.mTranslationX, 0,
        [](const MotionKeyAttributes* k) { return k->mTranslationX; }));
    child->setTranslationY(keyframed(progress, mStartPoint.mTranslationY, mEndPoint.mTranslationY, 0,
        [](const MotionKeyAttributes* k) { return k->mTranslationY; }));
    child->setTranslationZ(keyframed(progress, mStartPoint.mTranslationZ, mEndPoint.mTranslationZ, 0,
        [](const MotionKeyAttributes* k) { return k->mTranslationZ; }));
}

void Motion::getDpDt(float pos, float locationX, float locationY, float out[2]) {
    // Slope of the anchor point (locationX,locationY on the widget) w.r.t. progress:
    // dAnchor = dPosition + loc * dSize (anchor = top-left + loc*size).
    buildPath();
    if (mArcCurveFit) {
        out[0] = (float)(mArcCurveFit->getSlope(pos, 0)
                         + locationX * (mEndMotionPath.mWidth - mStartMotionPath.mWidth));
        out[1] = (float)(mArcCurveFit->getSlope(pos, 1)
                         + locationY * (mEndMotionPath.mHeight - mStartMotionPath.mHeight));
    } else if (mPositionCurveFit) {
        out[0] = (float)(mPositionCurveFit->getSlope(pos, 0)
                         + locationX * mPositionCurveFit->getSlope(pos, 2));
        out[1] = (float)(mPositionCurveFit->getSlope(pos, 1)
                         + locationY * mPositionCurveFit->getSlope(pos, 3));
    } else {
        out[0] = out[1] = 0;
    }
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
    if (id == MotionType::TYPE_EASING)              { mTransitionEasing = value; mEasingDirty = true; return true; }
    if (id == MotionType::TYPE_ANIMATE_RELATIVE_TO) { mAnimateRelativeTo = value; return true; }
    return false;
}
bool Motion::setValue(int /*id*/, bool /*value*/) { return false; }
int  Motion::getId(const std::string& name) { return MotionType::getId(name); }

} // namespace cdroid
