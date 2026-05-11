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
#include <widget/achart/chart/xychart.h>
#include <widget/achart/chart/scatterchart.h>
namespace cdroid{

XYChart::XYChart(){
}
XYChart::XYChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,const std::shared_ptr<XYMultipleSeriesRenderer>& renderer) {
    mDataset = dataset;
    mRenderer = renderer;
}

void XYChart::setDatasetRenderer(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
        const std::shared_ptr<XYMultipleSeriesRenderer>& renderer) {
    mDataset = dataset;
    mRenderer = renderer;
}

void XYChart::draw(Canvas& canvas, int x, int y, int width, int height,  Paint& paint) {
    //paint.setAntiAlias(mRenderer->isAntialiasing());
    int legendSize = getLegendSize(mRenderer, height / 5, mRenderer->getAxisTitleTextSize());
    std::vector<int> margins = mRenderer->getMargins();
    int left = x + margins[1];
    int top = y + margins[0];
    int right = x + width - margins[3];
    int sLength = mDataset->getSeriesCount();
    std::vector<std::string> titles(sLength);
    for (int i = 0; i < sLength; i++) {
        titles[i] = mDataset->getSeriesAt(i)->getTitle();
    }
    if (mRenderer->isFitLegend() && mRenderer->isShowLegend()) {
        legendSize = drawLegend(canvas, mRenderer, titles, left, right, y, width, height, legendSize,paint, true);
    }
    int bottom = y + height - margins[2] - legendSize;
    /*if (mScreenR == null) {
        mScreenR = new Rect();
    }*/
    mScreenR.set(left, top, right - left,bottom - top);
    drawBackground(mRenderer, canvas, x, y, width, height, paint, false, DefaultRenderer::NO_COLOR);

    /*if (paint.getTypeface() == null
            || (mRenderer->getTextTypeface() != null && paint.getTypeface().equals(
                    mRenderer->getTextTypeface()))
            || !paint.getTypeface().toString().equals(mRenderer->getTextTypefaceName())
            || paint.getTypeface().getStyle() != mRenderer->getTextTypefaceStyle()) {
        if (mRenderer->getTextTypeface() != null) {
            paint.setTypeface(mRenderer->getTextTypeface());
        } else {
            paint.setTypeface(Typeface::create(mRenderer->getTextTypefaceName(),
                                              mRenderer->getTextTypefaceStyle()));
        }
    }*/
    const int orientation = mRenderer->getOrientation();
    if (orientation == XYMultipleSeriesRenderer::Orientation::VERTICAL) {
        right -= legendSize;
        bottom += legendSize - 20;
    }
    int angle = orientation;
    bool rotate = angle == 90;
    mScale = (float) (height) / width;
    mTranslate = std::abs(width - height) / 2;
    if (mScale < 1) {
        mTranslate *= -1;
    }
    mCenter ={float(x + width) / 2.f, float(y + height) / 2.f};
    if (rotate) {
        transform(canvas, angle, false);
    }

    int maxScaleNumber = -INT_MAX;//Integer.MAX_VALUE;
    for (int i = 0; i < sLength; i++) {
        maxScaleNumber = std::max(maxScaleNumber, mDataset->getSeriesAt(i)->getScaleNumber());
    }
    maxScaleNumber++;
    if (maxScaleNumber < 0) {
        return;
    }
    std::vector<double> minX(maxScaleNumber);
    std::vector<double> maxX(maxScaleNumber);
    std::vector<double> minY(maxScaleNumber);
    std::vector<double> maxY(maxScaleNumber);
    std::vector<bool> isMinXSet(maxScaleNumber);
    std::vector<bool> isMaxXSet(maxScaleNumber);
    std::vector<bool> isMinYSet(maxScaleNumber);
    std::vector<bool> isMaxYSet(maxScaleNumber);

    for (int i = 0; i < maxScaleNumber; i++) {
        minX[i] = mRenderer->getXAxisMin(i);
        maxX[i] = mRenderer->getXAxisMax(i);
        minY[i] = mRenderer->getYAxisMin(i);
        maxY[i] = mRenderer->getYAxisMax(i);
        isMinXSet[i] = mRenderer->isMinXSet(i);
        isMaxXSet[i] = mRenderer->isMaxXSet(i);
        isMinYSet[i] = mRenderer->isMinYSet(i);
        isMaxYSet[i] = mRenderer->isMaxYSet(i);
        if (mCalcRange.find(i) == mCalcRange.end()) {
            mCalcRange.insert({i, std::vector<double>(4)});
        }
    }
    std::vector<double> xPixelsPerUnit(maxScaleNumber);
    std::vector<double> yPixelsPerUnit(maxScaleNumber);
    for (int i = 0; i < sLength; i++) {
        auto series = mDataset->getSeriesAt(i);
        int scale = series->getScaleNumber();
        if (series->getItemCount() == 0) {
            continue;
        }
        if (!isMinXSet[scale]) {
            double minimumX = series->getMinX();
            minX[scale] = std::min(minX[scale], minimumX);
            mCalcRange[scale][0] = minX[scale];
        }
        if (!isMaxXSet[scale]) {
            double maximumX = series->getMaxX();
            maxX[scale] = std::max(maxX[scale], maximumX);
            mCalcRange[scale][1] = maxX[scale];
        }
        if (!isMinYSet[scale]) {
            double minimumY = series->getMinY();
            minY[scale] = std::min(minY[scale], minimumY);
            mCalcRange[scale][2] = minY[scale];
        }
        if (!isMaxYSet[scale]) {
            double maximumY = series->getMaxY();
            maxY[scale] = std::max(maxY[scale],maximumY);
            mCalcRange[scale][3] = maxY[scale];
        }
    }
    for (int i = 0; i < maxScaleNumber; i++) {
        if (maxX[i] - minX[i] != 0) {
            xPixelsPerUnit[i] = (right - left) / (maxX[i] - minX[i]);
        }
        if (maxY[i] - minY[i] != 0) {
            yPixelsPerUnit[i] = (float) ((bottom - top) / (maxY[i] - minY[i]));
        }
         // the X axis on multiple scales was wrong without this fix
        if (i > 0) {
            xPixelsPerUnit[i] = xPixelsPerUnit[0];
            minX[i] = minX[0];
            maxX[i] = maxX[0];
        }
    }

    bool hasValues = false;
    for (int i = 0; i < sLength; i++) {
        auto series = mDataset->getSeriesAt(i);
        if (series->getItemCount() == 0) {
            continue;
        }
        hasValues = true;
    }
    
    bool showLabels = mRenderer->isShowLabels() && hasValues;
    bool showGridX = mRenderer->isShowGridX();
    bool showGridY = mRenderer->isShowGridY();
    if (showGridX || showGridY) {
        // Draw the grid lines under everything else.
        std::vector<double> xLabels = getValidLabels(getXLabels(minX[0], maxX[0], mRenderer->getXLabels()));
        std::map<int, std::vector<double>> allYLabels = getYLabels(minY, maxY, maxScaleNumber);

        int xLabelsLeft = left;
        bool showXLabels = mRenderer->isShowXLabels();
        bool showYLabels = mRenderer->isShowYLabels();
        // Only draw the grid.
        mRenderer->setShowLabels(false);
        /*if (mGridPaint == null) {
            mGridPaint = new Paint();
            mGridPaint.setAntiAlias(true);
        }*/
        drawXLabels(xLabels, mRenderer->getXTextLabelLocations(), canvas, paint, xLabelsLeft, top,
              bottom, xPixelsPerUnit[0], minX[0], maxX[0]);
        drawYLabels(allYLabels, canvas, paint, maxScaleNumber, left, right, bottom, yPixelsPerUnit,
              minY);
        mRenderer->setShowLabels(showXLabels, showYLabels);
    }

    // use a linked list for these reasons:
    // 1) Avoid a large contiguous memory allocation
    // 2) We don't need random seeking, only sequential reading/writing, so
    // linked list makes sense
    mClickableAreas.clear();
    for (int i = 0; i < sLength; i++) {
        auto series = mDataset->getSeriesAt(i);
        int scale = series->getScaleNumber();
        if (series->getItemCount() == 0) {
            continue;
        }

        hasValues = true;
        auto seriesRenderer = std::dynamic_pointer_cast<XYSeriesRenderer>(mRenderer->getSeriesRendererAt(i));

        std::vector<float> points;
        std::vector<double> values;
        const float yAxisValue = std::min((float)bottom, (float) (bottom + yPixelsPerUnit[scale] * minY[scale]));
        std::vector<ClickableArea>& clickableArea = mClickableAreas[i];

        auto range = series->getRange(minX[scale], maxX[scale], seriesRenderer->isDisplayBoundingPoints());
        int startIndex = -1;
        clickableArea.clear();
        for (auto&value : range) {
            double xValue = value.first;//getKey();
            double yValue = value.second;//getValue();
            if (startIndex < 0 && (!isNullValue(yValue) || isRenderNullValues())) {
                startIndex = series->getIndexForKey(xValue);
            }

            values.push_back(value.first/*getKey()*/);
            values.push_back(value.second/*getValue()*/);

            if (!isNullValue(yValue)) {
                points.push_back((float) (left + xPixelsPerUnit[scale] * (xValue - minX[scale])));
                points.push_back((float) (bottom - yPixelsPerUnit[scale] * (yValue - minY[scale])));
            } else if (isRenderNullValues()) {
                points.push_back((float) (left + xPixelsPerUnit[scale] * (xValue - minX[scale])));
                points.push_back((float) (bottom - yPixelsPerUnit[scale] * (-minY[scale])));
            } else {
                if (points.size() > 0) {
                    drawSeries(series, canvas, paint, points, seriesRenderer, yAxisValue, i, orientation, startIndex);
                    auto  clickableAreasForSubSeries = clickableAreasForPoints(points, values, yAxisValue, i, startIndex);
                    clickableArea.insert(clickableArea.end(),clickableAreasForSubSeries.begin(),clickableAreasForSubSeries.end());
                    points.clear();
                    values.clear();
                    startIndex = -1;
                }
                clickableArea.push_back(ClickableArea({},0,0));//null);
            }
        }

        const int count = series->getAnnotationCount();
        paint.setColor(mRenderer->getLabelsColor());
        if (count > 0) {
            canvas.set_color(mRenderer->getLabelsColor());
            //canvas.set_font_size(mRenderer->getAnnotationsTextSize());
            //paint.setTextAlign(mRenderer->getAnnotationsTextAlign());
            Rect bound;
            for (int j = 0; j < count; j++) {
                const float xS = (float) (left + xPixelsPerUnit[scale] * (series->getAnnotationX(j) - minX[scale]));
                const float yS = (float) (bottom - yPixelsPerUnit[scale] * (series->getAnnotationY(j) - minY[scale]));
                //paint.getTextBounds(series->getAnnotationAt(j), 0, series->getAnnotationAt(j).length(),bound);
                if (xS < (xS + bound.width) && yS < height/*canvas.getHeight()*/) {
                    drawString(canvas, series->getAnnotationAt(j), xS, yS, paint);
                }
            }
        }

        if (points.size() > 0) {
            drawSeries(series, canvas, paint, points, seriesRenderer, yAxisValue, i, orientation, startIndex);
            auto clickableAreasForSubSeries = clickableAreasForPoints(points, values, yAxisValue, i, startIndex);
            clickableArea.insert(clickableArea.end(),clickableAreasForSubSeries.begin(),clickableAreasForSubSeries.end());
        }
    }/*endof for*/
    // draw stuff over the margins such as data doesn't render on these areas
    drawBackground(mRenderer, canvas, x, bottom, width, height - bottom, paint, true, mRenderer->getMarginsColor());
    drawBackground(mRenderer, canvas, x, y, width, margins[0], paint, true, mRenderer->getMarginsColor());
    if (orientation == XYMultipleSeriesRenderer::Orientation::HORIZONTAL) {
        drawBackground(mRenderer, canvas, x, y, left - x, height - y, paint, true, mRenderer->getMarginsColor());
        drawBackground(mRenderer, canvas, right, y, margins[3], height - y, paint, true, mRenderer->getMarginsColor());
    } else if (orientation == XYMultipleSeriesRenderer::Orientation::VERTICAL) {
        drawBackground(mRenderer, canvas, right, y, width - right, height - y, paint, true, mRenderer->getMarginsColor());
        drawBackground(mRenderer, canvas, x, y, left - x, height - y, paint, true, mRenderer->getMarginsColor());
    }

    const bool showTickMarks = mRenderer->isShowTickMarks();
    const bool showCustomTextGridX = mRenderer->isShowCustomTextGridX();
    const bool showCustomTextGridY = mRenderer->isShowCustomTextGridY();
    if (showLabels || showGridX) {
        std::vector<double> xLabels = getValidLabels(getXLabels(minX[0], maxX[0], mRenderer->getXLabels()));
        std::map<int, std::vector<double>> allYLabels = getYLabels(minY, maxY, maxScaleNumber);

        int xLabelsLeft = left;
        if (showLabels) {
            canvas.set_color(mRenderer->getXLabelsColor());
            canvas.set_font_size(mRenderer->getLabelsTextSize());
            paint.setTextAlign(mRenderer->getXLabelsAlign());
            if (mRenderer->getXLabelsAlign() == Align::LEFT) {
                xLabelsLeft += mRenderer->getLabelsTextSize() / 4;
            }
        }
        // Draw just the labels and not the grid lines.
        mRenderer->setShowGrid(false);
        drawXLabels(xLabels, mRenderer->getXTextLabelLocations(), canvas, paint, xLabelsLeft,
                top, bottom, xPixelsPerUnit[0], minX[0], maxX[0]);
        drawYLabels(allYLabels, canvas, paint, maxScaleNumber, left, right, bottom, yPixelsPerUnit, minY);
        mRenderer->setShowGridX(showGridX);
        mRenderer->setShowGridY(showGridY);
        if (showLabels) {//grid horizontal lines
            canvas.set_color(mRenderer->getLabelsColor());
            for (int i = 0; i < maxScaleNumber; i++) {
                int axisAlign = mRenderer->getYAxisAlign(i);
                auto yTextLabelLocations = mRenderer->getYTextLabelLocations(i);
                for (const double location : yTextLabelLocations) {
                    if (minY[i] <= location && location <= maxY[i]) {
                        const float yLabel = (float) (bottom - yPixelsPerUnit[i] * (location - minY[i]));
                        std::string label = mRenderer->getYTextLabel(location, i);
                        canvas.set_color(mRenderer->getYLabelsColor(i));
                        paint.setTextAlign(mRenderer->getYLabelsAlign(i));
                        if (orientation == XYMultipleSeriesRenderer::Orientation::HORIZONTAL) {
                            if (axisAlign == Align::LEFT) {
                                if(showTickMarks){
                                    canvas.move_to(left + getLabelLinePos(axisAlign),yLabel);
                                    canvas.line_to(left, yLabel);
                                    canvas.stroke();
                                }
                                drawText(canvas, label, left, yLabel - 2, paint, mRenderer->getYLabelsAngle());
                            } else {
                                if(showTickMarks){
                                    canvas.move_to(right, yLabel);
                                    canvas.line_to(right + getLabelLinePos(axisAlign), yLabel);
                                    canvas.stroke();
                                }
                                drawText(canvas, label, right, yLabel - 2, paint, mRenderer->getYLabelsAngle());
                            }

                            if (showCustomTextGridY) {
                                canvas.set_color(mRenderer->getGridColor());
                                canvas.move_to(left, yLabel);
                                canvas.line_to(right, yLabel);
                                canvas.stroke();
                            }
                        } else {
                            if(showTickMarks){canvas.move_to(right - getLabelLinePos(axisAlign),yLabel);
                                canvas.line_to(right, yLabel);
                                canvas.stroke();
                            }
                            drawText(canvas, label, right + 10, yLabel - 2, paint, mRenderer->getYLabelsAngle());
                            if (showCustomTextGridY) {
                                canvas.set_color(mRenderer->getGridColor());
                                canvas.move_to(right, yLabel);
                                canvas.line_to(left, yLabel);
                                canvas.stroke();
                            }
                        }
                    }
                }
            }
        }

        if (showLabels) {
            canvas.set_color(mRenderer->getLabelsColor());
            float size = mRenderer->getAxisTitleTextSize();
            canvas.set_font_size(size);
            paint.setTextAlign(Align::CENTER);
            if (orientation == XYMultipleSeriesRenderer::Orientation::HORIZONTAL) {
                drawText(canvas, mRenderer->getXTitle(), x + width / 2,
                    bottom + mRenderer->getLabelsTextSize() * 4 / 3 + mRenderer->getXLabelsPadding() + size,
                    paint, 0);
                for (int i = 0; i < maxScaleNumber; i++) {
                    int axisAlign = mRenderer->getYAxisAlign(i);
                    if (axisAlign == Align::LEFT) {
                        drawText(canvas, mRenderer->getYTitle(i), x + size, y + height / 2, paint, -90);
                    } else {
                        drawText(canvas, mRenderer->getYTitle(i), x + width, y + height / 2, paint, -90);
                    }
                }
                canvas.set_font_size(mRenderer->getChartTitleTextSize());
                drawText(canvas, mRenderer->getChartTitle(), x + width / 2,
                         y + mRenderer->getChartTitleTextSize(), paint, 0);
            } else if (orientation == XYMultipleSeriesRenderer::Orientation::VERTICAL) {
                drawText(canvas, mRenderer->getXTitle(), x + width / 2,
                         y + height - size + mRenderer->getXLabelsPadding(), paint, -90);
                drawText(canvas, mRenderer->getYTitle(), right + 20, y + height / 2, paint, 0);
                canvas.set_font_size(mRenderer->getChartTitleTextSize());
                drawText(canvas, mRenderer->getChartTitle(), x + size, top + height / 2, paint, 0);
            }
        }
    }/*end if (showLabels || showGridX||showGridY)*/
    if (orientation == XYMultipleSeriesRenderer::Orientation::HORIZONTAL) {
        drawLegend(canvas, mRenderer, titles, left, right, y + (int) mRenderer->getXLabelsPadding(),
                   width, height, legendSize, paint, false);
    } else if (orientation == XYMultipleSeriesRenderer::Orientation::VERTICAL) {
        transform(canvas, angle, true);
        drawLegend(canvas, mRenderer, titles, left, right, y + (int) mRenderer->getXLabelsPadding(),
                   width, height, legendSize, paint, false);
        transform(canvas, angle, false);
    }
    if (mRenderer->isShowAxes()) {
        canvas.set_color(mRenderer->getAxesColor());
        canvas.move_to(left, bottom);
        canvas.line_to(right, bottom);
        bool rightAxis = false;
        for (int i = 0; i < maxScaleNumber && !rightAxis; i++) {
            rightAxis = mRenderer->getYAxisAlign(i) == Align::RIGHT;
        }
        if (orientation == XYMultipleSeriesRenderer::Orientation::HORIZONTAL) {
            canvas.move_to(left, top);
            canvas.line_to(left, bottom);
            if (rightAxis) {
                canvas.move_to(right, top);
                canvas.line_to(right, bottom);
            }
        } else if (orientation == XYMultipleSeriesRenderer::Orientation::VERTICAL) {
            canvas.move_to(right, top);
            canvas.line_to(right, bottom);
        }
        canvas.stroke();
    }
    if (rotate) {
        transform(canvas, angle, true);
    }
}

