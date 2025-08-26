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
#ifndef __STANDARD_MENU_POPUP_H__
#define __STANDARD_MENU_POPUP_H__
#include <view/view.h>
#include <widget/listview.h>
#include <menu/menupopup.h>
namespace cdroid{
class MenuAdapter;
class MenuPopupWindow;
class StandardMenuPopup:public MenuPopup{//implements OnDismissListener, OnItemClickListener,MenuPresenter, OnKeyListener {
private:
    Context* mContext;
    MenuBuilder* mMenu;
    MenuAdapter* mAdapter;
    int mPopupMaxWidth;
    std::string mPopupStyleAttr;
    std::string mPopupStyleRes;

    MenuPopupWindow* mPopup;
    PopupWindow::OnDismissListener mOnDismissListener;
    View::OnAttachStateChangeListener mAttachStateChangeListener;
    ViewTreeObserver::OnGlobalLayoutListener mGlobalLayoutListener;
    View* mAnchorView;
    View* mShownAnchorView;
    Callback mPresenterCallback;
    ViewTreeObserver* mTreeObserver;

    /** Whether the popup has been dismissed. Once dismissed, it cannot be opened again. */
    bool mWasDismissed;
    /** Whether the cached content width value is valid. */
    bool mHasContentWidth;
    bool mShowTitle;
    bool mOverflowOnly;
    /** Cached content width. */
    int mContentWidth;
    int mDropDownGravity = Gravity::NO_GRAVITY;
private:
    void onGlobalLayout();
    void onViewDetachedFromWindow(View* v);
    bool tryShow();
public:
    StandardMenuPopup(Context* context, MenuBuilder* menu, View* anchorView,
            const std::string& popupStyleAttr,const std::string& popupStyleRes, bool overflowOnly);

    void setForceShowIcon(bool forceShow) override;

    void setGravity(int gravity) override;
    void show() override;
    void dismiss() override;

    void addMenu(MenuBuilder* menu) override;

    bool isShowing() override;
    virtual void onDismiss();

    void updateMenuView(bool cleared) override;
    void setCallback(const Callback& cb) override;

    bool onSubMenuSelected(SubMenuBuilder* subMenu)override;
    void onCloseMenu(MenuBuilder* menu, bool allMenusAreClosing)override;

    bool flagActionItems()override;

    Parcelable* onSaveInstanceState() override;
    void onRestoreInstanceState(Parcelable& state)override;

    void setAnchorView(View* anchor) override;

    virtual bool onKey(View& v, int keyCode, KeyEvent& event);
    void setOnDismissListener(const PopupWindow::OnDismissListener& listener) override;

    ListView* getListView()const override;

    void setHorizontalOffset(int x) override;
    void setVerticalOffset(int y) override;

    void setShowTitle(bool showTitle)override;
};
}/*endof namespace*/
#endif/*__STANDARD_MENU_POPUP_H__*/
