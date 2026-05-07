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
#include <drawable/gradientdrawable.h>
#include <widget/achart/chart/barchart.h>
namespace cdroid{

BarChart::BarChart(Type type) {
    mType = type;
}

BarChart::BarChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
        const std::shared_ptr<XYMultipleSeriesRenderer>& renderer, Type type)
    :XYChart(dataset, renderer){
    mType = type;
}

std::vector<ClickableArea>  BarChart::clickableAreasForPoints(const std::vector<float>& points,
        const std::vector<double>& values,float yAxisValue, int seriesIndex, int startIndex) {
    const int seriesNr = mDataset->getSeriesCount();
    const int length = points.size();
    std::vector<ClickableArea> ret(length / 2);
    float halfDiffX = getHalfDiffX(points, length, seriesNr);
    for (int i = 0; i < length; i += 2) {
        const float x = points.at(i);
        const float y = points.at(i + 1);
        const float minY = std::min(y, yAxisValue);
        const float maxY = std::max(y, yAxisValue);
        if (mType == Type::STACKED||mType == Type::HEAPED) {
            ret[i / 2] = ClickableArea({x - halfDiffX, minY, halfDiffX*2.f, maxY-minY},
                    values.at(i), values.at(i + 1));
        } else {
            const float startX = x - seriesNr * halfDiffX + seriesIndex * 2 * halfDiffX;
            ret[i / 2] = ClickableArea({startX, minY, 2 * halfDiffX, maxY-minY},
                    values.at(i), values.at(i + 1));
        }
    }
    return ret;
}

void  BarChart::drawSeries(Canvas& canvas,  Paint& paint,std::vector<float>& points,
        const std::shared_ptr<XYSeriesRenderer>& seriesRenderer, float yAxisValue, int seriesIndex, int startIndex) {
    const int seriesNr = mDataset->getSeriesCount();
    const int length = points.size();
    const float halfDiffX = getHalfDiffX(points, length, seriesNr);
    canvas.set_color(seriesRenderer->getColor());
    paint.setStyle(Style::FILL);
    for (int i = 0; i < length; i += 2) {
        float x = points.at(i);
        float y = points.at(i + 1);
        
        if (mType == Type::HEAPED && seriesIndex > 0) {
            const float lastY = mPreviousSeriesPoints.at(i + 1);
            y = y + (lastY - yAxisValue);
            points[i + 1]= y;
            drawBar(canvas, x, lastY, x, y, halfDiffX, seriesNr, seriesIndex, paint);
        }else{
            drawBar(canvas, x, yAxisValue, x, y, halfDiffX, seriesNr, seriesIndex, paint);
        }
        mPreviousSeriesPoints = points;
    }
    canvas.set_color(seriesRenderer->getColor());
}

void  BarChart::drawBar(Canvas& canvas, float xMin, float yMin, float xMax, float yMax,
        float halfDiffX, int seriesNr, int seriesIndex,  Paint& paint) {
    const int scale = mDataset->getSeriesAt(seriesIndex)->getScaleNumber();
    if (mType == Type::STACKED|| mType == Type::HEAPED) {
        drawBar(canvas, xMin - halfDiffX, yMax, xMax + halfDiffX, yMin, scale, seriesIndex, paint);
    } else {
        const float startX = xMin - seriesNr * halfDiffX + seriesIndex * 2 * halfDiffX;
        drawBar(canvas, startX, yMax, startX + 2 * halfDiffX, yMin, scale, seriesIndex, paint);
    }
}

