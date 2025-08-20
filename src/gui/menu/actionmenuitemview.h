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
#ifndef __ACTION_MENUITEM_VIEW_H__
#define __ACTION_MENUITEM_VIEW_H__
#include <widget/textview.h>
#include <widget/forwardinglistener.h>
#include <menu/menubuilder.h>
#include <menu/actionmenuview.h>
namespace cdroid{
class ActionMenuItemView:public TextView,public MenuView::ItemView,public ActionMenuView::ActionMenuChildView{
        //implements MenuView.ItemView, View.OnClickListener, ActionMenuView.ActionMenuChildView {
public:
    DECLARE_UIEVENT(ShowableListMenu*,PopupCallback);
    /*class PopupCallback {
        virtual ShowableListMenu* getPopup()=0;
    };*/
private:
    static constexpr int MAX_ICON_SIZE = 32; // dp
    class ActionMenuItemForwardingListener;
    MenuItemImpl* mItemData;
    std::string mTitle;
    Drawable* mIcon;
    MenuBuilder::ItemInvoker mItemInvoker;
    ForwardingListener* mForwardingListener;
    PopupCallback* mPopupCallback;

    bool mAllowTextWithIcon;
    bool mExpandedFormat;
    int mMinWidth;
    int mSavedPaddingLeft;
    int mMaxIconSize;
private:
    bool shouldAllowTextWithIcon();
    void updateTextButtonVisibility();
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
public:
    ActionMenuItemView(Context* context,const AttributeSet& attrs);
    //void onConfigurationChanged(Configuration newConfig) override;

    std::string getAccessibilityClassName() const;

    void setPadding(int l, int t, int r, int b) override;

    MenuItemImpl* getItemData();

    void initialize(MenuItemImpl* itemData, int menuType) override;
    bool onTouchEvent(MotionEvent& e) override;

    void onClick(View& v);

    void setItemInvoker(const MenuBuilder::ItemInvoker& invoker);

    void setPopupCallback(const PopupCallback& popupCallback);

    bool prefersCondensedTitle()const;

    void setCheckable(bool checkable);

    void setChecked(bool checked);

    void setExpandedFormat(bool expandedFormat);

    void setIcon(Drawable* icon);
    bool hasText()const;

    void setShortcut(bool showShortcut, int shortcutKey);

    void setTitle(const std::string& title);

    bool dispatchPopulateAccessibilityEventInternal(AccessibilityEvent& event)override;
    void onPopulateAccessibilityEventInternal(AccessibilityEvent& event)override;
    bool dispatchHoverEvent(MotionEvent& event)override;

    bool showsIcon();

    bool needsDividerBefore();
    bool needsDividerAfter();

    void onRestoreInstanceState(Parcelable& state)override;
};/*endof ActionMenuItemView*/
#if 0
class ActionMenuItemView::ActionMenuItemForwardingListener:public ForwardingListener {
public:
    ActionMenuItemForwardingListener();
    ShowableListMenu* getPopup() override;
    bool onForwardingStarted() override;
};
#endif
}/*endof namespace*/
#endif/*__ACTION_MENUITEM_VIEW_H__*/

