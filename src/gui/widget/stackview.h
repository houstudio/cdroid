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
#ifndef __STACK_VIEW_H__
#define __STACK_VIEW_H__
#include <widget/framelayout.h>
#include <widget/adapterviewanimator.h>
namespace cdroid{
class ImageView;
class StackView:public AdapterViewAnimator {
private:
    class StackFrame;
    class StackSlider;
    class HolographicHelper;
    static constexpr int DEFAULT_ANIMATION_DURATION = 400;
    static constexpr int MINIMUM_ANIMATION_DURATION = 50;
    static constexpr int STACK_RELAYOUT_DURATION = 100;

    static constexpr float PERSPECTIVE_SHIFT_FACTOR_Y = 0.1f;
    static constexpr float PERSPECTIVE_SHIFT_FACTOR_X = 0.1f;

    float mPerspectiveShiftX;
    float mPerspectiveShiftY;
    float mNewPerspectiveShiftX;
    float mNewPerspectiveShiftY;

    static constexpr float PERSPECTIVE_SCALE_FACTOR = 0.f;
    static constexpr int ITEMS_SLIDE_UP = 0;
    static constexpr int ITEMS_SLIDE_DOWN = 1;
    static constexpr int GESTURE_NONE = 0;
    static constexpr int GESTURE_SLIDE_UP = 1;
    static constexpr int GESTURE_SLIDE_DOWN = 2;
    static constexpr float SWIPE_THRESHOLD_RATIO = 0.2f;
    static constexpr float SLIDE_UP_RATIO = 0.7f;
    static constexpr int INVALID_POINTER = -1;
    static constexpr int NUM_ACTIVE_VIEWS = 5;
    static constexpr int FRAME_PADDING = 4;
    static constexpr int MIN_TIME_BETWEEN_INTERACTION_AND_AUTOADVANCE = 5000;
    static constexpr long MIN_TIME_BETWEEN_SCROLLS = 100;

    Rect mTouchRect;
    float mInitialY;
    float mInitialX;
    int mActivePointerId;
    int mYVelocity = 0;
    int mSwipeGestureType = GESTURE_NONE;
    int mSlideAmount;
    int mSwipeThreshold;
    int mTouchSlop;
    int mMaximumVelocity;
    VelocityTracker* mVelocityTracker;
    bool mTransitionIsSetup = false;
    int mResOutColor;
    int mClickColor;

    static std::shared_ptr<HolographicHelper> sHolographicHelper;
    ImageView* mHighlight;
    ImageView* mClickFeedback;
    bool mClickFeedbackIsValid = false;
    StackSlider* mStackSlider;
    bool mFirstLayoutHappened = false;
    long mLastInteractionTime = 0;
    long mLastScrollTime;
    int mStackMode;
    int mFramePadding;
    Rect stackInvalidateRect;
private:
    void initStackView();
    void transformViewAtIndex(int index, View* view, bool animate);
    void setupStackSlider(View* v, int mode);
    void updateChildTransforms();
    void onLayout();
    void pacedScroll(bool up);
    void beginGestureIfNeeded(float deltaY);
    void onSecondaryPointerUp(MotionEvent& ev);
    void handlePointerUp(MotionEvent& ev);
    void measureChildren(); 
    bool goForward();
    bool goBackward();
protected:
    void dispatchDraw(Canvas& canvas)override;
    void onLayout(bool changed, int left, int top, int right, int bottom) override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
public:
    class LayoutParams;
    StackView(Context* context,const  AttributeSet& attrs);
    ~StackView()override;
    void transformViewForTransition(int fromIndex, int toIndex, View* view, bool animate);

    void showNext() override;
    void showPrevious() override;

    void showOnly(int childIndex, bool animate) override;

    void updateClickFeedback();

    void showTapFeedback(View* v)override;
    void hideTapFeedback(View* v)override;

    FrameLayout* getFrameForChild() override;

    void applyTransformForChildAtIndex(View* child, int relativeIndex);

    bool onGenericMotionEvent(MotionEvent& event)override;
    bool onInterceptTouchEvent(MotionEvent& ev) override;
    bool onTouchEvent(MotionEvent& ev) override;

    LayoutParams* createOrReuseLayoutParams(View* v);

    void advance() override;

    std::string getAccessibilityClassName()const override;

    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info)override;

    bool performAccessibilityActionInternal(int action, Bundle& arguments);
};

class StackView::LayoutParams:public ViewGroup::LayoutParams {
    int horizontalOffset;
    int verticalOffset;
    View* mView;
    StackView*mStackView;
private:
    friend StackView;
    Rect parentRect;
    Rect invalidateRect;
    RectF invalidateRectf;
    Rect globalInvalidateRect;
public:
    LayoutParams(View* view);
    LayoutParams(Context* c,const AttributeSet& attrs);

    void invalidateGlobalRegion(View* v, Rect r);

    Rect getInvalidateRect() const;

    void resetInvalidateRect();
    void setVerticalOffset(int newVerticalOffset);
    void setHorizontalOffset(int newHorizontalOffset);
    void setOffsets(int newHorizontalOffset, int newVerticalOffset);
};

class StackView::StackFrame:public FrameLayout {
    ObjectAnimator* transformAnimator;
    ObjectAnimator* sliderAnimator;
public:
    StackFrame(Context* context);
    void setTransformAnimator(ObjectAnimator* oa);
    void setSliderAnimator(ObjectAnimator* oa);
    bool cancelTransformAnimator();
    bool cancelSliderAnimator();
};

class StackView::StackSlider {
    StackView*mStackView;
    View* mView;
    float mYProgress;
    float mXProgress;

    static constexpr int NORMAL_MODE = 0;
    static constexpr int BEGINNING_OF_STACK_MODE = 1;
    static constexpr int END_OF_STACK_MODE = 2;
    int mMode = NORMAL_MODE;
private:
    friend StackView;
    float cubic(float r)const;
    float highlightAlphaInterpolator(float r)const;
    float viewAlphaInterpolator(float r)const;
    float rotationInterpolator(float r)const;
    float getDuration(bool invert, float velocity)const;
public:
    StackSlider(StackView*);
    StackSlider(const StackSlider* copy);

    void setView(View* v);
    void setMode(int mode);

    void setYProgress(float r);
    void setXProgress(float r);
    float getYProgress()const;
    float getXProgress()const;

    float getDurationForNeutralPosition()const;
    float getDurationForOffscreenPosition()const;
    float getDurationForNeutralPosition(float velocity)const;
    float getDurationForOffscreenPosition(float velocity)const;
};

class StackView::HolographicHelper {
private:
    static constexpr int RES_OUT = 0;
    static constexpr int CLICK_FEEDBACK = 1;
    float mDensity;
    //BlurMaskFilter mSmallBlurMaskFilter;
    //BlurMaskFilter mLargeBlurMaskFilter;
    Canvas* mCanvas;
    Canvas* mMaskCanvas;
    Cairo::Matrix mIdentityMatrix;
public:
    HolographicHelper(Context* context);
    Bitmap createClickOutline(View* v, int color);
    Bitmap createResOutline(View* v, int color);
    Bitmap createOutline(View* v, int type, int color);
    void drawOutline(Canvas& dest, Bitmap src);
};

}/*endof namespace*/
#endif/*__STACK_VIEW_H__*/
