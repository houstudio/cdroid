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
#ifndef __ABSTRACT_CHART_H__
#define __ABSTRACT_CHART_H__
#include <string>
#include <vector>
#include <map>
#include <core/rect.h>
#include <core/canvas.h>
#include <core/numberformat.h>
#include <widget/achart/model/seriesselection.h>
#include <widget/achart/renderer/defaultrenderer.h>
#include <widget/achart/renderer/simpleseriesrenderer.h>
namespace cdroid{
class AbstractChart {
public:
    struct Paint{
        int color;
        int style;
        int textAlign;
        Paint(){
            color=0;
            textAlign=0;
            style=0;
        }
        void setColor(int c){color=c;}
        void setTextAlign(int a){textAlign=a;}
        void setStyle(int s){style=s;}
    };
protected:
    enum Style{STROKE=1,FILL=2};
    enum Align{LEFT,CENTER,RIGHT};
    int m_width;
    int m_height;
private:
    static std::vector<float> calculateDrawPoints(float p1x, float p1y, float p2x, float p2y,int screenHeight, int screenWidth);
    std::string getFitText(const std::string& text, float width,  Paint& paint)const;
protected:
    void drawBackground(const std::shared_ptr<DefaultRenderer>& renderer, Canvas& canvas,
            int x, int y, int width,int height, Paint& paint, bool newColor, int color);
    int drawLegend(Canvas& canvas, const std::shared_ptr<DefaultRenderer>& renderer, const std::vector<std::string>& titles,
             int left,int right,int y, int width, int height, int legendSize,  Paint& paint, bool calculate);

    void drawString(Canvas& canvas,const std::string& text, float x, float y,  Paint& paint);
    bool getExceed(float currentWidth, const std::shared_ptr<DefaultRenderer>& renderer, int right, int width);

    std::string getLabel(const NumberFormat* format, double label)const;

    virtual void drawPath(Canvas& canvas,const std::vector<float>& points, Paint& paint, bool circular);

    int getLegendSize(const std::shared_ptr<DefaultRenderer>& renderer, int defaultHeight, float extraHeight);

    virtual void drawLabel(Canvas& canvas,const std::string& labelText, const std::shared_ptr<DefaultRenderer>& renderer,
            std::vector<RectF>& prevLabelsBounds, int centerX, int centerY, float shortRadius, float longRadius,
            float currentAngle, float angle, int left, int right, int color, Paint& paint, bool line,bool display);
public:
    virtual ~AbstractChart()=default;
    void setSize(int32_t width,int32_t height);
    virtual void draw(Canvas& canvas, int x, int y, int width, int height,  Paint& paint)=0;

    bool isVertical(const std::shared_ptr<DefaultRenderer>& renderer)const;

    virtual int getLegendShapeWidth(int seriesIndex)const=0;

    virtual void drawLegendShape(Canvas& canvas, const std::shared_ptr<SimpleSeriesRenderer>& renderer,
            float x,float y, int seriesIndex, Paint& paint)=0;

    bool isNullValue(double value) const;

    virtual SeriesSelection* getSeriesAndPointForScreenCoordinate(const PointF& screenPoint)const;
};
}/*endof namespace*/
#endif/*__ABSTRACT_CHART_H__*/
