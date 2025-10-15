/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __SCALE_GESTURE_DETECTOR_H__
#define __SCALE_GESTURE_DETECTOR_H__
#include <view/gesturedetector.h>
namespace cdroid{
class ScaleGestureDetector {
public:
    /**
     * The listener for receiving notifications when gestures occur.
     * If you want to listen for all the different gestures then implement
     * this interface. If you only want to listen for a subset it might
     * be easier to extend {@link SimpleOnScaleGestureListener}.
     *
     * An application will receive events in the following order:
     * <ul>
     *  <li>One {@link OnScaleGestureListener#onScaleBegin(ScaleGestureDetector)}
     *  <li>Zero or more {@link OnScaleGestureListener#onScale(ScaleGestureDetector)}
     *  <li>One {@link OnScaleGestureListener#onScaleEnd(ScaleGestureDetector)}
     * </ul>
     */
    struct OnScaleGestureListener {
        std::function<bool(ScaleGestureDetector&)> onScale;
        std::function<bool(ScaleGestureDetector&)> onScaleBegin;

        /**
         * Responds to the end of a scale gesture. Reported by existing
         * pointers going up.
         *
         * Once a scale has ended, {@link ScaleGestureDetector#getFocusX()}
         * and {@link ScaleGestureDetector#getFocusY()} will return focal point
         * of the pointers remaining on the screen.
         *
         * @param detector The detector reporting the event - use this to
         *          retrieve extended info about event state.
         */
        std::function<void(ScaleGestureDetector&)> onScaleEnd;
    };
private:
    Context* mContext;
    OnScaleGestureListener mListener;

    float mFocusX;
    float mFocusY;

    bool mQuickScaleEnabled;
    bool mStylusScaleEnabled;
    bool mInProgress;

    float mCurrSpan;
    float mPrevSpan;
    float mInitialSpan;
    float mCurrSpanX;
    float mCurrSpanY;
    float mPrevSpanX;
    float mPrevSpanY;
    int64_t mCurrTime;
    int64_t mPrevTime;
    int mSpanSlop;
    int mMinSpan;

    float mAnchoredScaleStartX;
    float mAnchoredScaleStartY;
    int mAnchoredScaleMode = ANCHORED_SCALE_MODE_NONE;

    static constexpr long TOUCH_STABILIZE_TIME = 128; // ms
    static constexpr float SCALE_FACTOR = .5f;
    static constexpr int ANCHORED_SCALE_MODE_NONE = 0;
    static constexpr int ANCHORED_SCALE_MODE_DOUBLE_TAP = 1;
    static constexpr int ANCHORED_SCALE_MODE_STYLUS = 2;

    /**
     * Consistency verifier for debugging purposes.
     */
    InputEventConsistencyVerifier* mInputEventConsistencyVerifier =nullptr;
    GestureDetector* mGestureDetector;

    bool mEventBeforeOrAboveStartingGestureEvent;

    /**
     * Creates a ScaleGestureDetector with the supplied listener.
     * @see android.os.Handler#Handler()
     *
     * @param context the application's context
     * @param listener the listener invoked for all the callbacks, this must
     * not be null.
     * @param handler the handler to use for running deferred listener events.
     *
     * @throws NullPointerException if {@code listener} is null.
     */
private:
    bool inAnchoredScaleMode()const;
public:
    ScaleGestureDetector(Context* context,const OnScaleGestureListener& listener);
    ~ScaleGestureDetector();
    /**
     * Accepts MotionEvents and dispatches events to a {@link OnScaleGestureListener}
     * when appropriate.
     *
     * <p>Applications should pass a complete and consistent event stream to this method.
     * A complete and consistent event stream involves all MotionEvents from the initial
     * ACTION_DOWN to the final ACTION_UP or ACTION_CANCEL.</p>
     *
     * @param event The event to process
     * @return true if the event was processed and the detector wants to receive the
     *         rest of the MotionEvents in this event stream.
     */
    bool onTouchEvent(MotionEvent& event);

