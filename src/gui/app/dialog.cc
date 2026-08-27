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
}

Dialog::~Dialog(){
    if(mWindow){
        WindowManager::getInstance().removeWindow(mWindow);
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
            TypedValue bgValue;
            if (mContext->getTheme().resolveAttribute(
                    (int)internal::R::attr::colorBackground, &bgValue, true)) {
                int bgColor = bgValue.data;
                if (bgValue.resourceId != 0) bgColor = mContext->getColor((int)bgValue.resourceId);
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
    attrs.width  = frm->getMeasuredWidth()  + horzMargin;
    attrs.height = frm->getMeasuredHeight() + vertMargin;
    WindowManager::getInstance().relayoutWindow(mWindow);

    LOGD("size=%dx%d %d,%d",frm->getMeasuredWidth(),frm->getMeasuredHeight(),mWindow->getWidth(),mWindow->getHeight());
    frm->layout(lp->leftMargin,lp->topMargin,mWindow->getWidth()-horzMargin, mWindow->getHeight()-vertMargin);
    mShowing = true;
}

void Dialog::hide(){
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
        mWindow->setVisibility(View::INVISIBLE);
        mWindow->close();          // proper window lifecycle cleanup (posts remove + onDestroy)
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
    if (cancel && !mCancelable) {
        mCancelable = true;
        //updateWindowForCancelable();
    }
        
    //mWindow.setCloseOnTouchOutside(cancel);
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

bool Dialog::onKeyDown(int keyCode,KeyEvent& event){
    return false;
}

bool Dialog::onKeyLongPress(int keyCode,KeyEvent& event){
    return false;
}

bool Dialog::onKeyUp(int keyCode,KeyEvent& event){
    if (keyCode == KeyEvent::KEYCODE_BACK && event.isTracking() && !event.isCanceled()) {
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

}//endof namespace
