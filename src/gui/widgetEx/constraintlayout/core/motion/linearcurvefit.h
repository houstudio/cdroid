/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.LinearCurveFit.
 *
 * Simple linear interpolation in multiple dimensions.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_UTILS_LINEAR_CURVE_FIT_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_UTILS_LINEAR_CURVE_FIT_H

#include <vector>

#include <widgetEx/constraintlayout/core/motion/curvefit.h>

namespace cdroid {

class LinearCurveFit : public CurveFit {
public:
    LinearCurveFit(const std::vector<double>& time, const std::vector<std::vector<double>>& y);

    void   getPos(double t, std::vector<double>& v) override;
    void   getPos(double t, std::vector<float>& v) override;
    double getPos(double t, int j) override;
    void   getSlope(double t, std::vector<double>& v) override;
    double getSlope(double t, int j) override;
    std::vector<double> getTimePoints() override;

private:
    // Length traveled by the first two dims assuming x,y. Added for future work; unused upstream.
    double getLength2D(double t);

    std::vector<double> mT;
    std::vector<std::vector<double>> mY;
    double mTotalLength; // NaN until computed
    bool mExtrapolate = true;
    std::vector<double> mSlopeTemp;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_UTILS_LINEAR_CURVE_FIT_H
