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
#include <widget/achart/chart/rangebarchart.h>
namespace cdroid{

//public static final String TYPE = "RangeBar";

RangeBarChart::RangeBarChart() {
}

RangeBarChart::RangeBarChart(Type type):BarChart(type){
}

RangeBarChart::RangeBarChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
        const std::shared_ptr<XYMultipleSeriesRenderer>& renderer, Type type)
    :BarChart(dataset, renderer, type){
}

std::vector<ClickableArea> RangeBarChart::clickableAreasForPoints(const std::vector<float>& points,const std::vector<double>& values,
            float yAxisValue, int seriesIndex, int startIndex){
    const int seriesNr = mDataset->getSeriesCount();
    const int length = points.size();
    std::vector<ClickableArea> ret(length / 4);
    const float halfDiffX = getHalfDiffX(points, length, seriesNr);
    for (int i = 0; i <= length-4; i += 4) {
        const float x1 = points.at(i);
        const float y1 = points.at(i + 1);
        const float x2 = points.at(i + 2);
        const float y2 = points.at(i + 3);
        const float minY = std::min(x1, x2);
        const float maxY = std::max(y1, y2);
        if (mType == Type::STACKED||mType == Type::HEAPED) {
            ret[i / 4] = ClickableArea({x1 - halfDiffX, minY, halfDiffX*2.f, maxY-minY},
                    values.at(i), values.at(i + 1));
        } else {
            const float startX = x1 - seriesNr * halfDiffX + seriesIndex * 2 * halfDiffX;
            ret[i / 4] = ClickableArea({startX, minY, 2 * halfDiffX, maxY-minY},
                    values.at(i), values.at(i + 1));
        }
    }
    return ret;
}

void RangeBarChart::drawSeries(Canvas& canvas,  Paint& paint,std::vector<float>& points,
        const std::shared_ptr<XYSeriesRenderer>& seriesRenderer, float yAxisValue, int seriesIndex, int startIndex) {
    const int seriesNr = mDataset->getSeriesCount();
    const int length = points.size();
    paint.setColor(seriesRenderer->getColor());
    canvas.set_color(seriesRenderer->getColor());
    paint.setStyle(Style::FILL);
    const float halfDiffX = getHalfDiffX(points, length, seriesNr);
    int start = 0;
    if (startIndex > 0) {
        start = 2;
    }
    for (int i = start; i < length; i += 4) {
        if (points.size() > i + 3) {
            const float xMin = points.at(i);
            const float yMin = points.at(i + 1);
            // xMin = xMax
            const float xMax = points.at(i + 2);
            const float yMax = points.at(i + 3);
            drawBar(canvas, xMin, yMin, xMax, yMax, halfDiffX, seriesNr, seriesIndex,i/4, paint);
        }
    }
    paint.setColor(seriesRenderer->getColor());
}

void RangeBarChart::drawChartValuesText(Canvas& canvas, const std::shared_ptr<XYSeries>& series,
        const std::shared_ptr<XYSeriesRenderer>& renderer, Paint& paint,
        const std::vector<float>& points, int seriesIndex, int startIndex) {
    const int seriesNr = mDataset->getSeriesCount();
    const float halfDiffX = getHalfDiffX(points, points.size(), seriesNr);
    int start = 0;
    if (startIndex > 0) {
        start = 2;
    }
    for (int i = start; i < points.size(); i += 4) {
        const int index = startIndex + i / 2;
        float x = points.at(i);
        if (mType == Type::DEFAULT) {
            x += seriesIndex * 2 * halfDiffX - (seriesNr - 1.5f) * halfDiffX;
        }

        if (!isNullValue(series->getY(index + 1)) && points.size() > i + 3) {
            // draw the maximum value
            drawText(canvas, getLabel(renderer->getChartValuesFormat(), series->getY(index + 1)), x,
                     points.at(i + 3) - renderer->getChartValuesSpacing(), paint, 0);
        }
        if (!isNullValue(series->getY(index)) && points.size() > i + 1) {
            // draw the minimum value
            drawText(canvas,
                getLabel(renderer->getChartValuesFormat(), series->getY(index)),
                x,
                points.at(i + 1) + renderer->getChartValuesTextSize()
                + renderer->getChartValuesSpacing() - 3, paint, 0);
        }
    }
}

float RangeBarChart::getCoeficient() const{
    return 0.5f;
}

std::string RangeBarChart::getChartType() const{
    return "RangeBar"/*TYPE*/;
}
}
