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
#include <widget/internal_R.h>
using namespace cdroid::internal;

namespace cdroid{

DECLARE_WIDGET2(ExpandedMenuView, "androidx.appcompat.view.menu.ExpandedMenuView");
ExpandedMenuView::ExpandedMenuView(Context* context,const AttributeSet* attrs)
    :ExpandedMenuView(context,attrs,(int)R::attr::listViewStyle){}

ExpandedMenuView::ExpandedMenuView(Context* context,const AttributeSet* pAttrs,int defStyleAttr)
    :ListView(context, pAttrs, defStyleAttr){

    // androidx re-reads { background, divider } via TintTypedArray only as a tint shim; the
    // base ctors already resolve both through the defStyleAttr chain (ListView reads its
    // divider with defStyleAttr, View the background), so no extra read is needed here.
    // windowAnimationStyle is not read either — androidx leaves mAnimations at 0
    // (getWindowAnimations() is vestigial); the hosting dialog's own theme carries its window
    // animations (theme windowAnimationStyle -> Window enter/exit).
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