std::vector<double> XYChart::getXLabels(double min, double max, int count) const{
    return MathHelper::getLabels(min, max, count);
}

std::map<int, std::vector<double>> XYChart::getYLabels(const std::vector<double>& minY,const std::vector<double>& maxY, int maxScaleNumber) const{
    std::map<int, std::vector<double>> allYLabels;
    for (int i = 0; i < maxScaleNumber; i++) {
        allYLabels.insert({i,
                getValidLabels(MathHelper::getLabels(minY[i], maxY[i], mRenderer->getYLabels()))});
    }
    return allYLabels;
}

Rect XYChart::getScreenR() const{
    return mScreenR;
}

void XYChart::setScreenR(const Rect& screenR) {
    mScreenR = screenR;
}

std::vector<double> XYChart::getValidLabels(const std::vector<double>& labels) const{
    std::vector<double> result = labels;
    result.erase(
        std::remove_if(result.begin(), result.end(), 
                      [](double label) { return std::isnan(label); }),
        result.end()
    );
    return result;
}

void XYChart::drawSeries(const std::shared_ptr<XYSeries>& series, Canvas& canvas,  Paint& paint,std::vector<float>& pointsList,
            const std::shared_ptr<XYSeriesRenderer>& seriesRenderer, float yAxisValue, int seriesIndex, int orientation,int startIndex) {
    BasicStroke* stroke = seriesRenderer->getStroke();
    //Cap cap = paint.getStrokeCap();
    //Join join = paint.getStrokeJoin();
    //float miter = paint.getStrokeMiter();
    //PathEffect pathEffect = paint.getPathEffect();
    //Style style = paint.getStyle();
    canvas.save();
    if (stroke != nullptr) {
        //PathEffect effect = null;
        if (!stroke->getIntervals().empty()) {
            //effect = new DashPathEffect(stroke.getIntervals(), stroke.getPhase());
            canvas.set_dash(stroke->getIntervals(),0);
        }
        //setStroke(stroke.getCap(), stroke.getJoin(), stroke.getMiter(), Style.FILL_AND_STROKE,effect, paint);
    }
    drawSeries(canvas, paint, pointsList, seriesRenderer, yAxisValue, seriesIndex, startIndex);
    drawPoints(canvas, paint, pointsList, seriesRenderer, yAxisValue, seriesIndex, startIndex);

    canvas.set_font_size(seriesRenderer->getChartValuesTextSize());//paint.setTextSize(seriesRenderer.getChartValuesTextSize());
    if (orientation == XYMultipleSeriesRenderer::Orientation::HORIZONTAL) {
        paint.setTextAlign(Align::CENTER);
    } else {
        paint.setTextAlign(Align::LEFT);
    }
    if (seriesRenderer->isDisplayChartValues()) {
        paint.setTextAlign(seriesRenderer->getChartValuesTextAlign());
        drawChartValuesText(canvas, series, seriesRenderer, paint, pointsList, seriesIndex, startIndex);
    }
    /*if (stroke != null) {
        setStroke(cap, join, miter, style, pathEffect, paint);
    }*/
    canvas.restore();
}

