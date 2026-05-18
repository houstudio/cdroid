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
#ifndef __PIE_CHART_H__
#define __PIE_CHART_H__
#include <widget/achart/chart/piemapper.h>
#include <widget/achart/chart/roundchart.h>
namespace cdroid{
class PieMapper;
/**
 * The pie chart rendering class.
 */
class PieChart :public RoundChart {
private:
    void drawArc(Canvas& canvas,double centerX, double centerY,
            double radius, double startAngle, double sweepAngle,int paintStyle);
public:
    /**
     * Builds a new pie chart instance.
     *
     * @param dataset the series dataset
     * @param renderer the series renderer
     */
    PieChart(const std::shared_ptr<CategorySeries>& dataset, const std::shared_ptr<DefaultRenderer>& renderer);
    /**
     * The graphical representation of the pie chart.
     *
     * @param canvas the canvas to paint to
     * @param x the top left x value of the view to draw to
     * @param y the top left y value of the view to draw to
     * @param width the width of the view to draw to
     * @param height the height of the view to draw to
     * @param paint the paint
     */
    void draw(Canvas& canvas, int x, int y, int width, int height,  Paint& paint)override;

    bool getSeriesAndPointForScreenCoordinate(const PointF& screenPoint,SeriesSelection&)const override;
};
}/*endof namespace*/
#endif/*__PIE_CHART_H__*/
