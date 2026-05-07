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
package org.achartengine.chart;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.achartengine.model.XYMultipleSeriesDataset;
import org.achartengine.model.XYSeries;
import org.achartengine.renderer.XYMultipleSeriesRenderer;

import android.graphics.Canvas;
import android.graphics.Paint;

namespace cdroid{
/**
 * The time chart rendering class.
 */
//public static final String TYPE = "Time";

TimeChart::TimeChart() {
}

TimeChart::TimeChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset, const std::shared_ptr<XYMultipleSeriesRenderer>& renderer) {
    super(dataset, renderer);
}

std::string TimeChart::getDateFormat() const{
    return mDateFormat;
}

void TimeChart::setDateFormat(const std::string& format) {
    mDateFormat = format;
}

void TimeChart::drawXLabels(std::vector<double>& xLabels, std::vector<double>& xTextLabelLocations, Canvas& canvas,
                           Paint paint, int left, int top, int bottom, double xPixelsPerUnit, double minX, double maxX) {
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
                         bottom + mRenderer.getLabelsTextSize() * 4 / 3 + mRenderer.getXLabelsPadding(), paint, mRenderer.getXLabelsAngle());
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

DateFormat TimeChart::getDateFormat(double start, double end) {
    if (mDateFormat != null) {
        SimpleDateFormat format = null;
        try {
            format = new SimpleDateFormat(mDateFormat);
            return format;
        } catch (Exception e) {
            // do nothing here
        }
    }
    DateFormat format = SimpleDateFormat.getDateInstance(SimpleDateFormat.MEDIUM);
    double diff = end - start;
    if (diff > DAY && diff < 5 * DAY) {
        format = SimpleDateFormat.getDateTimeInstance(SimpleDateFormat.SHORT, SimpleDateFormat.SHORT);
    } else if (diff < DAY) {
        format = SimpleDateFormat.getTimeInstance(SimpleDateFormat.MEDIUM);
    }
    return format;
}

std::string TimeChart::getChartType() const{
    return TYPE;
}

std::vector<double> TimeChart::getXLabels(double min, double max, int count) {
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
}/*endof namespace*/
