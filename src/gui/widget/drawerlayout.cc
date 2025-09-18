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
#include <core/build.h>
#include <widget/drawerlayout.h>
#include <porting/cdlog.h>
namespace cdroid{

DECLARE_WIDGET(DrawerLayout)

DrawerLayout::DrawerLayout(int w,int h):ViewGroup(w,h){
    initView();
}

void DrawerLayout::initView(){
    mInLayout = false;
    mStatusBarBackground = nullptr;
    mShadowLeftResolved  = nullptr;
    mShadowRightResolved = nullptr;
    mShadowStart= mShadowEnd  = nullptr;
    mShadowLeft = mShadowRight= nullptr;
    setDescendantFocusability(ViewGroup::FOCUS_AFTER_DESCENDANTS);
    const float density = mContext->getDisplayMetrics().density;
    mMinDrawerMargin = (int) (MIN_DRAWER_MARGIN * density + 0.5f);
    float minVel = MIN_FLING_VELOCITY * density;

    mLeftCallback  = new ViewDragCallback(this,Gravity::LEFT);
    mRightCallback = new ViewDragCallback(this,Gravity::RIGHT);

    mLeftDragger = ViewDragHelper::create(this, TOUCH_SLOP_SENSITIVITY, mLeftCallback);
    mLeftDragger->setEdgeTrackingEnabled(ViewDragHelper::EDGE_LEFT);
    mLeftDragger->setMinVelocity(minVel);
    mLeftCallback->setDragger(mLeftDragger);

    mRightDragger = ViewDragHelper::create(this, TOUCH_SLOP_SENSITIVITY, mRightCallback);
    mRightDragger->setEdgeTrackingEnabled(ViewDragHelper::EDGE_RIGHT);
    mRightDragger->setMinVelocity(minVel);
    mRightCallback->setDragger(mRightDragger);

    // So that we can catch the back button
    setFocusableInTouchMode(true);

    setMotionEventSplittingEnabled(false);
    mDrawerElevation = DRAWER_ELEVATION * density;
}

DrawerLayout::DrawerLayout(Context*ctx,const AttributeSet&atts)
  :ViewGroup(ctx,atts){
    initView();
}

DrawerLayout::~DrawerLayout(){
    delete mLeftCallback;
    delete mRightCallback;

    delete mLeftDragger;
    delete mRightDragger;
}

void DrawerLayout::setDrawerElevation(float elevation) {
    mDrawerElevation = elevation;
    for (int i = 0; i < getChildCount(); i++) {
        View* child = getChildAt(i);
        if (isDrawerView(child)) {
            child->setElevation(mDrawerElevation);
        }
    }
}

float DrawerLayout::getDrawerElevation()const{
    return mDrawerElevation;
}

void DrawerLayout::setDrawerShadow(Drawable* shadowDrawable,int gravity){
    /*
     * TODO Someone someday might want to set more complex drawables here.
     * They're probably nuts, but we might want to consider registering callbacks,
     * setting states, etc. properly.
     */
    if (SET_DRAWER_SHADOW_FROM_ELEVATION) {
        // No op. Drawer shadow will come from setting an elevation on the drawer.
        return;
    }
    if ((gravity & Gravity::START) == Gravity::START) {
        mShadowStart = shadowDrawable;
    } else if ((gravity & Gravity::END) == Gravity::END) {
        mShadowEnd = shadowDrawable;
    } else if ((gravity & Gravity::LEFT) == Gravity::LEFT) {
        mShadowLeft = shadowDrawable;
    } else if ((gravity & Gravity::RIGHT) == Gravity::RIGHT) {
        mShadowRight = shadowDrawable;
    } else {
        return;
    }
    resolveShadowDrawables();
    invalidate();
}

void DrawerLayout::setDrawerShadow(const std::string&resId,int gravity) {
    setDrawerShadow(mContext->getDrawable(resId), gravity);
}

void DrawerLayout::setScrimColor(int color) {
    mScrimColor = color;
    invalidate();
}

void DrawerLayout::addDrawerListener(const DrawerListener& listener) {
    mListeners.push_back(listener);
}

void DrawerLayout::removeDrawerListener(const DrawerListener& listener){
    auto it = std::find(mListeners.begin(),mListeners.end(),listener);
    if(it!=mListeners.end()){
        mListeners.erase(it);
    }
}

void DrawerLayout::setDrawerLockMode(int lockMode) {
    setDrawerLockMode(lockMode, Gravity::LEFT);
    setDrawerLockMode(lockMode, Gravity::RIGHT);
}

void DrawerLayout::setDrawerLockMode(int lockMode,int edgeGravity) {
    int absGravity = Gravity::getAbsoluteGravity(edgeGravity,getLayoutDirection());

    switch (edgeGravity) {
    case Gravity::LEFT:  mLockModeLeft = lockMode;   break;
    case Gravity::RIGHT: mLockModeRight = lockMode;  break;
    case Gravity::START: mLockModeStart = lockMode;  break;
    case Gravity::END:   mLockModeEnd = lockMode;    break;
    }

    if (lockMode != LOCK_MODE_UNLOCKED) {
        // Cancel interaction in progress
        ViewDragHelper* helper = absGravity == Gravity::LEFT ? mLeftDragger : mRightDragger;
        helper->cancel();
    }
    View*toOpen,*toClose;
    switch (lockMode) {
    case LOCK_MODE_LOCKED_OPEN:
        toOpen = findDrawerWithGravity(absGravity);
        if (toOpen) openDrawer(toOpen);
        break;
    case LOCK_MODE_LOCKED_CLOSED:
        toClose = findDrawerWithGravity(absGravity);
        if (toClose) closeDrawer(toClose);
        break;
     default: break;//do nothing
   }
}

void DrawerLayout::setDrawerLockMode(int lockMode,View* drawerView) {
    LOGE_IF(!isDrawerView(drawerView),"View %p:%d is not a drawer with appropriate layout_gravity",drawerView,drawerView->getId());
    const int gravity = ((LayoutParams*) drawerView->getLayoutParams())->gravity;
    setDrawerLockMode(lockMode, gravity);
}


int DrawerLayout::getDrawerLockMode(int edgeGravity) {
    const int layoutDirection = getLayoutDirection();
    int lockMode;
    switch (edgeGravity) {
    case Gravity::LEFT:
        if (mLockModeLeft != LOCK_MODE_UNDEFINED) {
            return mLockModeLeft;
        }
        lockMode = (layoutDirection == View::LAYOUT_DIRECTION_LTR) ? mLockModeStart : mLockModeEnd;
        if (lockMode != LOCK_MODE_UNDEFINED) {
           return lockMode;
        }
        break;
    case Gravity::RIGHT:
        if (mLockModeRight != LOCK_MODE_UNDEFINED) {
           return mLockModeRight;
        }
        lockMode = (layoutDirection == View::LAYOUT_DIRECTION_LTR) ? mLockModeEnd : mLockModeStart;
        if (lockMode != LOCK_MODE_UNDEFINED) {
            return lockMode;
        }
        break;
    case Gravity::START:
        if (mLockModeStart != LOCK_MODE_UNDEFINED) {
           return mLockModeStart;
        }
        lockMode = (layoutDirection == View::LAYOUT_DIRECTION_LTR) ? mLockModeLeft : mLockModeRight;
        if (lockMode != LOCK_MODE_UNDEFINED) {
            return lockMode;
        }
        break;
    case Gravity::END:
        if (mLockModeEnd != LOCK_MODE_UNDEFINED) {
           return mLockModeEnd;
        }
        lockMode = (layoutDirection == View::LAYOUT_DIRECTION_LTR) ? mLockModeRight : mLockModeLeft;
        if (lockMode != LOCK_MODE_UNDEFINED) {
           return lockMode;
        }
        break;
   }
   return LOCK_MODE_UNLOCKED;
}

int DrawerLayout::getDrawerLockMode(View* drawerView) {
    LOGE_IF(!isDrawerView(drawerView),"View %p:%d is not a drawer",drawerView,drawerView->getId());
    const int drawerGravity = ((LayoutParams*) drawerView->getLayoutParams())->gravity;
    return getDrawerLockMode(drawerGravity);
}

void DrawerLayout::setDrawerTitle(int edgeGravity,const std::string& title) {
    int absGravity = Gravity::getAbsoluteGravity(edgeGravity,getLayoutDirection());
    if (absGravity == Gravity::LEFT) {
        mTitleLeft = title;
    } else if (absGravity == Gravity::RIGHT) {
        mTitleRight = title;
    }
}

const std::string DrawerLayout::getDrawerTitle(int edgeGravity) {
    int absGravity = Gravity::getAbsoluteGravity(edgeGravity, getLayoutDirection());
    if (absGravity == Gravity::LEFT) {
        return mTitleLeft;
    } else if (absGravity == Gravity::RIGHT) {
        return mTitleRight;
    }
    return std::string();
}

bool DrawerLayout::isInBoundsOfChild(float x, float y, View* child) {
    child->getHitRect(mChildHitRect);
    return mChildHitRect.contains((int) x, (int) y);
}

bool DrawerLayout::dispatchTransformedGenericPointerEvent(MotionEvent& event, View* child){
   bool handled = false;;
   if (!child->hasIdentityMatrix()) {
      Matrix childMatrix = child->getMatrix();
      MotionEvent* transformedEvent = getTransformedMotionEvent(event, child);
      handled = child->dispatchGenericMotionEvent(*transformedEvent);
      transformedEvent->recycle();
   } else {
      const float offsetX = float(getScrollX() - child->getLeft());
      const float offsetY = float(getScrollY() - child->getTop());
      event.offsetLocation(offsetX, offsetY);
      handled = child->dispatchGenericMotionEvent(event);
      event.offsetLocation(-offsetX, -offsetY);
   }
   return handled;
}

/**
* Copied from ViewGroup#getTransformedMotionEvent(MotionEvent, View) then  modified in order to
* make calls that are otherwise too visibility restricted to make.
*/
MotionEvent* DrawerLayout::getTransformedMotionEvent(MotionEvent& event, View* child) {
    const float offsetX = getScrollX() - child->getLeft();
    const float offsetY = getScrollY() - child->getTop();
    MotionEvent* transformedEvent = MotionEvent::obtain(event);
    transformedEvent->offsetLocation(offsetX, offsetY);
    Matrix childMatrix = child->getMatrix();
    if (!child->hasIdentityMatrix()){//!childMatrix.isIdentity()) {
        mChildInvertedMatrix = childMatrix;
        mChildInvertedMatrix.invert();//childMatrix.invert(mChildInvertedMatrix);
        transformedEvent->transform(mChildInvertedMatrix);
    }
    return transformedEvent;
}

void DrawerLayout::updateDrawerState(int forGravity,int activeState, View* activeDrawer) {
    int leftState = mLeftDragger->getViewDragState();
    int rightState = mRightDragger->getViewDragState();

    int state;
    if (leftState == STATE_DRAGGING || rightState == STATE_DRAGGING) {
        state = STATE_DRAGGING;
    } else if (leftState == STATE_SETTLING || rightState == STATE_SETTLING) {
        state = STATE_SETTLING;
    } else {
        state = STATE_IDLE;
    }

    if (activeDrawer && activeState == STATE_IDLE) {
        LayoutParams* lp = (LayoutParams*) activeDrawer->getLayoutParams();
        if (lp->onScreen == 0) {
            dispatchOnDrawerClosed(activeDrawer);
        } else if (lp->onScreen == 1) {
            dispatchOnDrawerOpened(activeDrawer);
        }
    }

    if (state != mDrawerState) {
        mDrawerState = state;

        // Notify the listeners. Do that from the end of the list so that if a listener
        // removes itself as the result of being called, it won't mess up with our iteration
        const int listenerCount = (int)mListeners.size();
        for (int i = listenerCount - 1; i >= 0; i--) {
            if(mListeners.at(i).onDrawerStateChanged)
                mListeners.at(i).onDrawerStateChanged(state);
        }
    }
}

void DrawerLayout::dispatchOnDrawerClosed(View* drawerView) {
    LayoutParams* lp = (LayoutParams*) drawerView->getLayoutParams();
    if ((lp->openState & LayoutParams::FLAG_IS_OPENED) == 1) {
        lp->openState = 0;

        // Notify the listeners. Do that from the end of the list so that if a listener
        // removes itself as the result of being called, it won't mess up with our iteration
        const int listenerCount = (int)mListeners.size();
        for (int i = listenerCount - 1; i >= 0; i--) {
            if(mListeners.at(i).onDrawerClosed)
                mListeners.at(i).onDrawerClosed(*drawerView);
        }

        updateChildrenImportantForAccessibility(drawerView, false);

        // Only send WINDOW_STATE_CHANGE if the host has window focus. This
        // may change if support for multiple foreground windows (e.g. IME)
        // improves.
        if (hasWindowFocus()) {
            View* rootView = getRootView();
            if (rootView != nullptr) {
                rootView->sendAccessibilityEvent(AccessibilityEvent::TYPE_WINDOW_STATE_CHANGED);
            }
        }
    }
}

void DrawerLayout::dispatchOnDrawerOpened(View* drawerView) {
    LayoutParams* lp = (LayoutParams*) drawerView->getLayoutParams();
    if ((lp->openState & LayoutParams::FLAG_IS_OPENED) == 0) {
        lp->openState = LayoutParams::FLAG_IS_OPENED;
        // Notify the listeners. Do that from the end of the list so that if a listener
        // removes itself as the result of being called, it won't mess up with our iteration
        const int listenerCount = (int)mListeners.size();
        for (int i = listenerCount - 1; i >= 0; i--) {
            if(mListeners.at(i).onDrawerOpened)
                mListeners.at(i).onDrawerOpened(*drawerView);
        }

        updateChildrenImportantForAccessibility(drawerView, true);

        // Only send WINDOW_STATE_CHANGE if the host has window focus.
        if (hasWindowFocus()) {
            sendAccessibilityEvent(AccessibilityEvent::TYPE_WINDOW_STATE_CHANGED);
        }
    }
}

void DrawerLayout::updateChildrenImportantForAccessibility(View* drawerView, bool isDrawerOpen) {
    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        if ((!isDrawerOpen && !isDrawerView(child)) || (isDrawerOpen && child == drawerView)) {
            // Drawer is closed and this is a content view or this is an
            // open drawer view, so it should be visible.
            child->setImportantForAccessibility(View::IMPORTANT_FOR_ACCESSIBILITY_YES);
        } else {
            child->setImportantForAccessibility(View::IMPORTANT_FOR_ACCESSIBILITY_NO_HIDE_DESCENDANTS);
        }
    }
}

