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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.ArcCurveFit.
 */
#include <widgetEx/constraintlayout/core/motion/arccurvefit.h>

#include <algorithm>
#include <cmath>

namespace cdroid {

namespace {
constexpr double kEpsilon = 0.001;
// Internal arc-segment modes (from the public ARC_* modes).
constexpr int kStartVertical   = 1;
constexpr int kStartHorizontal = 2;
constexpr int kStartLinear     = 3;
constexpr int kDownArc         = 4;
constexpr int kUpArc           = 5;

// Mimic Java Arrays.binarySearch on a sorted ascending vector: found -> index; else -(insertion)-1.
int binarySearch(const std::vector<double>& a, double key) {
    auto it = std::lower_bound(a.begin(), a.end(), key);
    const int idx = static_cast<int>(it - a.begin());
    if (it != a.end() && *it == key) return idx;
    return -idx - 1;
}
} // namespace

// ===========================================================================
// ArcCurveFit::Arc — one quarter-ellipse (or linear) segment
// ===========================================================================
class ArcCurveFit::Arc {
  public:
    Arc(int mode, double t1, double t2, double x1, double y1, double x2, double y2);

    void   setPoint(double time);
    double getX() const {
        return mEllipseCenterX + mEllipseA * mTmpSinAngle;
    }
    double getY() const {
        return mEllipseCenterY + mEllipseB * mTmpCosAngle;
    }
    double getDX() const;
    double getDY() const;
    double getLinearX(double t) const;
    double getLinearY(double t) const;
    double getLinearDX() const {
        return mEllipseCenterX;    // slope cached here in linear mode
    }
    double getLinearDY() const {
        return mEllipseCenterY;
    }

    double mTime1 = 0, mTime2 = 0;
    bool   mLinear = false;

  private:
    double lookup(double v) const;
    void   buildTable(double x1, double y1, double x2, double y2);

