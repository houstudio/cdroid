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
#ifndef __TARGET_RANGE_CHART_H__
#define __TARGET_RANGE_CHART_H__
#include <widget/achart/chart/xychart.h>

namespace cdroid{
/**
 * The target range chart rendering class.
 */
class TargetRangeChart :public XYChart {
public:
    static constexpr const char*const TYPE="TargetRange";
private:
    float mMin = FLT_MAX;//Float.MAX_VALUE;
    float mMax = FLT_MIN;//Float.MIN_VALUE;
    float mTarget = 0.f/0.f;//Float.NaN;
    bool mInitialized = false;
protected:
    bool isRenderNullValues() const override;
    std::vector<ClickableArea> clickableAreasForPoints(const std::vector<float>& points,const std::vector<double>& values,
            float yAxisValue, int seriesIndex, int startIndex) override;
public:
    TargetRangeChart();

    void drawSeries(Canvas& canvas,  Paint& paint,std::vector<float>& points,
            const std::shared_ptr<XYSeriesRenderer>& seriesRenderer,
            float yAxisValue, int seriesIndex, int startIndex)override;
    void setValues(float min, float max);
    void setValues(float min, float max, float target);

    double getDefaultMinimum() const override;
    std::string getChartType() const override;

    int getLegendShapeWidth(int seriesIndex) const override;
    void drawLegendShape(Canvas& canvas, const std::shared_ptr<SimpleSeriesRenderer>& renderer, float x, float y, int seriesIndex,  Paint& paint) override;
};
}/*endof namespace*/
#endif/*__TARGET_RANGE_CHART_H__*/
