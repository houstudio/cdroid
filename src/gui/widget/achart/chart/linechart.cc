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
#include <widget/achart/chart/linechart.h>
#include <widget/achart/chart/scatterchart.h>
#include <widget/achart/renderer/xyseriesrenderer.h>
namespace cdroid {

LineChart::LineChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
        const std::shared_ptr<XYMultipleSeriesRenderer>& renderer)
    :XYChart(dataset, renderer){
    mPointsChart = new ScatterChart(dataset, renderer);
}

LineChart::~LineChart(){
    delete mPointsChart;
}

void LineChart::setDatasetRenderer(
        const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
        const std::shared_ptr<XYMultipleSeriesRenderer>& renderer) {
    XYChart::setDatasetRenderer(dataset, renderer);
    mPointsChart = new ScatterChart(dataset, renderer);
}

void LineChart::drawSeries(Canvas& canvas,  Paint& paint,std::vector<float>& points,
        const std::shared_ptr<XYSeriesRenderer>& renderer, float yAxisValue, int seriesIndex, int startIndex) {
    const float lineWidth = canvas.get_line_width();
    canvas.set_line_width(renderer->getLineWidth());
    std::vector<XYSeriesRenderer::FillOutsideLine> fillOutsideLine = renderer->getFillOutsideLine();

    for (XYSeriesRenderer::FillOutsideLine& fill : fillOutsideLine) {
        if (fill.getType() != XYSeriesRenderer::FillOutsideLine::Type::NONE) {
            canvas.set_color(fill.getColor());
            // TODO: find a way to do area charts without duplicating data
            std::vector<float> fillPoints;
            std::vector<int> range = fill.getFillRange();
            if (range.empty()) {
                fillPoints.insert(fillPoints.end(),points.begin(),points.end());
            } else {
                fillPoints.insert(fillPoints.end(),
                        points.begin()+range[0]*2,points.begin()+range[1]*2);
            }

            float referencePoint;
            switch (fill.getType()) {
            case XYSeriesRenderer::FillOutsideLine::BOUNDS_ALL:
                referencePoint = yAxisValue;
                break;
            case XYSeriesRenderer::FillOutsideLine::BOUNDS_BELOW:
                referencePoint = yAxisValue;
                break;
            case XYSeriesRenderer::FillOutsideLine::BOUNDS_ABOVE:
                referencePoint = yAxisValue;
                break;
            case XYSeriesRenderer::FillOutsideLine::BELOW:
                referencePoint = (dynamic_cast<Cairo::ImageSurface*>(canvas.get_target().get()))->get_height();//m_height;//canvas.getHeight();
                break;
            case XYSeriesRenderer::FillOutsideLine::ABOVE:
                referencePoint = 0;
                break;
            default:
                throw std::runtime_error("You have added a new type of filling but have not implemented.");
            }
            int length = fillPoints.size();
            if((length > 0 && fill.getType() == XYSeriesRenderer::FillOutsideLine::Type::BOUNDS_ABOVE && fillPoints[1] < referencePoint)
                 || (fill.getType() == XYSeriesRenderer::FillOutsideLine::Type::BOUNDS_BELOW && fillPoints[1] > referencePoint)){
                std::vector<float> boundsPoints;
                bool add = false;
                if (((fill.getType() == XYSeriesRenderer::FillOutsideLine::BOUNDS_ABOVE) && (fillPoints.at(1) < referencePoint))
                        || ((fill.getType() == XYSeriesRenderer::FillOutsideLine::BOUNDS_BELOW) && (fillPoints.at(1) > referencePoint))) {
                    boundsPoints.push_back(fillPoints.at(0));
                    boundsPoints.push_back(fillPoints.at(1));
                    add = true;
                }

                for (int i = 3; i < fillPoints.size(); i += 2) {
                    float prevValue = fillPoints.at(i - 2);
                    float value = fillPoints.at(i);

                    if (prevValue < referencePoint && value > referencePoint || prevValue > referencePoint
                            && value < referencePoint) {
                        float prevX = fillPoints.at(i - 3);
                        float x = fillPoints.at(i - 1);
                        boundsPoints.push_back(prevX + (x - prevX) * (referencePoint - prevValue) / (value - prevValue));
                        boundsPoints.push_back(referencePoint);
                        if (((fill.getType() == XYSeriesRenderer::FillOutsideLine::BOUNDS_ABOVE) && (value > referencePoint))
                                || ((fill.getType() == XYSeriesRenderer::FillOutsideLine::BOUNDS_BELOW) && (value < referencePoint))) {
                            i += 2;
                            add = false;
                        } else {
                            boundsPoints.push_back(x);
                            boundsPoints.push_back(value);
                            add = true;
                        }
                    } else {
                        if (add || ((fill.getType() == XYSeriesRenderer::FillOutsideLine::BOUNDS_ABOVE) && (value < referencePoint))
                                || ((fill.getType() == XYSeriesRenderer::FillOutsideLine::BOUNDS_BELOW) && (value > referencePoint))) {
                            boundsPoints.push_back(fillPoints.at(i - 1));
                            boundsPoints.push_back(value);
                        }
                    }
                }

                fillPoints.clear();
                fillPoints = boundsPoints;//addAll(boundsPoints);
            }
            length = fillPoints.size();
            if(length){
                fillPoints[0]= fillPoints.at(0) + 1;
                fillPoints.push_back(fillPoints.at(length - 2));
                fillPoints.push_back(referencePoint);
                fillPoints.push_back(fillPoints.at(0));
                fillPoints.push_back(fillPoints.at(length + 1));
                for (int i = 0; i < length + 4; i += 2) {
                    if (fillPoints.at(i + 1) < 0) {
                        fillPoints[i + 1]= 0.f;
                    }
                }
                paint.setStyle(Style::FILL);
                drawPath(canvas, fillPoints, paint, true);
            }
        }
    }
    canvas.set_color(renderer->getColor());
    paint.setStyle(Style::STROKE);
    drawPath(canvas, points, paint, false);
    canvas.set_line_width(lineWidth);
}

