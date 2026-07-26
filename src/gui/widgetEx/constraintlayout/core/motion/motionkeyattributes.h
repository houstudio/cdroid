/*
 * Copyright (C) 2018 The Android Open Source Project
 *
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

    MotionKeyAttributes() { mType = KEY_TYPE; }

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
