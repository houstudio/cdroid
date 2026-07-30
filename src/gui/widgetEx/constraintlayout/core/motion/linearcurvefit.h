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