void DrawerLayout::updateChildAccessibilityAction(View* child) {
    /*child->removeAccessibilityAction(ACTION_DISMISS.getId());
    if (isDrawerOpen(child)  && getDrawerLockMode(child) != LOCK_MODE_LOCKED_OPEN) {
        child->replaceAccessibilityAction(ACTION_DISMISS, nullptr, mActionDismiss);
    }*/
}

void DrawerLayout::dispatchOnDrawerSlide(View* drawerView, float slideOffset) {
    // Notify the listeners. Do that from the end of the list so that if a listener
    // removes itself as the result of being called, it won't mess up with our iteration
    const int listenerCount = (int)mListeners.size();
    for (int i = listenerCount - 1; i >= 0; i--) {
        if(mListeners.at(i).onDrawerSlide)
            mListeners.at(i).onDrawerSlide(*drawerView, slideOffset);
    }
}

void DrawerLayout::setDrawerViewOffset(View* drawerView, float slideOffset) {
    LayoutParams* lp = (LayoutParams*) drawerView->getLayoutParams();
    if (slideOffset == lp->onScreen) {
        return;
    }

    lp->onScreen = slideOffset;
    dispatchOnDrawerSlide(drawerView, slideOffset);
}

float DrawerLayout::getDrawerViewOffset(View* drawerView) {
    return ((LayoutParams*) drawerView->getLayoutParams())->onScreen;
}

