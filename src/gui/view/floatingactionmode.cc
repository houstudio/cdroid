/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * 浮窗/popup 式 ActionMode 实现: 接通 ActionMode.Callback 的 4 个事件函数。
 * 复用 MenuBuilder (点击链路) + MenuPopupHelper (弹窗), 范式同 PopupMenu。
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

    // 菜单项点击 → onActionItemClicked (MenuBuilder 的点击链路: popup→performItemAction→invoke→dispatchMenuItemSelected→这里)
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
    mPopup->setOnDismissListener([this]{ onDestroy(); });
}

FloatingActionMode::~FloatingActionMode(){
    delete mPopup;     // 内含 dismiss (若仍在显示), 其 dtor 清理 popup 窗口/presenter
    delete mMenu;
    delete mInflater;
}

bool FloatingActionMode::show(){
    LOGD("FloatingActionMode::show menu=%p size=%d", mMenu, mMenu->size());
    // onCreateActionMode: app 可返回 false 中止
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
    // onPrepareActionMode 让 app 刷新菜单内容 (核心接通); popup 视觉刷新为后续优化 (菜单项原地改, adapter 暂不强制刷新)
    if(mCallback.onPrepareActionMode)
        mCallback.onPrepareActionMode(*this, *mMenu);
}

void FloatingActionMode::finish(){
    if(mFinished) return;
    mFinished = true;
    mPopup->dismiss();   // → onDismiss 监听 → onDestroy → onDestroyActionMode
}

Menu* FloatingActionMode::getMenu(){ return mMenu; }

MenuInflater* FloatingActionMode::getMenuInflater(){
    if(mInflater == nullptr) mInflater = new MenuInflater(mContext);
    return mInflater;
}

void FloatingActionMode::onDestroy(){
    // popup 被关闭 (finish 主动, 或外部点击 dismiss) → 通知 app 销毁。
    if(mCallback.onDestroyActionMode)
        mCallback.onDestroyActionMode(*this);
}

}//namespace
