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
#include <widget/achart/chart/doughnutchart.h>
namespace cdroid{
/**
 * The doughnut chart rendering class.
 */
DoughnutChart::DoughnutChart(const std::shared_ptr<MultipleCategorySeries>& dataset,
        const std::shared_ptr<DefaultRenderer>& renderer)
    :RoundChart(nullptr, renderer){
    mDataset = dataset;
}

static void drawRingSlice(Canvas& canvas, double centerX, double centerY, double outerRadius,
        double innerRadius, double startAngleDegrees, double sweepAngleDegrees) {
    const double startRadians = startAngleDegrees * M_PI / 180.0;
    const double endRadians = (startAngleDegrees + sweepAngleDegrees) * M_PI / 180.0;
    canvas.begin_new_path();
    canvas.arc(centerX, centerY, outerRadius, startRadians, endRadians);
    canvas.arc_negative(centerX, centerY, innerRadius, endRadians, startRadians);
    canvas.close_path();
    canvas.fill();
}

void DoughnutChart::draw(Canvas& canvas, int x, int y, int width, int height,  Paint& paint) {
    //paint.setAntiAlias(mRenderer->isAntialiasing());
    //paint.setStyle(Style.FILL);
    canvas.set_font_size(mRenderer->getLabelsTextSize());
    int legendSize = getLegendSize(mRenderer, height / 5, 0);
    int left = x;
    int top = y;
    int right = x + width;
    int ringSegments = 0;
    const int cLength = mDataset->getCategoriesCount();
    const std::vector<std::string>& categories = mDataset->getCategories();
    for (int category = 0; category < cLength; category++) {
        ringSegments += mDataset->getItemCount(category);
    }
    if (mRenderer->isFitLegend()) {
        legendSize = drawLegend(canvas, mRenderer, categories, left, right, y, width, height, legendSize, paint, true);
    }

    const int bottom = y + height - legendSize;
    drawBackground(mRenderer, canvas, x, y, width, height, paint, false, DefaultRenderer::NO_COLOR);
    mStep = SHAPE_WIDTH * 3 / 4;

    const int radiusBase = std::min(std::abs(right - left), std::abs(bottom - top));
    const double rCoef = 0.35 * mRenderer->getScale();
    const double ringThickness = cLength > 0 ? std::max(6.0, (radiusBase * 0.20) / cLength) : 0.0;
    const double ringGap = 2.0;
    double radius = radiusBase * rCoef;
    if (mCenterX == NO_VALUE) {
        mCenterX = (left + right) / 2;
    }
    if (mCenterY == NO_VALUE) {
        mCenterY = (bottom + top) / 2;
    }

    mPieMapper->setDimensions(radiusBase, mCenterX, mCenterY);
    const bool loadPieCfg = !mPieMapper->areAllSegmentPresent(ringSegments);
    if (loadPieCfg) {
        mPieMapper->clearPieSegments();
    }

    float shortRadius = radius * 0.9f;
    float longRadius = radius * 1.1f;
    std::vector<RectF> prevLabelsBounds;
    for (int category = 0; category < cLength; category++) {
        const int sLength = mDataset->getItemCount(category);
        double total = 0;
        std::vector<std::string> titles(sLength);
        for (int i = 0; i < sLength; i++) {
            total += mDataset->getValues(category)[i];
            titles[i] = mDataset->getTitles(category)[i];
        }
        if (total <= 0) {
            radius = std::max(0.0, radius - ringThickness - ringGap);
            shortRadius = std::max(0.f, shortRadius - static_cast<float>(ringThickness + ringGap));
            continue;
        }
        float currentAngle = mRenderer->getStartAngle();
        const double innerRadius = std::max(0.0, radius - ringThickness);
        for (int i = 0; i < sLength; i++) {
            canvas.set_color(mRenderer->getSeriesRendererAt(i)->getColor());
            const float value = (float) mDataset->getValues(category)[i];
            const float angle = (float) (value / total * 360);
            drawRingSlice(canvas, mCenterX, mCenterY, radius, innerRadius, currentAngle, angle);
            drawLabel(canvas, mDataset->getTitles(category)[i], mRenderer, prevLabelsBounds, mCenterX,
                      mCenterY, shortRadius, longRadius, currentAngle, angle, left, right,
                      mRenderer->getLabelsColor(), paint, true, false);
            if (loadPieCfg) {
               mPieMapper->addPieSegment((category<<16)|i, value, currentAngle, angle,innerRadius,radius-innerRadius);
            }
            currentAngle += angle;
        }
        radius = std::max(0.0, innerRadius - ringGap);
        shortRadius = std::max(0.f, shortRadius - static_cast<float>(ringThickness + ringGap));
    }
    prevLabelsBounds.clear();
    drawLegend(canvas, mRenderer, categories, left, right, y, width, height, legendSize, paint,false);
    drawTitle(canvas, x, y, width, paint);
}

int DoughnutChart::getLegendShapeWidth(int seriesIndex) const{
    return SHAPE_WIDTH;
}

void DoughnutChart::drawLegendShape(Canvas& canvas, const std::shared_ptr<SimpleSeriesRenderer>& renderer,
        float x, float y, int seriesIndex,  Paint& paint) {
    mStep = std::max(1, mStep - 1);
    //canvas.drawCircle(x + SHAPE_WIDTH - mStep, y, mStep, paint);
    canvas.arc(x+SHAPE_WIDTH,y,mStep,0,M_PI*2.0);
    if(paint.style==Style::FILL)canvas.fill();else canvas.stroke();
}

bool DoughnutChart::getSeriesAndPointForScreenCoordinate(const PointF& screenPoint,SeriesSelection&selection) const{
    return mPieMapper->getSeriesAndPointForScreenCoordinate(screenPoint,selection);
}

}/*endof namespace*/
