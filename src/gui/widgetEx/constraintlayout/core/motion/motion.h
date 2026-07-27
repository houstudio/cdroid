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

#include <memory>
#include <vector>

#include <widgetEx/constraintlayout/core/motion/curvefit.h>
#include <widgetEx/constraintlayout/core/motion/easing.h>
#include <widgetEx/constraintlayout/core/motion/motionconstrainedpoint.h>
#include <widgetEx/constraintlayout/core/motion/motionpaths.h>
#include <widgetEx/constraintlayout/core/motion/motionwidget.h>
#include <widgetEx/constraintlayout/core/motion/typedvalues.h>

namespace cdroid {

class Motion : public TypedValues {
  public:
    Motion();
    ~Motion()override;

    void setView(MotionWidget* view);
    void setStart(MotionWidget* mw);
    void setEnd(MotionWidget* mw);
    void setStartState(MotionWidget* mw);
    void setEndState(MotionWidget* mw);

    // Add an attribute keyframe (e.g. alpha=0 at frame 50). Stored sorted by frame position.
    void addKey(class MotionKeyAttributes* key);
    // Add a position keyframe (control point the widget passes through).
    void addKey(class MotionKeyPosition* key);
    void addKey(class MotionKeyCycle* key);

    // Build the interpolation tables. The MVP uses linear lerp (no tables); kept for API parity.
    void setup(int parentWidth, int parentHeight, float transitionDuration);

    // Write the interpolated rect's 4 corners (8 floats) at progress p into path[offset..].
    void buildRect(float p, std::vector<float>& path, int offset);

    // Apply the interpolated position + transforms at `progress` to `child`.
    void interpolate(MotionWidget* child, float progress);

    // Rate of change (pixels per unit progress) of the anchor point at (locationX,locationY) on the
    // widget, at `pos`. Out = (dAnchorX/dProgress, dAnchorY/dProgress). Used by TouchResponse to map
    // a drag delta to a progress delta (the anchor's travel distance is the real drag range).
    void getDpDt(float pos, float locationX, float locationY, float out[2]);

    // TypedValues motion-property dispatch (easing / arc / stagger stored on the start path).
    bool setValue(int id, int value) override;
    bool setValue(int id, float value) override;
    bool setValue(int id, const std::string& value) override;
    bool setValue(int id, bool value) override;
    int  getId(const std::string& name) override;

    MotionWidget* getView() const {
        return mView;
    }
    MotionPaths& getStartMotionPath() {
        return mStartMotionPath;
    }
    MotionPaths& getEndMotionPath() {
        return mEndMotionPath;
    }

  private:
    static float lerp(float start, float end, float defaultValue, float progress);
    // Parse mTransitionEasing into mEasing (called before interpolation if pending).
    void buildEasing();
    // Map a raw [0,1] progress through the easing curve (identity if no easing set).
    float eased(float progress) const;
    // Lazily build the position CurveFit (spline through start + KeyPosition control points + end)
    // from the current start/end/keyframes. Called by interpolate/getDpDt when mPathDirty.
    void buildPath();
    // Piecewise-linear interpolation of an attribute across [start, keyframes..., end]. NAN
    // start/end are replaced by `defaultValue` (matching lerp's semantics); `get` extracts the
    // keyframe's value (NAN = that keyframe doesn't set this attribute). With no keyframes this
    // degenerates to the plain lerp.
    float keyframed(float progress, float startVal, float endVal, float defaultValue,
                    float (*get)(const class MotionKeyAttributes*)) const;

    MotionWidget* mView = nullptr;
    std::unique_ptr<Easing> mEasing;
    bool mEasingDirty = false;
    std::unique_ptr<CurveFit> mPositionCurveFit; // spline through the position keyframes (start..end)
    std::unique_ptr<CurveFit> mArcCurveFit;      // quarter-ellipse arc (x,y) when mPathMotionArc set
    bool mPathDirty = true;                       // start/end/keyframes changed -> rebuild
    int mParentWidth = 0;   // parent dimensions, needed by TYPE_SCREEN KeyPositions (set via setup)
    int mParentHeight = 0;
    std::vector<class MotionKeyAttributes*> mAttributeKeys;
    std::vector<class MotionKeyPosition*> mPositionKeys;
    std::vector<class MotionKeyCycle*> mCycleKeys;
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
