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
#ifndef __DIAL_CHART_H__
#define __DIAL_CHART_H__
#include <widget/achart/chart/roundchart.h>
#include <widget/achart/renderer/dialrenderer.h>

namespace cdroid{
/**
 * The dial chart rendering class.
 */
class DialChart :public RoundChart {
private:
    /** The radius of the needle. */
    static constexpr int NEEDLE_RADIUS = 10;
    /** The series renderer. */
    std::shared_ptr<DialRenderer> mRenderer;
private:
    double getAngleForValue(double value, double minAngle, double maxAngle, double min,double max)const;
    void drawTicks(Canvas& canvas, double min, double max, double minAngle, double maxAngle,int centerX, int centerY,
            double longRadius, double shortRadius, double ticks,  Paint& paint,bool labels);

    void drawNeedle(Canvas& canvas, double angle, int centerX, int centerY, double radius,bool arrow,  Paint& paint);
public:
    DialChart(const std::shared_ptr<CategorySeries>& dataset, const std::shared_ptr<DialRenderer>& renderer);

    void draw(Canvas& canvas, int x, int y, int width, int height,  Paint& paint) override;
};
}/*endof namespace*/
#endif/*__DIAL_CHART_H__*/
