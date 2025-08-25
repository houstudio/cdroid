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
#ifndef __MENU_POPUP_H__
#define __MENU_POPUP_H__
#include <menu/menubuilder.h>
#include <menu/menupresenter.h>
#include <widget/listview.h>
#include <widget/popupwindow.h>
namespace cdroid{
class MenuView;
class MenuAdapter;
class MenuPopup:public MenuPresenter{//implements ShowableListMenu, MenuPresenter,
        //AdapterView::OnItemClickListener {
private:
    Rect mEpicenterBounds;
protected:
    static int measureIndividualMenuWidth(ListAdapter* adapter, ViewGroup* parent,
            Context* context, int maxAllowedWidth);
    static MenuAdapter* toMenuAdapter(ListAdapter* adapter);
    static bool shouldPreserveIconSpacing(MenuBuilder* menu);
public:
    virtual void setForceShowIcon(bool forceShow)=0;

    virtual void addMenu(MenuBuilder* menu)=0;

    virtual void setGravity(int dropDownGravity)=0;

    virtual void setAnchorView(View* anchor)=0;

    virtual void setHorizontalOffset(int x)=0;
    virtual void setVerticalOffset(int y)=0;

    /**
     * Specifies the anchor-relative bounds of the popup's transition
     * epicenter.
     *
     * @param bounds anchor-relative bounds
     */
    void setEpicenterBounds(const Rect& bounds);
    Rect getEpicenterBounds() const;

    /**
     * Set whether a title entry should be shown in the popup menu (if a title exists for the
     * menu).
     *
     * @param showTitle
     */
    virtual void setShowTitle(bool showTitle)=0;

    /**
     * Set a listener to receive a callback when the popup is dismissed.
     *
     * @param listener Listener that will be notified when the popup is dismissed.
     */
    virtual void setOnDismissListener(const PopupWindow::OnDismissListener& listener)=0;

    void initForMenu(Context* context, MenuBuilder* menu) override;
    ViewGroup* getMenuView(ViewGroup* root)override;

    bool expandItemActionView(MenuBuilder& menu, MenuItemImpl& item) override;
    bool collapseItemActionView(MenuBuilder& menu, MenuItemImpl& item) override;

    int getId()const override;

    void onItemClick(AdapterView&parent, View& view, int position, long id);
    virtual void show();
    virtual void dismiss();
    virtual bool isShowing();
    virtual ListView*getListView()const=0;
};
}/*endof namespace*/
#endif/*__MENU_POP_H__*/
