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
#include <widget/slidingpanelayout.h>
#include <core/build.h>
namespace cdroid{

DECLARE_WIDGET(SlidingPaneLayout)

SlidingPaneLayout::SlidingPaneLayout(int w,int h):ViewGroup(w,h){
    initView();
}

SlidingPaneLayout::SlidingPaneLayout(Context* context, const AttributeSet& attrs)
    :ViewGroup(context, attrs){
    initView();
}

void SlidingPaneLayout::initView(){
    const float density = mContext->getDisplayMetrics().density;
    mOverhangSize = (int) (DEFAULT_OVERHANG_SIZE * density + 0.5f);
    mShadowDrawableLeft = nullptr;
    mShadowDrawableRight= nullptr;
    mLockMode = LOCK_MODE_UNLOCKED;
    mParallaxOffset = 0;
    mSlideRange = 0;
    mSlideOffset= 0;
    mParallaxBy = 0;
    mInitialMotionX = 0;
    mInitialMotionY = 0;
    mSlideableView = nullptr;
    mCanSlide = true;
    mFirstLayout = true;
    mIsUnableToDrag = false;
    mPreservedOpenState = false;
    //setWillNotDraw(false);
    //setAccessibilityDelegate(new AccessibilityDelegate());
    setImportantForAccessibility(IMPORTANT_FOR_ACCESSIBILITY_YES);
    mDragHelper = ViewDragHelper::create(this, 0.5f, new DragHelperCallback(this));
    mDragHelper->setMinVelocity(MIN_FLING_VELOCITY * density);
}

SlidingPaneLayout::~SlidingPaneLayout(){
    delete mDragHelper;
    delete mShadowDrawableLeft;
    delete mShadowDrawableRight;
}

void SlidingPaneLayout::setLockMode(int mode){
    mLockMode = mode;
}

int SlidingPaneLayout::getLockMode()const{
    return mLockMode;
}

void SlidingPaneLayout::setParallaxDistance(int parallaxBy) {
    mParallaxBy = parallaxBy;
    requestLayout();
}

int SlidingPaneLayout::getParallaxDistance() const{
    return mParallaxBy;
}

void SlidingPaneLayout::setSliderFadeColor(int color) {
    mSliderFadeColor = color;
}

int SlidingPaneLayout::getSliderFadeColor() const{
    return mSliderFadeColor;
}

void SlidingPaneLayout::setCoveredFadeColor(int color) {
    mCoveredFadeColor = color;
}

int SlidingPaneLayout::getCoveredFadeColor() const{
    return mCoveredFadeColor;
}

void SlidingPaneLayout::setPanelSlideListener(const PanelSlideListener& listener) {
    addPanelSlideListener(listener);
}

void SlidingPaneLayout::addPanelSlideListener(const PanelSlideListener& listener) {
    auto it = std::find(mPanelSlideListeners.begin(),mPanelSlideListeners.end(),listener);
    if(it==mPanelSlideListeners.end()){
        mPanelSlideListeners.push_back(listener);
    }
}

void SlidingPaneLayout::removePanelSlideListener(const PanelSlideListener& listener) {
    auto it = std::find(mPanelSlideListeners.begin(),mPanelSlideListeners.end(),listener);
    if(it!=mPanelSlideListeners.end()){
        mPanelSlideListeners.erase(it);
    }
}

void SlidingPaneLayout::dispatchOnPanelSlide(View* panel) {
    for(PanelSlideListener listener : mPanelSlideListeners){
        if (listener.onPanelSlide != nullptr) {
            listener.onPanelSlide(*panel, mSlideOffset);
        }
    }
}

void SlidingPaneLayout::dispatchOnPanelOpened(View* panel) {
    for(PanelSlideListener listener : mPanelSlideListeners){
        if (listener.onPanelOpened != nullptr) {
            listener.onPanelOpened(*panel);
        }
    }
    sendAccessibilityEvent(AccessibilityEvent::TYPE_WINDOW_STATE_CHANGED);
}

void SlidingPaneLayout::dispatchOnPanelClosed(View* panel) {
    for(PanelSlideListener listener : mPanelSlideListeners){
        if (listener.onPanelClosed != nullptr) {
            listener.onPanelClosed(*panel);
        }
    }
    sendAccessibilityEvent(AccessibilityEvent::TYPE_WINDOW_STATE_CHANGED);
}

void SlidingPaneLayout::updateObscuredViewsVisibility(View* panel) {
    const bool isLayoutRtl = isLayoutRtlSupport();
    const int startBound = isLayoutRtl ? (getWidth() - getPaddingRight()) : getPaddingLeft();
    const int endBound = isLayoutRtl ? getPaddingLeft() : (getWidth() - getPaddingRight());
    const int topBound = getPaddingTop();
    const int bottomBound = getHeight() - getPaddingBottom();
    int left;
    int right;
    int top;
    int bottom;
    if (panel != nullptr && viewIsOpaque(panel)) {
        left = panel->getLeft();
        right = panel->getRight();
        top = panel->getTop();
        bottom = panel->getBottom();
    } else {
        left = right = top = bottom = 0;
    }

    for (int i = 0, childCount = getChildCount(); i < childCount; i++) {
        View* child = getChildAt(i);

        if (child == panel) {
            // There are still more children above the panel but they won't be affected.
            break;
        } else if (child->getVisibility() == GONE) {
            continue;
        }

        int clampedChildLeft = std::max((isLayoutRtl ? endBound : startBound), child->getLeft());
        int clampedChildTop = std::max(topBound, child->getTop());
        int clampedChildRight = std::min((isLayoutRtl ? startBound : endBound), child->getRight());
        int clampedChildBottom = std::min(bottomBound, child->getBottom());
        int vis;
        if (clampedChildLeft >= left && clampedChildTop >= top
                && clampedChildRight <= right && clampedChildBottom <= bottom) {
            vis = INVISIBLE;
        } else {
            vis = VISIBLE;
        }
        child->setVisibility(vis);
    }
}

void SlidingPaneLayout::setAllChildrenVisible() {
    for (int i = 0, childCount = getChildCount(); i < childCount; i++) {
        View* child = getChildAt(i);
        if (child->getVisibility() == INVISIBLE) {
            child->setVisibility(VISIBLE);
        }
    }
}

bool SlidingPaneLayout::viewIsOpaque(View* v) {
    if (v->isOpaque()) {
        return true;
    }

    // View#isOpaque didn't take all valid opaque scrollbar modes into account
    // before API 18 (JB-MR2). On newer devices rely solely on isOpaque above and return false
    // here. On older devices, check the view's background drawable directly as a fallback.
    if (Build::VERSION::SDK_INT >= 18) {
        return false;
    }

    Drawable* bg = v->getBackground();
    if (bg != nullptr) {
        return bg->getOpacity() == PixelFormat::OPAQUE;
    }
    return false;
}

void SlidingPaneLayout::addView(View* child, int index, ViewGroup::LayoutParams* params) {
    if (getChildCount() == 1) {
        // Wrap detail view inside a touch blocker container
        View* detailView = new TouchBlocker(child);
        ViewGroup::addView(detailView, index, params);
        return;
    }
    ViewGroup::addView(child, index, params);
}

void SlidingPaneLayout::removeView(View* view) {
    if (dynamic_cast<TouchBlocker*>(view->getParent())) {
        ViewGroup::removeView(view->getParent());
        return;
    }
    ViewGroup::removeView(view);
}

void SlidingPaneLayout::onAttachedToWindow() {
    ViewGroup::onAttachedToWindow();
    mFirstLayout = true;
    /*if (mFoldingFeatureObserver != null) {
         Activity activity = getActivityOrNull(getContext());
         if (activity != null) {
              mFoldingFeatureObserver.registerLayoutStateChangeCallback(activity);
         }
    }*/
}

void SlidingPaneLayout::onDetachedFromWindow() {
    ViewGroup::onDetachedFromWindow();
    mFirstLayout = true;
    /*if (mFoldingFeatureObserver != null) {
            mFoldingFeatureObserver.unregisterLayoutStateChangeCallback();
    }*/
    for (int i = 0, count = mPostedRunnables.size(); i < count; i++) {
        DisableLayerRunnable& dlr = mPostedRunnables.at(i);
        dlr.run();
    }
    mPostedRunnables.clear();
}

void SlidingPaneLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    int widthMode = MeasureSpec::getMode(widthMeasureSpec);
    int widthSize = MeasureSpec::getSize(widthMeasureSpec);
    int heightMode = MeasureSpec::getMode(heightMeasureSpec);
    int heightSize = MeasureSpec::getSize(heightMeasureSpec);