/*void XYChart::setStroke(Cap cap, Join join, float miter, Style style, PathEffect pathEffect, Paint& paint) {
    paint.setStrokeCap(cap);
    paint.setStrokeJoin(join);
    paint.setStrokeMiter(miter);
    paint.setPathEffect(pathEffect);
    paint.setStyle(style);
}*/

void XYChart::drawPoints(Canvas& canvas, Paint& paint, std::vector<float>& pointsList,
            const std::shared_ptr<XYSeriesRenderer>& seriesRenderer, float yAxisValue, int seriesIndex, int startIndex) {
    if (isRenderPoints(seriesRenderer)) {
        ScatterChart* pointsChart = getPointsChart();
        if (pointsChart != nullptr) {
            pointsChart->drawSeries(canvas, paint, pointsList, seriesRenderer, yAxisValue, seriesIndex,startIndex);
        }
    }
}

void XYChart::drawChartValuesText(Canvas& canvas, const std::shared_ptr<XYSeries>& series,
        const std::shared_ptr<XYSeriesRenderer>& renderer, Paint& paint,
        const std::vector<float>& points, int seriesIndex, int startIndex) {
    if (points.size() > 1) { // there are more than one point
        // record the first point's position
        float previousPointX = points.at(0);
        float previousPointY = points.at(1);
        for (int k = 0; k < points.size(); k += 2) {
            if (k == 2) {// decide whether to display first two points' values or not
                if ((std::abs(points.at(2) - points.at(0)) > renderer->getDisplayChartValuesDistance())
                        || (std::abs(points.at(3) - points.at(1)) > renderer->getDisplayChartValuesDistance())) {
                    // first point
                    drawText(canvas, getLabel(renderer->getChartValuesFormat(), series->getY(startIndex)),
                             points.at(0), points.at(1) - renderer->getChartValuesSpacing(), paint, 0);
                    // second point
                    drawText(canvas,
                             getLabel(renderer->getChartValuesFormat(), series->getY(startIndex + 1)),
                             points.at(2), points.at(3) - renderer->getChartValuesSpacing(), paint, 0);

                    previousPointX = points.at(2);
                    previousPointY = points.at(3);
                }
            } else if (k > 2) {
                // compare current point's position with the previous point's, if they
                // are not too close, display
                if ((std::abs(points.at(k) - previousPointX) > renderer->getDisplayChartValuesDistance())
                        || (std::abs(points.at(k + 1) - previousPointY) > renderer->getDisplayChartValuesDistance())) {
                    drawText(canvas,
                             getLabel(renderer->getChartValuesFormat(), series->getY(startIndex + k / 2)),
                             points.at(k), points.at(k + 1) - renderer->getChartValuesSpacing(), paint, 0);
                    previousPointX = points.at(k);
                    previousPointY = points.at(k + 1);
                }
            }
        }
    } else { // if only one point, display it
        for (int k = 0; k < points.size(); k += 2) {
            drawText(canvas,
                     getLabel(renderer->getChartValuesFormat(), series->getY(startIndex + k / 2)),
                     points.at(k), points.at(k + 1) - renderer->getChartValuesSpacing(), paint, 0);
        }
    }
}

