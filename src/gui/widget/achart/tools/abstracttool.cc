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
#include <widget/achart/touchhandler.h>
#include <widget/achart/graphicalview.h>
namespace cdroid{
TouchHandler::AbstractTool::AbstractTool(GraphicalView* graphicalView)
{//: mGraphicalView(graphicalView) {
    mChart = graphicalView->getChart();
    if (dynamic_cast<XYChart*>(mChart)) {
        mRenderer = ((XYChart*) mChart)->getRenderer();
    }
}

TouchHandler::AbstractTool::AbstractTool(AbstractChart* chart) {
    mChart = chart;
    if (dynamic_cast<XYChart*>(chart)) {
        mRenderer = ((XYChart*) chart)->getRenderer();
    }
}


int TouchHandler::AbstractTool::dpToPx(int dp) const{
    //DisplayMetrics displayMetrics = mContext.getResources().getDisplayMetrics();
    return dp;//(int) ((dp * displayMetrics.density) + 0.5);
}

/**
 * Returns the current chart range.
 *
 * @param scale the scale
 * @return the chart range
 */
std::vector<double> TouchHandler::AbstractTool::getRange(int scale) const{
    
    double minX = mRenderer->getXAxisMin(scale);
    double maxX = mRenderer->getXAxisMax(scale);
    double minY = mRenderer->getYAxisMin(scale);
    double maxY = mRenderer->getYAxisMax(scale);
    return { minX, maxX, minY, maxY };
}

/**
 * Sets the range to the calculated one, if not already set.
 *
 * @param range the range
 * @param scale the scale
 */
void TouchHandler::AbstractTool::checkRange(std::vector<double>& range, int scale) {
    if (dynamic_cast<XYChart*>(mChart)) {
        std::vector<double> calcRange = ((XYChart*) mChart)->getCalcRange(scale);
        if (calcRange.size()) {
            if (!mRenderer->isMinXSet(scale)) {
                range[0] = calcRange[0];
                mRenderer->setXAxisMin(range[0], scale);
            }
            if (!mRenderer->isMaxXSet(scale)) {
                range[1] = calcRange[1];
                mRenderer->setXAxisMax(range[1], scale);
            }
            if (!mRenderer->isMinYSet(scale)) {
                range[2] = calcRange[2];
                mRenderer->setYAxisMin(range[2], scale);
            }
            if (!mRenderer->isMaxYSet(scale)) {
                range[3] = calcRange[3];
                mRenderer->setYAxisMax(range[3], scale);
            }
        }
    }
}
/**
 * Sets a new range on the X axis.
 *
 * @param min the minimum value
 * @param max the maximum value
 * @param scale the scale
 */
void TouchHandler::AbstractTool::setXRange(double min, double max, int scale) {
    mRenderer->setXAxisMin(min, scale);
    mRenderer->setXAxisMax(max, scale);
}

/**
 * Sets a new range on the Y axis.
 *
 * @param min the minimum value
 * @param max the maximum value
 * @param scale the scale
 */
void TouchHandler::AbstractTool::setYRange(double min, double max, int scale) {
    mRenderer->setYAxisMin(min, scale);
    mRenderer->setYAxisMax(max, scale);
}

}/*endof namespace*/
