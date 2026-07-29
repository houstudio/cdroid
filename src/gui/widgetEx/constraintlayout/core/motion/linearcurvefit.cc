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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.LinearCurveFit.
 */
#include <widgetEx/constraintlayout/core/motion/linearcurvefit.h>

#include <cmath>
#include <limits>

namespace cdroid {

LinearCurveFit::LinearCurveFit(const std::vector<double>& time,
                               const std::vector<std::vector<double>>& y) {
    const int dim = (int) y[0].size();
    mSlopeTemp.assign(dim, 0.0);
    mT = time;
    mY = y;
    mTotalLength = std::numeric_limits<double>::quiet_NaN();
    if (dim > 2) {
        // Upstream computes a 2D length here but discards it (mTotalLength := 0); the only
        // consumer is the unused getLength2D(). Ported verbatim.
        double sum = 0;
        double lastx = 0, lasty = 0;
        for (int i = 0; i < (int) time.size(); i++) {
            double px = y[i][0];
            double py = y[i][0];
            if (i > 0) {
                sum += std::hypot(px - lastx, py - lasty);
            }
            lastx = px;
            lasty = py;
        }
        mTotalLength = 0;
    }
}

double LinearCurveFit::getLength2D(double t) {
    if (std::isnan(mTotalLength)) {
        return 0;
    }
    const int n = (int) mT.size();
    if (t <= mT[0]) {
        return 0;
    }
    if (t >= mT[n - 1]) {
        return mTotalLength;
    }
    double sum = 0;
    double last_x = 0, last_y = 0;

    for (int i = 0; i < n - 1; i++) {
        double px = mY[i][0];
        double py = mY[i][1];
        if (i > 0) {
            sum += std::hypot(px - last_x, py - last_y);
        }
        last_x = px;
        last_y = py;
        if (t == mT[i]) {
            return sum;
        }
        if (t < mT[i + 1]) {
            double h = mT[i + 1] - mT[i];
            double x = (t - mT[i]) / h;
            double x1 = mY[i][0];
            double x2 = mY[i + 1][0];
            double y1 = mY[i][1];
            double y2 = mY[i + 1][1];

            py -= y1 * (1 - x) + y2 * x;
            px -= x1 * (1 - x) + x2 * x;
            sum += std::hypot(py, px);

            return sum;
        }
    }
    return 0;
}

void LinearCurveFit::getPos(double t, std::vector<double>& v) {
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
                v[j] = y1 * (1 - x) + y2 * x;
            }
            return;
        }
    }
}

void LinearCurveFit::getPos(double t, std::vector<float>& v) {
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
                v[j] = (float) (y1 * (1 - x) + y2 * x);
            }
            return;
        }
    }
}

double LinearCurveFit::getPos(double t, int j) {
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
            return (y1 * (1 - x) + y2 * x);
        }
    }
    return 0; // should never reach here
}

void LinearCurveFit::getSlope(double t, std::vector<double>& v) {
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
            for (int j = 0; j < dim; j++) {
                double y1 = mY[i][j];
                double y2 = mY[i + 1][j];
                v[j] = (y2 - y1) / h;
            }
            break;
        }
    }
}

double LinearCurveFit::getSlope(double t, int j) {
    const int n = (int) mT.size();

    if (t < mT[0]) {
        t = mT[0];
    } else if (t >= mT[n - 1]) {
        t = mT[n - 1];
    }
    for (int i = 0; i < n - 1; i++) {
        if (t <= mT[i + 1]) {
            double h = mT[i + 1] - mT[i];
            double y1 = mY[i][j];
            double y2 = mY[i + 1][j];
            return (y2 - y1) / h;
        }
    }
    return 0; // should never reach here
}

std::vector<double> LinearCurveFit::getTimePoints() {
    return mT;
}

} // namespace cdroid