    int layoutHeight = 0;
    int maxLayoutHeight = 0;
    switch (heightMode) {
    case MeasureSpec::EXACTLY:
        layoutHeight = maxLayoutHeight = heightSize - getPaddingTop() - getPaddingBottom();
        break;
    case MeasureSpec::AT_MOST:
        maxLayoutHeight = heightSize - getPaddingTop() - getPaddingBottom();
        break;
    }

    float weightSum = 0;
    bool canSlide = false;
    const int widthAvailable = std::max(widthSize - getPaddingLeft() - getPaddingRight(), 0);
    int widthRemaining = widthAvailable;
    const int childCount = getChildCount();

    if (childCount > 2) {
        LOGE("onMeasure: More than two child views are not supported.");
    }

    // We'll find the current one below.
    mSlideableView = nullptr;

    // First pass. Measure based on child LayoutParams width/height.
    // Weight will incur a second pass.
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();

        if (child->getVisibility() == GONE) {
            lp->dimWhenOffset = false;
            continue;
        }

        if (lp->weight > 0) {
            weightSum += lp->weight;

            // If we have no width, weight is the only contributor to the final size.
            // Measure this view on the weight pass only.
            if (lp->width == 0) continue;
        }

        int childWidthSpec;
        const int horizontalMargin = lp->leftMargin + lp->rightMargin;

        int childWidthSize = std::max(widthAvailable - horizontalMargin, 0);
        // When the parent width spec is UNSPECIFIED, measure each of child to get its
        // desired width.
        if (lp->width == LayoutParams::WRAP_CONTENT) {
            childWidthSpec = MeasureSpec::makeMeasureSpec(childWidthSize,
                    widthMode == MeasureSpec::UNSPECIFIED ? widthMode : MeasureSpec::AT_MOST);
        } else if (lp->width == LayoutParams::MATCH_PARENT) {
            childWidthSpec = MeasureSpec::makeMeasureSpec(childWidthSize, widthMode);
        } else {
            childWidthSpec = MeasureSpec::makeMeasureSpec(lp->width, MeasureSpec::EXACTLY);
        }

        int childHeightSpec = getChildMeasureSpec(heightMeasureSpec,
                getPaddingTop() + getPaddingBottom(), lp->height);
        child->measure(childWidthSpec, childHeightSpec);
        const int childWidth = child->getMeasuredWidth();
        const int childHeight = child->getMeasuredHeight();

        if (childHeight > layoutHeight) {
            if (heightMode == MeasureSpec::AT_MOST) {
                layoutHeight = std::min(childHeight, maxLayoutHeight);
            } else if (heightMode == MeasureSpec::UNSPECIFIED) {
                layoutHeight = childHeight;
            }
        }

