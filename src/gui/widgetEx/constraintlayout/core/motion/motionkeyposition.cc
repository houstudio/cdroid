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
 */
#include <widgetEx/constraintlayout/core/motion/motionkeyposition.h>

#include <widgetEx/constraintlayout/core/motion/typedvalues.h>

namespace cdroid {

bool MotionKeyPosition::setValue(int type, int value) {
    if (type == TypedValues::PositionType::TYPE_POSITION_TYPE) {
        mPositionType = value;
        return true;
    }
    if (type == TypedValues::PositionType::TYPE_PATH_MOTION_ARC) {
        mPathMotionArc = value;
        return true;
    }
    if (type == TypedValues::TYPE_FRAME_POSITION) {
        mFramePosition = value;
        return true;
    }
    return false;
}

bool MotionKeyPosition::setValue(int type, float value) {
    using P = TypedValues::PositionType;
    switch (type) {
    case P::TYPE_PERCENT_X:
        mPercentX = value;
        break;
    case P::TYPE_PERCENT_Y:
        mPercentY = value;
        break;
    case P::TYPE_PERCENT_WIDTH:
        mPercentWidth = value;
        break;
    case P::TYPE_PERCENT_HEIGHT:
        mPercentHeight = value;
        break;
    case P::TYPE_SIZE_PERCENT:
        mPercentWidth = mPercentHeight = value;
        break;
    default:
        return false;
    }
    return true;
}

bool MotionKeyPosition::setValue(int type, const std::string& value) {
    if (type == TypedValues::PositionType::TYPE_TRANSITION_EASING) {
        mTransitionEasing = value;
        return true;
    }
    return false;
}

} // namespace cdroid
