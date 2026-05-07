#include <widget/achart/chart/combinedxychart.h>
#include <widget/achart/chart/dragcontrolchart.h>
#include <widget/achart/touchhandler.h>
#include <widget/achart/graphicalview.h>
namespace cdroid {

TouchHandler::TouchHandler(GraphicalView* view, AbstractChart* chart) {
    mGraphicalView = view;
    mMove = nullptr;
    mPan = nullptr;
    mPinchZoom = nullptr;
    zoomR = mGraphicalView->getZoomRectangle();
    if (dynamic_cast<XYChart*>(chart)) {
        mRenderer = ((XYChart*) chart)->getRenderer();
    } else {
        mRenderer = ((RoundChart*) chart)->getRenderer();
    }
    if(mRenderer!=nullptr){
        if (mRenderer->isPanEnabled()) {
            mPan = new Pan(chart);
        }
        if (mRenderer->isZoomEnabled()) {
            mPinchZoom = new Zoom(chart, true, 1);
        }
    }
    auto combinedChart = dynamic_cast<CombinedXYChart*>(chart);
    if(combinedChart!=nullptr){
        const std::vector<XYChart*> charts = combinedChart->getCharts();
        for (size_t i = 0; i < charts.size(); ++i) {
            if (dynamic_cast<DragControlChart*>(charts[i]) != nullptr) {
                mMove = new Move(mGraphicalView, static_cast<int>(i));
                LOGD("move=%p",mMove);
                break;
            }
        }
    }
}

/**
 * Handles the touch event.
 *
 * @param event the touch event
 * @return  handleTouch
 */
bool TouchHandler::handleTouch(MotionEvent& event) {
#if 10
    const int action = event.getAction();
    //if(mMove!=nullptr)return handleMoveEvent(event);
    AbstractChart* chart = mGraphicalView->getChart();
    if ((mRenderer != nullptr) && (action == MotionEvent::ACTION_MOVE)) {
        if (mLastX >= 0 || mLastY >= 0) {
            const float newX = event.getX(0);
            const float newY = event.getY(0);
            if (event.getPointerCount() > 1 && (mLastX2 >= 0 || mLastY2 >= 0) && mRenderer->isZoomEnabled()) {
                const float newX2 = event.getX(1);
                const float newY2 = event.getY(1);
                const float newDeltaX = std::abs(newX - newX2);
                const float newDeltaY = std::abs(newY - newY2);
                const float oldDeltaX = std::abs(mLastX - mLastX2);
                const float oldDeltaY = std::abs(mLastY - mLastY2);
                const float tan1 = std::abs(newY-mLastY)/std::abs(newX-mLastX);
                const float tan2 = std::abs(newY2-mLastY2)/std::abs(newX2-mLastX2);
                if(oldDeltaX>0.f||oldDeltaY>0.f){
                    float zoomRate = 1.0f;
                    if (tan1 <= 0.25f && tan2 <= 0.25f && oldDeltaX > 0.0f) {
                        zoomRate = newDeltaX / oldDeltaX;
                        applyZoom(zoomRate, Zoom::ZOOM_AXIS_X);
                    } else if (tan1 >= 3.73f && tan2 >= 3.73f && oldDeltaY > 0.0f) {
                        zoomRate = newDeltaY / oldDeltaY;
                        applyZoom(zoomRate,Zoom::ZOOM_AXIS_Y);
                    } else {
                        if (std::abs(newX-mLastX)>std::abs(newY-mLastY))
                            zoomRate=newDeltaX / oldDeltaX;
                        else
                            zoomRate =newDeltaY / oldDeltaY;
                        applyZoom(zoomRate,Zoom::ZOOM_AXIS_XY);
                    }
                }
                mLastX2 = newX2;
                mLastY2 = newY2;
            } else if (mPan!=nullptr||mRenderer->isPanEnabled()) {
                mPan->apply(mLastX, mLastY, newX, newY);
                mLastX2 = 0;
                mLastY2 = 0;
                mLastX = newX;
                mLastY = newY;
                mGraphicalView->invalidate();
                return true;
            }
            mLastX=newX;
            mLastY=newY;
            mGraphicalView->invalidate();
            return true;
        }
    } else if (action == MotionEvent::ACTION_DOWN) {
        mLastX = event.getX();
        mLastY = event.getY();
        if(mRenderer->isZoomEnabled()){
            //mGraphicalView->zoomIn();
        }
        //return false;
    } else if (action == MotionEvent::ACTION_UP){// || action == MotionEvent::ACTION_POINTER_UP) {
        mLastX = 0;
        mLastY = 0;
        mLastX2 = -1.f;
        mLastY2 = -1.f;
    }else if (action == MotionEvent::ACTION_POINTER_UP) {
        mLastX = -1;
        mLastY = -1;
        //return false;
    }else if(action == MotionEvent::ACTION_CANCEL){
        mLastX = mLastY =0.f;
        mLastX2 = mLastY2= -1.f;
        return false;
    }
    if (/*action == MotionEvent::ACTION_UP &&*/ chart != nullptr) {
        SeriesSelection* selection = chart->getSeriesAndPointForScreenCoordinate({event.getX(), event.getY()});
        LOGD("selection=%p ,selction:%d/%d",selection,(selection?selection->getSeriesIndex():-1),(selection?selection->getPointIndex():-1));
        if (selection != nullptr) {
            mLastSeriesSelection=selection->getSeriesIndex();
            if (selection->getSeriesIndex()) {//mLastSeriesSelection
                delete selection;
                //chart->setSelection(nullptr);
            } else {
                //chart->setSelection(selection);
                selection = nullptr;
            }
            auto sr=mRenderer->getSeriesRendererAt(mLastSeriesSelection);
            sr->setColor(0xFFFF0000);
            mGraphicalView->invalidate();
            return true;
        }
    }
    return !mRenderer->isClickEnabled();
#else
  if (mMove != nullptr) {
        return handleMoveEvent(event);
    }

    AbstractChart* chart = mGraphicalView->getChart();
    auto renderer = mRenderer;//mGraphicalView->getRenderer();
    const int action = event.getActionMasked();

    if (action == MotionEvent::ACTION_DOWN) {
        mLastX = event.getX();
        mLastY = event.getY();
        mLastX2 = -1.0f;
        mLastY2 = -1.0f;
        mWasPanning = false;
        mWasZooming = false;
        return renderer != nullptr
            && (renderer->isClickEnabled() || renderer->isPanEnabled() || renderer->isZoomEnabled());
    }

    if (renderer != nullptr/* && mGraphicalView->isDrawn()*/) {
        if (action == MotionEvent::ACTION_MOVE && (mLastX >= 0.0f || mLastY >= 0.0f)) {
            const float newX = event.getX(0);
            const float newY = event.getY(0);
            if (event.getPointerCount() > 1 && (mLastX2 >= 0.0f || mLastY2 >= 0.0f) && mPinchZoom != nullptr) {
                const float newX2 = event.getX(1);
                const float newY2 = event.getY(1);
                const float newDeltaX = std::fabs(newX - newX2);
                const float newDeltaY = std::fabs(newY - newY2);
                const float oldDeltaX = std::fabs(mLastX - mLastX2);
                const float oldDeltaY = std::fabs(mLastY - mLastY2);
                if (oldDeltaX > 0.0f || oldDeltaY > 0.0f) {
                    float zoomRate = 1.0f;
                    const float deltaX1 = std::fabs(newX - mLastX);
                    const float deltaY1 = std::fabs(newY - mLastY);
                    const float deltaX2 = std::fabs(newX2 - mLastX2);
                    const float deltaY2 = std::fabs(newY2 - mLastY2);
                    const float tan1 = deltaX1 > 0.0f ? deltaY1 / deltaX1 : std::numeric_limits<float>::infinity();
                    const float tan2 = deltaX2 > 0.0f ? deltaY2 / deltaX2 : std::numeric_limits<float>::infinity();
                    int axis = Zoom::ZOOM_AXIS_XY;

                    if (tan1 <= 0.25f && tan2 <= 0.25f && oldDeltaX > 0.0f) {
                        zoomRate = newDeltaX / oldDeltaX;
                        axis = Zoom::ZOOM_AXIS_X;
                    } else if (tan1 >= 3.73f && tan2 >= 3.73f && oldDeltaY > 0.0f) {
                        zoomRate = newDeltaY / oldDeltaY;
                        axis = Zoom::ZOOM_AXIS_Y;
                    } else if (std::max(oldDeltaX, oldDeltaY) > 0.0f) {
                        zoomRate = oldDeltaX >= oldDeltaY
                            ? (oldDeltaX > 0.0f ? newDeltaX / oldDeltaX : 1.0f)
                            : (oldDeltaY > 0.0f ? newDeltaY / oldDeltaY : 1.0f);
                    }

                    if (1){
                        applyZoom(zoomRate, axis);//, (newX + newX2) * 0.5f, (newY + newY2) * 0.5f)) {
                        mWasZooming = true;
                        mWasPanning = false;
                        mLastX2 = newX2;
                        mLastY2 = newY2;
                        mLastX = newX;
                        mLastY = newY;
                        mGraphicalView->invalidate();
                        return true;
                    }
                }
                mLastX2 = newX2;
                mLastY2 = newY2;
            } else if (mPan != nullptr) {
                if (std::hypot(newX - mLastX, newY - mLastY) < 3.0f) {
                    return false;
                }
                const bool handled = mPan->apply(mLastX, mLastY, newX, newY);
                mLastX2 = -1.0f;
                mLastY2 = -1.0f;
                mLastX = newX;
                mLastY = newY;
                if (handled) {
                    mWasPanning = true;
                    mGraphicalView->invalidate();
                    return true;
                }
            }
        } else if (action == MotionEvent::ACTION_POINTER_DOWN && event.getPointerCount() > 1) {
            mLastX2 = event.getX(1);
            mLastY2 = event.getY(1);
            return false;
        } else if (action == MotionEvent::ACTION_UP) {
            const bool handledGesture = mWasPanning || mWasZooming;
            mWasPanning = false;
            mWasZooming = false;
            mLastX2 = -1.0f;
            mLastY2 = -1.0f;
            if (handledGesture) {
                return true;
            }
        } else if (action == MotionEvent::ACTION_POINTER_UP) {
            mLastX = -1.0f;
            mLastY = -1.0f;
            mLastX2 = -1.0f;
            mLastY2 = -1.0f;
            return mWasZooming;
        }
        if (action == MotionEvent::ACTION_CANCEL) {
            mLastX = 0.0f;
            mLastY = 0.0f;
            mLastX2 = -1.0f;
            mLastY2 = -1.0f;
            mWasPanning = false;
            mWasZooming = false;
            return false;
        }
    }

    if (action == MotionEvent::ACTION_UP && chart != nullptr) {
        SeriesSelection* selection = chart->getSeriesAndPointForScreenCoordinate({event.getX(), event.getY()});
        if (selection != nullptr) {
            /*SeriesSelection* currentSelection = chart->getSelection();
            if (isSameSelection(currentSelection, selection)) {
                delete selection;
                chart->setSelection(nullptr);
            } else {
                chart->setSelection(selection);
                selection = nullptr;
            }*/
            mGraphicalView->invalidate();
            return true;
        }
    }

    return false;
#endif
}

bool TouchHandler::handleMoveEvent(MotionEvent& event) {
    const int action = event.getAction();
    if (action == MotionEvent::ACTION_MOVE&& (mLastX >= 0 || mLastY >= 0)) {
        float newX = event.getX(0);
        float newY = event.getY(0);
        if(mMove!=nullptr){//&&mMove->apply(mLastX, mLastY, newX, newY)){
            mLastX = newX;
            mLastY = newY;
            mGraphicalView->invalidate();
        }
    } else if (action == MotionEvent::ACTION_DOWN) {
        mLastX = event.getX(0);
        mLastY = event.getY(0);
    } else if (action == MotionEvent::ACTION_UP || action == MotionEvent::ACTION_POINTER_UP) {
        if(mMove!=nullptr)mMove->reset();
        mLastX = 0;
        mLastY = 0;
        if (action == MotionEvent::ACTION_POINTER_UP) {
            mLastX = -1;
            mLastY = -1;
        }
    }
    return true;
}

void TouchHandler::applyZoom(float zoomRate, int axis) {
    zoomRate = std::max(zoomRate, 0.9f);
    zoomRate = std::min(zoomRate, 1.1f);
    if (mPinchZoom != nullptr && zoomRate > 0.9 && zoomRate < 1.1) {
        mPinchZoom->setZoomRate(zoomRate);
        mPinchZoom->apply(axis);
        mGraphicalView->invalidate();
    }
}

/**
 * Adds a new zoom listener.
 *
 * @param listener zoom listener
 */
void TouchHandler::addZoomListener(const ZoomListener& listener) {
    if (mPinchZoom != nullptr) {
        mPinchZoom->addZoomListener(listener);
    }
}

/**
 * Removes a zoom listener.
 *
 * @param listener zoom listener
 */
void TouchHandler::removeZoomListener(const ZoomListener& listener) {
    if (mPinchZoom != nullptr) {
        mPinchZoom->removeZoomListener(listener);
    }
}

/**
 * Adds a new pan listener.
 *
 * @param listener pan listener
 */
void TouchHandler::addPanListener(const PanListener& listener) {
    if (mPan != nullptr) {
        mPan->addPanListener(listener);
    }
}

/**
 * Removes a pan listener.
 *
 * @param listener pan listener
 */
void TouchHandler::removePanListener(const PanListener& listener) {
    if (mPan != nullptr) {
        mPan->removePanListener(listener);
    }
}

void TouchHandler::addMoveListener(const MoveListener& listener) {
    if(mMove!=nullptr) {
        mMove->addMoveListener(listener);
    }
}

void TouchHandler::removeMoveListener(const MoveListener& listener) {
    if(mMove!=nullptr) {
        mMove->removeMoveListener(listener);
    }
}

///////////////////////////////////////////////////////////////////////////////////

}/*endof namespace*/
