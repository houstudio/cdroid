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
#ifndef __ACHART_TOOL_ZOOM_H__
#define __ACHART_TOOL_ZOOM_H__
#include <widget/achart/touchhandler.h>
namespace cdroid{
TouchHandler::Zoom::Zoom(AbstractChart* chart, bool in, float rate):AbstractTool(chart){
    mZoomIn = in;
    setZoomRate(rate);
}

void TouchHandler::Zoom::setZoomRate(float rate) {
    mZoomRate = rate;
}

void TouchHandler::Zoom::apply(int zoom_axis) {
    if (dynamic_cast<XYChart*>(mChart)) {
        int scales = mRenderer->getScalesCount();
        for (int i = 0; i < scales; i++) {
            auto range = getRange(i);
            checkRange(range, i);
            std::vector<double> limits = mRenderer->getZoomLimits();

            double centerX = (range[0] + range[1]) / 2;
            double centerY = (range[2] + range[3]) / 2;
            double newWidth = range[1] - range[0];
            double newHeight = range[3] - range[2];
            double newXMin = centerX - newWidth / 2;
            double newXMax = centerX + newWidth / 2;
            double newYMin = centerY - newHeight / 2;
            double newYMax = centerY + newHeight / 2;

            // if already reached last zoom, then it will always set to reached
            if (i == 0) {
                limitsReachedX = limits.size() && (newXMin <= limits[0] || newXMax >= limits[1]);
                limitsReachedY = limits.size() && (newYMin <= limits[2] || newYMax >= limits[3]);
            }

            if (mZoomIn) {
                if (mRenderer->isZoomXEnabled() && (zoom_axis == ZOOM_AXIS_X || zoom_axis == ZOOM_AXIS_XY)) {
                    if (limitsReachedX && mZoomRate < 1) {
                        // ignore pinch zoom out once reached X limit
                    } else {
                        newWidth /= mZoomRate;
                    }
                }

                if (mRenderer->isZoomYEnabled() && (zoom_axis == ZOOM_AXIS_Y || zoom_axis == ZOOM_AXIS_XY)) {
                    if (limitsReachedY && mZoomRate < 1) {
                    } else {
                        newHeight /= mZoomRate;
                    }
                }
            } else {
                if (mRenderer->isZoomXEnabled() && !limitsReachedX
                        && (zoom_axis == ZOOM_AXIS_X || zoom_axis == ZOOM_AXIS_XY)) {
                    newWidth *= mZoomRate;
                }

                if (mRenderer->isZoomYEnabled() && !limitsReachedY
                        && (zoom_axis == ZOOM_AXIS_Y || zoom_axis == ZOOM_AXIS_XY)) {
                    newHeight *= mZoomRate;
                }
            }

            double minX, minY;
            if (limits.size()) {
                minX = std::min(mRenderer->getZoomInLimitX(), limits[1] - limits[0]);
                minY = std::min(mRenderer->getZoomInLimitY(), limits[3] - limits[2]);
            } else {
                minX = mRenderer->getZoomInLimitX();
                minY = mRenderer->getZoomInLimitY();
            }
            newWidth = std::max(newWidth, minX);
            newHeight = std::max(newHeight, minY);

            if (mRenderer->isZoomXEnabled() && (zoom_axis == ZOOM_AXIS_X || zoom_axis == ZOOM_AXIS_XY)) {
                newXMin = centerX - newWidth / 2;
                newXMax = centerX + newWidth / 2;
                setXRange(newXMin, newXMax, i);
            }
            if (mRenderer->isZoomYEnabled() && (zoom_axis == ZOOM_AXIS_Y || zoom_axis == ZOOM_AXIS_XY)) {
                newYMin = centerY - newHeight / 2;
                newYMax = centerY + newHeight / 2;
                setYRange(newYMin, newYMax, i);
            }
        }
    } else {
        auto renderer = ((RoundChart*) mChart)->getRenderer();
        if (mZoomIn) {
            renderer->setScale(renderer->getScale() * mZoomRate);
        } else {
            renderer->setScale(renderer->getScale() / mZoomRate);
        }
    }
    notifyZoomListeners(ZoomEvent(mZoomIn, mZoomRate));
}

void TouchHandler::Zoom::addZoomListener(const ZoomListener& listener) {
    mZoomListeners.push_back(listener);
}

/**
 * Removes a zoom listener.
 *
 * @param listener zoom listener
 */
void TouchHandler::Zoom::removeZoomListener(const ZoomListener& listener) {
    auto it =std::find(mZoomListeners.begin(),mZoomListeners.end(),listener);
    if(it!=mZoomListeners.end())mZoomListeners.erase(it);
}

void TouchHandler::Zoom::notifyZoomListeners(const ZoomEvent& e) {
    for (ZoomListener& listener : mZoomListeners) {
        listener.zoomApplied(e);
    }
}

void TouchHandler::Zoom::notifyZoomResetListeners() {
    for (ZoomListener& listener : mZoomListeners) {
        listener.zoomReset();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

TouchHandler::FitZoom::FitZoom(AbstractChart* chart):AbstractTool(chart){
}

/**
 * Apply the tool.
 */
void TouchHandler::FitZoom::apply() {
    if (dynamic_cast<XYChart*>(mChart)) {
        if (((XYChart*) mChart)->getDataset() == nullptr) {
            return;
        }
        int scales = mRenderer->getScalesCount();
        if (mRenderer->isInitialRangeSet()) {
            for (int i = 0; i < scales; i++) {
                if (mRenderer->isInitialRangeSet(i)) {
                    mRenderer->setRange(mRenderer->getInitialRange(i), i);
                }
            }
        } else {
            auto series = ((XYChart*) mChart)->getDataset()->getSeries();
            //double[] range = null;
            const auto length = series.size();
            if (length > 0) {
                for (int i = 0; i < scales; i++) {
                    std::vector<double>range = { MathHelper::NULL_VALUE, -MathHelper::NULL_VALUE,
                                           MathHelper::NULL_VALUE, -MathHelper::NULL_VALUE
                                         };
                    for (int j = 0; j < length; j++) {
                        if (i == series[j]->getScaleNumber()) {
                            range[0] = std::min(range[0], series[j]->getMinX());
                            range[1] = std::max(range[1], series[j]->getMaxX());
                            range[2] = std::min(range[2], series[j]->getMinY());
                            range[3] = std::max(range[3], series[j]->getMaxY());
                        }
                    }
                    double marginX = std::abs(range[1] - range[0]) / 40;
                    double marginY = std::abs(range[3] - range[2]) / 40;
                    mRenderer->setRange({ range[0] - marginX, range[1] + marginX,range[2] - marginY, range[3] + marginY}, i);
                }
            }
        }
    } else {
        auto renderer = ((RoundChart*) mChart)->getRenderer();
        renderer->setScale(renderer->getOriginalScale());
    }
}

}/*endof namespace*/
#endif/*__ACHART_TOOL_ZOOM_H__*/
