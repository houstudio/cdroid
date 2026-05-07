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
#ifndef __COMBINED_TIMED_CHART_H__
#define __COMBINED_TIMED_CHART_H__

#include <widget/achart/chart/combinedxychart.h>
namespace cdroid{
/**
 * The combined XY chart rendering class.
 */
class CombinedTimeChart :public CombinedXYChart {
    /** The number of milliseconds in a day. */
public:
    static constexpr long DAY = 24 * 60 * 60 * 1000;
private:
    /** The date format pattern to be used in formatting the X axis labels. */
    std::string mDateFormat;
    /** The starting point for labels. */
    mutable double mStartPoint;
private:
    std::string getDateFormat(double start, double end)const;
protected:
    void drawXLabels(const std::vector<double>& xLabels,const std::vector<double>& xTextLabelLocations, Canvas& canvas,
             Paint& paint, int left, int top, int bottom, double xPixelsPerUnit, double minX, double maxX) override;

    std::vector<double> getXLabels(double min, double max, int count);
public:
    CombinedTimeChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
            const std::shared_ptr<XYMultipleSeriesRenderer>& renderer,const std::vector<std::string>& types);
    /**
     * Returns the date format pattern to be used for formatting the X axis
     * labels.
     *
     * @return the date format pattern for the X axis labels
     */
    std::string getDateFormat() const;
    /**
     * Sets the date format pattern to be used for formatting the X axis labels.
     *
     * @param format the date format pattern for the X axis labels. If null, an
     *          appropriate default format will be used.
     */
    void setDateFormat(const std::string& format);
};
}/*endof namespace*/
#endif/*__COMBINED_TIMED_CHART_H__*/
