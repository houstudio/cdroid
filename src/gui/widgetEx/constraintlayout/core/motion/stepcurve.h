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
