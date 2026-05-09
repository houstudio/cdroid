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
#include <widget/achart/chart/bubblechart.h>
#include <widget/achart/model/xyvalueseries.h>
#include <widget/achart/renderer/xyseriesrenderer.h>
namespace cdroid{
/**
 * The bubble chart rendering class.
 */
//public static final String TYPE = "Bubble";

BubbleChart::BubbleChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
        const std::shared_ptr<XYMultipleSeriesRenderer>& renderer)
    :XYChart(dataset, renderer){
}

void BubbleChart::drawSeries(Canvas& canvas,  Paint& paint,std::vector<float>& points,
        const std::shared_ptr<XYSeriesRenderer>& renderer, float yAxisValue, int seriesIndex, int startIndex) {
    canvas.set_color(renderer->getColor());
    paint.setStyle(Style::FILL);
    const int length = points.size();
    auto series = std::dynamic_pointer_cast<XYValueSeries>(mDataset->getSeriesAt(seriesIndex));
    const double max = series->getMaxValue();
    const double coef = MAX_BUBBLE_SIZE / max;
    for (int i = 0; i < length; i += 2) {
        double size = series->getValue(startIndex + i / 2) * coef + MIN_BUBBLE_SIZE;
        drawCircle(canvas, paint, points.at(i), points.at(i + 1), (float) size);
    }
}

std::vector<ClickableArea> BubbleChart::clickableAreasForPoints(const std::vector<float>& points,
        const std::vector<double>& values, float yAxisValue, int seriesIndex, int startIndex) {
    int length = points.size();
    auto series = (XYValueSeries*)(mDataset->getSeriesAt(seriesIndex).get());
    double max = series->getMaxValue();
    double coef = MAX_BUBBLE_SIZE / max;
    std::vector<ClickableArea> ret(length / 2);
    for (int i = 0; i < length; i += 2) {
        const float size = series->getValue(startIndex + i / 2) * coef + MIN_BUBBLE_SIZE;
        ret[i / 2] = ClickableArea({points.at(i) - (float) size, points.at(i + 1) - (float) size,
                float(size)*2.f, float(size)*2.f}, values.at(i), values.at(i + 1));
    }
    return ret;
}

int BubbleChart::getLegendShapeWidth(int seriesIndex) const{
    return SHAPE_WIDTH;
}

void BubbleChart::drawLegendShape(Canvas& canvas, const std::shared_ptr<SimpleSeriesRenderer>& renderer,
        float x, float y,int seriesIndex,  Paint& paint) {
    paint.setStyle(Style::FILL);
    canvas.set_color(renderer->getColor());
    drawCircle(canvas, paint, x + SHAPE_WIDTH, y, 3);
}

void BubbleChart::drawCircle(Canvas& canvas,  Paint& paint, float x, float y, float radius) {
    //canvas.drawCircle(x, y, radius, paint);
     canvas.arc(x,y,radius,0,M_PI*2.0);
     if(paint.style==Style::FILL)canvas.fill();canvas.stroke();
}

std::string BubbleChart::getChartType() const{
    return "Bubble"/*TYPE*/;
}
}/*endof namespace*/
