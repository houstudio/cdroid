/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.HyperSpline.
 */
#include <widgetEx/constraintlayout/core/motion/hyperspline.h>

#include <cmath>

namespace cdroid {

HyperSpline::HyperSpline() = default;

HyperSpline::HyperSpline(const std::vector<std::vector<double>>& points) {
    setup(points);
}

void HyperSpline::setup(const std::vector<std::vector<double>>& points) {
    mDimensionality = (int) points[0].size();
    mPoints = (int) points.size();
    mCtl.assign(mDimensionality, std::vector<double>(mPoints, 0.0));
    mCurve.assign(mDimensionality, {});
    for (int d = 0; d < mDimensionality; d++) {
        for (int p = 0; p < mPoints; p++) {
            mCtl[d][p] = points[p][d];
        }
    }

    for (int d = 0; d < mDimensionality; d++) {
        mCurve[d] = calcNaturalCubic((int) mCtl[d].size(), mCtl[d]);
    }

    mCurveLength.assign(mPoints - 1, 0.0);
    mTotalLength = 0;
    std::vector<Cubic> temp(mDimensionality, Cubic(0, 0, 0, 0));
    for (int p = 0; p < (int) mCurveLength.size(); p++) {
        for (int d = 0; d < mDimensionality; d++) {
            temp[d] = mCurve[d][p];
        }
        mCurveLength[p] = approxLength(temp);
        mTotalLength += mCurveLength[p];
    }
}

void HyperSpline::getVelocity(double p, std::vector<double>& v) {
    double pos = p * mTotalLength;
    int k = 0;
    for (; k < (int) mCurveLength.size() - 1 && mCurveLength[k] < pos; k++) {
        pos -= mCurveLength[k];
    }
    for (int i = 0; i < (int) v.size(); i++) {
        v[i] = mCurve[i][k].vel(pos / mCurveLength[k]);
    }
}

void HyperSpline::getPos(double p, std::vector<double>& x) {
    double pos = p * mTotalLength;
    int k = 0;
    for (; k < (int) mCurveLength.size() - 1 && mCurveLength[k] < pos; k++) {
        pos -= mCurveLength[k];
    }
    for (int i = 0; i < (int) x.size(); i++) {
        x[i] = mCurve[i][k].eval(pos / mCurveLength[k]);
    }
}

void HyperSpline::getPos(double p, std::vector<float>& x) {
    double pos = p * mTotalLength;
    int k = 0;
    for (; k < (int) mCurveLength.size() - 1 && mCurveLength[k] < pos; k++) {
        pos -= mCurveLength[k];
    }
    for (int i = 0; i < (int) x.size(); i++) {
        x[i] = (float) mCurve[i][k].eval(pos / mCurveLength[k]);
    }
}

double HyperSpline::getPos(double p, int splineNumber) {
    double pos = p * mTotalLength;
    int k = 0;
    for (; k < (int) mCurveLength.size() - 1 && mCurveLength[k] < pos; k++) {
        pos -= mCurveLength[k];
    }
    return mCurve[splineNumber][k].eval(pos / mCurveLength[k]);
}

double HyperSpline::approxLength(const std::vector<Cubic>& curve) {
    double sum = 0;
    int n = (int) curve.size();
    std::vector<double> oldv(n, 0.0);
    for (double i = 0; i < 1; i += .1) {
        double s = 0;
        for (int j = 0; j < n; j++) {
            double tmp = oldv[j];
            tmp -= oldv[j] = curve[j].eval(i);
            s += tmp * tmp;
        }
        if (i > 0) {
            sum += std::sqrt(s);
        }
    }
    double s = 0;
    for (int j = 0; j < n; j++) {
        double tmp = oldv[j];
        tmp -= oldv[j] = curve[j].eval(1);
        s += tmp * tmp;
    }
    sum += std::sqrt(s);
    return sum;
}

std::vector<HyperSpline::Cubic> HyperSpline::calcNaturalCubic(int n, const std::vector<double>& x) {
    std::vector<double> gamma(n, 0.0);
    std::vector<double> delta(n, 0.0);
    std::vector<double> d(n, 0.0);
    n -= 1;

    gamma[0] = 1.0f / 2.0f;
    for (int i = 1; i < n; i++) {
        gamma[i] = 1 / (4 - gamma[i - 1]);
    }
    gamma[n] = 1 / (2 - gamma[n - 1]);

    delta[0] = 3 * (x[1] - x[0]) * gamma[0];
    for (int i = 1; i < n; i++) {
        delta[i] = (3 * (x[i + 1] - x[i - 1]) - delta[i - 1]) * gamma[i];
    }
    delta[n] = (3 * (x[n] - x[n - 1]) - delta[n - 1]) * gamma[n];

    d[n] = delta[n];
    for (int i = n - 1; i >= 0; i--) {
        d[i] = delta[i] - gamma[i] * d[i + 1];
    }

    std::vector<Cubic> c;
    c.reserve(n);
    for (int i = 0; i < n; i++) {
        c.push_back(Cubic((float) x[i], d[i],
                          3 * (x[i + 1] - x[i]) - 2 * d[i] - d[i + 1],
                          2 * (x[i] - x[i + 1]) + d[i] + d[i + 1]));
    }
    return c;
}

} // namespace cdroid
