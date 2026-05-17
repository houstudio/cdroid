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
#ifndef __DOUGHNUT_CHART_H__
#define __DOUGHNUT_CHART_H__

#include <widget/achart/chart/piemapper.h>
#include <widget/achart/chart/roundchart.h>
#include <widget/achart/model/multiplecategoryseries.h>
namespace cdroid{
/**
 * The doughnut chart rendering class.
 */
class DoughnutChart :public RoundChart {
protected:
    /** The series dataset. */
    std::shared_ptr<MultipleCategorySeries> mDataset;
    /** A step variable to control the size of the legend shape. */
    int mStep;
    int getSeriesSelectionColor(int series)const override;
private:
    void drawRingSlice(Canvas& canvas, double cx, double cy, double outerRadius,
            double innerRadius, double startAngleDegrees, double sweepAngleDegrees,int style); 
public:
    /**
     * Builds a new doughnut chart instance.
     *
     * @param dataset the series dataset
     * @param renderer the series renderer
     */
    DoughnutChart(const std::shared_ptr<MultipleCategorySeries>& dataset, const std::shared_ptr<DefaultRenderer>& renderer);
    /**
     * The graphical representation of the doughnut chart.
     *
     * @param canvas the canvas to paint to
     * @param x the top left x value of the view to draw to
     * @param y the top left y value of the view to draw to
     * @param width the width of the view to draw to
     * @param height the height of the view to draw to
     * @param paint the paint
     */
    void draw(Canvas& canvas, int x, int y, int width, int height,  Paint& paint)override;

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

    bool getSeriesAndPointForScreenCoordinate(const PointF& screenPoint,SeriesSelection&) const override;
};
}/*endof namespace*/
#endif/*__DOUGHNUT_CHART_H__*/