void XYChart::drawText(Canvas& canvas, const std::string& text, float x, float y,  Paint& paint,float extraAngle) {
    const float angle = -mRenderer->getOrientation() + extraAngle;
    if (angle != 0) {
        canvas.translate(x,y);
        canvas.rotate_degrees(angle);
        canvas.translate(-x,-y);
    }
    drawString(canvas, text, x, y, paint);
    if (angle != 0) {
        canvas.translate(x,y);
        canvas.rotate_degrees(-angle);
        canvas.translate(-x,-y);
    }
}

void XYChart::transform(Canvas& canvas, float angle, bool inverse) {
    if (inverse) {
        canvas.translate(mCenter.x, mCenter.y);
        canvas.rotate(-angle*M_PI/180.0);
        canvas.translate(-mCenter.x, -mCenter.y);
        canvas.scale(1.0 / mScale, mScale);
        canvas.translate(mTranslate,-mTranslate);
    } else {
        canvas.translate(mCenter.x, mCenter.y);
        canvas.rotate(angle*M_PI/180.0);
        canvas.translate(-mCenter.x, -mCenter.y);
        canvas.translate(-mTranslate,mTranslate);
        canvas.scale(mScale, 1.0 / mScale);
    }
}

void XYChart::drawXLabels(const std::vector<double>& xLabels,const std::vector<double>& xTextLabelLocations, Canvas& canvas,
             Paint& paint, int left, int top, int bottom, double xPixelsPerUnit, double minX, double maxX) {
    const int length = xLabels.size();
    const bool showXLabels = mRenderer->isShowXLabels();
    const bool showGridY = mRenderer->isShowGridY();
    const bool showTickMarks = mRenderer->isShowTickMarks();
    for (int i = 0; i < length; i++) {
        double label = xLabels.at(i);
        float xLabel = (float) (left + xPixelsPerUnit * (label - minX));
        if (showXLabels) {
            canvas.set_color(mRenderer->getXLabelsColor());
            if(showTickMarks){
                canvas.move_to(xLabel, bottom);
                canvas.line_to(xLabel, bottom + mRenderer->getLabelsTextSize() / 3);
                canvas.stroke();
            }
            drawText(canvas, getLabel(mRenderer->getLabelFormat(), label), xLabel,
                     bottom + mRenderer->getLabelsTextSize() * 4 / 3 + mRenderer->getXLabelsPadding(),
                     paint, mRenderer->getXLabelsAngle());
        }
        if (showGridY) {
            canvas.set_line_width(mRenderer->getGridLineWidth());
            canvas.set_color(mRenderer->getGridColor());
            canvas.move_to(xLabel, bottom);
            canvas.line_to(xLabel, top);
            canvas.stroke();
        }
    }
    drawXTextLabels(xTextLabelLocations, canvas, paint, showXLabels, left, top, bottom,
                    xPixelsPerUnit, minX, maxX);
}

