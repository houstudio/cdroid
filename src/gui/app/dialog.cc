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
#include <app/dialog.h>
#include <core/windowmanager.h>
#include <core/looper.h>
#include <core/messagequeue.h>
#include <view/gravity.h>
#include <content/contextthemewrapper.h>
#include <widget/internal_R.h>
namespace cdroid{
using namespace cdroid::internal;

Dialog::Dialog(Context*context):Dialog(context,0,true){
}

// AOSP Dialog(Context, int themeResId, boolean createContextThemeWrapper):
// themeResId == 0 resolves ?attr/dialogTheme from the caller's theme, then the
// context is wrapped in a ContextThemeWrapper (owned by this Dialog). The
// window inflates with the themed context (AOSP new PhoneWindow(mContext)).
Dialog::Dialog(Context* context,int themeResId,bool createContextThemeWrapper){
    if (createContextThemeWrapper) {
        if (themeResId == 0) {
            TypedValue outValue;
            context->getTheme().resolveAttribute(R::attr::dialogTheme, &outValue, true);
            themeResId = outValue.resourceId;
        }
        mContext = new ContextThemeWrapper(context, themeResId);
        mOwnsContext = true;
    } else {
        mContext = context;
        mOwnsContext = false;
    }
    mCreated = false;
    mShowing = false;
    mCancelable = true;
    mWindow = new Window(mContext, 0, 0, 640, 320);
    // AOSP Dialog ctor: mWindow.setCallback(this) — the Dialog receives the
    // window's input dispatch + lifecycle through the Window.Callback seam
    // (cleared in dismissDialog: the window teardown is posted and may outlive
    // the Dialog).
    mWindow->setCallback(this);
}

Dialog::~Dialog(){
    if(mWindow){
        WindowManager::getInstance().removeWindow(mWindow);
    } else if (mDismissedWindow && Looper::getMainLooper()->getQueue()->isQuitting()) {
        // Dismissed earlier: the window's teardown was handed to close()'s exit
        // animation + posted deletes. At quit those posts are dropped by the
        // dying looper, so the shell stays alive but the tree may never be
        // detached — remove it here (idempotent, membership-checked). Runtime
        // never consults the stash: the post may have freed the window and the
        // address reused by an unrelated one.
        WindowManager::getInstance().removeWindow(mDismissedWindow);
    }
    if(mOwnsContext){
        delete mContext;
    }
}

Context*Dialog::getContext()const{
    return mContext;
}

bool Dialog::isShowing()const{
    return mShowing;
}

void Dialog::create(){
    if(!mCreated)dispatchOnCreate(nullptr);
}

void Dialog::show(){
    if(mShowing){
        // AOSP Dialog.show on a shown dialog: it may be hidden (hide()) — make
        // the window visible again and nothing else.
        if (mWindow != nullptr) mWindow->setVisibility(View::VISIBLE);
        return;
    }
    mCanceled = false;
    if(!mCreated) dispatchOnCreate(nullptr);
    onStart();

    // Theme backdrop (AOSP DecorView role): the dialog window surface is
    // opaque black; when the content supplied no background, paint the live
    // theme's colorBackground on the frame so unpainted areas follow the
    // theme (Light shows light — dialogTheme's ThemeOverlay resolves
    // colorBackground per host theme) instead of raw black.
    {
        ViewGroup* frame = (ViewGroup*)mWindow->getChildAt(0);
        if (frame != nullptr && frame->getBackground() == nullptr) {
            // Theme.resolveAttribute flattens a color *reference* to its pool
            // value (TypedValue.data with resourceId=0) — as an ARGB int that
            // is effectively transparent, so the frame never wipes the dialog
            // surface between frames and animating content (e.g. the radial
            // time picker's crossfade) accumulates on the retained surface.
            // Resolve through obtainStyledAttributes instead — the same
            // engine path widgets use, which chases the reference.
            const uint32_t attrs[] = { (uint32_t) internal::R::attr::colorBackground, 0 };
            auto ta = mContext->obtainStyledAttributes(attrs);
            if (ta != nullptr) {
                const int bgColor = (int) ta->getColor(0, 0xFF000000u);
                frame->setBackground(new ColorDrawable(bgColor));
            }
        }
    }

    ViewGroup*frm=(ViewGroup*)mWindow->getChildAt(0);
    MarginLayoutParams*lp=(MarginLayoutParams*)frm->getLayoutParams();
    const int horzMargin = lp->leftMargin+ lp->rightMargin;
    const int vertMargin = lp->topMargin + lp->bottomMargin;
    Point pt;
    WindowManager::getInstance().getDefaultDisplay().getSize(pt);
    LOGD("size=%dx%d margin=%d,%d",pt.x,pt.y,horzMargin,vertMargin);
    int widthSpec  = MeasureSpec::makeMeasureSpec(pt.x-horzMargin,MeasureSpec::EXACTLY);
    int heightSpec = MeasureSpec::makeMeasureSpec(pt.y-vertMargin,MeasureSpec::AT_MOST);

    widthSpec  = frm->getChildMeasureSpec(widthSpec ,0,lp->width);
    heightSpec = frm->getChildMeasureSpec(heightSpec,0,lp->height);
    frm->measure(widthSpec,heightSpec);
    LOGD("spec=%x/%x measured=%dx%d",widthSpec,heightSpec,frm->getMeasuredWidth(),frm->getMeasuredHeight());

    // AOSP: Dialog never places its own window. ViewRootImpl hands WMS the
    // measured wrap-content size via WindowManager.LayoutParams and WMS's
    // applyGravityAndUpdateFrame places the frame — Gravity::apply with the
    // default gravity (NO_GRAVITY, the LayoutParams default) centers each axis,
    // which is what centers dialogs on Android. gravity is NOT touched here:
    // hosts may stamp window attributes before show (AOSP Dialog.show never
    // writes gravity either).
    WindowManager::LayoutParams& attrs = mWindow->getAttributes();
    /* The themed windowBackground insets the decor content (the traversal
     * lays the panel at its padding — a 16px themed dialog background on a
     * 206px window leaves 174px for title+list+buttons, squeezing the list
     * to ~1.x rows). AOSP ViewRootImpl sizes a wrap-content window INCLUDING
     * the decor background padding; the manual measure here must do the
     * same or the window is short by 2x the padding. */
    int bgPadH = 0, bgPadV = 0;
    Rect bgPad;
    if (mWindow->getBackground() != nullptr && mWindow->getBackground()->getPadding(bgPad)) {
        /* Drawable::getPadding's Rect convention: the right/bottom insets are
         * stored in the .width/.height slots (View::resolvePadding reads
         * padding.width as the AOSP padding.right) — .right() would be
         * left+width and double-count the left inset. */
        bgPadH = bgPad.left + bgPad.width;
        bgPadV = bgPad.top + bgPad.height;
    }
    attrs.width  = frm->getMeasuredWidth()  + horzMargin + bgPadH;
    attrs.height = frm->getMeasuredHeight() + vertMargin + bgPadV;
    WindowManager::getInstance().relayoutWindow(mWindow);

    LOGD("size=%dx%d %d,%d",frm->getMeasuredWidth(),frm->getMeasuredHeight(),mWindow->getWidth(),mWindow->getHeight());
    frm->layout(lp->leftMargin,lp->topMargin,mWindow->getWidth()-horzMargin, mWindow->getHeight()-vertMargin);
    mShowing = true;
}

void Dialog::hide(){
    // AOSP Dialog.hide: mDecor.setVisibility(View.INVISIBLE) — the window stays
    // alive, state retained, cheap re-show via show(). Window-level INVISIBLE
    // routes hideWindow (drops the frame from the compositor and repaints what
    // was under it); nothing is torn down.
    if (mWindow != nullptr && mWindow->getVisibility() == View::VISIBLE) {
        mWindow->setVisibility(View::INVISIBLE);
    }
}

void Dialog::dismiss(){
    // Guard against re-entrant dismiss (mirrors Android Dialog.dismiss: a dismissed dialog
    // no-ops). Without this, onDismiss -> (callback) -> dismiss -> onDismiss recurses
    // (e.g. MenuDialogHelper.onDismiss -> presenter.onCloseMenu -> MenuDialogHelper.onCloseMenu
    // -> dismiss). dismissDialog() sets mShowing=false before firing onDismiss.
    if (!mShowing) return;
    dismissDialog();
}

void Dialog::dismissDialog(){
    onStop();
    mShowing = false;
    if(mOnDismissListener){
        mOnDismissListener(*this);
    }
    if(mWindow){
        mWindow->setCallback(nullptr);  // drop the `this` back-pointer before the posted teardown
        // NO setVisibility(INVISIBLE) here: Window::close gates the themed ghost
        // exit on getVisibility()==VISIBLE, and INVISIBLE would additionally route
        // onVisibilityChanged->hideWindow, erasing the frame at once — the dialog
        // would vanish with a hard cut every time despite the theme's
        // Animation.Dialog exit (AOSP dismiss goes WMS remove + exit animation).
        // close() tears the view tree down synchronously and hands the pixels to
        // the compositor ghost; nothing lingers that INVISIBLE used to hide.
        mWindow->close();          // proper window lifecycle cleanup (posts remove + onDestroy)
        mDismissedWindow = mWindow; // keep the arbitration handle (see dialog.h)
        mWindow = nullptr;         // idempotent: prevent double-close crash on re-entry
    }
}

void Dialog::dispatchOnCreate(void*buddle){
    if (!mCreated) {
        onCreate();
        mCreated = true;
    }
}

void Dialog::onCreate(){
}

void Dialog::onStart(){
}

void Dialog::onStop(){
}

void Dialog::setCancelable(bool flag){
    mCancelable = flag;
    //updateWindowForCancelable();
}

void Dialog::setCanceledOnTouchOutside(bool cancel) {
    // AOSP Dialog.setCanceledOnTouchOutside (Dialog.java:1338-1344).
    if (cancel && !mCancelable) {
        mCancelable = true;
    }
    if (mWindow == nullptr) return;
    mWindow->setCloseOnTouchOutside(cancel);
    /* CDROID stage-1 substitution: AOSP dialog windows are touch-modal, so an
       out-of-frame tap is delivered as the real gesture and consumed by
       shouldCloseOnTouch's UP-out-of-bounds clause. CDROID dispatch is
       topmost-hit only (non-modal), so the dialog window instead opts into
       the OUTSIDE notification — WindowManager synthesizes ACTION_OUTSIDE for
       a DOWN outside it, and shouldCloseOnTouch's ACTION_OUTSIDE clause fires.
       Same consumption point, same Dialog.cancel() dismissal. */
    mWindow->setFlags(cancel ? WindowManager::LayoutParams::FLAG_WATCH_OUTSIDE_TOUCH : 0,
                      WindowManager::LayoutParams::FLAG_WATCH_OUTSIDE_TOUCH);
}

bool Dialog::onTouchEvent(MotionEvent& event) {
    // AOSP Dialog.onTouchEvent (Dialog.java:802-807).
    if (mCancelable && mShowing && mWindow
            && mWindow->shouldCloseOnTouch(mContext, event)) {
        cancel();
        return true;
    }
    return false;
}

void Dialog::cancel(){
    if (!mCanceled && mOnCancelListener) {
        mCanceled = true;
        // Obtain a new message so this dialog can be re-used
        //Message.obtain(mCancelMessage).sendToTarget();
        mOnCancelListener(*this);
    }
    dismiss();
}

void Dialog::setOnCancelListener(OnCancelListener listener){
    mOnCancelListener = listener;
}

void Dialog::setOnDismissListener(OnDismissListener listener){
    mOnDismissListener = listener;
}

void Dialog::setOnShowListener(OnShowListener listener){
    mOnShowListener = listener;
}

void Dialog::setOnKeyListener(OnKeyListener listener){
    mOnKeyListener = listener;
}

Window* Dialog::getWindow()const{
    return mWindow;
}

View* Dialog::getCurrentFocus(){
    return nullptr;
}

View* Dialog::findViewById(int id){
    return mWindow->findViewById(id);
}

void Dialog::setContentView(int layoutResId){
    View*v=LayoutInflater::from(mContext)->inflate(layoutResId,nullptr,false);
    mWindow->addView(v);
}

void Dialog::setContentView(View*view){
    mWindow->addView(view);
}

void Dialog::addContentView(View* view,ViewGroup::LayoutParams* params){
    mWindow->addView(view);
}

void Dialog::setTitle(const std::string&title){
    mWindow->setText(title);
}

/* Dialog.java:635-637 -- setTitle(@StringRes int) */
void Dialog::setTitle(int titleId){
    setTitle(mContext->getString(titleId));
}

bool Dialog::onKeyDown(int keyCode,KeyEvent& event){
    // AOSP Dialog.onKeyDown (Dialog.java:1163-1169): track BACK so the UP side
    // can cancel. Without startTracking, onKeyUp's isTracking() is always false
    // and BACK-to-cancel never fires. ESC additionally stands in for BACK on
    // desktop keylayouts (same x64 convenience Window::onKeyDown carries — no
    // key produces KEYCODE_BACK there).
    if (keyCode == KeyEvent::KEYCODE_BACK || keyCode == KeyEvent::KEYCODE_ESCAPE) {
        event.startTracking();
        return true;
    }
    return false;
}

bool Dialog::onKeyLongPress(int keyCode,KeyEvent& event){
    return false;
}

bool Dialog::onKeyUp(int keyCode,KeyEvent& event){
    // ESC pairs with the DOWN-side tracking above (desktop BACK stand-in).
    if ((keyCode == KeyEvent::KEYCODE_BACK || keyCode == KeyEvent::KEYCODE_ESCAPE)
            && event.isTracking() && !event.isCanceled()) {
        onBackPressed();
        return true;
    }
    return false;
}

bool Dialog::onKeyMultiple(int keyCode, int repeatCount,KeyEvent& event){
    return false;
}

void Dialog::onBackPressed(){
    if (mCancelable) {
        cancel();
    }
}

/*================ WindowCallback graft (AOSP Dialog.java:832-1005) ================*/

bool Dialog::dispatchKeyEvent(KeyEvent& event){
    // AOSP Dialog.dispatchKeyEvent (Dialog.java:832-841). The onKeyListener is
    // CDROID's void-flavored DialogInterface event (AOSP's returns boolean and
    // can consume) — notify-only here, no consumption branch (recorded
    // deviation; the listener still observes every key first).
    if (mOnKeyListener) {
        mOnKeyListener(*this, event.getKeyCode(), event);
    }
    if (mWindow->superDispatchKeyEvent(event)) {
        return true;
    }
    return event.dispatch(this, mWindow->getKeyDispatcherState(), this);
}

bool Dialog::dispatchKeyShortcutEvent(KeyEvent& event){
    // AOSP Dialog.dispatchKeyShortcutEvent (Dialog.java:851-857).
    if (mWindow->superDispatchKeyShortcutEvent(event)) {
        return true;
    }
    return onKeyShortcut(event.getKeyCode(), event);
}

bool Dialog::dispatchTouchEvent(MotionEvent& event){
    // AOSP Dialog.dispatchTouchEvent (Dialog.java:871-875): the decor tree
    // first, the Dialog's own onTouchEvent (outside-touch cancel) last.
    if (mWindow->superDispatchTouchEvent(event)) {
        return true;
    }
    return onTouchEvent(event);
}

bool Dialog::dispatchTrackballEvent(MotionEvent& event){
    // AOSP Dialog.dispatchTrackballEvent (Dialog.java:889-893); CDROID has no
    // trackball source, so the super pass is the whole story in practice.
    if (mWindow->superDispatchTrackballEvent(event)) {
        return true;
    }
    return onTrackballEvent(event);
}

bool Dialog::dispatchGenericMotionEvent(MotionEvent& event){
    // AOSP Dialog.dispatchGenericMotionEvent (Dialog.java:907-911).
    if (mWindow->superDispatchGenericMotionEvent(event)) {
        return true;
    }
    return onGenericMotionEvent(event);
}

bool Dialog::dispatchPopulateAccessibilityEvent(AccessibilityEvent& event){
    // AOSP Dialog.dispatchPopulateAccessibilityEvent (Dialog.java:918-921)
    // stamps the concrete class name; CDROID's AccessibilityEvent has no
    // className field yet — shape only.
    (void)event;
    return false;
}

View* Dialog::onCreatePanelView(int featureId){
    // AOSP Dialog.onCreatePanelView (Dialog.java:931-933).
    (void)featureId;
    return nullptr;
}

bool Dialog::onCreatePanelMenu(int featureId, Menu& menu){
    // AOSP Dialog.onCreatePanelMenu (Dialog.java:939-945).
    if (featureId == Window::FEATURE_OPTIONS_PANEL) {
        return onCreateOptionsMenu(menu);
    }
    return false;
}

bool Dialog::onPreparePanel(int featureId, View* view, Menu& menu){
    // AOSP Dialog.onPreparePanel (Dialog.java:951-957).
    if (featureId == Window::FEATURE_OPTIONS_PANEL) {
        return onPrepareOptionsMenu(menu) && menu.hasVisibleItems();
    }
    return true;
}

bool Dialog::onMenuOpened(int featureId, Menu& menu){
    // AOSP Dialog.onMenuOpened (Dialog.java:962-967) also notifies an
    // ActionBar; dialogs carry none.
    (void)featureId; (void)menu;
    return true;
}

bool Dialog::onMenuItemSelected(int featureId, MenuItem& item){
    // AOSP Dialog.onMenuItemSelected (Dialog.java:973-976).
    (void)featureId; (void)item;
    return false;
}

void Dialog::onPanelClosed(int featureId, Menu& menu){
    // AOSP Dialog.onPanelClosed (Dialog.java:981-986) — ActionBar visibility
    // only; nothing to do without one.
    (void)featureId; (void)menu;
}

void Dialog::onWindowDismissed(bool finishTask, bool suppressWindowTransition){
    // AOSP Dialog.onWindowDismissed (Dialog.java:818-820).
    (void)finishTask; (void)suppressWindowTransition;
    dismiss();
}

bool Dialog::onKeyShortcut(int keyCode, KeyEvent& event){
    // AOSP Dialog.onKeyShortcut (Dialog.java:1049-1051).
    (void)keyCode; (void)event;
    return false;
}

bool Dialog::onTrackballEvent(MotionEvent& event){
    // AOSP Dialog.onTrackballEvent (Dialog.java:1106-1108).
    (void)event;
    return false;
}

bool Dialog::onGenericMotionEvent(MotionEvent& event){
    // AOSP Dialog.onGenericMotionEvent (Dialog.java:1113-1115).
    (void)event;
    return false;
}

bool Dialog::onCreateOptionsMenu(Menu& menu){
    // AOSP Dialog.onCreateOptionsMenu (Dialog.java:991-996).
    (void)menu;
    return true;
}

bool Dialog::onPrepareOptionsMenu(Menu& menu){
    // AOSP Dialog.onPrepareOptionsMenu (Dialog.java:1001-1006).
    (void)menu;
    return true;
}

}//endof namespace
