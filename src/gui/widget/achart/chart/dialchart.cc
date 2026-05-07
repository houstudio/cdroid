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

#include <widget/achart/chart/dialchart.h>
namespace cdroid{

DialChart::DialChart(const std::shared_ptr<CategorySeries>& dataset,
        const std::shared_ptr<DialRenderer>& renderer)
    :RoundChart(dataset, renderer){
    mRenderer = renderer;
}

void DialChart::draw(Canvas& canvas, int x, int y, int width, int height,  Paint& paint) {
    //paint.setAntiAlias(mRenderer->isAntialiasing());
    paint.setStyle(Style::FILL);
    canvas.set_font_size(mRenderer->getLabelsTextSize());
    int legendSize = getLegendSize(mRenderer, height / 5, 0);
    int left = x;
    int top = y;
    int right = x + width;

    const int sLength = mDataset->getItemCount();
    std::vector<std::string> titles(sLength);
    setSize(width,height);
    for (int i = 0; i < sLength; i++) {
        titles[i] = mDataset->getCategory(i);
    }

    if (mRenderer->isFitLegend()) {
        legendSize = drawLegend(canvas, mRenderer, titles, left, right, y, width, height, legendSize, paint, true);
    }
    int bottom = y + height - legendSize;
    drawBackground(mRenderer, canvas, x, y, width, height, paint, false, DefaultRenderer::NO_COLOR);

    int mRadius = std::min(std::abs(right - left), std::abs(bottom - top));
    int radius = (int) (mRadius * 0.35 * mRenderer->getScale());
    if (mCenterX == NO_VALUE) {
        mCenterX = (left + right) / 2;
    }
    if (mCenterY == NO_VALUE) {
        mCenterY = (bottom + top) / 2;
    }
    float shortRadius = radius * 0.78f;
    float longRadius = radius * 0.92f;
    double min = mRenderer->getMinValue();
    double max = mRenderer->getMaxValue();
    double angleMin = mRenderer->getAngleMin();
    double angleMax = mRenderer->getAngleMax();
    if (!mRenderer->isMinValueSet() || !mRenderer->isMaxValueSet()) {
        int count = mRenderer->getSeriesRendererCount();
        for (int i = 0; i < count; i++) {
            double value = mDataset->getValue(i);
            if (!mRenderer->isMinValueSet()) {
                min = std::min(min, value);
            }
            if (!mRenderer->isMaxValueSet()) {
                max = std::max(max, value);
            }
        }
    }
    if (min == max) {
        min = min * 0.5;
        max = max * 1.5;
    }

    canvas.set_color(mRenderer->getLabelsColor());
    double minorTicks = mRenderer->getMinorTicksSpacing();
    double majorTicks = mRenderer->getMajorTicksSpacing();
    if (minorTicks == MathHelper::NULL_VALUE) {
        minorTicks = (max - min) / 30;
    }
    if (majorTicks == MathHelper::NULL_VALUE) {
        majorTicks = (max - min) / 10;
    }
    canvas.set_color(mRenderer->getAxesColor());
    canvas.set_line_width(2.0);
    canvas.arc(mCenterX, mCenterY, radius, angleMin * M_PI/ 180.0, angleMax * M_PI / 180.0);
    canvas.stroke();
    if(minorTicks>0)
    drawTicks(canvas, min, max, angleMin, angleMax, mCenterX, mCenterY, longRadius, radius,
              minorTicks, paint, false);
    if(majorTicks>0)
    drawTicks(canvas, min, max, angleMin, angleMax, mCenterX, mCenterY, longRadius, shortRadius,
              majorTicks, paint, true);

    const int count = mRenderer->getSeriesRendererCount();
    for (int i = 0; i < count; i++) {
        double angle = getAngleForValue(mDataset->getValue(i), angleMin, angleMax, min, max);
        canvas.set_color(mRenderer->getSeriesRendererAt(i)->getColor());
        bool type = mRenderer->getVisualTypeForIndex(i) == DialRenderer::Type::ARROW;
        drawNeedle(canvas, angle, mCenterX, mCenterY, shortRadius, type, paint);
        LOGD("type=%d angle=%.2f shortRadius=%.2f",type,angle,shortRadius);
    }
    drawLegend(canvas, mRenderer, titles, left, right, y, width, height, legendSize, paint, false);
    drawTitle(canvas, x, y, width, paint);
}

double DialChart::getAngleForValue(double value, double minAngle, double maxAngle,double min, double max) const{
    const double angleDiff = maxAngle - minAngle;
    const double diff = max - min;
    return (minAngle + (value - min) * angleDiff / diff)*M_PI/180.0;
}

void DialChart::drawTicks(Canvas& canvas, double min, double max, double minAngle, double maxAngle,
        int centerX, int centerY, double longRadius, double shortRadius, double ticks,  Paint& paint,bool labels) {
    for (double i = min; i <= max; i += ticks) {
        double angle = getAngleForValue(i, minAngle, maxAngle, min, max);
        double sinValue = std::sin(angle);
        double cosValue = std::cos(angle);
        int x1 = std::round(centerX + (float) (shortRadius * sinValue));
        int y1 = std::round(centerY + (float) (shortRadius * cosValue));
        int x2 = std::round(centerX + (float) (longRadius * sinValue));
        int y2 = std::round(centerY + (float) (longRadius * cosValue));
        canvas.move_to(x1, y1);
        canvas.line_to(x2, y2);
        canvas.stroke();
        if (labels) {
            paint.setTextAlign(Align::LEFT);
            if (x1 <= x2) {
                paint.setTextAlign(Align::RIGHT);
            }
            std::string text = std::to_string(i);
            if (std::round(i) == (long) i) {
                text = std::to_string((long) i);
            }
            //canvas.drawText(text, x1, y1, paint);
            canvas.move_to(x1,y1);
            canvas.show_text(text);
        }
    }
}

void DialChart::drawNeedle(Canvas& canvas, double angle, int centerX, int centerY, double radius,bool arrow,  Paint& paint) {
    double diff = M_PI/2.0;//Math.toRadians(90);
    int needleSinValue = (int) (NEEDLE_RADIUS * std::sin(angle - diff));
    int needleCosValue = (int) (NEEDLE_RADIUS * std::cos(angle - diff));
    int needleX = (int) (radius * std::sin(angle));
    int needleY = (int) (radius * std::cos(angle));
    int needleCenterX = centerX + needleX;
    int needleCenterY = centerY + needleY;
    std::vector<float> points;
    if (arrow) {
        int arrowBaseX = centerX + (int) (radius * 0.85 * std::sin(angle));
        int arrowBaseY = centerY + (int) (radius * 0.85 * std::cos(angle));
        points = { 
            float(arrowBaseX - needleSinValue),
            float(arrowBaseY - needleCosValue), float(needleCenterX), float(needleCenterY),
            float(arrowBaseX + needleSinValue), float(arrowBaseY + needleCosValue)
                             };
        float width = canvas.get_line_width();
        canvas.set_line_width(5);
        canvas.move_to(centerX, centerY);
        canvas.line_to(needleCenterX, needleCenterY);
        canvas.stroke();
        canvas.set_line_width(width);
    } else {
        points = { float(centerX - needleSinValue), float(centerY - needleCosValue),
            float(needleCenterX), float(needleCenterY), 
            float(centerX + needleSinValue),
            float(centerY + needleCosValue)};
    }
    drawPath(canvas, points, paint, true);
}

}/**/
