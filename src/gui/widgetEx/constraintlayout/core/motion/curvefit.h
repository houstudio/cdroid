/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.CurveFit.
 *
 * Base for curve fitting / interpolation. Implementations must be differentiable and able to
 * extrapolate beyond their sample points. Java double[]/double[][] map to std::vector<double> /
 * std::vector<std::vector<double>>; the in-place getPos/getSlope contract is preserved (the caller
 * sizes the output vector to the dimensionality).
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_UTILS_CURVE_FIT_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_UTILS_CURVE_FIT_H

#include <memory>
#include <vector>

namespace cdroid {

class CurveFit {
  public:
    static const int SPLINE    = 0;
    static const int LINEAR    = 1;
    static const int CONSTANT  = 2;

    virtual ~CurveFit() = default;

    // Factory: SPLINE -> MonotonicCurveFit, CONSTANT -> Constant, default -> LinearCurveFit.
    static std::unique_ptr<CurveFit> get(int type, const std::vector<double>& time,
                                         const std::vector<std::vector<double>>& y);
    // Arc path curve fit: per-segment quarter-ellipses (ArcCurveFit), used for pathMotionArc.
    static std::unique_ptr<CurveFit> getArc(const std::vector<int>& arcModes,
                                            const std::vector<double>& time,
                                            const std::vector<std::vector<double>>& y);

    // `v` is written in place and must be pre-sized to the curve's dimensionality.
    virtual void   getPos(double t, std::vector<double>& v) = 0;
    virtual void   getPos(double t, std::vector<float>& v) = 0;
    virtual double getPos(double t, int j) = 0;
    virtual void   getSlope(double t, std::vector<double>& v) = 0;
    virtual double getSlope(double t, int j) = 0;
    virtual std::vector<double> getTimePoints() = 0;

    // A curve that is constant everywhere (used when only one sample point is given).
    // Defined out-of-line below (nested class cannot derive from its enclosing class inline).
    class Constant;
};

// A curve that is constant everywhere (used when only one sample point is given).
class CurveFit::Constant : public CurveFit {
  public:
    Constant(double time, const std::vector<double>& value);
    void   getPos(double t, std::vector<double>& v) override;
    void   getPos(double t, std::vector<float>& v) override;
    double getPos(double t, int j) override;
    void   getSlope(double t, std::vector<double>& v) override;
    double getSlope(double t, int j) override;
    std::vector<double> getTimePoints() override;
  private:
    double mTime;
    std::vector<double> mValue;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_UTILS_CURVE_FIT_H
