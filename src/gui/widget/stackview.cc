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
#include <widget/imageview.h>
#include <widget/stackview.h>
namespace cdroid{

DECLARE_WIDGET(StackView);

StackView::HolographicHelper* StackView::sHolographicHelper;
StackView::StackView(Context* context,const  AttributeSet& attrs)
    :AdapterViewAnimator(context, attrs){

    mResOutColor = attrs.getColor("resOutColor", 0);
    mClickColor = attrs.getColor("clickColor", 0);

    initStackView();
}

void StackView::initStackView() {
    configureViewAnimator(NUM_ACTIVE_VIEWS, 1);
    setStaticTransformationsEnabled(true);
    const ViewConfiguration& configuration = ViewConfiguration::get(getContext());
    mTouchSlop = configuration.getScaledTouchSlop();
    mMaximumVelocity = configuration.getScaledMaximumFlingVelocity();
    mActivePointerId = INVALID_POINTER;

    mVelocityTracker = nullptr;
    mHighlight = new ImageView(1,1);
    mHighlight->setLayoutParams(new LayoutParams(mHighlight));
    addViewInLayout(mHighlight, -1, new LayoutParams(mHighlight));

    mClickFeedback = new ImageView(1,1);
    mClickFeedback->setLayoutParams(new LayoutParams(mClickFeedback));
    addViewInLayout(mClickFeedback, -1, new LayoutParams(mClickFeedback));
    mClickFeedback->setVisibility(INVISIBLE);

    mStackSlider = new StackSlider(this);

    if (sHolographicHelper == nullptr) {
        sHolographicHelper = new HolographicHelper(mContext);
    }
    setClipChildren(false);
    setClipToPadding(false);

    mStackMode = ITEMS_SLIDE_DOWN;

    mWhichChild = -1;

    const float density = mContext->getDisplayMetrics().density;
    mFramePadding = (int) std::ceil(density * FRAME_PADDING);
}

StackView::~StackView(){
    if(mVelocityTracker){
        mVelocityTracker->recycle();
        mVelocityTracker = nullptr;
    }
    delete mStackSlider;
}

void StackView::transformViewForTransition(int fromIndex, int toIndex, View* view, bool animate) {
    if (!animate) {
        ((StackFrame*) view)->cancelSliderAnimator();
        view->setRotationX(0.f);
        LayoutParams* lp = (LayoutParams*) view->getLayoutParams();
        lp->setVerticalOffset(0);
        lp->setHorizontalOffset(0);
    }

    if ((fromIndex == -1) && (toIndex == getNumActiveViews() -1)) {
        transformViewAtIndex(toIndex, view, false);
        view->setVisibility(VISIBLE);
        view->setAlpha(1.0f);
    } else if ((fromIndex == 0) && (toIndex == 1)) {
        // Slide item in
        ((StackFrame*) view)->cancelSliderAnimator();
        view->setVisibility(VISIBLE);

        int duration = std::round(mStackSlider->getDurationForNeutralPosition(mYVelocity));
        StackSlider* animationSlider = new StackSlider(mStackSlider);
        animationSlider->setView(view);

        if (animate) {
            PropertyValuesHolder* slideInY = PropertyValuesHolder::ofFloat("YProgress",{0.0f});
            PropertyValuesHolder* slideInX = PropertyValuesHolder::ofFloat("XProgress",{0.0f});
            ObjectAnimator* slideIn = ObjectAnimator::ofPropertyValuesHolder(animationSlider,{slideInX, slideInY});
            slideIn->setDuration(duration);
            slideIn->setInterpolator(new LinearInterpolator());
            ((StackFrame*) view)->setSliderAnimator(slideIn);
            slideIn->start();
        } else {
            animationSlider->setYProgress(0.f);
            animationSlider->setXProgress(0.f);
        }
    } else if ((fromIndex == 1) && (toIndex == 0)) {
        // Slide item out
        ((StackFrame*) view)->cancelSliderAnimator();
        int duration = std::round(mStackSlider->getDurationForOffscreenPosition(mYVelocity));

        StackSlider* animationSlider = new StackSlider(mStackSlider);
        animationSlider->setView(view);
        if (animate) {
            PropertyValuesHolder* slideOutY = PropertyValuesHolder::ofFloat("YProgress",{1.0f});
            PropertyValuesHolder* slideOutX = PropertyValuesHolder::ofFloat("XProgress",{0.0f});
            ObjectAnimator* slideOut = ObjectAnimator::ofPropertyValuesHolder(animationSlider,{slideOutX, slideOutY});
            slideOut->setDuration(duration);
            slideOut->setInterpolator(new LinearInterpolator());
            ((StackFrame*) view)->setSliderAnimator(slideOut);
            slideOut->start();
        } else {
            animationSlider->setYProgress(1.0f);
            animationSlider->setXProgress(0.f);
        }
    } else if (toIndex == 0) {
        // Make sure this view that is "waiting in the wings" is invisible
        view->setAlpha(0.0f);
        view->setVisibility(INVISIBLE);
    } else if (((fromIndex == 0) || (fromIndex == 1)) && (toIndex > 1)) {
        view->setVisibility(VISIBLE);
        view->setAlpha(1.0f);
        view->setRotationX(0.f);
        LayoutParams* lp = (LayoutParams*) view->getLayoutParams();
        lp->setVerticalOffset(0);
        lp->setHorizontalOffset(0);
    } else if (fromIndex == -1) {
        view->setAlpha(1.0f);
        view->setVisibility(VISIBLE);
    } else if (toIndex == -1) {
        if (animate) {
            postDelayed([view]() {
                view->setAlpha(0);
            }, STACK_RELAYOUT_DURATION);
        } else {
            view->setAlpha(0.f);
        }
    }

    // Implement the faked perspective
    if (toIndex != -1) {
        transformViewAtIndex(toIndex, view, animate);
    }
}

void StackView::transformViewAtIndex(int index, View* view, bool animate) {
    const float maxPerspectiveShiftY = mPerspectiveShiftY;
    const float maxPerspectiveShiftX = mPerspectiveShiftX;

    if (mStackMode == ITEMS_SLIDE_DOWN) {
        index = mMaxNumActiveViews - index - 1;
        if (index == mMaxNumActiveViews - 1) index--;
    } else {
        index--;
        if (index < 0) index++;
    }

    float r = (index * 1.0f) / (mMaxNumActiveViews - 2);

    const float scale = 1.f - PERSPECTIVE_SCALE_FACTOR * (1.f - r);

    float perspectiveTranslationY = r * maxPerspectiveShiftY;
    float scaleShiftCorrectionY = (scale - 1.f) *
            (getMeasuredHeight() * (1.f - PERSPECTIVE_SHIFT_FACTOR_Y) / 2.0f);
    const float transY = perspectiveTranslationY + scaleShiftCorrectionY;

    float perspectiveTranslationX = (1.f - r) * maxPerspectiveShiftX;
    float scaleShiftCorrectionX =  (1.f - scale) *
            (getMeasuredWidth() * (1.f - PERSPECTIVE_SHIFT_FACTOR_X) / 2.0f);
    const float transX = perspectiveTranslationX + scaleShiftCorrectionX;

    // If this view is currently being animated for a certain position, we need to cancel
    // this animation so as not to interfere with the new transformation.
    if (dynamic_cast<StackFrame*>(view)) {
        ((StackFrame*) view)->cancelTransformAnimator();
    }

    if (animate) {
        PropertyValuesHolder* translationX = PropertyValuesHolder::ofFloat("translationX",{transX});
        PropertyValuesHolder* translationY = PropertyValuesHolder::ofFloat("translationY",{transY});
        PropertyValuesHolder* scalePropX = PropertyValuesHolder::ofFloat("scaleX",{scale});
        PropertyValuesHolder* scalePropY = PropertyValuesHolder::ofFloat("scaleY",{scale});

        ObjectAnimator* oa = ObjectAnimator::ofPropertyValuesHolder(view,{scalePropX, scalePropY, translationY, translationX});
        oa->setDuration(STACK_RELAYOUT_DURATION);
        if (dynamic_cast<StackFrame*>(view)) {
            ((StackFrame*) view)->setTransformAnimator(oa);
        }
        oa->start();
    } else {
        view->setTranslationX(transX);
        view->setTranslationY(transY);
        view->setScaleX(scale);
        view->setScaleY(scale);
    }
}

void StackView::setupStackSlider(View* v, int mode) {
    mStackSlider->setMode(mode);
    if (v != nullptr) {
        mHighlight->setImageBitmap(sHolographicHelper->createResOutline(v, mResOutColor));
        mHighlight->setRotation(v->getRotation());
        mHighlight->setTranslationY(v->getTranslationY());
        mHighlight->setTranslationX(v->getTranslationX());
        mHighlight->bringToFront();
        v->bringToFront();
        mStackSlider->setView(v);

        v->setVisibility(VISIBLE);
    }
}

void StackView::showNext() {
    if (mSwipeGestureType != GESTURE_NONE) return;
    if (!mTransitionIsSetup) {
        View* v = getViewAtRelativeIndex(1);
        if (v != nullptr) {
            setupStackSlider(v, StackSlider::NORMAL_MODE);
            mStackSlider->setYProgress(0);
            mStackSlider->setXProgress(0);
        }
    }
    AdapterViewAnimator::showNext();
}

void StackView::showPrevious() {
    if (mSwipeGestureType != GESTURE_NONE) return;
    if (!mTransitionIsSetup) {
        View* v = getViewAtRelativeIndex(0);
        if (v != nullptr) {
            setupStackSlider(v, StackSlider::NORMAL_MODE);
            mStackSlider->setYProgress(1);
            mStackSlider->setXProgress(0);
        }
    }
    AdapterViewAnimator::showPrevious();
}

void StackView::showOnly(int childIndex, bool animate) {
    AdapterViewAnimator::showOnly(childIndex, animate);

    // Here we need to make sure that the z-order of the children is correct
    for (int i = mCurrentWindowEnd; i >= mCurrentWindowStart; i--) {
        int index = modulo(i, getWindowSize());
        auto it = mViewsMap.find(index);
        if (it != mViewsMap.end()) {
            View* v = it->second->view;
            if (v != nullptr) v->bringToFront();
        }
    }
    if (mHighlight != nullptr) {
        mHighlight->bringToFront();
    }
    mTransitionIsSetup = false;
    mClickFeedbackIsValid = false;
}

void StackView::updateClickFeedback() {
    if (!mClickFeedbackIsValid) {
        View* v = getViewAtRelativeIndex(1);
        if (v != nullptr) {
            mClickFeedback->setImageBitmap(
                    sHolographicHelper->createClickOutline(v, mClickColor));
            mClickFeedback->setTranslationX(v->getTranslationX());
            mClickFeedback->setTranslationY(v->getTranslationY());
        }
        mClickFeedbackIsValid = true;
    }
}

void StackView::showTapFeedback(View* v) {
    updateClickFeedback();
    mClickFeedback->setVisibility(VISIBLE);
    mClickFeedback->bringToFront();
    invalidate();
}

void StackView::hideTapFeedback(View* v) {
    mClickFeedback->setVisibility(INVISIBLE);
    invalidate();
}

void StackView::updateChildTransforms() {
    for (int i = 0; i < getNumActiveViews(); i++) {
        View* v = getViewAtRelativeIndex(i);
        if (v != nullptr) {
            transformViewAtIndex(i, v, false);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//private static class StackFrame:public FrameLayout {

StackView::StackFrame::StackFrame(Context* context):FrameLayout(context,AttributeSet(context,"")){
}

void StackView::StackFrame::setTransformAnimator(ObjectAnimator* oa) {
    transformAnimator = oa;
}

void StackView::StackFrame::setSliderAnimator(ObjectAnimator* oa) {
    sliderAnimator = oa;
}

bool StackView::StackFrame::cancelTransformAnimator() {
    if (transformAnimator != nullptr) {
        ObjectAnimator* oa = transformAnimator;
        if (oa != nullptr) {
            oa->cancel();
            return true;
        }
    }
    return false;
}

bool StackView::StackFrame::cancelSliderAnimator() {
    if (sliderAnimator != nullptr) {
        ObjectAnimator* oa = sliderAnimator;
        if (oa != nullptr) {
            oa->cancel();
            return true;
        }
    }
    return false;
}
////////////////////////////////////////////////////////////////////////////////////////

FrameLayout* StackView::getFrameForChild() {
    StackFrame* fl = new StackFrame(mContext);
    fl->setPadding(mFramePadding, mFramePadding, mFramePadding, mFramePadding);
    return fl;
}

void StackView::applyTransformForChildAtIndex(View* child, int relativeIndex) {
}

void StackView::dispatchDraw(Canvas& canvas) {
    bool expandClipRegion = false;

    //canvas.getClipBounds(stackInvalidateRect);
    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child =  getChildAt(i);
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
        if ((lp->horizontalOffset == 0 && lp->verticalOffset == 0) ||
                child->getAlpha() == 0.f || child->getVisibility() != VISIBLE) {
            lp->resetInvalidateRect();
        }
        Rect childInvalidateRect = lp->getInvalidateRect();
        if (!childInvalidateRect.empty()) {
            expandClipRegion = true;
            stackInvalidateRect.Union(childInvalidateRect);
        }
    }

    // We only expand the clip bounds if necessary.
    if (expandClipRegion) {
        canvas.save();
        //canvas.clipRectUnion(stackInvalidateRect);
        AdapterViewAnimator::dispatchDraw(canvas);
        canvas.restore();
    } else {
        AdapterViewAnimator::dispatchDraw(canvas);
    }
}

static int compareFloats(float f1, float f2) {
    if (f1 < f2) {
        return -1;
    } else if (f1 > f2) {
        return 1;
    } else {
        return 0;
    }
}
void StackView::onLayout() {
    if (!mFirstLayoutHappened) {
        mFirstLayoutHappened = true;
        updateChildTransforms();
    }

    const int newSlideAmount = std::round(SLIDE_UP_RATIO * getMeasuredHeight());
    if (mSlideAmount != newSlideAmount) {
        mSlideAmount = newSlideAmount;
        mSwipeThreshold = std::round(SWIPE_THRESHOLD_RATIO * newSlideAmount);
    }

    if (compareFloats(mPerspectiveShiftY, mNewPerspectiveShiftY) != 0 ||
            compareFloats(mPerspectiveShiftX, mNewPerspectiveShiftX) != 0) {

        mPerspectiveShiftY = mNewPerspectiveShiftY;
        mPerspectiveShiftX = mNewPerspectiveShiftX;
        updateChildTransforms();
    }
}

bool StackView::onGenericMotionEvent(MotionEvent& event) {
    if ((event.getSource() & InputDevice::SOURCE_CLASS_POINTER) != 0) {
        switch (event.getAction()) {
            case MotionEvent::ACTION_SCROLL: {
                const float vscroll = event.getAxisValue(MotionEvent::AXIS_VSCROLL);
                if (vscroll < 0) {
                    pacedScroll(false);
                    return true;
                } else if (vscroll > 0) {
                    pacedScroll(true);
                    return true;
                }
            }
        }
    }
    return AdapterViewAnimator::onGenericMotionEvent(event);
}

// This ensures that the frequency of stack flips caused by scrolls is capped
void StackView::pacedScroll(bool up) {
    long timeSinceLastScroll = SystemClock::uptimeMillis() - mLastScrollTime;
    if (timeSinceLastScroll > MIN_TIME_BETWEEN_SCROLLS) {
        if (up) {
            showPrevious();
        } else {
            showNext();
        }
        mLastScrollTime = SystemClock::uptimeMillis();
    }
}

bool StackView::onInterceptTouchEvent(MotionEvent& ev) {
    int action = ev.getAction();
    switch(action & MotionEvent::ACTION_MASK) {
        case MotionEvent::ACTION_DOWN: {
            if (mActivePointerId == INVALID_POINTER) {
                mInitialX = ev.getX();
                mInitialY = ev.getY();
                mActivePointerId = ev.getPointerId(0);
            }
            break;
        }
        case MotionEvent::ACTION_MOVE: {
            int pointerIndex = ev.findPointerIndex(mActivePointerId);
            if (pointerIndex == INVALID_POINTER) {
                // no data for our primary pointer, this shouldn't happen, log it
                LOGD("Error: No data for our primary pointer.");
                return false;
            }
            float newY = ev.getY(pointerIndex);
            float deltaY = newY - mInitialY;

            beginGestureIfNeeded(deltaY);
            break;
        }
        case MotionEvent::ACTION_POINTER_UP: {
            onSecondaryPointerUp(ev);
            break;
        }
        case MotionEvent::ACTION_UP:
        case MotionEvent::ACTION_CANCEL: {
            mActivePointerId = INVALID_POINTER;
            mSwipeGestureType = GESTURE_NONE;
        }
    }

    return mSwipeGestureType != GESTURE_NONE;
}

void StackView::beginGestureIfNeeded(float deltaY) {
    if (int(std::abs(deltaY)) > mTouchSlop && mSwipeGestureType == GESTURE_NONE) {
        const int swipeGestureType = deltaY < 0 ? GESTURE_SLIDE_UP : GESTURE_SLIDE_DOWN;
        cancelLongPress();
        requestDisallowInterceptTouchEvent(true);

        if (mAdapter == nullptr) return;
        const int adapterCount = getCount();

        int activeIndex;
        if (mStackMode == ITEMS_SLIDE_UP) {
            activeIndex = (swipeGestureType == GESTURE_SLIDE_DOWN) ? 0 : 1;
        } else {
            activeIndex = (swipeGestureType == GESTURE_SLIDE_DOWN) ? 1 : 0;
        }

        bool endOfStack = mLoopViews && adapterCount == 1
                && ((mStackMode == ITEMS_SLIDE_UP && swipeGestureType == GESTURE_SLIDE_UP)
                || (mStackMode == ITEMS_SLIDE_DOWN && swipeGestureType == GESTURE_SLIDE_DOWN));
        bool beginningOfStack = mLoopViews && adapterCount == 1
                && ((mStackMode == ITEMS_SLIDE_DOWN && swipeGestureType == GESTURE_SLIDE_UP)
                || (mStackMode == ITEMS_SLIDE_UP && swipeGestureType == GESTURE_SLIDE_DOWN));

        int stackMode;
        if (mLoopViews && !beginningOfStack && !endOfStack) {
            stackMode = StackSlider::NORMAL_MODE;
        } else if (mCurrentWindowStartUnbounded + activeIndex == -1 || beginningOfStack) {
            activeIndex++;
            stackMode = StackSlider::BEGINNING_OF_STACK_MODE;
        } else if (mCurrentWindowStartUnbounded + activeIndex == adapterCount - 1 || endOfStack) {
            stackMode = StackSlider::END_OF_STACK_MODE;
        } else {
            stackMode = StackSlider::NORMAL_MODE;
        }

        mTransitionIsSetup = stackMode == StackSlider::NORMAL_MODE;

        View* v = getViewAtRelativeIndex(activeIndex);
        if (v == nullptr) return;

        setupStackSlider(v, stackMode);

        // We only register this gesture if we've made it this far without a problem
        mSwipeGestureType = swipeGestureType;
        cancelHandleClick();
    }
}

bool StackView::onTouchEvent(MotionEvent& ev) {
    AdapterViewAnimator::onTouchEvent(ev);

    const int action = ev.getAction();
    int pointerIndex = ev.findPointerIndex(mActivePointerId);
    if (pointerIndex == INVALID_POINTER) {
        // no data for our primary pointer, this shouldn't happen, log it
        LOGD("Error: No data for our primary pointer.");
        return false;
    }

    float newY = ev.getY(pointerIndex);
    float newX = ev.getX(pointerIndex);
    float deltaY = newY - mInitialY;
    float deltaX = newX - mInitialX;
    if (mVelocityTracker == nullptr) {
        mVelocityTracker = VelocityTracker::obtain();
    }
    mVelocityTracker->addMovement(ev);

    switch (action & MotionEvent::ACTION_MASK) {
    case MotionEvent::ACTION_MOVE: {
        beginGestureIfNeeded(deltaY);

        float rx = deltaX / (mSlideAmount * 1.0f);
        if (mSwipeGestureType == GESTURE_SLIDE_DOWN) {
            float r = (deltaY - mTouchSlop * 1.0f) / mSlideAmount * 1.0f;
            if (mStackMode == ITEMS_SLIDE_DOWN) r = 1 - r;
            mStackSlider->setYProgress(1.f - r);
            mStackSlider->setXProgress(rx);
            return true;
        } else if (mSwipeGestureType == GESTURE_SLIDE_UP) {
            float r = -(deltaY + mTouchSlop * 1.0f) / mSlideAmount * 1.0f;
            if (mStackMode == ITEMS_SLIDE_DOWN) r = 1.f - r;
            mStackSlider->setYProgress(r);
            mStackSlider->setXProgress(rx);
            return true;
        }
        break;
    }
    case MotionEvent::ACTION_UP: {
        handlePointerUp(ev);
        break;
    }
    case MotionEvent::ACTION_POINTER_UP: {
        onSecondaryPointerUp(ev);
        break;
    }
    case MotionEvent::ACTION_CANCEL: {
        mActivePointerId = INVALID_POINTER;
        mSwipeGestureType = GESTURE_NONE;
        break;
    }
    }
    return true;
}

void StackView::onSecondaryPointerUp(MotionEvent& ev) {
    const int activePointerIndex = ev.getActionIndex();
    const int pointerId = ev.getPointerId(activePointerIndex);
    if (pointerId == mActivePointerId) {

        int activeViewIndex = (mSwipeGestureType == GESTURE_SLIDE_DOWN) ? 0 : 1;

        View* v = getViewAtRelativeIndex(activeViewIndex);
        if (v == nullptr) return;

        // Our primary pointer has gone up -- let's see if we can find
        // another pointer on the view. If so, then we should replace
        // our primary pointer with this new pointer and adjust things
        // so that the view doesn't jump
        for (int index = 0; index < ev.getPointerCount(); index++) {
            if (index != activePointerIndex) {

                float x = ev.getX(index);
                float y = ev.getY(index);

                mTouchRect.set(v->getLeft(), v->getTop(), v->getWidth(), v->getHeight());
                if (mTouchRect.contains(std::round(x), std::round(y))) {
                    float oldX = ev.getX(activePointerIndex);
                    float oldY = ev.getY(activePointerIndex);

                    // adjust our frame of reference to avoid a jump
                    mInitialY += (y - oldY);
                    mInitialX += (x - oldX);

                    mActivePointerId = ev.getPointerId(index);
                    if (mVelocityTracker != nullptr) {
                        mVelocityTracker->clear();
                    }
                    // ok, we're good, we found a new pointer which is touching the active view
                    return;
                }
            }
        }
        // if we made it this far, it means we didn't find a satisfactory new pointer :(,
        // so end the gesture
        handlePointerUp(ev);
    }
}

void StackView::handlePointerUp(MotionEvent& ev) {
    const int pointerIndex = ev.findPointerIndex(mActivePointerId);
    float newY = ev.getY(pointerIndex);
    int deltaY = (int) (newY - mInitialY);
    mLastInteractionTime = SystemClock::uptimeMillis();

    if (mVelocityTracker != nullptr) {
        mVelocityTracker->computeCurrentVelocity(1000, mMaximumVelocity);
        mYVelocity = (int) mVelocityTracker->getYVelocity(mActivePointerId);
    }

    if (mVelocityTracker != nullptr) {
        mVelocityTracker->recycle();
        mVelocityTracker = nullptr;
    }

    if (deltaY > mSwipeThreshold && mSwipeGestureType == GESTURE_SLIDE_DOWN
            && mStackSlider->mMode == StackSlider::NORMAL_MODE) {
        // We reset the gesture variable, because otherwise we will ignore showPrevious() /
        // showNext();
        mSwipeGestureType = GESTURE_NONE;

        // Swipe threshold exceeded, swipe down
        if (mStackMode == ITEMS_SLIDE_UP) {
            showPrevious();
        } else {
            showNext();
        }
        mHighlight->bringToFront();
    } else if (deltaY < -mSwipeThreshold && mSwipeGestureType == GESTURE_SLIDE_UP
            && mStackSlider->mMode == StackSlider::NORMAL_MODE) {
        // We reset the gesture variable, because otherwise we will ignore showPrevious() /
        // showNext();
        mSwipeGestureType = GESTURE_NONE;

        // Swipe threshold exceeded, swipe up
        if (mStackMode == ITEMS_SLIDE_UP) {
            showNext();
        } else {
            showPrevious();
        }

        mHighlight->bringToFront();
    } else if (mSwipeGestureType == GESTURE_SLIDE_UP ) {
        // Didn't swipe up far enough, snap back down
        int duration;
        float finalYProgress = (mStackMode == ITEMS_SLIDE_DOWN) ? 1 : 0;
        if (mStackMode == ITEMS_SLIDE_UP || mStackSlider->mMode != StackSlider::NORMAL_MODE) {
            duration = std::round(mStackSlider->getDurationForNeutralPosition());
        } else {
            duration = std::round(mStackSlider->getDurationForOffscreenPosition());
        }

        StackSlider* animationSlider = new StackSlider(mStackSlider);
        PropertyValuesHolder* snapBackY = PropertyValuesHolder::ofFloat("YProgress",{finalYProgress});
        PropertyValuesHolder* snapBackX = PropertyValuesHolder::ofFloat("XProgress",{0.f});
        ObjectAnimator* pa = ObjectAnimator::ofPropertyValuesHolder(animationSlider,{snapBackX, snapBackY});
        pa->setDuration(duration);
        pa->setInterpolator(new LinearInterpolator());
        pa->start();
    } else if (mSwipeGestureType == GESTURE_SLIDE_DOWN) {
        // Didn't swipe down far enough, snap back up
        float finalYProgress = (mStackMode == ITEMS_SLIDE_DOWN) ? 0 : 1;
        int duration;
        if (mStackMode == ITEMS_SLIDE_DOWN || mStackSlider->mMode != StackSlider::NORMAL_MODE) {
            duration = std::round(mStackSlider->getDurationForNeutralPosition());
        } else {
            duration = std::round(mStackSlider->getDurationForOffscreenPosition());
        }

        StackSlider* animationSlider = new StackSlider(mStackSlider);
        PropertyValuesHolder* snapBackY = PropertyValuesHolder::ofFloat("YProgress",{finalYProgress});
        PropertyValuesHolder* snapBackX = PropertyValuesHolder::ofFloat("XProgress", {0.0f});
        ObjectAnimator* pa = ObjectAnimator::ofPropertyValuesHolder(animationSlider,{snapBackX, snapBackY});
        pa->setDuration(duration);
        pa->start();
    }

    mActivePointerId = INVALID_POINTER;
    mSwipeGestureType = GESTURE_NONE;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//private class StackSlider {

StackView::StackSlider::StackSlider(StackView*stc) {
    mStackView = stc;
}

StackView::StackSlider::StackSlider(const StackSlider* copy) {
    mView = copy->mView;
    mStackView = copy->mStackView;
    mYProgress = copy->mYProgress;
    mXProgress = copy->mXProgress;
    mMode = copy->mMode;
}

float StackView::StackSlider::cubic(float r) const{
    return (float) (std::pow(2 * r - 1, 3) + 1) / 2.0f;
}

float StackView::StackSlider::highlightAlphaInterpolator(float r) const{
    float pivot = 0.4f;
    if (r < pivot) {
        return 0.85f * cubic(r / pivot);
    } else {
        return 0.85f * cubic(1 - (r - pivot) / (1 - pivot));
    }
}

float StackView::StackSlider::viewAlphaInterpolator(float r) const{
    float pivot = 0.3f;
    if (r > pivot) {
        return (r - pivot) / (1 - pivot);
    } else {
        return 0;
    }
}

float StackView::StackSlider::rotationInterpolator(float r) const{
    float pivot = 0.2f;
    if (r < pivot) {
        return 0;
    } else {
        return (r - pivot) / (1 - pivot);
    }
}

void StackView::StackSlider::setView(View* v) {
    mView = v;
}

void StackView::StackSlider::setYProgress(float r) {
    // enforce r between 0 and 1
    r = std::min(1.0f, r);
    r = std::max(0.f, r);

    mYProgress = r;
    if (mView == nullptr) return;

    LayoutParams* viewLp = (LayoutParams*) mView->getLayoutParams();
    LayoutParams* highlightLp = (LayoutParams*) mStackView->mHighlight->getLayoutParams();

    int stackDirection = (mStackView->mStackMode == ITEMS_SLIDE_UP) ? 1 : -1;

    // We need to prevent any clipping issues which may arise by setting a layer type.
    // This doesn't come for free however, so we only want to enable it when required.
    if (compareFloats(0.f, mYProgress) != 0 && compareFloats(1.0f, mYProgress) != 0) {
        if (mView->getLayerType() == LAYER_TYPE_NONE) {
            mView->setLayerType(LAYER_TYPE_HARDWARE);
        }
    } else {
        if (mView->getLayerType() != LAYER_TYPE_NONE) {
            mView->setLayerType(LAYER_TYPE_NONE);
        }
    }
    float alpha = 0.f;
    switch (mMode) {
        case NORMAL_MODE:
            viewLp->setVerticalOffset(std::round(-r * stackDirection * mStackView->mSlideAmount));
            highlightLp->setVerticalOffset(std::round(-r * stackDirection * mStackView->mSlideAmount));
            mStackView->mHighlight->setAlpha(highlightAlphaInterpolator(r));

            alpha = viewAlphaInterpolator(1.f - r);

            // We make sure that views which can't be seen (have 0 alpha) are also invisible
            // so that they don't interfere with click events.
            if (mView->getAlpha() == 0 && alpha != 0 && mView->getVisibility() != VISIBLE) {
                mView->setVisibility(VISIBLE);
            } else if ((alpha == 0) && (mView->getAlpha() != 0) && (mView->getVisibility() == VISIBLE)) {
                mView->setVisibility(INVISIBLE);
            }

            mView->setAlpha(alpha);
            mView->setRotationX(stackDirection * 90.0f * rotationInterpolator(r));
            mStackView->mHighlight->setRotationX(stackDirection * 90.0f * rotationInterpolator(r));
            break;
        case END_OF_STACK_MODE:
            r = r * 0.2f;
            viewLp->setVerticalOffset(std::round(-stackDirection * r * mStackView->mSlideAmount));
            highlightLp->setVerticalOffset(std::round(-stackDirection * r * mStackView->mSlideAmount));
            mStackView->mHighlight->setAlpha(highlightAlphaInterpolator(r));
            break;
        case BEGINNING_OF_STACK_MODE:
            r = (1.f - r) * 0.2f;
            viewLp->setVerticalOffset(std::round(stackDirection * r * mStackView->mSlideAmount));
            highlightLp->setVerticalOffset(std::round(stackDirection * r * mStackView->mSlideAmount));
            mStackView->mHighlight->setAlpha(highlightAlphaInterpolator(r));
            break;
    }
}

void StackView::StackSlider::setXProgress(float r) {
    // enforce r between 0 and 1
    r = std::min(2.0f, r);
    r = std::max(-2.0f, r);

    mXProgress = r;

    if (mView == nullptr) return;
    LayoutParams* viewLp = (LayoutParams*) mView->getLayoutParams();
    LayoutParams* highlightLp = (LayoutParams*) mStackView->mHighlight->getLayoutParams();

    r *= 0.2f;
    viewLp->setHorizontalOffset(std::round(r * mStackView->mSlideAmount));
    highlightLp->setHorizontalOffset(std::round(r * mStackView->mSlideAmount));
}

void StackView::StackSlider::setMode(int mode) {
    mMode = mode;
}

float StackView::StackSlider::getDurationForNeutralPosition() const{
    return getDuration(false, 0);
}

float StackView::StackSlider::getDurationForOffscreenPosition() const{
    return getDuration(true, 0);
}

float StackView::StackSlider::getDurationForNeutralPosition(float velocity) const{
    return getDuration(false, velocity);
}

float StackView::StackSlider::getDurationForOffscreenPosition(float velocity) const{
    return getDuration(true, velocity);
}

float StackView::StackSlider::getDuration(bool invert, float velocity) const{
    if (mView != nullptr) {
        LayoutParams* viewLp = (LayoutParams*) mView->getLayoutParams();

        float d = (float) std::hypot(viewLp->horizontalOffset, viewLp->verticalOffset);
        float maxd = (float) std::hypot(mStackView->mSlideAmount, 0.4f * mStackView->mSlideAmount);
        if (d > maxd) {
            // Because mSlideAmount is updated in onLayout(), it is possible that d > maxd
            // if we get onLayout() right before this method is called.
            d = maxd;
        }

        if (velocity == 0) {
            return (invert ? (1 - d / maxd) : d / maxd) * DEFAULT_ANIMATION_DURATION;
        } else {
            float duration = invert ? d / std::abs(velocity) : (maxd - d) / std::abs(velocity);
            if (duration < MINIMUM_ANIMATION_DURATION || duration > DEFAULT_ANIMATION_DURATION) {
                return getDuration(invert, 0);
            } else {
                return duration;
            }
        }
    }
    return 0;
}

float StackView::StackSlider::getYProgress() const{
    return mYProgress;
}

float StackView::StackSlider::getXProgress() const{
    return mXProgress;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
StackView::LayoutParams* StackView::createOrReuseLayoutParams(View* v) {
    ViewGroup::LayoutParams* currentLp = v->getLayoutParams();
    if (dynamic_cast<LayoutParams*>(currentLp)) {
        LayoutParams* lp = (LayoutParams*) currentLp;
        lp->setHorizontalOffset(0);
        lp->setVerticalOffset(0);
        lp->width = 0;
        lp->width = 0;
        return lp;
    }
    return new LayoutParams(v);
}

void StackView::onLayout(bool changed, int left, int top, int right, int bottom) {
    checkForAndHandleDataChanged();

    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);

        int childRight = mPaddingLeft + child->getMeasuredWidth();
        int childBottom = mPaddingTop + child->getMeasuredHeight();
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();

        child->layout(mPaddingLeft + lp->horizontalOffset, mPaddingTop + lp->verticalOffset,
                childRight + lp->horizontalOffset, childBottom + lp->verticalOffset);

    }
    onLayout();
}

void StackView::advance() {
    long timeSinceLastInteraction = SystemClock::uptimeMillis() - mLastInteractionTime;

    if (mAdapter == nullptr) return;
    const int adapterCount = getCount();
    if (adapterCount == 1 && mLoopViews) return;

    if (mSwipeGestureType == GESTURE_NONE &&
            timeSinceLastInteraction > MIN_TIME_BETWEEN_INTERACTION_AND_AUTOADVANCE) {
        showNext();
    }
}

void StackView::measureChildren() {
    const int count = getChildCount();

    const int measuredWidth = getMeasuredWidth();
    const int measuredHeight = getMeasuredHeight();

    const int childWidth = std::round(measuredWidth*(1.f-PERSPECTIVE_SHIFT_FACTOR_X))
            - mPaddingLeft - mPaddingRight;
    const int childHeight = std::round(measuredHeight*(1.f-PERSPECTIVE_SHIFT_FACTOR_Y))
            - mPaddingTop - mPaddingBottom;

    int maxWidth = 0;
    int maxHeight = 0;

    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        child->measure(MeasureSpec::makeMeasureSpec(childWidth, MeasureSpec::AT_MOST),
                MeasureSpec::makeMeasureSpec(childHeight, MeasureSpec::AT_MOST));

        if (child != mHighlight && child != mClickFeedback) {
            const int childMeasuredWidth = child->getMeasuredWidth();
            const int childMeasuredHeight = child->getMeasuredHeight();
            if (childMeasuredWidth > maxWidth) {
                maxWidth = childMeasuredWidth;
            }
            if (childMeasuredHeight > maxHeight) {
                maxHeight = childMeasuredHeight;
            }
        }
    }

    mNewPerspectiveShiftX = PERSPECTIVE_SHIFT_FACTOR_X * measuredWidth;
    mNewPerspectiveShiftY = PERSPECTIVE_SHIFT_FACTOR_Y * measuredHeight;

    // If we have extra space, we try and spread the items out
    if (maxWidth > 0 && count > 0 && maxWidth < childWidth) {
        mNewPerspectiveShiftX = measuredWidth - maxWidth;
    }

    if (maxHeight > 0 && count > 0 && maxHeight < childHeight) {
        mNewPerspectiveShiftY = measuredHeight - maxHeight;
    }
}

void StackView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    int widthSpecSize = MeasureSpec::getSize(widthMeasureSpec);
    int heightSpecSize = MeasureSpec::getSize(heightMeasureSpec);
    const int widthSpecMode = MeasureSpec::getMode(widthMeasureSpec);
    const int heightSpecMode = MeasureSpec::getMode(heightMeasureSpec);

    bool haveChildRefSize = (mReferenceChildWidth != -1 && mReferenceChildHeight != -1);

    // We need to deal with the case where our parent hasn't told us how
    // big we should be. In this case we should
    float factorY = 1/(1 - PERSPECTIVE_SHIFT_FACTOR_Y);
    if (heightSpecMode == MeasureSpec::UNSPECIFIED) {
        heightSpecSize = haveChildRefSize ?
                std::round(mReferenceChildHeight * (1.f + factorY)) +
                mPaddingTop + mPaddingBottom : 0;
    } else if (heightSpecMode == MeasureSpec::AT_MOST) {
        if (haveChildRefSize) {
            int height = std::round(mReferenceChildHeight * (1.f + factorY))
                    + mPaddingTop + mPaddingBottom;
            if (height <= heightSpecSize) {
                heightSpecSize = height;
            } else {
                heightSpecSize |= MEASURED_STATE_TOO_SMALL;

            }
        } else {
            heightSpecSize = 0;
        }
    }

    float factorX = 1/(1 - PERSPECTIVE_SHIFT_FACTOR_X);
    if (widthSpecMode == MeasureSpec::UNSPECIFIED) {
        widthSpecSize = haveChildRefSize ?
                std::round(mReferenceChildWidth * (1 + factorX)) +
                mPaddingLeft + mPaddingRight : 0;
    } else if (heightSpecMode == MeasureSpec::AT_MOST) {
        if (haveChildRefSize) {
            int width = mReferenceChildWidth + mPaddingLeft + mPaddingRight;
            if (width <= widthSpecSize) {
                widthSpecSize = width;
            } else {
                widthSpecSize |= MEASURED_STATE_TOO_SMALL;
            }
        } else {
            widthSpecSize = 0;
        }
    }
    setMeasuredDimension(widthSpecSize, heightSpecSize);
    measureChildren();
}

std::string StackView::getAccessibilityClassName() const{
    return "StackView";
}

void StackView::onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info) {
    AdapterViewAnimator::onInitializeAccessibilityNodeInfoInternal(info);
    info.setScrollable(getChildCount() > 1);
    /*if (isEnabled()) {
        if (getDisplayedChild() < getChildCount() - 1) {
            info.addAction(AccessibilityNodeInfo.AccessibilityAction::ACTION_SCROLL_FORWARD);
            if (mStackMode == ITEMS_SLIDE_UP) {
                info.addAction(AccessibilityNodeInfo.AccessibilityAction::ACTION_PAGE_DOWN);
            } else {
                info.addAction(AccessibilityNodeInfo.AccessibilityAction::ACTION_PAGE_UP);
            }
        }
        if (getDisplayedChild() > 0) {
            info.addAction(AccessibilityNodeInfo.AccessibilityAction::ACTION_SCROLL_BACKWARD);
            if (mStackMode == ITEMS_SLIDE_UP) {
                info.addAction(AccessibilityNodeInfo.AccessibilityAction::ACTION_PAGE_UP);
            } else {
                info.addAction(AccessibilityNodeInfo.AccessibilityAction::ACTION_PAGE_DOWN);
            }
        }
    }*/
}

