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
#include <widget/achart/chart/cubiclinechart.h>
namespace cdroid{
/** The chart type. */
//public static final String TYPE = "Cubic";

CubicLineChart::CubicLineChart() {
    // default is to have first control point at about 33% of the distance,
    mFirstMultiplier = 0.33f;
    // and the next at 66% of the distance.
    mSecondMultiplier = 1 - mFirstMultiplier;
}

CubicLineChart::CubicLineChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
        const std::shared_ptr<XYMultipleSeriesRenderer>& renderer,float smoothness)
    :LineChart(dataset, renderer){
    mFirstMultiplier = smoothness;
    mSecondMultiplier = 1.f - mFirstMultiplier;
}

void CubicLineChart::drawPath(Canvas& canvas, const std::vector<float>& points,  Paint& paint, bool circular) {
    //Path p = new Path();
    float x = points.at(0);
    float y = points.at(1);
    canvas.begin_new_path();
    canvas.move_to(x, y);

    int length = points.size();
    if (circular) {
        length -= 4;
    }
    PointF p1,p2,p3;
    for (int i = 0; i < length; i += 2) {
        int nextIndex = i + 2 < length ? i + 2 : i;
        int nextNextIndex = i + 4 < length ? i + 4 : nextIndex;
        calc(points, p1, i, nextIndex, mSecondMultiplier);
        p2.x = points.at(nextIndex);
        p2.y = points.at(nextIndex + 1);
        calc(points, p3, nextIndex, nextNextIndex, mFirstMultiplier);
        // From last point, approaching x1/y1 and x2/y2 and ends up at x3/y3
        canvas.curve_to/*p.cubicTo*/(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
    }
    if (circular) {
        for (int i = length; i < length + 4; i += 2) {
            canvas.line_to(points.at(i), points.at(i + 1));
        }
        canvas.line_to(points.at(0), points.at(1));
    }
    canvas.close_path();
    if(paint.style==Style::FILL)
        canvas.fill();
    else
        canvas.stroke();
    //canvas.drawPath(p, paint);
}

void CubicLineChart::calc(const std::vector<float>& points, PointF& result, int index1, int index2, float multiplier) {
    float p1x = points.at(index1);
    float p1y = points.at(index1 + 1);
    float p2x = points.at(index2);
    float p2y = points.at(index2 + 1);

    float diffX = p2x - p1x; // p2.x - p1.x;
    float diffY = p2y - p1y; // p2.y - p1.y;
    result.x = (p1x + (diffX * multiplier));
    result.y = (p1y + (diffY * multiplier));
}

std::string CubicLineChart::getChartType() const{
    return "Cubic"/*TYPE*/;
}
}