        widthRemaining -= childWidth;
        // Skip first child (list pane), the list pane is always a non-sliding pane.
        if (i == 0) {
            continue;
        }
        canSlide |= lp->slideable = widthRemaining < 0;
        if (lp->slideable) {
            mSlideableView = child;
        }
    }
    // Second pass. Resolve weight.
    // Child views overlap when the combined width of child views cannot fit into the
    // available width. Each of child views is sized to fill all available space. If there is
    // no overlap, distribute the extra width proportionally to weight.
    if (canSlide || weightSum > 0) {
        for (int i = 0; i < childCount; i++) {
            View* child = getChildAt(i);
            if (child->getVisibility() == GONE) {
                continue;
            }

            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
            const bool skippedFirstPass = lp->width == 0 && lp->weight > 0;
            const int measuredWidth = skippedFirstPass ? 0 : child->getMeasuredWidth();
            int newWidth = measuredWidth;
            int childWidthSpec = 0;
            if (canSlide) {
                // Child view consumes available space if the combined width cannot fit into
                // the layout available width.
                const int horizontalMargin = lp->leftMargin + lp->rightMargin;
                newWidth = widthAvailable - horizontalMargin;
                childWidthSpec = MeasureSpec::makeMeasureSpec(
                        newWidth, MeasureSpec::EXACTLY);

            } else if (lp->weight > 0) {
                // Distribute the extra width proportionally similar to LinearLayout
                const int widthToDistribute = std::max(0, widthRemaining);
                const int addedWidth = (int) (lp->weight * widthToDistribute / weightSum);
                newWidth = measuredWidth + addedWidth;
                childWidthSpec = MeasureSpec::makeMeasureSpec(newWidth, MeasureSpec::EXACTLY);
            }
            const int childHeightSpec = measureChildHeight(child, heightMeasureSpec,
                    getPaddingTop() + getPaddingBottom());
            if (measuredWidth != newWidth) {
                child->measure(childWidthSpec, childHeightSpec);
                const int childHeight = child->getMeasuredHeight();
                if (childHeight > layoutHeight) {
                    if (heightMode == MeasureSpec::AT_MOST) {
                        layoutHeight = std::min(childHeight, maxLayoutHeight);
                    } else if (heightMode == MeasureSpec::UNSPECIFIED) {
                        layoutHeight = childHeight;
                    }
                }
            }
        }
    }

    // At this point, all child views have been measured. Calculate the device fold position
    // in the view. Update the split position to where the fold when it exists.
    std::vector<Rect> splitViews;// = splitViewPositions();

    if (splitViews.size() && !canSlide) {
        for (int i = 0; i < childCount; i++) {
            View* child = getChildAt(i);

            if (child->getVisibility() == GONE) {
                continue;
            }

            Rect splitView = splitViews.at(i);
            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();

            // If child view cannot fit in the separating view, expand the child view to fill
            // available space.
            const int horizontalMargin = lp->leftMargin + lp->rightMargin;
            const int childHeightSpec = MeasureSpec::makeMeasureSpec(child->getMeasuredHeight(),
                    MeasureSpec::EXACTLY);
            int childWidthSpec = MeasureSpec::makeMeasureSpec(splitView.width,
                    MeasureSpec::AT_MOST);
            child->measure(childWidthSpec, childHeightSpec);
            if ((child->getMeasuredWidthAndState() & MEASURED_STATE_TOO_SMALL) == 1 || (
                    getMinimumWidth(child) != 0
                            && splitView.width < getMinimumWidth(child))) {
                childWidthSpec = MeasureSpec::makeMeasureSpec(widthAvailable - horizontalMargin,
                        MeasureSpec::EXACTLY);
                child->measure(childWidthSpec, childHeightSpec);
                // Skip first child (list pane), the list pane is always a non-sliding pane.
                if (i == 0) {
                    continue;
                }
                canSlide = lp->slideable = true;
                mSlideableView = child;
            } else {
                childWidthSpec = MeasureSpec::makeMeasureSpec(splitView.width,
                        MeasureSpec::EXACTLY);
                child->measure(childWidthSpec, childHeightSpec);
            }
        }
    }

    const int measuredWidth = widthSize;
    const int measuredHeight = layoutHeight + getPaddingTop() + getPaddingBottom();

    setMeasuredDimension(measuredWidth, measuredHeight);
    mCanSlide = canSlide;

    if (mDragHelper->getViewDragState() != ViewDragHelper::STATE_IDLE && !canSlide) {
        // Cancel scrolling in progress, it's no longer relevant.
        mDragHelper->abort();
    }
}

int SlidingPaneLayout::getMinimumWidth(View* child) {
    if (dynamic_cast<TouchBlocker*>(child)) {
        return ((TouchBlocker*)child)->getChildAt(0)->getMinimumWidth();
    }
    return child->getMinimumWidth();
}

int SlidingPaneLayout::measureChildHeight(View* child,int spec, int padding) {
    LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
    int childHeightSpec;
    const bool skippedFirstPass = lp->width == 0 && lp->weight > 0;
    if (skippedFirstPass) {
        // This was skipped the first time; figure out a real height spec.
        childHeightSpec = getChildMeasureSpec(spec, padding, lp->height);

    } else {
        childHeightSpec = MeasureSpec::makeMeasureSpec(
                child->getMeasuredHeight(), MeasureSpec::EXACTLY);
    }
    return childHeightSpec;
}