void XYChart::drawYLabels(const std::map<int,std::vector<double>>& allYLabels, Canvas& canvas,
         Paint& paint,int maxScaleNumber, int left, int right, int bottom,
        const std::vector<double>& yPixelsPerUnit,const std::vector<double>& minY) {
    const int orientation = mRenderer->getOrientation();
    const bool showGridX = mRenderer->isShowGridX();
    const bool showYLabels = mRenderer->isShowYLabels();
    const bool showTickMarks = mRenderer->isShowTickMarks();
    for (int i = 0; i < maxScaleNumber; i++) {
        paint.setTextAlign(mRenderer->getYLabelsAlign(i));
        const std::vector<double>& yLabels = allYLabels.find(i)->second;
        int length = yLabels.size();
        for (int j = 0; j < length; j++) {
            double label = yLabels.at(j);
            int axisAlign = mRenderer->getYAxisAlign(i);
            bool textLabel = mRenderer->getYTextLabel(label, i).size();// != null;
            float yLabel = (float) (bottom - yPixelsPerUnit[i] * (label - minY[i]));
            if (orientation == XYMultipleSeriesRenderer::Orientation::HORIZONTAL) {
                if (showYLabels && !textLabel) {
                    canvas.set_color(mRenderer->getYLabelsColor(i));//paint.setColor(mRenderer->getYLabelsColor(i));
                    if (axisAlign == Align::LEFT) {
                        //canvas.drawLine(left + getLabelLinePos(axisAlign), yLabel, left, yLabel, paint);
                        if(showTickMarks){
                            drawLine(canvas,left + getLabelLinePos(axisAlign), yLabel,left, yLabel);
                            canvas.stroke();
                        }
                        drawText(canvas, getLabel(mRenderer->getLabelFormat(), label),
                                 left - mRenderer->getYLabelsPadding(),
                                 yLabel - mRenderer->getYLabelsVerticalPadding(), paint,
                                 mRenderer->getYLabelsAngle());
                    } else {
                        if(showTickMarks){
                            drawLine(canvas,right, yLabel,right + getLabelLinePos(axisAlign), yLabel);
                            canvas.stroke();
                        }
                        drawText(canvas, getLabel(mRenderer->getLabelFormat(), label),
                                 right + mRenderer->getYLabelsPadding(),
                                 yLabel - mRenderer->getYLabelsVerticalPadding(), paint,
                                 mRenderer->getYLabelsAngle());
                    }
                }
                if (showGridX) {
                    canvas.set_color(mRenderer->getGridColor());//paint.setColor(mRenderer->getGridColor());
                    //canvas.drawLine(left, yLabel, right, yLabel, paint);
                    if(showTickMarks){
                        drawLine(canvas,left,yLabel,right,yLabel);
                        canvas.stroke();
                    }
                }
            } else if (orientation == XYMultipleSeriesRenderer::Orientation::VERTICAL) {
                if (showYLabels && !textLabel) {
                    canvas.set_color(mRenderer->getYLabelsColor(i));//paint.setColor(mRenderer->getYLabelsColor(i));
                    //canvas.drawLine(right - getLabelLinePos(axisAlign), yLabel, right, yLabel, paint);
                    if(showTickMarks){
                        drawLine(canvas,right - getLabelLinePos(axisAlign), yLabel,right, yLabel);
                        canvas.stroke();
                    }
                    drawText(canvas, getLabel(mRenderer->getLabelFormat(), label),
                             right + 10 + mRenderer->getYLabelsPadding(), yLabel - 2, paint,
                             mRenderer->getYLabelsAngle());
                }
                if (showGridX) {
                    canvas.set_color(mRenderer->getGridColor());//paint.setColor(mRenderer->getGridColor());
                    //canvas.drawLine(right, yLabel, left, yLabel, paint);
                    if(showTickMarks){
                        drawLine(canvas,right, yLabel,left, yLabel);
                        canvas.stroke();
                    }
                }
            }
        }
    }
}

