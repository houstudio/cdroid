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
#include <widgetEx/navigationview/navigationbarpresenter.h>
#include <widgetEx/navigationview/navigationbarmenuview.h>
#include <menu/menubuilder.h>
#include <menu/submenubuilder.h>

namespace cdroid{

namespace {
// AOSP NavigationBarPresenter.SavedState carries the selected item id and
// badge states; the badge half is not ported. The selected id rides this
// minimal Parcelable.
class SavedState : public Parcelable {
public:
    int selectedItemId = 0;
};
} // namespace

NavigationBarPresenter::NavigationBarPresenter()
    : mMenuView(nullptr)
    , mId(0) {
}

void NavigationBarPresenter::setMenuView(NavigationBarMenuView* menuView) {
    mMenuView = menuView;
}

void NavigationBarPresenter::initForMenu(Context* context, MenuBuilder* menu) {
    mMenuView->initialize(menu);
}

ViewGroup* NavigationBarPresenter::getMenuView(ViewGroup* root) {
    return mMenuView;
}

void NavigationBarPresenter::updateMenuView(bool cleared) {
    if (mUpdateSuspended) {
        return;
    }
    if (cleared) {
        mMenuView->buildMenuView();
    } else {
        mMenuView->updateMenuView();
    }
}

bool NavigationBarPresenter::onSubMenuSelected(SubMenuBuilder* subMenu) {
    return false;
}

void NavigationBarPresenter::setId(int id) {
    mId = id;
}

int NavigationBarPresenter::getId() const {
    return mId;
}

Parcelable* NavigationBarPresenter::onSaveInstanceState() {
    SavedState* savedState = new SavedState();
    savedState->selectedItemId = mMenuView->getSelectedItemId();
    return savedState;
}

void NavigationBarPresenter::onRestoreInstanceState(Parcelable& state) {
    if (SavedState* saved = dynamic_cast<SavedState*>(&state)) {
        mMenuView->tryRestoreSelectedItemId(saved->selectedItemId);
    }
    delete &state;
}

void NavigationBarPresenter::setUpdateSuspended(bool updateSuspended) {
    mUpdateSuspended = updateSuspended;
}

} // namespace cdroid
