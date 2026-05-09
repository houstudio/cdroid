/**
 * Copyright (C) 2013 Henning Dodenhof
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
#include<widget/achart/chart/timechart.h>
#include<widget/achart/chart/combinedtimechart.h>

namespace cdroid{

CombinedTimeChart::CombinedTimeChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
        const std::shared_ptr<XYMultipleSeriesRenderer>& renderer, const std::vector<std::string>& types)
    :CombinedXYChart(dataset, renderer, types){
}

void CombinedTimeChart::drawXLabels(const std::vector<double>& xLabels,const std::vector<double>& xTextLabelLocations,
        Canvas& canvas, Paint& paint, int left, int top, int bottom, double xPixelsPerUnit, double minX, double maxX) {
    int length = xLabels.size();
    if (length > 0) {
        const bool showLabels = mRenderer->isShowLabels();
        const bool showGridY = mRenderer->isShowGridY();
        TimeChart::DateFormat format(getDateFormat(xLabels.at(0), xLabels.at(length - 1)));
        for (int i = 0; i < length; i++) {
            int64_t label = std::round(xLabels.at(i));
            float xLabel = (float) (left + xPixelsPerUnit * (label - minX));
            if (showLabels) {
                canvas.set_color(mRenderer->getXLabelsColor());
                canvas.move_to(xLabel, bottom);
                canvas.line_to(xLabel, bottom + mRenderer->getLabelsTextSize() / 3);
                canvas.stroke();
                drawText(canvas, format.format(label), xLabel,
                         bottom + mRenderer->getLabelsTextSize() * 4 / 3 + mRenderer->getXLabelsPadding(),
                         paint, mRenderer->getXLabelsAngle());
            }
            if (showGridY) {
                canvas.set_color(mRenderer->getGridColor());
                canvas.move_to(xLabel, bottom);
                canvas.line_to(xLabel, top);
            }
        }
    }
    drawXTextLabels(xTextLabelLocations, canvas, paint, true, left, top, bottom, xPixelsPerUnit,
                    minX, maxX);
}

std::vector<double> CombinedTimeChart::getXLabels(double min, double max, int count) {
    std::vector<double> result;
    if (!mRenderer->isXRoundedLabels()) {
        if (mDataset->getSeriesCount() > 0) {
            auto series = mDataset->getSeriesAt(0);
            int length = series->getItemCount();
            int intervalLength = 0;
            int startIndex = -1;
            for (int i = 0; i < length; i++) {
                double value = series->getX(i);
                if (min <= value && value <= max) {
                    intervalLength++;
                    if (startIndex < 0) {
                        startIndex = i;
                    }
                }
            }
            if (intervalLength < count) {
                for (int i = startIndex; i < startIndex + intervalLength; i++) {
                    result.push_back(series->getX(i));
                }
            } else {
                float step = (float) intervalLength / count;
                int intervalCount = 0;
                for (int i = 0; i < length && intervalCount < count; i++) {
                    double value = series->getX(std::round(i * step));
                    if (min <= value && value <= max) {
                        result.push_back(value);
                        intervalCount++;
                    }
                }
            }
            return result;
        } else {
            return CombinedXYChart::getXLabels(min, max, count);
        }
    }
    if (std::isnan(mStartPoint)) {
        //mStartPoint = min - (min % DAY) + DAY + new Date(std::round(min)).getTimezoneOffset() * 60* 1000;
        mStartPoint = min - std::fmod(min, static_cast<double>(CombinedTimeChart::DAY)) + CombinedTimeChart::DAY;
    }
    if (count > 25) {
        count = 25;
    }

    const double cycleMath = (max - min) / count;
    if (cycleMath <= 0) {
        return result;
    }
    double cycle = DAY;

    if (cycleMath <= DAY) {
        while (cycleMath < cycle / 2) {
            cycle = cycle / 2;
        }
    } else {
        while (cycleMath > cycle) {
            cycle = cycle * 2;
        }
    }

    double val = mStartPoint - std::floor((mStartPoint - min) / cycle) * cycle;
    int i = 0;
    while (val < max && i++ <= count) {
        result.push_back(val);
        val += cycle;
    }

    return result;
}

std::string CombinedTimeChart::getDateFormat(double start, double end) const{
    double diff = end - start;

    if (diff > DAY * 2) {
        return "d MMM";
    } else {
        return "d MMM, HH:mm";
    }
}

std::string CombinedTimeChart::getDateFormat() const{
    return mDateFormat;
}

void CombinedTimeChart::setDateFormat(const std::string& format) {
    mDateFormat = format;
}

}
