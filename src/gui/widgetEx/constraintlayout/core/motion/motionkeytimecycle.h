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
 * Ported from androidx.constraintlayout.core.motion.key.MotionKeyTimeCycle.
 * Like MotionKeyCycle but the oscillation evolves over time (for looping animations). CDROID
 * overlays it as a Cycle-style sin wave keyed off transition progress (a fidelity simplification —
 * Android keys the wave off absolute time).
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_KEY_TIME_CYCLE_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_KEY_TIME_CYCLE_H
#include <string>
#include <widgetEx/constraintlayout/core/motion/motionkey.h>
#include <widgetEx/constraintlayout/core/motion/typedvalues.h>
namespace cdroid {
class MotionKeyTimeCycle : public MotionKey {
  public:
    static constexpr int KEY_TYPE = 3;
    MotionKeyTimeCycle() {
        mType = KEY_TYPE;
    }
    void getAttributeNames(std::unordered_set<std::string>& a) const override {}
    void addValues(std::unordered_map<std::string, SplineSet*>&) override {}
    MotionKey* clone() const override {
        return new MotionKeyTimeCycle(*this);
    }
    bool setValue(int type, int value) override {
        if (type == TypedValues::CycleType::TYPE_WAVE_SHAPE) {
            mWaveShape = value;
            return true;
        }
        if (type == TypedValues::TYPE_FRAME_POSITION) {
            mFramePosition = value;
            return true;
        }
        return false;
    }
    bool setValue(int type, float value) override;
    bool setValue(int, const std::string&) override {
        return false;
    }
    bool setValue(int, bool) override {
        return false;
    }
    std::string mTransitionEasing;
    int mCurveFit = -1, mWaveShape = 0;
    float mWavePeriod = NAN, mWaveOffset = 0;
    float mAlpha = NAN, mElevation = NAN, mRotation = NAN, mRotationX = NAN, mRotationY = NAN;
    float mTransitionPathRotate = NAN, mScaleX = NAN, mScaleY = NAN;
    float mTranslationX = NAN, mTranslationY = NAN, mTranslationZ = NAN, mProgress = NAN;
};
} // namespace cdroid
#endif
