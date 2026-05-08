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
#include <widget/achart/chart/targetrangechart.h>
namespace cdroid{
/** The constant to identify this chart type. */

TargetRangeChart::TargetRangeChart() {
}

void TargetRangeChart::drawSeries(Canvas& canvas, Paint& paint,std::vector<float>& points,
        const std::shared_ptr<XYSeriesRenderer>& seriesRenderer, float yAxisValue, int seriesIndex, int startIndex) {

    // Don't draw anything, if values haven't been initialized
    if (!mInitialized) {
        return;
    }
    const Rect screen = getScreenR();
    if (screen.empty() || seriesRenderer == nullptr) {
        return;
    }

    const float topY = static_cast<float>(toScreenPoint({0.0, mMax})[1]);
    const float bottomY = static_cast<float>(toScreenPoint({0.0, mMin})[1]);
    const float bandTop = std::min(topY, bottomY);
    const float bandBottom = std::max(topY, bottomY);
    const int overlayColor = Color::toArgb(Color::red(seriesRenderer->getColor()),
                                           Color::green(seriesRenderer->getColor()),
                                           Color::blue(seriesRenderer->getColor()),
                                           std::max<uint8_t>(Color::alpha(seriesRenderer->getColor()) / 3, 0x40));

    canvas.set_color(overlayColor);
    const int bandHeight = std::max(1, static_cast<int>(std::round(bandBottom - bandTop)));
    canvas.rectangle(screen.left, std::round(bandTop), screen.width, bandHeight);
    canvas.fill();

    if (!std::isnan(mTarget)) {
        const float targetY = static_cast<float>(toScreenPoint({0.0, mTarget})[1]);
        const double originalLineWidth = canvas.get_line_width();
        canvas.set_color(Color::BLACK);
        canvas.set_line_width(2.0);
        canvas.move_to(screen.left, targetY);
        canvas.line_to(screen.right(), targetY);
        canvas.stroke();
        canvas.set_line_width(originalLineWidth);
    }
    /*Paint overlayPaint = new Paint();
    overlayPaint.setColor(seriesRenderer->getColor());
    overlayPaint.setStyle(Style.FILL);

    // TODO color and stroke width should be configurable
    Paint linePaint = new Paint();
    linePaint.setColor(Color.BLACK);
    linePaint.setStrokeWidth(2);

    canvas.drawRect(0, (float) toScreenPoint(new double[] {0, mMax})[1], canvas.getWidth(),
                    (float) toScreenPoint(new double[] {0, mMin})[1], overlayPaint);

    if (!std::isnan(mTarget)) {
        canvas.drawLine(0, (float) toScreenPoint(new double[] {0, mTarget})[1], canvas.getWidth(),
                        (float) toScreenPoint(new double[] {0, mTarget})[1], linePaint);
    }*/
}

void TargetRangeChart::setValues(float min, float max) {
    setValues(min, max, 0.f/0.f/*Float.NaN*/);
}

void TargetRangeChart::setValues(float min, float max, float target) {
    mMin = min;
    mMax = max;
    mTarget = target;
    mInitialized = true;
}

bool TargetRangeChart::isRenderNullValues() const {
    return true;
}

double TargetRangeChart::getDefaultMinimum() const {
    return 0;
}

std::string TargetRangeChart::getChartType() const{
    return TYPE;
}

std::vector<ClickableArea> TargetRangeChart::clickableAreasForPoints(const std::vector<float>& points,
        const std::vector<double>& values,float yAxisValue, int seriesIndex, int startIndex) {
    return {};
}

int TargetRangeChart::getLegendShapeWidth(int seriesIndex) const{
    return 0;
}

void TargetRangeChart::drawLegendShape(Canvas& canvas, const std::shared_ptr<SimpleSeriesRenderer>& renderer,
        float x, float y,int seriesIndex, Paint& paint) {
}

}
