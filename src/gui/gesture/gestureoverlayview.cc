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
#include <gesture/gestureoverlayview.h>
namespace cdroid{
DECLARE_WIDGET(GestureOverlayView)
GestureOverlayView::GestureOverlayView(Context* context,const AttributeSet& attrs)
  :FrameLayout(context, attrs){

    init();
    mGestureStrokeWidth = attrs.getFloat("gestureStrokeWidth", mGestureStrokeWidth);
    mInvalidateExtraBorder = std::max(1, int(mGestureStrokeWidth - 1));
    mCertainGestureColor = attrs.getColor("gestureColor",mCertainGestureColor);
    mUncertainGestureColor = attrs.getColor("uncertainGestureColor",mUncertainGestureColor);
    mFadeDuration = attrs.getInt("fadeDuration", (int) mFadeDuration);
    mFadeOffset = attrs.getInt("fadeOffset", (int) mFadeOffset);
    mGestureStrokeType = attrs.getInt("gestureStrokeType",std::unordered_map<std::string,int>{
            {"single",(int)GESTURE_STROKE_TYPE_SINGLE},
            {"multiple",(int)GESTURE_STROKE_TYPE_MULTIPLE}}, mGestureStrokeType);
    mGestureStrokeLengthThreshold = attrs.getFloat("gestureStrokeLengthThreshold",  mGestureStrokeLengthThreshold);
    mGestureStrokeAngleThreshold = attrs.getFloat("gestureStrokeAngleThreshold", mGestureStrokeAngleThreshold);
    mGestureStrokeSquarenessTreshold = attrs.getFloat("gestureStrokeSquarenessThreshold", mGestureStrokeSquarenessTreshold);
    mInterceptEvents = attrs.getBoolean("eventsInterceptionEnabled", mInterceptEvents);
    mFadeEnabled = attrs.getBoolean("fadeEnabled", mFadeEnabled);
    mOrientation = attrs.getInt("orientation", mOrientation);
}

void GestureOverlayView::init() {
    setWillNotDraw(false);
    mFadingOut = std::bind(&GestureOverlayView::FadeOutProc,this);
    mInterpolator = AccelerateDecelerateInterpolator::Instance;
    mCurrentColor = mCertainGestureColor;
    mFadeDuration = 150;
    mFadeOffset  = 420;
    mFadingAlpha = 1.0f;
    mIsGesturing = false;
    mFadeEnabled = true;
    mIsFadingOut = false;
    mResetGesture= false;
    mInterceptEvents= true;
    mGestureVisible = true;
    mFadingHasStarted = false;
    mResetMultipleStrokes = false;
    mPreviousWasGesturing = false;
    mHandleGestureActions = false;
    mGestureStrokeType = GESTURE_STROKE_TYPE_SINGLE;
    mCertainGestureColor = 0xFFFFFF00;
    mUncertainGestureColor = 0x48FFFF00;
    mInvalidateExtraBorder = 10;
    mGestureStrokeWidth=12.f;
    mGestureStrokeLengthThreshold = 50.0f;
    mGestureStrokeSquarenessTreshold = 0.275f;
    mCurrentGesture = nullptr;
    setPaintAlpha(255);
}

GestureOverlayView::~GestureOverlayView(){
    delete mCurrentGesture;
}

const std::vector<GesturePoint>& GestureOverlayView::getCurrentStroke() const{
    return mStrokeBuffer;
}

int GestureOverlayView::getOrientation() const{
    return mOrientation;
}

void GestureOverlayView::setOrientation(int orientation) {
    mOrientation = orientation;
}

void GestureOverlayView::setGestureColor(int color) {
    mCertainGestureColor = color;
}

void GestureOverlayView::setUncertainGestureColor(int color) {
    mUncertainGestureColor = color;
}

int GestureOverlayView::getUncertainGestureColor() const{
    return mUncertainGestureColor;
}

int GestureOverlayView::getGestureColor() const{
    return mCertainGestureColor;
}

float GestureOverlayView::getGestureStrokeWidth() const{
    return mGestureStrokeWidth;
}

void GestureOverlayView::setGestureStrokeWidth(float gestureStrokeWidth) {
    mGestureStrokeWidth = gestureStrokeWidth;
    mInvalidateExtraBorder = std::max(1, int(gestureStrokeWidth - 1));
}

int GestureOverlayView::getGestureStrokeType() const{
    return mGestureStrokeType;
}

void GestureOverlayView::setGestureStrokeType(int gestureStrokeType) {
    mGestureStrokeType = gestureStrokeType;
}

float GestureOverlayView::getGestureStrokeLengthThreshold() const{
    return mGestureStrokeLengthThreshold;
}

void GestureOverlayView::setGestureStrokeLengthThreshold(float gestureStrokeLengthThreshold) {
    mGestureStrokeLengthThreshold = gestureStrokeLengthThreshold;
}

float GestureOverlayView::getGestureStrokeSquarenessTreshold() const{
    return mGestureStrokeSquarenessTreshold;
}

void GestureOverlayView::setGestureStrokeSquarenessTreshold(float gestureStrokeSquarenessTreshold) {
    mGestureStrokeSquarenessTreshold = gestureStrokeSquarenessTreshold;
}

float GestureOverlayView::getGestureStrokeAngleThreshold() const{
    return mGestureStrokeAngleThreshold;
}

void GestureOverlayView::setGestureStrokeAngleThreshold(float gestureStrokeAngleThreshold) {
    mGestureStrokeAngleThreshold = gestureStrokeAngleThreshold;
}

bool GestureOverlayView::isEventsInterceptionEnabled() const{
    return mInterceptEvents;
}

void GestureOverlayView::setEventsInterceptionEnabled(bool enabled) {
    mInterceptEvents = enabled;
}

bool GestureOverlayView::isFadeEnabled() const{
    return mFadeEnabled;
}

void GestureOverlayView::setFadeEnabled(bool fadeEnabled) {
    mFadeEnabled = fadeEnabled;
}

Gesture* GestureOverlayView::getGesture() const{
    return mCurrentGesture;
}

void GestureOverlayView::setGesture(Gesture* gesture) {
    if (mCurrentGesture != nullptr) {
        clear(false);
    }

    setCurrentColor(mCertainGestureColor);
    mCurrentGesture = gesture;

    Path* path = mCurrentGesture->toPath();
    RectF bounds;
    path->compute_bounds(bounds,true);//computeBounds(bounds, true);

    // TODO: The path should also be scaled to fit inside this view
    mPath.reset();//rewind();
    mPath.append_path(*path, -bounds.left + (getWidth() - bounds.width) / 2.0f,
            -bounds.top + (getHeight() - bounds.height) / 2.0f);

    mResetGesture = true;

    invalidate();
}

Path GestureOverlayView::getGesturePath() {
    return mPath;
}

Path GestureOverlayView::getGesturePath(Path& path) {
    path.reset();//
    path.append_path(mPath);
    return path;
}

bool GestureOverlayView::isGestureVisible() const{
    return mGestureVisible;
}

void GestureOverlayView::setGestureVisible(bool visible) {
    mGestureVisible = visible;
}

long GestureOverlayView::getFadeOffset() const{
    return mFadeOffset;
}

void GestureOverlayView::setFadeOffset(long fadeOffset) {
    mFadeOffset = fadeOffset;
}

void GestureOverlayView::addOnGestureListener(const OnGestureListener& listener) {
    mOnGestureListeners.push_back(listener);
}

void GestureOverlayView::removeOnGestureListener(const OnGestureListener& listener) {
    auto it = std::find(mOnGestureListeners.begin(), mOnGestureListeners.end(),listener);
    if(it !=  mOnGestureListeners.end())
        mOnGestureListeners.erase(it);
}

void GestureOverlayView::removeAllOnGestureListeners() {
    mOnGestureListeners.clear();
}

void GestureOverlayView::addOnGesturePerformedListener(const OnGesturePerformedListener& listener) {
    mOnGesturePerformedListeners.push_back(listener);
    if (mOnGesturePerformedListeners.size() > 0) {
        mHandleGestureActions = true;
    }
}

void GestureOverlayView::removeOnGesturePerformedListener(const OnGesturePerformedListener& listener) {
    auto it = std::find(mOnGesturePerformedListeners.begin(),mOnGesturePerformedListeners.end(),listener);
    if(it!=mOnGesturePerformedListeners.end())
        mOnGesturePerformedListeners.erase(it);
    if (mOnGesturePerformedListeners.size() <= 0) {
        mHandleGestureActions = false;
    }
}

void GestureOverlayView::removeAllOnGesturePerformedListeners() {
    mOnGesturePerformedListeners.clear();
    mHandleGestureActions = false;
}

void GestureOverlayView::addOnGesturingListener(const OnGesturingListener& listener) {
    mOnGesturingListeners.push_back(listener);
}

void GestureOverlayView::removeOnGesturingListener(const OnGesturingListener& listener) {
    auto it = std::find(mOnGesturingListeners.begin(),mOnGesturingListeners.end(),listener);
    if(it!=mOnGesturingListeners.end())
        mOnGesturingListeners.erase(it);
}

void GestureOverlayView::removeAllOnGesturingListeners() {
    mOnGesturingListeners.clear();
}

bool GestureOverlayView::isGesturing() const{
    return mIsGesturing;
}

void GestureOverlayView::setCurrentColor(int color) {
    mCurrentColor = color;
    if (mFadingHasStarted) {
        setPaintAlpha(int(255 * mFadingAlpha));
    } else {
        setPaintAlpha(255);
    }
    invalidate();
}

void GestureOverlayView::draw(Canvas& canvas) {
    FrameLayout::draw(canvas);
    if ((mCurrentGesture != nullptr) && mGestureVisible) {
        Color pc(mCurrentColor);
        pc.setAlpha(uint8_t(mPaintAlpha));
        canvas.set_source_rgba(pc.red(),pc.green(),pc.blue(),pc.alpha());
        canvas.set_line_width(mGestureStrokeWidth);
        canvas.set_line_cap(Cairo::Context::LineCap::ROUND);
        canvas.set_line_join(Cairo::Context::LineJoin::ROUND);
        //canvas.drawPath(mPath, mGesturePaint);
        mPath.append_to_context(&canvas);
        canvas.stroke();
    }
}

void GestureOverlayView::setPaintAlpha(int alpha) {
    alpha += alpha >> 7;
    const int baseAlpha = mCurrentColor >> 24;
    const int useAlpha = baseAlpha * alpha >> 8;
    mPaintAlpha = useAlpha;
}

void GestureOverlayView::clear(bool animated) {
    clear(animated, false, true);
}

void GestureOverlayView::clear(bool animated, bool fireActionPerformed, bool immediate) {
    setPaintAlpha(255);
    removeCallbacks(mFadingOut);
    mResetGesture = false;
    mFireActionPerformed = fireActionPerformed;
    mResetMultipleStrokes = false;

    if (animated && mCurrentGesture) {
        mFadingAlpha = 1.0f;
        mIsFadingOut = true;
        mFadingHasStarted = false;
        mFadingStart = AnimationUtils::currentAnimationTimeMillis() + mFadeOffset;

        postDelayed(mFadingOut, mFadeOffset);
    } else {
        mFadingAlpha = 1.0f;
        mIsFadingOut = false;
        mFadingHasStarted = false;

        if (immediate) {
            delete mCurrentGesture;
            mCurrentGesture = nullptr;
            mPath.reset();//rewind();
            invalidate();
        } else if (fireActionPerformed) {
            postDelayed(mFadingOut, mFadeOffset);
        } else if (mGestureStrokeType == GESTURE_STROKE_TYPE_MULTIPLE) {
            mResetMultipleStrokes = true;
            postDelayed(mFadingOut, mFadeOffset);
        } else {
            delete mCurrentGesture;
            mCurrentGesture = nullptr;
            mPath.reset();//rewind();
            invalidate();
        }
    }
}

void GestureOverlayView::cancelClearAnimation() {
    setPaintAlpha(255);
    mIsFadingOut = false;
    mFadingHasStarted = false;
    removeCallbacks(mFadingOut);
    mPath.reset();//rewind();
    mCurrentGesture = nullptr;
}

void GestureOverlayView::cancelGesture() {
    mIsListeningForGestures = false;

    // add the stroke to the current gesture
    mCurrentGesture->addStroke(new GestureStroke(mStrokeBuffer));

    // pass the event to handlers
    const auto now = SystemClock::uptimeMillis();
    MotionEvent* event = MotionEvent::obtain(now, now, MotionEvent::ACTION_CANCEL, 0.0f, 0.0f, 0);

    std::vector<OnGestureListener>& listeners = mOnGestureListeners;
    size_t count = listeners.size();
    for (size_t i = 0; i < count; i++) {
        listeners.at(i).onGestureCancelled(*this, *event);
    }

    event->recycle();

    clear(false);
    mIsGesturing = false;
    mPreviousWasGesturing = false;
    mStrokeBuffer.clear();

    std::vector<OnGesturingListener> otherListeners = mOnGesturingListeners;
    count = otherListeners.size();
    for (size_t i = 0; i < count; i++) {
        otherListeners.at(i).onGesturingEnded(*this);
    }
}

void GestureOverlayView::onDetachedFromWindow() {
    FrameLayout::onDetachedFromWindow();
    cancelClearAnimation();
}

bool GestureOverlayView::dispatchTouchEvent(MotionEvent& event) {
    if (isEnabled()) {
        const bool cancelDispatch = (mIsGesturing || (mCurrentGesture &&
                mCurrentGesture->getStrokesCount() > 0 && mPreviousWasGesturing)) &&
                mInterceptEvents;

        processEvent(event);
        if (cancelDispatch) {
            event.setAction(MotionEvent::ACTION_CANCEL);
        }
        FrameLayout::dispatchTouchEvent(event);
        return true;
    }

    return FrameLayout::dispatchTouchEvent(event);
}

bool GestureOverlayView::processEvent(MotionEvent& event) {
    switch (event.getAction()) {
    case MotionEvent::ACTION_DOWN:
        touchDown(event);
        invalidate();
        return true;
    case MotionEvent::ACTION_MOVE:
        if (mIsListeningForGestures) {
            const Rect rect = touchMove(event);
            if (!rect.empty()) {
                invalidate(rect);
            }
            return true;
        }
        break;
    case MotionEvent::ACTION_UP:
        if (mIsListeningForGestures) {
            touchUp(event, false);
            invalidate();
            return true;
        }
        break;
    case MotionEvent::ACTION_CANCEL:
        if (mIsListeningForGestures) {
            touchUp(event, true);
            invalidate();
            return true;
        }
    }

    return false;
}

void GestureOverlayView::touchDown(MotionEvent& event) {
    mIsListeningForGestures = true;

    float x = event.getX();
    float y = event.getY();

    mX = x;
    mY = y;

    mTotalLength = 0;
    mIsGesturing = false;

    if (mGestureStrokeType == GESTURE_STROKE_TYPE_SINGLE || mResetGesture) {
        if (mHandleGestureActions) setCurrentColor(mUncertainGestureColor);
        mResetGesture = false;
        delete mCurrentGesture;
        mCurrentGesture = nullptr;
        mPath.reset();//rewind();
    } else if ((mCurrentGesture == nullptr) || (mCurrentGesture->getStrokesCount() == 0)) {
        if (mHandleGestureActions) setCurrentColor(mUncertainGestureColor);
    }

    // if there is fading out going on, stop it.
    if (mFadingHasStarted) {
        cancelClearAnimation();
    } else if (mIsFadingOut) {
        setPaintAlpha(255);
        mIsFadingOut = false;
        mFadingHasStarted = false;
        removeCallbacks(mFadingOut);
    }

    if (mCurrentGesture == nullptr) {
        mCurrentGesture = new Gesture();
    }

    mStrokeBuffer.push_back(GesturePoint(x, y, event.getEventTime()));
    mPath.move_to(x, y);

    const int border = mInvalidateExtraBorder;
    mInvalidRect.set(int(x - border), int(y - border), int(x + border), int(y + border));

    mCurveEndX = x;
    mCurveEndY = y;

    // pass the event to handlers
    std::vector<OnGestureListener> listeners = mOnGestureListeners;
    const size_t count = listeners.size();
    for (size_t i = 0; i < count; i++) {
        listeners.at(i).onGestureStarted(*this, event);
    }
}

Rect GestureOverlayView::touchMove(MotionEvent& event) {
    Rect areaToRefresh={0,0,0,0};

    const float x = event.getX();
    const float y = event.getY();

    const float previousX = mX;
    const float previousY = mY;

    const float dx = std::abs(x - previousX);
    const float dy = std::abs(y - previousY);

    if (dx >= GestureStroke::TOUCH_TOLERANCE || dy >= GestureStroke::TOUCH_TOLERANCE) {
        areaToRefresh = mInvalidRect;

        // start with the curve end
        const int border = mInvalidateExtraBorder;
        areaToRefresh.set(int(mCurveEndX - border), int(mCurveEndY - border),
                int(mCurveEndX + border), int(mCurveEndY + border));

        float cX = mCurveEndX = (x + previousX) / 2;
        float cY = mCurveEndY = (y + previousY) / 2;

        mPath.quad_to(previousX, previousY, cX, cY);

        // union with the control point of the new curve
        areaToRefresh.Union((int) previousX - border, (int) previousY - border,
                (int) previousX + border, (int) previousY + border);

        // union with the end point of the new curve
        areaToRefresh.Union((int) cX - border, (int) cY - border,
                (int) cX + border, (int) cY + border);

        mX = x;
        mY = y;

        mStrokeBuffer.push_back(GesturePoint(x, y, event.getEventTime()));

        if (mHandleGestureActions && !mIsGesturing) {
            mTotalLength += (float) std::hypot(dx, dy);

            if (mTotalLength > mGestureStrokeLengthThreshold) {
                OrientedBoundingBox* box =  GestureUtils::computeOrientedBoundingBox(mStrokeBuffer);

                float angle = std::abs(box->orientation);
                if (angle > 90.f) {
                    angle = 180.f - angle;
                }

                if (box->squareness > mGestureStrokeSquarenessTreshold ||
                        (mOrientation == ORIENTATION_VERTICAL ?
                                angle < mGestureStrokeAngleThreshold :
                                angle > mGestureStrokeAngleThreshold)) {

                    mIsGesturing = true;
                    setCurrentColor(mCertainGestureColor);

                    std::vector<OnGesturingListener> listeners = mOnGesturingListeners;
                    const size_t count = listeners.size();
                    for (size_t i = 0; i < count; i++) {
                        listeners.at(i).onGesturingStarted(*this);
                    }
                }
                delete box;
            }
        }

        // pass the event to handlers
        std::vector<OnGestureListener> listeners = mOnGestureListeners;
        const size_t count = listeners.size();
        for (size_t i = 0; i < count; i++) {
            listeners.at(i).onGesture(*this, event);
        }
    }

    return areaToRefresh;
}

void GestureOverlayView::touchUp(MotionEvent& event, bool cancel) {
    mIsListeningForGestures = false;

    // A gesture wasn't started or was cancelled
    if (mCurrentGesture != nullptr) {
        // add the stroke to the current gesture
        mCurrentGesture->addStroke(new GestureStroke(mStrokeBuffer));

        if (!cancel) {
            // pass the event to handlers
            std::vector<OnGestureListener> listeners = mOnGestureListeners;
            const size_t count = listeners.size();
            for (size_t i = 0; i < count; i++) {
                listeners.at(i).onGestureEnded(*this, event);
            }

            clear(mHandleGestureActions && mFadeEnabled, mHandleGestureActions && mIsGesturing,
                    false);
        } else {
            cancelGesture(event);

        }
    } else {
        cancelGesture(event);
    }

    mStrokeBuffer.clear();
    mPreviousWasGesturing = mIsGesturing;
    mIsGesturing = false;

    std::vector<OnGesturingListener> listeners = mOnGesturingListeners;
    const size_t count = listeners.size();
    for (size_t i = 0; i < count; i++) {
        listeners.at(i).onGesturingEnded(*this);
    }
}

void GestureOverlayView::cancelGesture(MotionEvent& event) {
    // pass the event to handlers
    std::vector<OnGestureListener> listeners = mOnGestureListeners;
    const size_t count = listeners.size();
    for (size_t i = 0; i < count; i++) {
        listeners.at(i).onGestureCancelled(*this, event);
    }
    clear(false);
}

void GestureOverlayView::fireOnGesturePerformed() {
    std::vector<OnGesturePerformedListener> actionListeners = mOnGesturePerformedListeners;
    const int count = actionListeners.size();
    for (int i = 0; i < count; i++) {
        actionListeners.at(i).onGesturePerformed(*this, *mCurrentGesture);
    }
}

void GestureOverlayView::FadeOutProc(){
    if (mIsFadingOut) {
        const int64_t now = AnimationUtils::currentAnimationTimeMillis();
        const long duration = now - mFadingStart;

        if (duration > mFadeDuration) {
            if (mFireActionPerformed) {
                fireOnGesturePerformed();
            }

            mPreviousWasGesturing = false;
            mIsFadingOut = false;
            mFadingHasStarted = false;
            mPath.reset();//rewind();
            delete mCurrentGesture;
            mCurrentGesture = nullptr;
            setPaintAlpha(255);
        } else {
            mFadingHasStarted = true;
            float interpolatedTime = std::max(0.0f, std::min(1.0f, duration / (float) mFadeDuration));
            mFadingAlpha = 1.0f - mInterpolator->getInterpolation(interpolatedTime);
            setPaintAlpha(int(255 * mFadingAlpha));
            postDelayed(mFadingOut, FADE_ANIMATION_RATE);
        }
    } else if (mResetMultipleStrokes) {
        mResetGesture = true;
    } else {
        fireOnGesturePerformed();

        mFadingHasStarted = false;
        mPath.reset();//rewind();
        delete mCurrentGesture;
        mCurrentGesture = nullptr;
        mPreviousWasGesturing = false;
        setPaintAlpha(255);
    }

    invalidate();
}
}/*endof namespace*/