/* @return the absolute gravity of the child drawerView, resolved according
 * to the current layout direction */
int DrawerLayout::getDrawerViewAbsoluteGravity(View* drawerView) {
    const int gravity = ((LayoutParams*) drawerView->getLayoutParams())->gravity;
    return Gravity::getAbsoluteGravity(gravity, getLayoutDirection());
}

bool DrawerLayout::checkDrawerViewAbsoluteGravity(View* drawerView, int checkFor) {
    int absGravity = getDrawerViewAbsoluteGravity(drawerView);
    return (absGravity & checkFor) == checkFor;
}

View* DrawerLayout::findOpenDrawer() {
    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        LayoutParams* childLp = (LayoutParams*) child->getLayoutParams();
        if ((childLp->openState & LayoutParams::FLAG_IS_OPENED) == 1) {
            return child;
        }
    }
    return nullptr;
}

void DrawerLayout::moveDrawerToOffset(View* drawerView, float slideOffset) {
    const float oldOffset = getDrawerViewOffset(drawerView);
    const int width = drawerView->getWidth();
    const int oldPos = (int) (width * oldOffset);
    const int newPos = (int) (width * slideOffset);
    const int dx = newPos - oldPos;

    drawerView->offsetLeftAndRight(
            checkDrawerViewAbsoluteGravity(drawerView, Gravity::LEFT) ? dx : -dx);
    setDrawerViewOffset(drawerView, slideOffset);
}

/**
 * @param gravity the gravity of the child to return. If specified as a
 *            relative value, it will be resolved according to the current
 *            layout direction.
 * @return the drawer with the specified gravity
 */
View* DrawerLayout::findDrawerWithGravity(int gravity) {
    const int absHorizGravity = Gravity::getAbsoluteGravity(
            gravity, getLayoutDirection()) & Gravity::HORIZONTAL_GRAVITY_MASK;
    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        const int childAbsGravity = getDrawerViewAbsoluteGravity(child);
        if ((childAbsGravity & Gravity::HORIZONTAL_GRAVITY_MASK) == absHorizGravity) {
            return child;
        }
    }
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        const int childAbsGravity = getDrawerViewAbsoluteGravity(child);
        if ((childAbsGravity & Gravity::VERTICAL_GRAVITY_MASK) == gravity) {
            return child;
        }
    }
    return nullptr;
}

void DrawerLayout::onDetachedFromWindow() {
    ViewGroup::onDetachedFromWindow();
    mFirstLayout = true;
}

void DrawerLayout::onAttachedToWindow() {
    ViewGroup::onAttachedToWindow();
    mFirstLayout = true;
}

void DrawerLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    int widthMode = MeasureSpec::getMode(widthMeasureSpec);
    int heightMode= MeasureSpec::getMode(heightMeasureSpec);
    int widthSize = MeasureSpec::getSize(widthMeasureSpec);
    int heightSize= MeasureSpec::getSize(heightMeasureSpec);

    if ((widthMode != MeasureSpec::EXACTLY) || (heightMode != MeasureSpec::EXACTLY)) {
        if (isInEditMode()) {
            // Don't crash the layout editor. Consume all of the space if specified
            // or pick a magic number from thin air otherwise.
            // TODO Better communication with tools of this bogus state.
            // It will crash on a real device.
            if (widthMode == MeasureSpec::AT_MOST) {
                widthMode = MeasureSpec::EXACTLY;
            } else if (widthMode == MeasureSpec::UNSPECIFIED) {
                widthMode = MeasureSpec::EXACTLY;
                widthSize = 300;
            }
            if (heightMode == MeasureSpec::AT_MOST) {
                heightMode = MeasureSpec::EXACTLY;
            } else if (heightMode == MeasureSpec::UNSPECIFIED) {
                heightMode = MeasureSpec::EXACTLY;
                heightSize = 300;
            }
        } else {
            LOGE("DrawerLayout must be measured with MeasureSpec::EXACTLY.");
        }
    }

    setMeasuredDimension(widthSize, heightSize);

    bool applyInsets = false;//mLastInsets != null && getFitsSystemWindows();
    int layoutDirection = getLayoutDirection();

    // Only one drawer is permitted along each vertical edge (left / right). These two booleans
    // are tracking the presence of the edge drawers.
    bool hasDrawerOnLeftEdge = false;
    bool hasDrawerOnRightEdge = false;
    int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);

        if (child->getVisibility() == GONE) {
            continue;
        }

        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
#if 0
        if (applyInsets) {
            const int cgrav = Gravity::getAbsoluteGravity(lp->gravity, layoutDirection);
            if (child->getFitsSystemWindows()) {
                if (Build::VERSION::SDK_INT >= 21) {
                    WindowInsets wi = (WindowInsets) mLastInsets;
                    if (cgrav == Gravity::LEFT) {
                        wi = wi.replaceSystemWindowInsets(wi.getSystemWindowInsetLeft(),
                                wi.getSystemWindowInsetTop(), 0, wi.getSystemWindowInsetBottom());
                    } else if (cgrav == Gravity::RIGHT) {
                        wi = wi.replaceSystemWindowInsets(0, wi.getSystemWindowInsetTop(),
                                wi.getSystemWindowInsetRight(), wi.getSystemWindowInsetBottom());
                    }
                    child->dispatchApplyWindowInsets(wi);
                }
            } else {
                if (Build::VERSION::SDK_INT >= 21) {
                    WindowInsets wi = (WindowInsets) mLastInsets;
                    if (cgrav == Gravity::LEFT) {
                        wi = wi.replaceSystemWindowInsets(wi.getSystemWindowInsetLeft(),
                                wi.getSystemWindowInsetTop(), 0, wi.getSystemWindowInsetBottom());
                    } else if (cgrav == Gravity::RIGHT) {
                        wi = wi.replaceSystemWindowInsets(0, wi.getSystemWindowInsetTop(),
                                wi.getSystemWindowInsetRight(), wi.getSystemWindowInsetBottom());
                    }
                    lp->leftMargin = wi.getSystemWindowInsetLeft();
                    lp->topMargin = wi.getSystemWindowInsetTop();
                    lp->rightMargin = wi.getSystemWindowInsetRight();
                    lp->bottomMargin = wi.getSystemWindowInsetBottom();
                }
            }
        }
