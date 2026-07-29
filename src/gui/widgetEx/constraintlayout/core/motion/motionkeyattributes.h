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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.key.MotionKeyAttributes.
 *
 * An attribute keyframe: sets transform/alpha values at a frame position (0..100). Motion collects
 * these per attribute and interpolates piecewise across [start, keyframe..., end]. getAttributeNames
 * reports which attributes this keyframe touches (those not NaN).
 *
 * addValues(HashMap<String,SplineSet>) — the SplineSet build path — is stubbed (the spline-set
 * system is ported later); Motion's MVP applies keyframes via piecewise-linear interpolation.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_KEY_ATTRIBUTES_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_KEY_ATTRIBUTES_H

#include <widgetEx/constraintlayout/core/motion/motionkey.h>

namespace cdroid {

class MotionKeyAttributes : public MotionKey {
  public:
    static constexpr int KEY_TYPE = 1;

    MotionKeyAttributes() {
        mType = KEY_TYPE;
    }

    void getAttributeNames(std::unordered_set<std::string>& attributes) const override;
    void addValues(std::unordered_map<std::string, SplineSet*>& splines) override;
    MotionKey* clone() const override;

    bool setValue(int type, int value) override;
    bool setValue(int type, float value) override;
    bool setValue(int type, const std::string& value) override;
    bool setValue(int type, bool value) override;

    // Public for Motion's piecewise interpolation (Java package-private).
    float mAlpha = NAN, mElevation = NAN;
    float mRotation = NAN, mRotationX = NAN, mRotationY = NAN;
    float mPivotX = NAN, mPivotY = NAN;
    float mTransitionPathRotate = NAN;
    float mScaleX = NAN, mScaleY = NAN;
    float mTranslationX = NAN, mTranslationY = NAN, mTranslationZ = NAN;
    float mProgress = NAN;
    int   mVisibility = MotionKey::UNSET;
    int   mCurveFit = MotionKey::UNSET;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_KEY_ATTRIBUTES_H
