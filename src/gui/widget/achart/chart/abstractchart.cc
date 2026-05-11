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
#include <utils/textutils.h>
#include <utils/mathutils.h>
#include <widget/achart/util/mathhelper.h>
#include <widget/achart/chart/abstractchart.h>
#include <widget/achart/renderer/xymultipleseriesrenderer.h>
namespace cdroid {

void AbstractChart::drawBackground(const std::shared_ptr<DefaultRenderer>& renderer, Canvas& canvas, int x, int y, int width,
                                   int height, Paint& paint, bool newColor, int color) {
    if (renderer->isApplyBackgroundColor() || newColor) {
        if (newColor) {
            canvas.set_color(color);
        } else {
            canvas.set_color(renderer->getBackgroundColor());
        }
        paint.setStyle(Style::FILL);
        canvas.rectangle(x, y, width, height);//, paint);
        canvas.fill();
    }
}

void AbstractChart::drawLine(Canvas&canvas,float x1,float y1,float x2,float y2)const{
    canvas.move_to(x1,y1);
    canvas.line_to(x2,y2);
}

static std::vector<float>getTextWidths(Canvas& ctx, const std::string& text) {
    std::vector<float> widths;
    for (size_t i = 0; i < text.length(); ++i) {
        std::string char_str = text.substr(i, 1);
        Cairo::TextExtents extents;
        ctx.get_text_extents(char_str, extents);
        widths.push_back(extents.width);
    }
    return widths;
}

int AbstractChart::drawLegend(Canvas& canvas, const std::shared_ptr<DefaultRenderer>& renderer, const std::vector<std::string>& titles,
        int left, int right, int y, int width, int height, int legendSize, Paint& paint, bool calculate) {
    float size = 32;
    if (renderer->isShowLegend()) {
        float currentX = left;
        float currentY = y + height - legendSize + size;
        paint.setTextAlign(Align::LEFT);
        canvas.set_font_size(renderer->getLegendTextSize());//paint.setTextSize(renderer.getLegendTextSize());
        const int sLength = std::min((int)titles.size(), renderer->getSeriesRendererCount());
        for (int i = 0; i < sLength; i++) {
            auto r = renderer->getSeriesRendererAt(i);
            const float lineSize = getLegendShapeWidth(i);
            if (r->isShowLegendItem()) {
                std::string text = titles[i];
                if (titles.size() == renderer->getSeriesRendererCount()) {
                    paint.setColor(r->getColor());
                    canvas.set_color(r->getColor());//paint.setColor(r->getColor());
                } else {
                    paint.setColor(Color::LTGRAY);
                    canvas.set_color(Color::LTGRAY);//paint.setColor(Color::LTGRAY);
                }
                Cairo::TextExtents te;
                canvas.get_text_extents(text,te);
                std::vector<float> widths = getTextWidths(canvas,text);//paint.getTextWidths(text, widths);
                float sum = te.width;
                float extraSize = lineSize + 50 + sum;
                float currentWidth = currentX + extraSize;

                if (i > 0 && getExceed(currentWidth, renderer, right, width)) {
                    currentX = left;
                    currentY += renderer->getLegendTextSize();
                    size += renderer->getLegendTextSize();
                    currentWidth = currentX + extraSize;
                }
                if (getExceed(currentWidth, renderer, right, width)) {
                    float maxWidth = right - currentX - lineSize - 10;
                    if (isVertical(renderer)) {
                        maxWidth = width - currentX - lineSize - 10;
                    }
                    int nr = 1;//paint.breakText(text, true, maxWidth, widths);
                    text = text.substr(0, nr) + "...";
                }
                if (!calculate) {
                    drawLegendShape(canvas, r, currentX, currentY, i, paint);
                    drawString(canvas, text, currentX + lineSize + 5, currentY + 5, paint);
                }
                currentX += extraSize;
            }
        }
    }
    return std::round(size + renderer->getLegendTextSize());
}

void AbstractChart::drawString(Canvas& canvas,const std::string& text, float x, float y, Paint& paint) {
    if (!text.empty()) {
        std::vector<std::string> lines = TextUtils::split(text,"\n");//.split("\n");
        int yOff = 0;
        for (int i = 0; i < lines.size(); ++i) {
            Cairo::TextExtents te; //canvas.drawText(lines[i], x, y + yOff, paint);
            canvas.get_text_extents(lines[i],te);
            switch(paint.textAlign){
            case Align::LEFT:canvas.move_to(x,y+yOff);break;
            case Align::CENTER:canvas.move_to(x-te.width/2.0,y+yOff);break;
            case Align::RIGHT:canvas.move_to(x-te.width,y+yOff);break;
            }
            canvas.show_text(lines[i]);
            yOff = yOff + te.height/*rect.height*/ + 5; // space between lines is 5
        }
    }
}

bool AbstractChart::getExceed(float currentWidth, const std::shared_ptr<DefaultRenderer>& renderer, int right, int width) {
    bool exceed = currentWidth > right;
    if (isVertical(renderer)) {
        exceed = currentWidth > width;
    }
    return exceed;
}

bool AbstractChart::isVertical(const std::shared_ptr<DefaultRenderer>& renderer) const{
    auto derived_renderer = std::dynamic_pointer_cast<XYMultipleSeriesRenderer>(renderer);
    return (derived_renderer!=nullptr)
           && (derived_renderer->getOrientation() == XYMultipleSeriesRenderer::Orientation::VERTICAL);
}

std::string AbstractChart::getLabel(const std::string& format, double label) const{
    std::string text = "";
    if (!format.empty()) {
        text = TextUtils::stringPrintf(format.c_str(),label);
    } else {
        text =  TextUtils::stringPrintf("%g",label);
    }
    return text;
}

std::vector<float> AbstractChart::calculateDrawPoints(float p1x, float p1y, float p2x, float p2y,int screenHeight, int screenWidth) {
    float drawP1x;
    float drawP1y;
    float drawP2x;
    float drawP2y;
    if (p1y > screenHeight) {
        // Intersection with the top of the screen
        float m = (p2y - p1y) / (p2x - p1x);
        drawP1x = (screenHeight - p1y + m * p1x) / m;
        drawP1y = screenHeight;

        if (drawP1x < 0) {
            // If Intersection is left of the screen we calculate the intersection
            // with the left border
            drawP1x = 0;
            drawP1y = p1y - m * p1x;
        } else if (drawP1x > screenWidth) {
            // If Intersection is right of the screen we calculate the intersection
            // with the right border
            drawP1x = screenWidth;
            drawP1y = m * screenWidth + p1y - m * p1x;
        }
    } else if (p1y < 0) {
        float m = (p2y - p1y) / (p2x - p1x);
        drawP1x = (-p1y + m * p1x) / m;
        drawP1y = 0;
        if (drawP1x < 0) {
            drawP1x = 0;
            drawP1y = p1y - m * p1x;
        } else if (drawP1x > screenWidth) {
            drawP1x = screenWidth;
            drawP1y = m * screenWidth + p1y - m * p1x;
        }
    } else {
        // If the point is in the screen use it
        drawP1x = p1x;
        drawP1y = p1y;
    }

    if (p2y > screenHeight) {
        float m = (p2y - p1y) / (p2x - p1x);
        drawP2x = (screenHeight - p1y + m * p1x) / m;
        drawP2y = screenHeight;
        if (drawP2x < 0) {
            drawP2x = 0;
            drawP2y = p1y - m * p1x;
        } else if (drawP2x > screenWidth) {
            drawP2x = screenWidth;
            drawP2y = m * screenWidth + p1y - m * p1x;
        }
    } else if (p2y < 0) {
        float m = (p2y - p1y) / (p2x - p1x);
        drawP2x = (-p1y + m * p1x) / m;
        drawP2y = 0;
        if (drawP2x < 0) {
            drawP2x = 0;
            drawP2y = p1y - m * p1x;
        } else if (drawP2x > screenWidth) {
            drawP2x = screenWidth;
            drawP2y = m * screenWidth + p1y - m * p1x;
        }
    } else {
        // If the point is in the screen use it
        drawP2x = p2x;
        drawP2y = p2y;
    }

    return { drawP1x, drawP1y, drawP2x, drawP2y };
}

void AbstractChart::drawPath(Canvas& canvas,const std::vector<float>& points, Paint& paint, bool circular) {
    auto image_surface = std::dynamic_pointer_cast<Cairo::ImageSurface>(canvas.get_target());
    const int height = image_surface->get_height();
    const int width = image_surface->get_width();

    std::vector<float> tempDrawPoints;
    if (points.size() < 4) {
        return;
    }
    canvas.begin_new_path();
    tempDrawPoints = calculateDrawPoints(points[0], points[1], points[2], points[3], height, width);
    canvas.move_to/*path.moveTo*/(tempDrawPoints[0], tempDrawPoints[1]);
    canvas.line_to/*path.lineTo*/(tempDrawPoints[2], tempDrawPoints[3]);

    int length = points.size();
    for (int i = 4; i < length; i += 2) {
        if ((points[i - 1] < 0 && points[i + 1] < 0)
                || (points[i - 1] > height && points[i + 1] > height)) {
            continue;
        }
        tempDrawPoints = calculateDrawPoints(points[i - 2], points[i - 1], points[i], points[i + 1],
                                             height, width);
        if (!circular) {
            canvas.move_to/*path.moveTo*/(tempDrawPoints[0], tempDrawPoints[1]);
        }
        canvas.line_to/*path.lineTo*/(tempDrawPoints[2], tempDrawPoints[3]);
    }
    if (circular) {
        canvas.line_to/*path.lineTo*/(points[0], points[1]);
    }
    canvas.close_path();//canvas.drawPath(path, paint);
    if(paint.style==Style::FILL)
        canvas.fill();
    else
        canvas.stroke();
}

std::string AbstractChart::getFitText(const std::string& text, float width, Paint& paint) const{
    std::string newText = text;
    int length = text.length();
    int diff = 0;
    /*while (paint.measureText(newText) > width && diff < length) {
        diff++;
        newText = text.substr(0, length - diff) + "...";
    }
    if (diff == length) {
        newText = "...";
    }*/
    return newText;
}

int AbstractChart::getLegendSize(const std::shared_ptr<DefaultRenderer>& renderer, int defaultHeight, float extraHeight) {
    int legendSize = renderer->getLegendHeight();
    if (renderer->isShowLegend() && legendSize == 0) {
        legendSize = defaultHeight;
    }
    if (!renderer->isShowLegend() && renderer->isShowLabels()) {
        legendSize = (int) (renderer->getLabelsTextSize() * 4 / 3 + extraHeight);
    }
    return legendSize;
}

void AbstractChart::drawLabel(Canvas& canvas,const std::string& labelText, const std::shared_ptr<DefaultRenderer>& renderer,
        std::vector<RectF>& prevLabelsBounds, int centerX, int centerY, float shortRadius, float longRadius,
        float currentAngle, float angle, int left, int right, int color, Paint& paint, bool line,bool display) {
    if (renderer->isShowLabels() || display) {
        canvas.set_color(color);
        paint.setColor(color);
        double rAngle = MathUtils::toRadians(90 - (currentAngle + angle / 2));
        double sinValue = std::sin(rAngle);
        double cosValue = std::cos(rAngle);
        int x1 = std::round(centerX + (float) (shortRadius * sinValue));
        int y1 = std::round(centerY + (float) (shortRadius * cosValue));
        int x2 = std::round(centerX + (float) (longRadius * sinValue));
        int y2 = std::round(centerY + (float) (longRadius * cosValue));

        float size = renderer->getLabelsTextSize();
        float extra = std::max(size / 2, 10.f);
        paint.setTextAlign(Align::LEFT);
        if (x1 > x2) {
            extra = -extra;
            paint.setTextAlign(Align::RIGHT);
        }
        float xLabel = x2 + extra;
        float yLabel = y2;
        float width = right - xLabel;
        if (x1 > x2) {
            width = xLabel - left;
        }
        //labelText = getFitText(labelText, width, paint);
        Cairo::TextExtents te;
        canvas.get_text_extents(labelText,te);
        float widthLabel = te.width;//paint.measureText(labelText);
        bool okBounds = false;
        while (!okBounds && line) {
            bool intersects = false;
            int length = prevLabelsBounds.size();
            for (int j = 0; j < length && !intersects; j++) {
                RectF prevLabelBounds = prevLabelsBounds.at(j);
                if (prevLabelBounds.intersect(xLabel, yLabel, widthLabel, size)) {
                    intersects = true;
                    yLabel = std::max(yLabel, prevLabelBounds.bottom());
                }
            }
            okBounds = !intersects;
        }

        if (line) {
            y2 = (int) (yLabel - size / 2);
            canvas.move_to(x1,y1);
            canvas.line_to(x2,y2);
            canvas.line_to(x2 + extra, y2);
            canvas.stroke();
        } else {
            paint.setTextAlign(Align::CENTER);
        }
        //canvas.drawText(labelText, xLabel, yLabel, paint);
        switch(paint.textAlign){
        case Align::LEFT  : canvas.move_to(xLabel, yLabel);break;
        case Align::CENTER: canvas.move_to(xLabel-te.width/2.0,yLabel);break;
        case Align::RIGHT : canvas.move_to(xLabel-te.width,yLabel);break;
        }
        //canvas.move_to(xLabel, yLabel);
        canvas.show_text(labelText);
        if (line) {
            prevLabelsBounds.push_back({xLabel, yLabel, widthLabel, size});
        }
    }
}

bool AbstractChart::isNullValue(double value) const{
    return std::isnan(value) || std::isinf(value) || value == MathHelper::NULL_VALUE;
}

SeriesSelection* AbstractChart::getSeriesAndPointForScreenCoordinate(const PointF& screenPoint) const{
    return nullptr;
}

}