#endif
        if (isContentView(child)) {
            // Content views get measured at exactly the layout's size.
            const int contentWidthSpec = MeasureSpec::makeMeasureSpec(
                    widthSize - lp->leftMargin - lp->rightMargin, MeasureSpec::EXACTLY);
            const int contentHeightSpec = MeasureSpec::makeMeasureSpec(
                    heightSize - lp->topMargin - lp->bottomMargin, MeasureSpec::EXACTLY);
            child->measure(contentWidthSpec, contentHeightSpec);
        } else if (isDrawerView(child)) {
            if (SET_DRAWER_SHADOW_FROM_ELEVATION) {
                if (child->getElevation() != mDrawerElevation) {
                    child->setElevation(mDrawerElevation);
                }
            }
            const int childGravity = getDrawerViewAbsoluteGravity(child) & Gravity::HORIZONTAL_GRAVITY_MASK;
            // Note that the isDrawerView check guarantees that childGravity here is either LEFT or RIGHT
            const bool isLeftEdgeDrawer = (childGravity == Gravity::LEFT);
            if ((isLeftEdgeDrawer && hasDrawerOnLeftEdge)
                    || (!isLeftEdgeDrawer && hasDrawerOnRightEdge)) {
                LOGE("Child drawer has absolute gravity %d but this already has a drawer view along that edge",childGravity);
            }
            if (isLeftEdgeDrawer) {
                hasDrawerOnLeftEdge = true;
            } else {
                hasDrawerOnRightEdge = true;
            }
            const int drawerWidthSpec = getChildMeasureSpec(widthMeasureSpec,
                    mMinDrawerMargin + lp->leftMargin + lp->rightMargin, lp->width);
            const int drawerHeightSpec = getChildMeasureSpec(heightMeasureSpec,
                    lp->topMargin + lp->bottomMargin, lp->height);
            child->measure(drawerWidthSpec, drawerHeightSpec);
        }else{
            LOGE("Child %p at index %f does not have a valid layout_gravity - must be Gravity.LEFT, "
                   "Gravity.RIGHT or Gravity.NO_GRAVITY",child,i);
        }
    }
}

void DrawerLayout::resolveShadowDrawables() {
    if (SET_DRAWER_SHADOW_FROM_ELEVATION) {
        return;
    }
    mShadowLeftResolved = resolveLeftShadow();
    mShadowRightResolved = resolveRightShadow();
}

Drawable* DrawerLayout::resolveLeftShadow() {
    int layoutDirection = getLayoutDirection();
    // Prefer shadows defined with start/end gravity over left and right.
    if (layoutDirection == View::LAYOUT_DIRECTION_LTR) {
        if (mShadowStart) {
            // Correct drawable layout direction, if needed.
            mirror(mShadowStart, layoutDirection);
            return mShadowStart;
        }
    } else {
        if (mShadowEnd) {
            // Correct drawable layout direction, if needed.
            mirror(mShadowEnd, layoutDirection);
            return mShadowEnd;
        }
    }
    return mShadowLeft;
}

Drawable* DrawerLayout::resolveRightShadow() {
    int layoutDirection = getLayoutDirection();
    if (layoutDirection == View::LAYOUT_DIRECTION_LTR) {
        if (mShadowEnd != nullptr) {
            // Correct drawable layout direction, if needed.
            mirror(mShadowEnd, layoutDirection);
            return mShadowEnd;
        }
    } else {
        if (mShadowStart != nullptr) {
            // Correct drawable layout direction, if needed.
            mirror(mShadowStart, layoutDirection);
            return mShadowStart;
        }
    }
    return mShadowRight;
}

/* Change the layout direction of the given drawable.
 * Return true if auto-mirror is supported and drawable's layout direction can be changed.
 * Otherwise, return false.*/
bool DrawerLayout::mirror(Drawable* drawable, int layoutDirection) {
    if (drawable == nullptr || !drawable->isAutoMirrored()) {
        return false;
    }

    drawable->setLayoutDirection(layoutDirection);
    return true;
}

void DrawerLayout::onLayout(bool changed, int l, int t, int width, int height) {
    mInLayout = true;
    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);

        if (child->getVisibility() == GONE) continue;

        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();

        if (isContentView(child)) {
            child->layout(lp->leftMargin, lp->topMargin, child->getMeasuredWidth(),child->getMeasuredHeight());
        } else { // Drawer, if it wasn't onMeasure would have thrown an exception.
            const int childWidth = child->getMeasuredWidth();
            const int childHeight = child->getMeasuredHeight();
            int childLeft;

            float newOffset;
            if (checkDrawerViewAbsoluteGravity(child, Gravity::LEFT)) {
                childLeft = -childWidth + (int) (childWidth * lp->onScreen);
                newOffset = (float) (childWidth + childLeft) / childWidth;
            } else { // Right; onMeasure checked for us.
                childLeft = width - (int) (childWidth * lp->onScreen);
                newOffset = (float) (width - childLeft) / childWidth;
            }

            bool changeOffset = newOffset != lp->onScreen;

            int vgrav = lp->gravity & Gravity::VERTICAL_GRAVITY_MASK;

            LOGD("child %p:%d size=%d,%d left=%d",child,child->getId(),childWidth,childHeight,childLeft);
            switch (vgrav) {
            default:
            case Gravity::TOP:
                child->layout(childLeft, lp->topMargin, childWidth,childHeight);
                break;

            case Gravity::BOTTOM:
                child->layout(childLeft,
                        height - lp->bottomMargin - child->getMeasuredHeight(),childWidth,height);
                break;

            case Gravity::CENTER_VERTICAL: {
                int childTop = (height - childHeight) / 2;
                // Offset for margins. If things don't fit right because of
                // bad measurement before, oh well.
                if (childTop < lp->topMargin) {
                    childTop = lp->topMargin;
                } else if (childTop + childHeight > height - lp->bottomMargin) {
                    childTop = height - lp->bottomMargin - childHeight;
                }
                child->layout(childLeft, childTop, childWidth,childHeight);
                break;
                }
            }

            if (changeOffset) setDrawerViewOffset(child, newOffset);

            int newVisibility = lp->onScreen > 0 ? VISIBLE : INVISIBLE;
            if (child->getVisibility() != newVisibility) {
                child->setVisibility(newVisibility);
            }
        }
    }
    mInLayout = false;
    mFirstLayout = false;
}

void DrawerLayout::requestLayout() {
    if (!mInLayout) {
        ViewGroup::requestLayout();
    }
}