bool StackView::goForward() {
    if (getDisplayedChild() < getChildCount() - 1) {
        showNext();
        return true;
    }
    return false;
}

bool StackView::goBackward() {
    if (getDisplayedChild() > 0) {
        showPrevious();
        return true;
    }
    return false;
}

bool StackView::performAccessibilityActionInternal(int action, Bundle& arguments) {
    if (AdapterViewAnimator::performAccessibilityActionInternal(action, &arguments)) {
        return true;
    }
    if (!isEnabled()) {
        return false;
    }
    switch (action) {
    case AccessibilityNodeInfo::ACTION_SCROLL_FORWARD: {
        return goForward();
    }
    case AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD: {
        return goBackward();
    }
    /*case R.id.accessibilityActionPageUp: {
        if (mStackMode == ITEMS_SLIDE_UP) {
            return goBackward();
        } else {
            return goForward();
        }
    }
    case R.id.accessibilityActionPageDown: {
        if (mStackMode == ITEMS_SLIDE_UP) {
            return goForward();
        } else {
            return goBackward();
        }
    }*/
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////

StackView::LayoutParams::LayoutParams(View* view)
  :ViewGroup::LayoutParams(0, 0){
    width = 0;
    height = 0;
    horizontalOffset = 0;
    verticalOffset = 0;
    mView = view;
    mStackView = nullptr;
}

StackView::LayoutParams::LayoutParams(Context* c,const AttributeSet& attrs)
    :ViewGroup::LayoutParams(c, attrs){
    horizontalOffset = 0;
    verticalOffset = 0;
    width = 0;
    height = 0;
    mStackView = nullptr;
}

void StackView::LayoutParams::invalidateGlobalRegion(View* v, Rect r) {
    // We need to make a new rect here, so as not to modify the one passed
    globalInvalidateRect= r;
    globalInvalidateRect.Union(0, 0, mStackView->getWidth(), mStackView->getHeight());
    View* p = v;
    if (!(v->getParent() != nullptr/*&& v->getParent() instanceof View)*/)) return;

    bool firstPass = true;
    parentRect.set(0, 0, 0, 0);
    while ((p->getParent() != nullptr) /*&& p.getParent() instanceof View*/
            && !parentRect.contains(globalInvalidateRect)) {
        if (!firstPass) {
            globalInvalidateRect.offset(p->getLeft() - p->getScrollX(), p->getTop()
                    - p->getScrollY());
        }
        firstPass = false;
        p = (View*) p->getParent();
        parentRect.set(p->getScrollX(), p->getScrollY(),
                p->getWidth() + p->getScrollX(), p->getHeight() + p->getScrollY());
        p->invalidate(globalInvalidateRect.left, globalInvalidateRect.top,
                globalInvalidateRect.width, globalInvalidateRect.width);
    }

    p->invalidate(globalInvalidateRect.left, globalInvalidateRect.top,
            globalInvalidateRect.width, globalInvalidateRect.height);
}

Rect StackView::LayoutParams::getInvalidateRect() const{
    return invalidateRect;
}

void StackView::LayoutParams::resetInvalidateRect() {
    invalidateRect.set(0, 0, 0, 0);
}

void StackView::LayoutParams::setVerticalOffset(int newVerticalOffset) {
    setOffsets(horizontalOffset, newVerticalOffset);
}

void StackView::LayoutParams::setHorizontalOffset(int newHorizontalOffset) {
    setOffsets(newHorizontalOffset, verticalOffset);
}

void StackView::LayoutParams::setOffsets(int newHorizontalOffset, int newVerticalOffset) {
    int horizontalOffsetDelta = newHorizontalOffset - horizontalOffset;
    horizontalOffset = newHorizontalOffset;
    int verticalOffsetDelta = newVerticalOffset - verticalOffset;
    verticalOffset = newVerticalOffset;

    if (mView != nullptr) {
        mView->requestLayout();
        int left = std::min(mView->getLeft() + horizontalOffsetDelta, mView->getLeft());
        int right = std::max(mView->getRight() + horizontalOffsetDelta, mView->getRight());
        int top = std::min(mView->getTop() + verticalOffsetDelta, mView->getTop());
        int bottom = std::max(mView->getBottom() + verticalOffsetDelta, mView->getBottom());
#if 0
        invalidateRectf.set(left, top, right-left, bottom-top);

        float xoffset = -invalidateRectf.left;
        float yoffset = -invalidateRectf.top;
        invalidateRectf.offset(xoffset, yoffset);
        mView->getMatrix().transform_rectangle(invalidateRectf);
        invalidateRectf.offset(-xoffset, -yoffset);

        invalidateRect.set((int) std::floor(invalidateRectf.left),
                (int) std::floor(invalidateRectf.top),
                (int) std::ceil(invalidateRectf.width),
                (int) std::ceil(invalidateRectf.height));
#else
        Rect rctmp={left, top, right-left, bottom-top};
        int xoffset=-rctmp.left;
        int yoffset=-rctmp.top;
        rctmp.offset(xoffset,yoffset);
        mView->getMatrix().transform_rectangle((Cairo::RectangleInt&)rctmp);
        rctmp.offset(-xoffset,-xoffset);
        invalidateRect = rctmp;
#endif
        invalidateGlobalRegion(mView, invalidateRect);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
/*static class HolographicHelper*/
StackView::HolographicHelper::HolographicHelper(Context* context) {
    mDensity = context->getDisplayMetrics().density;

    /*mHolographicPaint.setFilterBitmap(true);
    mHolographicPaint.setMaskFilter(TableMaskFilter.CreateClipTable(0, 30));
    mErasePaint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.DST_OUT));
    mErasePaint.setFilterBitmap(true);

    mSmallBlurMaskFilter = new BlurMaskFilter(2 * mDensity, BlurMaskFilter.Blur.NORMAL);
    mLargeBlurMaskFilter = new BlurMaskFilter(4 * mDensity, BlurMaskFilter.Blur.NORMAL);*/
}

Bitmap StackView::HolographicHelper::createClickOutline(View* v, int color) {
    return createOutline(v, CLICK_FEEDBACK, color);
}

Bitmap StackView::HolographicHelper::createResOutline(View* v, int color) {
    return createOutline(v, RES_OUT, color);
}

Bitmap StackView::HolographicHelper::createOutline(View* v, int type, int color) {
    /*mHolographicPaint.setColor(color);
    if (type == RES_OUT) {
        mBlurPaint.setMaskFilter(mSmallBlurMaskFilter);
    } else if (type == CLICK_FEEDBACK) {
        mBlurPaint.setMaskFilter(mLargeBlurMaskFilter);
    }*/

    if (v->getMeasuredWidth() == 0 || v->getMeasuredHeight() == 0) {
        return nullptr;
    }

    /*Bitmap bitmap = Bitmap.createBitmap(v.getResources().getDisplayMetrics(),
            v.getMeasuredWidth(), v.getMeasuredHeight(), Bitmap.Config.ARGB_8888);
    mCanvas->setBitmap(bitmap);*/

    float rotationX = v->getRotationX();
    float rotation = v->getRotation();
    float translationY = v->getTranslationY();
    float translationX = v->getTranslationX();
    v->setRotationX(0);
    v->setRotation(0);
    v->setTranslationY(0);
    v->setTranslationX(0);
    v->draw(*mCanvas);
    v->setRotationX(rotationX);
    v->setRotation(rotation);
    v->setTranslationY(translationY);
    v->setTranslationX(translationX);

    //drawOutline(*mCanvas, bitmap);
    //mCanvas->setBitmap(nullptr);
    return nullptr;//bitmap;
}

void StackView::HolographicHelper::drawOutline(Canvas& dest, Bitmap src) {
    int xy[2];
    /*Bitmap mask = src.extractAlpha(mBlurPaint, xy);
    mMaskCanvas->setBitmap(mask);
    mMaskCanvas->drawBitmap(src, -xy[0], -xy[1], mErasePaint);
    dest.drawColor(0, PorterDuff.Mode.CLEAR);
    dest.setMatrix(mIdentityMatrix);
    dest.drawBitmap(mask, xy[0], xy[1], mHolographicPaint);
    mMaskCanvas->setBitmap(null);
    mask.recycle();*/
}
}/*endof namespace*/
