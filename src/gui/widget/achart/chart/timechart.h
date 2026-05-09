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
#ifndef __ACHART_TIME_CHART_H__
#define __ACHART_TIME_CHART_H__

#include <widget/achart/chart/linechart.h>

namespace cdroid{
/**
 * The time chart rendering class.
 */
class TimeChart :public LineChart {
public:
    static constexpr long DAY = 24 * 60 * 60 * 1000;
    class DateFormat {
    private:
        std::string mPattern;
    public:
        DateFormat(const std::string& fmt_pattern);
        std::string format(std::int64_t timestamp_ms) const;
    };
private:
    /** The date format pattern to be used in formatting the X axis labels. */
    std::string mDateFormat;
    /** The starting point for labels. */
    mutable double mStartPoint;

    std::string getDateFormat(double start, double end)const;
protected:
    void drawXLabels(const std::vector<double>& xLabels,const std::vector<double>& xTextLabelLocations, Canvas& canvas,
                Paint& paint, int left, int top, int bottom, double xPixelsPerUnit, double minX, double maxX)override;
    std::vector<double> getXLabels(double min, double max, int count)const override;
public:
    TimeChart();
    TimeChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset, const std::shared_ptr<XYMultipleSeriesRenderer>& renderer);
    std::string getDateFormat() const;
    void setDateFormat(const std::string& format);
    std::string getChartType() const override;
};
}/*endof namespace*/
#endif/*__ACHART_TIME_CHART_H__*/
