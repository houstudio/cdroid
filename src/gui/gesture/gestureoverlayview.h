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
#include <widget/framelayout.h>
#include <gesture/gesturelibraries.h>
namespace cdroid{
class GestureOverlayView:public FrameLayout {
public:
    static constexpr int GESTURE_STROKE_TYPE_SINGLE = 0;
    static constexpr int GESTURE_STROKE_TYPE_MULTIPLE = 1;

    static constexpr int ORIENTATION_HORIZONTAL = 0;
    static constexpr int ORIENTATION_VERTICAL = 1;
public:
    class OnGesturingListener:public EventSet {
    public:
        CallbackBase<void,GestureOverlayView&> onGesturingStarted;
        CallbackBase<void,GestureOverlayView&> onGesturingEnded;
    };

    class OnGestureListener:public EventSet{
    public:
        CallbackBase<void,GestureOverlayView&,MotionEvent&> onGestureStarted;
        CallbackBase<void,GestureOverlayView&,MotionEvent&> onGesture;
        CallbackBase<void,GestureOverlayView&,MotionEvent&> onGestureEnded;
        CallbackBase<void,GestureOverlayView&,MotionEvent&> onGestureCancelled;
    };

    class  OnGesturePerformedListener:public EventSet{
    public:
        CallbackBase<void,GestureOverlayView&, Gesture&> onGesturePerformed;
    };
private:
    static constexpr int FADE_ANIMATION_RATE = 16;
    static constexpr bool GESTURE_RENDERING_ANTIALIAS = true;
    static constexpr bool DITHER_FLAG = true;

    long mFadeDuration = 150;
    long mFadeOffset = 420;
    int64_t mFadingStart;
    bool mFadingHasStarted;
    bool mFadeEnabled = true;
    bool mFireActionPerformed;
    bool mResetMultipleStrokes;

    int mCurrentColor;
    int mPaintAlpha;
    int mCertainGestureColor = 0xFFFFFF00;
    int mUncertainGestureColor = 0x48FFFF00;
    float mGestureStrokeWidth = 12.0f;
    int mInvalidateExtraBorder = 10;

    int mGestureStrokeType = GESTURE_STROKE_TYPE_SINGLE;
    float mGestureStrokeLengthThreshold = 50.0f;
    float mGestureStrokeSquarenessTreshold = 0.275f;
    float mGestureStrokeAngleThreshold = 40.0f;

    int mOrientation = ORIENTATION_VERTICAL;

    Rect mInvalidRect;
    Path mPath;
    bool mGestureVisible = true;

    float mX;
    float mY;

    float mCurveEndX;
    float mCurveEndY;

    float mTotalLength;
    bool mIsGesturing = false;
    bool mPreviousWasGesturing = false;
    bool mInterceptEvents = true;
    bool mIsListeningForGestures;
    bool mResetGesture;

    // current gesture
    Gesture* mCurrentGesture;
    std::vector<GesturePoint> mStrokeBuffer;

    // TODO: Make this a list of WeakReferences
    std::vector<OnGestureListener> mOnGestureListeners;
    // TODO: Make this a list of WeakReferences
    std::vector<OnGesturePerformedListener> mOnGesturePerformedListeners;
    // TODO: Make this a list of WeakReferences
    std::vector<OnGesturingListener> mOnGesturingListeners;

    bool mHandleGestureActions;

    // fading out effect
    bool mIsFadingOut = false;
    float mFadingAlpha = 1.0f;
    const AccelerateDecelerateInterpolator*mInterpolator;// =new AccelerateDecelerateInterpolator();

    Runnable mFadingOut;// = new FadeOutRunnable();
private:
    void init();
    void FadeOutProc();
    void setCurrentColor(int color);
    void setPaintAlpha(int alpha);
    void clear(bool animated, bool fireActionPerformed, bool immediate);
    bool processEvent(MotionEvent& event);
    void touchDown(MotionEvent& event);
    Rect touchMove(MotionEvent& event);
    void touchUp(MotionEvent& event, bool cancel);
    void cancelGesture(MotionEvent& event);
    void fireOnGesturePerformed();
protected:
    void onDetachedFromWindow()override;
public:
    GestureOverlayView(Context* context,const AttributeSet& attrs);
    ~GestureOverlayView()override;
    const std::vector<GesturePoint>& getCurrentStroke() const;

    int getOrientation() const;
    void setOrientation(int orientation);
    void setGestureColor(int color);
    void setUncertainGestureColor(int color);
    int getUncertainGestureColor()const;
    int getGestureColor()const;
    float getGestureStrokeWidth()const;
    void setGestureStrokeWidth(float gestureStrokeWidth);
    int getGestureStrokeType()const;
    void setGestureStrokeType(int gestureStrokeType);
    float getGestureStrokeLengthThreshold()const;
    void setGestureStrokeLengthThreshold(float gestureStrokeLengthThreshold);
    float getGestureStrokeSquarenessTreshold()const;
    void setGestureStrokeSquarenessTreshold(float gestureStrokeSquarenessTreshold);
    float getGestureStrokeAngleThreshold()const;
    void setGestureStrokeAngleThreshold(float gestureStrokeAngleThreshold);
    bool isEventsInterceptionEnabled()const;
    void setEventsInterceptionEnabled(bool enabled);
    bool isFadeEnabled()const;
    void setFadeEnabled(bool fadeEnabled);
    Gesture* getGesture()const;
    void setGesture(Gesture* gesture);
    Path getGesturePath();
    Path getGesturePath(Path& path);
    bool isGestureVisible()const;
    void setGestureVisible(bool visible);
    long getFadeOffset()const;
    void setFadeOffset(long fadeOffset);
    void addOnGestureListener(const OnGestureListener& listener);
    void removeOnGestureListener(const OnGestureListener& listener);
    void removeAllOnGestureListeners();
    void addOnGesturePerformedListener(const OnGesturePerformedListener& listener);
    void removeOnGesturePerformedListener(const OnGesturePerformedListener& listener);
    void removeAllOnGesturePerformedListeners();
    void addOnGesturingListener(const OnGesturingListener& listener);
    void removeOnGesturingListener(const OnGesturingListener& listener);
    void removeAllOnGesturingListeners();
    bool isGesturing()const;
    void draw(Canvas& canvas);
    void clear(bool animated);
    void cancelClearAnimation();
    void cancelGesture();
    bool dispatchTouchEvent(MotionEvent& event)override;
};
}/*endof namespace*/

