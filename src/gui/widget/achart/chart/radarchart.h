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
#ifndef __RADAR_CHART_H__
#define __RADAR_CHART_H__

#include <widget/achart/chart/doughnutchart.h>

namespace cdroid{
/**
 * The radar chart rendering class.
 */
class RadarChart : public DoughnutChart {
protected:
public:
    RadarChart(const std::shared_ptr<MultipleCategorySeries>& dataset, const std::shared_ptr<DefaultRenderer>& renderer);

    void draw(Canvas& canvas, int x, int y, int width, int height, Paint& paint) override;

    int getLegendShapeWidth(int seriesIndex) const override;

    void drawLegendShape(Canvas& canvas,const std::shared_ptr<SimpleSeriesRenderer>& renderer, float x, float y,
            int seriesIndex, Paint& paint) override;
};
}/*endof namespace*/

#endif/*__RADAR_CHART_H__*/
