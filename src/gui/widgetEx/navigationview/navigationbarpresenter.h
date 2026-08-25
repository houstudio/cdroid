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
#ifndef __NAVIGATION_BAR_PRESENTER_H__
#define __NAVIGATION_BAR_PRESENTER_H__
// Port of com.google.android.material.navigation.NavigationBarPresenter -
// the MenuPresenter binding a NavigationBarView's MenuBuilder to its
// NavigationBarMenuView. Badge saved-state is not ported.
#include <menu/menuitemimpl.h>
#include <menu/menupresenter.h>

namespace cdroid{

class NavigationBarMenuView;

class NavigationBarPresenter : public MenuPresenter {
private:
    NavigationBarMenuView* mMenuView;
    bool mUpdateSuspended = false;
    int mId;
public:
    NavigationBarPresenter();

    void setMenuView(NavigationBarMenuView* menuView);

    void initForMenu(Context* context, MenuBuilder* menu) override;
    ViewGroup* getMenuView(ViewGroup* root) override;
    void updateMenuView(bool cleared) override;
    void setCallback(const Callback& cb) override {}
    bool onSubMenuSelected(SubMenuBuilder* subMenu) override;
    void onCloseMenu(MenuBuilder* menu, bool allMenusAreClosing) override {}
    bool flagActionItems() override { return false; }
    bool expandItemActionView(MenuBuilder& menu, MenuItemImpl& item) override { return false; }
    bool collapseItemActionView(MenuBuilder& menu, MenuItemImpl& item) override { return false; }

    void setId(int id);
    int getId() const override;
    Parcelable* onSaveInstanceState() override;
    void onRestoreInstanceState(Parcelable& state) override;

    void setUpdateSuspended(bool updateSuspended);
};

} // namespace cdroid
#endif /* __NAVIGATION_BAR_PRESENTER_H__ */