std::vector<ClickableArea> LineChart::clickableAreasForPoints(const std::vector<float>& points,
        const std::vector<double>& values, float yAxisValue, int seriesIndex, int startIndex) {
    const int length = points.size();
    std::vector<ClickableArea> ret(length / 2);
    const int selectableBuffer = mRenderer->getSelectableBuffer();
    for (int i = 0; i < length; i += 2) {
        ret[i / 2] = ClickableArea({points.at(i) - selectableBuffer, points.at(i + 1) - selectableBuffer,
                selectableBuffer*2.f, selectableBuffer*2.f},
                values.at(i), values.at(i + 1));
    }
    return ret;
}

int LineChart::getLegendShapeWidth(int seriesIndex) const{
    return SHAPE_WIDTH;
}

void LineChart::drawLegendShape(Canvas& canvas, const std::shared_ptr<SimpleSeriesRenderer>& renderer,
        float x, float y, int seriesIndex,  Paint& paint) {
    const float lineWidth = ((const XYSeriesRenderer*)renderer.get())->getLineWidth();
    if(lineWidth!=0.f){
        canvas.set_line_width(lineWidth);
        drawLine(canvas,x, y,x + SHAPE_WIDTH, y);
        canvas.stroke();
    }
    if (isRenderPoints(renderer)) {
        mPointsChart->drawLegendShape(canvas, renderer, x + 5, y, seriesIndex, paint);
    }
}

bool LineChart::isRenderPoints(const std::shared_ptr<SimpleSeriesRenderer>& renderer) const{
    return ((XYSeriesRenderer*) renderer.get())->getPointStyle() != PointStyle::POINT;
}

ScatterChart* LineChart::getPointsChart() const{
    return mPointsChart;
}

std::string LineChart::getChartType() const{
    return "Line"/*TYPE*/;
}
}/*endof namespace*/