void SlidingPaneLayout::onLayout(bool changed, int l, int t, int width, int height) {
    const bool isLayoutRtl = isLayoutRtlSupport();
    const int paddingStart = isLayoutRtl ? getPaddingRight() : getPaddingLeft();
    const int paddingEnd = isLayoutRtl ? getPaddingLeft() : getPaddingRight();
    const int paddingTop = getPaddingTop();

    const int childCount = getChildCount();
    int xStart = paddingStart;
    int nextXStart = xStart;

    if (mFirstLayout) {
        mSlideOffset = mCanSlide && mPreservedOpenState ? 1.f : 0.f;
    }

    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);

        if (child->getVisibility() == GONE) {
            continue;
        }

        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();

        const int childWidth = child->getMeasuredWidth();
        int offset = 0;

        if (lp->slideable) {
            const int margin = lp->leftMargin + lp->rightMargin;
            const int range = std::min(nextXStart, width - paddingEnd) - xStart - margin;
            mSlideRange = range;
            const int lpMargin = isLayoutRtl ? lp->rightMargin : lp->leftMargin;
            lp->dimWhenOffset = xStart + lpMargin + range + childWidth / 2 > width - paddingEnd;
            const int pos = (int) (range * mSlideOffset);
            xStart += pos + lpMargin;
            mSlideOffset = (float) pos / mSlideRange;
        } else if (mCanSlide && mParallaxBy != 0) {
            offset = (int) ((1.f - mSlideOffset) * mParallaxBy);
            xStart = nextXStart;
        } else {
            xStart = nextXStart;
        }

        int childRight;
        int childLeft;
        if (isLayoutRtl) {
            childRight = width - xStart + offset;
            childLeft = childRight - childWidth;
        } else {
            childLeft = xStart - offset;
            childRight = childLeft + childWidth;
        }

        const int childTop = paddingTop;
        const int childHeight =child->getMeasuredHeight();
        child->layout(childLeft, paddingTop, childWidth, childHeight);

        // If a folding feature separates the content, we use its width as the extra
        // offset for the next child, in order to avoid rendering the content under it.
        /*if (mFoldingFeature != nullptr
                && mFoldingFeature->getOrientation() == FoldingFeature::Orientation::VERTICAL
                && mFoldingFeature->isSeparating()) {
            nextXOffset = mFoldingFeature->getBounds().width;
        }*/
        nextXStart += child->getWidth();
    }

    if (mFirstLayout) {
        if (mCanSlide) {
            if (mParallaxBy != 0) {
                parallaxOtherViews(mSlideOffset);
            }
        }
        updateObscuredViewsVisibility(mSlideableView);
    }

    mFirstLayout = false;
}

void SlidingPaneLayout::onSizeChanged(int w, int h, int oldw, int oldh) {
    ViewGroup::onSizeChanged(w, h, oldw, oldh);
    // Recalculate sliding panes and their details
    if (w != oldw) {
        mFirstLayout = true;
    }
}

void SlidingPaneLayout::requestChildFocus(View* child, View* focused) {
    ViewGroup::requestChildFocus(child, focused);
    if (!isInTouchMode() && !mCanSlide) {
        mPreservedOpenState = child == mSlideableView;
    }
}

bool SlidingPaneLayout::onInterceptTouchEvent(MotionEvent& ev) {
    const int action = ev.getActionMasked();

    // Preserve the open state based on the last view that was touched.
    if (!mCanSlide && action == MotionEvent::ACTION_DOWN && getChildCount() > 1) {
        // After the first things will be slideable.
        View* secondChild = getChildAt(1);
        if (secondChild != nullptr) {
            mPreservedOpenState = !mDragHelper->isViewUnder(secondChild,
                    (int) ev.getX(), (int) ev.getY());
        }
    }

    if (!mCanSlide || (mIsUnableToDrag && action != MotionEvent::ACTION_DOWN)) {
        mDragHelper->cancel();
        return ViewGroup::onInterceptTouchEvent(ev);
    }

    if (action == MotionEvent::ACTION_CANCEL || action == MotionEvent::ACTION_UP) {
        mDragHelper->cancel();
        return false;
    }

    bool interceptTap = false;

    switch (action) {
        case MotionEvent::ACTION_DOWN: {
            mIsUnableToDrag = false;
            const float x = ev.getX();
            const float y = ev.getY();
            mInitialMotionX = x;
            mInitialMotionY = y;

            if (mDragHelper->isViewUnder(mSlideableView, (int) x, (int) y)
                    && isDimmed(mSlideableView)) {
                interceptTap = true;
            }
            break;
        }

        case MotionEvent::ACTION_MOVE: {
            const float x = ev.getX();
            const float y = ev.getY();
            const float adx = std::abs(x - mInitialMotionX);
            const float ady = std::abs(y - mInitialMotionY);
            const int slop = mDragHelper->getTouchSlop();
            if ((adx > slop) && (ady > adx)) {
                mDragHelper->cancel();
                mIsUnableToDrag = true;
                return false;
            }
        }
    }

    const bool interceptForDrag = mDragHelper->shouldInterceptTouchEvent(ev);

    return interceptForDrag || interceptTap;
}

bool SlidingPaneLayout::onTouchEvent(MotionEvent& ev) {
    if (!mCanSlide) {
        return ViewGroup::onTouchEvent(ev);
    }

    mDragHelper->processTouchEvent(ev);

    bool wantTouchEvents = true;

    switch (ev.getActionMasked()) {
    case MotionEvent::ACTION_DOWN:
        mInitialMotionX = ev.getX();
        mInitialMotionY = ev.getY();
        break;
    case MotionEvent::ACTION_UP:
        if (isDimmed(mSlideableView)) {
            const float x = ev.getX();
            const float y = ev.getY();
            const float dx = x - mInitialMotionX;
            const float dy = y - mInitialMotionY;
            const int slop = mDragHelper->getTouchSlop();
            if (dx * dx + dy * dy < slop * slop
                    && mDragHelper->isViewUnder(mSlideableView, (int) x, (int) y)) {
                // Taps close a dimmed open pane.
                closePane(0);
                break;
            }
        }
        break;
    }

    return wantTouchEvents;
}

bool SlidingPaneLayout::closePane(int initialVelocity) {
    if(!mCanSlide){
        mPreservedOpenState = false;
    }
    if (mFirstLayout || smoothSlideTo(1.f, initialVelocity)) {
        mPreservedOpenState = false;
        return true;
    }
    return false;
}

