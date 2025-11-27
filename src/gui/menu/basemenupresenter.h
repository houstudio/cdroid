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
#ifndef __BASE_MENU_PRESENTER_H__
#define __BASE_MENU_PRESENTER_H__
#include <menu/menuview.h>
#include <menu/menupresenter.h>
namespace cdroid{
class BaseMenuPresenter:public MenuPresenter {
private:
    std::string mMenuLayoutRes;
    std::string mItemLayoutRes;
    int mId;
protected:
    Context* mSystemContext;
    Context* mContext;
    MenuBuilder* mMenu;
    LayoutInflater* mSystemInflater;
    LayoutInflater* mInflater;
    Callback mCallback;
    MenuView * mMenuView;
    ViewGroup* mContainer;
protected:
    virtual void addItemView(View* itemView, int childIndex);
    virtual bool filterLeftoverView(ViewGroup* parent, int childIndex);
public:
    BaseMenuPresenter(Context* context,const std::string& menuLayoutRes,const std::string& itemLayoutRes);

    void initForMenu(Context* context, MenuBuilder* menu)override;
    ViewGroup* getMenuView(ViewGroup* root)override;
    void updateMenuView(bool cleared)override;
    void setCallback(const Callback& cb)override;
    Callback getCallback();

    View* createItemView(ViewGroup* parent);

    virtual View* getItemView(MenuItemImpl* item, View* convertView, ViewGroup* parent);
    virtual void bindItemView(MenuItemImpl* item, View* itemView)=0;

    virtual bool shouldIncludeItem(int childIndex, MenuItemImpl* item);

    void onCloseMenu(MenuBuilder* menu, bool allMenusAreClosing)override;
    bool onSubMenuSelected(SubMenuBuilder* menu)override;
    bool flagActionItems()override;

    bool expandItemActionView(MenuBuilder& menu, MenuItemImpl& item)override;
    bool collapseItemActionView(MenuBuilder& menu, MenuItemImpl& item)override;

    int getId() const override;
    void setId(int id);
};
}/*endof namespace*/
#endif/*__BASE_MENU_PRESENTER_H__*/
