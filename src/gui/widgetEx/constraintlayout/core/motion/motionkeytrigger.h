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
 * Ported from androidx.constraintlayout.core.motion.key.MotionKeyTrigger.
 * Fires callbacks when the transition progress crosses the keyframe's frame position. The trigger
 * type (CROSS / POSITIVE_CROSS / NEGATIVE_CROSS) determines the crossing direction.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_KEY_TRIGGER_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_KEY_TRIGGER_H
#include <string>
#include <widgetEx/constraintlayout/core/motion/motionkey.h>
#include <widgetEx/constraintlayout/core/motion/typedvalues.h>
namespace cdroid {
class MotionKeyTrigger : public MotionKey {
  public:
    static constexpr int KEY_TYPE = 5;
    MotionKeyTrigger() {
        mType = KEY_TYPE;
    }
    void getAttributeNames(std::unordered_set<std::string>&) const override {}
    void addValues(std::unordered_map<std::string, SplineSet*>&) override {}
    MotionKey* clone() const override {
        return new MotionKeyTrigger(*this);
    }
    bool setValue(int type, int value) override {
        if (type == TypedValues::TYPE_FRAME_POSITION) {
            mFramePosition = value;
            return true;
        }
        return false;
    }
    bool setValue(int, float) override {
        return false;
    }
    bool setValue(int type, const std::string& value) override;
    bool setValue(int, bool) override {
        return false;
    }
    std::string mCross, mPositiveCross, mNegativeCross;
    int mTriggerID = UNSET, mTriggerReceiver = UNSET, mTriggerCollisionId = UNSET;
    int mCurveFit = -1;
    float mTriggerSlack = 0.1f;
    bool mFireCrossReset = true, mFireNegativeReset = true, mFirePositiveReset = true;
};
} // namespace cdroid
#endif
