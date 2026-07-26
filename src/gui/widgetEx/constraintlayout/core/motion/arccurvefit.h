/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.ArcCurveFit.
 *
 * Stitches the x,y path together with quarter-ellipses (the Material "arc around a corner" motion).
 * Each segment between consecutive points is one Arc; linear segments are used where the points are
 * colinear or the mode is ARC_START_LINEAR. A lookup table arc-length-parameterizes the ellipse so
 * the widget travels it at constant velocity. Used by Motion when pathMotionArc is set.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_ARC_CURVE_FIT_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_ARC_CURVE_FIT_H

#include <memory>
#include <vector>

#include <widgetEx/constraintlayout/core/motion/curvefit.h>

namespace cdroid {

class ArcCurveFit : public CurveFit {
public:
    static const int ARC_START_LINEAR     = 0;
    static const int ARC_START_VERTICAL   = 1;
    static const int ARC_START_HORIZONTAL = 2;
    static const int ARC_START_FLIP       = 3;
    static const int ARC_BELOW            = 4;
    static const int ARC_ABOVE            = 5;

    ArcCurveFit(const std::vector<int>& arcModes, const std::vector<double>& time,
                const std::vector<std::vector<double>>& y);
    ~ArcCurveFit() override; // Arc is incomplete in the header

    void   getPos(double t, std::vector<double>& v) override;
    void   getPos(double t, std::vector<float>& v) override;
    double getPos(double t, int j) override;
    void   getSlope(double t, std::vector<double>& v) override;
    double getSlope(double t, int j) override;
    std::vector<double> getTimePoints() override { return mTime; }

private:
    class Arc; // one quarter-ellipse segment (defined in the .cc)
    std::vector<double> mTime;
    std::vector<std::unique_ptr<Arc>> mArcs;
    bool mExtrapolate = true;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_ARC_CURVE_FIT_H
