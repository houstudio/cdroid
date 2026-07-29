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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.ArcCurveFit.
 *
 * Stitches the x,y path together with quarter-ellipses (the Material "arc around a corner" motion).
 * Each segment between consecutive points is one Arc; linear segments are used where the points are
 * colinear or the mode is ARC_START_LINEAR. A lookup table arc-length-parameterizes the ellipse so
 * the widget travels it at constant velocity. Used by Motion when pathMotionArc is set.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_ARC_CURVE_FIT_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_ARC_CURVE_FIT_H

#include <memory>
#include <vector>

#include <widgetEx/constraintlayout/core/motion/curvefit.h>

namespace cdroid {

class ArcCurveFit : public CurveFit {
  public:
    static const int ARC_START_LINEAR     = 0;
    static const int ARC_START_VERTICAL   = 1;
    static const int ARC_START_HORIZONTAL = 2;
    static const int ARC_START_FLIP       = 3;
    static const int ARC_BELOW            = 4;
    static const int ARC_ABOVE            = 5;

    ArcCurveFit(const std::vector<int>& arcModes, const std::vector<double>& time,
                const std::vector<std::vector<double>>& y);
    ~ArcCurveFit() override; // Arc is incomplete in the header

    void   getPos(double t, std::vector<double>& v) override;
    void   getPos(double t, std::vector<float>& v) override;
    double getPos(double t, int j) override;
    void   getSlope(double t, std::vector<double>& v) override;
    double getSlope(double t, int j) override;
    std::vector<double> getTimePoints() override {
        return mTime;
    }

  private:
    class Arc; // one quarter-ellipse segment (defined in the .cc)
    std::vector<double> mTime;
    std::vector<std::unique_ptr<Arc>> mArcs;
    bool mExtrapolate = true;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_ARC_CURVE_FIT_H
