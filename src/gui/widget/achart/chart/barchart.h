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
#ifndef __BAR_CHART_H__
#define __BAR_CHART_H__
#include <widget/achart/chart/xychart.h>

namespace cdroid{
/**
 * The bar chart rendering class.
 */
class BarChart :public XYChart {
public:
    enum Type {
        DEFAULT, STACKED,HEAPED
    };
private:
    /** The legend shape width. */
    static constexpr int SHAPE_WIDTH = 12;
    /** The chart type. */
    std::vector<float>mPreviousSeriesPoints;
protected:
    Type mType = Type::DEFAULT;

    BarChart() =default;

    BarChart(Type type);
private:
    void drawBar(Canvas& canvas, float xMin, float yMin, float xMax, float yMax, int scale,int seriesIndex,  Paint& paint);

    int getGradientPartialColor(int minColor, int maxColor, float fraction)const;
protected:
    std::vector<ClickableArea> clickableAreasForPoints(const std::vector<float>& points,const std::vector<double>& values,
            float yAxisValue, int seriesIndex, int startIndex)override;

    void drawBar(Canvas& canvas, float xMin, float yMin, float xMax, float yMax, float halfDiffX, int seriesNr, int seriesIndex,  Paint& paint);

    void drawChartValuesText(Canvas& canvas,  const std::shared_ptr<XYSeries>& series, const std::shared_ptr<XYSeriesRenderer>& renderer,
             Paint& paint,const std::vector<float>& points, int seriesIndex, int startIndex)override;

    float getHalfDiffX(const std::vector<float>& points, int length, int seriesNr)const;
    float getCoeficient() const;
    bool isRenderNullValues() const;
public:
    BarChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset, const std::shared_ptr<XYMultipleSeriesRenderer>& renderer, Type type);
    void drawSeries(Canvas& canvas,  Paint& paint,std::vector<float>& points,
         const std::shared_ptr<XYSeriesRenderer>& seriesRenderer, float yAxisValue, int seriesIndex, int startIndex) override;
    int getLegendShapeWidth(int seriesIndex) const override;

    void drawLegendShape(Canvas& canvas, const std::shared_ptr<SimpleSeriesRenderer>& renderer, float x, float y,int seriesIndex,  Paint& paint)override;
    
    double getDefaultMinimum() const override;

    std::string getChartType() const override;
};
}/*endof namespace*/
#endif/*__BAR_CHART_H__*/