bool SlidingPaneLayout::openPane(int initialVelocity) {
    if(!mCanSlide){
        mPreservedOpenState = true;
    }
    if (mFirstLayout || smoothSlideTo(0.f, initialVelocity)) {
        mPreservedOpenState = true;
        return true;
    }
    return false;
}

bool SlidingPaneLayout::openPane() {
    return openPane(0);
}

bool SlidingPaneLayout::closePane() {
    return closePane(0);
}

bool SlidingPaneLayout::isOpen() const{
    return !mCanSlide || (mSlideOffset == 1.f);
}

bool SlidingPaneLayout::isSlideable() const{
    return mCanSlide;
}

void SlidingPaneLayout::onPanelDragged(int newLeft) {
    if (mSlideableView == nullptr) {
        // This can happen if we're aborting motion during layout because everything now fits.
        mSlideOffset = 0;
        return;
    }
    const bool isLayoutRtl = isLayoutRtlSupport();
    const LayoutParams* lp = (LayoutParams*) mSlideableView->getLayoutParams();

    int childWidth = mSlideableView->getWidth();
    const int newStart = isLayoutRtl ? getWidth() - newLeft - childWidth : newLeft;

    const int paddingStart = isLayoutRtl ? getPaddingRight() : getPaddingLeft();
    const int lpMargin = isLayoutRtl ? lp->rightMargin : lp->leftMargin;
    const int startBound = paddingStart + lpMargin;

    mSlideOffset = (float) (newStart - startBound) / mSlideRange;

    if (mParallaxBy != 0) {
        parallaxOtherViews(mSlideOffset);
    }

    dispatchOnPanelSlide(mSlideableView);
}

bool SlidingPaneLayout::drawChild(Canvas& canvas, View* child, int64_t drawingTime) {
    const bool isLayoutRtl = isLayoutRtlSupport();
    const bool enableEdgeLeftTracking = isLayoutRtl ^ isOpen();
    if (enableEdgeLeftTracking) {
        mDragHelper->setEdgeTrackingEnabled(ViewDragHelper::EDGE_LEFT);
        Insets gestureInsets = getSystemGestureInsets();
        if (gestureInsets != Insets::NONE) {
            // Gesture insets will be 0 if the device doesn't have gesture navigation enabled.
            mDragHelper->setEdgeSize(std::max(mDragHelper->getDefaultEdgeSize(),
                    gestureInsets.left));
        }
    } else {
        mDragHelper->setEdgeTrackingEnabled(ViewDragHelper::EDGE_RIGHT);
        Insets gestureInsets = getSystemGestureInsets();
        if (gestureInsets != Insets::NONE) {
            // Gesture insets will be 0 if the device doesn't have gesture navigation enabled.
            mDragHelper->setEdgeSize(std::max(mDragHelper->getDefaultEdgeSize(),
                    gestureInsets.right));
        }
    }
    LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
    bool result;
    canvas.save();

    if (mCanSlide && !lp->slideable && mSlideableView != nullptr) {
        // Clip against the slider; no sense drawing what will immediately be covered.
        Rect rect;
        double x1, y1, x2, y2;
        canvas.get_clip_extents(x1, y1, x2, y2);//canvas.getClipBounds(rect);
        rect.left   = static_cast<int>(std::floor(x1));
        rect.top    = static_cast<int>(std::floor(y1));
        rect.width  = static_cast<int>(std::ceil(x2) - std::floor(x1));
        rect.height = static_cast<int>(std::ceil(y2) - std::floor(y1));
        if (isLayoutRtlSupport()) {
            rect.left = std::max(rect.left, mSlideableView->getRight());
        } else {
            rect.width = std::min(rect.right(), mSlideableView->getLeft())-rect.left;
        }
        canvas.rectangle(rect.left,rect.top,rect.width,rect.height);
        canvas.clip();
    }

    result = ViewGroup::drawChild(canvas, child, drawingTime);

    canvas.restore();

    return result;    
}

Insets SlidingPaneLayout::getSystemGestureInsets() {
    Insets gestureInsets;
    /*if (sEdgeSizeUsingSystemGestureInsets) {
        WindowInsets rootInsetsCompat = this->getRootWindowInsets();
        if (rootInsetsCompat != null) {
            gestureInsets = rootInsetsCompat.getSystemGestureInsets();
        }
    }*/
    return gestureInsets;
}

void SlidingPaneLayout::invalidateChildRegion(View* v) {
#if 0
    if (Build::VERSION::SDK_INT >= 17) {
        //ViewCompat.setLayerPaint(v, ((LayoutParams*) v.getLayoutParams()).dimPaint);
        return;
    }

    if (Build::VERSION::SDK_INT >= 16) {
        // Private API hacks! Nasty! Bad!
        //
        // In Jellybean, some optimizations in the hardware UI renderer
        // prevent a changed Paint on a View using a hardware layer from having
        // the intended effect. This twiddles some internal bits on the view to force
        // it to recreate the display list.
        if (!mDisplayListReflectionLoaded) {
            try {
                mGetDisplayList = View.class.getDeclaredMethod("getDisplayList", (Class[]) null);
            } catch (NoSuchMethodException e) {
                LOGE("Couldn't fetch getDisplayList method; dimming won't work right.",e);
            }
            try {
                mRecreateDisplayList = View.class.getDeclaredField("mRecreateDisplayList");
                mRecreateDisplayList.setAccessible(true);
            } catch (NoSuchFieldException e) {
                LOGE("Couldn't fetch mRecreateDisplayList field; dimming will be slow.",e);
            }
            mDisplayListReflectionLoaded = true;
        }
        if (mGetDisplayList == null || mRecreateDisplayList == null) {
            // Slow path. REALLY slow path. Let's hope we don't get here.
            v->invalidate();
            return;
        }

        try {
            mRecreateDisplayList.setBoolean(v, true);
            mGetDisplayList.invoke(v, (Object[]) null);
        } catch (Exception e) {
            LOGE("Error refreshing display list state", e);
        }
    }
#endif
    this->postInvalidateOnAnimation(v->getLeft(), v->getTop(), v->getWidth(),v->getHeight());
}

