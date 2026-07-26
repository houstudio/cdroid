/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.MotionPaths.
 */
#include <widgetEx/constraintlayout/core/motion/motionpaths.h>

#include <cmath>

namespace cdroid {

MotionPaths::MotionPaths() = default;

bool MotionPaths::diff(float a, float b) {
    if (std::isnan(a) || std::isnan(b)) {
        return std::isnan(a) != std::isnan(b);
    }
    return std::fabs(a - b) > 0.000001f;
}

void MotionPaths::setBounds(float x, float y, float w, float h) {
    mX = x; mY = y; mWidth = w; mHeight = h;
}

void MotionPaths::fillStandard(std::vector<double>& data, const std::vector<int>& toUse) const {
    const float set[] = {mPosition, mX, mY, mWidth, mHeight, mPathRotate};
    int c = 0;
    for (int i : toUse) {
        if (i >= 0 && i < (int)(sizeof(set) / sizeof(set[0]))) {
            data[c++] = set[i];
        }
    }
}

void MotionPaths::different(const MotionPaths& points, std::vector<bool>& mask,
                            std::vector<std::string>& /*custom*/, bool arcMode) const {
    // std::vector<bool> is a proxy — assign the combined bool rather than |=.
    bool diffx = diff(mX, points.mX);
    bool diffy = diff(mY, points.mY);
    int c = 0;
    mask[c] = mask[c] | diff(mPosition, points.mPosition); c++;
    mask[c] = mask[c] | (diffx || diffy || arcMode); c++;
    mask[c] = mask[c] | (diffx || diffy || arcMode); c++;
    mask[c] = mask[c] | diff(mWidth, points.mWidth); c++;
    mask[c] = mask[c] | diff(mHeight, points.mHeight); c++;
}

bool MotionPaths::hasCustomData(const std::string& name) const {
    return mCustomAttributes.find(name) != mCustomAttributes.end();
}

int MotionPaths::getCustomDataCount(const std::string& name) const {
    auto it = mCustomAttributes.find(name);
    return (it != mCustomAttributes.end()) ? it->second.numberOfInterpolatedValues() : 0;
}

int MotionPaths::getCustomData(const std::string& name, std::vector<double>& value, int offset) const {
    auto it = mCustomAttributes.find(name);
    if (it == mCustomAttributes.end()) return 0;
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

// --- deferred (chunk 5c, ported with Motion + MotionKeyPosition) ---
void MotionPaths::initCartesian(MotionKeyPosition* /*c*/, MotionPaths* /*start*/, MotionPaths* /*end*/) {}
void MotionPaths::initAxis(MotionKeyPosition* /*c*/, MotionPaths* /*start*/, MotionPaths* /*end*/) {}
void MotionPaths::initPath(MotionKeyPosition* /*c*/, MotionPaths* /*start*/, MotionPaths* /*end*/) {}

void MotionPaths::getRect(const std::vector<int>& /*toUse*/, std::vector<double>& /*data*/,
                          std::vector<float>& /*path*/, int /*offset*/) {
    // TODO(chunk 5c): read interpolated bounds from the CurveFit at a progress.
}

void MotionPaths::setView(float /*position*/, Motion* /*motion*/, MotionWidget* /*childView*/) {
    // TODO(chunk 5c): apply the interpolated rect/transforms to the child view.
}

} // namespace cdroid
