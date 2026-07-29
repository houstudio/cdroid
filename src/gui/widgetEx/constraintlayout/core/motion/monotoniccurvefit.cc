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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.MonotonicCurveFit.
 */
#include <widgetEx/constraintlayout/core/motion/monotoniccurvefit.h>

#include <cmath>

namespace cdroid {

MonotonicCurveFit::MonotonicCurveFit(const std::vector<double>& time,
                                     const std::vector<std::vector<double>>& y) {
    const int n = (int) time.size();
    const int dim = (int) y[0].size();
    mSlopeTemp.assign(dim, 0.0);
    std::vector<std::vector<double>> slope(n - 1, std::vector<double>(dim, 0.0));
    std::vector<std::vector<double>> tangent(n, std::vector<double>(dim, 0.0));
    for (int j = 0; j < dim; j++) {
        for (int i = 0; i < n - 1; i++) {
            double dt = time[i + 1] - time[i];
            slope[i][j] = (y[i + 1][j] - y[i][j]) / dt;
            if (i == 0) {
                tangent[i][j] = slope[i][j];
            } else {
                tangent[i][j] = (slope[i - 1][j] + slope[i][j]) * 0.5f;
            }
        }
        tangent[n - 1][j] = slope[n - 2][j];
    }

    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < dim; j++) {
            if (slope[i][j] == 0.) {
                tangent[i][j] = 0.;
                tangent[i + 1][j] = 0.;
            } else {
                double a = tangent[i][j] / slope[i][j];
                double b = tangent[i + 1][j] / slope[i][j];
                double h = std::hypot(a, b);
                if (h > 9.0) {
                    double t = 3. / h;
                    tangent[i][j] = t * a * slope[i][j];
                    tangent[i + 1][j] = t * b * slope[i][j];
                }
            }
        }
    }
    mT = time;
    mY = y;
    mTangent = tangent;
}

void MonotonicCurveFit::getPos(double t, std::vector<double>& v) {
    const int n = (int) mT.size();
    const int dim = (int) mY[0].size();
    if (mExtrapolate) {
        if (t <= mT[0]) {
            getSlope(mT[0], mSlopeTemp);
            for (int j = 0; j < dim; j++) {
                v[j] = mY[0][j] + (t - mT[0]) * mSlopeTemp[j];
            }
            return;
        }
        if (t >= mT[n - 1]) {
            getSlope(mT[n - 1], mSlopeTemp);
            for (int j = 0; j < dim; j++) {
                v[j] = mY[n - 1][j] + (t - mT[n - 1]) * mSlopeTemp[j];
            }
            return;
        }
    } else {
        if (t <= mT[0]) {
            for (int j = 0; j < dim; j++) {
                v[j] = mY[0][j];
            }
            return;
        }
        if (t >= mT[n - 1]) {
            for (int j = 0; j < dim; j++) {
                v[j] = mY[n - 1][j];
            }
            return;
        }
    }

    for (int i = 0; i < n - 1; i++) {
        if (t == mT[i]) {
            for (int j = 0; j < dim; j++) {
                v[j] = mY[i][j];
            }
        }
        if (t < mT[i + 1]) {
            double h = mT[i + 1] - mT[i];
            double x = (t - mT[i]) / h;
            for (int j = 0; j < dim; j++) {
                double y1 = mY[i][j];
                double y2 = mY[i + 1][j];
                double t1 = mTangent[i][j];
                double t2 = mTangent[i + 1][j];
                v[j] = interpolate(h, x, y1, y2, t1, t2);
            }
            return;
        }
    }
}

void MonotonicCurveFit::getPos(double t, std::vector<float>& v) {
    const int n = (int) mT.size();
    const int dim = (int) mY[0].size();
    if (mExtrapolate) {
        if (t <= mT[0]) {
            getSlope(mT[0], mSlopeTemp);
            for (int j = 0; j < dim; j++) {
                v[j] = (float) (mY[0][j] + (t - mT[0]) * mSlopeTemp[j]);
            }
            return;
        }
        if (t >= mT[n - 1]) {
            getSlope(mT[n - 1], mSlopeTemp);
            for (int j = 0; j < dim; j++) {
                v[j] = (float) (mY[n - 1][j] + (t - mT[n - 1]) * mSlopeTemp[j]);
            }
            return;
        }
    } else {
        if (t <= mT[0]) {
            for (int j = 0; j < dim; j++) {
                v[j] = (float) mY[0][j];
            }
            return;
        }
        if (t >= mT[n - 1]) {
            for (int j = 0; j < dim; j++) {
                v[j] = (float) mY[n - 1][j];
            }
            return;
        }
    }

    for (int i = 0; i < n - 1; i++) {
        if (t == mT[i]) {
            for (int j = 0; j < dim; j++) {
                v[j] = (float) mY[i][j];
            }
        }
        if (t < mT[i + 1]) {
            double h = mT[i + 1] - mT[i];
            double x = (t - mT[i]) / h;
            for (int j = 0; j < dim; j++) {
                double y1 = mY[i][j];
                double y2 = mY[i + 1][j];
                double t1 = mTangent[i][j];
                double t2 = mTangent[i + 1][j];
                v[j] = (float) interpolate(h, x, y1, y2, t1, t2);
            }
            return;
        }
    }
}

