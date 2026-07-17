/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 *********************************************************************************/
#include <view/floatingactionmode.h>
#include <menu/menubuilder.h>
#include <menu/menupopuphelper.h>
#include <menu/menuinflater.h>
#include <porting/cdlog.h>
namespace cdroid{

FloatingActionMode::FloatingActionMode(Context* context, View* anchor, const ActionMode::Callback& callback)
    :mContext(context), mAnchor(anchor), mCallback(callback)
    ,mMenu(nullptr), mPopup(nullptr), mInflater(nullptr){
    mMenu = new MenuBuilder(context);

    MenuBuilder::Callback cbk;
    cbk.onMenuItemSelected = [this](MenuBuilder&, MenuItem& item){
        if(mCallback.onActionItemClicked)
            return mCallback.onActionItemClicked(*this, item);
        return false;
    };
    cbk.onMenuModeChange = [](MenuBuilder&){};
    mMenu->setCallback(cbk);

    mPopup = new MenuPopupHelper(context, mMenu, anchor);
    mPopup->setForceShowIcon(true);
    // 浮窗 ActionMode 覆盖内容 (对标 AOSP FloatingToolbar 浮于内容之上): 让 getMaxAvailableHeight
    // 从锚点顶部起算, 否则大锚点 (如整屏 ListView) 会算出极小可用高度, 导致 popup 被夹到几像素高。
    mPopup->setOverlapAnchor(true);
    mPopup->setOnDismissListener([this]{ onDestroy(); });
}

FloatingActionMode::~FloatingActionMode(){
    delete mPopup;
    delete mMenu;
    delete mInflater;
}

bool FloatingActionMode::show(){
    LOGD("FloatingActionMode::show menu=%p size=%d", mMenu, mMenu->size());
    if(mCallback.onCreateActionMode && !mCallback.onCreateActionMode(*this, *mMenu)){
        LOGD("FloatingActionMode::show onCreateActionMode returned false, abort");
        return false;
    }
    LOGD("FloatingActionMode::show after onCreate, menu size=%d", mMenu->size());
    if(mCallback.onPrepareActionMode)
        mCallback.onPrepareActionMode(*this, *mMenu);
    LOGD("FloatingActionMode::show -> mPopup->show()");
    mPopup->show();
    LOGD("FloatingActionMode::show mPopup->show() done, showing=%d", mPopup->isShowing());
    return true;
}

void FloatingActionMode::setTitle(const std::string& title){ mTitle = title; }
void FloatingActionMode::setSubtitle(const std::string& subtitle){ mSubtitle = subtitle; }
void FloatingActionMode::setCustomView(View* view){ mCustomView = view; }
std::string FloatingActionMode::getTitle(){ return mTitle; }
std::string FloatingActionMode::getSubtitle(){ return mSubtitle; }
View* FloatingActionMode::getCustomView(){ return mCustomView; }

void FloatingActionMode::invalidate(){
    if(mCallback.onPrepareActionMode)
        mCallback.onPrepareActionMode(*this, *mMenu);
}

void FloatingActionMode::finish(){
    if(mFinished) return;
    mFinished = true;
    mPopup->dismiss();
}

Menu* FloatingActionMode::getMenu(){ return mMenu; }

MenuInflater* FloatingActionMode::getMenuInflater(){
    if(mInflater == nullptr) mInflater = new MenuInflater(mContext);
    return mInflater;
}

void FloatingActionMode::onDestroy(){
    if(mCallback.onDestroyActionMode){
        mCallback.onDestroyActionMode(*this);
    }
    if(mOnFinished){
        mOnFinished();
    }
}

}//namespace
