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
#include <widget/popupwindow.h>
#include <cdlog.h>
namespace cdroid{

PopupWindow::PopupWindow(Context* context,const AttributeSet& attrs)
    :PopupWindow(context,attrs,"android:attr/popupWindowStyle"){
}

PopupWindow::PopupWindow(Context* context,const AttributeSet& attrs, const std::string& defStyleAttr)
    :PopupWindow(context,attrs,defStyleAttr,""){
}

PopupWindow::PopupWindow(Context* context,const AttributeSet& attrs, const std::string& defStyleAttr, const std::string& defStyleRes){
    LOGD("create PopupWindow %p",this);
    init();
    mContext = context;
    AttributeSet attpop= context->obtainStyledAttributes(defStyleAttr);
    attpop.Override(attrs);
    Drawable* bg = attpop.getDrawable("popupBackground");
    mElevation = attpop.getFloat/*Dimension*/("popupElevation", 0);
    mOverlapAnchor = attpop.getBoolean("overlapAnchor", false);
#if 0 
    // Preserve default behavior from Gingerbread. If the animation is
    // undefined or explicitly specifies the Gingerbread animation style,
    // use a sentinel value.
    if (a.hasValueOrEmpty("popupAnimationStyle")) {
        int animStyle = a.getResourceId(R.styleable.PopupWindow_popupAnimationStyle, 0);
        if (animStyle == R.style.Animation_PopupWindow) {
            mAnimationStyle = ANIMATION_STYLE_DEFAULT;
        } else {
            mAnimationStyle = animStyle;
        }
    } else {
        mAnimationStyle = ANIMATION_STYLE_DEFAULT;
    }

    Transition enterTransition = getTransition(a.getResourceId(
            R.styleable.PopupWindow_popupEnterTransition, 0));
    Transition exitTransition;
    if (a.hasValueOrEmpty(R.styleable.PopupWindow_popupExitTransition)) {
        exitTransition = getTransition(a.getResourceId(
                R.styleable.PopupWindow_popupExitTransition, 0));
    } else {
        exitTransition = enterTransition == null ? null : enterTransition.clone();
    }

    setEnterTransition(enterTransition);
    setExitTransition(exitTransition);
#endif
    setBackgroundDrawable(bg);
}

PopupWindow::PopupWindow(View* contentView, int width, int height, bool focusable) {
    init();
    LOGD("contentView=%p",contentView);
    if (contentView) {
        mContext = contentView->getContext();
        //mWindowManager = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
    }

    setContentView(contentView);
    setWidth(width);
    setHeight(height);
    setFocusable(focusable);
}

PopupWindow::PopupWindow(int width, int height):PopupWindow(nullptr,width,height){
}

PopupWindow::~PopupWindow(){
    LOGD("destroy PopupWindow %p",this);
    delete mBackground;
    delete mAboveAnchorBackgroundDrawable;
    delete mBelowAnchorBackgroundDrawable;
}

void PopupWindow::init(){
    mIsShowing = false;
    mIsDropdown= false;
    mFocusable = true;
    mTouchable = true;
    mAboveAnchor = false;
    mClipToScreen = false;
    mOutsideTouchable = false;
    mClippingEnabled  = true;
    mSplitTouchEnabled= -1;
    mGravity = Gravity::NO_GRAVITY;
    mInputMethodMode = INPUT_METHOD_FROM_FOCUSABLE;
    mIsTransitioningToDismiss = false;
    mAllowScrollingAnchorParent= true;
    mLayoutInsetDecor = false;
    mAttachedInDecor  = true;
    mAttachedInDecorSet= false;
    mEpicenterBounds.set(0,0,0,0);

    mWidth = LayoutParams::WRAP_CONTENT;
    mHeight = LayoutParams::WRAP_CONTENT;
    mWidthMode = mHeightMode =0;
    mParentRootView = nullptr;
    mAnchor = nullptr;
    mAnchorRoot = nullptr;
    mBackground = nullptr;
    mBackgroundView = nullptr;
    mAboveAnchorBackgroundDrawable = nullptr;
    mBelowAnchorBackgroundDrawable = nullptr;

    mOnScrollChangedListener= [this](){alignToAnchor();};
    mOnLayoutChangeListener = [this](View&v,int,int,int,int,int,int,int,int){
        alignToAnchor();
    };
    mOnAnchorDetachedListener.onViewAttachedToWindow=[this](View&){ alignToAnchor();};
    mOnAnchorDetachedListener.onViewDetachedFromWindow=[](View&){};
    mOnAnchorRootDetachedListener.onViewAttachedToWindow=[](View&){/*nothing*/};
    mOnAnchorRootDetachedListener.onViewDetachedFromWindow=[this](View&v){
        mIsAnchorRootAttached = false;
    };
}

void PopupWindow::setEnterTransition(Transition* enterTransition) {
    mEnterTransition = enterTransition;
}

Transition* PopupWindow::getEnterTransition()const{
    return mEnterTransition;
}

void PopupWindow::setExitTransition(Transition* exitTransition) {
    mExitTransition = exitTransition;
}

Transition* PopupWindow::getExitTransition()const{
    return mExitTransition;
}

void PopupWindow::setEpicenterBounds(const Rect& bounds) {
    mEpicenterBounds = bounds;
}

Drawable* PopupWindow::getBackground() {
    return mBackground;
}

void PopupWindow::setBackgroundDrawable(Drawable* background) {
    mBackground = background;

    if (dynamic_cast<StateListDrawable*>(mBackground)) {
        StateListDrawable* stateList = (StateListDrawable*) mBackground;

        // Find the above-anchor view - this one's easy, it should be labeled as such.
        int aboveAnchorStateIndex = -1;//stateList->getStateDrawableIndex(ABOVE_ANCHOR_STATE_SET);

        // Now, for the below-anchor view, look for any other drawable specified in the
        // StateListDrawable which is not for the above-anchor state and use that.
        const int count = stateList->getStateCount();
        int belowAnchorStateIndex = -1;
        for (int i = 0; i < count; i++) {
            if (i != aboveAnchorStateIndex) {
                belowAnchorStateIndex = i;
                break;
            }
        }

        // Store the drawables we found, if we found them. Otherwise, set them both
        // to null so that we'll just use refreshDrawableState.
        if (aboveAnchorStateIndex != -1 && belowAnchorStateIndex != -1) {
            mAboveAnchorBackgroundDrawable = stateList->getStateDrawable(aboveAnchorStateIndex);
            mBelowAnchorBackgroundDrawable = stateList->getStateDrawable(belowAnchorStateIndex);
        } else {
            mBelowAnchorBackgroundDrawable = nullptr;
            mAboveAnchorBackgroundDrawable = nullptr;
        }
    }
}

float PopupWindow::getElevation() const{
    return mElevation;
}

void PopupWindow::setElevation(float elevation) {
    mElevation = elevation;
}


View* PopupWindow::getContentView() {
    return mContentView;
}

void PopupWindow::setContentView(View* contentView) {
    if (isShowing()) {
        return;
    }

    mContentView = contentView;

    if ((mContext == nullptr) && (mContentView != nullptr)) {
        mContext = mContentView->getContext();
    }

    /*if (mWindowManager == null && mContentView != nullptr) {
        mWindowManager = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
    }*/

    // Setting the default for attachedInDecor based on SDK version here
    // instead of in the constructor since we might not have the context
    // object in the constructor. We only want to set default here if the
    // app hasn't already set the attachedInDecor.
    if (mContext != nullptr && !mAttachedInDecorSet) {
        // Attach popup window in decor frame of parent window by default for
        // {@link Build.VERSION_CODES.LOLLIPOP_MR1} or greater. Keep current
        // behavior of not attaching to decor frame for older SDKs.
        setAttachedInDecor(true);//mContext.getApplicationInfo().targetSdkVersion >= Build::VERSION_CODES::LOLLIPOP_MR1);
    }
}

void PopupWindow::setTouchInterceptor(const View::OnTouchListener& l) {
    mTouchInterceptor = l;
}

bool PopupWindow::isFocusable() const{
    return mFocusable;
}

void PopupWindow::setFocusable(bool focusable) {
    mFocusable = focusable;
}

int PopupWindow::getInputMethodMode()const{
    return mInputMethodMode;
}

void PopupWindow::setInputMethodMode(int mode) {
    mInputMethodMode = mode;
}

void PopupWindow::setSoftInputMode(int mode){
    mSoftInputMode = mode;
}

int PopupWindow::getSoftInputMode()const{
    return mSoftInputMode;
}

bool PopupWindow::isTouchable()const{
    return mTouchable;
}

void PopupWindow::setTouchable(bool touchable){
    mTouchable = touchable;
}

bool PopupWindow::isOutsideTouchable()const{
    return mOutsideTouchable;
}

void PopupWindow::setOutsideTouchable(bool touchable){
    mOutsideTouchable = touchable;
}

bool PopupWindow::isClippingEnabled()const{
    return mClippingEnabled;
}

void PopupWindow::setClippingEnabled(bool enabled){
    mClippingEnabled = enabled;
}

bool PopupWindow::isClippedToScreen()const{
    return mClipToScreen;
}

void PopupWindow::setIsClippedToScreen(bool enabled){
    mClipToScreen = enabled;
}

bool PopupWindow::isSplitTouchEnabled()const{
    return mSplitTouchEnabled == 1;
}

void PopupWindow::setSplitTouchEnabled(bool enabled){
    mSplitTouchEnabled = enabled ? 1 : 0;
}


bool PopupWindow::isLayoutInScreenEnabled()const{
    return mLayoutInScreen;
}

void PopupWindow::setLayoutInScreenEnabled(bool enabled){
    mLayoutInScreen = enabled;
}

bool PopupWindow::isLaidOutInScreen()const{
    return mLayoutInScreen;
}

void PopupWindow::setIsLaidOutInScreen(bool enabled){
    mLayoutInScreen = enabled;
}

bool PopupWindow::isAttachedInDecor()const{
    return mAttachedInDecor;
}

void PopupWindow::setAttachedInDecor(bool enabled){
    mAttachedInDecor = enabled;
    mAttachedInDecorSet = true;
}

void PopupWindow::setLayoutInsetDecor(bool enabled) {
    mLayoutInsetDecor = enabled;
}

bool PopupWindow::isLayoutInsetDecor()const{
    return mLayoutInsetDecor;
}

void PopupWindow::setWindowLayoutType(int layoutType){
    mWindowLayoutType = layoutType;
}

int PopupWindow::getWindowLayoutType()const{
    return mWindowLayoutType;
}

bool PopupWindow::isTouchModal()const{
    return !mNotTouchModal;
}

void PopupWindow::setTouchModal(bool touchModal){
    mNotTouchModal =!touchModal;
}

void PopupWindow::setOverlapAnchor(bool overlapAnchor) {
    mOverlapAnchor = overlapAnchor;
}

bool PopupWindow::getOverlapAnchor()const{
    return mOverlapAnchor;
}

void PopupWindow::setShowing(bool isShowing) {
     mIsShowing = isShowing;
}

bool PopupWindow::isShowing()const{
    return mIsShowing;
}

void PopupWindow::setDropDown(bool isDropDown) {
     mIsDropdown = isDropDown;
}

void PopupWindow::setHeight(int height){
    mHeight = height;
}

int PopupWindow::getHeight()const{
    return mHeight;
}

void PopupWindow::setWidth(int width){
    mWidth = width;
}

int PopupWindow::getWidth()const{
    return mHeight;
}

void PopupWindow::showAtLocation(View* parent, int gravity, int x, int y){
    if (isShowing() || (mContentView == nullptr)) {
        return;
    }
    //TransitionManager.endTransitions(mDecorView);

    detachFromAnchor();

    mIsShowing = true;
    mIsDropdown = false;
    mGravity = gravity;

    WindowManager::LayoutParams* p=createPopupLayoutParams(0);
    preparePopup(p);

    p->x = x;
    p->y = y;
    invokePopup(p);
}

void PopupWindow::showAsDropDown(View* anchor){
    showAsDropDown(anchor,0,0);
}

void PopupWindow::showAsDropDown(View* anchor, int xoff, int yoff){
    showAsDropDown(anchor,xoff,yoff,DEFAULT_ANCHORED_GRAVITY);
}

void PopupWindow::showAsDropDown(View* anchor, int xoff, int yoff,int gravity){
    if (isShowing() || !hasContentView()) {
        return;
    }

    //TransitionManager::endTransitions(mDecorView);

    attachToAnchor(anchor, xoff, yoff, gravity);

    mIsShowing = true;
    mIsDropdown = true;

    WindowManager::LayoutParams* p = createPopupLayoutParams(0);//anchor.getApplicationWindowToken());
    p->x = xoff;
    p->y = yoff;
    preparePopup(p);

    const bool aboveAnchor = findDropDownPosition(anchor,p, xoff, yoff,
           p->width, p->height, gravity, mAllowScrollingAnchorParent);
    updateAboveAnchor(aboveAnchor);
    //p->accessibilityIdOfAnchor = (anchor) ? anchor->getAccessibilityViewId() : -1;
    invokePopup(p);
}

void PopupWindow::preparePopup(WindowManager::LayoutParams*p){
    //if (mDecorView)  mDecorView->cancelTransitions();

    // When a background is available, we embed the content view within
    // another view that owns the background drawable.
    if (mBackground) {
        Drawable* bg = mBackground->getConstantState()->newDrawable();
        mBackgroundView = createBackgroundView(mContentView);
        mBackgroundView->setBackground(bg);//mBackground);
    } else {
        mBackgroundView = mContentView;
    }

    mDecorView = createDecorView(mBackgroundView);
    LOGD("createDecorView %p",mDecorView);
    mDecorView->setIsRootNamespace(true);

    // The background owner should be elevated so that it casts a shadow.
    mBackgroundView->setElevation(mElevation);

    // We may wrap that in another view, so we'll need to manually specify
    // the surface insets.
    //p->setSurfaceInsets(mBackgroundView, true /*manual*/, true /*preservePrevious*/);

    mPopupViewInitialLayoutDirectionInherited =
            (mContentView->getRawLayoutDirection() == View::LAYOUT_DIRECTION_INHERIT);
}

PopupWindow::PopupBackgroundView* PopupWindow::createBackgroundView(View* contentView) {
    ViewGroup::LayoutParams* layoutParams = mContentView->getLayoutParams();
    int height;
    if (layoutParams && (layoutParams->height == LayoutParams::WRAP_CONTENT)) {
        height = LayoutParams::WRAP_CONTENT;
    } else {
        height = LayoutParams::MATCH_PARENT;
    }

    PopupBackgroundView* backgroundView = new PopupBackgroundView(mContext);
    PopupBackgroundView::LayoutParams* listParams = new PopupBackgroundView::LayoutParams(
                LayoutParams::MATCH_PARENT, height);
    backgroundView->addView(contentView, listParams);

    return backgroundView;
}

PopupWindow::PopupDecorView* PopupWindow::createDecorView(View* contentView){
    ViewGroup::LayoutParams* layoutParams = mContentView->getLayoutParams();
    int height;
    if (layoutParams  && (layoutParams->height == LayoutParams::WRAP_CONTENT)) {
        height = LayoutParams::WRAP_CONTENT;
    } else {
        height = LayoutParams::MATCH_PARENT;
    }

    PopupDecorView* decorView = new PopupDecorView(mWidth,mHeight);
    decorView->addView(contentView, LayoutParams::MATCH_PARENT, height);
    //decorView->setClipChildren(false);
    //decorView->setClipToPadding(false);
    return decorView;
}

void PopupWindow::updateAboveAnchor(bool aboveAnchor){
    if (aboveAnchor != mAboveAnchor) 
        return ;
    mAboveAnchor = aboveAnchor;

    if (mBackground  && mBackgroundView ) {
        // If the background drawable provided was a StateListDrawable
        // with above-anchor and below-anchor states, use those.
        // Otherwise, rely on refreshDrawableState to do the job.
        if (mAboveAnchorBackgroundDrawable) {
            if (mAboveAnchor) {
                mBackgroundView->setBackground(mAboveAnchorBackgroundDrawable);
            } else {
                mBackgroundView->setBackground(mBelowAnchorBackgroundDrawable);
            }
        } else {
            mBackgroundView->refreshDrawableState();
        }
    }
}

void PopupWindow::invokePopup(WindowManager::LayoutParams* p){
    //mDecorView->setFitsSystemWindows(mLayoutInsetDecor);
    setLayoutDirectionFromAnchor();
    WindowManager::getInstance().moveWindow(mDecorView,p->x,p->y);
    LOGD("invokePopup(%d,%d)",p->x,p->y);
    mDecorView->setLayoutParams(p);
    //mWindowManager->addView(mDecorView, p);
    /*if (mEnterTransition != nullptr) {
        mDecorView->requestEnterTransition(mEnterTransition);
    }*/
}

void PopupWindow::setLayoutDirectionFromAnchor() {
    if (mAnchor != nullptr) {
        View* anchor = mAnchor;//.get();
        if (anchor != nullptr && mPopupViewInitialLayoutDirectionInherited) {
            mDecorView->setLayoutDirection(anchor->getLayoutDirection());
        }
    }
}

bool PopupWindow::isAboveAnchor() const{
    return mAboveAnchor;
}

int PopupWindow::computeGravity() {
    int gravity = (mGravity == Gravity::NO_GRAVITY) ?  Gravity::START | Gravity::TOP : mGravity;
    if (mIsDropdown && (mClipToScreen || mClippingEnabled)) {
        gravity |= Gravity::DISPLAY_CLIP_VERTICAL;
    }
    return gravity;
}


int PopupWindow::getMaxAvailableHeight(View* anchor){
    return getMaxAvailableHeight(anchor, 0,false);
}

WindowManager::LayoutParams* PopupWindow::createPopupLayoutParams(long token){
    WindowManager::LayoutParams* p = new WindowManager::LayoutParams();
    p->x = p->y = 0;
    p->width = p->height = 0;
    // These gravity settings put the view at the top left corner of the
    // screen. The view is then positioned to the appropriate location by
    // setting the x and y offsets to match the anchor's bottom-left
    // corner.
    p->gravity = computeGravity();
    p->flags = computeFlags(p->flags);
    p->type = mWindowLayoutType;
    //p.token = token;
    //p.softInputMode = mSoftInputMode;
    //p.windowAnimations = computeAnimationResource();

    if (mBackground != nullptr) {
        p->format = mBackground->getOpacity();
    } else {
        p->format = PixelFormat::TRANSLUCENT;
    }

    if (mHeightMode < 0) {
        p->height = mLastHeight = mHeightMode;
    } else {
        p->height = mLastHeight = mHeight;
    }

    if (mWidthMode < 0) {
        p->width = mLastWidth = mWidthMode;
    } else {
        p->width = mLastWidth = mWidth;
    }

    //p->privateFlags = PRIVATE_FLAG_WILL_NOT_REPLACE_ON_RELAUNCH
    //        | PRIVATE_FLAG_LAYOUT_CHILD_WINDOW_IN_PARENT_FRAME;

    // Used for debugging.
    //p->setTitle("PopupWindow:" + Integer.toHexString(hashCode()));

    return p;
}

int PopupWindow::computeFlags(int curFlags){
    return curFlags;
}

bool PopupWindow::tryFitVertical(WindowManager::LayoutParams* outParams, int yOffset, int height,
        int anchorHeight, int drawingLocationY, int screenLocationY, int displayFrameTop,
        int displayFrameBottom, bool allowResize){
    const int winOffsetY = screenLocationY - drawingLocationY;
    const int anchorTopInScreen = outParams->y + winOffsetY;
    const int spaceBelow = displayFrameBottom - anchorTopInScreen;
    if ((anchorTopInScreen >= displayFrameTop) && (height <= spaceBelow)) {
        return true;
    }

    int spaceAbove = anchorTopInScreen - anchorHeight - displayFrameTop;
    if (height <= spaceAbove) {
        // Move everything up.
        if (mOverlapAnchor) {
            yOffset += anchorHeight;
        }
        outParams->y = drawingLocationY - height + yOffset;

        return true;
    }

    if (positionInDisplayVertical(outParams, height, drawingLocationY, screenLocationY,
            displayFrameTop, displayFrameBottom, allowResize)) {
        return true;
    }

    return false;
}

bool PopupWindow::positionInDisplayVertical(WindowManager::LayoutParams* outParams, int height,
        int drawingLocationY, int screenLocationY, int displayFrameTop, int displayFrameBottom,
        bool canResize){
    bool fitsInDisplay = true;
    const int winOffsetY = screenLocationY - drawingLocationY;
    outParams->y += winOffsetY;
    outParams->height = height;

    int bottom = outParams->y + height;
    if (bottom > displayFrameBottom) {
        // The popup is too far down, move it back in.
        outParams->y -= bottom - displayFrameBottom;
    }

    if (outParams->y < displayFrameTop) {
        // The popup is too far up, move it back in and clip if
        // it's still too large.
        outParams->y = displayFrameTop;

        const int displayFrameHeight = displayFrameBottom - displayFrameTop;
        if (canResize && (height > displayFrameHeight)) {
            outParams->height = displayFrameHeight;
        } else {
            fitsInDisplay = false;
        }
    }
    outParams->y -= winOffsetY;
    return fitsInDisplay;
}

bool PopupWindow::tryFitHorizontal(WindowManager::LayoutParams* outParams, int xOffset, int width,
        int anchorWidth, int drawingLocationX, int screenLocationX, int displayFrameLeft,
        int displayFrameRight, bool allowResize){
    const int winOffsetX = screenLocationX - drawingLocationX;
    const int anchorLeftInScreen = outParams->x + winOffsetX;
    const int spaceRight = displayFrameRight - anchorLeftInScreen;
    if ((anchorLeftInScreen >= displayFrameLeft) && (width <= spaceRight)) {
        return true;
    }

    if (positionInDisplayHorizontal(outParams, width, drawingLocationX, screenLocationX,
            displayFrameLeft, displayFrameRight, allowResize)) {
        return true;
    }
    return false;
}

bool PopupWindow::positionInDisplayHorizontal(WindowManager::LayoutParams* outParams, int width,
        int drawingLocationX, int screenLocationX, int displayFrameLeft, int displayFrameRight,
        bool canResize){
    bool fitsInDisplay = true; 
    // Use screen coordinates for comparison against display frame.
    const int winOffsetX = screenLocationX - drawingLocationX;
    outParams->x += winOffsetX;

    int right = outParams->x + width;
    if (right > displayFrameRight) {
        // The popup is too far right, move it back in.
        outParams->x -= right - displayFrameRight;
    }

    if (outParams->x < displayFrameLeft) {
        // The popup is too far left, move it back in and clip if it's
        // still too large.
        outParams->x = displayFrameLeft;

        const int displayFrameWidth = displayFrameRight - displayFrameLeft;
        if (canResize && width > displayFrameWidth) {
            outParams->width = displayFrameWidth;
        } else {
            fitsInDisplay = false;
        }
    }
    outParams->x -= winOffsetX;
    return fitsInDisplay;
}

const std::string PopupWindow::computeAnimationResource() {
    /*if (mAnimationStyle == ANIMATION_STYLE_DEFAULT) {
        if (mIsDropdown) {
            return mAboveAnchor
                    ? com.android.internal.R.style.Animation_DropDownUp
                    : com.android.internal.R.style.Animation_DropDownDown;
        }
        return 0;
    }*/
    return "";//mAnimationStyle;
}

bool PopupWindow::findDropDownPosition(View* anchor,WindowManager::LayoutParams* outParams,
       int xOffset, int yOffset, int width, int height, int gravity, bool allowScroll){
    int anchorHeight = anchor->getHeight();
    int anchorWidth = anchor->getWidth();
    if (mOverlapAnchor) {
        yOffset -= anchorHeight;
    }

    // Initially, align to the bottom-left corner of the anchor plus offsets.
    int appScreenLocation[2];
    View* appRootView = anchor->getRootView();//getAppRootView(anchor);
    appRootView->getLocationOnScreen(appScreenLocation);

    int screenLocation[2];
    anchor->getLocationOnScreen(screenLocation);

    int drawingLocation[2];
    drawingLocation[0] = screenLocation[0] - appScreenLocation[0];
    drawingLocation[1] = screenLocation[1] - appScreenLocation[1];
    outParams->x = drawingLocation[0] + xOffset;
    outParams->y = drawingLocation[1] + anchorHeight + yOffset;

    Rect displayFrame;
    appRootView->getWindowVisibleDisplayFrame(displayFrame);
    if (width == LayoutParams::MATCH_PARENT) {
        width = displayFrame.width;
    }
    if (height == LayoutParams::MATCH_PARENT) {
        height = displayFrame.height;
    }

    // Let the window manager know to align the top to y.
    outParams->gravity = computeGravity();
    outParams->width = width;
    outParams->height = height;

    // If we need to adjust for gravity RIGHT, align to the bottom-right
    // corner of the anchor (still accounting for offsets).
    int hgrav = Gravity::getAbsoluteGravity(gravity, anchor->getLayoutDirection())
                & Gravity::HORIZONTAL_GRAVITY_MASK;
    if (hgrav == Gravity::RIGHT) {
        outParams->x -= width - anchorWidth;
    }

    // First, attempt to fit the popup vertically without resizing.
    bool fitsVertical = tryFitVertical(outParams, yOffset, height,
            anchorHeight, drawingLocation[1], screenLocation[1], displayFrame.top,
            displayFrame.bottom(), false);

    // Next, attempt to fit the popup horizontally without resizing.
    bool fitsHorizontal = tryFitHorizontal(outParams, xOffset, width,
            anchorWidth, drawingLocation[0], screenLocation[0], displayFrame.left,
            displayFrame.right(), false);

    // If the popup still doesn't fit, attempt to scroll the parent.
    if (!fitsVertical || !fitsHorizontal) {
        int scrollX = anchor->getScrollX();
        int scrollY = anchor->getScrollY();
        Rect r = {scrollX, scrollY,  width + xOffset,
                    height + anchorHeight + yOffset};
        if (allowScroll && anchor->requestRectangleOnScreen(r, true)) {
            // Reset for the new anchor position.
            anchor->getLocationOnScreen(screenLocation);
            drawingLocation[0] = screenLocation[0] - appScreenLocation[0];
            drawingLocation[1] = screenLocation[1] - appScreenLocation[1];
            outParams->x = drawingLocation[0] + xOffset;
            outParams->y = drawingLocation[1] + anchorHeight + yOffset;
            // Preserve the gravity adjustment.
            if (hgrav == Gravity::RIGHT) {
                outParams->x -= width - anchorWidth;
            }
        }
        // Try to fit the popup again and allowing resizing.
        tryFitVertical(outParams, yOffset, height, anchorHeight, drawingLocation[1],
                screenLocation[1], displayFrame.top, displayFrame.bottom(), mClipToScreen);
        tryFitHorizontal(outParams, xOffset, width, anchorWidth, drawingLocation[0],
                screenLocation[0], displayFrame.left, displayFrame.right(), mClipToScreen);
    }

    // Return whether the popup's top edge is above the anchor's top edge.
    return outParams->y < drawingLocation[1];
}

int PopupWindow::getMaxAvailableHeight(View* anchor, int yOffset,bool ignoreBottomDecorations){
     Rect displayFrame;
     Rect visibleDisplayFrame;

     View* appView = anchor->getRootView();//getAppRootView(anchor);
     appView->getWindowVisibleDisplayFrame(visibleDisplayFrame);
     if (ignoreBottomDecorations) {
        // In the ignore bottom decorations case we want to
        // still respect all other decorations so we use the inset visible
        // frame on the top right and left and take the bottom
        // value from the full frame.
        anchor->getWindowDisplayFrame(displayFrame);
        displayFrame.top = visibleDisplayFrame.top;
        displayFrame.width= visibleDisplayFrame.width;
        displayFrame.left = visibleDisplayFrame.left;
    } else {
        displayFrame = visibleDisplayFrame;
    }

    int anchorPos[2];
    anchor->getLocationOnScreen(anchorPos);

    const int bottomEdge = displayFrame.bottom();

    int distanceToBottom;
    if (mOverlapAnchor) {
        distanceToBottom = bottomEdge - anchorPos[1] - yOffset;
    } else {
        distanceToBottom = bottomEdge - (anchorPos[1] + anchor->getHeight()) - yOffset;
    }
    const int distanceToTop = anchorPos[1] - displayFrame.top + yOffset;

    // anchorPos[1] is distance from anchor to top of screen
    int returnedHeight = std::max(distanceToBottom, distanceToTop);
    if (mBackground ) {
        Rect mTempRect; 
        mBackground->getPadding(mTempRect);
        returnedHeight -= mTempRect.top + mTempRect.height;
    }

    return returnedHeight;
}

void PopupWindow::dismiss(){
    if (!isShowing() /*|| isTransitioningToDismiss()*/) {
        return;
    }
    PopupDecorView* decorView = mDecorView;
    View* contentView = mContentView;

    ViewGroup* contentHolder;
    ViewGroup* contentParent = contentView->getParent();
    contentHolder = ((ViewGroup*) contentParent);

    // Ensure any ongoing or pending transitions are canceled.
    //decorView->cancelTransitions();

    mIsShowing = false;
    mIsTransitioningToDismiss = true;
    // This method may be called as part of window detachment, in which
    // case the anchor view (and its root) will still return true from
    // isAttachedToWindow() during execution of this method; however, we
    // can expect the OnAttachStateChangeListener to have been called prior
    // to executing this method, so we can rely on that instead.
    /*Transition exitTransition = mExitTransition;
    if (exitTransition && decorView->isLaidOut()
            && (mIsAnchorRootAttached || mAnchorRoot == nullptr)) {
        // The decor view is non-interactive and non-IME-focusable during exit transitions.
        LayoutParams p = (LayoutParams) decorView.getLayoutParams();
        p.flags |= LayoutParams.FLAG_NOT_TOUCHABLE;
        p.flags |= LayoutParams.FLAG_NOT_FOCUSABLE;
        p.flags &= ~LayoutParams.FLAG_ALT_FOCUSABLE_IM;
        mWindowManager.updateViewLayout(decorView, p);

        View anchorRoot = mAnchorRoot != null ? mAnchorRoot.get() : null;
        Rect epicenter = getTransitionEpicenter();

        // Once we start dismissing the decor view, all state (including
        // the anchor root) needs to be moved to the decor view since we
        // may open another popup while it's busy exiting.
        decorView.startExitTransition(exitTransition, anchorRoot, epicenter,
                new TransitionListenerAdapter() {
                    @Override
                    public void onTransitionEnd(Transition transition) {
                        dismissImmediate(decorView, contentHolder, contentView);
                    }
                });
    } else */{
        dismissImmediate(decorView, contentHolder, contentView);
    }

    // Clears the anchor view.
    detachFromAnchor();
    if (mOnDismissListener != nullptr) {
        mOnDismissListener();
    }
}

Rect PopupWindow::getTransitionEpicenter(){
    if (mAnchor == nullptr || mDecorView == nullptr) {
        return Rect::MakeWH(0,0);
    }

    int anchorLocation[2],popupLocation[2];
    mAnchor->getLocationOnScreen(anchorLocation);
    mDecorView->getLocationOnScreen(popupLocation);

    // Compute the position of the anchor relative to the popup.
    Rect bounds = {0, 0, mAnchor->getWidth(), mAnchor->getHeight()};
    bounds.offset(anchorLocation[0] - popupLocation[0], anchorLocation[1] - popupLocation[1]);

    // Use anchor-relative epicenter, if specified.
    if (!mEpicenterBounds.empty()){// != null) {
        int offsetX = bounds.left;
        int offsetY = bounds.top;
        bounds=mEpicenterBounds;//set(mEpicenterBounds);
        bounds.offset(offsetX, offsetY);
    }
    return bounds;
}

void PopupWindow::dismissImmediate(View* decorView, ViewGroup* contentHolder, View* contentView){
    if (decorView->getParent()!=nullptr) {
        //mWindowManager.removeViewImmediate(decorView);
    }

    if (contentHolder != nullptr) {
        ViewGroup*vg=dynamic_cast<ViewGroup*>(contentView);
        contentHolder->removeView(contentView);
    }

    // This needs to stay until after all transitions have ended since we
    // need the reference to cancel transitions in preparePopup().
    ((Window*)(mDecorView))->close();
    LOGD("close mDecorView %p which its contentView=%p",mDecorView,contentView);
    mDecorView = nullptr;
    mBackgroundView = nullptr;
    mIsTransitioningToDismiss = false;
}

void PopupWindow::setOnDismissListener(OnDismissListener onDismissListener) {
    mOnDismissListener = onDismissListener;
}

/** @hide */
PopupWindow::OnDismissListener PopupWindow::getOnDismissListener() {
    return mOnDismissListener;
}

void PopupWindow::update(){
    if (!isShowing() || !hasContentView()) {
        return;
    }
    WindowManager::LayoutParams* p = getDecorViewLayoutParams();

    bool bUpdate = false;

    /*int newAnim = computeAnimationResource();
    if (newAnim != p.windowAnimations) {
        p.windowAnimations = newAnim;
        bUpdate = true;
    }*/

    int newFlags = computeFlags(p->flags);
    if (newFlags != p->flags) {
        p->flags = newFlags;
        bUpdate = true;
    }

    int newGravity = computeGravity();
    if (newGravity != p->gravity) {
        p->gravity = newGravity;
        bUpdate = true;
    }

    if (bUpdate) {
        update(mAnchor, p);
    }
}

void PopupWindow::update(View* anchor,WindowManager::LayoutParams* params) {
    setLayoutDirectionFromAnchor();
    // mWindowManager.updateViewLayout(mDecorView, params);
    LOGD("PopupWindow.pos=%d,%d",params->x,params->y);
}

void PopupWindow::update(int width, int height){
    WindowManager::LayoutParams*p=getDecorViewLayoutParams();
    update(p->x,p->y, width, height);
}

void PopupWindow::update(int x, int y, int width, int height,bool force){
    if (width >= 0) {
        mLastWidth = width;
        setWidth(width);
    }

    if (height >= 0) {
        mLastHeight = height;
        setHeight(height);
    }

    if (!isShowing() || !hasContentView()) {
        return;
    }


    bool updated = force;

    WindowManager::LayoutParams* p = getDecorViewLayoutParams();
    int finalWidth = mWidthMode < 0 ? mWidthMode : mLastWidth;
    if (width != -1 && p->width != finalWidth) {
        p->width = mLastWidth = finalWidth;
        updated = true;
    }

    int finalHeight = mHeightMode < 0 ? mHeightMode : mLastHeight;
    if (height != -1 && p->height != finalHeight) {
        p->height = mLastHeight = finalHeight;
        updated = true;
    }

    if (p->x != x) {
        p->x = x;
        updated = true;
    }

    if (p->y != y) {
        p->y = y;
        updated = true;
    }

    /*int newAnim = computeAnimationResource();
    if (newAnim != p->windowAnimations) {
        p->windowAnimations = newAnim;
        updated = true;
    }*/

    const int newFlags = computeFlags(p->flags);
    if (newFlags != p->flags) {
        p->flags = newFlags;
        updated = true;
    }

    const int newGravity = computeGravity();
    if (newGravity != p->gravity) {
        p->gravity = newGravity;
        updated = true;
    }
    View* anchor = nullptr;
    int newAccessibilityIdOfAnchor = -1;

    if (mAnchor != nullptr) {
        anchor = mAnchor;
        newAccessibilityIdOfAnchor = anchor->getAccessibilityViewId();
    }

    /*if (newAccessibilityIdOfAnchor != p.accessibilityIdOfAnchor) {
        p.accessibilityIdOfAnchor = newAccessibilityIdOfAnchor;
        update = true;
    }*/

    if (updated) {
        update(anchor, p);
    }
}

void PopupWindow::update(View* anchor, int xoff, int yoff, int width, int height){
    update(anchor,false,xoff,yoff,width,height);
}

void PopupWindow::update(View* anchor, bool updateLocation, int xoff, int yoff, int width, int height){
    if (!isShowing() || !hasContentView()) {
        return;
    }

    View* oldAnchor = mAnchor;
    int gravity = mAnchoredGravity;

    bool needsUpdate = updateLocation && (mAnchorXoff != xoff || mAnchorYoff != yoff);
    if (oldAnchor == nullptr || oldAnchor != anchor || (needsUpdate && !mIsDropdown)) {
        attachToAnchor(anchor, xoff, yoff, gravity);
    } else if (needsUpdate) {
        // No need to register again if this is a DropDown, showAsDropDown already did.
        mAnchorXoff = xoff;
        mAnchorYoff = yoff;
    }

    WindowManager::LayoutParams* p = getDecorViewLayoutParams();
    int oldGravity = p->gravity;
    int oldWidth = p->width;
    int oldHeight = p->height;
    int oldX = p->x;
    int oldY = p->y;

    // If an explicit width/height has not specified, use the most recent
    // explicitly specified value (either from setWidth/Height or update).
    if (width < 0) {
        width = mWidth;
    }
    if (height < 0) {
        height = mHeight;
    }

    bool aboveAnchor = findDropDownPosition(anchor, p, mAnchorXoff, mAnchorYoff,
                width, height, gravity, mAllowScrollingAnchorParent);
    updateAboveAnchor(aboveAnchor);

    bool paramsChanged = oldGravity != p->gravity || oldX != p->x || oldY != p->y
                || oldWidth != p->width || oldHeight != p->height;

    // If width and mWidth were both < 0 then we have a MATCH_PARENT or
    // WRAP_CONTENT case. findDropDownPosition will have resolved this to
    // absolute values, but we don't want to update mWidth/mHeight to these
    // absolute values.
    int newWidth = width < 0 ? width : p->width;
    int newHeight = height < 0 ? height : p->height;
    update(p->x, p->y, newWidth, newHeight, paramsChanged);
}

bool PopupWindow::hasContentView()const{
    return mContentView!=nullptr;
}

bool PopupWindow::hasDecorView()const{
    return mDecorView!=nullptr;
}

WindowManager::LayoutParams* PopupWindow::getDecorViewLayoutParams() {
    return (WindowManager::LayoutParams*) mDecorView->getLayoutParams();
}

void PopupWindow::detachFromAnchor(){
    View* anchor = mAnchor;//getAnchor();
    if (anchor) {
        ViewTreeObserver* vto = anchor->getViewTreeObserver();
        vto->removeOnScrollChangedListener(mOnScrollChangedListener);
        anchor->removeOnAttachStateChangeListener(mOnAnchorDetachedListener);
    }

    View* anchorRoot = mAnchorRoot;// != null ? mAnchorRoot.get() : null;
    if (anchorRoot != nullptr) {
        anchorRoot->removeOnAttachStateChangeListener(mOnAnchorRootDetachedListener);
        anchorRoot->removeOnLayoutChangeListener(mOnLayoutChangeListener);
    }

    mAnchor = nullptr;
    mAnchorRoot = nullptr;
    mIsAnchorRootAttached = false;
}

void PopupWindow::attachToAnchor(View* anchor, int xoff, int yoff, int gravity){
    detachFromAnchor();

    ViewTreeObserver* vto = anchor->getViewTreeObserver();
    if (vto) {
        vto->addOnScrollChangedListener(mOnScrollChangedListener);
    }
    anchor->addOnAttachStateChangeListener(mOnAnchorDetachedListener);

    View* anchorRoot = anchor->getRootView();
    anchorRoot->addOnAttachStateChangeListener(mOnAnchorRootDetachedListener);
    anchorRoot->addOnLayoutChangeListener(mOnLayoutChangeListener);

    mAnchor = anchor;//new WeakReference<>(anchor);
    mAnchorRoot = anchorRoot;//new WeakReference<>(anchorRoot);
    mIsAnchorRootAttached = anchorRoot->isAttachedToWindow();
    mParentRootView = mAnchorRoot;

    mAnchorXoff = xoff;
    mAnchorYoff = yoff;
    mAnchoredGravity = gravity;
}

void PopupWindow::alignToAnchor() {
    if (mAnchor && mAnchor->isAttachedToWindow() && hasDecorView()) {
        WindowManager::LayoutParams* p = getDecorViewLayoutParams();
        updateAboveAnchor(findDropDownPosition(mAnchor, p, mAnchorXoff, mAnchorYoff,
                p->width, p->height, mAnchoredGravity, false));
        update(p->x, p->y, -1, -1, true);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
PopupWindow::PopupDecorView::PopupDecorView(int w,int h)
   :Window(0,0,w,h){
    mPop = nullptr;
}

bool PopupWindow::PopupDecorView::dispatchKeyEvent(KeyEvent& event){
     if (event.getKeyCode() == KeyEvent::KEYCODE_BACK) {
        if (getKeyDispatcherState() == nullptr) {
            return Window::dispatchKeyEvent(event);
        }

        if (event.getAction() == KeyEvent::ACTION_DOWN && event.getRepeatCount() == 0) {
            KeyEvent::DispatcherState* state = getKeyDispatcherState();
            if (state ) {
                state->startTracking(event, this);
            }
            return true;
        } else if (event.getAction() == KeyEvent::ACTION_UP) {
            KeyEvent::DispatcherState* state = getKeyDispatcherState();
            if (state && state->isTracking(event) && !event.isCanceled()) {
                mPop->dismiss();
                return true;
            }
        }
        return Window::dispatchKeyEvent(event);
    } else {
        return Window::dispatchKeyEvent(event);
    }
}

bool PopupWindow::PopupDecorView::dispatchTouchEvent(MotionEvent& ev){
    if (mPop && mPop->mTouchInterceptor && mPop->mTouchInterceptor(*this, ev)) {
        return true;
    }
    return Window::dispatchTouchEvent(ev);
}

bool PopupWindow::PopupDecorView::onTouchEvent(MotionEvent& event){
    const int x = (int) event.getX();
    const int y = (int) event.getY();

    if ((event.getAction() == MotionEvent::ACTION_DOWN)
           && ((x < 0) || (x >= getWidth()) || (y < 0) || (y >= getHeight()))) {
        mPop->dismiss();
        return true;
    } else if (event.getAction() == MotionEvent::ACTION_OUTSIDE) {
        mPop->dismiss();
        return true;
    } else {
        return Window::onTouchEvent(event);
    }
}

PopupWindow::PopupBackgroundView::PopupBackgroundView(Context* context)
:FrameLayout(context,AttributeSet(context,"")){
}
}