void DrawerLayout::computeScroll() {
    const int childCount = getChildCount();
    float scrimOpacity = 0;
    for (int i = 0; i < childCount; i++) {
        float onscreen = ((LayoutParams*) getChildAt(i)->getLayoutParams())->onScreen;
        scrimOpacity = std::max(scrimOpacity, onscreen);
    }
    mScrimOpacity = scrimOpacity;

    bool leftDraggerSettling = mLeftDragger->continueSettling(true);
    bool rightDraggerSettling = mRightDragger->continueSettling(true);
    if (leftDraggerSettling || rightDraggerSettling) {
        postInvalidateOnAnimation();
    }
}

bool DrawerLayout::hasOpaqueBackground(View* v) {
    Drawable* bg = v->getBackground();
    if (bg != nullptr) {
        return bg->getOpacity() == PixelFormat::OPAQUE;
    }
    return false;
}


void DrawerLayout::setStatusBarBackground(Drawable* bg) {
    mStatusBarBackground = bg;
    invalidate();
}



Drawable* DrawerLayout::getStatusBarBackgroundDrawable() {
    return mStatusBarBackground;
}


void DrawerLayout::setStatusBarBackground(const std::string& resId) {
    mStatusBarBackground = getContext()->getDrawable(resId);
    invalidate();
}


void DrawerLayout::setStatusBarBackgroundColor(int color) {
    mStatusBarBackground = new ColorDrawable(color);
    invalidate();
}

void DrawerLayout::onRtlPropertiesChanged(int layoutDirection) {
     resolveShadowDrawables();
}

void DrawerLayout::onDraw(Canvas& c) {
    ViewGroup::onDraw(c);
#if 0
    if (mDrawStatusBarBackground && mStatusBarBackground != nullptr) {
        int inset = 0;
        if (Build::VERSION::SDK_INT >= 21) {
            inset = mLastInsets != nullptr
                    ? ((WindowInsets) mLastInsets).getSystemWindowInsetTop() : 0;
        } else {
            inset = 0;
        }
        if (inset > 0) {
            mStatusBarBackground->setBounds(0, 0, getWidth(), inset);
            mStatusBarBackground->draw(c);
        }
    }
#endif
}

bool DrawerLayout::drawChild(Canvas& canvas, View* child, int64_t drawingTime) {
    int height = getHeight();
    bool drawingContent = isContentView(child);
    int clipLeft = 0, clipRight = getWidth();

    canvas.save();
    if (drawingContent) {
        const int childCount = getChildCount();
        for (int i = 0; i < childCount; i++) {
            View* v = getChildAt(i);
            if (v == child || v->getVisibility() != VISIBLE
                    || !hasOpaqueBackground(v) || !isDrawerView(v)
                    || v->getHeight() < height) {
                continue;
            }

            if (checkDrawerViewAbsoluteGravity(v, Gravity::LEFT)) {
                int vright = v->getRight();
                if (vright > clipLeft) clipLeft = vright;
            } else {
                int vleft = v->getLeft();
                if (vleft < clipRight) clipRight = vleft;
            }
        }
        canvas.rectangle(clipLeft, 0, clipRight-clipLeft, getHeight());
        canvas.clip();
    }
    bool result = ViewGroup::drawChild(canvas, child, drawingTime);
    canvas.restore();

    if ((mScrimOpacity > 0) && drawingContent) {
        const int baseAlpha = (mScrimColor & 0xff000000) >> 24;
        const int imag = (int) (baseAlpha * mScrimOpacity);
        const int color = imag << 24 | (mScrimColor & 0xffffff);
        canvas.set_color(color);

        canvas.rectangle(clipLeft, 0, clipRight, getHeight());
        canvas.fill();
    } else if (mShadowLeftResolved && checkDrawerViewAbsoluteGravity(child, Gravity::LEFT)) {
        const int shadowWidth = mShadowLeftResolved->getIntrinsicWidth();
        const int childRight  = child->getRight();
        const int drawerPeekDistance = mLeftDragger->getEdgeSize();
        const float alpha = std::max(.0f, std::min((float) childRight / drawerPeekDistance, 1.f));
        mShadowLeftResolved->setBounds(childRight, child->getTop(),shadowWidth, child->getHeight());
        mShadowLeftResolved->setAlpha((int) (0xff * alpha));
        mShadowLeftResolved->draw(canvas);
    } else if (mShadowRightResolved && checkDrawerViewAbsoluteGravity(child, Gravity::RIGHT)) {
        const int shadowWidth = mShadowRightResolved->getIntrinsicWidth();
        const int childLeft = child->getLeft();
        const int showing   = getWidth() - childLeft;
        const int drawerPeekDistance = mRightDragger->getEdgeSize();
        const float alpha =  std::max(.0f, std::min((float) showing / drawerPeekDistance, 1.f));
        mShadowRightResolved->setBounds(childLeft - shadowWidth, child->getTop(),
                shadowWidth, child->getWidth());
        mShadowRightResolved->setAlpha((int) (0xff * alpha));
        mShadowRightResolved->draw(canvas);
    }
    return result;
}

bool DrawerLayout::isContentView(View* child)const{
    return ((LayoutParams*) child->getLayoutParams())->gravity == Gravity::NO_GRAVITY;
}

bool DrawerLayout::isDrawerView(View* child)const{
    const int gravity = ((LayoutParams*) child->getLayoutParams())->gravity;
    const int absGravity = Gravity::getAbsoluteGravity(gravity,child->getLayoutDirection());
    if ((absGravity & Gravity::LEFT) != 0) {
        // This child is a left-edge drawer
        return true;
    }
    if ((absGravity & Gravity::RIGHT) != 0) {
        // This child is a right-edge drawer
        return true;
    }
    return false;
}

bool DrawerLayout::onInterceptTouchEvent(MotionEvent& ev) {
    const int action = ev.getActionMasked();
   
    // "|" used deliberately here; both methods should be invoked.
    const bool interceptForDrag = mLeftDragger->shouldInterceptTouchEvent(ev)
            | mRightDragger->shouldInterceptTouchEvent(ev);
   
    bool interceptForTap = false;
   
    switch (action) {
    case MotionEvent::ACTION_DOWN:
        mInitialMotionX = ev.getX();
        mInitialMotionY = ev.getY();
        if (mScrimOpacity > 0) {
            View* child = mLeftDragger->findTopChildUnder((int) mInitialMotionX, (int) mInitialMotionY);
            if (child  && isContentView(child)) {
                interceptForTap = true;
            }
        }
        mDisallowInterceptRequested = false;
        mChildrenCanceledTouch = false;
        break;
   
    case MotionEvent::ACTION_MOVE:
        // If we cross the touch slop, don't perform the delayed peek for an edge touch.
        if (mLeftDragger->checkTouchSlop(ViewDragHelper::DIRECTION_ALL)) {
            mLeftCallback->removeCallbacks();
            mRightCallback->removeCallbacks();
        }
        break;
    case MotionEvent::ACTION_CANCEL:
    case MotionEvent::ACTION_UP:
        closeDrawers(true);
        mDisallowInterceptRequested = false;
        mChildrenCanceledTouch = false;
    }  
    return interceptForDrag || interceptForTap || hasPeekingDrawer() || mChildrenCanceledTouch;
}
    
