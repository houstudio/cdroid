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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02101-1301  USA
 *********************************************************************************/
#ifndef __FLOATING_ACTION_MODE_H__
#define __FLOATING_ACTION_MODE_H__
#include <view/actionmode.h>
namespace cdroid{
class Context;       // core/context.h
class View;          // view/view.h (actionmode.h 仅前向声明)
class MenuBuilder;
class MenuPopupHelper;
class MenuInflater;

/**
 * 浮窗/popup 式 ActionMode (CDROID 无 toolbar/action-bar 时用), 对标 androidx FloatingActionMode。
 * 用 MenuBuilder + MenuPopupHelper 把 ActionMode.Callback 的 4 个事件函数真正接通:
 *   show() → onCreateActionMode/onPrepareActionMode → 弹菜单;
 *   点菜单项 → onActionItemClicked;
 *   finish()/popup 外部 dismiss → onDestroyActionMode。
 * Menu/MenuItem 由本类保证非空, 经引用传入回调 (对齐 Android @NonNull)。
 * 回调 std::function 未 set 时跳过 (空检查)。
 */
class FloatingActionMode : public ActionMode {
public:
    FloatingActionMode(Context* context, View* anchor, const ActionMode::Callback& callback);
    ~FloatingActionMode() override;

    /** 调 onCreateActionMode/onPrepareActionMode 并弹出菜单; onCreateActionMode 返回 false → 中止, 返 false。 */
    bool show();

    // —— ActionMode 纯虚实现 ——
    void setTitle(const std::string& title) override;
    void setSubtitle(const std::string& subtitle) override;
    void setCustomView(View* view) override;
    void invalidate() override;
    void finish() override;
    std::string getTitle() override;
    std::string getSubtitle() override;
    View* getCustomView() override;
    Menu* getMenu() override;
    MenuInflater* getMenuInflater() override;
private:
    void onDestroy();   // popup dismiss 回调 → onDestroyActionMode
    Context* mContext;
    View* mAnchor;
    ActionMode::Callback mCallback;
    MenuBuilder* mMenu;
    MenuPopupHelper* mPopup;
    MenuInflater* mInflater;
    std::string mTitle;
    std::string mSubtitle;
    View* mCustomView = nullptr;
    bool mFinished = false;
};
}//namespace
#endif/*__FLOATING_ACTION_MODE_H__*/
