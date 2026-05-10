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

static int multiAlpha(int color,float alpha){
    const int a = ((color&0xFF000000)>>24)*alpha;
    return (color&0xFFFFFF)|(a<<24);
}

void DragControlChart::drawSeries(Canvas& canvas, Paint& paint,std::vector<float>& points,
        const std::shared_ptr<XYSeriesRenderer>& seriesRenderer, float yAxisValue, int seriesIndex, int startIndex) {

    if (mContext == nullptr) {
        //throw std::runtime_error("Context has to be injected before drawing this!");
    }

    // At least two coordinates are required
    // points comes like this: 0:x1, 1:y1, 2:x2, 3:y2, ... - that's why we check for four items
    if (points.size() < 4) { //points.size<4 means rightHandlex> mScreenR.right()
        //return;
    }
    float leftHandleX = points.at(0);
    float rightHandleX = points.size()>2?points.at(2):FLT_MAX;
    if (leftHandleX > rightHandleX) {
        std::swap(leftHandleX, rightHandleX);
    }

    // Left and right overlay
    const int handleHalfWidth =8;
    const Rect screen = getScreenR();
    paint.setColor(seriesRenderer->getColor());
    canvas.set_color(multiAlpha(seriesRenderer->getColor(),0.5));
    canvas.rectangle(0, screen.top, points.at(0),screen.height);
    if(points.size()>2){
        canvas.rectangle(points.at(2), screen.top, screen.width,screen.height);
    }
    canvas.fill();
    const float overlayBottom = static_cast<float>(std::max(0, screen.bottom()));
    const float handleTop = static_cast<float>(screen.top);
    const float handleBottom = static_cast<float>(screen.bottom());
    Rect rc = {(int)leftHandleX,screen.top,handleHalfWidth*2,screen.height};

    canvas.set_color(seriesRenderer->getColor());
    const auto drawHandle = [&canvas](const Rect&r) {
        const float centerX = r.left+r.width/2;
        const float gripCenterY = r.top+r.height/2;

        canvas.rectangle(r.left,r.top,r.width,r.height);
        canvas.fill_preserve();

        canvas.set_color(kGripColor);
        for (int offset = -1; offset <= 1; ++offset) {
            const float gripX = centerX + offset * 3.0f;
            canvas.move_to(gripX, gripCenterY - 10.0f);
            canvas.line_to(gripX, gripCenterY + 10.0f);
        }
        canvas.stroke();
    };
    drawHandle(rc);
    if(points.size()>2){
        rc.left =rightHandleX;
        canvas.set_color(seriesRenderer->getColor());
        drawHandle(rc);
    }
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
