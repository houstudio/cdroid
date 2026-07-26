/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.key.MotionKeyPosition.
 *
 * A position keyframe: defines an intermediate control point the widget passes through during the
 * transition. percentX/percentY are in the start→end path's coordinate frame (cartesian mode);
 * Motion converts them to absolute (x,y) and interpolates piecewise-linearly through
 * [start, control points, end]. Other modes (path/screen/axis) deferred.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_KEY_POSITION_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_KEY_POSITION_H

#include <widgetEx/constraintlayout/core/motion/motionkey.h>

namespace cdroid {

class MotionKeyPosition : public MotionKey {
public:
    static constexpr int KEY_TYPE = 2;
    static constexpr int TYPE_CARTESIAN = 0;
    static constexpr int TYPE_PATH = 1;
    static constexpr int TYPE_SCREEN = 2;
    static constexpr int TYPE_AXIS = 3;

    MotionKeyPosition() : mPositionType(TYPE_CARTESIAN) { mType = KEY_TYPE; }

    void getAttributeNames(std::unordered_set<std::string>& attributes) const override {}
    void addValues(std::unordered_map<std::string, SplineSet*>& splines) override {}
    MotionKey* clone() const override { return new MotionKeyPosition(*this); }

    bool setValue(int type, int value) override;
    bool setValue(int type, float value) override;
    bool setValue(int type, const std::string& value) override;
    bool setValue(int type, bool value) override { return false; }

    std::string mTransitionEasing;
    int mPathMotionArc = UNSET;
    int mDrawPath = 0;
    float mPercentWidth = NAN, mPercentHeight = NAN;
    float mPercentX = NAN, mPercentY = NAN;
    float mAltPercentX = NAN, mAltPercentY = NAN;
    int mPositionType;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_KEY_POSITION_H
