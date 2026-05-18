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
#include <core/rect.h>
#include<widget/achart/chart/piechart.h>

namespace cdroid{

PieChart::PieChart(const std::shared_ptr<CategorySeries>& dataset,const std::shared_ptr<DefaultRenderer>& renderer)
    :RoundChart(dataset, renderer){
    const int sLength = mDataset->getItemCount();
    for(int i=0;i<sLength;i++){
        if(mRenderer->getSeriesRendererAt(i)->isHighlighted()){
            mDataIndex = i;
            break;
        }
    }
}

void PieChart::drawArc(Canvas& canvas,double centerX, double centerY, double radius,
        double startAngle, double sweepAngle,int paintStyle) {
    double startRad = startAngle * M_PI / 180.0;
    double endRad = (startAngle + sweepAngle) * M_PI / 180.0;
    canvas.move_to(centerX, centerY);
    canvas.arc(centerX, centerY, radius, startRad, endRad);
    canvas.close_path();
    if(paintStyle&Style::FILL)canvas.fill();
    else canvas.stroke();
}

void PieChart::draw(Canvas& canvas, int x, int y, int width, int height,  Paint& paint) {
    //paint.setAntiAlias(mRenderer->isAntialiasing());
    //paint.setStyle(Style::FILL);
    canvas.set_font_size(mRenderer->getLabelsTextSize());
    int legendSize = getLegendSize(mRenderer, height / 5, 0);
    const int left = x;
    const int top = y;
    const int right = x + width;
    const int categoryCount = mDataset->getItemCount();
    double total = 0;
    double maxValue = FLT_MIN;
    std::vector<std::string> titles(categoryCount);
    for (int i = 0; i < categoryCount; i++) {
        total += mDataset->getValue(i);
        maxValue = std::max(maxValue,mDataset->getValue(i));
        titles[i] = mDataset->getCategory(i);
    }
    if (mRenderer->isFitLegend()) {
        legendSize = drawLegend(canvas, mRenderer, titles, left, right, y,
                width, height, legendSize, paint, true);
    }
    const int bottom = y + height - legendSize;
    drawBackground(mRenderer, canvas, x, y, width, height, paint, false, DefaultRenderer::NO_COLOR);

    float currentAngle = mRenderer->getStartAngle();
    const int mRadius = std::min(std::abs(right - left), std::abs(bottom - top));
    const int radius = (int) (mRadius * 0.35 * mRenderer->getScale());

    if (mCenterX == NO_VALUE) {
        mCenterX = (left + right) / 2;
    }
    if (mCenterY == NO_VALUE) {
        mCenterY = (bottom + top) / 2;
    }

    // Hook in clip detection after center has been calculated
    mPieMapper->setDimensions(radius, mCenterX, mCenterY);
    const bool loadPieCfg = !mPieMapper->areAllSegmentPresent(categoryCount);
    if (loadPieCfg) {
        mPieMapper->clearPieSegments();
    }

    const float shortRadius = radius * 0.9f;
    const float longRadius = radius * 1.1f;
    std::vector<RectF> prevLabelsBounds;

    for (int i = 0; i < categoryCount; i++) {
        auto seriesRenderer = mRenderer->getSeriesRendererAt(i);
        if (seriesRenderer->isGradientEnabled()) {
            Color startColor(seriesRenderer->getGradientStartColor());
            Color endColor(seriesRenderer->getGradientStopColor());
            Cairo::RefPtr<Cairo::RadialGradient> grad = Cairo::RadialGradient::create(
                    mCenterX, mCenterY, 0.0, mCenterX, mCenterY, radius );
            grad->add_color_stop_rgba(0.0, startColor.red(), startColor.green(), startColor.blue(), startColor.alpha());
            grad->add_color_stop_rgba(1.0, endColor.red(), endColor.green(), endColor.blue(), endColor.alpha());
            canvas.set_source(grad);
        } else {
            canvas.set_color(seriesRenderer->getColor());
        }

        const float value = (float) mDataset->getValue(i);
        const float sweepAngle = (float) (value / total * 360.f);
        const float radiusScale = 1.f;//std::sqrt(value/maxValue);
        float translateX = 0,translateY=0;
        if (seriesRenderer->isHighlighted()) {
            const double rAngle = (90.0 - (currentAngle + sweepAngle / 2))*M_PI/180.0;
            translateX = (float) (radius * 0.1 * std::sin(rAngle));
            translateY = (float) (radius * 0.1 * std::cos(rAngle));
        }
        drawArc(canvas, mCenterX+translateX, mCenterY+translateY, radius*radiusScale, currentAngle, sweepAngle,Style::FILL);
        if(mDataIndex==i){
            canvas.set_color(getSeriesSelectionColor(i));
            drawArc(canvas, mCenterX+translateX,  mCenterY+translateY, (radius + 4)*radiusScale, currentAngle, sweepAngle,Style::STROKE);
        }
        canvas.set_color(seriesRenderer->getColor());
        drawLabel(canvas, mDataset->getCategory(i), mRenderer, prevLabelsBounds, mCenterX, mCenterY, shortRadius,
                longRadius, currentAngle, sweepAngle, left, right, mRenderer->getLabelsColor(), paint, true, false);
        if (mRenderer->isDisplayValues()) {
            drawLabel(canvas, getLabel(mRenderer->getSeriesRendererAt(i)->getChartValuesFormat(),
                mDataset->getValue(i)), mRenderer, prevLabelsBounds, mCenterX, mCenterY, shortRadius / 2, longRadius / 2,
                currentAngle, sweepAngle, left, right, mRenderer->getLabelsColor(), paint, false, true);
        }

        if (loadPieCfg) {
            mPieMapper->addPieSegment(i, value, currentAngle, sweepAngle);
        }
        currentAngle += sweepAngle;
    }
    prevLabelsBounds.clear();
    drawLegend(canvas, mRenderer, titles, left, right, y, width, height, legendSize, paint, false);
    drawTitle(canvas, x, y, width, paint);
}

bool PieChart::getSeriesAndPointForScreenCoordinate(const PointF& screenPoint,SeriesSelection&selection) const{
    return mPieMapper->getSeriesAndPointForScreenCoordinate(screenPoint,selection);
}

}
