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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.CurveFit.
 */
#include <widgetEx/constraintlayout/core/motion/curvefit.h>

#include <widgetEx/constraintlayout/core/motion/arccurvefit.h>
#include <porting/cdlog.h>
#include <widgetEx/constraintlayout/core/motion/linearcurvefit.h>
#include <widgetEx/constraintlayout/core/motion/monotoniccurvefit.h>

namespace cdroid {

std::unique_ptr<CurveFit> CurveFit::get(int type, const std::vector<double>& time,
                                        const std::vector<std::vector<double>>& y) {
    int resolvedType = type;
    if (time.size() == 1) {
        resolvedType = CONSTANT;
    }
    switch (resolvedType) {
    case SPLINE:
        return std::make_unique<MonotonicCurveFit>(time, y);
    case CONSTANT:
        return std::make_unique<Constant>(time[0], y[0]);
    default:
        return std::make_unique<LinearCurveFit>(time, y);
    }
}

std::unique_ptr<CurveFit> CurveFit::getArc(const std::vector<int>& arcModes,
        const std::vector<double>& time,
        const std::vector<std::vector<double>>& y) {
    return std::make_unique<ArcCurveFit>(arcModes, time, y);
}

// ---------------------------------------------------------------------------
// CurveFit::Constant
// ---------------------------------------------------------------------------
CurveFit::Constant::Constant(double time, const std::vector<double>& value)
    : mTime(time), mValue(value) {}

void CurveFit::Constant::getPos(double /*t*/, std::vector<double>& v) {
    for (size_t i = 0; i < mValue.size(); i++) {
        v[i] = mValue[i];
    }
}

void CurveFit::Constant::getPos(double /*t*/, std::vector<float>& v) {
    for (size_t i = 0; i < mValue.size(); i++) {
        v[i] = (float) mValue[i];
    }
}

double CurveFit::Constant::getPos(double /*t*/, int j) {
    return mValue[j];
}

void CurveFit::Constant::getSlope(double /*t*/, std::vector<double>& v) {
    for (size_t i = 0; i < mValue.size(); i++) {
        v[i] = 0;
    }
}

double CurveFit::Constant::getSlope(double /*t*/, int /*j*/) {
    return 0;
}

std::vector<double> CurveFit::Constant::getTimePoints() {
    return {mTime};
}

} // namespace cdroid
