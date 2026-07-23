/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * Line-by-line port of AOSP android-36 com.android.internal.view.FloatingActionMode.
 * Renders via FloatingToolbar (show/dismiss/updateLayout). Click handling routes the
 * toolbar's menu-item click to ActionMode.Callback.onActionItemClicked.
 *********************************************************************************/
#include <view/floatingactionmode.h>
#include <widget/floatingtoolbar.h>
#include <widget/cdwindow.h>
#include <menu/menubuilder.h>
#include <menu/menuinflater.h>
#include <menu/menuitem.h>
#include <core/rect.h>
#include <porting/cdlog.h>
namespace cdroid{

FloatingActionMode::FloatingActionMode(Context* context, const ActionMode::Callback& callback,
        View* originatingView)
    : mContext(context)
    , mOriginatingView(originatingView)
    , mCallback(callback)
    , mMenu(nullptr) {
    mMenu = new MenuBuilder(context);

    // The toolbar is created from the root view (which is the Window in CDROID's flat model).
    // AOSP passes a FloatingToolbar in from DecorView; CDROID's DecorView analog is the Window.
    Window* window = dynamic_cast<Window*>(originatingView->getRootView());
    mFloatingToolbar = new FloatingToolbar(window);

    // Route a menu-item click (from the toolbar) to ActionMode.Callback.onActionItemClicked.
    mFloatingToolbar->setOnMenuItemClickListener(
            [this](MenuItem& item) -> bool {
                if (mCallback.onActionItemClicked)
                    return mCallback.onActionItemClicked(*this, item);
                return false;
            });
}

FloatingActionMode::~FloatingActionMode() {
    delete mFloatingToolbar;
    delete mMenu;
    delete mInflater;
}

bool FloatingActionMode::show() {
    LOGD("FloatingActionMode::show menu=%p size=%d", mMenu, mMenu->size());
    if (mCallback.onCreateActionMode && !mCallback.onCreateActionMode(*this, *mMenu)) {
        LOGD("FloatingActionMode::show onCreateActionMode returned false, abort");
        return false;
    }
    mFloatingToolbar->setMenu(mMenu);
    invalidateContentRect();   // sets the content rect from the originating view
    if (mCallback.onPrepareActionMode)
        mCallback.onPrepareActionMode(*this, *mMenu);
    mFloatingToolbar->show();
    return true;
}

void FloatingActionMode::setTitle(const std::string& title){ mTitle = title; }
void FloatingActionMode::setSubtitle(const std::string& subtitle){ mSubtitle = subtitle; }
void FloatingActionMode::setCustomView(View* view){ mCustomView = view; }
std::string FloatingActionMode::getTitle(){ return mTitle; }
std::string FloatingActionMode::getSubtitle(){ return mSubtitle; }
View* FloatingActionMode::getCustomView(){ return mCustomView; }

void FloatingActionMode::invalidate() {
    if (mCallback.onPrepareActionMode)
        mCallback.onPrepareActionMode(*this, *mMenu);
    invalidateContentRect();
    mFloatingToolbar->updateLayout();
}

void FloatingActionMode::finish() {
    if (mFinished) return;
    mFinished = true;
    mFloatingToolbar->dismiss();
    onDestroy();
}

Menu* FloatingActionMode::getMenu(){ return mMenu; }

MenuInflater* FloatingActionMode::getMenuInflater(){
    if (mInflater == nullptr) mInflater = new MenuInflater(mContext);
    return mInflater;
}

void FloatingActionMode::invalidateContentRect() {
    Rect contentRect;
    if (mCallback.onGetContentRect) {
        // Caller (e.g. Editor) supplies the precise content rect (selection rect).
        // Mirrors AOSP Callback2.onGetContentRect.
        mCallback.onGetContentRect(*this,*mOriginatingView,contentRect);
    } else {
        // Fallback: the originating view's on-screen bounds (AOSP default).
        int pos[2] = {0, 0};
        mOriginatingView->getLocationOnScreen(pos);
        contentRect.set(pos[0], pos[1], mOriginatingView->getWidth(), mOriginatingView->getHeight());
    }
    mFloatingToolbar->setContentRect(contentRect);
}

void FloatingActionMode::onDestroy(){
    if (mCallback.onDestroyActionMode) {
        mCallback.onDestroyActionMode(*this);
    }
    if (mOnFinished) {
        mOnFinished();
    }
}

}//namespace
