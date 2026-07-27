/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.Oscillator.
 */
#include <widgetEx/constraintlayout/core/motion/oscillator.h>

#include <algorithm>
#include <cmath>
#include <sstream>

namespace cdroid {

namespace {
// Java Arrays.binarySearch semantics: index of key if present, else (-(insertion point) - 1).
// `a` must be sorted ascending.
int binSearch(const std::vector<double>& a, double key) {
    auto it = std::lower_bound(a.begin(), a.end(), key);
    int idx = (int) (it - a.begin());
    if (it != a.end() && *it == key) {
        return idx;
    }
    return -idx - 1;
}

// Java Math.signum(double): 0 at 0, else +/-1.
double signum(double d) {
    return (d > 0.0) ? 1.0 : (d < 0.0) ? -1.0 : 0.0;
}
} // namespace

Oscillator::Oscillator() : mPI2(M_PI * 2) {}

std::string Oscillator::toString() const {
    std::ostringstream ss;
    ss << "pos =";
    for (double p : mPosition) ss << " " << p;
    ss << " period=";
    for (float p : mPeriod) ss << " " << p;
    return ss.str();
}

void Oscillator::setType(int type, const std::string& customType) {
    mType = type;
    mCustomType = customType;
    if (!mCustomType.empty()) {
        mCustomCurve = std::make_unique<MonotonicCurveFit>(MonotonicCurveFit::buildWave(customType));
    }
}

void Oscillator::addPoint(double position, float period) {
    int len = (int) mPeriod.size() + 1;
    int j = binSearch(mPosition, position);
    if (j < 0) {
        j = -j - 1;
    }
    mPosition.resize(len);
    mPeriod.resize(len);
    mArea.assign(len, 0.0);
    // shift [j, len-1) up by one to make room at j (Java: System.arraycopy(j -> j+1, len-j-1)).
    for (int i = len - 1; i > j; i--) {
        mPosition[i] = mPosition[i - 1];
        mPeriod[i] = mPeriod[i - 1];
    }
    mPosition[j] = position;
    mPeriod[j] = period;
    mNormalized = false;
}

void Oscillator::normalize() {
    double totalArea = 0;
    double totalCount = 0;
    for (size_t i = 0; i < mPeriod.size(); i++) {
        totalCount += mPeriod[i];
    }
    for (size_t i = 1; i < mPeriod.size(); i++) {
        float h = (mPeriod[i - 1] + mPeriod[i]) / 2;
        double w = mPosition[i] - mPosition[i - 1];
        totalArea = totalArea + w * h;
    }
    for (size_t i = 0; i < mPeriod.size(); i++) {
        mPeriod[i] *= (float) (totalCount / totalArea);
    }
    mArea[0] = 0;
    for (size_t i = 1; i < mPeriod.size(); i++) {
        float h = (mPeriod[i - 1] + mPeriod[i]) / 2;
        double w = mPosition[i] - mPosition[i - 1];
        mArea[i] = mArea[i - 1] + w * h;
    }
    mNormalized = true;
}

double Oscillator::getP(double time) const {
    if (time < 0) {
        time = 0;
    } else if (time > 1) {
        time = 1;
    }
    int index = binSearch(mPosition, time);
    double p = 0;
    if (index > 0) {
        p = 1;
    } else if (index != 0) {
        index = -index - 1;
        double t = time;
        double m = (mPeriod[index] - mPeriod[index - 1])
                   / (mPosition[index] - mPosition[index - 1]);
        p = mArea[index - 1]
            + (mPeriod[index - 1] - m * mPosition[index - 1]) * (t - mPosition[index - 1])
            + m * (t * t - mPosition[index - 1] * mPosition[index - 1]) / 2;
    }
    return p;
}

double Oscillator::getValue(double time, double phase) const {
    double angle = phase + getP(time);
    switch (mType) {
    default:
    case SIN_WAVE:
        return std::sin(mPI2 * angle);
    case SQUARE_WAVE:
        return signum(0.5 - std::fmod(angle, 1.0)); // signum(0.5 - angle%1)
    case TRIANGLE_WAVE:
        return 1 - std::fabs(std::fmod(angle * 4 + 1, 4) - 2);
    case SAW_WAVE:
        return std::fmod(angle * 2 + 1, 2) - 1;
    case REVERSE_SAW_WAVE:
        return (1 - std::fmod(angle * 2 + 1, 2));
    case COS_WAVE:
        return std::cos(mPI2 * (phase + angle));
    case BOUNCE: {
        double x = 1 - std::fabs(std::fmod(angle * 4, 4) - 2);
        return 1 - x * x;
    }
    case CUSTOM:
        return mCustomCurve->getPos(std::fmod(angle, 1.0), 0);
    }
}

double Oscillator::getDP(double time) const {
    if (time <= 0) {
        time = 0.00001;
    } else if (time >= 1) {
        time = .999999;
    }
    int index = binSearch(mPosition, time);
    double p = 0;
    if (index > 0) {
        return 0;
    }
    if (index != 0) {
        index = -index - 1;
        double t = time;
        double m = (mPeriod[index] - mPeriod[index - 1])
                   / (mPosition[index] - mPosition[index - 1]);
        p = m * t + (mPeriod[index - 1] - m * mPosition[index - 1]);
    }
    return p;
}

double Oscillator::getSlope(double time, double phase, double dphase) const {
    double angle = phase + getP(time);
    double dangle_dtime = getDP(time) + dphase;
    switch (mType) {
    default:
    case SIN_WAVE:
        return mPI2 * dangle_dtime * std::cos(mPI2 * angle);
    case SQUARE_WAVE:
        return 0;
    case TRIANGLE_WAVE:
        return 4 * dangle_dtime * signum(std::fmod(angle * 4 + 3, 4) - 2);
    case SAW_WAVE:
        return dangle_dtime * 2;
    case REVERSE_SAW_WAVE:
        return -dangle_dtime * 2;
    case COS_WAVE:
        return -mPI2 * dangle_dtime * std::sin(mPI2 * angle);
    case BOUNCE:
        return 4 * dangle_dtime * (std::fmod(angle * 4 + 2, 4) - 2);
    case CUSTOM:
        return mCustomCurve->getSlope(std::fmod(angle, 1.0), 0);
    }
}

} // namespace cdroid
