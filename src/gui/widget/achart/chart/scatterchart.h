/**
 * Copyright (C) 2009 - 2012 SC 4ViewSoft SRL
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
#ifndef __SCATTER_CHART_H__
#define __SCATTER_CHART_H__

#include <widget/achart/chart/xychart.h>
namespace cdroid{
/**
 * The scatter chart rendering class.
 */
class ScatterChart :public XYChart {
private:
    /** The default point shape size. */
    static constexpr float SIZE = 5;
    /** The legend shape width. */
    static constexpr int SHAPE_WIDTH = 10;
    /** The point shape size. */
    float mSize = SIZE;
private:
    void drawX(Canvas& canvas,  Paint& paint, float x, float y);
    void drawCircle(Canvas& canvas,  Paint& paint, float x, float y);
    void drawTriangle(Canvas& canvas,  Paint& paint, std::vector<float>& path, float x, float y);
    void drawSquare(Canvas& canvas,  Paint& paint, float x, float y);
    void drawDiamond(Canvas& canvas,  Paint& paint, std::vector<float>& path, float x, float y);
protected:
    void setDatasetRenderer(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
            const std::shared_ptr<XYMultipleSeriesRenderer>& renderer)override;
    std::vector<ClickableArea> clickableAreasForPoints(const std::vector<float>& points,const std::vector<double>& values,
            float yAxisValue, int seriesIndex, int startIndex) override;
public:
    ScatterChart();
    ScatterChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
            const std::shared_ptr<XYMultipleSeriesRenderer>& renderer);

    void drawSeries(Canvas& canvas,  Paint& paint,std::vector<float>& points,
            const std::shared_ptr<XYSeriesRenderer>& seriesRenderer,
            float yAxisValue, int seriesIndex, int startIndex)override;

    int getLegendShapeWidth(int seriesIndex) const override;
    void drawLegendShape(Canvas& canvas, const std::shared_ptr<SimpleSeriesRenderer>& renderer,
            float x, float y,int seriesIndex,  Paint& paint)override;

    std::string getChartType() const override;
};
}/*endof namespace*/
#endif/*__SCATTER_CHART_H__*/
