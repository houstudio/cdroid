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
#include <view/scalegesturedetector.h>
namespace cdroid{

ScaleGestureDetector::ScaleGestureDetector(Context* context,const OnScaleGestureListener& listener) {
    mContext = context;
    mListener = listener;
    mInProgress = false;
    mQuickScaleEnabled = false;
    mGestureDetector = nullptr;
    ViewConfiguration& viewConfiguration = ViewConfiguration::get(context);
    mInputEventConsistencyVerifier = InputEventConsistencyVerifier::isInstrumentationEnabled() ?
                new InputEventConsistencyVerifier((Object*)this, 0) : nullptr;
    mSpanSlop = viewConfiguration.getScaledTouchSlop() * 2;
    mMinSpan = viewConfiguration.getScaledMinimumScalingSpan();
    //mHandler = handler;
    // Quick scale is enabled by default after JB_MR2
    setQuickScaleEnabled(true);
    // Stylus scale is enabled by default after LOLLIPOP_MR1
    setStylusScaleEnabled(true);
}

ScaleGestureDetector::~ScaleGestureDetector(){
    delete mGestureDetector;
    delete mInputEventConsistencyVerifier;
}

bool ScaleGestureDetector::onTouchEvent(MotionEvent& event) {
    if (mInputEventConsistencyVerifier) {
        mInputEventConsistencyVerifier->onTouchEvent(event, 0);
    }

    mCurrTime = event.getEventTime();

    const int action = event.getActionMasked();

    // Forward the event to check for double tap gesture
    if (mQuickScaleEnabled) {
        mGestureDetector->onTouchEvent(event);
    }

    const int count = event.getPointerCount();
    const bool isStylusButtonDown = (event.getButtonState() & MotionEvent::BUTTON_STYLUS_PRIMARY) != 0;

    const bool anchoredScaleCancelled = (mAnchoredScaleMode == ANCHORED_SCALE_MODE_STYLUS) && !isStylusButtonDown;
    const bool streamComplete = (action == MotionEvent::ACTION_UP) || (action == MotionEvent::ACTION_CANCEL) || anchoredScaleCancelled;

    if ((action == MotionEvent::ACTION_DOWN) || streamComplete) {
        // Reset any scale in progress with the listener.
        // If it's an ACTION_DOWN we're beginning a new event stream.
        // This means the app probably didn't give us all the events. Shame on it.
        if (mInProgress) {
            if(mListener.onScaleEnd)mListener.onScaleEnd(*this);
            mInProgress = false;
            mInitialSpan = 0;
            mAnchoredScaleMode = ANCHORED_SCALE_MODE_NONE;
        } else if (inAnchoredScaleMode() && streamComplete) {
            mInProgress = false;
            mInitialSpan = 0;
            mAnchoredScaleMode = ANCHORED_SCALE_MODE_NONE;
        }

        if (streamComplete) {
            return true;
        }
    }

    if (!mInProgress && mStylusScaleEnabled && !inAnchoredScaleMode()
            && !streamComplete && isStylusButtonDown) {
        // Start of a button scale gesture
        mAnchoredScaleStartX = event.getX();
        mAnchoredScaleStartY = event.getY();
        mAnchoredScaleMode = ANCHORED_SCALE_MODE_STYLUS;
        mInitialSpan = 0;
    }

    const bool configChanged = (action == MotionEvent::ACTION_DOWN) ||(action == MotionEvent::ACTION_POINTER_UP)
           ||(action == MotionEvent::ACTION_POINTER_DOWN) || anchoredScaleCancelled;

    const bool pointerUp = action == MotionEvent::ACTION_POINTER_UP;
    const int skipIndex = pointerUp ? event.getActionIndex() : -1;

    // Determine focal point
    float sumX = 0, sumY = 0;
    const int div = pointerUp ? count - 1 : count;
    float focusX;
    float focusY;
    if (inAnchoredScaleMode()) {
        // In anchored scale mode, the focal pt is always where the double tap
        // or button down gesture started
        focusX = mAnchoredScaleStartX;
        focusY = mAnchoredScaleStartY;
        if (event.getY() < focusY) {
            mEventBeforeOrAboveStartingGestureEvent = true;
        } else {
            mEventBeforeOrAboveStartingGestureEvent = false;
        }
    } else {
        for (int i = 0; i < count; i++) {
            if (skipIndex == i) continue;
            sumX += event.getX(i);
            sumY += event.getY(i);
        }

        focusX = sumX / div;
        focusY = sumY / div;
    }

    // Determine average deviation from focal point
    float devSumX = 0, devSumY = 0;
    for (int i = 0; i < count; i++) {
        if (skipIndex == i) continue;

        // Convert the resulting diameter into a radius.
        devSumX += std::abs(event.getX(i) - focusX);
        devSumY += std::abs(event.getY(i) - focusY);
    }
    const float devX = devSumX / div;
    const float devY = devSumY / div;

    // Span is the average distance between touch points through the focal point;
    // i.e. the diameter of the circle with a radius of the average deviation from
    // the focal point.
    const float spanX = devX * 2;
    const float spanY = devY * 2;
    float span;
    if (inAnchoredScaleMode()) {
        span = spanY;
    } else {
        span = (float) std::hypot(spanX, spanY);
    }

    // Dispatch begin/end events as needed.
    // If the configuration changes, notify the app to reset its current state by beginning
    // a fresh scale event stream.
    const bool wasInProgress = mInProgress;
    mFocusX = focusX;
    mFocusY = focusY;
    if (!inAnchoredScaleMode() && mInProgress && (span < mMinSpan || configChanged)) {
        if(mListener.onScaleEnd) mListener.onScaleEnd(*this);
        mInProgress = false;
        mInitialSpan = span;
    }
    if (configChanged) {
        mPrevSpanX = mCurrSpanX = spanX;
        mPrevSpanY = mCurrSpanY = spanY;
        mInitialSpan = mPrevSpan = mCurrSpan = span;
    }

    const int minSpan = inAnchoredScaleMode() ? mSpanSlop : mMinSpan;
    if (!mInProgress && (span >=  minSpan) && (wasInProgress || (std::abs(span - mInitialSpan) > mSpanSlop))) {
        mPrevSpanX = mCurrSpanX = spanX;
        mPrevSpanY = mCurrSpanY = spanY;
        mPrevSpan = mCurrSpan = span;
        mPrevTime = mCurrTime;
        mInProgress = (mListener.onScaleBegin?mListener.onScaleBegin(*this):true);
    }

    // Handle motion; focal point and span/scale factor are changing.
    if (action == MotionEvent::ACTION_MOVE) {
        mCurrSpanX = spanX;
        mCurrSpanY = spanY;
        mCurrSpan = span;

        bool updatePrev = true;

        if (mInProgress) {
            updatePrev = (mListener.onScale?mListener.onScale(*this):false);
        }

        if (updatePrev) {
            mPrevSpanX = mCurrSpanX;
            mPrevSpanY = mCurrSpanY;
            mPrevSpan = mCurrSpan;
            mPrevTime = mCurrTime;
        }
    }

    return true;
}

bool ScaleGestureDetector::inAnchoredScaleMode()const {
    return mAnchoredScaleMode != ANCHORED_SCALE_MODE_NONE;
}

void ScaleGestureDetector::setQuickScaleEnabled(bool scales) {
    mQuickScaleEnabled = scales;
    if (mQuickScaleEnabled && (mGestureDetector == nullptr)) {
        GestureDetector::OnGestureListener gestureListener;
        GestureDetector::OnDoubleTapListener doubleTapListener;
        doubleTapListener.onDoubleTap = [this](MotionEvent&e){
            // Double tap: start watching for a swipe
            mAnchoredScaleStartX = e.getX();
            mAnchoredScaleStartY = e.getY();
            mAnchoredScaleMode = ANCHORED_SCALE_MODE_DOUBLE_TAP;
            return true;
        };
        mGestureDetector = new GestureDetector(mContext, gestureListener);
        mGestureDetector->setOnDoubleTapListener(doubleTapListener);
    }
}

bool ScaleGestureDetector::isQuickScaleEnabled()const {
    return mQuickScaleEnabled;
}

void ScaleGestureDetector::setStylusScaleEnabled(bool scales) {
    mStylusScaleEnabled = scales;
}

bool ScaleGestureDetector::isStylusScaleEnabled()const {
    return mStylusScaleEnabled;
}

bool ScaleGestureDetector::isInProgress()const {
    return mInProgress;
}

float ScaleGestureDetector::getFocusX() const{
    return mFocusX;
}

float ScaleGestureDetector::getFocusY() const{
    return mFocusY;
}

float ScaleGestureDetector::getCurrentSpan()const {
    return mCurrSpan;
}

float ScaleGestureDetector::getCurrentSpanX() const{
    return mCurrSpanX;
}

float ScaleGestureDetector::getCurrentSpanY() const{
    return mCurrSpanY;
}

float ScaleGestureDetector::getPreviousSpan() const{
    return mPrevSpan;
}

float ScaleGestureDetector::getPreviousSpanX() const{
    return mPrevSpanX;
}

float ScaleGestureDetector::getPreviousSpanY() const{
    return mPrevSpanY;
}

float ScaleGestureDetector::getScaleFactor() const{
    if (inAnchoredScaleMode()) {
        // Drag is moving up; the further away from the gesture
        // start, the smaller the span should be, the closer,
        // the larger the span, and therefore the larger the scale
        const bool scaleUp = (mEventBeforeOrAboveStartingGestureEvent && (mCurrSpan < mPrevSpan)) ||
                (!mEventBeforeOrAboveStartingGestureEvent && (mCurrSpan > mPrevSpan));
        const float spanDiff = (std::abs(1 - (mCurrSpan / mPrevSpan)) * SCALE_FACTOR);
        return mPrevSpan <= mSpanSlop ? 1 : scaleUp ? (1 + spanDiff) : (1 - spanDiff);
    }
    return mPrevSpan > 0 ? mCurrSpan / mPrevSpan : 1;
}

long ScaleGestureDetector::getTimeDelta() const{
    return mCurrTime - mPrevTime;
}

int64_t ScaleGestureDetector::getEventTime() const{
    return mCurrTime;
}
}
