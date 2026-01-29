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
#ifndef __EXPANDED_MENU_VIEW_H__
#define __EXPANDED_MENU_VIEW_H__
#include <menu/menuview.h>
#include <widget/listview.h>
namespace cdroid{
class ExpandedMenuView:public ListView,public MenuView{// implements ItemInvoker, MenuView, OnItemClickListener {
private:
    int mAnimations;
    MenuBuilder* mMenu;
protected:
    void onDetachedFromWindow() override;
public:
    ExpandedMenuView(Context* context,const AttributeSet& attrs);
    void initialize(MenuBuilder* menu)override;
    bool invokeItem(MenuItemImpl* item);

    void onItemClick(AdapterView& parent, View& v, int position, long id);
    int getWindowAnimations()override;
};
}/*endof namespace*/
#endif/*__EXPANDED_MENU_VIEW_H__*/
