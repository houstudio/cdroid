/*********************************************************************************
 * Copyright (C) 2019] [houzh@msn.com]
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
#ifndef __BOTTOM_NAVIGATION_ITEM_VIEW_H__
#define __BOTTOM_NAVIGATION_ITEM_VIEW_H__
// Port of com.google.android.material.bottomnavigation.BottomNavigationItemView
// - the concrete item for BottomNavigationMenuView. Material selects the item
// layout and default margin resources; CDROID builds the item in code, so only
// the default top margin (design_bottom_navigation_margin, 8dp) remains.
#include <widgetEx/navigationview/navigationbaritemview.h>

namespace cdroid{

class BottomNavigationItemView : public NavigationBarItemView {
public:
    BottomNavigationItemView(Context* context, const AttributeSet* attrs = nullptr);
};

} // namespace cdroid
#endif /* __BOTTOM_NAVIGATION_ITEM_VIEW_H__ */
