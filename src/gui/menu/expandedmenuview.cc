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
#include <menu/expandedmenuview.h>
#include <menu/menubuilder.h>
namespace cdroid{

DECLARE_WIDGET(ExpandedMenuView)
ExpandedMenuView::ExpandedMenuView(Context* context,const AttributeSet& attrs)
    :ListView(context, attrs){

    //TypedArray a = context.obtainStyledAttributes(attrs, com.android.internal.R.styleable.MenuView, 0, 0);
    //mAnimations = attrs.getResourceId(com.android.internal.R.styleable.MenuView_windowAnimationStyle, 0);
    setOnItemClickListener([this](AdapterView& parent, View& v, int position, long id){
        onItemClick(parent,v,position,id);
    });
}

void ExpandedMenuView::initialize(MenuBuilder* menu) {
    mMenu = menu;
}

void ExpandedMenuView::onDetachedFromWindow(){
    ListView::onDetachedFromWindow();

    // Clear the cached bitmaps of children
    setChildrenDrawingCacheEnabled(false);
}

bool ExpandedMenuView::invokeItem(MenuItemImpl* item) {
    return mMenu->performItemAction((MenuItem*)item, 0);
}

void ExpandedMenuView::onItemClick(AdapterView& parent, View& v, int position, long id) {
    invokeItem((MenuItemImpl*) getAdapter()->getItem(position));
}

int ExpandedMenuView::getWindowAnimations() {
    return mAnimations;
}

}
