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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.HyperSpline.
 *
 * Natural-cubic spline interpolation in N dimensions, parameterized by arc length.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_HYPER_SPLINE_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_HYPER_SPLINE_H

#include <vector>

namespace cdroid {

class HyperSpline {
  public:
    // A natural cubic polynomial a + b*u + c*u^2 + d*u^3.
    class Cubic {
      public:
        Cubic(double a, double b, double c, double d) : mA(a), mB(b), mC(c), mD(d) {}
        double eval(double u) const {
            return (((mD * u) + mC) * u + mB) * u + mA;
        }
        double vel(double v) const {
            return (mD * 3 * v + mC * 2) * v + mB;
        }
      private:
        double mA, mB, mC, mD;
    };

    // points: [mPoints][dimensionality]
    explicit HyperSpline(const std::vector<std::vector<double>>& points);
    HyperSpline();

    void setup(const std::vector<std::vector<double>>& points);

    void   getVelocity(double p, std::vector<double>& v);
    void   getPos(double p, std::vector<double>& x);
    void   getPos(double p, std::vector<float>& x);
    double getPos(double p, int splineNumber);
    double approxLength(const std::vector<Cubic>& curve);

    static std::vector<Cubic> calcNaturalCubic(int n, const std::vector<double>& x);

  private:
    int mPoints = 0;
    int mDimensionality = 0;
    std::vector<std::vector<Cubic>> mCurve; // [dim][segment]
    std::vector<double> mCurveLength;
    double mTotalLength = 0;
    std::vector<std::vector<double>> mCtl;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_HYPER_SPLINE_H
