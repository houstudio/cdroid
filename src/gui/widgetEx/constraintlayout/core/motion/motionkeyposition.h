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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.key.MotionKeyPosition.
 *
 * A position keyframe: defines an intermediate control point the widget passes through during the
 * transition. All four modes are supported — CARTESIAN (percent in the start→end frame), PATH,
 * AXIS, and SCREEN (relative to the parent) — converted to absolute (x,y) in Motion::buildPath
 * and interpolated (spline/arc) through [start, control points, end].
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_KEY_POSITION_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_KEY_POSITION_H

#include <widgetEx/constraintlayout/core/motion/motionkey.h>

namespace cdroid {

class MotionKeyPosition : public MotionKey {
  public:
    static constexpr int KEY_TYPE = 2;
    static constexpr int TYPE_CARTESIAN = 0;
    static constexpr int TYPE_PATH = 1;
    static constexpr int TYPE_SCREEN = 2;
    static constexpr int TYPE_AXIS = 3;

    MotionKeyPosition() : mPositionType(TYPE_CARTESIAN) {
        mType = KEY_TYPE;
    }

    void getAttributeNames(std::unordered_set<std::string>& attributes) const override {}
    void addValues(std::unordered_map<std::string, SplineSet*>& splines) override {}
    MotionKey* clone() const override {
        return new MotionKeyPosition(*this);
    }

    bool setValue(int type, int value) override;
    bool setValue(int type, float value) override;
    bool setValue(int type, const std::string& value) override;
    bool setValue(int type, bool value) override {
        return false;
    }

    std::string mTransitionEasing;
    int mPathMotionArc = UNSET;
    int mDrawPath = 0;
    float mPercentWidth = NAN, mPercentHeight = NAN;
    float mPercentX = NAN, mPercentY = NAN;
    float mAltPercentX = NAN, mAltPercentY = NAN;
    int mPositionType;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_KEY_POSITION_H
