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
#include <widget/achart/chart/radarchart.h>

namespace cdroid{
namespace {
static const int kGridLevelCount = 5;

static int withAlpha(int color, int alpha) {
    return Color::toArgb(Color::red(color), Color::green(color), Color::blue(color), alpha);
}

static PointF makeRadarPoint(int centerX, int centerY, double radius, double angleRadians) {
    PointF point;
    point.x = static_cast<float>(centerX + std::cos(angleRadians) * radius);
    point.y = static_cast<float>(centerY - std::sin(angleRadians) * radius);
    return point;
}
}

RadarChart::RadarChart(const std::shared_ptr<MultipleCategorySeries>& dataset,const std::shared_ptr<DefaultRenderer>& renderer)
    : DoughnutChart(dataset, renderer) {
}

void RadarChart::draw(Canvas& canvas, int x, int y, int width, int height, Paint& paint) {
    if (mDataset == nullptr || mRenderer == nullptr || mDataset->getCategoriesCount() <= 0) {
        return;
    }

    canvas.set_font_size(mRenderer->getLabelsTextSize());
    int legendSize = getLegendSize(mRenderer, height / 5, 0);
    const int left = x;
    const int top = y;
    const int right = x + width;
    std::vector<std::string> categories= mDataset->getCategories();
    if (mRenderer->isFitLegend()) {
        legendSize = drawLegend(canvas, mRenderer, categories, left, right, y, width, height,
                                legendSize, paint, true);
    }

    const int bottom = y + height - legendSize;
    drawBackground(mRenderer, canvas, x, y, width, height, paint, false, DefaultRenderer::NO_COLOR);

    const int categoryCount = std::min(mDataset->getCategoriesCount(), mRenderer->getSeriesRendererCount());
    const int axisCount = mDataset->getItemCount(0);
    if (axisCount <= 2) {
        drawLegend(canvas, mRenderer, categories, left, right, y, width, height, legendSize, paint, false);
        drawTitle(canvas, x, y, width, paint);
        return;
    }

    if (mCenterX == NO_VALUE) {
        mCenterX = (left + right) / 2;
    }
    if (mCenterY == NO_VALUE) {
        mCenterY = (bottom + top) / 2;
    }

    const int radiusBase = std::min(std::abs(right - left), std::abs(bottom - top));
    const double chartRadius = radiusBase * 0.30 * mRenderer->getScale();
    const std::vector<std::string> axisTitles = mDataset->getTitles(0);

    double globalMaxValue = 0.0;
    for (int category = 0; category < categoryCount; ++category) {
        const std::vector<double> values = mDataset->getValues(category);
        for (size_t i = 0; i < values.size(); ++i) {
            globalMaxValue = std::max(globalMaxValue, values[i]);
        }
    }
    if (globalMaxValue <= 0.0) {
        globalMaxValue = 1.0;
    }

    const double startAngle = mRenderer->getStartAngle() * M_PI / 180.0;
    canvas.set_color(withAlpha(mRenderer->getLabelsColor(), 0x55));
    canvas.set_line_width(1.0);
    paint.setStyle(STROKE);
    for (int level = 1; level <= kGridLevelCount; ++level) {
        std::vector<float> gridPoints;
        const double ratio = static_cast<double>(level) / kGridLevelCount;
        const double levelRadius = chartRadius * ratio;
        for (int axis = 0; axis < axisCount; ++axis) {
            const double angle = startAngle + M_PI / 2.0 - axis * (2.0 * M_PI / axisCount);
            auto pt = makeRadarPoint(mCenterX, mCenterY, levelRadius, angle);
            gridPoints.push_back(pt.x);
            gridPoints.push_back(pt.y);
        }
        drawPath(canvas,gridPoints,paint,true);
    }

    auto resolveTextAlign=[](float x, int centerX){
        if (std::fabs(x - centerX) < 4.0f) {
            return Align::CENTER;
        }
        return x < centerX ? Align::RIGHT : Align::LEFT;
    };

    for (int axis = 0; axis < axisCount; ++axis) {
        const double angle = startAngle + M_PI / 2.0 - axis * (2.0 * M_PI / axisCount);
        const PointF outerPoint = makeRadarPoint(mCenterX, mCenterY, chartRadius, angle);
        canvas.set_color(withAlpha(mRenderer->getLabelsColor(), 0x70));
        drawLine(canvas,mCenterX, mCenterY,outerPoint.x, outerPoint.y);
        canvas.stroke();

        const std::string axisLabel = axis < static_cast<int>(axisTitles.size()) ? axisTitles[axis]
                                                                                  : std::to_string(axis + 1);
        const PointF labelPoint = makeRadarPoint(mCenterX, mCenterY, chartRadius * 1.18, angle);
        paint.setTextAlign(resolveTextAlign(labelPoint.x, mCenterX));
        drawString(canvas, axisLabel, labelPoint.x, labelPoint.y, paint);
    }
    paint.setStyle(FILL);
    for (int category = 0; category < categoryCount; ++category) {
        const std::vector<double> values = mDataset->getValues(category);
        std::vector<float> polygon;
        polygon.reserve(values.size());
        for (size_t axis = 0; axis < values.size(); ++axis) {
            const double angle = startAngle + M_PI / 2.0 - axis * (2.0 * M_PI / axisCount);
            const double normalized = std::max(0.0, values[axis]) / globalMaxValue;
            const auto pt = makeRadarPoint(mCenterX, mCenterY, chartRadius * normalized, angle);
            polygon.push_back(pt.x);polygon.push_back(pt.y);
        }

        const int color = mRenderer->getSeriesRendererAt(category)->getColor();
        canvas.set_color(withAlpha(color, 0x33));
        canvas.set_line_width(2.0);
        drawPath(canvas,polygon,paint,true);

        canvas.set_color(color);
        canvas.set_line_width(2.0);
        for (size_t axis = 0; axis < polygon.size(); axis+=2) {
            canvas.begin_new_path();
            canvas.arc(polygon[axis], polygon[axis+1], 3.0, 0.0, 2.0 * M_PI);
            canvas.fill();
        }
    }

    drawLegend(canvas, mRenderer, categories, left, right, y, width, height, legendSize, paint, false);
    drawTitle(canvas, x, y, width, paint);
}

int RadarChart::getLegendShapeWidth(int seriesIndex) const {
    (void)seriesIndex;
    return SHAPE_WIDTH + 6;
}

void RadarChart::drawLegendShape(Canvas& canvas,const std::shared_ptr<SimpleSeriesRenderer>& renderer, float x, float y,
        int seriesIndex, Paint& paint) {
    (void)renderer;
    (void)seriesIndex;
    (void)paint;
    canvas.move_to(x, y + SHAPE_WIDTH / 2.0f);
    canvas.line_to(x + SHAPE_WIDTH / 2.0f, y - SHAPE_WIDTH / 2.0f);
    canvas.line_to(x + SHAPE_WIDTH, y + SHAPE_WIDTH / 2.0f);
    canvas.stroke();
}
}/*endof namespace*/

