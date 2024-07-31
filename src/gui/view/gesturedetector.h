#ifndef __GESTURE_DETECTOR_H__
#define __GESTURE_DETECTOR_H__
#include <core/context.h>
#include <view/motionevent.h>
#include <view/velocitytracker.h>
#include <view/viewconfiguration.h>
#include <view/inputeventconsistencyverifier.h>
namespace cdroid{
class GestureDetector {
public:
    /**
     * The listener that is used to notify when gestures occur.
     * If you want to listen for all the different gestures then implement
     * this interface. If you only want to listen for a subset it might
     * be easier to extend {@link SimpleOnGestureListener}.
     */
    struct OnGestureListener {

        /**
         * Notified when a tap occurs with the down {@link MotionEvent}
         * that triggered it. This will be triggered immediately for
         * every down event. All other events should be preceded by this.
         *
         * @param e The down motion event.
         */
        CallbackBase<bool,MotionEvent&> onDown;

        /**
         * The user has performed a down {@link MotionEvent} and not performed
         * a move or up yet. This event is commonly used to provide visual
         * feedback to the user to let them know that their action has been
         * recognized i.e. highlight an element.
         *
         * @param e The down motion event
         */
        CallbackBase<void,MotionEvent&> onShowPress;

        /**
         * Notified when a tap occurs with the up {@link MotionEvent}
         * that triggered it.
         *
         * @param e The up motion event that completed the first tap
         * @return true if the event is consumed, else false
         */
        CallbackBase<bool,MotionEvent&> onSingleTapUp;

        /**
         * Notified when a scroll occurs with the initial on down {@link MotionEvent} and the
         * current move {@link MotionEvent}. The distance in x and y is also supplied for
         * convenience.
         *
         * @param e1 The first down motion event that started the scrolling. A {@code null} event
         *           indicates an incomplete event stream or error state.
         * @param e2 The move motion event that triggered the current onScroll.
         * @param distanceX The distance along the X axis that has been scrolled since the last
         *              call to onScroll. This is NOT the distance between {@code e1}
         *              and {@code e2}.
         * @param distanceY The distance along the Y axis that has been scrolled since the last
         *              call to onScroll. This is NOT the distance between {@code e1}
         *              and {@code e2}.
         * @return true if the event is consumed, else false
         */
        CallbackBase<bool,MotionEvent*,MotionEvent&,float,float> onScroll;//(MotionEvent* e1,MotionEvent& e2, float distanceX, float distanceY);

        CallbackBase<void,MotionEvent&> onLongPress;

        /**
         * Notified of a fling event when it occurs with the initial on down {@link MotionEvent}
         * and the matching up {@link MotionEvent}. The calculated velocity is supplied along
         * the x and y axis in pixels per second.
         *
         * @param e1 The first down motion event that started the fling. A {@code null} event
         *           indicates an incomplete event stream or error state.
         * @param e2 The move motion event that triggered the current onFling.
         * @param velocityX The velocity of this fling measured in pixels per second
         *              along the x axis.
         * @param velocityY The velocity of this fling measured in pixels per second
         *              along the y axis.
         * @return true if the event is consumed, else false
         */
        CallbackBase<bool,MotionEvent*,MotionEvent&,float,float> onFling;//(MotionEvent*e1,MotionEvent& e2, float velocityX,float velocityY);
    };

    /**
     * The listener that is used to notify when a double-tap or a confirmed
     * single-tap occur.
     */
    struct OnDoubleTapListener {
        /**
         * Notified when a single-tap occurs.
         * <p>
         * Unlike {@link OnGestureListener#onSingleTapUp(MotionEvent)}, this
         * will only be called after the detector is confident that the user's
         * first tap is not followed by a second tap leading to a double-tap
         * gesture.
         *
         * @param e The down motion event of the single-tap.
         * @return true if the event is consumed, else false
         */
        CallbackBase<bool,MotionEvent&> onSingleTapConfirmed;

        /**
         * Notified when a double-tap occurs. Triggered on the down event of second tap.
         *
         * @param e The down motion event of the first tap of the double-tap.
         * @return true if the event is consumed, else false
         */
        CallbackBase<bool,MotionEvent&> onDoubleTap;

        /**
         * Notified when an event within a double-tap gesture occurs, including
         * the down, move, and up events.
         *
         * @param e The motion event that occurred during the double-tap gesture.
         * @return true if the event is consumed, else false
         */
        CallbackBase<bool,MotionEvent&> onDoubleTapEvent;
    };

    typedef CallbackBase<bool,MotionEvent&>OnContextClickListener;
private:
    int mTouchSlopSquare;
    int mDoubleTapTouchSlopSquare;
    int mDoubleTapSlopSquare;
    float mAmbiguousGestureMultiplier;
    int mMinimumFlingVelocity;
    int mMaximumFlingVelocity;

    static constexpr int LONGPRESS_TIMEOUT = ViewConfiguration::getLongPressTimeout();
    static constexpr int TAP_TIMEOUT = ViewConfiguration::getTapTimeout();
    static constexpr int DOUBLE_TAP_TIMEOUT = ViewConfiguration::getDoubleTapTimeout();
    static constexpr int DOUBLE_TAP_MIN_TIME = ViewConfiguration::getDoubleTapMinTime();

    OnGestureListener mListener;
    OnDoubleTapListener mDoubleTapListener;
    OnContextClickListener mContextClickListener;
    Runnable mPressRunnable;
    Runnable mLongPressRunnable;
    Runnable mTapRunnable;

    bool mStillDown;
    bool mDeferConfirmSingleTap;
    bool mInLongPress;
    bool mInContextClick;
    bool mAlwaysInTapRegion;
    bool mAlwaysInBiggerTapRegion;
    bool mIgnoreNextUpEvent;
    // Whether a classification has been recorded by statsd for the current event stream. Reset on
    // ACTION_DOWN.
    bool mHasRecordedClassification;
    MotionEvent* mCurrentDownEvent;
    MotionEvent* mCurrentMotionEvent;
    MotionEvent* mPreviousUpEvent;

    bool mIsDoubleTapping;

    float mLastFocusX;
    float mLastFocusY;
    float mDownFocusX;
    float mDownFocusY;

    bool mIsLongpressEnabled;
    VelocityTracker*mVelocityTracker;
    InputEventConsistencyVerifier* mInputEventConsistencyVerifier =nullptr;
private:
    void init(Context* context);
    void cancel();
    void cancelTaps();
    bool isConsideredDoubleTap(MotionEvent& firstDown,MotionEvent& firstUp,MotionEvent& secondDown);
    void dispatchLongPress();
    void recordGestureClassification(int classification);
public:
    GestureDetector(Context* context,const OnGestureListener& listener);
    ~GestureDetector();
    void setOnDoubleTapListener(const OnDoubleTapListener& onDoubleTapListener);
    void setContextClickListener(const OnContextClickListener& onContextClickListener);
    void setIsLongpressEnabled(bool isLongpressEnabled);
    bool isLongpressEnabled()const;

    bool onTouchEvent(MotionEvent& ev);
    bool onGenericMotionEvent(MotionEvent& ev);

};
}/*endof namespace*/
#endif/*__GESTURE_DETECTOR_H__*/
