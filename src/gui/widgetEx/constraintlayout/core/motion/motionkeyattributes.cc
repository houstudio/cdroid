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
 */
#include <widgetEx/constraintlayout/core/motion/motionkeyattributes.h>

#include <widgetEx/constraintlayout/core/motion/typedvalues.h>

namespace cdroid {

void MotionKeyAttributes::getAttributeNames(std::unordered_set<std::string>& attributes) const {
    using A = TypedValues::AttributesType;
    if (!std::isnan(mAlpha))               attributes.insert(A::S_ALPHA);
    if (!std::isnan(mElevation))           attributes.insert(A::S_ELEVATION);
    if (!std::isnan(mRotation))            attributes.insert(A::S_ROTATION_Z);
    if (!std::isnan(mRotationX))           attributes.insert(A::S_ROTATION_X);
    if (!std::isnan(mRotationY))           attributes.insert(A::S_ROTATION_Y);
    if (!std::isnan(mPivotX))              attributes.insert(A::S_PIVOT_X);
    if (!std::isnan(mPivotY))              attributes.insert(A::S_PIVOT_Y);
    if (!std::isnan(mTranslationX))        attributes.insert(A::S_TRANSLATION_X);
    if (!std::isnan(mTranslationY))        attributes.insert(A::S_TRANSLATION_Y);
    if (!std::isnan(mTranslationZ))        attributes.insert(A::S_TRANSLATION_Z);
    if (!std::isnan(mTransitionPathRotate)) attributes.insert(A::S_PATH_ROTATE);
    if (!std::isnan(mScaleX))              attributes.insert(A::S_SCALE_X);
    if (!std::isnan(mScaleY))              attributes.insert(A::S_SCALE_Y);
    if (!std::isnan(mProgress))            attributes.insert(A::S_PROGRESS);
    for (const auto& kv : mCustom) attributes.insert(std::string(TypedValues::S_CUSTOM) + "," + kv.first);
}

void MotionKeyAttributes::addValues(std::unordered_map<std::string, SplineSet*>& /*splines*/) {
    // Unused: Motion applies attribute keyframes via its CurveFit (spline) interpolation path in
    // Motion::interpolate, not via per-key SplineSet population. SplineSet is ported for cycles.
}

MotionKey* MotionKeyAttributes::clone() const {
    auto* k = new MotionKeyAttributes(*this);
    return k;
}

bool MotionKeyAttributes::setValue(int type, int value) {
    if (type == TypedValues::AttributesType::TYPE_VISIBILITY) {
        mVisibility = value;
        return true;
    }
    if (type == TypedValues::AttributesType::TYPE_CURVE_FIT)  {
        mCurveFit = value;
        return true;
    }
    if (type == TypedValues::TYPE_FRAME_POSITION)             {
        mFramePosition = value;
        return true;
    }
    return false;
}

bool MotionKeyAttributes::setValue(int type, float value) {
    using A = TypedValues::AttributesType;
    switch (type) {
    case A::TYPE_ALPHA:
        mAlpha = value;
        break;
    case A::TYPE_ELEVATION:
        mElevation = value;
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
    case A::TYPE_PIVOT_X:
        mPivotX = value;
        break;
    case A::TYPE_PIVOT_Y:
        mPivotY = value;
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
    case A::TYPE_TRANSLATION_Z:
        mTranslationZ = value;
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

bool MotionKeyAttributes::setValue(int /*type*/, const std::string& /*value*/) {
    return false;
}
bool MotionKeyAttributes::setValue(int /*type*/, bool /*value*/) {
    return false;
}

} // namespace cdroid
