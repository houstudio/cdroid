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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.StepCurve.
 */
#include <widgetEx/constraintlayout/core/motion/stepcurve.h>

namespace cdroid {

StepCurve::StepCurve(const std::string& configString) {
    mStr = configString;
    std::vector<double> values(mStr.length() / 2);
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
    mCurveFit = std::make_unique<MonotonicCurveFit>(genSpline(values));
}

MonotonicCurveFit StepCurve::genSpline(const std::vector<double>& values) {
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

double StepCurve::getDiff(double x) const {
    return mCurveFit->getSlope(x, 0);
}

double StepCurve::get(double x) const {
    return mCurveFit->getPos(x, 0);
}

} // namespace cdroid
