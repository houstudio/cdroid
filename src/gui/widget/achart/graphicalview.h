#ifndef __ACHART_GRAPHICAL_VIEW_H__
#define __ACHART_GRAPHICAL_VIEW_H__
#include <view/view.h>
#include <widget/achart/chart/xychart.h>
#include <widget/achart/chart/roundchart.h>
#include <widget/achart/renderer/defaultrenderer.h>
#include <widget/achart/model/seriesselection.h>
//#include <widget/achart/touchhandler.h>
namespace cdroid{
/**
 * The view that encapsulates the graphical chart.
 */
class GraphicalView :public View {
public:
    class ZoomListener:public EventSet{
    public:
        std::function<void(float zoomRate,bool isZoomIm)>zoomApplied;
        std::function<void()> zoomReset;
    };
    using PanListener = CallbackBase<void>;/*panApplied*/
private:
  /** The chart to be drawn. */
    AbstractChart* mChart;
    /** The chart renderer. */
    std::shared_ptr<DefaultRenderer> mRenderer;
    /** The zoom buttons rectangle. */
    RectF mZoomR;
    /** The zoom in icon. */
    Bitmap zoomInImage;
    /** The zoom out icon. */
    Bitmap zoomOutImage;
    /** The fit zoom icon. */
    Bitmap fitZoomImage;
    /** The zoom buttons background color. */
    static constexpr int ZOOM_BUTTONS_COLOR = 0xAF969696;
    /** The old x coordinate. */
    float oldX,oldX2;
    /** The old y coordinate. */
    float oldY,oldY2;
    float mZoomRate;
    int mOverlaySeriesIndex;
    bool mDraggingLeft = false;
    bool mDraggingRight = false;
    bool mMoving;
    /** If the graphical view is drawn. */
    bool mDrawn;
    std::vector<PanListener> mPanListeners;
    std::vector<ZoomListener> mZoomListeners;
    static constexpr int ZOOM_AXIS_X=1;
    static constexpr int ZOOM_AXIS_Y=2;
    static constexpr int ZOOM_AXIS_XY=3;
protected:
    void onDraw(Canvas& canvas) override;
    RectF getZoomRectangle()const;
    void setXRange(double min, double max, int scale);
    void setYRange(double min, double max, int scale);
    std::vector<double>getRange(int scale)const;
    void checkRange(std::vector<double>& range, int scale);
    double getAxisRatio(std::vector<double>& range) const;
    bool handleTouch(MotionEvent& event);
    void handleSelection(int,int);
    bool handleMoveEvent(MotionEvent& event);
    void zoom(int axis,float zoomrate,bool zoomIn);
    void pan(float oldX, float oldY, float newX, float newY);
    bool move(float oldX,float oldY,float newX,float newY);
    void notifyPanListeners();
    void notifyZoomListeners(float zoomrate,bool isZoomin);
    void notifyZoomResetListeners();
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

    bool onTouchEvent(MotionEvent& event) override;
  
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
