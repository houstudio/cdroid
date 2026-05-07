#ifndef __TOUCH_HANDLER_H__
#define __TOUCH_HANDLER_H__
#include <core/callbackbase.h>
#include <view/motionevent.h>
#include <widget/achart/chart/abstractchart.h>
#include <widget/achart/chart/xychart.h>
#include <widget/achart/chart/roundchart.h>
#include <widget/achart/chart/combinedxychart.h>
#include <widget/achart/renderer/defaultrenderer.h>
#include <widget/achart/renderer/xymultipleseriesrenderer.h>
namespace cdroid{
class GraphicalView;
class TouchHandler {
public:
    using MoveListener = CallbackBase<void>;/*moveApplied*/
    using PanListener = CallbackBase<void>;/*panApplied*/
    class ZoomEvent {
    private:
        /** A flag to be used to know if this is a zoom in or out. */
        bool mZoomIn;
        /** The zoom rate. */
        float mZoomRate;
    public:
        ZoomEvent(bool in, float rate) {
            mZoomIn = in;
            mZoomRate = rate;
        }
    
        bool isZoomIn() const{
            return mZoomIn;
        }
        float getZoomRate() const{
            return mZoomRate;
        }
    };
    class ZoomListener:public EventSet{
    public:
        /**
         * Called when a zoom change is triggered.
         * @param e the zoom event
         */
        std::function<void(const ZoomEvent&)>zoomApplied;
        /**
         * Called when a zoom reset is done.
         */
        std::function<void()> zoomReset;
    };

    class AbstractTool;
    class Pan;
    class Move;
    class Zoom;
    class FitZoom;
private:
    /** The chart renderer. */
    std::shared_ptr<DefaultRenderer> mRenderer;
    /** The old x coordinate. */
    float mLastX=0;
    /** The old y coordinate. */
    float mLastY=0;
    /** The old x2 coordinate. */
    float mLastX2=-1.f;
    /** The old y2 coordinate. */
    float mLastY2=-1.f;
    int mLastSeriesSelection=-1;
    /** The zoom buttons rectangle. */
    RectF zoomR;
    /** The pan tool. */
    Pan* mPan;
    Move*mMove;
    /** The zoom for the pinch gesture. */
    Zoom* mPinchZoom;
    /** The graphical view. */
    GraphicalView* mGraphicalView;
private:
    bool handleMoveEvent(MotionEvent& event);
    void applyZoom(float zoomRate, int axis);
public:
    /**
     * Creates a new graphical view.
     *
     * @param view the graphical view
     * @param chart the chart to be drawn
     */
    TouchHandler(GraphicalView* view, AbstractChart* chart);
    virtual ~TouchHandler();
    /**
     * Handles the touch event.
     *
     * @param event the touch event
     * @return  handleTouch
     */
    bool handleTouch(MotionEvent& event);


    /**
     * Adds a new zoom listener.
     *
     * @param listener zoom listener
     */
    void addZoomListener(const ZoomListener& listener);
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
    void addMoveListener(const MoveListener& listener);
    void removeMoveListener(const MoveListener& listener);
};

class TouchHandler::AbstractTool {
protected:
    /** The chart. */
    AbstractChart* mChart;
    /** The renderer. */
    std::shared_ptr<XYMultipleSeriesRenderer> mRenderer;
protected:
    int dpToPx(int dp)const;
    /**
     * Sets a new range on the X axis.
     *
     * @param min the minimum value
     * @param max the maximum value
     * @param scale the scale
     */
    void setXRange(double min, double max, int scale);

    /**
     * Sets a new range on the Y axis.
     *
     * @param min the minimum value
     * @param max the maximum value
     * @param scale the scale
     */
    void setYRange(double min, double max, int scale);
public:
    AbstractTool(GraphicalView* graphicalView);
    virtual ~AbstractTool()=default;
    /**
     * Abstract tool constructor.
     *
     * @param chart the chart
     */
    AbstractTool(AbstractChart* chart);
    /**
     * Returns the current chart range.
     *
     * @param scale the scale
     * @return the chart range
     */
    std::vector<double> getRange(int scale)const;

