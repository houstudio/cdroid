/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.Easing.
 *
 * Engine for cubic-spline / named easing curves. getInterpolator() parses a config string
 * ("cubic(0.4,0,0.2,1)", "standard", "spline(...)", "Schlick(...)") into an Easing subclass; the
 * base class is the identity. MotionLayout maps progress -> eased progress through get().
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_EASING_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_EASING_H

#include <memory>
#include <string>

namespace cdroid {

class Schlick;
class StepCurve;

class Easing {
  public:
    virtual ~Easing() = default;

    // Parse a config string into an easing. Empty string -> nullptr (use caller default).
    static std::unique_ptr<Easing> getInterpolator(const std::string& configString);

    // Identity by default: y = x.
    virtual double get(double x) const;
    virtual double getDiff(double x) const;
    virtual std::string toString() const;

    std::string mStr = "identity";

    // Cubic-bezier easing — defined out-of-line below (a nested class cannot derive from its
    // still-incomplete enclosing class inside the class body).
    class CubicEasing;

  private:
    // Named-easing cubic definitions (matched in getInterpolator).
    static const std::string STANDARD;
    static const std::string ACCELERATE;
    static const std::string DECELERATE;
    static const std::string LINEAR;
    static const std::string ANTICIPATE;
    static const std::string OVERSHOOT;
};

// Cubic-bezier easing solved by binary search on the parametric x(t).
class Easing::CubicEasing : public Easing {
  public:
    explicit CubicEasing(const std::string& configString);
    CubicEasing(double x1, double y1, double x2, double y2);
    double get(double x) const override;
    double getDiff(double x) const override;
  private:
    void setup(double x1, double y1, double x2, double y2);
    double getX(double t) const;
    double getY(double t) const;
    double mX1 = 0, mY1 = 0, mX2 = 0, mY2 = 0;
    static double sError;
    static double sDError;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_EASING_H
