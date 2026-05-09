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
#ifndef __ROUND_CHART_H__
#define __ROUND_CHART_H__
#include <widget/achart/renderer/defaultrenderer.h>
#include <widget/achart/renderer/simpleseriesrenderer.h>
#include <widget/achart/chart/abstractchart.h>
#include <widget/achart/model/categoryseries.h>
namespace cdroid{
/**
 * An abstract class to be extended by round like chart rendering classes.
 */
class RoundChart :public AbstractChart {
protected:
    /** The legend shape width. */
    static constexpr int SHAPE_WIDTH = 10;
    /** A no value constant. */
    static constexpr int NO_VALUE = INT_MAX;
    /** The series dataset. */
     std::shared_ptr<CategorySeries> mDataset;
    /** The series renderer. */
    std::shared_ptr<DefaultRenderer> mRenderer;
    /** The chart center X axis. */
    int mCenterX = NO_VALUE;
    /** The chart center y axis. */
    int mCenterY = NO_VALUE;
public:
    RoundChart(const std::shared_ptr<CategorySeries>& dataset, const std::shared_ptr<DefaultRenderer>& renderer);
    void drawTitle(Canvas& canvas, int x, int y, int width,  Paint& paint);

    int getLegendShapeWidth(int seriesIndex) const;
    void drawLegendShape(Canvas& canvas, const std::shared_ptr<SimpleSeriesRenderer>& renderer, float x, float y,int seriesIndex,  Paint& paint)override;

    const std::shared_ptr<DefaultRenderer>& getRenderer() const;

    int getCenterX() const;

    int getCenterY() const;

    void setCenterX(int centerX);
    void setCenterY(int centerY);
};
}/*endof namespace*/
#endif/*__ROUND_CHART_H__*/