bool DrawerLayout::dispatchGenericMotionEvent(MotionEvent& event) {
    
    // If this is not a pointer event, or if this is an hover exit, or we are not displaying
    // that the content view can't be interacted with, then don't override and do anything
    // special.
    if ((event.getSource() & InputDevice::SOURCE_CLASS_POINTER) == 0
            || event.getAction() == MotionEvent::ACTION_HOVER_EXIT
            || mScrimOpacity <= 0) {
        return ViewGroup::dispatchGenericMotionEvent(event);
    }
    const int childrenCount = getChildCount();
    if (childrenCount != 0) {
        const float x = event.getX();
        const float y = event.getY();
   
        // Walk through children from top to bottom.
        for (int i = childrenCount - 1; i >= 0; i--) {
            View* child = getChildAt(i);
   
            // If the event is out of bounds or the child is the content view, don't dispatch
            // to it.
            if (!isInBoundsOfChild(x, y, child) || isContentView(child)) {
                continue;
            }
   
            // If a child handles it, return true.
            if (dispatchTransformedGenericPointerEvent(event, child)) {
                return true;
            }
        }
    }   
    return false;
}
    
    
bool DrawerLayout::onTouchEvent(MotionEvent& ev) {
    mLeftDragger->processTouchEvent(ev);
    mRightDragger->processTouchEvent(ev);
   
    const int action = ev.getAction();
    bool wantTouchEvents = true;
   
    switch (action & MotionEvent::ACTION_MASK) {
    case MotionEvent::ACTION_DOWN:
        mInitialMotionX = ev.getX();
        mInitialMotionY = ev.getY();
        mDisallowInterceptRequested = false;
        mChildrenCanceledTouch = false;
        break;
   
    case MotionEvent::ACTION_UP: {
        float x = ev.getX();
        float y = ev.getY();
        bool peekingOnly = true;
        View* touchedView = mLeftDragger->findTopChildUnder((int) x, (int) y);
        if (touchedView && isContentView(touchedView)) {
            const float dx = x - mInitialMotionX;
            const float dy = y - mInitialMotionY;
            const int slop = mLeftDragger->getTouchSlop();
            if (dx * dx + dy * dy < slop * slop) {
                // Taps close a dimmed open drawer but only if it isn't locked open.
                View* openDrawer = findOpenDrawer();
                if (openDrawer) {
                    peekingOnly = getDrawerLockMode(openDrawer) == LOCK_MODE_LOCKED_OPEN;
                }
            }
        }
        closeDrawers(peekingOnly);
        mDisallowInterceptRequested = false;
        break;
        }
   
    case MotionEvent::ACTION_CANCEL:
        closeDrawers(true);
        mDisallowInterceptRequested = false;
        mChildrenCanceledTouch = false;
        break;
    }
   
    return wantTouchEvents;
}
    
    
void DrawerLayout::requestDisallowInterceptTouchEvent(bool disallowIntercept) {
    if (CHILDREN_DISALLOW_INTERCEPT
            || (!mLeftDragger->isEdgeTouched(ViewDragHelper::EDGE_LEFT)
                    && !mRightDragger->isEdgeTouched(ViewDragHelper::EDGE_RIGHT))) {
        // If we have an edge touch we want to skip this and track it for later instead.
        ViewGroup::requestDisallowInterceptTouchEvent(disallowIntercept);
    }
    mDisallowInterceptRequested = disallowIntercept;
    if (disallowIntercept) {
        closeDrawers(true);
    }
}
    
//Close all currently open drawer views by animating them out of view.
    
void DrawerLayout::closeDrawers() {
    closeDrawers(false);
}
    
void DrawerLayout::closeDrawers(bool peekingOnly) {
    bool needsInvalidate = false;
    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
   
        if (!isDrawerView(child) || (peekingOnly && !lp->isPeeking)) {
            continue;
        }
   
        const int childWidth = child->getWidth();
   
        if (checkDrawerViewAbsoluteGravity(child, Gravity::LEFT)) {
            needsInvalidate |= mLeftDragger->smoothSlideViewTo(child,-childWidth, child->getTop());
        } else {
            needsInvalidate |= mRightDragger->smoothSlideViewTo(child,getWidth(), child->getTop());
        }
   
        lp->isPeeking = false;
    }
   
    mLeftCallback->removeCallbacks();
    mRightCallback->removeCallbacks();
   
    if (needsInvalidate) {
        invalidate();
    }
   
}
   
    
void DrawerLayout::openDrawer(View* drawerView) {
    openDrawer(drawerView, true);
}
       
void DrawerLayout::openDrawer(View* drawerView, bool animate) {
    LOGE_IF(!isDrawerView(drawerView),"View %p:%d is not a sliding drawer",drawerView,drawerView->getId());
   
    LayoutParams* lp = (LayoutParams*) drawerView->getLayoutParams();
    if (mFirstLayout) {
        lp->onScreen = 1.f;
        lp->openState = LayoutParams::FLAG_IS_OPENED;
   
        updateChildrenImportantForAccessibility(drawerView, true);
    } else if (animate) {
        lp->openState |= LayoutParams::FLAG_IS_OPENING;
   
        if (checkDrawerViewAbsoluteGravity(drawerView, Gravity::LEFT)) {
            mLeftDragger->smoothSlideViewTo(drawerView, 0, drawerView->getTop());
        } else {
            mRightDragger->smoothSlideViewTo(drawerView, getWidth() - drawerView->getWidth(),
                    drawerView->getTop());
        }
    } else {
        moveDrawerToOffset(drawerView, 1.f);
        updateDrawerState(lp->gravity, STATE_IDLE, drawerView);
        drawerView->setVisibility(VISIBLE);
    }
    invalidate();
}
    
    
void DrawerLayout::openDrawer(int gravity) {
    openDrawer(gravity, true);
}
    
const std::string DrawerLayout::gravityToString(int gravity) {
    if ((gravity & Gravity::LEFT) == Gravity::LEFT) {
        return "LEFT";
    }
    if ((gravity & Gravity::RIGHT) == Gravity::RIGHT) {
        return "RIGHT";
    }
    return std::to_string(gravity);
} 

void DrawerLayout::openDrawer(int gravity, bool animate) {
    View* drawerView = findDrawerWithGravity(gravity);
    LOGE_IF(drawerView == nullptr,"No drawer view found with gravity %s",gravityToString(gravity).c_str());
    openDrawer(drawerView, animate);
}
    
void DrawerLayout::closeDrawer(View* drawerView) {
    closeDrawer(drawerView, true);
}
    
