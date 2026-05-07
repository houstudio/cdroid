/**
 * Copyright (C) 2009 - 2012 SC 4ViewSoft SRL
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __MATH_HELPER_H__
#define __MATH_HELPER_H__
#include <cfloat>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
namespace cdroid{
class MathHelper {
public:
    /** A value that is used a null value. */
    static constexpr double NULL_VALUE = DBL_MAX;
    /**
     * A number formatter to be used to make sure we have a maximum number of
     * fraction digits in the labels.
     */
    MathHelper() =default;
public:
    static std::vector<double> minmax(std::vector<double>& values) {
        if (values.size() == 0) {
            return {0,0};//new double[2];
        }
        double min = values.at(0);
        double max = min;
        int length = values.size();
        for (int i = 1; i < length; i++) {
            double value = values.at(i);
            min = std::min(min, value);
            max = std::max(max, value);
        }
        return { min, max };
    }

    static std::vector<double> getLabels(double start, double end,int approxNumLabels) {
        std::vector<double> labels;
        if (approxNumLabels <= 0) {
            return labels;
        }
        std::vector<double> labelParams = computeLabels(start, end, approxNumLabels);
        // when the start > end the inc will be negative so it will still work
        const int numLabels = 1 + (int) ((labelParams[1] - labelParams[0]) / labelParams[2]);
        // we want the range to be inclusive but we don't want to blow up when
        // looping for the case where the min and max are the same. So we loop
        // on
        // numLabels not on the values.
        for (int i = 0; i < numLabels; i++) {
            double z = labelParams[0] + i * labelParams[2];
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(5) << z;
            std::string formattedStr = oss.str();
            std::istringstream iss(formattedStr);
            double parsedValue;
            if (iss >> parsedValue) {
                z = parsedValue;
            }
            labels.push_back(z);
        }
        return labels;
    }
private:
    static std::vector<double> computeLabels(double start, double end,int approxNumLabels) {
        if (std::abs(start - end) < 0.0000001f) {
            return { start, start, 0 };
        }
        double s = start;
        double e = end;
        bool switched = false;
        if (s > e) {
            switched = true;
            double tmp = s;
            s = e;
            e = tmp;
        }
        double xStep = roundUp(std::abs(s - e) / approxNumLabels);
        // Compute x starting point so it is a multiple of xStep.
        double xStart = xStep * std::ceil(s / xStep);
        double xEnd = xStep * std::floor(e / xStep);
        if (switched) {
            return { xEnd, xStart, -1.0 * xStep };
        }
        return { xStart, xEnd, xStep };
    }

    static double roundUp(double val) {
        int exponent = (int) std::floor(std::log10(val));
        double rval = val * std::pow(10, -exponent);
        if (rval > 5.0) {
            rval = 10.0;
        } else if (rval > 2.0) {
            rval = 5.0;
        } else if (rval > 1.0) {
            rval = 2.0;
        }
        rval *= std::pow(10, exponent);
        return rval;
    }
};
}/*endof namespace*/
#endif/*__MATH_HELPER_H__*/
