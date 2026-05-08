#include <widget/achart/graphicalview.h>
namespace cdroid {
DECLARE_WIDGET(GraphicalView)
GraphicalView::GraphicalView(Context*ctx,const AttributeSet&attr):View(ctx,attr){
}

GraphicalView::GraphicalView(Context* context, AbstractChart* chart)
    :View(-1,-1){
    mChart = chart;
    mHandler = new Handler();
    mZoomIn = nullptr;
    mZoomOut= nullptr;
    //mMove = nullptr;
    mFitZoom= nullptr;
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

    if (std::dynamic_pointer_cast<XYMultipleSeriesRenderer>(mRenderer)
            && std::dynamic_pointer_cast<XYMultipleSeriesRenderer>(mRenderer)->getMarginsColor() == XYMultipleSeriesRenderer::NO_COLOR) {
        //((XYMultipleSeriesRenderer*) mRenderer)->setMarginsColor(mPaint.getColor());
    }
    //if (mRenderer->isZoomEnabled() && mRenderer->isZoomButtonsVisible()|| mRenderer->isExternalZoomEnabled())
    {
        mZoomIn = new TouchHandler::Zoom(mChart, true, mRenderer->getZoomRate());
        mZoomOut = new TouchHandler::Zoom(mChart, false, mRenderer->getZoomRate());
        //mFitZoom = new TouchHandler::FitZoom(mChart);
    }

    mTouchHandler = new TouchHandler(this, mChart);
}

GraphicalView::~GraphicalView(){
    delete mChart;
    delete mZoomIn;
    delete mZoomOut;
    delete mTouchHandler;
}
/**
 * Returns the current series selection object.
 *
 * @return the series selection
 */
SeriesSelection* GraphicalView::getCurrentSeriesAndPoint() const{
    return mChart->getSeriesAndPointForScreenCoordinate({float(oldX), float(oldY)});
}

/**
 * Transforms the currently selected screen point to a real point.
 *
 * @param scale the scale
 * @return the currently selected real point
 */
std::vector<double> GraphicalView::toRealPoint(int scale) {
    if (dynamic_cast<XYChart*>(mChart)) {
        XYChart* chart = (XYChart*) mChart;
        return chart->toRealPoint(oldX, oldY, scale);
    }
    return {};
}

void GraphicalView::onDraw(Canvas& canvas) {
    View::onDraw(canvas);
    //canvas.getClipBounds(mRect);
    int top = getTop();//mRect.top;
    int left = getLeft();//mRect.left;
    int width = getWidth();//mRect.width();
    int height = getHeight();//mRect.height();
    if (mRenderer->isInScroll()) {
        top = 0;
        left = 0;
        width = getMeasuredWidth();
        height = getMeasuredHeight();
    }
    AbstractChart::Paint mPaint;
    mChart->draw(canvas, left, top, width, height, mPaint);
    if (mRenderer != nullptr && mRenderer->isZoomEnabled() && mRenderer->isZoomButtonsVisible()) {
        /*mPaint.setColor(ZOOM_BUTTONS_COLOR);
        zoomSize = std::max(zoomSize, std::min(width, height) / 7);
        mZoomR.set(left + width - zoomSize * 3, top + height - zoomSize * 0.775f, left + width, top
                   + height);
        canvas.drawRoundRect(mZoomR, zoomSize / 3, zoomSize / 3, mPaint);
        float buttonY = top + height - zoomSize * 0.625f;
        canvas.drawBitmap(zoomInImage, left + width - zoomSize * 2.75f, buttonY, null);
        canvas.drawBitmap(zoomOutImage, left + width - zoomSize * 1.75f, buttonY, null);
        canvas.drawBitmap(fitZoomImage, left + width - zoomSize * 0.75f, buttonY, null);*/
    }
    mDrawn = true;
}

/**
 * Sets the zoom rate.
 *
 * @param rate the zoom rate
 */
void GraphicalView::setZoomRate(float rate) {
    if (mZoomIn != nullptr && mZoomOut != nullptr) {
        mZoomIn->setZoomRate(rate);
        mZoomOut->setZoomRate(rate);
    }
}

/**
 * Do a chart zoom in.
 */
void GraphicalView::zoomIn() {
    if (mZoomIn != nullptr) {
        mZoomIn->apply(TouchHandler::Zoom::ZOOM_AXIS_XY);
        invalidate();
    }
}

/**
 * Do a chart zoom out.
 */
void GraphicalView::zoomOut() {
    if (mZoomOut != nullptr) {
        mZoomOut->apply(TouchHandler::Zoom::ZOOM_AXIS_XY);
        invalidate();
    }
}

/**
 * Do a chart zoom reset / fit zoom.
 */
void GraphicalView::zoomReset() {
    if (mFitZoom != nullptr) {
        mFitZoom->apply();
        mZoomIn->notifyZoomResetListeners();
        invalidate();
    }
}

/**
 * Adds a new zoom listener.
 *
 * @param listener zoom listener
 */
void GraphicalView::addZoomListener(const ZoomListener& listener, bool onButtons, bool onPinch) {
    if (onButtons) {
        if (mZoomIn != nullptr) {
            mZoomIn->addZoomListener(listener);
            mZoomOut->addZoomListener(listener);
        }
        if (onPinch) {
            mTouchHandler->addZoomListener(listener);
        }
    }
}

/**
 * Removes a zoom listener.
 *
 * @param listener zoom listener
 */
void GraphicalView::removeZoomListener(const ZoomListener& listener) {
    if (mZoomIn != nullptr) {
        mZoomIn->removeZoomListener(listener);
        mZoomOut->removeZoomListener(listener);
    }
    mTouchHandler->removeZoomListener(listener);
}

/**
 * Adds a new pan listener.
 *
 * @param listener pan listener
 */
void GraphicalView::addPanListener(const PanListener& listener) {
    mTouchHandler->addPanListener(listener);
}

/**
 * Removes a pan listener.
 *
 * @param listener pan listener
 */
void GraphicalView::removePanListener(const PanListener& listener) {
    mTouchHandler->removePanListener(listener);
}

/**
 * Adds a new move listener.
 *
 * @param listener move listener
 */
void GraphicalView::addMoveListener(const MoveListener& listener) {
    mTouchHandler->addMoveListener(listener);
}

/**
 * Removes a move listener.
 *
 * @param listener move listener
 */
void GraphicalView::removeMoveListener(const MoveListener& listener) {
    mTouchHandler->removeMoveListener(listener);
}

RectF GraphicalView::getZoomRectangle() const {
    return mZoomR;
}

bool GraphicalView::onTouchEvent(MotionEvent& event) {
    const int action = event.getActionMasked();
    if (action == MotionEvent::ACTION_DOWN||action == MotionEvent::ACTION_UP||action == MotionEvent::ACTION_MOVE) {
        // save the x and y so they can be used in the click and long press
        // listeners
        oldX = event.getX();
        oldY = event.getY();
    }
    if (mRenderer != nullptr && mDrawn && (mRenderer->isPanEnabled() || mRenderer->isZoomEnabled())) {
        if (mTouchHandler->handleTouch(event)) {
            return true;
        }
    }
    return View::onTouchEvent(event);
}

/**
 * Schedule a view content repaint.
 */
void GraphicalView::repaint() {
    /*mHandler.post(new Runnable() {
        public void run() {
            invalidate();
        }
    });*/
}

/**
 * Schedule a view content repaint, in the specified rectangle area.
 *
 * @param left the left position of the area to be repainted
 * @param top the top position of the area to be repainted
 * @param right the right position of the area to be repainted
 * @param bottom the bottom position of the area to be repainted
 */
void GraphicalView::repaint(int left,int top,int right,int bottom) {
    /*mHandler.post(new Runnable() {
        public void run() {
            invalidate(left, top, right, bottom);
        }
    });*/
}

/**
 * Saves the content of the graphical view to a bitmap.
 *
 * @return the bitmap
 */
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

/**
 * Returns the chart that belongs to this view
 *
 * @return the chart
 */
AbstractChart* GraphicalView::getChart() const {
    return mChart;
}

}
