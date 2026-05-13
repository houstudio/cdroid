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
#ifndef __WIND_ROSE_CHART_H__
#define __WIND_ROSE_CHART_H__

#include <map>
#include <widget/achart/chart/doughnutchart.h>

namespace cdroid{
/**
 * The wind rose chart rendering class.
 */
class RoseChart : public DoughnutChart {
private:
    int mInnerColor;
    int mIntervalAngle;
    int mBgLineColor;
    int mBgLines;

    bool mShowInner;
    bool mInnerStroke;
    bool mShowBgLines;
    bool mShowBgCircle;
    bool mShowOuterLabels;

    std::map<float, int> mBgSegments;
private:
    void drawBgCircles(Canvas& canvas, float radius);
    void drawBgLines(Canvas& canvas, float radius, int sectorCount);
public:
    RoseChart(const std::shared_ptr<MultipleCategorySeries>& dataset, const std::shared_ptr<DefaultRenderer>& renderer);

    void draw(Canvas& canvas, int x, int y, int width, int height, Paint& paint) override;

    void setIntervalAngle(int angle);
    void showInner();
    void hideInner();
    void setInnerColor(int color);
    void setInnerStroke(bool stroke);
    void showOuterLabels();
    void hideOuterLabels();
    void showBgLines(int color);
    void hideBgLines();
    void showBgCircle(const std::map<float, int>& bgSegments);
    void hideBgCircle();
    void setBgLines(int count);
};
}/*endof namespace*/

#endif/*__WIND_ROSE_CHART_H__*/

