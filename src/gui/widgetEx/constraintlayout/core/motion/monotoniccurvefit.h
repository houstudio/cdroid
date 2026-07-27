/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.MonotonicCurveFit.
 *
 * Monotone cubic Hermite spline interpolation in multiple dimensions (Fritsch-Carlson tangents).
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_UTILS_MONOTONIC_CURVE_FIT_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_UTILS_MONOTONIC_CURVE_FIT_H

#include <string>
#include <vector>

#include <widgetEx/constraintlayout/core/motion/curvefit.h>

namespace cdroid {

class MonotonicCurveFit : public CurveFit {
  public:
    MonotonicCurveFit(const std::vector<double>& time, const std::vector<std::vector<double>>& y);

    void   getPos(double t, std::vector<double>& v) override;
    void   getPos(double t, std::vector<float>& v) override;
    double getPos(double t, int j) override;
    void   getSlope(double t, std::vector<double>& v) override;
    double getSlope(double t, int j) override;
    std::vector<double> getTimePoints() override;

    // Build a periodic wave spline from a "spline(v0,v1,...)" config string (values 0..1).
    static MonotonicCurveFit buildWave(const std::string& configString);

  private:
    static double interpolate(double h, double x, double y1, double y2, double t1, double t2);
    static double diff(double h, double x, double y1, double y2, double t1, double t2);
    static MonotonicCurveFit buildWave(const std::vector<double>& values);

    std::vector<double> mT;
    std::vector<std::vector<double>> mY;
    std::vector<std::vector<double>> mTangent;
    bool mExtrapolate = true;
    std::vector<double> mSlopeTemp;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_UTILS_MONOTONIC_CURVE_FIT_H