void XYChart::drawXTextLabels(const std::vector<double>& xTextLabelLocations, Canvas& canvas,  Paint& paint, bool showLabels,
        int left, int top, int bottom, double xPixelsPerUnit, double minX,double maxX) {
    const bool showCustomTextGridX = mRenderer->isShowCustomTextGridX();
    const bool showTickMarks = mRenderer->isShowTickMarks();
    if (showLabels) {
        //paint.setColor(mRenderer->getXLabelsColor());
        for (double location : xTextLabelLocations) {
            if (minX <= location && location <= maxX) {
                float xLabel = (float) (left + xPixelsPerUnit * (location - minX));
                canvas.set_color(mRenderer->getXLabelsColor());
                if(showTickMarks){
                    canvas.move_to(xLabel, bottom);
                    canvas.line_to(xLabel, bottom + mRenderer->getLabelsTextSize() / 3);
                    canvas.stroke();
                }
                drawText(canvas, mRenderer->getXTextLabel(location), xLabel,
                         bottom + mRenderer->getLabelsTextSize() * 4 / 3, paint, mRenderer->getXLabelsAngle());
                if (showCustomTextGridX) {
                    canvas.set_color(mRenderer->getGridColor());
                    canvas.move_to(xLabel, bottom);
                    canvas.line_to(xLabel, top);
                    canvas.stroke();
                }
            }
        }
    }
}