    /**
     * Sets the range to the calculated one, if not already set.
     *
     * @param range the range
     * @param scale the scale
     */
    void checkRange(std::vector<double>& range, int scale);
};

class TouchHandler::FitZoom :public TouchHandler::AbstractTool {
public:
    /**
     * Builds an instance of the fit zoom tool.
     *
     * @param chart the XY chart
     */
    FitZoom(AbstractChart* chart);

    /**
     * Apply the tool.
     */
    void apply();
};
class TouchHandler::Zoom :public TouchHandler::AbstractTool {
private:
    /** A flag to be used to know if this is a zoom in or out. */
    bool mZoomIn;
    /** Zoom limits reached on the X axis. */
    bool limitsReachedX = false;
    /** Zoom limits reached on the Y axis. */
    bool limitsReachedY = false;
    /** The zoom rate. */
    float mZoomRate;
    /** The zoom listeners. */
    std::vector<ZoomListener> mZoomListeners;
public:
    /** Zoom on X axis and Y axis */
    static constexpr int ZOOM_AXIS_XY = 0;
    /** Zoom on X axis independently */
    static constexpr int ZOOM_AXIS_X = 1;
    /** Zoom on Y axis independently */
    static constexpr int ZOOM_AXIS_Y = 2;
public:
    /**
     * Builds the zoom tool.
     *
     * @param chart the chart
     * @param in zoom in or out
     * @param rate the zoom rate
     */
    Zoom(AbstractChart* chart, bool in, float rate);

    void setZoomRate(float rate);

    void apply(int zoom_axis);
    void addZoomListener(const ZoomListener& listener);

    /**
     * Removes a zoom listener.
     *
     * @param listener zoom listener
     */
    void removeZoomListener(const ZoomListener& listener);
    void notifyZoomListeners(const ZoomEvent& e);
    void notifyZoomResetListeners();
};
class TouchHandler::Pan :public TouchHandler::AbstractTool {
private:
    /** The pan listeners. */
    std::vector<PanListener> mPanListeners;;
    /** Pan limits reached on the X axis. */
    bool limitsReachedX = false;
    /** Pan limits reached on the X axis. */
    bool limitsReachedY = false;
private:
    double getAxisRatio(const std::vector<double>& range) const;
    void notifyPanListeners();
    Pan()=delete;
public:
    /**
     * Builds and instance of the pan tool.
     *
     * @param chart the XY chart
     */
    Pan(AbstractChart* chart);

    /**
     * Apply the tool.
     *
     * @param oldX the previous location on X axis
     * @param oldY the previous location on Y axis
     * @param newX the current location on X axis
     * @param newY the current location on the Y axis
     */
    bool apply(float oldX, float oldY, float newX, float newY);
    void addPanListener(const PanListener& listener);
    void removePanListener(const PanListener& listener);
};
class TouchHandler::Move :public AbstractTool {
private:
    std::vector<MoveListener> mMoveListeners;
    Context* mContext;
    int mDragBuffer;
    int mOverlaySeriesIndex;

    bool mDraggingLeft = false;
    bool mDraggingRight = false;
    bool mMoving = false;
private:
public:
    /**
     * Builds and instance of the move tool.
     *
     * @param chart the XY chart
     */
    Move(GraphicalView*, int overlaySeriesIndex);

    /**
     * Apply the tool.
     *
     * @param oldX the previous location on X axis
     * @param oldY the previous location on Y axis
     * @param newX the current location on X axis
     * @param newY the current location on the Y axis
     */
    void apply(float oldX, float oldY, float newX, float newY);
    /**
     * Reset movement mode
     */
    void reset();

    /**
     * Adds a new move listener.
     *
     * @param listener move listener
     */
    void addMoveListener(const MoveListener& listener);
    void removeMoveListener(const MoveListener& listener);
    void notifyMoveListeners();
};

}/*endof namespace*/
#endif/*__TOUCH_HANDLER_H__*/
