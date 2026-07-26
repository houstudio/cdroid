/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.key.MotionKey.
 */
#include <widgetEx/constraintlayout/core/motion/motionkey.h>

namespace cdroid {

// Base defaults: keyframes accept no typed values here (subclasses override).
bool MotionKey::setValue(int /*type*/, int /*value*/)             { return false; }
bool MotionKey::setValue(int /*type*/, float /*value*/)           { return false; }
bool MotionKey::setValue(int /*type*/, const std::string& /*value*/) { return false; }
bool MotionKey::setValue(int /*type*/, bool /*value*/)            { return false; }

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