void DrawerLayout::closeDrawer(View* drawerView, bool animate) {
    LOGE_IF(!isDrawerView(drawerView),"View %p:%d is not a sliding drawer",drawerView,drawerView->getId());
    
    LayoutParams* lp = (LayoutParams*) drawerView->getLayoutParams();
    if (mFirstLayout) {
        lp->onScreen = 0.f;
        lp->openState = 0;
    } else if (animate) {
        lp->openState |= LayoutParams::FLAG_IS_CLOSING;
   
        if (checkDrawerViewAbsoluteGravity(drawerView, Gravity::LEFT)) {
            mLeftDragger->smoothSlideViewTo(drawerView, -drawerView->getWidth(),
                    drawerView->getTop());
        } else {
            mRightDragger->smoothSlideViewTo(drawerView, getWidth(), drawerView->getTop());
        }
    } else {
        moveDrawerToOffset(drawerView, 0.f);
        updateDrawerState(lp->gravity, STATE_IDLE, drawerView);
        drawerView->setVisibility(INVISIBLE);
    }
    invalidate();
}
    
    
void DrawerLayout::closeDrawer(int gravity) {
    closeDrawer(gravity, true);
}
    
    
void DrawerLayout::closeDrawer(int gravity, bool animate) {
    View* drawerView = findDrawerWithGravity(gravity);
    LOGE_IF(drawerView==nullptr,"No drawer view found with gravity %s",gravityToString(gravity).c_str());
    closeDrawer(drawerView, animate);
}
     
bool DrawerLayout::isDrawerOpen(View* drawer) {
    LOGE_IF(!isDrawerView(drawer),"View %p:%d is not a drawer",drawer,drawer->getId());
    LayoutParams* drawerLp = (LayoutParams*) drawer->getLayoutParams();
    return (drawerLp->openState & LayoutParams::FLAG_IS_OPENED) == 1;
}

bool DrawerLayout::isDrawerOpen(int drawerGravity) {
    View* drawerView = findDrawerWithGravity(drawerGravity);
    if (drawerView != nullptr) {
        return isDrawerOpen(drawerView);
    }
    return false;
}
     
bool DrawerLayout::isDrawerVisible(View* drawer) {
    LOGE_IF(!isDrawerView(drawer),"View %p:%d is not a drawer",drawer,drawer->getId());
    return ((LayoutParams*) drawer->getLayoutParams())->onScreen > 0;
}


bool DrawerLayout::isDrawerVisible(int drawerGravity) {
    View* drawerView = findDrawerWithGravity(drawerGravity);
    if (drawerView != nullptr) {
        return isDrawerVisible(drawerView);
    }
    return false;
}
    
bool DrawerLayout::hasPeekingDrawer() {
    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        LayoutParams* lp = (LayoutParams*) getChildAt(i)->getLayoutParams();
        if (lp->isPeeking) {
            return true;
        }
    }
    return false;
}

DrawerLayout::LayoutParams* DrawerLayout::generateDefaultLayoutParams()const {
    return new LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::MATCH_PARENT);
}

DrawerLayout::LayoutParams* DrawerLayout::generateLayoutParams(const ViewGroup::LayoutParams* p)const {
    return dynamic_cast<const LayoutParams*>(p)
            ? new LayoutParams((const LayoutParams&)*p)
            : dynamic_cast<const ViewGroup::MarginLayoutParams*>(p)
            ? new LayoutParams((const MarginLayoutParams&)*p)
            : new LayoutParams(*p);
}

bool DrawerLayout::checkLayoutParams(const ViewGroup::LayoutParams* p)const {
    return dynamic_cast<const LayoutParams*>(p) && ViewGroup::checkLayoutParams(p);
}

DrawerLayout::LayoutParams* DrawerLayout::generateLayoutParams(const AttributeSet& attrs)const {
    return new LayoutParams(getContext(), attrs);
}

void DrawerLayout::addFocusables(std::vector<View*>& views, int direction, int focusableMode){
    if (getDescendantFocusability() == FOCUS_BLOCK_DESCENDANTS) {
        return;
    }

    // Only the views in the open drawers are focusables. Add normal child views when
    // no drawers are opened.
    const int childCount = getChildCount();
    bool bIsDrawerOpen = false;
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        if (isDrawerView(child)) {
            if (isDrawerOpen(child)) {
                bIsDrawerOpen = true;
                child->addFocusables(views, direction, focusableMode);
            }
        } else {
            mNonDrawerViews.push_back(child);
        }
    }

    if (!bIsDrawerOpen) {
        const size_t nonDrawerViewsCount = mNonDrawerViews.size();
        for (int i = 0; i < nonDrawerViewsCount; ++i) {
            View* child = mNonDrawerViews.at(i);
            if (child->getVisibility() == View::VISIBLE) {
                child->addFocusables(views, direction, focusableMode);
            }
        }
    }

    mNonDrawerViews.clear();
}

bool DrawerLayout::hasVisibleDrawer() {
    return findVisibleDrawer() != nullptr;
}

View* DrawerLayout::findVisibleDrawer() {
    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        if (isDrawerView(child) && isDrawerVisible(child)) {
            return child;
        }
    }
    return nullptr;
}

void DrawerLayout::cancelChildViewTouch() {
    // Cancel child touches
    if (!mChildrenCanceledTouch) {
        auto now = SystemClock::uptimeMillis();
        MotionEvent* cancelEvent = MotionEvent::obtain(now, now,
                MotionEvent::ACTION_CANCEL, 0.0f, 0.0f, 0);
        const int childCount = getChildCount();
        for (int i = 0; i < childCount; i++) {
            getChildAt(i)->dispatchTouchEvent(*cancelEvent);
        }
        cancelEvent->recycle();
        mChildrenCanceledTouch = true;
    }
}

bool DrawerLayout::onKeyDown(int keyCode, KeyEvent& event) {
    if (keyCode == KeyEvent::KEYCODE_BACK && hasVisibleDrawer()) {
        event.startTracking();
        return true;
    }
    return ViewGroup::onKeyDown(keyCode, event);
}

bool DrawerLayout::onKeyUp(int keyCode, KeyEvent& event) {
    if (keyCode == KeyEvent::KEYCODE_BACK) {
        View* visibleDrawer = findVisibleDrawer();
        if (visibleDrawer && getDrawerLockMode(visibleDrawer) == LOCK_MODE_UNLOCKED) {
            closeDrawers();
        }
        return visibleDrawer != nullptr;
    }
    return ViewGroup::onKeyUp(keyCode, event);
}

