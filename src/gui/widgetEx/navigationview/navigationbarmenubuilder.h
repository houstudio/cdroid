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
#ifndef __NAVIGATION_BAR_MENU_BUILDER_H__
#define __NAVIGATION_BAR_MENU_BUILDER_H__
// Port of com.google.android.material.navigation.NavigationBarMenuBuilder -
// a wrapper around MenuBuilder that flattens submenus into an ordered item
// list and keeps content/visibility counts. CDROID substrate note: material's
// DividerMenuItem separators around submenus are not ported; a submenu's
// children are flattened in place (the divider entries are simply omitted).
#include <vector>

namespace cdroid{
class MenuBuilder;
class MenuPresenter;
class MenuItem;

class NavigationBarMenuBuilder {
private:
    MenuBuilder* mMenuBuilder;
    std::vector<MenuItem*> mItems;
    int mContentItemCount;
    int mVisibleContentItemCount;
    int mVisibleMainItemCount;
public:
    NavigationBarMenuBuilder(MenuBuilder* menuBuilder);

    /** Total item count including flattened submenu items. */
    int size() const;
    /** Number of content (non-subheader) items. */
    int getContentItemCount() const;
    /** Number of visible content (non-subheader) items. */
    int getVisibleContentItemCount() const;
    /** Number of visible items that are not under a subheader. */
    int getVisibleMainContentItemCount() const;
    MenuItem* getItemAt(int i);
    bool performItemAction(MenuItem* item, MenuPresenter* presenter, int flags);
    /** Re-read the underlying MenuBuilder into the flattened item list. */
    void refreshItems();
};

} // namespace cdroid
#endif /* __NAVIGATION_BAR_MENU_BUILDER_H__ */
