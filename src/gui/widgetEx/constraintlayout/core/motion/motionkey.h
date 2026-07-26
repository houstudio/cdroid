/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.key.MotionKey.
 *
 * Abstract base for motion keyframes (KeyAttributes / KeyPosition / KeyCycle / ...). A keyframe
 * carries a frame position (0..100) + per-attribute values applied at that position; Motion
 * collects them, sorts by position, and feeds them to the CurveFit / SplineSet engine.
 * Subclasses implement getAttributeNames() / addValues() / clone().
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_KEY_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_KEY_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <widgetEx/constraintlayout/core/motion/customvariable.h>

namespace cdroid {

class SplineSet; // forward — addValues populates SplineSets (spline-set system ported later)

class MotionKey {
public:
    static constexpr int UNSET = -1;

    // Attribute name constants (match TypedValues.AttributesType.S_*).
    static constexpr const char* ALPHA = "alpha";
    static constexpr const char* ELEVATION = "elevation";
    static constexpr const char* ROTATION = "rotationZ";
    static constexpr const char* ROTATION_X = "rotationX";
    static constexpr const char* ROTATION_Y = "rotationY";
    static constexpr const char* TRANSITION_PATH_ROTATE = "transitionPathRotate";
    static constexpr const char* SCALE_X = "scaleX";
    static constexpr const char* SCALE_Y = "scaleY";
    static constexpr const char* PIVOT_X = "pivotX";
    static constexpr const char* PIVOT_Y = "pivotY";
    static constexpr const char* TRANSLATION_X = "translationX";
    static constexpr const char* TRANSLATION_Y = "translationY";
    static constexpr const char* CUSTOM = "CUSTOM";
    static constexpr const char* VISIBILITY = "visibility";

    MotionKey() = default;
    virtual ~MotionKey() = default;

    int mFramePosition = UNSET;
    int mType = 0;
    int mViewId = UNSET;
    std::unordered_map<std::string, CustomVariable> mCustom;

    virtual void getAttributeNames(std::unordered_set<std::string>& attributes) const = 0;
    virtual void addValues(std::unordered_map<std::string, SplineSet*>& splines) = 0;
    virtual MotionKey* clone() const = 0;

    virtual void setInterpolation(std::unordered_map<std::string, int>& interpolation) const {}
    virtual bool setValue(int type, int value);
    virtual bool setValue(int type, float value);
    virtual bool setValue(int type, const std::string& value);
    virtual bool setValue(int type, bool value);

    MotionKey& copy(const MotionKey& src);
    void setViewId(int id) { mViewId = id; }
    void setFramePosition(int pos) { mFramePosition = pos; }
    int  getFramePosition() const { return mFramePosition; }

    void setCustomAttribute(const std::string& name, int type, float value);
    void setCustomAttribute(const std::string& name, int type, int value);
    void setCustomAttribute(const std::string& name, int type, bool value);
    void setCustomAttribute(const std::string& name, int type, const std::string& value);
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_KEY_H
