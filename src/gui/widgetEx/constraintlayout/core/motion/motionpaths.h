/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.MotionPaths.
 *
 * The geometric path keyframe: position (x/y/w/h) + path controls (drawPath/pathRotate/progress/
 * pathMotionArc/relativeAngle/mode/animateCircleAngleTo/animateRelativeTo) + easing + custom
 * attributes. Motion builds a sorted list of MotionPaths keyframes and, via a CurveFit, reads back
 * interpolated bounds at a progress (getRect/setView/getCenter).
 *
 * Chunk 5b ports the data holder + fillStandard/different/setBounds/custom-data. Deferred (chunk
 * 5c with Motion): the init*(MotionKeyPosition,...) keyframe-position builders and the curve-fit
 * readback (getCenter/getRect/setView/getCenterVelocity/setDpDt) — those need MotionKeyPosition
 * and the Motion controller and are the heart of the interpolation, ported carefully next.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_PATHS_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_PATHS_H

#include <string>
#include <unordered_map>
#include <vector>

#include <widgetEx/constraintlayout/core/motion/customvariable.h>

namespace cdroid {

class Easing;
class Motion;
class MotionKeyPosition;
class MotionWidget;

class MotionPaths {
  public:
    static constexpr int OFF_POSITION   = 0;
    static constexpr int OFF_X          = 1;
    static constexpr int OFF_Y          = 2;
    static constexpr int OFF_WIDTH      = 3;
    static constexpr int OFF_HEIGHT     = 4;
    static constexpr int OFF_PATH_ROTATE = 5;
    // KeyPosition types (aligned with MotionKeyPosition when ported).
    static constexpr int PERPENDICULAR = 1;
    static constexpr int CARTESIAN     = 2;
    static constexpr int AXIS          = 3;
    static constexpr int SCREEN        = 4;

    MotionPaths();

    std::string mId;
    Easing* mKeyFrameEasing = nullptr;
    int   mDrawPath = 0;
    float mTime = 0;
    float mPosition = 0;
    float mX = 0, mY = 0, mWidth = 0, mHeight = 0;
    float mPathRotate = NAN;
    float mProgress = NAN;
    int   mPathMotionArc = -1 /*UNSET*/;
    std::string mAnimateRelativeTo;
    float mRelativeAngle = NAN;
    Motion* mRelativeToController = nullptr;
    std::unordered_map<std::string, CustomVariable> mCustomAttributes;
    int   mMode = 0;
    int   mAnimateCircleAngleTo = 0;

    void setBounds(float x, float y, float w, float h);
    void fillStandard(std::vector<double>& data, const std::vector<int>& toUse) const;
    void different(const MotionPaths& points, std::vector<bool>& mask,
                   std::vector<std::string>& custom, bool arcMode) const;
    bool hasCustomData(const std::string& name) const;
    int  getCustomDataCount(const std::string& name) const;
    int  getCustomData(const std::string& name, std::vector<double>& value, int offset) const;

    // KeyPosition keyframe builders + curve-fit readback — ported with Motion (chunk 5c).
    void initCartesian(MotionKeyPosition* c, MotionPaths* start, MotionPaths* end);
    void initAxis(MotionKeyPosition* c, MotionPaths* start, MotionPaths* end);
    void initPath(MotionKeyPosition* c, MotionPaths* start, MotionPaths* end);
    void getRect(const std::vector<int>& toUse, std::vector<double>& data,
                 std::vector<float>& path, int offset);
    void setView(float position, Motion* motion, MotionWidget* childView);

  private:
    static bool diff(float a, float b);
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_PATHS_H