bool SlidingPaneLayout::smoothSlideTo(float slideOffset, int velocity) {
    if (!mCanSlide) {
        // Nothing to do.
        return false;
    }

    const bool isLayoutRtl = isLayoutRtlSupport();
    const LayoutParams* lp = (LayoutParams*) mSlideableView->getLayoutParams();

    int x;
    if (isLayoutRtl) {
        const int startBound = getPaddingRight() + lp->rightMargin;
        const int childWidth = mSlideableView->getWidth();
        x = (int) (getWidth() - (startBound + slideOffset * mSlideRange + childWidth));
    } else {
        const int startBound = getPaddingLeft() + lp->leftMargin;
        x = (int) (startBound + slideOffset * mSlideRange);
    }

    if (mDragHelper->smoothSlideViewTo(mSlideableView, x, mSlideableView->getTop())) {
        setAllChildrenVisible();
        postInvalidateOnAnimation();
        return true;
    }
    return false;
}

void SlidingPaneLayout::computeScroll() {
    if (mDragHelper->continueSettling(true)) {
        if (!mCanSlide) {
            mDragHelper->abort();
            return;
        }
        postInvalidateOnAnimation();
    }
}

void SlidingPaneLayout::setShadowDrawableLeft(Drawable* d) {
    mShadowDrawableLeft = d;
}

void SlidingPaneLayout::setShadowDrawableRight(Drawable* d) {
    mShadowDrawableRight = d;
}

void SlidingPaneLayout::setShadowResourceLeft(const std::string& resId) {
    setShadowDrawableLeft(getContext()->getDrawable(resId));
}

void SlidingPaneLayout::setShadowResourceRight(const std::string& resId) {
    setShadowDrawableRight(getContext()->getDrawable(resId));
}

void SlidingPaneLayout::draw(Canvas& c) {
    ViewGroup::draw(c);
    const bool isLayoutRtl = isLayoutRtlSupport();
    Drawable* shadowDrawable;
    if (isLayoutRtl) {
        shadowDrawable = mShadowDrawableRight;
    } else {
        shadowDrawable = mShadowDrawableLeft;
    }

    View* shadowView = getChildCount() > 1 ? getChildAt(1) : nullptr;
    if (shadowView == nullptr || shadowDrawable == nullptr) {
        // No need to draw a shadow if we don't have one.
        return;
    }

    const int top = shadowView->getTop();
    const int bottom = shadowView->getBottom();

    const int shadowWidth = shadowDrawable->getIntrinsicWidth();
    int left;
    int right;
    if (isLayoutRtlSupport()) {
        left = shadowView->getRight();
        right = left + shadowWidth;
    } else {
        right = shadowView->getLeft();
        left = right - shadowWidth;
    }

    shadowDrawable->setBounds(left, top, right, bottom);
    shadowDrawable->draw(c);
}

void SlidingPaneLayout::parallaxOtherViews(float slideOffset) {
    const bool isLayoutRtl = isLayoutRtlSupport();
    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* v = getChildAt(i);
        if (v == mSlideableView) continue;

        const int oldOffset = (int) ((1.f - mParallaxOffset) * mParallaxBy);
        mParallaxOffset = slideOffset;
        const int newOffset = (int) ((1.f - slideOffset) * mParallaxBy);
        const int dx = oldOffset - newOffset;

        v->offsetLeftAndRight(isLayoutRtl ? -dx : dx);

    }
}

bool SlidingPaneLayout::canScroll(View* v, bool checkV, int dx, int x, int y) {
    if (dynamic_cast<ViewGroup*>(v)) {
        ViewGroup* group = (ViewGroup*) v;
        const int scrollX = v->getScrollX();
        const int scrollY = v->getScrollY();
        const int count = group->getChildCount();
        // Count backwards - let topmost views consume scroll distance first.
        for (int i = count - 1; i >= 0; i--) {
            // TODO: Add versioned support here for transformed views.
            // This will not work for transformed views in Honeycomb+
            View* child = group->getChildAt(i);
            if (x + scrollX >= child->getLeft() && x + scrollX < child->getRight()
                    && y + scrollY >= child->getTop() && y + scrollY < child->getBottom()
                    && canScroll(child, true, dx, x + scrollX - child->getLeft(),
                            y + scrollY - child->getTop())) {
                return true;
            }
        }
    }

    return checkV && v->canScrollHorizontally((isLayoutRtlSupport() ? dx : -dx));
}

bool SlidingPaneLayout::isDimmed(View* child) const{
    if (child == nullptr) {
        return false;
    }
    const LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
    return mCanSlide && lp->dimWhenOffset && mSlideOffset > 0;
}

ViewGroup::LayoutParams* SlidingPaneLayout::generateDefaultLayoutParams() const{
    return new LayoutParams();
}

ViewGroup::LayoutParams* SlidingPaneLayout::generateLayoutParams(const ViewGroup::LayoutParams* p) const{
    return dynamic_cast<const MarginLayoutParams*>(p)
            ? new LayoutParams((const MarginLayoutParams&)*p)
            : new LayoutParams(*p);
}

bool SlidingPaneLayout::checkLayoutParams(const ViewGroup::LayoutParams* p) const{
    return dynamic_cast<const LayoutParams*>(p) && ViewGroup::checkLayoutParams(p);
}

ViewGroup::LayoutParams* SlidingPaneLayout::generateLayoutParams(const AttributeSet& attrs) const{
    return new LayoutParams(getContext(), attrs);
}

Parcelable* SlidingPaneLayout::onSaveInstanceState() {
    /*Parcelable* superState = ViewGroup::onSaveInstanceState();

    SavedState* ss = new SavedState(superState);
    ss.isOpen = isSlideable() ? isOpen() : mPreservedOpenState;
    ss.mLockMode = mLockMode;
    return ss;*/
    return nullptr;
}