void  BarChart::drawBar(Canvas& canvas, float xMin, float yMin,
        float xMax, float yMax,int scale,int seriesIndex,  Paint& paint) {
    auto renderer = mRenderer->getSeriesRendererAt(seriesIndex);
    float temp;
    if (xMin > xMax) {
        temp = xMin;
        xMin = xMax;
        xMax = temp;
    }
    if (yMin > yMax) {
        temp = yMin;
        yMin = yMax;
        yMax = temp;
    }
    if (renderer->isGradientEnabled()) {
        float minY = (float) toScreenPoint({ 0, renderer->getGradientStopValue() }, scale)[1];
        float maxY = (float) toScreenPoint({ 0, renderer->getGradientStartValue() },scale)[1];
        float gradientMinY = std::max(minY, std::min(yMin, yMax));
        float gradientMaxY = std::min(maxY, std::max(yMin, yMax));
        int gradientMinColor = renderer->getGradientStopColor();
        int gradientMaxColor = renderer->getGradientStartColor();
        int gradientStartColor = gradientMaxColor;
        int gradientStopColor = gradientMinColor;

        if (yMin < minY) {
            canvas.set_color(gradientMinColor);
            canvas.rectangle(std::round(xMin), std::round(yMin), std::round(xMax-xMin),
                            std::round(gradientMinY-yMin));
        } else {
            gradientStopColor = getGradientPartialColor(gradientMinColor, gradientMaxColor,
                                (maxY - gradientMinY) / (maxY - minY));
        }
        if (yMax > maxY) {
            canvas.set_color(gradientMaxColor);
            canvas.rectangle(std::round(xMin), std::round(gradientMaxY), std::round(xMax-xMin),
                            std::round(yMax-gradientMaxY));
        } else {
            gradientStartColor = getGradientPartialColor(gradientMaxColor, gradientMinColor,
                                 (gradientMaxY - minY) / (maxY - minY));
        }
        GradientDrawable gradient(GradientDrawable::Orientation::BOTTOM_TOP, {gradientStartColor, gradientStopColor});
        gradient.setBounds(std::round(xMin), std::round(gradientMinY), std::round(xMax), std::round(gradientMaxY));
        gradient.draw(canvas);
    } else {
        if (std::abs(yMin - yMax) < 1) {
            if (yMin < yMax) {
                yMax = yMin + 1;
            } else {
                yMax = yMin - 1;
            }
        }//canvas.set_color(paint.color);
        canvas.rectangle(std::round(xMin), std::round(yMin), std::round(xMax-xMin), std::round(yMax-yMin));
        canvas.fill();
    }
}

int  BarChart::getGradientPartialColor(int minColor, int maxColor, float fraction) const{
    uint8_t alpha = std::round(fraction * Color::alpha(minColor) + (1.f - fraction)
                           * Color::alpha(maxColor));
    uint8_t r = std::round(fraction * Color::red(minColor) + (1.f - fraction) * Color::red(maxColor));
    uint8_t g = std::round(fraction * Color::green(minColor) + (1.f - fraction) * Color::green(maxColor));
    uint8_t b = std::round(fraction * Color::blue(minColor) + (1.f - fraction) * Color::blue((maxColor)));
    return Color::toArgb( r, g, b,alpha);
}

void  BarChart::drawChartValuesText(Canvas& canvas,  const std::shared_ptr<XYSeries>& series, 
        const std::shared_ptr<XYSeriesRenderer>& renderer, Paint& paint,
        const std::vector<float>& points, int seriesIndex, int startIndex) {
    const int seriesNr = mDataset->getSeriesCount();
    const int length = points.size();
    const float halfDiffX = getHalfDiffX(points, length, seriesNr);
    for (int i = 0; i < length; i += 2) {
        const int index = startIndex + i / 2;
        const double value = series->getY(index);
        if (!isNullValue(value)) {
            float x = points.at(i);
            if (mType == Type::DEFAULT) {
                x += seriesIndex * 2 * halfDiffX - (seriesNr - 1.5f) * halfDiffX;
            }
            if (value >= 0) {
                drawText(canvas, getLabel(renderer->getChartValuesFormat(), value), x, 
                        points.at(i + 1) - renderer->getChartValuesSpacing(), paint, 0);
            } else {
                drawText(canvas, getLabel(renderer->getChartValuesFormat(), value), x,
                        points.at(i + 1) + renderer->getChartValuesTextSize()
                        + renderer->getChartValuesSpacing() - 3, paint, 0);
            }
        }
    }
}

int  BarChart::getLegendShapeWidth(int seriesIndex) const{
    return SHAPE_WIDTH;
}

void  BarChart::drawLegendShape(Canvas& canvas, const std::shared_ptr<SimpleSeriesRenderer>& renderer,
        float x, float y, int seriesIndex,  Paint& paint) {
    canvas.rectangle(x, y - SHAPE_WIDTH/2, SHAPE_WIDTH, SHAPE_WIDTH);
    if(paint.style==Style::FILL)canvas.fill();else canvas.stroke();
}

float  BarChart::getHalfDiffX(const std::vector<float>& points, int length, int seriesNr) const{
    float barWidth = mRenderer->getBarWidth();
    if (barWidth > 0) {
        return barWidth / 2;
    }
    int div = length;
    if (length > 2) {
        div = length - 2;
    }
    float halfDiffX = (points.at(length - 2) - points.at(0)) / div;
    if (halfDiffX == 0) {
        halfDiffX = 10;
    }

    if ((mType != Type::STACKED)&&(mType != Type::HEAPED)) {
        halfDiffX /= seriesNr;
    }
    return (float) (halfDiffX / (getCoeficient() * (1 + mRenderer->getBarSpacing())));
}

float  BarChart::getCoeficient() const{
    return 1.f;
}

bool  BarChart::isRenderNullValues() const{
    return true;
}

double  BarChart::getDefaultMinimum() const{
    return 0;
}

std::string  BarChart::getChartType() const{
    return "Bar"/*TYPE*/;
}
}
