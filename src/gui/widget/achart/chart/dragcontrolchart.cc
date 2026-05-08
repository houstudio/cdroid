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
#include <widget/achart/chart/dragcontrolchart.h>
namespace cdroid{

//public static final String TYPE = "DragControl";
constexpr float kDefaultHandleHalfWidth = 8.0f;
constexpr float kMinimumHandleGap = 12.0f;
constexpr int kOverlayColor = 0x66000000;
constexpr int kHandleFillColor = 0xCCF2F2F2;
constexpr int kHandleStrokeColor = 0xFF6A6A6A;
constexpr int kGripColor = 0xFF5A5A5A;
DragControlChart::DragControlChart() {
}

DragControlChart::DragControlChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
        const std::shared_ptr<XYMultipleSeriesRenderer>& renderer)
    :XYChart(dataset, renderer){
}

void DragControlChart::drawSeries(Canvas& canvas, Paint& paint,std::vector<float>& points,
        const std::shared_ptr<XYSeriesRenderer>& seriesRenderer, float yAxisValue, int seriesIndex, int startIndex) {

    if (mContext == nullptr) {
        //throw std::runtime_error("Context has to be injected before drawing this!");
    }

    // At least two coordinates are required
    // points comes like this: 0:x1, 1:y1, 2:x2, 3:y2, ... - that's why we check for four items
    if (points.size() < 4) {
        return;
    }
    const Rect screen = getScreenR();
    float leftHandleX = points.at(0);
    float rightHandleX = points.at(2);
    if (leftHandleX > rightHandleX) {
        std::swap(leftHandleX, rightHandleX);
    }

    const float chartLeft = static_cast<float>(screen.left);
    const float chartRight = static_cast<float>(screen.right());
    leftHandleX = std::max(chartLeft, std::min(leftHandleX, chartRight));
    rightHandleX = std::max(leftHandleX + kMinimumHandleGap, std::min(rightHandleX, chartRight));
    if (rightHandleX > chartRight) {
        rightHandleX = chartRight;
        leftHandleX = std::min(leftHandleX, rightHandleX - kMinimumHandleGap);
    }

    const float overlayTop = 0.0f;
    const float overlayBottom = static_cast<float>(std::max(m_height, screen.bottom()));
    const float handleTop = static_cast<float>(screen.top);
    const float handleBottom = static_cast<float>(screen.bottom());
    const float handleHalfWidth = std::min(kDefaultHandleHalfWidth,
                                           std::max(4.0f, screen.width / 20.0f));

    canvas.set_color(kOverlayColor);
    if (leftHandleX > 0.0f) {
        canvas.rectangle(0, static_cast<int>(overlayTop), std::round(leftHandleX),
                         std::round(overlayBottom - overlayTop));
        canvas.fill();
    }
    if (rightHandleX < m_width) {
        canvas.rectangle(std::round(rightHandleX), static_cast<int>(overlayTop),
                         std::round(m_width - rightHandleX), std::round(overlayBottom - overlayTop));
        canvas.fill();
    }

    const auto drawHandle = [&](float centerX) {
        const int left = std::round(centerX - handleHalfWidth);
        const int top = std::round(handleTop);
        const int width = std::max(1, static_cast<int>(std::round(handleHalfWidth * 2.0f)));
        const int height = std::max(1, static_cast<int>(std::round(handleBottom - handleTop)));
        const float gripCenterY = handleTop + (handleBottom - handleTop) / 2.0f;

        canvas.set_color(kHandleFillColor);
        canvas.rectangle(left, top, width, height);
        canvas.fill();

        canvas.set_color(kHandleStrokeColor);
        canvas.rectangle(left, top, width, height);
        canvas.stroke();

        canvas.set_color(kGripColor);
        for (int offset = -1; offset <= 1; ++offset) {
            const float gripX = centerX + offset * 3.0f;
            canvas.move_to(gripX, gripCenterY - 10.0f);
            canvas.line_to(gripX, gripCenterY + 10.0f);
        }
        canvas.stroke();
    };

    drawHandle(leftHandleX);
    drawHandle(rightHandleX);
}

std::vector<ClickableArea> DragControlChart::clickableAreasForPoints(const std::vector<float>& points,
        const std::vector<double>& values, float yAxisValue, int seriesIndex, int startIndex) {

    return {};
}

bool DragControlChart::isRenderNullValues() const{
    return true;
}

double DragControlChart::getDefaultMinimum() const{
    return 0;
}

std::string DragControlChart::getChartType() const{
    return "DragControl"/*TYPE*/;
}

int DragControlChart::getLegendShapeWidth(int seriesIndex) const{
    return 0;
}

void DragControlChart::drawLegendShape(Canvas& canvas, const std::shared_ptr<SimpleSeriesRenderer>& renderer,
        float x, float y, int seriesIndex, Paint& paint) {
}

void DragControlChart::setContext(Context* context) {
    mContext = context;
}
}
