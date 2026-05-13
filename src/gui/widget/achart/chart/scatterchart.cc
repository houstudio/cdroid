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
#include <widget/achart/chart/scatterchart.h>
#include <widget/achart/renderer/xyseriesrenderer.h>
#include <widget/achart/renderer/simpleseriesrenderer.h>
namespace cdroid{

ScatterChart::ScatterChart() {
}

ScatterChart::ScatterChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset, const std::shared_ptr<XYMultipleSeriesRenderer>& renderer)
    :XYChart(dataset, renderer){
    mSize = renderer->getPointSize();
}

void ScatterChart::setDatasetRenderer(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
        const std::shared_ptr<XYMultipleSeriesRenderer>& renderer){
    XYChart::setDatasetRenderer(dataset, renderer);
    mSize = renderer->getPointSize();
}

void ScatterChart::drawSeries(Canvas& canvas,  Paint& paint,std::vector<float>& points,
        const std::shared_ptr<XYSeriesRenderer>& seriesRenderer, float yAxisValue, int seriesIndex, int startIndex) {
    auto renderer = seriesRenderer;
    mSize = renderer->getPointSize();
    canvas.save();
    canvas.set_color(renderer->getColor());
    std::vector<float>pathPTS;
    if(!renderer->isFillPoints()){
        paint.setStyle(Style::FILL);
    }else{
        paint.setStyle(Style::STROKE);
        canvas.set_line_width(renderer->getPointStrokeWidth());
    }
    const int length = points.size();
    switch (renderer->getPointStyle()) {
    case X:
        canvas.set_line_width(renderer->getPointStrokeWidth());
        for (int i = 0; i < length; i += 2) {
            drawX(canvas, paint, points.at(i), points.at(i + 1));
        }
        break;
    case PointStyle::CIRCLE:
        for (int i = 0; i < length; i += 2) {
            drawCircle(canvas, paint, points.at(i), points.at(i + 1));
        }
        break;
    case PointStyle::CIRCLE_FILLED:
        for (int i = 0; i < length; i += 2) {
            Paint fill;
            fill.setColor(Color::WHITE); // TODO
            fill.setStyle(Style::FILL);
            drawCircle(canvas, fill, points.at(i), points.at(i + 1));
            drawCircle(canvas, paint, points.at(i), points.at(i + 1));
        }
        break;
    case PointStyle::TRIANGLE:
        pathPTS.resize(6);
        for (int i = 0; i < length; i += 2) {
            drawTriangle(canvas, paint, pathPTS, points.at(i), points.at(i + 1));
        }
        break;
    case PointStyle::SQUARE:
        for (int i = 0; i < length; i += 2) {
            drawSquare(canvas, paint, points.at(i), points.at(i + 1));
        }
        break;
    case PointStyle::DIAMOND:
        pathPTS.resize(8);
        for (int i = 0; i < length; i += 2) {
            drawDiamond(canvas, paint, pathPTS, points.at(i), points.at(i + 1));
        }
        break;
    case PointStyle::POINT:
        for (int i = 0; i < length; i += 2) {
            LOGD("%.2f,%.2f",points.at(i), points.at(i + 1));
            //canvas.drawPoint(points.at(i), points.at(i + 1), paint);
            canvas.arc(points.at(i), points.at(i + 1),mSize,0,M_PI*2.0);
            canvas.fill();
        }
        break;
    }
    if (renderer->isFillPoints()){
        canvas.fill();
    }else {
        canvas.stroke();
    }
    canvas.restore();
}

std::vector<ClickableArea> ScatterChart::clickableAreasForPoints(const std::vector<float>& points,const std::vector<double>& values,
        float yAxisValue, int seriesIndex, int startIndex) {
    int length = points.size();
    std::vector<ClickableArea> ret(length / 2);
    for (int i = 0; i < length; i += 2) {
        int selectableBuffer = mRenderer->getSelectableBuffer();
        ret[i / 2] = ClickableArea({points.at(i) - selectableBuffer, points.at(i + 1) - selectableBuffer,
                selectableBuffer*2.f, selectableBuffer*2.f}, values.at(i), values.at(i + 1));
    }
    return ret;
}

