#ifndef __ACHART_GRAPHICAL_VIEW_H__
#define __ACHART_GRAPHICAL_VIEW_H__
#include <view/view.h>
#include <widget/achart/chart/xychart.h>
#include <widget/achart/chart/roundchart.h>
#include <widget/achart/renderer/defaultrenderer.h>
#include <widget/achart/model/seriesselection.h>
#include <widget/achart/touchhandler.h>
namespace cdroid{
/**
 * The view that encapsulates the graphical chart.
 */
class GraphicalView :public View {
public:
    using MoveListener=TouchHandler::MoveListener;
    using PanListener =TouchHandler::PanListener;
    using ZoomListener=TouchHandler::ZoomListener;
    using ZoomEvent = TouchHandler::ZoomEvent;
private:
  /** The chart to be drawn. */
    AbstractChart* mChart;
    /** The chart renderer. */
    std::shared_ptr<DefaultRenderer> mRenderer;
    /** The view bounds. */
    Rect mRect;
    /** The user interface thread handler. */
    Handler* mHandler;
    /** The zoom buttons rectangle. */
    RectF mZoomR;
    /** The zoom in icon. */
    Bitmap zoomInImage;
    /** The zoom out icon. */
    Bitmap zoomOutImage;
    /** The fit zoom icon. */
    Bitmap fitZoomImage;
    /** The zoom area size. */
    int zoomSize = 50;
    /** The zoom buttons background color. */
    static constexpr int ZOOM_BUTTONS_COLOR = 0xAF969696;
    /** The zoom in tool. */
    TouchHandler::Zoom* mZoomIn;
    /** The zoom out tool. */
    TouchHandler::Zoom* mZoomOut;
    /** The fit zoom tool. */
    TouchHandler::FitZoom* mFitZoom;
    /** The paint to be used when drawing the chart. */
    //Paint mPaint = new Paint();
    /** The touch handler. */
    TouchHandler* mTouchHandler;
    /** The old x coordinate. */
    float oldX;
    /** The old y coordinate. */
    float oldY;
    /** If the graphical view is drawn. */
    bool mDrawn;
protected:
    friend TouchHandler;
    void onDraw(Canvas& canvas) override;
    RectF getZoomRectangle()const;
public:
    GraphicalView(Context*,const AttributeSet&);
    ~GraphicalView()override;
    /**
     * Creates a new graphical view.
     * 
     * @param context the context
     * @param chart the chart to be drawn
     */
    GraphicalView(Context* context, AbstractChart* chart);
  
    /**
     * Returns the current series selection object.
     * 
     * @return the series selection
     */
    SeriesSelection* getCurrentSeriesAndPoint() const;
  
    /**
     * Transforms the currently selected screen point to a real point.
     * 
     * @param scale the scale
     * @return the currently selected real point
     */
    std::vector<double> toRealPoint(int scale);
  
  
    /**
     * Sets the zoom rate.
     * 
     * @param rate the zoom rate
     */
    void setZoomRate(float rate);
  
    /**
     * Do a chart zoom in.
     */
    void zoomIn();
  
    /**
     * Do a chart zoom out.
     */
    void zoomOut();
    /**
     * Do a chart zoom reset / fit zoom.
     */
    void zoomReset();

    /**
     * Adds a new zoom listener.
     * 
     * @param listener zoom listener
     */
    void addZoomListener(const ZoomListener& listener, bool onButtons, bool onPinch);
    /**
     * Removes a zoom listener.
     * 
     * @param listener zoom listener
     */
    void removeZoomListener(const ZoomListener& listener);
  
    /**
     * Adds a new pan listener.
     * 
     * @param listener pan listener
     */
    void addPanListener(const PanListener& listener);
    /**
     * Removes a pan listener.
     * 
     * @param listener pan listener
     */
    void removePanListener(const PanListener& listener);
    /**
     * Adds a new move listener.
     * 
     * @param listener move listener
     */
    void addMoveListener(const MoveListener& listener);
  
    /**
     * Removes a move listener.
     * 
     * @param listener move listener
     */
    void removeMoveListener(const MoveListener& listener);

    bool onTouchEvent(MotionEvent& event) override;
  
    /**
     * Schedule a view content repaint.
     */
    void repaint();
    /**
     * Schedule a view content repaint, in the specified rectangle area.
     * 
     * @param left the left position of the area to be repainted
     * @param top the top position of the area to be repainted
     * @param right the right position of the area to be repainted
     * @param bottom the bottom position of the area to be repainted
     */
    void repaint(int left, int top, int right, int bottom);
  
    /**
     * Saves the content of the graphical view to a bitmap.
     * 
     * @return the bitmap
     */
    Bitmap toBitmap();
  
    /**
     * Returns the chart that belongs to this view
     * 
     * @return the chart
     */
    AbstractChart* getChart() const;
};
}/*endof namespace*/
#endif/*__GRAPHICAL_VIEW_H__*/
