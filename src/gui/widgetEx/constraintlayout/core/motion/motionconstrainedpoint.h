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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.MotionConstrainedPoint.
 *
 * The full state of a widget at one keyframe position: geometry (position/x/y/w/h) + transforms
 * (alpha/rotation/translation/scale/pivot/pathRotate/progress/elevation) + custom variables. The
 * Motion engine interpolates between two points; different()/fillStandard() feed the curve-fit
 * solver array. applyParameters()/setState() read the state from a MotionWidget.
 *
 * Deferred: addValues(HashMap<String,SplineSet>) — the spline-set build path (SplineSet ported
 * later); setState(Rect, ...) — screen-rotation variant.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_CONSTRAINED_POINT_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_CONSTRAINED_POINT_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <widgetEx/constraintlayout/core/motion/customvariable.h>

namespace cdroid {

class MotionWidget;
class SplineSet; // forward — addValues is stubbed until the spline-set system is ported

class MotionConstrainedPoint {
  public:
    static constexpr int PERPENDICULAR = 1;
    static constexpr int CARTESIAN = 2;

    MotionConstrainedPoint();

    // Java fields are package-private; Motion/MotionPaths read them directly.
    float mAlpha = 1, mElevation = 0;
    float mRotation = 0, mRotationX = 0, rotationY = 0;
    float mScaleX = 1, mScaleY = 1;
    float mPivotX = NAN, mPivotY = NAN;
    float mTranslationX = 0, mTranslationY = 0, mTranslationZ = 0;
    float mPosition = 0, mX = 0, mY = 0, mWidth = 0, mHeight = 0;
    float mPathRotate = NAN, mProgress = NAN;
    int   mAnimateRelativeTo = -1;
    int   mMode = 0; // 1=perpendicular, 2=deltaRelative
    int   mVisibility = 0;
    int   mVisibilityMode = 0;
    bool  mApplyElevation = false;
    std::unordered_map<std::string, CustomVariable> mCustomVariable;

    double mTempValue[18] = {};
    double mTempDelta[18] = {};

    void setBounds(float x, float y, float w, float h);

    void different(const MotionConstrainedPoint& points, std::unordered_set<std::string>& keySet) const;
    void different(const MotionConstrainedPoint& points, std::vector<bool>& mask,
                   std::vector<std::string>& custom) const;
    void fillStandard(std::vector<double>& data, const std::vector<int>& toUse) const;

    bool hasCustomData(const std::string& name) const;
    int  getCustomDataCount(const std::string& name) const;
    int  getCustomData(const std::string& name, std::vector<double>& value, int offset) const;

    void applyParameters(MotionWidget* view);
    void setState(MotionWidget* view);
    void addValues(std::unordered_map<std::string, SplineSet*>& splines, int mFramePosition);

    bool operator<(const MotionConstrainedPoint& o) const {
        return mPosition < o.mPosition;
    }

  private:
    static bool diff(float a, float b);
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_CONSTRAINED_POINT_H
