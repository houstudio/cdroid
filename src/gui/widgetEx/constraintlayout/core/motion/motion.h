/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.Motion.
 *
 * The per-widget motion controller: captures a start and end state (setStart/setEnd) and, given a
 * progress in [0,1], produces the interpolated position + transforms (interpolate). The MotionLayout
 * widget owns one Motion per child and drives them each frame.
 *
 * MVP: linear interpolation between start and end (rect + all transforms), with NaN-unset handling
 * matching WidgetFrame.interpolate. Deferred (fidelity): the CurveFit[]/arc path engine, keyframe
 * (MotionKey*) position/attribute/cycle keyframes, and the SplineSet/KeyCycleOscillator attribute
 * oscillators — those make multi-keyframe eased/arc/cyclic motion. The linear MVP already produces
 * correct straight-line transitions, which covers the common case.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_H

#include <vector>

#include <widgetEx/constraintlayout/core/motion/motionconstrainedpoint.h>
#include <widgetEx/constraintlayout/core/motion/motionpaths.h>
#include <widgetEx/constraintlayout/core/motion/motionwidget.h>
#include <widgetEx/constraintlayout/core/motion/typedvalues.h>

namespace cdroid {

class Motion : public TypedValues {
public:
    Motion();

    void setView(MotionWidget* view);
    void setStart(MotionWidget* mw);
    void setEnd(MotionWidget* mw);
    void setStartState(MotionWidget* mw);
    void setEndState(MotionWidget* mw);

    // Build the interpolation tables. The MVP uses linear lerp (no tables); kept for API parity.
    void setup(int parentWidth, int parentHeight, float transitionDuration);

    // Write the interpolated rect's 4 corners (8 floats) at progress p into path[offset..].
    void buildRect(float p, std::vector<float>& path, int offset);

    // Apply the interpolated position + transforms at `progress` to `child`.
    void interpolate(MotionWidget* child, float progress);

    // TypedValues motion-property dispatch (easing / arc / stagger stored on the start path).
    bool setValue(int id, int value) override;
    bool setValue(int id, float value) override;
    bool setValue(int id, const std::string& value) override;
    bool setValue(int id, bool value) override;
    int  getId(const std::string& name) override;

    MotionWidget* getView() const { return mView; }
    MotionPaths& getStartMotionPath() { return mStartMotionPath; }
    MotionPaths& getEndMotionPath() { return mEndMotionPath; }

private:
    static float lerp(float start, float end, float defaultValue, float progress);

    MotionWidget* mView = nullptr;
    MotionPaths mStartMotionPath;
    MotionPaths mEndMotionPath;
    MotionConstrainedPoint mStartPoint;
    MotionConstrainedPoint mEndPoint;

    // Motion properties (TypedValues). Used by the deferred CurveFit/keyframe engine; stored now
    // so setValue works before that lands.
    int   mPathMotionArc = -1;
    int   mDrawPath = 0;
    float mStagger = NAN;
    float mPathRotate = NAN;
    std::string mTransitionEasing;
    std::string mAnimateRelativeTo;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_H
