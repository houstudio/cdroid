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
#include <widgetEx/constraintlayout/core/motion/motionkeytimecycle.h>
#include <widgetEx/constraintlayout/core/motion/typedvalues.h>
namespace cdroid {
bool MotionKeyTimeCycle::setValue(int type, float value) {
    using C = TypedValues::CycleType;
    using A = TypedValues::AttributesType;
    switch (type) {
    case C::TYPE_WAVE_PERIOD:
        mWavePeriod = value;
        break;
    case C::TYPE_WAVE_OFFSET:
        mWaveOffset = value;
        break;
    case C::TYPE_WAVE_PHASE:
        break; // not stored in TimeCycle
    case A::TYPE_ALPHA:
        mAlpha = value;
        break;
    case A::TYPE_ROTATION_Z:
        mRotation = value;
        break;
    case A::TYPE_ROTATION_X:
        mRotationX = value;
        break;
    case A::TYPE_ROTATION_Y:
        mRotationY = value;
        break;
    case A::TYPE_SCALE_X:
        mScaleX = value;
        break;
    case A::TYPE_SCALE_Y:
        mScaleY = value;
        break;
    case A::TYPE_TRANSLATION_X:
        mTranslationX = value;
        break;
    case A::TYPE_TRANSLATION_Y:
        mTranslationY = value;
        break;
    case A::TYPE_PATH_ROTATE:
        mTransitionPathRotate = value;
        break;
    case A::TYPE_PROGRESS:
        mProgress = value;
        break;
    default:
        return false;
    }
    return true;
}
} // namespace cdroid
