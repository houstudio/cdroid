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

bool CubicLineChart::getSeriesAndPointForScreenCoordinate(const PointF& screenPoint, SeriesSelection& selection) const {
    if (XYChart::getSeriesAndPointForScreenCoordinate(screenPoint, selection)) {
        return true;
    }

    const float selectableBuffer = mRenderer->getSelectableBuffer();

    for (int seriesIndex = mClickableAreas.size() - 1; seriesIndex >= 0; seriesIndex--) {
        auto it = mClickableAreas.find(seriesIndex);
        if (it == mClickableAreas.end()) continue;

        auto& areas = it->second;
        auto series = mDataset->getSeriesAt(seriesIndex);
        const auto scale = series->getScaleNumber();

        std::vector<PointF> controlPoints; // 获取所有控制点 (P0, C0, C1, P1, P1, C2, C3, P2, ...)

        for(auto& a:areas){
            const auto v = toScreenPoint({a.getX(),a.getY()},scale);
            controlPoints.push_back({float(v[0]),float(v[1])});
        }

        // 遍历每一段贝塞尔曲线 (每4个控制点一组: P_start, C1, C2, P_end)
        for (size_t bezier_idx = 0; bezier_idx + 3 < controlPoints.size(); bezier_idx += 3) { // 步长为3，因为 P_start -> C1 -> C2 -> P_end
             // 注意：实际 CubicLineChart 的控制点布局可能不同，比如是 P0, P1, ..., PN 然后内部计算 C0, C1, ...
             // 下面的伪代码假定是 P, C1, C2, P, C3, C4, P... 的模式
             const PointF& P0 = controlPoints[bezier_idx];     // 曲线起点
             const PointF& C1 = controlPoints[bezier_idx + 1]; // 第一个控制手柄
             const PointF& C2 = controlPoints[bezier_idx + 2]; // 第二个控制手柄
             const PointF& P1 = controlPoints[bezier_idx + 3]; // 曲线终点

             // --- 采样曲线 ---
             const int samples = 20;
             std::vector<PointF> samplePoints;
             for (int s = 0; s <= samples; ++s) {
                 const double t = static_cast<double>(s) / samples;
                 // 立方贝塞尔曲线公式: B(t) = (1-t)^3*P0 + 3*(1-t)^2*t*C1 + 3*(1-t)*t^2*C2 + t^3*P1
                 const double x = std::pow(1 - t, 3) * P0.x + 3 * std::pow(1 - t, 2) * t * C1.x +
                            3 * (1 - t) * std::pow(t, 2) * C2.x + std::pow(t, 3) * P1.x;

                 const double y = std::pow(1 - t, 3) * P0.y + 3 * std::pow(1 - t, 2) * t * C1.y +
                            3 * (1 - t) * std::pow(t, 2) * C2.y + std::pow(t, 3) * P1.y;

                 samplePoints.push_back({float(x), float(y)});
             }

             for (size_t j = 0; j < samplePoints.size() - 1; ++j) {
                 const PointF& sp1 = samplePoints[j];
                 const PointF& sp2 = samplePoints[j+1];
                 auto distance = pointToSegmentDistance(screenPoint, sp1, sp2);
                 if (distance < selectableBuffer) {
                     selection = SeriesSelection(seriesIndex, -1, sp1.x, sp1.y);
                     return true;
                 }
             }
        }
    }
    return false;
}
std::string CubicLineChart::getChartType() const{
    return "Cubic"/*TYPE*/;
}
}
