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
#ifndef __LINEA_CHART_H__
#define __LINEA_CHART_H__
#include <widget/achart/chart/xychart.h>
namespace cdroid {
class ScatterChart;
/**
 * The line chart rendering class.
 */
class LineChart :public XYChart {
private:
    /** The legend shape width. */
    static constexpr int SHAPE_WIDTH = 30;
    /** The scatter chart to be used to draw the data points. */
    ScatterChart* mPointsChart;
protected:
    LineChart()=default;
    void setDatasetRenderer(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,const std::shared_ptr<XYMultipleSeriesRenderer>& renderer)override;
    std::vector<ClickableArea> clickableAreasForPoints(const std::vector<float>& points,const std::vector<double>& values,
            float yAxisValue, int seriesIndex, int startIndex)override;
public:
    LineChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
            const std::shared_ptr<XYMultipleSeriesRenderer>& renderer);
    ~LineChart()override;
    void drawSeries(Canvas& canvas,  Paint& paint,std::vector<float>& points,
            const std::shared_ptr<XYSeriesRenderer>& seriesRenderer, float yAxisValue, int seriesIndex, int startIndex)override;

    int getLegendShapeWidth(int seriesIndex) const override;

    void drawLegendShape(Canvas& canvas, const std::shared_ptr<SimpleSeriesRenderer>& renderer,
            float x, float y,int seriesIndex,  Paint& paint)override;

    /**
     * Returns if the chart should display the points as a certain shape.
     *
     * @param renderer the series renderer
     */
    bool isRenderPoints(const std::shared_ptr<SimpleSeriesRenderer>& renderer) const override;
    bool getSeriesAndPointForScreenCoordinate(const PointF& screenPoint,SeriesSelection&selection) const override;
    /**
     * Returns the scatter chart to be used for drawing the data points.
     *
     * @return the data points scatter chart
     */
    ScatterChart* getPointsChart() const;

    std::string getChartType() const override;
};
}/*endof namespace*/
#endif/*__LINEA_CHART_H__*/
