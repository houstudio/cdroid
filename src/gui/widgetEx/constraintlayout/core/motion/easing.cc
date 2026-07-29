/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/

/*
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.Easing.
 */
#include <widgetEx/constraintlayout/core/motion/easing.h>

#include <porting/cdlog.h>
#include <widgetEx/constraintlayout/core/motion/schlick.h>
#include <widgetEx/constraintlayout/core/motion/stepcurve.h>

namespace cdroid {

// Named-easing cubic control strings (Java: private static final String ...).
const std::string Easing::STANDARD   = "cubic(0.4, 0.0, 0.2, 1)";
const std::string Easing::ACCELERATE = "cubic(0.4, 0.05, 0.8, 0.7)";
const std::string Easing::DECELERATE = "cubic(0.0, 0.0, 0.2, 0.95)";
const std::string Easing::LINEAR     = "cubic(1, 1, 0, 0)";
const std::string Easing::ANTICIPATE = "cubic(0.36, 0, 0.66, -0.56)";
const std::string Easing::OVERSHOOT  = "cubic(0.34, 1.56, 0.64, 1)";

double Easing::get(double x) const {
    return x;
}

double Easing::getDiff(double x) const {
    (void) x;
    return 1;
}

std::string Easing::toString() const {
    return mStr;
}

std::unique_ptr<Easing> Easing::getInterpolator(const std::string& configString) {
    if (configString.empty()) {
        return nullptr;
    }
    if (configString.compare(0, 5, "cubic") == 0) {
        return std::make_unique<CubicEasing>(configString);
    }
    if (configString.compare(0, 6, "spline") == 0) {
        return std::make_unique<StepCurve>(configString);
    }
    if (configString.compare(0, 7, "Schlick") == 0) {
        return std::make_unique<Schlick>(configString);
    }
    if (configString == "standard")   return std::make_unique<CubicEasing>(STANDARD);
    if (configString == "accelerate") return std::make_unique<CubicEasing>(ACCELERATE);
    if (configString == "decelerate") return std::make_unique<CubicEasing>(DECELERATE);
    if (configString == "linear")     return std::make_unique<CubicEasing>(LINEAR);
    if (configString == "anticipate") return std::make_unique<CubicEasing>(ANTICIPATE);
    if (configString == "overshoot")  return std::make_unique<CubicEasing>(OVERSHOOT);

    LOGW("transitionEasing syntax error: \"%s\" (use cubic(1.0,0.5,0.0,0.6) or "
         "[standard,accelerate,decelerate,linear])", configString.c_str());
    return std::make_unique<Easing>();
}

// ---------------------------------------------------------------------------
// Easing::CubicEasing
// ---------------------------------------------------------------------------
double Easing::CubicEasing::sError  = 0.01;
double Easing::CubicEasing::sDError = 0.0001;

Easing::CubicEasing::CubicEasing(const std::string& configString) {
    mStr = configString;
    size_t start = configString.find('(');
    size_t off1  = configString.find(',', start);
    mX1 = std::stod(configString.substr(start + 1, off1 - start - 1));
    size_t off2  = configString.find(',', off1 + 1);
    mY1 = std::stod(configString.substr(off1 + 1, off2 - off1 - 1));
    size_t off3  = configString.find(',', off2 + 1);
    mX2 = std::stod(configString.substr(off2 + 1, off3 - off2 - 1));
    size_t end   = configString.find(')', off3 + 1);
    mY2 = std::stod(configString.substr(off3 + 1, end - off3 - 1));
}

Easing::CubicEasing::CubicEasing(double x1, double y1, double x2, double y2) {
    setup(x1, y1, x2, y2);
}

void Easing::CubicEasing::setup(double x1, double y1, double x2, double y2) {
    mX1 = x1;
    mY1 = y1;
    mX2 = x2;
    mY2 = y2;
}

double Easing::CubicEasing::getX(double t) const {
    double t1 = 1 - t;
    double f1 = 3 * t1 * t1 * t;
    double f2 = 3 * t1 * t * t;
    double f3 = t * t * t;
    return mX1 * f1 + mX2 * f2 + f3;
}

double Easing::CubicEasing::getY(double t) const {
    double t1 = 1 - t;
    double f1 = 3 * t1 * t1 * t;
    double f2 = 3 * t1 * t * t;
    double f3 = t * t * t;
    return mY1 * f1 + mY2 * f2 + f3;
}

double Easing::CubicEasing::getDiff(double x) const {
    double t = 0.5;
    double range = 0.5;
    while (range > sDError) {
        double tx = getX(t);
        range *= 0.5;
        if (tx < x) {
            t += range;
        } else {
            t -= range;
        }
    }
    double x1 = getX(t - range);
    double x2 = getX(t + range);
    double y1 = getY(t - range);
    double y2 = getY(t + range);
    return (y2 - y1) / (x2 - x1);
}

double Easing::CubicEasing::get(double x) const {
    if (x <= 0.0) {
        return 0;
    }
    if (x >= 1.0) {
        return 1.0;
    }
    double t = 0.5;
    double range = 0.5;
    while (range > sError) {
        double tx = getX(t);
        range *= 0.5;
        if (tx < x) {
            t += range;
        } else {
            t -= range;
        }
    }
    double x1 = getX(t - range);
    double x2 = getX(t + range);
    double y1 = getY(t - range);
    double y2 = getY(t + range);
    return (y2 - y1) * (x - x1) / (x2 - x1) + y1;
}

} // namespace cdroid
