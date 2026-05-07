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
#ifndef __TIME_CHART_H__
#define __TIME_CHART_H__

#include <widget/achart/chart/linechart.h>

namespace cdroid{
/**
 * The time chart rendering class.
 */
class TimeChart :public LineChart {
public:
    static constexpr long DAY = 24 * 60 * 60 * 1000;
private:
    /** The date format pattern to be used in formatting the X axis labels. */
    std::string mDateFormat;
    /** The starting point for labels. */
    double mStartPoint;

    DateFormat getDateFormat(double start, double end);
protected:
    void drawXLabels(std::vector<double>& xLabels, std::vector<double>& xTextLabelLocations, Canvas& canvas,
                Paint paint, int left, int top, int bottom, double xPixelsPerUnit, double minX, double maxX)override;
    std::vector<double> getXLabels(double min, double max, int count)override;
public:
    TimeChart();

    TimeChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset, const std::shared_ptr<XYMultipleSeriesRenderer>& renderer);

    std::string getDateFormat() const{
        return mDateFormat;
    }

    void setDateFormat(const std::string& format) {
        mDateFormat = format;
    }

    std::string getChartType() const override;
};
}/*endof namespace*/
#endif/*__TIME_CHART_H__*/
