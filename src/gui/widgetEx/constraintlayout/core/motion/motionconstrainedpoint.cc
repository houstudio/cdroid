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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.MotionConstrainedPoint.
 */
#include <widgetEx/constraintlayout/core/motion/motionconstrainedpoint.h>

#include <cmath>

#include <widgetEx/constraintlayout/core/motion/customvariable.h>
#include <widgetEx/constraintlayout/core/motion/motionwidget.h>
#include <widgetEx/constraintlayout/core/motion/typedvalues.h>

namespace cdroid {

MotionConstrainedPoint::MotionConstrainedPoint() = default;

bool MotionConstrainedPoint::diff(float a, float b) {
    if (std::isnan(a) || std::isnan(b)) {
        return std::isnan(a) != std::isnan(b);
    }
    return std::fabs(a - b) > 0.000001f;
}

void MotionConstrainedPoint::setBounds(float x, float y, float w, float h) {
    mX = x;
    mY = y;
    mWidth = w;
    mHeight = h;
}

void MotionConstrainedPoint::different(const MotionConstrainedPoint& points,
                                       std::unordered_set<std::string>& keySet) const {
    using A = TypedValues::AttributesType;
    if (diff(mAlpha, points.mAlpha)) keySet.insert(A::S_ALPHA);
    if (diff(mElevation, points.mElevation)) keySet.insert(A::S_TRANSLATION_Z);
    if (mVisibility != points.mVisibility
            && mVisibilityMode == MotionWidget::VISIBILITY_MODE_NORMAL
            && (mVisibility == MotionWidget::VISIBLE || points.mVisibility == MotionWidget::VISIBLE)) {
        keySet.insert(A::S_ALPHA);
    }
    if (diff(mRotation, points.mRotation))  keySet.insert(A::S_ROTATION_Z);
    if (!(std::isnan(mPathRotate) && std::isnan(points.mPathRotate))) keySet.insert(A::S_PATH_ROTATE);
    if (!(std::isnan(mProgress) && std::isnan(points.mProgress)))     keySet.insert(A::S_PROGRESS);
    if (diff(mRotationX, points.mRotationX)) keySet.insert(A::S_ROTATION_X);
    if (diff(rotationY, points.rotationY))   keySet.insert(A::S_ROTATION_Y);
    if (diff(mPivotX, points.mPivotX))       keySet.insert(A::S_PIVOT_X);
    if (diff(mPivotY, points.mPivotY))       keySet.insert(A::S_PIVOT_Y);
    if (diff(mScaleX, points.mScaleX))       keySet.insert(A::S_SCALE_X);
    if (diff(mScaleY, points.mScaleY))       keySet.insert(A::S_SCALE_Y);
    if (diff(mTranslationX, points.mTranslationX)) keySet.insert(A::S_TRANSLATION_X);
    if (diff(mTranslationY, points.mTranslationY)) keySet.insert(A::S_TRANSLATION_Y);
    if (diff(mTranslationZ, points.mTranslationZ)) keySet.insert(A::S_TRANSLATION_Z);
    if (diff(mElevation, points.mElevation)) keySet.insert(A::S_ELEVATION);
}

void MotionConstrainedPoint::different(const MotionConstrainedPoint& points,
                                       std::vector<bool>& mask,
                                       std::vector<std::string>& /*custom*/) const {
    // std::vector<bool> is a proxy; assign the combined bool rather than |= on the reference.
    int c = 0;
    mask[c] = mask[c] | diff(mPosition, points.mPosition);
    c++;
    mask[c] = mask[c] | diff(mX, points.mX);
    c++;
    mask[c] = mask[c] | diff(mY, points.mY);
    c++;
    mask[c] = mask[c] | diff(mWidth, points.mWidth);
    c++;
    mask[c] = mask[c] | diff(mHeight, points.mHeight);
    c++;
}

void MotionConstrainedPoint::fillStandard(std::vector<double>& data, const std::vector<int>& toUse) const {
    const float set[] = {mPosition, mX, mY, mWidth, mHeight, mAlpha, mElevation,
                         mRotation, mRotationX, rotationY, mScaleX, mScaleY, mPivotX,
                         mPivotY, mTranslationX, mTranslationY, mTranslationZ, mPathRotate
                        };
    int c = 0;
    for (int i : toUse) {
        if (i < (int)(sizeof(set) / sizeof(set[0]))) {
            data[c++] = set[i];
        }
    }
}

bool MotionConstrainedPoint::hasCustomData(const std::string& name) const {
    return mCustomVariable.find(name) != mCustomVariable.end();
}

int MotionConstrainedPoint::getCustomDataCount(const std::string& name) const {
    auto it = mCustomVariable.find(name);
    return (it != mCustomVariable.end()) ? it->second.numberOfInterpolatedValues() : 0;
}

int MotionConstrainedPoint::getCustomData(const std::string& name, std::vector<double>& value, int offset) const {
    auto it = mCustomVariable.find(name);
    if (it == mCustomVariable.end()) return 0;
    const CustomVariable& a = it->second;
    if (a.numberOfInterpolatedValues() == 1) {
        value[offset] = a.getValueToInterpolate();
        return 1;
    }
    int n = a.numberOfInterpolatedValues();
    std::vector<float> f(n);
    a.getValuesToInterpolate(f);
    for (int i = 0; i < n; i++) value[offset++] = f[i];
    return n;
}

void MotionConstrainedPoint::applyParameters(MotionWidget* view) {
    mVisibility = view->getVisibility();
    mAlpha = (view->getVisibility() != MotionWidget::VISIBLE) ? 0.0f : view->getAlpha();
    mApplyElevation = false;
    mRotation = view->getRotationZ();
    mRotationX = view->getRotationX();
    rotationY = view->getRotationY();
    mScaleX = view->getScaleX();
    mScaleY = view->getScaleY();
    mPivotX = view->getPivotX();
    mPivotY = view->getPivotY();
    mTranslationX = view->getTranslationX();
    mTranslationY = view->getTranslationY();
    mTranslationZ = view->getTranslationZ();
    auto names = view->getCustomAttributeNames();
    for (const std::string& s : names) {
        CustomVariable* attr = view->getCustomAttribute(s);
        if (attr != nullptr && attr->isContinuous()) {
            mCustomVariable[s] = *attr;
        }
    }
}

void MotionConstrainedPoint::setState(MotionWidget* view) {
    setBounds((float) view->getX(), (float) view->getY(),
              (float) view->getWidth(), (float) view->getHeight());
    applyParameters(view);
}

void MotionConstrainedPoint::addValues(std::unordered_map<std::string, SplineSet*>& /*splines*/,
                                       int /*mFramePosition*/) {
    // TODO: populate the per-property SplineSets with this point's values. Ported with the
    // spline-set system (SplineSet/KeyCycleOscillator) — the Motion MVP uses a simpler path.
}

} // namespace cdroid
