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
#include <algorithm>
#include <cmath>
#include <core/color.h>
#include <core/rect.h>
#include <widget/achart/chart/rosechart.h>

namespace cdroid{
namespace {

static void drawFilledSector(Canvas& canvas, double centerX, double centerY, double radius,
        double startAngleDegrees, double sweepAngleDegrees) {
    const double startRadians = startAngleDegrees * M_PI / 180.0;
    const double endRadians = (startAngleDegrees + sweepAngleDegrees) * M_PI / 180.0;
    canvas.begin_new_path();
    canvas.move_to(centerX, centerY);
    canvas.arc(centerX, centerY, radius, startRadians, endRadians);
    canvas.close_path();
    canvas.fill();
}
}

RoseChart::RoseChart(const std::shared_ptr<MultipleCategorySeries>& dataset,const std::shared_ptr<DefaultRenderer>& renderer)
    : DoughnutChart(dataset, renderer),
      mInnerColor(0xFF686868),
      mShowInner(true),
      mInnerStroke(false),
      mIntervalAngle(0),
      mShowBgLines(false),
      mShowBgCircle(false),
      mBgLineColor(Color::BLACK),
      mShowOuterLabels(false),
      mBgLines(0) {
}

void RoseChart::drawBgCircles(Canvas& canvas, float radius) {
    if (!mShowBgCircle || mBgSegments.empty()) {
        return;
    }

    const double originalLineWidth = canvas.get_line_width();
    canvas.set_line_width(1.0);
    const int rc=mRenderer->getSeriesRendererCount();
    for (int i=0;i<rc){//std::map<float, int>::const_iterator it = mBgSegments.begin(); it != mBgSegments.end(); ++it) {
        const float newRadius = radius * it->first;
        if (newRadius <= 0.0f) {
            continue;
        }
        canvas.set_color(it->second);
        canvas.begin_new_path();
        canvas.arc(mCenterX, mCenterY, newRadius, 0.0, M_PI * 2.0);
        canvas.stroke();
    }
    canvas.set_line_width(originalLineWidth);
}

void RoseChart::drawBgLines(Canvas& canvas, float radius, int sectorCount) {
    if (!mShowBgLines || mBgLines <= 0 || sectorCount <= 0) {
        return;
    }

    const double originalLineWidth = canvas.get_line_width();
    canvas.set_line_width(1.0);
    canvas.set_color(mBgLineColor);
    const float totalAngle = 360.0f - static_cast<float>(mIntervalAngle * sectorCount);
    const float arcAngle = totalAngle / sectorCount;
    float offsetAngle = mRenderer->getStartAngle();
    for (int i = 0; i < mBgLines; ++i) {
        const double radians = (90.0 - (offsetAngle + mIntervalAngle + arcAngle / 2.0f)) * M_PI / 180.0;
        const float endX = static_cast<float>(mCenterX + radius * std::sin(radians));
        const float endY = static_cast<float>(mCenterY + radius * std::cos(radians));
        canvas.begin_new_path();
        canvas.move_to(mCenterX, mCenterY);
        canvas.line_to(endX, endY);
        canvas.stroke();
        offsetAngle += arcAngle + mIntervalAngle;
    }
    canvas.set_line_width(originalLineWidth);
}

void RoseChart::draw(Canvas& canvas, int x, int y, int width, int height, Paint& paint) {
    if (mDataset == NULL || mRenderer == NULL || mDataset->getCategoriesCount() <= 0) {
        return;
    }

    const int layerCount = std::min(mDataset->getCategoriesCount(), mRenderer->getSeriesRendererCount());
    if (layerCount <= 0) {
        return;
    }

    const int sectorCount = mDataset->getItemCount(0);
    if (sectorCount <= 0) {
        return;
    }

    for (int layer = 1; layer < layerCount; ++layer) {
        if (mDataset->getItemCount(layer) != sectorCount) {
            return;
        }
    }

    const float totalAngle = 360.0f - static_cast<float>(mIntervalAngle * sectorCount);
    const float arcAngle = sectorCount > 0 ? totalAngle / sectorCount : 0.0f;
    if (arcAngle <= 0.0f) {
        return;
    }

    canvas.set_font_size(mRenderer->getLabelsTextSize());
    int legendSize = getLegendSize(mRenderer, height / 5, 0);
    const int left = x;
    const int top = y;
    const int right = x + width;

    std::vector<std::string> categories(layerCount);
    for (int i = 0; i < layerCount; ++i) {
        categories[i] = mDataset->getCategory(i);
    }
    if (mRenderer->isFitLegend()) {
        legendSize = drawLegend(canvas, mRenderer, categories, left, right, y, width, height,
                                legendSize, paint, true);
    }

    const int bottom = y + height - legendSize;
    drawBackground(mRenderer, canvas, x, y, width, height, paint, false, DefaultRenderer::NO_COLOR);

    const int radiusBase = std::min(std::abs(right - left), std::abs(bottom - top));
    const float radius = static_cast<float>(radiusBase * 0.35 * mRenderer->getScale());
    if (mCenterX == NO_VALUE) {
        mCenterX = (left + right) / 2;
    }
    if (mCenterY == NO_VALUE) {
        mCenterY = (bottom + top) / 2;
    }

    if (mShowInner) {
        canvas.set_color(mInnerColor);
        canvas.begin_new_path();
        canvas.arc(mCenterX, mCenterY, radius, 0.0, M_PI * 2.0);
        if (mInnerStroke) {
            canvas.stroke();
        } else {
            canvas.fill();
        }
    }

    drawBgCircles(canvas, radius);
    drawBgLines(canvas, radius, sectorCount);

    const float labelRadius = mShowOuterLabels
        ? radius + std::max(10.0f, mRenderer->getLabelsTextSize())
        : radius - radius / 4.0f;
    std::vector<RectF> prevLabelsBounds;

    for (int layer = 0; layer < layerCount; ++layer) {
        const std::vector<double> values = mDataset->getValues(layer);
        const std::vector<std::string> titles = mDataset->getTitles(layer);
        auto seriesRenderer = mRenderer->getSeriesRendererAt(layer);
        float currentAngle = mRenderer->getStartAngle();

        if (seriesRenderer->isGradientEnabled()) {
            Color startColor(seriesRenderer->getGradientStartColor());
            Color endColor(seriesRenderer->getGradientStopColor());
            Cairo::RefPtr<Cairo::RadialGradient> grad = Cairo::RadialGradient::create(
                mCenterX, mCenterY, 0.0, mCenterX, mCenterY, std::max(1.0f, radius));
            grad->add_color_stop_rgba(0.0, startColor.red(), startColor.green(),
                                      startColor.blue(), startColor.alpha());
            grad->add_color_stop_rgba(1.0, endColor.red(), endColor.green(),
                                      endColor.blue(), endColor.alpha());
            canvas.set_source(grad);
        }

        for (int i = 0; i < sectorCount; ++i) {
            if (!seriesRenderer->isGradientEnabled()) {
                canvas.set_color(seriesRenderer->getColor());
            }

            const float percentage = static_cast<float>(values[i]);
            const float sectorRadius = std::max(0.0f, radius * (percentage / 100.0f));
            drawFilledSector(canvas, mCenterX, mCenterY, sectorRadius,
                currentAngle + static_cast<float>(mIntervalAngle), arcAngle);

            if (i < static_cast<int>(titles.size()) && !titles[i].empty()) {
                drawLabel(canvas, titles[i], mRenderer, prevLabelsBounds, mCenterX, mCenterY,
                          labelRadius, labelRadius, currentAngle + static_cast<float>(mIntervalAngle),
                          arcAngle, left, right, mRenderer->getLabelsColor(), paint, false, true);
            }
            currentAngle += arcAngle + mIntervalAngle;
        }
    }

    drawLegend(canvas, mRenderer, categories, left, right, y, width, height, legendSize, paint, false);
    drawTitle(canvas, x, y, width, paint);
}

void RoseChart::setIntervalAngle(int angle) {
    mIntervalAngle = std::max(0, angle);
}

void RoseChart::showInner() {
    mShowInner = true;
}

void RoseChart::hideInner() {
    mShowInner = false;
}

void RoseChart::setInnerColor(int color) {
    mInnerColor = color;
}

void RoseChart::setInnerStroke(bool stroke) {
    mInnerStroke = stroke;
}

void RoseChart::showOuterLabels() {
    mShowOuterLabels = true;
}

void RoseChart::hideOuterLabels() {
    mShowOuterLabels = false;
}

void RoseChart::showBgLines(int color) {
    mShowBgLines = true;
    mBgLineColor = color;
}

void RoseChart::hideBgLines() {
    mShowBgLines = false;
}

void RoseChart::showBgCircle(const std::map<float, int>& bgSegments) {
    mShowBgCircle = true;
    mBgSegments = bgSegments;
}

void RoseChart::hideBgCircle() {
    mShowBgCircle = false;
}

void RoseChart::setBgLines(int count) {
    mBgLines = std::max(0, count);
}
}/*endof namespace*/

