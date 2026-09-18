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
#include <widget/internal_R.h>
#include <widget/popupwindow.h>
#include <widget/framework_styleable.h>
#include <content/typedvalue.h>
#include <cdlog.h>
namespace cdroid{
using namespace cdroid::internal;

namespace {
// Clone a drawable through its ConstantState; null when the drawable has none
// (base Drawable::getConstantState returns null — only ConstantState-capable
// subclasses override it). Callers treat null as "cannot clone".
Drawable* newDrawableOrNull(Drawable* d) {
    if (d == nullptr) return nullptr;
    auto cs = d->getConstantState();   // shared_ptr<ConstantState>
    return cs ? cs->newDrawable() : nullptr;
}
} // namespace

PopupWindow::PopupWindow(Context*ctx)
    :PopupWindow(ctx,nullptr){}

PopupWindow::PopupWindow(Context* context,const AttributeSet* attrs)
    :PopupWindow(context,attrs,R::attr::popupWindowStyle){
}

PopupWindow::PopupWindow(Context* context,const AttributeSet* attrs, int defStyleAttr)
    :PopupWindow(context,attrs,defStyleAttr,0){
}

PopupWindow::PopupWindow(Context* context,const AttributeSet* attrs, int defStyleAttr, int defStyleRes){
    init();
    mContext = context;
    // AOSP: context.obtainStyledAttributes(attrs, R.styleable.PopupWindow, defStyleAttr,
    // defStyleRes). Read the consumed attrs (popupBackground/popupElevation/overlapAnchor)
    // via R.styleable.PopupWindow named indices (no more hand-cobbled attr-id array).
    auto ta = context->obtainStyledAttributes(attrs, R::styleable::PopupWindow, defStyleAttr, defStyleRes);
    if (ta) {
        Drawable* bg = ta->getDrawable(R::styleable::PopupWindow_popupBackground);
        mElevation = ta->getFloat(R::styleable::PopupWindow_popupElevation, 0);
        mOverlapAnchor = ta->getBoolean(R::styleable::PopupWindow_overlapAnchor, false);
        mAnimationStyle = ta->getResourceId(R::styleable::PopupWindow_popupAnimationStyle,
                                            ANIMATION_STYLE_DEFAULT);
        setBackgroundDrawable(bg);
    }
    LOGD("create PopupWindow %p background=%p",this,mBackground);
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
    LOGD("destroy PopupWindow %p mBackground=%p",this,mBackground);
    // Delete-at-any-time teardown. A STILL-SHOWING popup's decor Window must
    // be torn down here -- it would otherwise stay on screen forever holding a
    // dangling mPop back-pointer. Mechanical teardown only, mirroring
    // dismissImmediate: no dismiss notification fires (the owner is destroying
    // the object, not closing it).
    // An already-DISMISSED popup (mIsShowing false) must NOT close again:
    // dismissImmediate already closed the decor (synchronously) and detached
    // its back-pointer; the decor lives until its own posted delete. Calling
    // close() on that half-torn state re-enters WindowManager::removeWindow on
    // a window that is no longer in a consistent state.
    if ((mDecorView != nullptr) && mIsShowing) {
        if (!mOwnsContentView && (mContentView != nullptr)
                && (mContentView->getParent() != nullptr)) {
            // Borrowed content goes back to the owner before the decor tree
            // (which frees owned content) is posted for deletion.
            ViewGroup* holder = dynamic_cast<ViewGroup*>(mContentView->getParent());
            if (holder != nullptr) {
                holder->removeView(mContentView);
            }
        }
        ((Window*)mDecorView)->close();
        // The decor was JUST closed by us: alive until its posted delete, so
        // detaching the back-pointer is safe. In the already-DISMISSED case
        // below the decor may already be freed - touching it wrote to freed
        // memory (valgrind: invalid write in detachOwner from ~PopupWindow).
        mDecorView->detachOwner();
    }
    mDecorView = nullptr;
    // Symmetric unregister of the anchor/anchor-root listeners (they capture
    // this) - dismiss() normally does this, the destructor must too.
    detachFromAnchor();
    delete mBackground;
    // mAboveAnchor/mBelowAnchorBackgroundDrawable are children borrowed from the
    // mBackground StateListDrawable — the container owns and deletes them (AOSP
    // relies on GC here).
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
    mWindowLayoutType = 0;
    mGravity = Gravity::NO_GRAVITY;
    mInputMethodMode = INPUT_METHOD_FROM_FOCUSABLE;
    mAllowScrollingAnchorParent= true;
    mLayoutInsetDecor = false;
    mAttachedInDecor  = true;
    mAttachedInDecorSet= false;
    mEpicenterBounds.setEmpty();

    mWidth = LayoutParams::WRAP_CONTENT;
    mHeight = LayoutParams::WRAP_CONTENT;
    mWidthMode = mHeightMode =0;
    mParentRootView = nullptr;
    mAnchor = nullptr;
    mDecorView  = nullptr;
    mAnchorRoot = nullptr;
    mBackground = nullptr;
    mBackgroundView = nullptr;
    mWindowLayoutType = Window::TYPE_APPLICATION;
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

// Fragment/MenuPopupWindow API surface (androidx kept these for L popup
// content transitions). STORED ONLY: popup motion runs on the WINDOW level
// (setWindowAnimations from computeAnimationResource, wired in invokePopup);
// no consumer renders these android.transition objects on CDROID.
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

void PopupWindow::setAnimationStyle(int animationStyle) {
    mAnimationStyle = animationStyle;
}

int PopupWindow::getAnimationStyle() const {
    return mAnimationStyle;
}

void PopupWindow::setEpicenterBounds(const Rect& bounds) {
    mEpicenterBounds = bounds;
}

Drawable* PopupWindow::getBackground() {
    return mBackground;
}

// AOSP PopupWindow.ABOVE_ANCHOR_STATE_SET = { com.android.internal.R.attr.state_above_anchor }.
static const std::vector<int> ABOVE_ANCHOR_STATE_SET = { R::attr::state_above_anchor };

void PopupWindow::setBackgroundDrawable(Drawable* background) {
    if (mBackground != background) {
        // The old background is owned (ctor-inflated theme default, or a
        // previous set) — delete it on replace. AOSP leans on GC here; in C++
        // the Spinner sequence (PopupWindow ctor reads the THEME default
        // popupBackground, then Spinner applies android:popupBackground from
        // its styleable) leaks the first drawable without this. The above/
        // below anchor drawables borrow from the old container's children —
        // drop them before it goes (the extraction below re-derives them
        // from the new background, or leaves null for a non-state-list one).
        mAboveAnchorBackgroundDrawable = nullptr;
        mBelowAnchorBackgroundDrawable = nullptr;
        delete mBackground;
        mBackground = background;
    }

    if (dynamic_cast<StateListDrawable*>(mBackground)) {
        StateListDrawable* stateList = (StateListDrawable*) mBackground;

        // Find the above-anchor view - this one's easy, it should be labeled as such.
        // (AOSP android-36 renamed this call to findStateDrawableIndex.)
        int aboveAnchorStateIndex = stateList->getStateDrawableIndex(ABOVE_ANCHOR_STATE_SET);

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
    return mWidth;
}

void PopupWindow::showAtLocation(View* parent, int gravity, int x, int y){
    if (isShowing() || (mContentView == nullptr)) {
        return;
    }

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
        // setBackground takes ownership, so hand the wrapper a clone — mBackground
        // itself survives for updateAboveAnchor's state swap. A drawable without
        // ConstantState support cannot be cloned (base returns null): fall back to
        // a transparent placeholder so the wrapper structure and ownership stay
        // intact instead of dereferencing null.
        Drawable* bg = newDrawableOrNull(mBackground);
        if (bg == nullptr) bg = new ColorDrawable(Color::TRANSPARENT);
        mBackgroundView = createBackgroundView(mContentView);
        mBackgroundView->setBackground(bg);//mBackground);
    } else {
        mBackgroundView = mContentView;
        // AOSP preparePopup with no background drawable installs NOTHING: the
        // window surface is transparent (the geometric Window ctor sets no
        // window background), so a setBackgroundDrawable(null) popup composes
        // its content directly over the anchor window. The old theme
        // colorBackground injection here overwrote the app-owned background ON
        // the content view (View::setBackground deletes the previous drawable)
        // — destroying transparent-popup setups and mutating borrowed content.
    }

    mDecorView = createDecorView(mBackgroundView);
    LOGD("%p createDecorView %p background=%p/%p",this,mDecorView,mBackground,mBackgroundView->getBackground());
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

PopupWindow::PopupDecorView::PopupDecorView(Context*ctx,int w,int h,int type)
   :Window(ctx,0,0,w,h,type, /*themeWindowAnimations=*/false){
    mPop = nullptr;
    // No theme windowAnimationStyle here (AOSP: that mechanism belongs to app/activity
    // windows; popup windows carry their own animation style on LayoutParams). Also,
    // popups are aligned to their anchor AFTER construction — a ctor-time enter snap
    // would capture a stale resting position and drag the popup to it.
}

PopupWindow::PopupDecorView* PopupWindow::createDecorView(View* contentView){
    ViewGroup::LayoutParams* layoutParams = mContentView->getLayoutParams();
    int height;
    if (layoutParams  && (layoutParams->height == LayoutParams::WRAP_CONTENT)) {
        height = LayoutParams::WRAP_CONTENT;
    } else {
        height = LayoutParams::MATCH_PARENT;
    }

    /* WindowManager sorts windows for compositing by window_type
     * (mLayer = window_type<<16|idx). PopupDecorView defaults to
     * TYPE_APPLICATION, which sorts BELOW an IMEWindow (TYPE_SYSTEM_WINDOW),
     * hiding the popup behind the keyboard. Pass mWindowLayoutType so callers
     * can raise the popup (e.g. KeyboardView sets TYPE_SYSTEM_ALERT). */
    const int wtype = mWindowLayoutType ? mWindowLayoutType : Window::TYPE_APPLICATION;
    PopupDecorView* decorView = new PopupDecorView(mContext,mWidth,mHeight,wtype);
    decorView->attachOwner(this);
    decorView->addView(contentView, LayoutParams::MATCH_PARENT, height);
    //decorView->setClipChildren(false);
    //decorView->setClipToPadding(false);
    return decorView;
}

void PopupWindow::updateAboveAnchor(bool aboveAnchor){
    // Only update if it's a change (AOSP: if (aboveAnchor != mAboveAnchor) {...}).
    if (aboveAnchor == mAboveAnchor)
        return ;
    mAboveAnchor = aboveAnchor;

    // AOSP re-evaluates params.windowAnimations when a dropdown flips sides
    // (Animation_DropDownUp vs _Down): the exit must slide the way the popup
    // actually sits after an update() moved it across the anchor. Re-resolve
    // onto the decor window — the enter leg is not re-armed (the popup's first
    // frame is long drawn; applyWindowAnimationStyle skips the snap there).
    if (mIsDropdown && mDecorView != nullptr) {
        const int animRes = computeAnimationResource();
        if (animRes != 0) {
            ((Window*)mDecorView)->setWindowAnimations(animRes, /*enableExit=*/true);
        }
    }

    if (mBackground && mBackgroundView ) {
        // If the background drawable provided was a StateListDrawable
        // with above-anchor and below-anchor states, use those.
        // Otherwise, rely on refreshDrawableState to do the job.
        if (mAboveAnchorBackgroundDrawable) {
            // The two anchors are children borrowed from the mBackground container;
            // setBackground takes ownership, so hand it a clone (preparePopup does
            // the same for the initial background). A child without ConstantState
            // support cannot be cloned — degrade to the state-refresh path.
            Drawable* bg = newDrawableOrNull(mAboveAnchor ? mAboveAnchorBackgroundDrawable
                                                          : mBelowAnchorBackgroundDrawable);
            if (bg != nullptr) {
                mBackgroundView->setBackground(bg);
            } else {
                mBackgroundView->refreshDrawableState();
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
    LOGD("invokePopup(%d,%d,%d,%d)",p->x,p->y,mDecorView->getWidth(),mDecorView->getHeight());
    mDecorView->setLayoutParams(p);
    // The computed window flags must land on the DECOR's WindowManager
    // attributes — the dispatcher's ACTION_OUTSIDE pass reads
    // w->getAttributes().flags (windowmanager.cc onMotion), not this View-level
    // LayoutParams. AOSP: the flags ride the very params handed to
    // WindowManager.addView; CDROID's Window IS the decor, so mirror them into
    // mWindowAttributes (this is what brings outside-touch dismissal to life).
    ((Window*)mDecorView)->setFlags(p->flags,
        WindowManager::LayoutParams::FLAG_NOT_FOCUSABLE
        | WindowManager::LayoutParams::FLAG_NOT_TOUCHABLE
        | WindowManager::LayoutParams::FLAG_WATCH_OUTSIDE_TOUCH
        | WindowManager::LayoutParams::FLAG_ALT_FOCUSABLE_IM
        | WindowManager::LayoutParams::FLAG_SPLIT_TOUCH
        | WindowManager::LayoutParams::FLAG_NOT_TOUCH_MODAL);
    // AOSP carries the animation style on params.windowAnimations and the WindowManager
    // starts the popup's enter/exit from it. CDROID resolves it onto the decor Window
    // HERE — after the anchor alignment above — so an enter snap captures the final
    // resting position (the theme windowAnimationStyle path cannot be used from the
    // ctor for exactly that reason; see PopupDecorView's note).
    // AOSP semantics: BOTH enter and exit install (AOSP always animates popup
    // exits). The no-GC discipline for the deferred teardown lives in dismiss()'s
    // exit branch: borrowed-content owners must setAdapter(nullptr) before freeing
    // (the menu chain does; see the dismiss() comment).
    p->windowAnimations = computeAnimationResource();
    if (p->windowAnimations != 0) {
        ((Window*)mDecorView)->setWindowAnimations(p->windowAnimations, /*enableExit=*/true);
    }
}

void PopupWindow::setLayoutDirectionFromAnchor() {
    if (mAnchor != nullptr) {
        View* anchor = mAnchor;//.get();
        if ((anchor != nullptr) && mPopupViewInitialLayoutDirectionInherited) {
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
    // AOSP PopupWindow.computeFlags (PopupWindow.java:1670-1690), trimmed to
    // the flag bits CDROID defines (FLAG_IGNORE_CHEEK_PRESSES and the
    // FLAG_LAYOUT_* family are layout-only bits not ported; mSplitTouchEnabled
    // rides SPLIT_TOUCH where set). This is what brings the popup decor's
    // ACTION_OUTSIDE branch to life for outside-touchable popups.
    using LP = WindowManager::LayoutParams;
    curFlags &= ~(LP::FLAG_NOT_FOCUSABLE | LP::FLAG_NOT_TOUCHABLE
                  | LP::FLAG_WATCH_OUTSIDE_TOUCH | LP::FLAG_ALT_FOCUSABLE_IM
                  | LP::FLAG_SPLIT_TOUCH | LP::FLAG_NOT_TOUCH_MODAL);
    if (!mFocusable) {
        curFlags |= LP::FLAG_NOT_FOCUSABLE;
        if (mInputMethodMode == INPUT_METHOD_NOT_NEEDED) {
            curFlags |= LP::FLAG_ALT_FOCUSABLE_IM;
        }
    }
    if (!mTouchable) {
        curFlags |= LP::FLAG_NOT_TOUCHABLE;
    }
    if (mOutsideTouchable) {
        curFlags |= LP::FLAG_WATCH_OUTSIDE_TOUCH;
    }
    if (mNotTouchModal) {
        curFlags |= LP::FLAG_NOT_TOUCH_MODAL;
    }
    if (mSplitTouchEnabled == 1) {   // explicit setSplitTouchEnabled(true) only;
                                     // the -1 "unset" sentinel must not read as true
        curFlags |= LP::FLAG_SPLIT_TOUCH;
    }
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

int PopupWindow::computeAnimationResource() {
    // AOSP PopupWindow.computeAnimationResource: an explicitly set style wins; dropdowns get
    // the framework default grow/shrink-fade pair, above vs below the anchor. CDROID treats
    // 0 (@empty — Material's popupMenuStyle chain) like ANIMATION_STYLE_DEFAULT: on AOSP the
    // empty handoff pairs with popupEnter/ExitTransition (L popup transitions), which CDROID
    // popups do not run, so falling back to the classic dropdown animation keeps menus
    // animating. An explicit NON-zero style (even an empty one like Animation.PopupWindow)
    // still wins verbatim.
    if (mAnimationStyle == ANIMATION_STYLE_DEFAULT || mAnimationStyle == 0) {
        if (mIsDropdown) {
            return mAboveAnchor ? (int)R::style::Animation_DropDownUp
                                : (int)R::style::Animation_DropDownDown;
        }
        return 0;
    }
    return mAnimationStyle;
}

bool PopupWindow::findDropDownPosition(View* anchor,WindowManager::LayoutParams* outParams,
       int xOffset, int yOffset, int width, int height, int gravity, bool allowScroll){
    const int anchorHeight = anchor->getHeight();
    const int anchorWidth = anchor->getWidth();
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
    const int hgrav = Gravity::getAbsoluteGravity(gravity, anchor->getLayoutDirection())
                & Gravity::HORIZONTAL_GRAVITY_MASK;
    if (hgrav == Gravity::RIGHT) {
        outParams->x -= width - anchorWidth;
    }

    // First, attempt to fit the popup vertically without resizing.
    const bool fitsVertical = tryFitVertical(outParams, yOffset, height,
            anchorHeight, drawingLocation[1], screenLocation[1], displayFrame.top,
            displayFrame.bottom(), false);

    // Next, attempt to fit the popup horizontally without resizing.
    const bool fitsHorizontal = tryFitHorizontal(outParams, xOffset, width,
            anchorWidth, drawingLocation[0], screenLocation[0], displayFrame.left,
            displayFrame.right(), false);

    // If the popup still doesn't fit, attempt to scroll the parent.
    if (!fitsVertical || !fitsHorizontal) {
        const int scrollX = anchor->getScrollX();
        const int scrollY = anchor->getScrollY();
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
    // AOSP PopupWindow.dismiss(): synchronous end to end —
    // WindowManagerGlobal.removeViewImmediate(decor) -> ViewRootImpl.die(true)
    // tears the window down NOW; any exit animation plays on a compositor
    // ghost snapshot (the WMS surface-outlives-view analog — see
    // Window::close), so nothing here is deferred and there is no re-show
    // window to race, no generation to track, no mid-flight state.
    // The listener fires from a stack copy: the owner may DELETE this popup
    // inside it (delete-at-any-time), which would destroy the member
    // std::function while it is still executing. The member itself is NOT
    // cleared — AOSP never clears it; a reused popup keeps notifying.
    if (!isShowing() || (mDecorView == nullptr)) {   // AOSP's guards
        return;
    }
    PopupDecorView* decorView = mDecorView;
    View* contentView = mContentView;
    ViewGroup* contentHolder = (ViewGroup*) contentView->getParent();

    mIsShowing = false;

    dismissImmediate(decorView, contentHolder, contentView);

    // Clears the anchor view.
    detachFromAnchor();

    OnDismissListener onDismissListener = mOnDismissListener;
    if (onDismissListener != nullptr) {
        onDismissListener();
    }
}

Rect PopupWindow::getTransitionEpicenter(){
    if ((mAnchor == nullptr) || (mDecorView == nullptr)) {
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
        const int offsetX = bounds.left;
        const int offsetY = bounds.top;
        bounds = mEpicenterBounds;
        bounds.offset(offsetX, offsetY);
    }
    return bounds;
}

void PopupWindow::dismissImmediate(View* decorView, ViewGroup* contentHolder, View* contentView){
    // AOSP dismissImmediate: removeViewImmediate(decorView) FIRST — the window
    // teardown is synchronous and any exit animation plays on a ghost SNAPSHOT
    // taken while the content is still in the tree — then hand the borrowed
    // content back to its owner for reuse (owned content dies with the decor's
    // teardown cascade).
    ((PopupDecorView*)decorView)->detachOwner();   // neutralize the back-pointer:
                                // the decor lives until its posted delete, and the
                                // popup may be deleted inside the dismiss listener
                                // before that fires
    ((Window*)decorView)->close();
    LOGD("%p close mDecorView %p which its contentView=%p",this,decorView,contentView);
    if (!mOwnsContentView && (contentHolder != nullptr)) {
        contentHolder->removeView(contentView);
    }
    mDecorView = nullptr;
    mBackgroundView = nullptr;
}

void PopupWindow::setOnDismissListener(const OnDismissListener& onDismissListener) {
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

    const int newFlags = computeFlags(p->flags);
    if (newFlags != p->flags) {
        p->flags = newFlags;
        // Keep the decor's window attributes in step (see invokePopup): the
        // dispatcher reads getAttributes().flags, not the View LayoutParams.
        ((Window*)mDecorView)->setFlags(newFlags,
            WindowManager::LayoutParams::FLAG_NOT_FOCUSABLE
            | WindowManager::LayoutParams::FLAG_NOT_TOUCHABLE
            | WindowManager::LayoutParams::FLAG_WATCH_OUTSIDE_TOUCH
            | WindowManager::LayoutParams::FLAG_ALT_FOCUSABLE_IM
            | WindowManager::LayoutParams::FLAG_SPLIT_TOUCH
            | WindowManager::LayoutParams::FLAG_NOT_TOUCH_MODAL);
        bUpdate = true;
    }

    const int newGravity = computeGravity();
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
    if (mDecorView) {
        // Use mWidth/mHeight (actual pixel size from setWidth/setHeight), NOT params->width/height
        // (which is WRAP_CONTENT=-2 when mWidthMode<0). CDROID's layout() takes (l, t, W, H).
        WindowManager::getInstance().moveWindow(mDecorView, params->x, params->y,mWidth,mHeight);
    }
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
    const int finalWidth = mWidthMode < 0 ? mWidthMode : mLastWidth;
    if ((width != -1) && (p->width != finalWidth)) {
        p->width = mLastWidth = finalWidth;
        updated = true;
    }

    const int finalHeight = (mHeightMode < 0) ? mHeightMode : mLastHeight;
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
    const int gravity = mAnchoredGravity;

    const bool needsUpdate = updateLocation && (mAnchorXoff != xoff || mAnchorYoff != yoff);
    if ((oldAnchor == nullptr) || (oldAnchor != anchor) || (needsUpdate && !mIsDropdown)) {
        attachToAnchor(anchor, xoff, yoff, gravity);
    } else if (needsUpdate) {
        // No need to register again if this is a DropDown, showAsDropDown already did.
        mAnchorXoff = xoff;
        mAnchorYoff = yoff;
    }

    WindowManager::LayoutParams* p = getDecorViewLayoutParams();
    const int oldGravity = p->gravity;
    const int oldWidth = p->width;
    const int oldHeight = p->height;
    const int oldX = p->x;
    const int oldY = p->y;

    // If an explicit width/height has not specified, use the most recent
    // explicitly specified value (either from setWidth/Height or update).
    if (width < 0) {
        width = mWidth;
    }
    if (height < 0) {
        height = mHeight;
    }

    const bool aboveAnchor = findDropDownPosition(anchor, p, mAnchorXoff, mAnchorYoff,
                width, height, gravity, mAllowScrollingAnchorParent);
    updateAboveAnchor(aboveAnchor);

    const bool paramsChanged = (oldGravity != p->gravity) || (oldX != p->x) || (oldY != p->y)
                || (oldWidth != p->width) || (oldHeight != p->height);

    // If width and mWidth were both < 0 then we have a MATCH_PARENT or
    // WRAP_CONTENT case. findDropDownPosition will have resolved this to
    // absolute values, but we don't want to update mWidth/mHeight to these
    // absolute values.
    const int newWidth = width < 0 ? width : p->width;
    const int newHeight = height < 0 ? height : p->height;
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
bool PopupWindow::PopupDecorView::dispatchKeyEvent(KeyEvent& event){
     if (event.getKeyCode() == KeyEvent::KEYCODE_BACK) {
        if (getKeyDispatcherState() == nullptr) {
            return Window::dispatchKeyEvent(event);
        }

        if ((event.getAction() == KeyEvent::ACTION_DOWN) && (event.getRepeatCount() == 0)) {
            KeyEvent::DispatcherState* state = getKeyDispatcherState();
            if (state ) {
                state->startTracking(event, this);
            }
            return true;
        } else if (event.getAction() == KeyEvent::ACTION_UP) {
            KeyEvent::DispatcherState* state = getKeyDispatcherState();
            if (state && state->isTracking(event) && !event.isCanceled()) {
                // mPop may be detached (owner destroyed; our delete is posted).
                if (mPop != nullptr) mPop->dismiss();
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
        // mPop may be detached (owner destroyed; our delete is posted).
        if (mPop != nullptr) mPop->dismiss();
        return true;
    } else if (event.getAction() == MotionEvent::ACTION_OUTSIDE) {
        if (mPop != nullptr) mPop->dismiss();
        return true;
    } else {
        return Window::onTouchEvent(event);
    }
}

PopupWindow::PopupBackgroundView::PopupBackgroundView(Context* context)
:FrameLayout(context){
}
}
