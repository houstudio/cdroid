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
namespace cdroid{
  
double TouchHandler::Pan::getAxisRatio(const std::vector<double>& range) const{
    return std::abs(range[1] - range[0]) / std::abs(range[3] - range[2]);
}
void TouchHandler::Pan::notifyPanListeners() {
    for (PanListener& listener : mPanListeners) {
        listener();//.panApplied();
    }
}
/**
 * Builds and instance of the pan tool.
 *
 * @param chart the XY chart
 */
TouchHandler::Pan::Pan(AbstractChart* chart):AbstractTool(chart){
}

/**
 * Apply the tool.
 *
 * @param oldX the previous location on X axis
 * @param oldY the previous location on Y axis
 * @param newX the current location on X axis
 * @param newY the current location on the Y axis
 */
bool TouchHandler::Pan::apply(float oldX, float oldY, float newX, float newY) {
    bool notLimitedUp = true;
    bool notLimitedBottom = true;
    bool notLimitedLeft = true;
    bool notLimitedRight = true;
    bool changed = false;
    XYChart*xyChart= dynamic_cast<XYChart*>(mChart);
    if (xyChart) {
        //auto renderer = std::dynamic_pointer_cast<XYMultipleSeriesRenderer>(mRenderer);
        const int scales = mRenderer->getScalesCount();
        std::vector<double> limits = mRenderer->getPanLimits();
        bool limited = /*limits != null &&*/ limits.size() == 4;
        XYChart* chart = (XYChart*) mChart;
        for (int scale = 0; scale < scales; scale++) {
            auto range = getRange(scale);
            auto calcRange = chart->getCalcRange(scale);
            auto oldPoint = xyChart->toRealPoint(oldX,oldY,scale);
            auto newPoint = xyChart->toRealPoint(newX,newY,scale);
            checkRange(range, scale);
            if (range.size()<4||oldPoint.size()<2||newPoint.size()<2){
                continue;
            }

            double deltaX = oldPoint[0] - newPoint[0];
            double deltaY = oldPoint[1] - newPoint[1];
            double ratio = getAxisRatio(range);
            if (xyChart->isVertical(mRenderer)) {
                const double width = std::abs(range[1] - range[0]);
                const double height = std::abs(range[3] - range[2]);
                const double ratio = width / height;
                double newDeltaX = -deltaY * ratio;
                double newDeltaY = deltaX / ratio;
                deltaX = newDeltaX;
                deltaY = newDeltaY;
            }
            if (mRenderer->isPanXEnabled()) {
                if (!limits.empty()) {
                    if (notLimitedLeft) {
                        notLimitedLeft = limits[0] <= range[0] + deltaX;
                    }
                    if (notLimitedRight) {
                        notLimitedRight = limits[1] >= range[1] + deltaX;
                    }
                }
                if (!limited || (notLimitedLeft && notLimitedRight)) {
                    setXRange(range[0] + deltaX, range[1] + deltaX, scale);
                    limitsReachedX = false;
                } else {
                    limitsReachedX = true;
                }
                changed = true;
            }
            if (mRenderer->isPanYEnabled()) {
                if (!limits.empty()) {
                    if (notLimitedBottom) {
                        notLimitedBottom = limits[2] <= range[2] + deltaY;
                    }
                    if (notLimitedUp) {
                        notLimitedUp = limits[3] >= range[3] + deltaY;
                    }
                }
                if (!limited || (notLimitedBottom && notLimitedUp)) {
                    setYRange(range[2] + deltaY, range[3] + deltaY, scale);
                    limitsReachedY = false;
                } else {
                    limitsReachedY = true;
                }
            }
        }
        return changed;
    } else {
        RoundChart* chart = (RoundChart*) mChart;
        chart->setCenterX(chart->getCenterX() + (int) (newX - oldX));
        chart->setCenterY(chart->getCenterY() + (int) (newY - oldY));
        notifyPanListeners();
    }
    return false;
}

void TouchHandler::Pan::addPanListener(const PanListener& listener) {
    mPanListeners.push_back(listener);
}

void TouchHandler::Pan::removePanListener(const PanListener& listener) {
    auto it =std::find(mPanListeners.begin(),mPanListeners.end(),listener);
    if(it!=mPanListeners.end()){
        mPanListeners.erase(it);
    }
}

}/*endof namespace*/
