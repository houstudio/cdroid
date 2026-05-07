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
#ifndef __BUBBLE_CHART_H__
#define __BUBBLE_CHART_H__

#include <widget/achart/chart/xychart.h>
namespace cdroid{
/**
 * The bubble chart rendering class.
 */
class BubbleChart :public XYChart {
    /** The constant to identify this chart type. */
private:
    /** The legend shape width. */
    static constexpr int SHAPE_WIDTH = 10;
    /** The minimum bubble size. */
    static constexpr int MIN_BUBBLE_SIZE = 2;
    /** The maximum bubble size. */
    static constexpr int MAX_BUBBLE_SIZE = 20;
private:
    void drawCircle(Canvas& canvas,  Paint& paint, float x, float y, float radius);
protected:
    BubbleChart()=default;
    std::vector<ClickableArea> clickableAreasForPoints(const std::vector<float>& points,const std::vector<double>& values,
            float yAxisValue, int seriesIndex, int startIndex) override;
public:

    /**
     * Builds a new bubble chart instance.
     *
     * @param dataset the multiple series dataset
     * @param renderer the multiple series renderer
     */
    BubbleChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset, const std::shared_ptr<XYMultipleSeriesRenderer>& renderer);

    /**
     * The graphical representation of a series.
     *
     * @param canvas the canvas to paint to
     * @param paint the paint to be used for drawing
     * @param points the array of points to be used for drawing the series
     * @param seriesRenderer the series renderer
     * @param yAxisValue the minimum value of the y axis
     * @param seriesIndex the index of the series currently being drawn
     * @param startIndex the start index of the rendering points
     */
    void drawSeries(Canvas& canvas,  Paint& paint,std::vector<float>& points,
            const std::shared_ptr<XYSeriesRenderer>& seriesRenderer, float yAxisValue, int seriesIndex, int startIndex)override;

    /**
     * Returns the legend shape width.
     *
     * @param seriesIndex the series index
     * @return the legend shape width
     */
    int getLegendShapeWidth(int seriesIndex) const override;

    /**
     * The graphical representation of the legend shape.
     *
     * @param canvas the canvas to paint to
     * @param renderer the series renderer
     * @param x the x value of the point the shape should be drawn at
     * @param y the y value of the point the shape should be drawn at
     * @param seriesIndex the series index
     * @param paint the paint to be used for drawing
     */
    void drawLegendShape(Canvas& canvas, const std::shared_ptr<SimpleSeriesRenderer>& renderer, float x, float y,int seriesIndex,  Paint& paint)override;
    /**
     * Returns the chart type identifier.
     *
     * @return the chart type
     */
    std::string getChartType() const override;
};
}/*endof namespace*/
#endif/*__BUBBLE_CHART_H__*/
