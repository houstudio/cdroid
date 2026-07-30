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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.Oscillator.
 *
 * Variable-frequency oscillation curves (sine/square/triangle/saw/reverse-saw/cos/bounce/custom).
 * The phase accumulates a normalized area across keyed (position, period) samples so the frequency
 * can vary along the [0,1] progress axis. Used by KeyCycle / KeyTimeCycle animations.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_OSCILLATOR_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_OSCILLATOR_H

#include <memory>
#include <string>
#include <vector>

#include <widgetEx/constraintlayout/core/motion/monotoniccurvefit.h>

namespace cdroid {

class Oscillator {
  public:
    // Wave types (the int values line up with attribute enums).
    static const int SIN_WAVE         = 0;
    static const int SQUARE_WAVE      = 1;
    static const int TRIANGLE_WAVE    = 2;
    static const int SAW_WAVE         = 3;
    static const int REVERSE_SAW_WAVE = 4;
    static const int COS_WAVE         = 5;
    static const int BOUNCE           = 6;
    static const int CUSTOM           = 7;

    Oscillator();

    std::vector<float>  mPeriod;
    std::vector<double> mPosition;
    std::vector<double> mArea;

    void setType(int type, const std::string& customType);
    void addPoint(double position, float period);
    void normalize();

    double getValue(double time, double phase) const;
    double getSlope(double time, double phase, double dphase) const;

    std::string toString() const;

  private:
    double getP(double time) const;
    double getDP(double time) const;

    int mType = SIN_WAVE;
    std::string mCustomType;
    std::unique_ptr<MonotonicCurveFit> mCustomCurve;
    double mPI2;
    bool mNormalized = false;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_OSCILLATOR_H