void DrawerLayout::addView(View* child, int index, ViewGroup::LayoutParams* params) {
    ViewGroup::addView(child, index, params);

    View* openDrawer = findOpenDrawer();
    if (openDrawer != nullptr || isDrawerView(child)) {
       // A drawer is already open or the new view is a drawer, so the
       // new view should start out hidden.
       child->setImportantForAccessibility(View::IMPORTANT_FOR_ACCESSIBILITY_NO_HIDE_DESCENDANTS);
    } else {
        // Otherwise this is a content view and no drawer is open, so the
        // new view should start out visible.
        child->setImportantForAccessibility(View::IMPORTANT_FOR_ACCESSIBILITY_YES);
    }

    // We only need a delegate here if the framework doesn't understand
    // NO_HIDE_DESCENDANTS importance.
    if (!CAN_HIDE_DESCENDANTS) {
        //child->setAccessibilityDelegate(mChildAccessibilityDelegate);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

DrawerLayout::ViewDragCallback::ViewDragCallback(DrawerLayout*dl,int gravity){
    mDL = dl;
    mAbsGravity = gravity;
    mDragger = nullptr;
    mPeekRunnable=[this](){
        peekDrawer();
    };
}

void DrawerLayout::ViewDragCallback::setDragger(ViewDragHelper* dragger){
    mDragger = dragger;
}

void DrawerLayout::ViewDragCallback::removeCallbacks(){
    mDL->removeCallbacks(mPeekRunnable);
}

bool DrawerLayout::ViewDragCallback::tryCaptureView(View& child, int pointerId){
    return mDL->isDrawerView(&child) && mDL->checkDrawerViewAbsoluteGravity(&child, mAbsGravity)
                && mDL->getDrawerLockMode(&child) == DrawerLayout::LOCK_MODE_UNLOCKED;
}

void DrawerLayout::ViewDragCallback::onViewDragStateChanged(int state){
     mDL->updateDrawerState(mAbsGravity, state, mDragger->getCapturedView());
}

void DrawerLayout::ViewDragCallback::onViewPositionChanged(View& changedView, int left, int top, int dx, int dy){
    float offset;
    int childWidth = changedView.getWidth();

    // This reverses the positioning shown in onLayout.
    if (mDL->checkDrawerViewAbsoluteGravity(&changedView, Gravity::LEFT)) {
        offset = (float) (childWidth + left) / childWidth;
    } else {
        const int width = mDL->getWidth();
        offset = (float) (width - left) / childWidth;
    }
    mDL->setDrawerViewOffset(&changedView, offset);
    changedView.setVisibility(offset == 0 ? INVISIBLE : VISIBLE);
    mDL->invalidate();
}

void DrawerLayout::ViewDragCallback::onViewCaptured(View& capturedChild, int activePointerId){
    LayoutParams* lp = (LayoutParams*) capturedChild.getLayoutParams();
    lp->isPeeking = false;

    closeOtherDrawer();
}

void DrawerLayout::ViewDragCallback::closeOtherDrawer() {
     const int otherGrav = mAbsGravity == Gravity::LEFT ? Gravity::RIGHT : Gravity::LEFT;
     View* toClose = mDL->findDrawerWithGravity(otherGrav);
     if (toClose) {
         mDL->closeDrawer(toClose);
     }
}

void DrawerLayout::ViewDragCallback::onViewReleased(View& releasedChild, float xvel, float yvel){
    // Offset is how open the drawer is, therefore left/right values
    // are reversed from one another.
    const float offset = mDL->getDrawerViewOffset(&releasedChild);
    const int childWidth = releasedChild.getWidth();

    int left;
    if (mDL->checkDrawerViewAbsoluteGravity(&releasedChild, Gravity::LEFT)) {
        left = xvel > 0 || (xvel == 0 && offset > 0.5f) ? 0 : -childWidth;
    } else {
        const int width = mDL->getWidth();
        left = xvel < 0 || (xvel == 0 && offset > 0.5f) ? width - childWidth : width;
    }

    mDragger->settleCapturedViewAt(left, releasedChild.getTop());
    mDL->invalidate();
}

void DrawerLayout::ViewDragCallback::onEdgeTouched(int edgeFlags, int pointerId){
    mDL->postDelayed(mPeekRunnable, PEEK_DELAY);
}

void DrawerLayout::ViewDragCallback::peekDrawer(){
    View* toCapture;
    int childLeft;
    const int peekDistance = mDragger->getEdgeSize();
    const bool leftEdge = mAbsGravity == Gravity::LEFT;
    if (leftEdge) {
        toCapture = mDL->findDrawerWithGravity(Gravity::LEFT);
        childLeft = 0;//(toCapture ? -toCapture->getWidth() : 0) + peekDistance;
    } else {
        toCapture = mDL->findDrawerWithGravity(Gravity::RIGHT);
        //childLeft = mDL->getWidth()- peekDistance;
        childLeft = mDL->getWidth()- (toCapture?toCapture->getWidth():peekDistance);
    }
    LOGV_IF(toCapture,"isleft?=%d toCapture=%p (%d) %d->%d",leftEdge,toCapture,toCapture->getWidth(),toCapture->getLeft(),childLeft);
    // Only peek if it would mean making the drawer more visible and the drawer isn't locked
    if (toCapture && ((leftEdge && toCapture->getLeft() < childLeft)
            || (!leftEdge && toCapture->getLeft() > childLeft))
            && mDL->getDrawerLockMode(toCapture) == DrawerLayout::LOCK_MODE_UNLOCKED) {
        LayoutParams* lp = (LayoutParams*) toCapture->getLayoutParams();
        mDragger->smoothSlideViewTo(toCapture, childLeft, toCapture->getTop());
        lp->isPeeking = true;
        mDL->invalidate();

        closeOtherDrawer();

        mDL->cancelChildViewTouch();
    }
}

bool DrawerLayout::ViewDragCallback::onEdgeLock(int edgeFlags){
    if (ALLOW_EDGE_LOCK) {
        View* drawer = mDL->findDrawerWithGravity(mAbsGravity);
        if (drawer  && !mDL->isDrawerOpen(drawer)) {
            mDL->closeDrawer(drawer);
        }
        return true;
    }
    return false;
}

void DrawerLayout::ViewDragCallback::onEdgeDragStarted(int edgeFlags, int pointerId){
    View* toCapture;
    if ((edgeFlags & ViewDragHelper::EDGE_LEFT) == ViewDragHelper::EDGE_LEFT) {
        toCapture = mDL->findDrawerWithGravity(Gravity::LEFT);
    } else {
        toCapture = mDL->findDrawerWithGravity(Gravity::RIGHT);
    }

    if (toCapture  && mDL->getDrawerLockMode(toCapture) == DrawerLayout::LOCK_MODE_UNLOCKED) {
        mDragger->captureChildView(toCapture, pointerId);
    }
}

int DrawerLayout::ViewDragCallback::getViewHorizontalDragRange(View& child){
    return mDL->isDrawerView(&child) ? child.getWidth() : 0;
}

int DrawerLayout::ViewDragCallback::clampViewPositionHorizontal(View& child, int left, int dx){
    if (mDL->checkDrawerViewAbsoluteGravity(&child, Gravity::LEFT)) {
        return std::max(-child.getWidth(), std::min(left, 0));
    } else {
        const int width = mDL->getWidth();
        return std::max(width - child.getWidth(), std::min(left, width));
    }
}

int DrawerLayout::ViewDragCallback::clampViewPositionVertical(View& child, int top, int dy){
    return child.getTop();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

DrawerLayout::LayoutParams::LayoutParams(Context* c,const AttributeSet& attrs)
  :ViewGroup::MarginLayoutParams(c, attrs){
    gravity = attrs.getInt(0, Gravity::NO_GRAVITY);
    onScreen =.0f;
}

DrawerLayout::LayoutParams::LayoutParams(int width, int height)
:ViewGroup::MarginLayoutParams(width,height){
    gravity = Gravity::NO_GRAVITY;
}

DrawerLayout::LayoutParams::LayoutParams(int width, int height, int gravity)
  :LayoutParams(width, height){
    gravity = gravity;
}

DrawerLayout::LayoutParams::LayoutParams(const LayoutParams& source)
:ViewGroup::MarginLayoutParams(source){
    gravity = source.gravity;
}

DrawerLayout::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& source)
  :ViewGroup::MarginLayoutParams(source){
    gravity = Gravity::NO_GRAVITY;
}

DrawerLayout::LayoutParams::LayoutParams(const ViewGroup::MarginLayoutParams& source)
  :ViewGroup::MarginLayoutParams(source){
    gravity = Gravity::NO_GRAVITY;
}
}