void SlidingPaneLayout::onRestoreInstanceState(Parcelable& state) {
    /*if (!(state instanceof SavedState)) {
        ViewGroup::onRestoreInstanceState(state);
        return;
    }

    SavedState ss = (SavedState) state;
    ViewGroup::onRestoreInstanceState(ss.getSuperState());

    if (ss.isOpen) {
        openPane();
    } else {
        closePane();
    }
    mPreservedOpenState = ss.isOpen;
    setLockMode(ss.mLockMode);*/
}

bool SlidingPaneLayout::isLayoutRtlSupport() const{
    return this->getLayoutDirection() == View::LAYOUT_DIRECTION_RTL;
}

/*std::vector<Rect> SlidingPaneLayout::splitViewPositions() {
    if (mFoldingFeature == null || !mFoldingFeature.isSeparating()) {
        return null;
    }

    // Don't support horizontal fold in list-detail view layout
    if (mFoldingFeature.getBounds().left == 0) {
        return null;
    }
    // vertical split
    if (mFoldingFeature.getBounds().top == 0) {
        Rect splitPosition = getFoldBoundsInView(mFoldingFeature, this);
        if (splitPosition == null) {
            return null;
        }
        Rect leftRect = new Rect(getPaddingLeft(), getPaddingTop(),
                Math.max(getPaddingLeft(), splitPosition.left),
                getHeight() - getPaddingBottom());
        int rightBound = getWidth() - getPaddingRight();
        Rect rightRect = new Rect(Math.min(rightBound, splitPosition.right),
                getPaddingTop(), rightBound, getHeight() - getPaddingBottom());
        return new ArrayList<>(Arrays.asList(leftRect, rightRect));
    }
    return null;
}*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//private class DragHelperCallback extends ViewDragHelper.Callback {

SlidingPaneLayout::DragHelperCallback::DragHelperCallback(SlidingPaneLayout*spl):mSPL(spl) {
}

bool SlidingPaneLayout::DragHelperCallback::tryCaptureView(View& child, int pointerId) {
    if (isDraggable()) {
        return false;
    }

    return ((LayoutParams*) child.getLayoutParams())->slideable;
}

void SlidingPaneLayout::DragHelperCallback::onViewDragStateChanged(int state) {
    if (mSPL->mDragHelper->getViewDragState() == ViewDragHelper::STATE_IDLE) {
        if (mSPL->mSlideOffset == 0) {
            mSPL->updateObscuredViewsVisibility(mSPL->mSlideableView);
            mSPL->dispatchOnPanelClosed(mSPL->mSlideableView);
            mSPL->mPreservedOpenState = false;
        } else {
            mSPL->dispatchOnPanelOpened(mSPL->mSlideableView);
            mSPL->mPreservedOpenState = true;
        }
    }
}

void SlidingPaneLayout::DragHelperCallback::onViewCaptured(View& capturedChild, int activePointerId) {
    // Make all child views visible in preparation for sliding things around
    mSPL->setAllChildrenVisible();
}

void SlidingPaneLayout::DragHelperCallback::onViewPositionChanged(View& changedView, int left, int top, int dx, int dy) {
    mSPL->onPanelDragged(left);
    mSPL->invalidate();
}

void SlidingPaneLayout::DragHelperCallback::onViewReleased(View& releasedChild, float xvel, float yvel) {
    const LayoutParams* lp = (LayoutParams*) releasedChild.getLayoutParams();

    int left;
    if (mSPL->isLayoutRtlSupport()) {
        int startToRight =  mSPL->getPaddingRight() + lp->rightMargin;
        if (xvel < 0 || (xvel == 0 && mSPL->mSlideOffset > 0.5f)) {
            startToRight += mSPL->mSlideRange;
        }
        const int childWidth = mSPL->mSlideableView->getWidth();
        left = mSPL->getWidth() - startToRight - childWidth;
    } else {
        left = mSPL->getPaddingLeft() + lp->leftMargin;
        if (xvel > 0 || (xvel == 0 && mSPL->mSlideOffset > 0.5f)) {
            left += mSPL->mSlideRange;
        }
    }
    mSPL->mDragHelper->settleCapturedViewAt(left, releasedChild.getTop());
    mSPL->invalidate();
}

int SlidingPaneLayout::DragHelperCallback::getViewHorizontalDragRange(View& child) {
    return mSPL->mSlideRange;
}

int SlidingPaneLayout::DragHelperCallback::clampViewPositionHorizontal(View& child, int left, int dx) {
    const LayoutParams* lp = (LayoutParams*) mSPL->mSlideableView->getLayoutParams();

    int newLeft;
    if (mSPL->isLayoutRtlSupport()) {
        const int startBound = mSPL->getWidth() - (mSPL->getPaddingRight() + lp->rightMargin + mSPL->mSlideableView->getWidth());
        const int endBound =  startBound - mSPL->mSlideRange;
        newLeft = std::max(std::min(left, startBound), endBound);
    } else {
        const int startBound = mSPL->getPaddingLeft() + lp->leftMargin;
        const int endBound = startBound + mSPL->mSlideRange;
        newLeft = std::min(std::max(left, startBound), endBound);
    }
    return newLeft;
}

int SlidingPaneLayout::DragHelperCallback::clampViewPositionVertical(View& child, int top, int dy) {
    // Make sure we never move views vertically.
    // This could happen if the child has less height than its parent.
    return child.getTop();
}

void SlidingPaneLayout::DragHelperCallback::onEdgeTouched(int edgeFlags, int pointerId) {
    if (!isDraggable()) {
        return;
    }
    mSPL->mDragHelper->captureChildView(mSPL->mSlideableView, pointerId);
}

void SlidingPaneLayout::DragHelperCallback::onEdgeDragStarted(int edgeFlags, int pointerId) {
    if(!isDraggable()) return;
    mSPL->mDragHelper->captureChildView(mSPL->mSlideableView, pointerId);
}

bool SlidingPaneLayout::DragHelperCallback::isDraggable() const{
    if (mSPL->mIsUnableToDrag) {
        return false;
    }
    if (mSPL->getLockMode() == LOCK_MODE_LOCKED) {
        return false;
    }
    if (mSPL->isOpen() && mSPL->getLockMode() == LOCK_MODE_LOCKED_OPEN) {
        return false;
    }
    if (!mSPL->isOpen() && mSPL->getLockMode() == LOCK_MODE_LOCKED_CLOSED) {
        return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//public static class LayoutParams extends ViewGroup.MarginLayoutParams {

SlidingPaneLayout::LayoutParams::LayoutParams()
    :ViewGroup::MarginLayoutParams(MATCH_PARENT, MATCH_PARENT){
}

SlidingPaneLayout::LayoutParams::LayoutParams(int width, int height)
    :ViewGroup::MarginLayoutParams(width, height){
}

SlidingPaneLayout::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& source)
    :ViewGroup::MarginLayoutParams(source){
}

SlidingPaneLayout::LayoutParams::LayoutParams(const MarginLayoutParams& source)
    :ViewGroup::MarginLayoutParams(source){
}

SlidingPaneLayout::LayoutParams::LayoutParams(const LayoutParams& source)
    :ViewGroup::MarginLayoutParams(source){
    this->weight = source.weight;
}

SlidingPaneLayout::LayoutParams::LayoutParams(Context* c, const AttributeSet& attrs)
    :ViewGroup::MarginLayoutParams(c, attrs){

    this->weight = attrs.getFloat("weight", 0);
}

#if 0
/////////////////////////////////////////////////////////////////////////////////////////////////
//static class SavedState extends AbsSavedState

SlidingPaneLayout::SavedState::SavedState(Parcelable* superState):AbsSavedState(superState){
}

/*SlidingPaneLayout::SavedState::SavedState(Parcel& in, ClassLoader loader) {
    super(in, loader);
    isOpen = in.readInt() != 0;
}*/

