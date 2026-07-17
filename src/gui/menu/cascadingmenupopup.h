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
#ifndef __CASCADING_MENU_POPUP_H__
#define __CASCADING_MENU_POPUP_H__
#include <widget/listview.h>
#include <widget/popupwindow.h>
#include <menu/menupopup.h>
#include <menu/menuitemhoverlistener.h>
namespace cdroid{
class MenuAdapter;
class MenuPopupWindow;
class CascadingMenuPopup:public MenuPopup{// implements MenuPresenter, OnKeyListener,PopupWindow.OnDismissListener {
private:
    static constexpr int HORIZ_POSITION_LEFT = 0;
    static constexpr int HORIZ_POSITION_RIGHT = 1;
    static constexpr int SUBMENU_TIMEOUT_MS = 200;
    class CascadingMenuInfo;
private:
    Context* mContext;
    int mMenuMaxWidth;
    std::string mPopupStyleAttr;
    std::string mPopupStyleRes;
    Handler* mSubMenuHoverHandler;
    std::vector<MenuBuilder*> mPendingMenus;
    std::vector<CascadingMenuInfo*> mShowingMenus;
    ViewTreeObserver::OnGlobalLayoutListener mGlobalLayoutListener;
    View::OnAttachStateChangeListener mAttachStateChangeListener;
    MenuItemHoverListener mMenuItemHoverListener;

    int mRawDropDownGravity = Gravity::NO_GRAVITY;
    int mDropDownGravity = Gravity::NO_GRAVITY;
    View* mAnchorView;
    View* mShownAnchorView;
    int mLastPosition;
    int mXOffset;
    int mYOffset;
    bool mOverflowOnly;
    bool mHasXOffset;
    bool mHasYOffset;
    bool mForceShowIcon;
    bool mShowTitle;
    Callback mPresenterCallback;
    ViewTreeObserver* mTreeObserver;
    PopupWindow::OnDismissListener mOnDismissListener;
    std::string mItemLayout;

    /** Whether popup menus should disable exit animations when closing. */
    bool mShouldCloseImmediately;
private:
    void onGlobalLayout();
    void onViewAttachedToWindow(View&);
    void onViewDetachedFromWindow(View&);
    void onItemHoverExit(MenuBuilder& menu,MenuItem& item);
    void onItemHoverEnter(MenuBuilder& menu,MenuItem& item);
    MenuPopupWindow* createPopupWindow();
    int getInitialMenuPosition()const;
    int getNextMenuPosition(int nextMenuWidth);
    void showMenu(MenuBuilder* menu);
    MenuItem* findMenuItemForSubmenu(MenuBuilder* parent, MenuBuilder* submenu);
    View* findParentViewForSubmenu(CascadingMenuInfo* parentInfo, MenuBuilder* submenu);
    int findIndexOfAddedMenu(MenuBuilder* menu);
public:
    CascadingMenuPopup(Context* context,View* anchor, const std::string& popupStyleAttr, const std::string& popupStyleRes, bool overflowOnly);
    ~CascadingMenuPopup()override;
    void setForceShowIcon(bool forceShow)override;

    void show() override;
    void dismiss() override;
    bool isShowing() override;
    void addMenu(MenuBuilder* menu)override;

    bool onKey(View& v, int keyCode, KeyEvent& event);
    void onDismiss();
    void updateMenuView(bool cleared)override;

    void setCallback(const Callback& cb) override;
    bool onSubMenuSelected(SubMenuBuilder* subMenu)override;
    void onCloseMenu(MenuBuilder* menu, bool allMenusAreClosing)override;

    bool flagActionItems()override;

    Parcelable* onSaveInstanceState()override;
    void onRestoreInstanceState(Parcelable& state)override;

    void setGravity(int dropDownGravity)override;
    void setAnchorView(View* anchor)override;

    void setOnDismissListener(const PopupWindow::OnDismissListener& listener)override;

    ListView* getListView()const override;

    void setHorizontalOffset(int x)override;
    void setVerticalOffset(int y)override;
    void setShowTitle(bool showTitle) override;
};
class CascadingMenuPopup::CascadingMenuInfo {
public:
    MenuPopupWindow* window;
    MenuBuilder* menu;
    int position;
public:
    CascadingMenuInfo(MenuPopupWindow* window,MenuBuilder* menu,int position);
    ~CascadingMenuInfo();
    ListView* getListView();
};
}/*endof namespace*/
#endif/*__CASCADING_MENU_POPUP_H__*/
