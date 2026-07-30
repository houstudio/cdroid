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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.key.MotionKey.
 */
#include <widgetEx/constraintlayout/core/motion/motionkey.h>

namespace cdroid {

// Base defaults: keyframes accept no typed values here (subclasses override).
bool MotionKey::setValue(int /*type*/, int /*value*/)             {
    return false;
}
bool MotionKey::setValue(int /*type*/, float /*value*/)           {
    return false;
}
bool MotionKey::setValue(int /*type*/, const std::string& /*value*/) {
    return false;
}
bool MotionKey::setValue(int /*type*/, bool /*value*/)            {
    return false;
}

MotionKey& MotionKey::copy(const MotionKey& src) {
    mFramePosition = src.mFramePosition;
    mType = src.mType;
    mViewId = src.mViewId;
    mCustom = src.mCustom;
    return *this;
}

void MotionKey::setCustomAttribute(const std::string& name, int type, float value) {
    mCustom[name] = CustomVariable(name, type, value);
}
void MotionKey::setCustomAttribute(const std::string& name, int type, int value) {
    mCustom[name] = CustomVariable(name, type, value);
}
void MotionKey::setCustomAttribute(const std::string& name, int type, bool value) {
    mCustom[name] = CustomVariable(name, type, value);
}
void MotionKey::setCustomAttribute(const std::string& name, int type, const std::string& value) {
    mCustom[name] = CustomVariable(name, type, value);
}

} // namespace cdroid