void SlidingPaneLayout::SavedState::writeToParcel(Parcel& out, int flags) {
    AbsSavedState::writeToParcel(out, flags);
    out.writeInt(isOpen ? 1 : 0);
}


//class AccessibilityDelegate extends AccessibilityDelegate {
void SlidingPaneLayout::AccessibilityDelegate::onInitializeAccessibilityNodeInfo(View& host, AccessibilityNodeInfo& info) {
    AccessibilityNodeInfo* superNode = AccessibilityNodeInfo::obtain(info);
    View::AccessibilityDelegate::onInitializeAccessibilityNodeInfo(host, superNode);
    copyNodeInfoNoChildren(info, superNode);
    superNode.recycle();

    info.setClassName("SlidingPaneLayout");
    info.setSource(host);

    final ViewParent parent = ViewCompat.getParentForAccessibility(host);
    if (parent instanceof View) {
        info.setParent((View) parent);
    }

    // This is a best-approximation of addChildrenForAccessibility()
    // that accounts for filtering.
    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        if (!filter(child) && (child->getVisibility() == View::VISIBLE)) {
            // Force importance to "yes" since we can't read the value.
            child->setImportantForAccessibility(View::IMPORTANT_FOR_ACCESSIBILITY_YES);
            info.addChild(child);
        }
    }
}

void SlidingPaneLayout::AccessibilityDelegate::onInitializeAccessibilityEvent(View& host, AccessibilityEvent& event) {
    View::AccessibilityDelegate::onInitializeAccessibilityEvent(host, event);
    event.setClassName("SlidingPaneLayout");
}

bool SlidingPaneLayout::AccessibilityDelegate::onRequestSendAccessibilityEvent(ViewGroup& host, View* child, AccessibilityEvent& event) {
    if (!filter(child)) {
        return View::AccessibilityDelegate::onRequestSendAccessibilityEvent(host, child, event);
    }
    return false;
}

bool SlidingPaneLayout::AccessibilityDelegate::filter(View child) {
    return isDimmed(child);
}

void SlidingPaneLayout::AccessibilityDelegate::copyNodeInfoNoChildren(AccessibilityNodeInfo& dest, AccessibilityNodeInfo& src) {
    Rect rect;

    src.getBoundsInParent(rect);
    dest.setBoundsInParent(rect);

    src.getBoundsInScreen(rect);
    dest.setBoundsInScreen(rect);

    dest.setVisibleToUser(src.isVisibleToUser());
    dest.setPackageName(src.getPackageName());
    dest.setClassName(src.getClassName());
    dest.setContentDescription(src.getContentDescription());

    dest.setEnabled(src.isEnabled());
    dest.setClickable(src.isClickable());
    dest.setFocusable(src.isFocusable());
    dest.setFocused(src.isFocused());
    dest.setAccessibilityFocused(src.isAccessibilityFocused());
    dest.setSelected(src.isSelected());
    dest.setLongClickable(src.isLongClickable());

    dest.addAction(src.getActions());

    dest.setMovementGranularities(src.getMovementGranularities());
}
#endif

SlidingPaneLayout::DisableLayerRunnable::DisableLayerRunnable(View*v,View* childView):ViewRunnable(v) {
    mChildView = childView;
}

void SlidingPaneLayout::DisableLayerRunnable::run() {
    SlidingPaneLayout*spl=(SlidingPaneLayout*)mView;
    if (mChildView->getParent() == mView) {
        mChildView->setLayerType(View::LAYER_TYPE_NONE/*, nullptr*/);
        spl->invalidateChildRegion(mChildView);
    }
    //spl->mPostedRunnables.remove(this);
}

}/*endof namespace*/