const std::shared_ptr<XYMultipleSeriesRenderer>& XYChart::getRenderer() const{
    return mRenderer;
}

const std::shared_ptr<XYMultipleSeriesDataset>& XYChart::getDataset() const{
    return mDataset;
}

std::vector<double> XYChart::getCalcRange(int scale) const{
    auto it = mCalcRange.find(scale);
    if(it!=mCalcRange.end())return it->second;
    return{};
}

void XYChart::setCalcRange(const std::vector<double>& range, int scale) {
    mCalcRange.insert({scale, range});
}

std::vector<double> XYChart::toRealPoint(float screenX, float screenY) const{
    return toRealPoint(screenX, screenY, 0);
}

std::vector<double> XYChart::toScreenPoint(const std::vector<double>& realPoint) const{
    return toScreenPoint(realPoint, 0);
}

int XYChart::getLabelLinePos(int/*Align*/ align) const{
    int pos = 4;
    if (align == Align::LEFT) {
        pos = -pos;
    }
    return pos;
}

std::vector<double> XYChart::toRealPoint(float screenX, float screenY, int scale) const{
    double realMinX = mRenderer->getXAxisMin(scale);
    double realMaxX = mRenderer->getXAxisMax(scale);
    double realMinY = mRenderer->getYAxisMin(scale);
    double realMaxY = mRenderer->getYAxisMax(scale);
    if (!mScreenR.empty()) {
        return {
                   (screenX - mScreenR.left) * (realMaxX - realMinX) / mScreenR.width + realMinX,
                   (mScreenR.top + mScreenR.height - screenY) * (realMaxY - realMinY) / mScreenR.height + realMinY
               };
    } else {
        return { screenX, screenY };
    }
}