int ScatterChart::getLegendShapeWidth(int seriesIndex) const{
    return SHAPE_WIDTH;
}

void ScatterChart::drawLegendShape(Canvas& canvas, const std::shared_ptr<SimpleSeriesRenderer>& renderer,
        float x, float y,int seriesIndex,  Paint& paint) {
    std::vector<float>pathPTS;
     if (((XYSeriesRenderer*) renderer.get())->isFillPoints()) {
        paint.setStyle(Style::FILL);
    } else {
        paint.setStyle(Style::STROKE);
    }
    switch (((XYSeriesRenderer*) renderer.get())->getPointStyle()) {
    case PointStyle::X:
        drawX(canvas, paint, x + SHAPE_WIDTH, y);
        break;
    case PointStyle::CIRCLE:
        drawCircle(canvas, paint, x + SHAPE_WIDTH, y);
        break;
    case PointStyle::TRIANGLE:/*bug*/
        pathPTS.resize(6);
        drawTriangle(canvas, paint, pathPTS, x + SHAPE_WIDTH, y);
        break;
    case PointStyle::SQUARE:
        drawSquare(canvas, paint, x + SHAPE_WIDTH, y);
        break;
    case PointStyle::DIAMOND:/*bug*/
        pathPTS.resize(8);
        drawDiamond(canvas, paint, pathPTS, x + SHAPE_WIDTH, y);
        break;
    case PointStyle::POINT:
        //canvas.drawPoint(x + SHAPE_WIDTH, y, paint);
        canvas.arc(x+SHAPE_WIDTH,y,mSize,0,M_PI*2.0);
        break;
    }
    if(paint.style==Style::FILL){
        canvas.fill();
    }else{
        canvas.stroke();
    }
}

void ScatterChart::drawX(Canvas& canvas,  Paint& paint, float x, float y) {
    canvas.move_to(x - mSize, y - mSize);
    canvas.line_to(x + mSize, y + mSize);
    canvas.move_to(x + mSize, y - mSize);
    canvas.line_to(x - mSize, y + mSize);
    canvas.stroke();
}

void ScatterChart::drawCircle(Canvas& canvas,  Paint& paint, float x, float y) {
    canvas.arc(x,y,mSize,0,M_PI*2.0);
    if(paint.style==Style::FILL)canvas.fill();
    else canvas.stroke();
}

void ScatterChart::drawTriangle(Canvas& canvas,  Paint& paint, std::vector<float>& path, float x, float y) {
    path[0] = x;/*x,y is center of triangle*/
    path[1] = y - mSize - mSize / 2.0;
    path[2] = x - mSize;
    path[3] = y + mSize;
    path[4] = x + mSize;
    path[5] = path[3];
    drawPath(canvas, path, paint, true);
}

void ScatterChart::ScatterChart::drawSquare(Canvas& canvas,  Paint& paint, float x, float y) {
    //canvas.drawRect(x - size, y - size, x + size, y + size, paint);
    canvas.rectangle(x - mSize, y - mSize,mSize*2.0,mSize*2.0);
    if(paint.style==Style::STROKE)
        canvas.stroke();
    else canvas.fill();
}

void ScatterChart::drawDiamond(Canvas& canvas,  Paint& paint, std::vector<float>& path, float x, float y) {
    path[0] = x;
    path[1] = y - mSize;
    path[2] = x - mSize;
    path[3] = y;
    path[4] = x;
    path[5] = y + mSize;
    path[6] = x + mSize;
    path[7] = y;
    drawPath(canvas, path, paint, true);
}

std::string ScatterChart::getChartType() const{
    return "Scatter"/*()TYPE*/;
}
}/*endof namespace*/
