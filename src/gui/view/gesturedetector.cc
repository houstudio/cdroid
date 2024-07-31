#include <core/handler.h>
#include <view/choreographer.h>
#include <view/viewconfiguration.h>
#include <view/gesturedetector.h>
namespace cdroid{

#define TOUCH_GESTURE_CLASSIFIED__CLASSIFICATION__SINGLE_TAP 10000
#define TOUCH_GESTURE_CLASSIFIED__CLASSIFICATION__DOUBLE_TAP 10001
#define TOUCH_GESTURE_CLASSIFIED__CLASSIFICATION__LONG_PRESS 10002
#define TOUCH_GESTURE_CLASSIFIED__CLASSIFICATION__DEEP_PRESS 10003
#define TOUCH_GESTURE_CLASSIFIED__CLASSIFICATION__SCROLL     10004
#define TOUCH_GESTURE_CLASSIFIED__CLASSIFICATION__UNKNOWN_CLASSIFICATION 19999

GestureDetector::GestureDetector(Context* context,const OnGestureListener& listener) {
    /*if (handler != nullptr) {
        mHandler = new GestureHandler(handler);
    } else {
        mHandler = new GestureHandler();
    }*/
    mListener = listener;
    /*if (dynamic_cast<const OnDoubleTapListener*>(&listener)) {
        setOnDoubleTapListener((const OnDoubleTapListener&) listener);
    }
    if (dynamic_cast<const OnContextClickListener*>(&listener)) {
        setContextClickListener((const OnContextClickListener&) listener);
    }*/
    init(context);
}

void GestureDetector::init(Context* context) {
    mIsLongpressEnabled = true;
    mVelocityTracker = nullptr;
    mVelocityTracker = nullptr;
    mCurrentDownEvent = nullptr;
    mPreviousUpEvent  = nullptr;
    mCurrentMotionEvent= nullptr;
    mInputEventConsistencyVerifier = nullptr;
    mStillDown = false;
    mInLongPress = mInContextClick = false;
    mIsDoubleTapping = false;
    mAlwaysInTapRegion = false;
    mAlwaysInBiggerTapRegion =false;
    mIgnoreNextUpEvent = false;
    mDeferConfirmSingleTap = false;
    // Fallback to support pre-donuts releases
    int touchSlop, doubleTapSlop, doubleTapTouchSlop;
    if (context == nullptr) {
        //noinspection deprecation
        touchSlop = ViewConfiguration::getTouchSlop();
        doubleTapTouchSlop = touchSlop; // Hack rather than adding a hidden method for this
        doubleTapSlop = ViewConfiguration::getDoubleTapSlop();
        //noinspection deprecation
        mMinimumFlingVelocity = ViewConfiguration::getMinimumFlingVelocity();
        mMaximumFlingVelocity = ViewConfiguration::getMaximumFlingVelocity();
        mAmbiguousGestureMultiplier = ViewConfiguration::getAmbiguousGestureMultiplier();
    } else {
        //StrictMode.assertConfigurationContext(context, "GestureDetector#init");
        ViewConfiguration configuration = ViewConfiguration::get(context);
        touchSlop = configuration.getScaledTouchSlop();
        doubleTapTouchSlop = configuration.getScaledDoubleTapTouchSlop();
        doubleTapSlop = configuration.getScaledDoubleTapSlop();
        mMinimumFlingVelocity = configuration.getScaledMinimumFlingVelocity();
        mMaximumFlingVelocity = configuration.getScaledMaximumFlingVelocity();
        mAmbiguousGestureMultiplier = configuration.getScaledAmbiguousGestureMultiplier();
    }
    mTouchSlopSquare = touchSlop * touchSlop;
    mDoubleTapTouchSlopSquare = doubleTapTouchSlop * doubleTapTouchSlop;
    mDoubleTapSlopSquare = doubleTapSlop * doubleTapSlop;
    if(InputEventConsistencyVerifier::isInstrumentationEnabled())
         mInputEventConsistencyVerifier = new InputEventConsistencyVerifier((Object*)this, 0) ;
    mPressRunnable = [this](){
        if(mListener.onShowPress)mListener.onShowPress(*mCurrentDownEvent);
    };
    mLongPressRunnable=[this](){
        //recordGestureClassification(msg.arg1);
        dispatchLongPress();
    };
    mTapRunnable=[this](){
        if (mDoubleTapListener.onSingleTapConfirmed != nullptr) {
            if (!mStillDown) {
                recordGestureClassification(TOUCH_GESTURE_CLASSIFIED__CLASSIFICATION__SINGLE_TAP);
                mDoubleTapListener.onSingleTapConfirmed(*mCurrentDownEvent);
            } else {
                mDeferConfirmSingleTap = true;
            }
        }

    };
}

void GestureDetector::setOnDoubleTapListener(const OnDoubleTapListener& onDoubleTapListener) {
    mDoubleTapListener = onDoubleTapListener;
}

void GestureDetector::setContextClickListener(const OnContextClickListener& onContextClickListener) {
    mContextClickListener = onContextClickListener;
}

void GestureDetector::setIsLongpressEnabled(bool isLongpressEnabled) {
    mIsLongpressEnabled = isLongpressEnabled;
}

bool GestureDetector::isLongpressEnabled() const{
    return mIsLongpressEnabled;
}

bool GestureDetector::onTouchEvent(MotionEvent& ev) {
    if (mInputEventConsistencyVerifier) {
        mInputEventConsistencyVerifier->onTouchEvent(ev, 0);
    }

    const int action = ev.getAction();

    if (mCurrentMotionEvent ) {
        mCurrentMotionEvent->recycle();
    }
    mCurrentMotionEvent = MotionEvent::obtain(ev);

    if (mVelocityTracker == nullptr) {
        mVelocityTracker = VelocityTracker::obtain();
    }
    mVelocityTracker->addMovement(ev);

    const bool pointerUp = (action & MotionEvent::ACTION_MASK) == MotionEvent::ACTION_POINTER_UP;
    const int skipIndex = pointerUp ? ev.getActionIndex() : -1;
    const bool isGeneratedGesture = (ev.getFlags() & MotionEvent::FLAG_IS_GENERATED_GESTURE) != 0;

    // Determine focal point
    float sumX = 0, sumY = 0;
    const int count = ev.getPointerCount();
    for (int i = 0; i < count; i++) {
        if (skipIndex == i) continue;
        sumX += ev.getX(i);
        sumY += ev.getY(i);
    }
    const int div = pointerUp ? count - 1 : count;
    const float focusX = sumX / div;
    const float focusY = sumY / div;

    bool handled = false;

    switch (action & MotionEvent::ACTION_MASK) {
    case MotionEvent::ACTION_POINTER_DOWN:
        mDownFocusX = mLastFocusX = focusX;
        mDownFocusY = mLastFocusY = focusY;
        // Cancel long press and taps
        cancelTaps();
        break;

    case MotionEvent::ACTION_POINTER_UP:
        mDownFocusX = mLastFocusX = focusX;
        mDownFocusY = mLastFocusY = focusY;

        // Check the dot product of current velocities.
        // If the pointer that left was opposing another velocity vector, clear.
        mVelocityTracker->computeCurrentVelocity(1000, mMaximumFlingVelocity);
        {
            const int upIndex = ev.getActionIndex();
            const int id1 = ev.getPointerId(upIndex);
            const float x1 = mVelocityTracker->getXVelocity(id1);
            const float y1 = mVelocityTracker->getYVelocity(id1);
            for (int i = 0; i < count; i++) {
                if (i == upIndex) continue;

                const int id2 = ev.getPointerId(i);
                const float x = x1 * mVelocityTracker->getXVelocity(id2);
                const float y = y1 * mVelocityTracker->getYVelocity(id2);

                const float dot = x + y;
                if (dot < 0) {
                    mVelocityTracker->clear();
                    break;
                }
            }
        }
        break;

    case MotionEvent::ACTION_DOWN:
        if (mDoubleTapListener.onDoubleTap||mDoubleTapListener.onSingleTapConfirmed||mDoubleTapListener.onDoubleTapEvent) {
            //bool hadTapMessage mHandler->hasMessages(TAP);
            //if (hadTapMessage) mHandler->removeMessages(TAP);
            const int hadTapMessage = Choreographer::getInstance().removeCallbacks(Choreographer::CALLBACK_ANIMATION,&mTapRunnable,this);
            if (mCurrentDownEvent && mPreviousUpEvent && hadTapMessage
                    && isConsideredDoubleTap(*mCurrentDownEvent,*mPreviousUpEvent, ev)) {
                // This is a second tap
                mIsDoubleTapping = true;
                recordGestureClassification(TOUCH_GESTURE_CLASSIFIED__CLASSIFICATION__DOUBLE_TAP);
                // Give a callback with the first tap of the double-tap
                handled |= (mDoubleTapListener.onDoubleTap?mDoubleTapListener.onDoubleTap(*mCurrentDownEvent):false);
                // Give a callback with down event of the double-tap
                handled |= (mDoubleTapListener.onDoubleTapEvent?mDoubleTapListener.onDoubleTapEvent(ev):false);
            } else {
                // This is a first tap
                //mHandler->sendEmptyMessageDelayed(TAP, DOUBLE_TAP_TIMEOUT);
                Choreographer::getInstance().postCallbackDelayed(Choreographer::CALLBACK_ANIMATION,mTapRunnable,this,DOUBLE_TAP_TIMEOUT);
            }
        }

        mDownFocusX = mLastFocusX = focusX;
        mDownFocusY = mLastFocusY = focusY;
        if (mCurrentDownEvent != nullptr) {
            mCurrentDownEvent->recycle();
        }
        mCurrentDownEvent = MotionEvent::obtain(ev);
        mAlwaysInTapRegion = true;
        mAlwaysInBiggerTapRegion = true;
        mStillDown = true;
        mInLongPress = false;
        mDeferConfirmSingleTap = false;
        mHasRecordedClassification = false;

        if (mIsLongpressEnabled) {
            //mHandler->removeMessages(LONG_PRESS);
            //mHandler->sendMessageAtTime(mHandler->obtainMessage(LONG_PRESS, TOUCH_GESTURE_CLASSIFIED__CLASSIFICATION__LONG_PRESS, 0 /* arg2 */),
            //        mCurrentDownEvent->getDownTime() + ViewConfiguration::getLongPressTimeout());
            Choreographer::getInstance().removeCallbacks(Choreographer::CALLBACK_ANIMATION,&mLongPressRunnable,this);
            Choreographer::getInstance().postCallbackDelayed(Choreographer::CALLBACK_ANIMATION,mLongPressRunnable,this,ViewConfiguration::getLongPressTimeout());
        }
        //mHandler->sendEmptyMessageAtTime(SHOW_PRESS, mCurrentDownEvent->getDownTime() + TAP_TIMEOUT);
        Choreographer::getInstance().postCallbackDelayed(Choreographer::CALLBACK_ANIMATION,mPressRunnable,this,TAP_TIMEOUT);
        handled |= (mListener.onDown?mListener.onDown(ev):false);
        break;

    case MotionEvent::ACTION_MOVE:
        if ((mInLongPress==false) && (mInContextClick==false)){
            const int motionClassification = ev.getClassification();
            //const bool hasPendingLongPress = mHandler->hasMessages(LONG_PRESS);
            const int hasPendingLongPress = Choreographer::getInstance().removeCallbacks(Choreographer::CALLBACK_ANIMATION,&mLongPressRunnable,this);
            const float scrollX = mLastFocusX - focusX;
            const float scrollY = mLastFocusY - focusY;
            if (mIsDoubleTapping) {
                // Give the move events of the double-tap
                recordGestureClassification(TOUCH_GESTURE_CLASSIFIED__CLASSIFICATION__DOUBLE_TAP);
                handled |= (mDoubleTapListener.onDoubleTapEvent?mDoubleTapListener.onDoubleTapEvent(ev):false);
            } else if (mAlwaysInTapRegion) {
                const int deltaX = (int) (focusX - mDownFocusX);
                const int deltaY = (int) (focusY - mDownFocusY);
                int distance = (deltaX * deltaX) + (deltaY * deltaY);
                int slopSquare = isGeneratedGesture ? 0 : mTouchSlopSquare;

                const bool ambiguousGesture = motionClassification == MotionEvent::CLASSIFICATION_AMBIGUOUS_GESTURE;
                const bool shouldInhibitDefaultAction = hasPendingLongPress && ambiguousGesture;
                if (shouldInhibitDefaultAction) {
                    // Inhibit default long press
                    if (distance > slopSquare) {
                        // The default action here is to remove long press. But if the touch
                        // slop below gets increased, and we never exceed the modified touch
                        // slop while still receiving AMBIGUOUS_GESTURE, we risk that *nothing*
                        // will happen in response to user input. To prevent this,
                        // reschedule long press with a modified timeout.
                        const long longPressTimeout = ViewConfiguration::getLongPressTimeout();
                        //mHandler->removeMessages(LONG_PRESS);
                        //mHandler->sendMessageAtTime(mHandler->obtainMessage(LONG_PRESS,TOUCH_GESTURE_CLASSIFIED__CLASSIFICATION__LONG_PRESS,0 /* arg2 */),
                        //        ev.getDownTime() + (long) (longPressTimeout * mAmbiguousGestureMultiplier));
                        Choreographer::getInstance().removeCallbacks(Choreographer::CALLBACK_ANIMATION,&mLongPressRunnable,this);
                        Choreographer::getInstance().postCallbackDelayed(Choreographer::CALLBACK_ANIMATION,mLongPressRunnable,
                                this,(longPressTimeout * mAmbiguousGestureMultiplier));
                    }
                    // Inhibit default scroll. If a gesture is ambiguous, we prevent scroll
                    // until the gesture is resolved.
                    // However, for safety, simply increase the touch slop in case the
                    // classification is erroneous. Since the value is squared, multiply twice.
                    slopSquare *= mAmbiguousGestureMultiplier * mAmbiguousGestureMultiplier;
                }

                if (distance > slopSquare) {
                    recordGestureClassification(TOUCH_GESTURE_CLASSIFIED__CLASSIFICATION__SCROLL);
                    handled = (mListener.onScroll?mListener.onScroll(mCurrentDownEvent, ev, scrollX, scrollY):false);
                    mLastFocusX = focusX;
                    mLastFocusY = focusY;
                    mAlwaysInTapRegion = false;
                    //mHandler->removeMessages(TAP);
                    //mHandler->removeMessages(SHOW_PRESS);
                    //mHandler->removeMessages(LONG_PRESS);
                    Choreographer::getInstance().removeCallbacks(Choreographer::CALLBACK_ANIMATION,nullptr,this);
                }
                int doubleTapSlopSquare = isGeneratedGesture ? 0 : mDoubleTapTouchSlopSquare;
                if (distance > doubleTapSlopSquare) {
                    mAlwaysInBiggerTapRegion = false;
                }
            } else if ((std::abs(scrollX) >= 1) || (std::abs(scrollY) >= 1)) {
                recordGestureClassification(TOUCH_GESTURE_CLASSIFIED__CLASSIFICATION__SCROLL);
                handled = (mListener.onScroll?mListener.onScroll(mCurrentDownEvent, ev, scrollX, scrollY):false);
                mLastFocusX = focusX;
                mLastFocusY = focusY;
            }
            if ((motionClassification == MotionEvent::CLASSIFICATION_DEEP_PRESS) && hasPendingLongPress) {
                //mHandler->removeMessages(LONG_PRESS);
                //mHandler->sendMessage(mHandler->obtainMessage(LONG_PRESS,TOUCH_GESTURE_CLASSIFIED__CLASSIFICATION__DEEP_PRESS,0 /* arg2 */));
                Choreographer::getInstance().removeCallbacks(Choreographer::CALLBACK_ANIMATION,&mLongPressRunnable,this);
                Choreographer::getInstance().postCallback(Choreographer::CALLBACK_ANIMATION,mLongPressRunnable,this);
            }
        }
        break;

    case MotionEvent::ACTION_UP:{
            mStillDown = false;
            MotionEvent* currentUpEvent = MotionEvent::obtain(ev);
            if (mIsDoubleTapping) {
                // Finally, give the up event of the double-tap
                recordGestureClassification(TOUCH_GESTURE_CLASSIFIED__CLASSIFICATION__DOUBLE_TAP);
                handled |= (mDoubleTapListener.onDoubleTapEvent?mDoubleTapListener.onDoubleTapEvent(ev):false);
            } else if (mInLongPress) {
                //mHandler->removeMessages(TAP);
                Choreographer::getInstance().removeCallbacks(Choreographer::CALLBACK_ANIMATION,&mTapRunnable,this);
                mInLongPress = false;
            } else if (mAlwaysInTapRegion && !mIgnoreNextUpEvent) {
                recordGestureClassification(TOUCH_GESTURE_CLASSIFIED__CLASSIFICATION__SINGLE_TAP);
                handled = (mListener.onSingleTapUp?mListener.onSingleTapUp(ev):false);
                if (mDeferConfirmSingleTap && mDoubleTapListener.onSingleTapConfirmed != nullptr) {
                    mDoubleTapListener.onSingleTapConfirmed(ev);
                }
            } else if (!mIgnoreNextUpEvent) {

                // A fling must travel the minimum tap distance
                const int pointerId = ev.getPointerId(0);
                mVelocityTracker->computeCurrentVelocity(1000, mMaximumFlingVelocity);
                const float velocityY = mVelocityTracker->getYVelocity(pointerId);
                const float velocityX = mVelocityTracker->getXVelocity(pointerId);

                if ((std::abs(velocityY) > mMinimumFlingVelocity) || (std::abs(velocityX) > mMinimumFlingVelocity)) {
                    handled = (mListener.onFling?mListener.onFling(mCurrentDownEvent, ev, velocityX, velocityY):false);
                }
            }
            if (mPreviousUpEvent) {
                mPreviousUpEvent->recycle();
            }
            // Hold the event we obtained above - listeners may have changed the original.
            mPreviousUpEvent = currentUpEvent;
        }
        if (mVelocityTracker) {
            // This may have been cleared when we called out to the
            // application above.
            mVelocityTracker->recycle();
            mVelocityTracker = nullptr;
        }
        mIsDoubleTapping = false;
        mDeferConfirmSingleTap = false;
        mIgnoreNextUpEvent = false;
        //mHandler->removeMessages(SHOW_PRESS); mHandler->removeMessages(LONG_PRESS);
        Choreographer::getInstance().removeCallbacks(Choreographer::CALLBACK_ANIMATION,&mPressRunnable,this);
        Choreographer::getInstance().removeCallbacks(Choreographer::CALLBACK_ANIMATION,&mLongPressRunnable,this);
        break;

    case MotionEvent::ACTION_CANCEL:
        cancel();
        break;
    }

    if (!handled && mInputEventConsistencyVerifier) {
        mInputEventConsistencyVerifier->onUnhandledEvent(ev, 0);
    }
    return handled;
}

bool GestureDetector::onGenericMotionEvent(MotionEvent& ev) {
    if (mInputEventConsistencyVerifier) {
        mInputEventConsistencyVerifier->onGenericMotionEvent(ev, 0);
    }

    const int actionButton = ev.getActionButton();
    switch (ev.getActionMasked()) {
    case MotionEvent::ACTION_BUTTON_PRESS:
        if (mContextClickListener && !mInContextClick && !mInLongPress
                && (actionButton == MotionEvent::BUTTON_STYLUS_PRIMARY
                || actionButton == MotionEvent::BUTTON_SECONDARY)) {
            if (mContextClickListener(ev)) {
                mInContextClick = true;
                //mHandler->removeMessages(LONG_PRESS); mHandler->removeMessages(TAP);
                Choreographer::getInstance().removeCallbacks(Choreographer::CALLBACK_ANIMATION,&mPressRunnable,this);
                Choreographer::getInstance().removeCallbacks(Choreographer::CALLBACK_ANIMATION,&mTapRunnable,this);
                return true;
            }
        }
        break;

    case MotionEvent::ACTION_BUTTON_RELEASE:
        if (mInContextClick && (actionButton == MotionEvent::BUTTON_STYLUS_PRIMARY
                || actionButton == MotionEvent::BUTTON_SECONDARY)) {
            mInContextClick = false;
            mIgnoreNextUpEvent = true;
        }
        break;
    }
    return false;
}

void GestureDetector::cancel() {
    //mHandler->removeMessages(SHOW_PRESS);
    //mHandler->removeMessages(LONG_PRESS);
    //mHandler->removeMessages(TAP);
    Choreographer::getInstance().removeCallbacks(Choreographer::CALLBACK_ANIMATION,nullptr,this);
    mVelocityTracker->recycle();
    mVelocityTracker = nullptr;
    mIsDoubleTapping = false;
    mStillDown = false;
    mAlwaysInTapRegion = false;
    mAlwaysInBiggerTapRegion = false;
    mDeferConfirmSingleTap = false;
    mInLongPress = false;
    mInContextClick = false;
    mIgnoreNextUpEvent = false;
}

void GestureDetector::cancelTaps() {
    //mHandler->removeMessages(SHOW_PRESS);
    //mHandler->removeMessages(LONG_PRESS);
    //mHandler->removeMessages(TAP);
    Choreographer::getInstance().removeCallbacks(Choreographer::CALLBACK_ANIMATION,nullptr,this);
    mIsDoubleTapping = false;
    mAlwaysInTapRegion = false;
    mAlwaysInBiggerTapRegion = false;
    mDeferConfirmSingleTap = false;
    mInLongPress = false;
    mInContextClick = false;
    mIgnoreNextUpEvent = false;
}

bool GestureDetector::isConsideredDoubleTap(MotionEvent& firstDown, MotionEvent& firstUp,MotionEvent& secondDown) {
    if (!mAlwaysInBiggerTapRegion) {
        return false;
    }

    const long deltaTime = secondDown.getEventTime() - firstUp.getEventTime();
    if (deltaTime > DOUBLE_TAP_TIMEOUT || deltaTime < DOUBLE_TAP_MIN_TIME) {
        return false;
    }

    const int deltaX = (int) firstDown.getX() - (int) secondDown.getX();
    const int deltaY = (int) firstDown.getY() - (int) secondDown.getY();
    const bool isGeneratedGesture = (firstDown.getFlags() & MotionEvent::FLAG_IS_GENERATED_GESTURE) != 0;
    int slopSquare = isGeneratedGesture ? 0 : mDoubleTapSlopSquare;
    return (deltaX * deltaX + deltaY * deltaY < slopSquare);
}

void GestureDetector::dispatchLongPress() {
    //mHandler->removeMessages(TAP);
    Choreographer::getInstance().removeCallbacks(Choreographer::CALLBACK_ANIMATION,&mTapRunnable,this);
    mDeferConfirmSingleTap = false;
    mInLongPress = true;
    if(mListener.onLongPress)
        mListener.onLongPress(*mCurrentDownEvent);
}

void GestureDetector::recordGestureClassification(int classification) {
    if (mHasRecordedClassification || (classification == TOUCH_GESTURE_CLASSIFIED__CLASSIFICATION__UNKNOWN_CLASSIFICATION)) {
        // Only record the first classification for an event stream.
        return;
    }
    if ((mCurrentDownEvent == nullptr) || (mCurrentMotionEvent == nullptr)) {
        // If the complete event stream wasn't seen, don't record anything.
        mHasRecordedClassification = true;
        return;
    }
    /*FrameworkStatsLog.write( FrameworkStatsLog.TOUCH_GESTURE_CLASSIFIED,
            getClass().getName(), classification,
            (int) (SystemClock::uptimeMillis() - mCurrentMotionEvent.getDownTime()),
            (float) std::hypot(mCurrentMotionEvent.getRawX() - mCurrentDownEvent.getRawX(),
                               mCurrentMotionEvent.getRawY() - mCurrentDownEvent.getRawY()));*/
    mHasRecordedClassification = true;
}
}/*endof namespace*/