std::vector<double> XYChart::toScreenPoint(const std::vector<double>& realPoint, int scale) const{
    double realMinX = mRenderer->getXAxisMin(scale);
    double realMaxX = mRenderer->getXAxisMax(scale);
    double realMinY = mRenderer->getYAxisMin(scale);
    double realMaxY = mRenderer->getYAxisMax(scale);
    if (!mRenderer->isMinXSet(scale) || !mRenderer->isMaxXSet(scale) || !mRenderer->isMinXSet(scale)
            || !mRenderer->isMaxYSet(scale)) {
        std::vector<double> calcRange = getCalcRange(scale);
        realMinX = calcRange[0];
        realMaxX = calcRange[1];
        realMinY = calcRange[2];
        realMaxY = calcRange[3];
    }
    if (!mScreenR.empty()) {
        return {
                   (realPoint[0] - realMinX) * mScreenR.width / (realMaxX - realMinX) + mScreenR.left,
                   (realMaxY - realPoint[1]) * mScreenR.height / (realMaxY - realMinY) + mScreenR.top
               };
    } else {
        return realPoint;
    }
}

SeriesSelection* XYChart::getSeriesAndPointForScreenCoordinate(const PointF& screenPoint) const{
    if (mClickableAreas.empty()||(mRenderer==nullptr)){
        return nullptr;
    }
    const float selectableBuffer = mRenderer->getSelectableBuffer();
    for (int seriesIndex = mClickableAreas.size() - 1; seriesIndex >= 0; seriesIndex--) {
        // series 0 is drawn first. Then series 1 is drawn on top, and series 2
        // on top of that.
        // we want to know what the user clicked on, so traverse them in the
        // order they appear on the screen.
        int pointIndex = 0;
        auto it = mClickableAreas.find(seriesIndex);
        if (mClickableAreas.end()!=it) {
            RectF rectangle;
            for (const ClickableArea& area : it->second) {
                if (1/*area != null*/) {
                    rectangle = area.getRect();
                    if(!rectangle.empty()){
                        rectangle.inflate(selectableBuffer,selectableBuffer);
                    }
                    if (!rectangle.empty() && rectangle.contains(screenPoint.x, screenPoint.y)) {
                        return new SeriesSelection(seriesIndex, pointIndex, area.getX(), area.getY());
                    }
                }
                pointIndex++;
            }
        }
    }
    return AbstractChart::getSeriesAndPointForScreenCoordinate(screenPoint);
}

bool XYChart::isRenderNullValues() const{
    return false;
}

bool XYChart::isRenderPoints(const std::shared_ptr<SimpleSeriesRenderer>& renderer) const{
    return false;
}

double XYChart::getDefaultMinimum() const{
    return MathHelper::NULL_VALUE;
}

ScatterChart* XYChart::getPointsChart() const{
    return nullptr;
}
}