    std::vector<double> mLut;
    double mArcDistance = 0;
    double mX1 = 0, mX2 = 0, mY1 = 0, mY2 = 0;
    double mOneOverDeltaTime = 0;
    double mEllipseA = 0, mEllipseB = 0;
    double mEllipseCenterX = 0, mEllipseCenterY = 0;
    double mArcVelocity = 0;
    double mTmpSinAngle = 0, mTmpCosAngle = 0;
    bool   mVertical = false;
};

ArcCurveFit::Arc::Arc(int mode, double t1, double t2, double x1, double y1, double x2, double y2) {
    const double dx = x2 - x1;
    const double dy = y2 - y1;
    switch (mode) {
    case kStartVertical:
        mVertical = true;
        break;
    case kUpArc:
        mVertical = dy < 0;
        break;
    case kDownArc:
        mVertical = dy > 0;
        break;
    default:
        mVertical = false;
        break;
    }
    mTime1 = t1;
    mTime2 = t2;
    mOneOverDeltaTime = 1.0 / (mTime2 - mTime1);
    if (mode == kStartLinear) mLinear = true;

    if (mLinear || std::abs(dx) < kEpsilon || std::abs(dy) < kEpsilon) {
        mLinear = true;
        mX1 = x1;
        mX2 = x2;
        mY1 = y1;
        mY2 = y2;
        mArcDistance = std::hypot(dy, dx);
        mArcVelocity = mArcDistance * mOneOverDeltaTime;
        mEllipseCenterX = dx / (mTime2 - mTime1); // cache the slope in the unused center
        mEllipseCenterY = dy / (mTime2 - mTime1);
        return;
    }
    mLut.resize(101);
    mEllipseA = dx * (mVertical ? -1 : 1);
    mEllipseB = dy * (mVertical ? 1 : -1);
    mEllipseCenterX = mVertical ? x2 : x1;
    mEllipseCenterY = mVertical ? y1 : y2;
    buildTable(x1, y1, x2, y2);
    mArcVelocity = mArcDistance * mOneOverDeltaTime;
}

void ArcCurveFit::Arc::setPoint(double time) {
    const double percent = (mVertical ? (mTime2 - time) : (time - mTime1)) * mOneOverDeltaTime;
    const double angle = M_PI * 0.5 * lookup(percent);
    mTmpSinAngle = std::sin(angle);
    mTmpCosAngle = std::cos(angle);
}

double ArcCurveFit::Arc::getDX() const {
    const double vx = mEllipseA * mTmpCosAngle;
    const double vy = -mEllipseB * mTmpSinAngle;
    const double norm = mArcVelocity / std::hypot(vx, vy);
    return mVertical ? -vx * norm : vx * norm;
}

double ArcCurveFit::Arc::getDY() const {
    const double vx = mEllipseA * mTmpCosAngle;
    const double vy = -mEllipseB * mTmpSinAngle;
    const double norm = mArcVelocity / std::hypot(vx, vy);
    return mVertical ? -vy * norm : vy * norm;
}

double ArcCurveFit::Arc::getLinearX(double t) const {
    t = (t - mTime1) * mOneOverDeltaTime;
    return mX1 + t * (mX2 - mX1);
}

double ArcCurveFit::Arc::getLinearY(double t) const {
    t = (t - mTime1) * mOneOverDeltaTime;
    return mY1 + t * (mY2 - mY1);
}

double ArcCurveFit::Arc::lookup(double v) const {
    if (v <= 0) return 0;
    if (v >= 1) return 1;
    const double pos = v * (mLut.size() - 1);
    const int iv = static_cast<int>(pos);
    const double off = pos - iv;
    return mLut[iv] + off * (mLut[iv + 1] - mLut[iv]);
}

void ArcCurveFit::Arc::buildTable(double x1, double y1, double x2, double y2) {
    const double a = x2 - x1;
    const double b = y1 - y2;
    std::vector<double> ourPercent(91, 0.0);
    double lx = 0, ly = 0, dist = 0;
    for (int i = 0; i < 91; i++) {
        const double angle = (90.0 * i / 90.0) * M_PI / 180.0;
        const double s = std::sin(angle);
        const double c = std::cos(angle);
        const double px = a * s;
        const double py = b * c;
        if (i > 0) {
            dist += std::hypot(px - lx, py - ly);
            ourPercent[i] = dist;
        }
        lx = px;
        ly = py;
    }
    mArcDistance = dist;
    for (int i = 0; i < 91; i++) ourPercent[i] /= dist;
    for (int i = 0; i < 101; i++) {
        const double pos = i / 100.0;
        const int index = binarySearch(ourPercent, pos);
        if (index >= 0) {
            mLut[i] = index / 90.0;
        } else if (index == -1) {
            mLut[i] = 0;
        } else {
            const int p1 = -index - 2;
            const int p2 = -index - 1;
            mLut[i] = (p1 + (pos - ourPercent[p1]) / (ourPercent[p2] - ourPercent[p1])) / 90.0;
        }
    }
}

// ===========================================================================
// ArcCurveFit
// ===========================================================================
ArcCurveFit::ArcCurveFit(const std::vector<int>& arcModes, const std::vector<double>& time,
                         const std::vector<std::vector<double>>& y)
    : mTime(time) {
    mArcs.resize(time.size() - 1);
    int mode = kStartVertical;
    int last = kStartVertical;
    for (size_t i = 0; i < mArcs.size(); i++) {
        switch (arcModes[i]) {
        case ARC_START_VERTICAL:
            last = mode = kStartVertical;
            break;
        case ARC_START_HORIZONTAL:
            last = mode = kStartHorizontal;
            break;
        case ARC_START_FLIP:
            mode = (last == kStartVertical) ? kStartHorizontal : kStartVertical;
            last = mode;
            break;
        case ARC_START_LINEAR:
            mode = kStartLinear;
            break;
        case ARC_ABOVE:
            mode = kUpArc;
            break;
        case ARC_BELOW:
            mode = kDownArc;
            break;
        default:
            break;
        }
        mArcs[i] = std::make_unique<Arc>(mode, time[i], time[i + 1],
                                         y[i][0], y[i][1], y[i + 1][0], y[i + 1][1]);
    }
}

ArcCurveFit::~ArcCurveFit() = default;

void ArcCurveFit::getPos(double t, std::vector<double>& v) {
    v.resize(2);
    if (mExtrapolate) {
        if (t < mArcs[0]->mTime1) {
            const double t0 = mArcs[0]->mTime1;
            const double dt = t - t0;
            const int p = 0;
            if (mArcs[p]->mLinear) {
                v[0] = mArcs[p]->getLinearX(t0) + dt * mArcs[p]->getLinearDX();
                v[1] = mArcs[p]->getLinearY(t0) + dt * mArcs[p]->getLinearDY();
            } else {
                mArcs[p]->setPoint(t0);
                v[0] = mArcs[p]->getX() + dt * mArcs[p]->getDX();
                v[1] = mArcs[p]->getY() + dt * mArcs[p]->getDY();
            }
            return;
        }
        if (t > mArcs.back()->mTime2) {
            const double t0 = mArcs.back()->mTime2;
            const double dt = t - t0;
            const int p = static_cast<int>(mArcs.size() - 1);
            if (mArcs[p]->mLinear) {
                v[0] = mArcs[p]->getLinearX(t0) + dt * mArcs[p]->getLinearDX();
                v[1] = mArcs[p]->getLinearY(t0) + dt * mArcs[p]->getLinearDY();
            } else {
                mArcs[p]->setPoint(t);
                v[0] = mArcs[p]->getX() + dt * mArcs[p]->getDX();
                v[1] = mArcs[p]->getY() + dt * mArcs[p]->getDY();
            }
            return;
        }
    } else {
        if (t < mArcs[0]->mTime1) t = mArcs[0]->mTime1;
        else if (t > mArcs.back()->mTime2) t = mArcs.back()->mTime2;
    }
    for (auto& arc : mArcs) {
        if (t <= arc->mTime2) {
            if (arc->mLinear) {
                v[0] = arc->getLinearX(t);
                v[1] = arc->getLinearY(t);
            } else {
                arc->setPoint(t);
                v[0] = arc->getX();
                v[1] = arc->getY();
            }
            return;
        }
    }
}

void ArcCurveFit::getPos(double t, std::vector<float>& v) {
    std::vector<double> d(2);
    getPos(t, d);
    v[0] = static_cast<float>(d[0]);
    v[1] = static_cast<float>(d[1]);
}

double ArcCurveFit::getPos(double t, int j) {
    if (mExtrapolate) {
        if (t < mArcs[0]->mTime1) {
            const double t0 = mArcs[0]->mTime1;
            const double dt = t - t0;
            const int p = 0;
            if (mArcs[p]->mLinear) {
                return (j == 0) ? mArcs[p]->getLinearX(t0) + dt * mArcs[p]->getLinearDX()
                       : mArcs[p]->getLinearY(t0) + dt * mArcs[p]->getLinearDY();
            }
            mArcs[p]->setPoint(t0);
            return (j == 0) ? mArcs[p]->getX() + dt * mArcs[p]->getDX()
                   : mArcs[p]->getY() + dt * mArcs[p]->getDY();
        }
        if (t > mArcs.back()->mTime2) {
            const double t0 = mArcs.back()->mTime2;
            const double dt = t - t0;
            const int p = static_cast<int>(mArcs.size() - 1);
            if (mArcs[p]->mLinear) {
                return (j == 0) ? mArcs[p]->getLinearX(t0) + dt * mArcs[p]->getLinearDX()
                       : mArcs[p]->getLinearY(t0) + dt * mArcs[p]->getLinearDY();
            }
            return (j == 0) ? mArcs[p]->getLinearX(t0) + dt * mArcs[p]->getLinearDX()
                   : mArcs[p]->getLinearY(t0) + dt * mArcs[p]->getLinearDY();
        }
    } else {
        if (t < mArcs[0]->mTime1) t = mArcs[0]->mTime1;
        else if (t > mArcs.back()->mTime2) t = mArcs.back()->mTime2;
    }
    for (auto& arc : mArcs) {
        if (t <= arc->mTime2) {
            if (arc->mLinear) {
                return (j == 0) ? arc->getLinearX(t) : arc->getLinearY(t);
            }
            arc->setPoint(t);
            return (j == 0) ? arc->getX() : arc->getY();
        }
    }
    return NAN;
}

void ArcCurveFit::getSlope(double t, std::vector<double>& v) {
    v.resize(2);
    if (t < mArcs[0]->mTime1) t = mArcs[0]->mTime1;
    else if (t > mArcs.back()->mTime2) t = mArcs.back()->mTime2;
    for (auto& arc : mArcs) {
        if (t <= arc->mTime2) {
            if (arc->mLinear) {
                v[0] = arc->getLinearDX();
                v[1] = arc->getLinearDY();
            } else {
                arc->setPoint(t);
                v[0] = arc->getDX();
                v[1] = arc->getDY();
            }
            return;
        }
    }
}

double ArcCurveFit::getSlope(double t, int j) {
    if (t < mArcs[0]->mTime1) t = mArcs[0]->mTime1;
    if (t > mArcs.back()->mTime2) t = mArcs.back()->mTime2;
    for (auto& arc : mArcs) {
        if (t <= arc->mTime2) {
            if (arc->mLinear) {
                return (j == 0) ? arc->getLinearDX() : arc->getLinearDY();
            }
            arc->setPoint(t);
            return (j == 0) ? arc->getDX() : arc->getDY();
        }
    }
    return NAN;
}

} // namespace cdroid
