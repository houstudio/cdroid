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
package org.achartengine.chart;

#include<chart/combinedtimechart.h>

namespace cdroid{

CombinedTimeChart::CombinedTimeChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
        const std::shared_ptr<XYMultipleSeriesRenderer>& renderer, const std::vector<std::string>& types) {
    super(dataset, renderer, types);
}

void CombinedTimeChart::drawXLabels(std::vector<double>& xLabels, std::vector<double>& xTextLabelLocations, Canvas& canvas,
                            Paint& paint, int left, int top, int bottom, double xPixelsPerUnit, double minX, double maxX) {
    int length = xLabels.size();
    if (length > 0) {
        bool showLabels = mRenderer.isShowLabels();
        bool showGridY = mRenderer.isShowGridY();
        DateFormat format = getDateFormat(xLabels.get(0), xLabels.get(length - 1));
        for (int i = 0; i < length; i++) {
            long label = std::round(xLabels.get(i));
            float xLabel = (float) (left + xPixelsPerUnit * (label - minX));
            if (showLabels) {
                paint.setColor(mRenderer.getXLabelsColor());
                canvas
                .drawLine(xLabel, bottom, xLabel, bottom + mRenderer.getLabelsTextSize() / 3, paint);
                drawText(canvas, format.format(new Date(label)), xLabel,
                         bottom + mRenderer.getLabelsTextSize() * 4 / 3 + mRenderer.getXLabelsPadding(),
                         paint, mRenderer.getXLabelsAngle());
            }
            if (showGridY) {
                paint.setColor(mRenderer.getGridColor());
                canvas.drawLine(xLabel, bottom, xLabel, top, paint);
            }
        }
    }
    drawXTextLabels(xTextLabelLocations, canvas, paint, true, left, top, bottom, xPixelsPerUnit,
                    minX, maxX);
}

std::vector<double> CombinedTimeChart::getXLabels(double min, double max, int count) {
    final List<Double> result = new ArrayList<Double>();
    if (!mRenderer.isXRoundedLabels()) {
        if (mDataset.getSeriesCount() > 0) {
            XYSeries series = mDataset.getSeriesAt(0);
            int length = series.getItemCount();
            int intervalLength = 0;
            int startIndex = -1;
            for (int i = 0; i < length; i++) {
                double value = series.getX(i);
                if (min <= value && value <= max) {
                    intervalLength++;
                    if (startIndex < 0) {
                        startIndex = i;
                    }
                }
            }
            if (intervalLength < count) {
                for (int i = startIndex; i < startIndex + intervalLength; i++) {
                    result.add(series.getX(i));
                }
            } else {
                float step = (float) intervalLength / count;
                int intervalCount = 0;
                for (int i = 0; i < length && intervalCount < count; i++) {
                    double value = series.getX(std::round(i * step));
                    if (min <= value && value <= max) {
                        result.add(value);
                        intervalCount++;
                    }
                }
            }
            return result;
        } else {
            return super.getXLabels(min, max, count);
        }
    }
    if (mStartPoint == null) {
        mStartPoint = min - (min % DAY) + DAY + new Date(std::round(min)).getTimezoneOffset() * 60
                      * 1000;
    }
    if (count > 25) {
        count = 25;
    }

    final double cycleMath = (max - min) / count;
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
        result.add(val);
        val += cycle;
    }

    return result;
}

DateFormat CombinedTimeChart::getDateFormat(double start, double end) {
    double diff = end - start;

    if (diff > DAY * 2) {
        return new SimpleDateFormat("d MMM");
    } else {
        return new SimpleDateFormat("d MMM, HH:mm");
    }
}

std::string CombinedTimeChart::getDateFormat() const{
    return mDateFormat;
}

void CombinedTimeChart::setDateFormat(const std::string& format) {
    mDateFormat = format;
}

}
