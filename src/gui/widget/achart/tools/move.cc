/**
 * Copyright (C) 2013 Henning Dodenhof
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
/**
 * The move tool.
 */
void TouchHandler::Move::notifyMoveListeners() {
    for (MoveListener& listener : mMoveListeners) {
        listener/*.moveApplied*/();
    }
}

void TouchHandler::Move::removeMoveListener(const MoveListener& listener) {
    auto it = std::find(mMoveListeners.begin(),mMoveListeners.end(),listener);
    if(it!=mMoveListeners.end()){
        mMoveListeners.erase(it);
    }
}

TouchHandler::Move::Move(GraphicalView*gv,int overlaySeriesIndex):AbstractTool(gv){
    mContext = gv->getContext();
    mOverlaySeriesIndex = overlaySeriesIndex;

    mDragBuffer = dpToPx(16);
}

/**
 * Apply the tool.
 *
 * @param oldX the previous location on X axis
 * @param oldY the previous location on Y axis
 * @param newX the current location on X axis
 * @param newY the current location on the Y axis
 */
void TouchHandler::Move::apply(float oldX, float oldY, float newX, float newY) {
    std::vector<double> limits = mRenderer->getPanLimits();

    CombinedXYChart* chart = (CombinedXYChart*) mChart;
    auto series = chart->getDataset()->getSeriesAt(mOverlaySeriesIndex);

    double oldRealX1 = series->getX(0);
    double oldRealX2 = series->getX(1);

    double oldX1 = chart->toScreenPoint({ oldRealX1, 0 })[0];
    double oldX2 = chart->toScreenPoint({ oldRealX2, 0 })[0];

    double realLimitX1 = limits[0];
    double realLimitX2 = limits[1];

    double limitX1 = chart->toScreenPoint({ realLimitX1, 0 })[0];
    double limitX2 = chart->toScreenPoint({ realLimitX2, 0 })[0];

    double realDist = oldRealX2 - oldRealX1;
    double realHalfdist = realDist / 2;

    double newRealX1 = oldRealX1;
    double newRealX2 = oldRealX2;

    double newRealX = chart->toRealPoint(newX, 0)[0];

    if ((mDraggingLeft || std::abs(oldX - oldX1) < mDragBuffer) && !mMoving) {
        mDraggingLeft = true;

        if (newRealX < oldRealX2 - mRenderer->getZoomInLimitX()  && newX >= limitX1) {
            newRealX1 = newRealX;
            newRealX2 = oldRealX2;
        }
    } else if ((mDraggingRight || std::abs(oldX - oldX2) < mDragBuffer) && !mMoving) {
        mDraggingRight = true;

        if (newRealX > oldRealX1 + mRenderer->getZoomInLimitX() && newX <= limitX2) {
            newRealX1 = oldRealX1;
            newRealX2 = newRealX;
        }
    } else {
        mMoving = true;

        if (newRealX - realHalfdist > realLimitX1 && newRealX + realHalfdist < realLimitX2) {
            newRealX1 = newRealX - realHalfdist;
            newRealX2 = newRealX + realHalfdist;
        } else {
            if (newRealX - realHalfdist < realLimitX1) {
                newRealX1 = realLimitX1;
                newRealX2 = newRealX1 + realDist;
            } else {
                newRealX2 = realLimitX2;
                newRealX1 = newRealX2 - realDist;
            }
        }
    }

    for (int j = series->getItemCount() - 1; j >= 0; j--) {
        series->remove(j);
    }

    series->add(newRealX1, 0);
    series->add(newRealX2, 0);

    notifyMoveListeners();
}

void TouchHandler::Move::reset() {
    mDraggingLeft = false;
    mDraggingRight = false;
    mMoving = false;
}

void TouchHandler::Move::addMoveListener(const MoveListener& listener) {
    mMoveListeners.push_back(listener);
}
}/*endof namespace */
