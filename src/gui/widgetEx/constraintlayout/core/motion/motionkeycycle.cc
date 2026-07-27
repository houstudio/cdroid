/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.key.MotionKeyCycle.
 */
#include <widgetEx/constraintlayout/core/motion/motionkeycycle.h>

#include <widgetEx/constraintlayout/core/motion/typedvalues.h>

namespace cdroid {

void MotionKeyCycle::getAttributeNames(std::unordered_set<std::string>& attributes) const {
    using C = TypedValues::CycleType;
    if (!std::isnan(mAlpha))               attributes.insert(TypedValues::AttributesType::S_ALPHA);
    if (!std::isnan(mRotation))            attributes.insert(TypedValues::AttributesType::S_ROTATION_Z);
    if (!std::isnan(mRotationX))           attributes.insert(TypedValues::AttributesType::S_ROTATION_X);
    if (!std::isnan(mRotationY))           attributes.insert(TypedValues::AttributesType::S_ROTATION_Y);
    if (!std::isnan(mScaleX))              attributes.insert(TypedValues::AttributesType::S_SCALE_X);
    if (!std::isnan(mScaleY))              attributes.insert(TypedValues::AttributesType::S_SCALE_Y);
    if (!std::isnan(mTranslationX))        attributes.insert(TypedValues::AttributesType::S_TRANSLATION_X);
    if (!std::isnan(mTranslationY))        attributes.insert(TypedValues::AttributesType::S_TRANSLATION_Y);
    if (!std::isnan(mTransitionPathRotate)) attributes.insert(TypedValues::AttributesType::S_PATH_ROTATE);
    if (!std::isnan(mProgress))            attributes.insert(TypedValues::AttributesType::S_PROGRESS);
    (void)C::NAME;
}

void MotionKeyCycle::addValues(std::unordered_map<std::string, SplineSet*>& /*splines*/) {
    // TODO: populate KeyCycleOscillator (port with the oscillator system).
}

bool MotionKeyCycle::setValue(int type, int value) {
    if (type == TypedValues::CycleType::TYPE_WAVE_SHAPE) {
        mWaveShape = value;
        return true;
    }
    if (type == TypedValues::TYPE_FRAME_POSITION)        {
        mFramePosition = value;
        return true;
    }
    return false;
}

bool MotionKeyCycle::setValue(int type, float value) {
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
        mWavePhase = value;
        break;
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

bool MotionKeyCycle::setValue(int /*type*/, const std::string& /*value*/) {
    return false;
}

} // namespace cdroid
