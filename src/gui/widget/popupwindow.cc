#include <widget/popupwindow.h>
namespace cdroid{

PopupWindow::PopupWindow(Context* context,const AttributeSet& attrs){
    mContext = context;
    //mWindowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
#if 0 
    final TypedArray a = context.obtainStyledAttributes(
                attrs, R.styleable.PopupWindow, defStyleAttr, defStyleRes);
    Drawable* bg = a.getDrawable(R.styleable.PopupWindow_popupBackground);
    mElevation = a.getDimension(R.styleable.PopupWindow_popupElevation, 0);
    mOverlapAnchor = a.getBoolean(R.styleable.PopupWindow_overlapAnchor, false);

    // Preserve default behavior from Gingerbread. If the animation is
    // undefined or explicitly specifies the Gingerbread animation style,
    // use a sentinel value.
    if (a.hasValueOrEmpty(R.styleable.PopupWindow_popupAnimationStyle)) {
        int animStyle = a.getResourceId(R.styleable.PopupWindow_popupAnimationStyle, 0);
        if (animStyle == R.style.Animation_PopupWindow) {
            mAnimationStyle = ANIMATION_STYLE_DEFAULT;
        } else {
            mAnimationStyle = animStyle;
        }
    } else {
        mAnimationStyle = ANIMATION_STYLE_DEFAULT;
    }

    final Transition enterTransition = getTransition(a.getResourceId(
            R.styleable.PopupWindow_popupEnterTransition, 0));
    final Transition exitTransition;
    if (a.hasValueOrEmpty(R.styleable.PopupWindow_popupExitTransition)) {
        exitTransition = getTransition(a.getResourceId(
                R.styleable.PopupWindow_popupExitTransition, 0));
    } else {
        exitTransition = enterTransition == null ? null : enterTransition.clone();
    }

    a.recycle();

    setEnterTransition(enterTransition);
    setExitTransition(exitTransition);
    setBackgroundDrawable(bg);
#endif
}

PopupWindow::PopupWindow(View* contentView, int width, int height, bool focusable) {
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
        int count = stateList->getStateCount();
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

float PopupWindow::getElevation() {
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

    if (mContext == nullptr && mContentView != nullptr) {
        mContext = mContentView->getContext();
    }

    /*if (mWindowManager == null && mContentView != nullptr) {
        mWindowManager = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
    }*/

    // Setting the default for attachedInDecor based on SDK version here
    // instead of in the constructor since we might not have the context
    // object in the constructor. We only want to set default here if the
    // app hasn't already set the attachedInDecor.
    //if (mContext != null && !mAttachedInDecorSet) {
        // Attach popup window in decor frame of parent window by default for
        // {@link Build.VERSION_CODES.LOLLIPOP_MR1} or greater. Keep current
        // behavior of not attaching to decor frame for older SDKs.
        //setAttachedInDecor(mContext.getApplicationInfo().targetSdkVersion >= Build.VERSION_CODES.LOLLIPOP_MR1);
    //}
}

bool PopupWindow::isFocusable() {
    return mFocusable;
}

void PopupWindow::setFocusable(bool focusable) {
    mFocusable = focusable;
}
void PopupWindow::setShowing(bool isShowing) {
     mIsShowing = isShowing;
}
bool PopupWindow::isShowing(){
    return mIsShowing;
}
void PopupWindow::setDropDown(bool isDropDown) {
     mIsDropdown = isDropDown;
}
void PopupWindow::setHeight(int height){
    mHeight=height;
}

int PopupWindow::getHeight(){
    return mHeight;
}
void PopupWindow::setWidth(int width){
    mWidth=width;
}
int PopupWindow::getWidth(){
    return mHeight;
}

void PopupWindow::showAtLocation(View* parent, int gravity, int x, int y){
}

void PopupWindow::showAsDropDown(View* anchor){
    showAsDropDown(anchor,0,0);
}
void PopupWindow::showAsDropDown(View* anchor, int xoff, int yoff){
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

bool PopupWindow::isAboveAnchor() {
    return mAboveAnchor;
}

int PopupWindow::computeGravity() {
    int gravity = mGravity == Gravity::NO_GRAVITY ?  Gravity::START | Gravity::TOP : mGravity;
    if (mIsDropdown && (mClipToScreen || mClippingEnabled)) {
        gravity |= Gravity::DISPLAY_CLIP_VERTICAL;
    }
    return gravity;
}


int PopupWindow::getMaxAvailableHeight(View* anchor){
    return getMaxAvailableHeight(anchor, 0,false);
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

    int anchorPos[2];// = mTmpDrawingLocation;
    anchor->getLocationOnScreen(anchorPos);

    int bottomEdge = displayFrame.bottom();

    int distanceToBottom;
    if (mOverlapAnchor) {
        distanceToBottom = bottomEdge - anchorPos[1] - yOffset;
    } else {
        distanceToBottom = bottomEdge - (anchorPos[1] + anchor->getHeight()) - yOffset;
    }
    int distanceToTop = anchorPos[1] - displayFrame.top + yOffset;

    // anchorPos[1] is distance from anchor to top of screen
    int returnedHeight = std::max(distanceToBottom, distanceToTop);
    if (mBackground ) {
        Rect mTempRect; 
        mBackground->getPadding(mTempRect);
        returnedHeight -= mTempRect.top + mTempRect.bottom();
    }

    return returnedHeight;
}

void PopupWindow::dismiss(){
    if (!isShowing() /*|| isTransitioningToDismiss()*/) {
        return;
    }
}

void PopupWindow::setOnDismissListener(OnDismissListener onDismissListener) {
    mOnDismissListener = onDismissListener;
}

/** @hide */
PopupWindow::OnDismissListener PopupWindow::getOnDismissListener() {
    return mOnDismissListener;
}

void PopupWindow::update(){

}

void PopupWindow::update(int width, int height){
}

void PopupWindow::update(int x, int y, int width, int height){
}

bool PopupWindow::hasContentView()const{
    return mContentView!=nullptr;
}

bool PopupWindow::hasDecorView()const{
    return mDecorView!=nullptr;
}


/////////////////////////////////////////////////////////////////////////////////////
PopupWindow::PopupDecorView::PopupDecorView(Context* context):FrameLayout(context,AttributeSet()){
}

bool PopupWindow::PopupDecorView::dispatchKeyEvent(KeyEvent& event){
     if (event.getKeyCode() == KEY_BACK) {
        if (getKeyDispatcherState() == nullptr) {
            return FrameLayout::dispatchKeyEvent(event);
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
        return FrameLayout::dispatchKeyEvent(event);
    } else {
        return FrameLayout::dispatchKeyEvent(event);
    }
}

bool PopupWindow::PopupDecorView::dispatchTouchEvent(MotionEvent& ev){
    /*if (mTouchInterceptor && mTouchInterceptor->onTouch(this, ev)) {
        return true;
    }*/
    return FrameLayout::dispatchTouchEvent(ev);
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
        return FrameLayout::onTouchEvent(event);
    }
}

}
