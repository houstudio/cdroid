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
    if (mContext != nullptr && !mAttachedInDecorSet) {
        // Attach popup window in decor frame of parent window by default for
        // {@link Build.VERSION_CODES.LOLLIPOP_MR1} or greater. Keep current
        // behavior of not attaching to decor frame for older SDKs.
        setAttachedInDecor(true);//mContext.getApplicationInfo().targetSdkVersion >= Build.VERSION_CODES.LOLLIPOP_MR1);
    }
}

void PopupWindow::setTouchInterceptor(View::OnTouchListener l) {
    mTouchInterceptor = l;
}

bool PopupWindow::isFocusable() {
    return mFocusable;
}

void PopupWindow::setFocusable(bool focusable) {
    mFocusable = focusable;
}

bool PopupWindow::isOutsideTouchable()const{
    return mOutsideTouchable;
}

void PopupWindow::setOutsideTouchable(bool touchable){
    mOutsideTouchable=touchable;
}

bool PopupWindow::isLayoutInScreenEnabled()const{
    return mLayoutInScreen;
}

void PopupWindow::setLayoutInScreenEnabled(bool enabled){
    mLayoutInScreen=enabled;
}

bool PopupWindow::isAttachedInDecor()const{
    return mAttachedInDecor;
}

void PopupWindow::setAttachedInDecor(bool enabled){
    mAttachedInDecor= enabled;
    mAttachedInDecorSet=true;
}

void PopupWindow::setLayoutInsetDecor(bool enabled) {
    mLayoutInsetDecor = enabled;
}

bool PopupWindow::isLayoutInsetDecor()const{
    return mLayoutInsetDecor;
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

    LayoutParams* p=nullptr;//=createPopupLayoutParams(anchor.getApplicationWindowToken());
    preparePopup(p);

    bool aboveAnchor =false;
        //findDropDownPosition(anchor, p, xoff, yoff,
        //p.width, p.height, gravity, mAllowScrollingAnchorParent);
    updateAboveAnchor(aboveAnchor);
    //p.accessibilityIdOfAnchor = (anchor) ? anchor->getAccessibilityViewId() : -1;
    invokePopup(nullptr);
}

void PopupWindow::preparePopup(LayoutParams*p){
    //if (mDecorView)  mDecorView->cancelTransitions();

    // When a background is available, we embed the content view within
    // another view that owns the background drawable.
    if (mBackground) {
        mBackgroundView = createBackgroundView(mContentView);
        mBackgroundView->setBackground(mBackground);
    } else {
        mBackgroundView = mContentView;
    }

    mDecorView = createDecorView(mBackgroundView);
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
    if (layoutParams && layoutParams->height == LayoutParams::WRAP_CONTENT) {
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
    if (layoutParams  && layoutParams->height == LayoutParams::WRAP_CONTENT) {
        height = LayoutParams::WRAP_CONTENT;
    } else {
        height = LayoutParams::MATCH_PARENT;
    }

    PopupDecorView* decorView = new PopupDecorView(mContext);
    decorView->addView(contentView, LayoutParams::MATCH_PARENT, height);
    decorView->setClipChildren(false);
    decorView->setClipToPadding(false);

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

void PopupWindow::invokePopup(LayoutParams* p){
}

void PopupWindow::setLayoutDirectionFromAnchor() {
    if (mAnchor != nullptr) {
        View* anchor = mAnchor;//.get();
        if (anchor != nullptr && mPopupViewInitialLayoutDirectionInherited) {
            mDecorView->setLayoutDirection(anchor->getLayoutDirection());
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

void PopupWindow::update(View* anchor,LayoutParams* params) {
    setLayoutDirectionFromAnchor();
   // mWindowManager.updateViewLayout(mDecorView, params);
}

void PopupWindow::update(int width, int height){
}

void PopupWindow::update(int x, int y, int width, int height){
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


    bool updated = true;//force;
#if 0
    WindowManager.LayoutParams p = getDecorViewLayoutParams();
    int finalWidth = mWidthMode < 0 ? mWidthMode : mLastWidth;
    if (width != -1 && p.width != finalWidth) {
        p.width = mLastWidth = finalWidth;
        updated = true;
    }

    int finalHeight = mHeightMode < 0 ? mHeightMode : mLastHeight;
    if (height != -1 && p.height != finalHeight) {
        p.height = mLastHeight = finalHeight;
        updated = true;
    }

    if (p.x != x) {
        p.x = x;
        updated = true;
    }

    if (p.y != y) {
        p.y = y;
        updated = true;
    }

    int newAnim = computeAnimationResource();
    if (newAnim != p.windowAnimations) {
        p.windowAnimations = newAnim;
        updated = true;
    }

    const int newFlags = computeFlags(p.flags);
    if (newFlags != p.flags) {
        p.flags = newFlags;
        updated = true;
    }

    const int newGravity = computeGravity();
    if (newGravity != p.gravity) {
        p.gravity = newGravity;
        updated = true;
    }
#endif
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
        //update(anchor, p);
    }
}

bool PopupWindow::hasContentView()const{
    return mContentView!=nullptr;
}

bool PopupWindow::hasDecorView()const{
    return mDecorView!=nullptr;
}

void PopupWindow::detachFromAnchor(){
    View* anchor = mAnchor;//getAnchor();
    /*if (anchor) {
        ViewTreeObserver vto = anchor.getViewTreeObserver();
        vto->removeOnScrollChangedListener(mOnScrollChangedListener);
        anchor->removeOnAttachStateChangeListener(mOnAnchorDetachedListener);
    }*/

    /*View* anchorRoot = mAnchorRoot != null ? mAnchorRoot.get() : null;
    if (anchorRoot != nullptr) {
        anchorRoot->removeOnAttachStateChangeListener(mOnAnchorRootDetachedListener);
        anchorRoot->removeOnLayoutChangeListener(mOnLayoutChangeListener);
    }*/

    mAnchor = nullptr;
    mAnchorRoot = nullptr;
    mIsAnchorRootAttached = false;
}

void PopupWindow::attachToAnchor(View* anchor, int xoff, int yoff, int gravity){
    detachFromAnchor();

    /*ViewTreeObserver vto = anchor.getViewTreeObserver();
    if (vto) {
        vto.addOnScrollChangedListener(mOnScrollChangedListener);
    }*/
    //anchor.addOnAttachStateChangeListener(mOnAnchorDetachedListener);

    View* anchorRoot = anchor->getRootView();
    //anchorRoot->addOnAttachStateChangeListener(mOnAnchorRootDetachedListener);
    //anchorRoot->addOnLayoutChangeListener(mOnLayoutChangeListener);

    mAnchor = anchor;//new WeakReference<>(anchor);
    mAnchorRoot = anchorRoot;//new WeakReference<>(anchorRoot);
    mIsAnchorRootAttached = anchorRoot->isAttachedToWindow();
    mParentRootView = mAnchorRoot;

    mAnchorXoff = xoff;
    mAnchorYoff = yoff;
    mAnchoredGravity = gravity;
}

/////////////////////////////////////////////////////////////////////////////////////
PopupWindow::PopupDecorView::PopupDecorView(Context* context):FrameLayout(context,AttributeSet()){
    mPop=nullptr;
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
    if (mPop && mPop->mTouchInterceptor && mPop->mTouchInterceptor(*this, ev)) {
        return true;
    }
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

PopupWindow::PopupBackgroundView::PopupBackgroundView(Context* context)
:FrameLayout(context,AttributeSet()){
}
}
