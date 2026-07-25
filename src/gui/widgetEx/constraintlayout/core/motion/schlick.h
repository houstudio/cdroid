/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.Schlick.
 *
 * Schlick's bias and gain easing functions ("Schlick(s,t)").
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_SCHLICK_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_SCHLICK_H

#include <string>

#include <widgetEx/constraintlayout/core/motion/easing.h>

namespace cdroid {

class Schlick : public Easing {
public:
    explicit Schlick(const std::string& configString);
    double get(double x) const override;
    double getDiff(double x) const override;
private:
    double func(double x) const;
    double dfunc(double x) const;
    double mS = 0, mT = 0;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_SCHLICK_H