    /**
     * Set whether the associated {@link OnScaleGestureListener} should receive onScale callbacks
     * when the user performs a doubleTap followed by a swipe. Note that this is enabled by default
     * if the app targets API 19 and newer.
     * @param scales true to enable quick scaling, false to disable
     */
    void setQuickScaleEnabled(bool scales);
    /**
     * Return whether the quick scale gesture, in which the user performs a double tap followed by a
     * swipe, should perform scaling. {@see #setQuickScaleEnabled(bool)}.
     */
    bool isQuickScaleEnabled()const;
    /**
     * Sets whether the associates {@link OnScaleGestureListener} should receive
     * onScale callbacks when the user uses a stylus and presses the button.
     * Note that this is enabled by default if the app targets API 23 and newer.
     *
     * @param scales true to enable stylus scaling, false to disable.
     */
    void setStylusScaleEnabled(bool scales);
    /**
     * Return whether the stylus scale gesture, in which the user uses a stylus and presses the
     * button, should perform scaling. {@see #setStylusScaleEnabled(bool)}
     */
    bool isStylusScaleEnabled()const;

    /**
     * Returns {@code true} if a scale gesture is in progress.
     */
    bool isInProgress()const;
    /**
     * Get the X coordinate of the current gesture's focal point.
     * If a gesture is in progress, the focal point is between
     * each of the pointers forming the gesture.
     *
     * If {@link #isInProgress()} would return false, the result of this
     * function is undefined.
     *
     * @return X coordinate of the focal point in pixels.
     */
    float getFocusX() const;
    /**
     * Get the Y coordinate of the current gesture's focal point.
     * If a gesture is in progress, the focal point is between
     * each of the pointers forming the gesture.
     *
     * If {@link #isInProgress()} would return false, the result of this
     * function is undefined.
     *
     * @return Y coordinate of the focal point in pixels.
     */
    float getFocusY() const;
    /**
     * Return the average distance between each of the pointers forming the
     * gesture in progress through the focal point.
     *
     * @return Distance between pointers in pixels.
     */
    float getCurrentSpan()const;
    /**
     * Return the average X distance between each of the pointers forming the
     * gesture in progress through the focal point.
     *
     * @return Distance between pointers in pixels.
     */
    float getCurrentSpanX() const;
    /**
     * Return the average Y distance between each of the pointers forming the
     * gesture in progress through the focal point.
     *
     * @return Distance between pointers in pixels.
     */
    float getCurrentSpanY() const;
    /**
     * Return the previous average distance between each of the pointers forming the
     * gesture in progress through the focal point.
     *
     * @return Previous distance between pointers in pixels.
     */
    float getPreviousSpan() const;
    /**
     * Return the previous average X distance between each of the pointers forming the
     * gesture in progress through the focal point.
     *
     * @return Previous distance between pointers in pixels.
     */
    float getPreviousSpanX() const;

    /**
     * Return the previous average Y distance between each of the pointers forming the
     * gesture in progress through the focal point.
     *
     * @return Previous distance between pointers in pixels.
     */
    float getPreviousSpanY() const;

    /**
     * Return the scaling factor from the previous scale event to the current
     * event. This value is defined as
     * ({@link #getCurrentSpan()} / {@link #getPreviousSpan()}).
     *
     * @return The current scaling factor.
     */
    float getScaleFactor() const;

    /**
     * Return the time difference in milliseconds between the previous
     * accepted scaling event and the current scaling event.
     *
     * @return Time difference since the last scaling event in milliseconds.
     */
    long getTimeDelta() const;

    /**
     * Return the event time of the current event being processed.
     *
     * @return Current event time in milliseconds.
     */
    int64_t getEventTime() const;
};
}/*endof namespace*/
#endif/*__SCALE_GESTURE_DETECTOR_H__*/