double MonotonicCurveFit::getPos(double t, int j) {
    const int n = (int) mT.size();
    if (mExtrapolate) {
        if (t <= mT[0]) {
            return mY[0][j] + (t - mT[0]) * getSlope(mT[0], j);
        }
        if (t >= mT[n - 1]) {
            return mY[n - 1][j] + (t - mT[n - 1]) * getSlope(mT[n - 1], j);
        }
    } else {
        if (t <= mT[0]) {
            return mY[0][j];
        }
        if (t >= mT[n - 1]) {
            return mY[n - 1][j];
        }
    }

    for (int i = 0; i < n - 1; i++) {
        if (t == mT[i]) {
            return mY[i][j];
        }
        if (t < mT[i + 1]) {
            double h = mT[i + 1] - mT[i];
            double x = (t - mT[i]) / h;
            double y1 = mY[i][j];
            double y2 = mY[i + 1][j];
            double t1 = mTangent[i][j];
            double t2 = mTangent[i + 1][j];
            return interpolate(h, x, y1, y2, t1, t2);
        }
    }
    return 0; // should never reach here
}

void MonotonicCurveFit::getSlope(double t, std::vector<double>& v) {
    const int n = (int) mT.size();
    int dim = (int) mY[0].size();
    if (t <= mT[0]) {
        t = mT[0];
    } else if (t >= mT[n - 1]) {
        t = mT[n - 1];
    }

    for (int i = 0; i < n - 1; i++) {
        if (t <= mT[i + 1]) {
            double h = mT[i + 1] - mT[i];
            double x = (t - mT[i]) / h;
            for (int j = 0; j < dim; j++) {
                double y1 = mY[i][j];
                double y2 = mY[i + 1][j];
                double t1 = mTangent[i][j];
                double t2 = mTangent[i + 1][j];
                v[j] = diff(h, x, y1, y2, t1, t2) / h;
            }
            break;
        }
    }
}

double MonotonicCurveFit::getSlope(double t, int j) {
    const int n = (int) mT.size();

    if (t < mT[0]) {
        t = mT[0];
    } else if (t >= mT[n - 1]) {
        t = mT[n - 1];
    }
    for (int i = 0; i < n - 1; i++) {
        if (t <= mT[i + 1]) {
            double h = mT[i + 1] - mT[i];
            double x = (t - mT[i]) / h;
            double y1 = mY[i][j];
            double y2 = mY[i + 1][j];
            double t1 = mTangent[i][j];
            double t2 = mTangent[i + 1][j];
            return diff(h, x, y1, y2, t1, t2) / h;
        }
    }
    return 0; // should never reach here
}

std::vector<double> MonotonicCurveFit::getTimePoints() {
    return mT;
}

double MonotonicCurveFit::interpolate(double h, double x, double y1, double y2, double t1, double t2) {
    double x2 = x * x;
    double x3 = x2 * x;
    return -2 * x3 * y2 + 3 * x2 * y2 + 2 * x3 * y1 - 3 * x2 * y1 + y1
           + h * t2 * x3 + h * t1 * x3 - h * t2 * x2 - 2 * h * t1 * x2
           + h * t1 * x;
}

double MonotonicCurveFit::diff(double h, double x, double y1, double y2, double t1, double t2) {
    double x2 = x * x;
    return -6 * x2 * y2 + 6 * x * y2 + 6 * x2 * y1 - 6 * x * y1 + 3 * h * t2 * x2
           + 3 * h * t1 * x2 - 2 * h * t2 * x - 4 * h * t1 * x + h * t1;
}

MonotonicCurveFit MonotonicCurveFit::buildWave(const std::string& configString) {
    const std::string& str = configString;
    std::vector<double> values(str.length() / 2);
    size_t start = configString.find('(') + 1;
    size_t off1 = configString.find(',', start);
    int count = 0;
    while (off1 != std::string::npos) {
        std::string tmp = configString.substr(start, off1 - start);
        values[count++] = std::stod(tmp);
        start = off1 + 1;
        off1 = configString.find(',', start);
    }
    off1 = configString.find(')', start);
    std::string tmp = configString.substr(start, off1 - start);
    values[count++] = std::stod(tmp);

    values.resize(count);
    return buildWave(values);
}

MonotonicCurveFit MonotonicCurveFit::buildWave(const std::vector<double>& values) {
    int length = (int) values.size() * 3 - 2;
    int len = (int) values.size() - 1;
    double gap = 1.0 / len;
    std::vector<std::vector<double>> points(length, std::vector<double>(1, 0.0));
    std::vector<double> time(length, 0.0);
    for (int i = 0; i < (int) values.size(); i++) {
        double v = values[i];
        points[i + len][0] = v;
        time[i + len] = i * gap;
        if (i > 0) {
            points[i + len * 2][0] = v + 1;
            time[i + len * 2] = i * gap + 1;

            points[i - 1][0] = v - 1 - gap;
            time[i - 1] = i * gap + -1 - gap;
        }
    }
    return MonotonicCurveFit(time, points);
}

} // namespace cdroid
