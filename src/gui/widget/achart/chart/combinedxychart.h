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
#ifndef __COMBINED_XYCHART_H__
#define __COMBINED_XYCHART_H__
#include <core/canvas.h>
#include <widget/achart/chart/xychart.h>
namespace cdroid{
/**
 * The combined XY chart rendering class.
 */
class CombinedXYChart :public XYChart {
private:
    std::vector<XYChart*> mCharts;
    XYChart* getXYChart(const std::string& type)const;
    std::shared_ptr<XYMultipleSeriesDataset> createSubDataset(int seriesIndex) const;
    std::shared_ptr<XYMultipleSeriesRenderer> createSubRenderer(int seriesIndex) const;
    void configureSubChart(XYChart* chart, int seriesIndex) const;
    void configureTargetRangeChart(XYChart* chart, int seriesIndex) const;
    void syncSubChartState(XYChart* chart, int seriesIndex) const;
protected:
    std::vector<ClickableArea> clickableAreasForPoints(const std::vector<float>& points,const std::vector<double>& values,
            float yAxisValue, int seriesIndex, int startIndex)override;

    void drawSeries(const std::shared_ptr<XYSeries>& series, Canvas& canvas,  Paint& paint,std::vector<float>& pointsList,
            const std::shared_ptr<XYSeriesRenderer>& seriesRenderer, float yAxisValue, int seriesIndex, int orientation,int startIndex)override;
public:
    CombinedXYChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset, 
            const std::shared_ptr<XYMultipleSeriesRenderer>& renderer,
            const std::vector<std::string>& types);
    ~CombinedXYChart()override;
    void drawSeries(Canvas& canvas,  Paint& paint,std::vector<float>& points,
            const std::shared_ptr<XYSeriesRenderer>& seriesRenderer,
            float yAxisValue, int seriesIndex, int startIndex)override;
    bool getSeriesAndPointForScreenCoordinate(const PointF& screenPoint,SeriesSelection&) const override;
    int getLegendShapeWidth(int seriesIndex) const override;

    void drawLegendShape(Canvas& canvas, const std::shared_ptr<SimpleSeriesRenderer>& renderer,
            float x, float y,int seriesIndex,  Paint& paint)override;

    std::string getChartType() const override;
    std::vector<XYChart*> getCharts() const;
    void setSelection(int,int)override;
};
}/*endof namespace*/
#endif/*__COMBINED_XYCHART_H__*/
