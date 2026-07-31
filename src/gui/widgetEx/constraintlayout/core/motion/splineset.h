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
 * Ported from androidx.constraintlayout.core.motion.utils.SplineSet.
 * Spline-based attribute interpolation: collects (framePosition, value) pairs, builds a CurveFit,
 * and get(t) returns the spline-interpolated value at progress t. Replaces the piecewise-linear
 * keyframed() in Motion when Motion.setup builds the attribute map.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_SPLINE_SET_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_SPLINE_SET_H
#include <memory>
#include <string>
#include <vector>
#include <widgetEx/constraintlayout/core/motion/curvefit.h>
namespace cdroid {
class TypedValues;
class SplineSet {
  public:
    virtual ~SplineSet() = default;
    void setPoint(int position, float value);
    virtual void setup(int curveType);
    float get(float t) const;
    float getSlope(float t) const;
    void setType(const std::string& type) {
        mType = type;
    }
    const std::string& getType() const {
        return mType;
    }
    CurveFit* getCurveFit() const {
        return mCurveFit.get();
    }
    // Apply the interpolated value to a TypedValues target (widget.setValue).
    virtual void setProperty(TypedValues* widget, float t);
  protected:
    std::vector<int> mTimePoints;
    std::vector<float> mValues;
    std::string mType;
    std::unique_ptr<CurveFit> mCurveFit;
};
} // namespace cdroid
#endif
