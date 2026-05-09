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
#ifndef __DRAG_CONTROL_CHART_H__
#define __DRAG_CONTROL_CHART_H__

#include <widget/achart/chart/xychart.h>

namespace cdroid{
/**
 * The drag control chart rendering class.
 */
class DragControlChart :public XYChart {

private:
    Context* mContext;
protected:
    std::vector<ClickableArea> clickableAreasForPoints(const std::vector<float>& points,const std::vector<double>& values,
            float yAxisValue, int seriesIndex, int startIndex)override;

    bool isRenderNullValues() const override;

    DragControlChart();
public:
    DragControlChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
            const std::shared_ptr<XYMultipleSeriesRenderer>& renderer);
    void drawSeries(Canvas& canvas, Paint& paint,std::vector<float>& points,
         const std::shared_ptr<XYSeriesRenderer>& seriesRenderer,
         float yAxisValue, int seriesIndex, int startIndex)override;

    double getDefaultMinimum() const override;

    std::string getChartType() const override;

    int getLegendShapeWidth(int seriesIndex) const override;

    void drawLegendShape(Canvas& canvas, const std::shared_ptr<SimpleSeriesRenderer>& renderer,
            float x, float y,int seriesIndex, Paint& paint) override;

    void setContext(Context* context);
};
}/*endof namespace*/
#endif/*__DRAG_CONTROL_CHART_H__*/
