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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.key.MotionKeyCycle.
 *
 * A cycle keyframe: superimposes a wave oscillation on an attribute during the transition
 * (e.g. alpha bounces in a sine wave N times while the widget moves). Carries the same transform
 * fields as MotionKeyAttributes plus wave params (shape/period/offset/phase). Motion applies these
 * via the Oscillator system (ported in the motion math bedrock).
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_KEY_CYCLE_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_KEY_CYCLE_H

#include <widgetEx/constraintlayout/core/motion/motionkey.h>
#include <widgetEx/constraintlayout/core/motion/oscillator.h>

namespace cdroid {

class MotionKeyCycle : public MotionKey {
  public:
    static constexpr int KEY_TYPE = 4;

    MotionKeyCycle() {
        mType = KEY_TYPE;
    }

    void getAttributeNames(std::unordered_set<std::string>& attributes) const override;
    void addValues(std::unordered_map<std::string, SplineSet*>& splines) override;
    MotionKey* clone() const override {
        return new MotionKeyCycle(*this);
    }

    bool setValue(int type, int value) override;
    bool setValue(int type, float value) override;
    bool setValue(int type, const std::string& value) override;
    bool setValue(int type, bool value) override {
        return false;
    }

    // Wave parameters.
    int mWaveShape = -1;
    std::string mCustomWaveShape;
    float mWavePeriod = NAN;
    float mWaveOffset = 0;
    float mWavePhase = 0;
    // Transform values (the oscillation amplitude for each attribute).
    float mAlpha = NAN, mElevation = NAN;
    float mRotation = NAN, mRotationX = NAN, mRotationY = NAN;
    float mTransitionPathRotate = NAN;
    float mScaleX = NAN, mScaleY = NAN;
    float mTranslationX = NAN, mTranslationY = NAN, mTranslationZ = NAN;
    float mProgress = NAN;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_KEY_CYCLE_H
