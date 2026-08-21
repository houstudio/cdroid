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
#ifndef __NAVIGATION_MENU_VIEW_H__
#define __NAVIGATION_MENU_VIEW_H__
// Port of com.google.android.material.navigation.NavigationMenuView (the
// RecyclerView the presenter binds its adapter to; inflates from
// design_navigation_menu.xml on Android).
#include <widgetEx/recyclerview/recyclerview.h>
#include <menu/menuview.h>

namespace cdroid{

class NavigationMenuView:public RecyclerView,public MenuView{
public:
    NavigationMenuView(Context* context);
    NavigationMenuView(Context* context,const AttributeSet* attrs);
    void initialize(MenuBuilder* menu)override;
    int getWindowAnimations()override;
};

}//namespace cdroid
#endif/*__NAVIGATION_MENU_VIEW_H__*/
