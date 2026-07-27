/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.StepCurve.
 *
 * Translates a series of floats into a continuous easing curve via a monotonic spline.
 * Used with "spline(0,0.3,0.3,0.5,...,0.9,1)" (must start at 0 and end at 1).
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_STEP_CURVE_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_STEP_CURVE_H

#include <memory>
#include <string>

#include <widgetEx/constraintlayout/core/motion/easing.h>
#include <widgetEx/constraintlayout/core/motion/monotoniccurvefit.h>

namespace cdroid {

class StepCurve : public Easing {
  public:
    explicit StepCurve(const std::string& configString);
    double get(double x) const override;
    double getDiff(double x) const override;
  private:
    static MonotonicCurveFit genSpline(const std::vector<double>& values);
    std::unique_ptr<MonotonicCurveFit> mCurveFit;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_STEP_CURVE_H
