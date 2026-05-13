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
#ifndef __RANGE_BAR_CHART_H__
#define __RANGE_BAR_CHART_H__

#include <widget/achart/chart/barchart.h>
namespace cdroid{
/**
 * The range bar chart rendering class.
 */
class RangeBarChart :public BarChart {
protected:
    std::vector<ClickableArea> clickableAreasForPoints(const std::vector<float>& points,const std::vector<double>& values,
            float yAxisValue, int seriesIndex, int startIndex)override;
    void drawChartValuesText(Canvas& canvas, const std::shared_ptr<XYSeries>& series, const std::shared_ptr<XYSeriesRenderer>& renderer,
             Paint& paint,const std::vector<float>& points, int seriesIndex, int startIndex)override;
    float getCoeficient() const;
public:
    RangeBarChart();
    RangeBarChart(Type type);

    RangeBarChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
            const std::shared_ptr<XYMultipleSeriesRenderer>& renderer, Type type);

    void drawSeries(Canvas& canvas,  Paint& paint,std::vector<float>& points,
            const std::shared_ptr<XYSeriesRenderer>& seriesRenderer,
            float yAxisValue, int seriesIndex, int startIndex)override;

    std::string getChartType() const override;
};
}/*endof namespace */
#endif/*__RANGE_BAR_CHART_H__*/
