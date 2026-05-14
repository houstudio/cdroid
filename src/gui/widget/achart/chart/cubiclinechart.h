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
#ifndef __COBUIC_LINE_CHART_H__
#define __COBUIC_LINE_CHART_H__

#include <widget/achart/chart/linechart.h>
namespace cdroid{
/**
 * The interpolated (cubic) line chart rendering class.
 */
class CubicLineChart :public LineChart {

private:
    float mFirstMultiplier;
    float mSecondMultiplier;
    void calc(const std::vector<float>& points, PointF& result, int index1, int index2,float multiplier);
protected:
    void drawPath(Canvas& canvas, const std::vector<float>& points,  Paint& paint, bool circular)override; 
public:
    CubicLineChart();
    CubicLineChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
            const std::shared_ptr<XYMultipleSeriesRenderer>& renderer,float smoothness);
    bool getSeriesAndPointForScreenCoordinate(const PointF& screenPoint, SeriesSelection& selection) const override;
    std::string getChartType() const;
};
}/*endof namespace*/
#endif/*__COBUIC_LINE_CHART_H__*/
