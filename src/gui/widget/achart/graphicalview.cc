#include <widget/achart/graphicalview.h>
#include <widget/achart/chart/combinedxychart.h>
#include <widget/achart/chart/dragcontrolchart.h>
namespace cdroid {
DECLARE_WIDGET(GraphicalView)
GraphicalView::GraphicalView(Context*ctx,const AttributeSet&attr):View(ctx,attr) {
}

GraphicalView::GraphicalView(Context* context, AbstractChart* chart)
    :View(-1,-1) {
    mChart = chart;
    mZoomRate = 1.25f;
    setClickable(true);
    mDraggingLeft=mDraggingRight=false;
    mMoving = false;
    if (dynamic_cast<XYChart*>(mChart)) {
        mRenderer = ((XYChart*) mChart)->getRenderer();
    } else {
        mRenderer = ((RoundChart*) mChart)->getRenderer();
    }
    /*if (mRenderer->isZoomButtonsVisible()) {
        zoomInImage = BitmapFactory.decodeStream(GraphicalView.class
                      .getResourceAsStream("image/zoom_in.png"));
        zoomOutImage = BitmapFactory.decodeStream(GraphicalView.class
                       .getResourceAsStream("image/zoom_out.png"));
        fitZoomImage = BitmapFactory.decodeStream(GraphicalView.class
                       .getResourceAsStream("image/zoom-1.png"));
    }*/

    if (mRenderer->isZoomEnabled() ){//&& mRenderer->isZoomButtonsVisible()|| mRenderer->isExternalZoomEnabled()){
        mZoomRate = mRenderer->getZoomRate();
    }
    mOverlaySeriesIndex = -1;
    if(dynamic_cast<CombinedXYChart*>(mChart)!=nullptr){
        const std::vector<XYChart*> charts = ((CombinedXYChart*)mChart)->getCharts();
        for (size_t i = 0; i < charts.size(); ++i) {
            if (dynamic_cast<DragControlChart*>(charts[i]) != nullptr) {
                mOverlaySeriesIndex=i;
                LOGD("mOverlaySeriesIndex=%d",mOverlaySeriesIndex);
                break;
            }
        }
    }
}

GraphicalView::~GraphicalView() {
    delete mChart;
}

void GraphicalView::setXRange(double min, double max, int scale) {
    auto renderer = std::dynamic_pointer_cast<XYMultipleSeriesRenderer>(mRenderer);
    renderer->setXAxisMin(min, scale);
    renderer->setXAxisMax(max, scale);
}

void GraphicalView::setYRange(double min, double max, int scale) {
    auto renderer = std::dynamic_pointer_cast<XYMultipleSeriesRenderer>(mRenderer);
    renderer->setYAxisMin(min, scale);
    renderer->setYAxisMax(max, scale);
}

std::vector<double> GraphicalView::getRange(int scale) const {
    auto renderer = std::dynamic_pointer_cast<XYMultipleSeriesRenderer>(mRenderer);
    const double minX = renderer->getXAxisMin(scale);
    const double maxX = renderer->getXAxisMax(scale);
    const double minY = renderer->getYAxisMin(scale);
    const double maxY = renderer->getYAxisMax(scale);
    return { minX, maxX, minY, maxY };
}

void GraphicalView::checkRange(std::vector<double>& range, int scale) {
    if (dynamic_cast<XYChart*>(mChart)) {
        std::vector<double> calcRange = ((XYChart*) mChart)->getCalcRange(scale);
        auto renderer = std::dynamic_pointer_cast<XYMultipleSeriesRenderer>(mRenderer);
        if (calcRange.size()) {
            if (!renderer->isMinXSet(scale)) {
                range[0] = calcRange[0];
                renderer->setXAxisMin(range[0], scale);
            }
            if (!renderer->isMaxXSet(scale)) {
                range[1] = calcRange[1];
                renderer->setXAxisMax(range[1], scale);
            }
            if (!renderer->isMinYSet(scale)) {
                range[2] = calcRange[2];
                renderer->setYAxisMin(range[2], scale);
            }
            if (!renderer->isMaxYSet(scale)) {
                range[3] = calcRange[3];
                renderer->setYAxisMax(range[3], scale);
            }
        }
    }
}

void GraphicalView::zoom(int zoomAxis,float zoomRate,bool zoomIn) {
    if (dynamic_cast<XYChart*>(mChart)) {
        auto renderer = std::dynamic_pointer_cast<XYMultipleSeriesRenderer>(mRenderer);
        const int scales = renderer->getScalesCount();
        bool limitsReachedX=false,limitsReachedY=false;
        for (int i = 0; i < scales; i++) {
            auto range = getRange(i);
            checkRange(range, i);
            std::vector<double> limits = renderer->getZoomLimits();

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

            if (zoomIn) {
                if (renderer->isZoomXEnabled() && (zoomAxis&ZOOM_AXIS_X)) {
                    if (limitsReachedX && zoomRate < 1) {
                        // ignore pinch zoom out once reached X limit
                    } else {
                        newWidth /= zoomRate;
                    }
                }
                if (renderer->isZoomYEnabled() && (zoomAxis&ZOOM_AXIS_Y)) {
                    if (limitsReachedY && zoomRate < 1) {
                    } else {
                        newHeight /= zoomRate;
                    }
                }
            } else {
                if (renderer->isZoomXEnabled() && !limitsReachedX && (zoomAxis&ZOOM_AXIS_X)) {
                    newWidth *= zoomRate;
                }
                if (renderer->isZoomYEnabled() && !limitsReachedY && (zoomAxis&ZOOM_AXIS_Y)) {
                    newHeight *= zoomRate;
                }
            }

            double minX, minY;
            if (limits.size()) {
                minX = std::min(renderer->getZoomInLimitX(), limits[1] - limits[0]);
                minY = std::min(renderer->getZoomInLimitY(), limits[3] - limits[2]);
            } else {
                minX = renderer->getZoomInLimitX();
                minY = renderer->getZoomInLimitY();
            }
            newWidth = std::max(newWidth, minX);
            newHeight = std::max(newHeight, minY);

            if (renderer->isZoomXEnabled() && (zoomAxis&ZOOM_AXIS_X)) {
                newXMin = centerX - newWidth / 2;
                newXMax = centerX + newWidth / 2;
                setXRange(newXMin, newXMax, i);
            }
            if (renderer->isZoomYEnabled() && (zoomAxis&ZOOM_AXIS_Y)) {
                newYMin = centerY - newHeight / 2;
                newYMax = centerY + newHeight / 2;
                setYRange(newYMin, newYMax, i);
            }
        }
    } else {
        auto renderer = ((RoundChart*) mChart)->getRenderer();
        if (zoomIn) {
            renderer->setScale(renderer->getScale() * zoomRate);
        } else {
            renderer->setScale(renderer->getScale() / zoomRate);
        }
    }
    invalidate();
    notifyZoomListeners(zoomRate, zoomIn);
}
 
double GraphicalView::getAxisRatio(std::vector<double>& range) const {
    return std::abs(range[1] - range[0]) / std::abs(range[3] - range[2]);
}

void GraphicalView::pan(float oldX, float oldY, float newX, float newY) {
    bool notLimitedUp = true;
    bool notLimitedBottom = true;
    bool notLimitedLeft = true;
    bool notLimitedRight = true;
    bool changed = false;
    bool limitsReachedX = false;
    bool limitsReachedY = false;
    XYChart*xyChart= dynamic_cast<XYChart*>(mChart);
    if (xyChart) {
        auto renderer = std::dynamic_pointer_cast<XYMultipleSeriesRenderer>(mRenderer);
        const int scales = renderer->getScalesCount();
        std::vector<double> limits = renderer->getPanLimits();
        const bool limited = limits.size() == 4;
        XYChart* chart = (XYChart*) mChart;
        for (int scale = 0; scale < scales; scale++) {
            auto range = getRange(scale);
            auto calcRange = chart->getCalcRange(scale);
            auto oldPoint = xyChart->toRealPoint(oldX,oldY,scale);
            auto newPoint = xyChart->toRealPoint(newX,newY,scale);
            checkRange(range, scale);
            if (range.size()<4||oldPoint.size()<2||newPoint.size()<2) {
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
            if (renderer->isPanXEnabled()) {
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
            if (renderer->isPanYEnabled()) {
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
        return ;//changed;
    } else {
        RoundChart* chart = (RoundChart*) mChart;
        chart->setCenterX(chart->getCenterX() + (int) (newX - oldX));
        chart->setCenterY(chart->getCenterY() + (int) (newY - oldY));
        notifyPanListeners();
    }
    return;
}

bool GraphicalView::move(float oldX, float oldY, float newX, float newY) {
    auto* chart = dynamic_cast<CombinedXYChart*>(mChart);
    auto renderer = std::dynamic_pointer_cast<XYMultipleSeriesRenderer>(mRenderer);
    if ( (renderer == nullptr) || (chart == nullptr) ) {
        return false;
    }

    auto dataset = chart->getDataset();
    const std::vector<double> limits = renderer->getPanLimits();
    if ( (dataset == nullptr) || (mOverlaySeriesIndex < 0) || (limits.size() < 2)
            || (mOverlaySeriesIndex >= dataset->getSeriesCount()) ) {
        return false;
    }

    auto series = dataset->getSeriesAt(mOverlaySeriesIndex);
    if ((series == nullptr )|| (series->getItemCount() < 2)) {
        return false;
    }

    const double oldRealX1 = series->getX(0);
    const double oldRealX2 = series->getX(1);
    const std::vector<double> oldPoint1 = chart->toScreenPoint({oldRealX1, 0.0});
    const std::vector<double> oldPoint2 = chart->toScreenPoint({oldRealX2, 0.0});
    const std::vector<double> limitPoint1 = chart->toScreenPoint({limits[0], 0.0});
    const std::vector<double> limitPoint2 = chart->toScreenPoint({limits[1], 0.0});
    const std::vector<double> realPoint = chart->toRealPoint(newX, 0.0f);

    const double oldScreenX1 = oldPoint1[0];
    const double oldScreenX2 = oldPoint2[0];
    const double limitScreenX1 = limitPoint1[0];
    const double limitScreenX2 = limitPoint2[0];
    const double realLimitX1 = limits[0];
    const double realLimitX2 = limits[1];
    const double realDistance = oldRealX2 - oldRealX1;
    const double realHalfDistance = realDistance / 2.0;
    const double newRealX = realPoint[0];
    double newRealX1 = oldRealX1;
    double newRealX2 = oldRealX2;
    const double mDragBuffer = 16.0;
    if ((mDraggingLeft || std::fabs(oldX - oldScreenX1) < mDragBuffer) && !mMoving) {
        mDraggingLeft = true;
        if (newRealX < oldRealX2 - renderer->getZoomInLimitX() && newX >= limitScreenX1) {
            newRealX1 = newRealX;
        }
    } else if ((mDraggingRight || std::fabs(oldX - oldScreenX2) < mDragBuffer) && !mMoving) {
        mDraggingRight = true;
        if (newRealX > oldRealX1 + renderer->getZoomInLimitX() && newX <= limitScreenX2) {
            newRealX2 = newRealX;
        }
    } else if(oldX>oldScreenX1&&oldX<oldScreenX2){
        mMoving = true;
        if (newRealX - realHalfDistance > realLimitX1 && newRealX + realHalfDistance < realLimitX2) {
            newRealX1 = newRealX - realHalfDistance;
            newRealX2 = newRealX + realHalfDistance;
        } else if (newRealX - realHalfDistance <= realLimitX1) {
            newRealX1 = realLimitX1;
            newRealX2 = newRealX1 + realDistance;
        } else {
            newRealX2 = realLimitX2;
            newRealX1 = newRealX2 - realDistance;
        }
    }else{
        return false;
    }
    for (int i = series->getItemCount() - 1; i >= 0; --i) {
        series->remove(i);
    }
    invalidate();
    series->add(newRealX1, 0.0);
    series->add(newRealX2, 0.0);
    //LOGD("(%.2f,%.2f)",newRealX1,newRealX2);
    for(auto l:mListeners){
        if(l.onMoved){
            l.onMoved(*this,oldRealX1,oldRealX2,newRealX1,newRealX2);
        }
    }
    return true;
}

std::vector<double> GraphicalView::toRealPoint(int scale) {
    if (dynamic_cast<XYChart*>(mChart)) {
        return ((XYChart*) mChart)->toRealPoint(oldX, oldY, scale);
    }
    return {};
}

void GraphicalView::onDraw(Canvas& canvas) {
    View::onDraw(canvas);
    const int left =getPaddingLeft();
    const int top = getPaddingTop();
    const int right =getPaddingRight();
    const int bottom=getPaddingBottom();
    const int width = getWidth()-left-right;
    const int height = getHeight()-top-bottom;
    AbstractChart::Paint mPaint;
    mChart->draw(canvas, left,top, width, height, mPaint);
    if (mRenderer != nullptr && mRenderer->isZoomEnabled() /*&& mRenderer->isZoomButtonsVisible()*/) {
    }
    mDrawn = true;
}

void GraphicalView::setZoomRate(float rate) {
    mZoomRate = rate;
}

float GraphicalView::getZoomRate()const{
    return mZoomRate;
}

void GraphicalView::zoomIn() {
    zoom(ZOOM_AXIS_XY,mZoomRate,true);
}

void GraphicalView::zoomOut() {
    zoom(ZOOM_AXIS_XY,mZoomRate,false);
}

void GraphicalView::zoomReset() {
    if (dynamic_cast<XYChart*>(mChart)) {
        if (((XYChart*) mChart)->getDataset() == nullptr) {
            return;
        }
        auto renderer = std::dynamic_pointer_cast<XYMultipleSeriesRenderer>(mRenderer);
        const int scales = renderer->getScalesCount();
        if (renderer->isInitialRangeSet()) {
            for (int i = 0; i < scales; i++) {
               if (renderer->isInitialRangeSet(i)) {
                    renderer->setRange(renderer->getInitialRange(i), i);
               }
            }
        } else {
            auto series = ((XYChart*) mChart)->getDataset()->getSeries();
            std::vector<double> range;
            const int length = series.size();
            if (length > 0) {
                 for (int i = 0; i < scales; i++) {
                     range = { MathHelper::NULL_VALUE, -MathHelper::NULL_VALUE,
                         MathHelper::NULL_VALUE, -MathHelper::NULL_VALUE };
                     for (int j = 0; j < length; j++) {
                         if (i == series[j]->getScaleNumber()) {
                              range[0] = std::min(range[0], series[j]->getMinX());
                              range[1] = std::max(range[1], series[j]->getMaxX());
                              range[2] = std::min(range[2], series[j]->getMinY());
                              range[3] = std::max(range[3], series[j]->getMaxY());
                         }
                     }
                     const double marginX = std::abs(range[1] - range[0]) / 40;
                     const double marginY = std::abs(range[3] - range[2]) / 40;
                     renderer->setRange({ range[0] - marginX, range[1] + marginX,
                         range[2] - marginY, range[3] + marginY }, i);
                 }
            }
        }
    } else {
        mRenderer->setScale(mRenderer->getOriginalScale());
    }
    notifyZoomResetListeners();
    invalidate();
}

void GraphicalView::addChartListener(const ChartListener& listener) {
    auto it = std::find(mListeners.begin(),mListeners.end(),listener);
    if(it == mListeners.end()){
        mListeners.push_back(listener);
    }
}

void GraphicalView::removeChartListener(const ChartListener& listener) {
    auto it = std::find(mListeners.begin(),mListeners.end(),listener);
    if(it != mListeners.end()){
        mListeners.erase(it);
    }
}

void GraphicalView::notifyPanListeners(){
    for (ChartListener& listener : mListeners) {
        if(listener.onPanned){
            listener.onPanned(*this);
        }
    }
}

void GraphicalView::notifyZoomListeners(float zoomRate,bool zoomIn) {
    for (ChartListener& listener : mListeners) {
        if(listener.onZoom){
            listener.onZoom(*this,zoomRate,zoomIn);
        }
    }
}

void GraphicalView::notifyZoomResetListeners() {
    for (ChartListener& listener : mListeners) {
        if(listener.onZoomReset){
            listener.onZoomReset(*this);
        }
    }
}

RectF GraphicalView::getZoomRectangle() const {
    return mZoomR;
}

bool GraphicalView::onTouchEvent(MotionEvent& event) {
    const bool handled = handleTouch(event);
    return handled||View::onTouchEvent(event);
}

bool GraphicalView::handleTouch(MotionEvent& event) {
    const int action = event.getActionMasked();
    if((mOverlaySeriesIndex>=0)&&handleMoveEvent(event)) {
        return true;
    }
    if ((mRenderer != nullptr) && (action == MotionEvent::ACTION_MOVE)) {
        if (oldX >= 0 || oldY >= 0) {
            const float newX = event.getX(0);
            const float newY = event.getY(0);
            if (event.getPointerCount() > 1 && (oldX2 >= 0 || oldY2 >= 0) && mRenderer->isZoomEnabled()) {
                const float newX2 = event.getX(1);
                const float newY2 = event.getY(1);
                float newDeltaX = std::abs(newX - newX2);
                float newDeltaY = std::abs(newY - newY2);
                float oldDeltaX = std::abs(oldX - oldX2);
                float oldDeltaY = std::abs(oldY - oldY2);
                float zoomRate = 1;

                float tan1 = std::abs(newY - oldY) / std::abs(newX - oldX);
                float tan2 = std::abs(newY2 - oldY2) / std::abs(newX2 - oldX2);
                if (tan1 <= 0.25 && tan2 <= 0.25) {
                    // horizontal pinch zoom, |deltaY| / |deltaX| is [0 ~ 0.25], 0.25 is
                    // the approximate value of tan(PI / 12)
                    zoomRate = newDeltaX / oldDeltaX;
                    zoom(ZOOM_AXIS_X,zoomRate,true);
                } else if (tan1 >= 3.73 && tan2 >= 3.73) {
                    // pinch zoom vertically, |deltaY| / |deltaX| is [3.73 ~ infinity],
                    // 3.732 is the approximate value of tan(PI / 2 - PI / 12)
                    zoomRate = newDeltaY / oldDeltaY;
                    zoom(zoomRate, ZOOM_AXIS_Y,true);
                } else {
                    // pinch zoom diagonally
                    if (std::abs(newX - oldX) >= std::abs(newY - oldY)) {
                        zoomRate = newDeltaX / oldDeltaX;
                    } else {
                        zoomRate = newDeltaY / oldDeltaY;
                    }
                    zoom(ZOOM_AXIS_XY,zoomRate,true);
                }
                oldX2 = newX2;
                oldY2 = newY2;
            } else if (mRenderer->isPanEnabled()) {
                pan(oldX, oldY, newX, newY);
                oldX2 = oldY2 = 0;
            }
            oldX = newX;
            oldY = newY;
            invalidate();
            return true;
        }
    } else if (action == MotionEvent::ACTION_DOWN) {
        oldX = event.getX(0);
        oldY = event.getY(0);
        if (mRenderer != nullptr && mRenderer->isZoomEnabled() && mZoomR.contains(oldX, oldY)) {
            if (oldX < mZoomR.left + mZoomR.width / 3) {
                zoomIn();
            } else if (oldX < mZoomR.left + mZoomR.width * 2 / 3) {
                zoomOut();
            } else {
                zoomReset();
            }
            return true;
        }
    } else if (action == MotionEvent::ACTION_UP || action == MotionEvent::ACTION_POINTER_UP) {
        const int slop=ViewConfiguration::getTouchSlop();
        const int tapTime=ViewConfiguration::getTapTimeout();
        LOGD("duration=%d %d",int(event.getEventTime() - event.getDownTime()),tapTime);
        if( (event.getEventTime() - event.getDownTime()<=tapTime)
                || ( (event.getX(0)-oldX<slop) && (event.getY(0)-oldY<slop) ) ){
            handleSelection(oldX,oldY);
        }
        oldX = oldY = 0;
        oldX2 = oldY2 =0;
        if (action == MotionEvent::ACTION_POINTER_UP) {
            oldX = oldY = -1;
        }
    }
    return !mRenderer->isClickEnabled();
}

bool GraphicalView::handleMoveEvent(MotionEvent& event) {
    const int action = event.getActionMasked();
    if (action == MotionEvent::ACTION_MOVE && (oldX >= 0.0f || oldY >= 0.0f)) {
        const float newX = event.getX(0);
        const float newY = event.getY(0);
        if (move(oldX, oldY, newX, newY)) {
            oldX = newX;
            oldY = newY;
            invalidate();
        }
    } else if (action == MotionEvent::ACTION_DOWN) {
        oldX = event.getX(0);
        oldY = event.getY(0);
    } else if (action == MotionEvent::ACTION_UP || action == MotionEvent::ACTION_POINTER_UP) {
        mDraggingLeft = false;
        mDraggingRight = false;
        mMoving = false;
        oldX = 0.0f;
        oldY = 0.0f;
        if (action == MotionEvent::ACTION_POINTER_UP) {
            oldX = -1.0f;
            oldY = -1.0f;
        }
    }
    return true;
}

void GraphicalView::handleSelection(int x,int y){
    if( (mRenderer!=nullptr) && mRenderer->isClickEnabled() ){
        SeriesSelection sel;
        const bool hasSelection = mChart->getSeriesAndPointForScreenCoordinate({float(x),float(y)},sel);
        if( hasSelection && ( (sel.getSeriesIndex()!=mSelection.getSeriesIndex())
                    || (sel.getPointIndex()!=mSelection.getPointIndex())) ){
            LOGD_IF(hasSelection,"%d,%d",sel.getSeriesIndex(),sel.getPointIndex());
            mChart->setSelection(sel.getSeriesIndex(),sel.getPointIndex());
            mSelection = sel;
            for(auto& l:mListeners){
                if(l.onSelectChanged){
                    l.onSelectChanged(*this,mSelection);
                }
            }
            invalidate();
        }
    }
}

Bitmap GraphicalView::toBitmap() {
    setDrawingCacheEnabled(false);
    if (!isDrawingCacheEnabled()) {
        setDrawingCacheEnabled(true);
    }
    if (mRenderer->isApplyBackgroundColor()) {
        setDrawingCacheBackgroundColor(mRenderer->getBackgroundColor());
    }
    //setDrawingCacheQuality(View.DRAWING_CACHE_QUALITY_HIGH);
    return getDrawingCache(true);
}

AbstractChart* GraphicalView::getChart() const {
    return mChart;
}

}
