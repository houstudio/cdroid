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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  0210-1301  USA
*/
#include <widgetEx/wear/wearabledraweractionmenu.h>
#include <core/context.h>
#include <drawable/drawable.h>
#include <stdexcept>

namespace cdroid{

// androidx.wear.widget.drawer.WearableActionDrawerMenu.java (lines 38-475)

WearableActionDrawerMenu::WearableActionDrawerMenu(Context* context,
        const WearableActionDrawerMenuListener& listener)
    : mContext(context),
      mListener(listener),
      // The anonymous MenuItemChangedListener (lines 45-55)
      mItemChangedListener([this](WearableActionDrawerMenuItem& item) {
          for (size_t i = 0; i < mItems.size(); i++) {
              if (mItems[i] == &item) {
                  mListener.menuItemChanged((int) i);
              }
          }
      }) {
}

WearableActionDrawerMenu::~WearableActionDrawerMenu() {
    for (WearableActionDrawerMenuItem* item : mItems) {
        delete item;
    }
}

MenuItem* WearableActionDrawerMenu::add(const std::string& title) {
    return add(0, 0, 0, title);
}

// Upstream also carries add(int titleRes) / add(int,int,int,int titleRes) which funnel through
// Resources.getString(titleRes) (lines 68-75); CDROID's Menu interface is string-keyed and has
// no int-res overloads, so only the two string forms exist. addIntentOptions (lines 162-172) is
// absent from CDROID's Menu for the same reason.

MenuItem* WearableActionDrawerMenu::add(int groupId, int itemId, int order, const std::string& title) {
    WearableActionDrawerMenuItem* item =
            new WearableActionDrawerMenuItem(mContext, itemId, title, mItemChangedListener);
    mItems.push_back(item);
    mListener.menuItemAdded((int) mItems.size() - 1);
    return item;
}

void WearableActionDrawerMenu::clear() {
    for (WearableActionDrawerMenuItem* item : mItems) {
        delete item; // CDROID owns the items (upstream relies on GC)
    }
    mItems.clear();
    mListener.menuChanged();
}

void WearableActionDrawerMenu::removeItem(int id) {
    const int index = findItemIndex(id);
    if ((index < 0) || (index >= (int) mItems.size())) {
        return;
    }
    delete mItems[index];
    mItems.erase(mItems.begin() + index);
    mListener.menuItemRemoved(index);
}

MenuItem* WearableActionDrawerMenu::findItem(int id) const {
    const int index = findItemIndex(id);
    if ((index < 0) || (index >= (int) mItems.size())) {
        return nullptr;
    }
    return mItems[index];
}

int WearableActionDrawerMenu::size() const {
    return (int) mItems.size();
}

MenuItem* WearableActionDrawerMenu::getItem(int index) {
    if ((index < 0) || (index >= (int) mItems.size())) {
        return nullptr;
    }
    return mItems[index];
}

int WearableActionDrawerMenu::findItemIndex(int id) const {
    const int itemCount = (int) mItems.size();
    for (int i = 0; i < itemCount; i++) {
        if (mItems[i]->getItemId() == id) {
            return i;
        }
    }
    return -1;
}

void WearableActionDrawerMenu::close() {
    throw std::logic_error("close is not implemented");
}

SubMenu* WearableActionDrawerMenu::addSubMenu(const std::string& title) {
    throw std::logic_error("addSubMenu is not implemented");
}

SubMenu* WearableActionDrawerMenu::addSubMenu(int groupId, int itemId, int order,
        const std::string& title) {
    throw std::logic_error("addSubMenu is not implemented");
}

void WearableActionDrawerMenu::removeGroup(int groupId) {
    // Inert upstream as well (lines 174-176)
}

void WearableActionDrawerMenu::setGroupCheckable(int group, bool checkable, bool exclusive) {
    throw std::logic_error("setGroupCheckable is not implemented");
}

void WearableActionDrawerMenu::setGroupVisible(int group, bool visible) {
    throw std::logic_error("setGroupVisible is not implemented");
}

void WearableActionDrawerMenu::setGroupEnabled(int group, bool enabled) {
    throw std::logic_error("setGroupEnabled is not implemented");
}

// CDROID's Menu adds these beyond android.view.Menu; upstream has no counterpart, so both are
// inert like the other unsupported group operations.
void WearableActionDrawerMenu::setOptionalIconsVisible(bool visible) {
}

void WearableActionDrawerMenu::setGroupDividerEnabled(bool groupDividerEnabled) {
}

bool WearableActionDrawerMenu::hasVisibleItems() const {
    return false;
}

bool WearableActionDrawerMenu::performShortcut(int keyCode, KeyEvent& event, int flags) {
    throw std::logic_error("performShortcut is not implemented");
}

bool WearableActionDrawerMenu::isShortcutKey(int keyCode, const KeyEvent& event) {
    return false;
}

bool WearableActionDrawerMenu::performIdentifierAction(int id, int flags) {
    throw std::logic_error("performIdentifierAction is not implemented");
}

void WearableActionDrawerMenu::setQwertyMode(bool isQwerty) {
    // Inert upstream as well (lines 213-215)
}

// WearableActionDrawerMenuItem (lines 228-474)

WearableActionDrawerMenu::WearableActionDrawerMenuItem::WearableActionDrawerMenuItem(
        Context* context, int id, const std::string& title, const MenuItemChangedListener& listener)
    : mId(id),
      mContext(context),
      mItemChangedListener(listener),
      mTitle(title) {
}

int WearableActionDrawerMenu::WearableActionDrawerMenuItem::getItemId() const {
    return mId;
}

// Upstream also carries setTitle(int titleRes) resolving through Resources.getString (line
// 261); CDROID's MenuItem interface is string-keyed and has no int-title overload.
MenuItem& WearableActionDrawerMenu::WearableActionDrawerMenuItem::setTitle(const std::string& title) {
    mTitle = title;
    if (mItemChangedListener) {
        mItemChangedListener(*this);
    }
    return *this;
}

std::string WearableActionDrawerMenu::WearableActionDrawerMenuItem::getTitle() {
    return mTitle;
}

MenuItem& WearableActionDrawerMenu::WearableActionDrawerMenuItem::setTitleCondensed(
        const std::string& title) {
    return *this;
}

std::string WearableActionDrawerMenu::WearableActionDrawerMenuItem::getTitleCondensed() {
    return std::string(); // upstream returns null
}

MenuItem& WearableActionDrawerMenu::WearableActionDrawerMenuItem::setIcon(Drawable* icon) {
    if (mIconDrawable != icon) {
        delete mIconDrawable; // CDROID MenuItem ownership: the item owns its icon (MenuItemImpl)
    }
    mIconDrawable = icon;
    if (mItemChangedListener) {
        mItemChangedListener(*this);
    }
    return *this;
}

MenuItem& WearableActionDrawerMenu::WearableActionDrawerMenuItem::setIcon(int iconRes) {
    return setIcon(mContext->getDrawable(iconRes)); // ContextCompat.getDrawable upstream
}

Drawable* WearableActionDrawerMenu::WearableActionDrawerMenuItem::getIcon() {
    return mIconDrawable;
}

MenuItem& WearableActionDrawerMenu::WearableActionDrawerMenuItem::setOnMenuItemClickListener(
        const OnMenuItemClickListener& menuItemClickListener) {
    mClickListener = menuItemClickListener;
    return *this;
}

int WearableActionDrawerMenu::WearableActionDrawerMenuItem::getGroupId() const {
    return 0;
}

int WearableActionDrawerMenu::WearableActionDrawerMenuItem::getOrder() const {
    return 0;
}

MenuItem& WearableActionDrawerMenu::WearableActionDrawerMenuItem::setIntent(Intent* intent) {
    throw std::logic_error("setIntent is not implemented");
}

Intent* WearableActionDrawerMenu::WearableActionDrawerMenuItem::getIntent() {
    return nullptr;
}

MenuItem& WearableActionDrawerMenu::WearableActionDrawerMenuItem::setShortcut(
        int numericChar, int alphaChar) {
    throw std::logic_error("setShortcut is not implemented");
}

MenuItem& WearableActionDrawerMenu::WearableActionDrawerMenuItem::setCheckable(bool checkable) {
    return *this;
}

bool WearableActionDrawerMenu::WearableActionDrawerMenuItem::isCheckable() const {
    return false;
}

MenuItem& WearableActionDrawerMenu::WearableActionDrawerMenuItem::setChecked(bool checked) {
    return *this;
}

bool WearableActionDrawerMenu::WearableActionDrawerMenuItem::isChecked() const {
    return false;
}

MenuItem& WearableActionDrawerMenu::WearableActionDrawerMenuItem::setVisible(bool visible) {
    return *this;
}

bool WearableActionDrawerMenu::WearableActionDrawerMenuItem::isVisible() const {
    return false;
}

MenuItem& WearableActionDrawerMenu::WearableActionDrawerMenuItem::setEnabled(bool enabled) {
    return *this;
}

bool WearableActionDrawerMenu::WearableActionDrawerMenuItem::isEnabled() const {
    return false;
}

ContextMenuInfo* WearableActionDrawerMenu::WearableActionDrawerMenuItem::getMenuInfo() {
    return nullptr;
}

void WearableActionDrawerMenu::WearableActionDrawerMenuItem::setShowAsAction(int actionEnum) {
    throw std::logic_error("setShowAsAction is not implemented");
}

MenuItem& WearableActionDrawerMenu::WearableActionDrawerMenuItem::setShowAsActionFlags(int actionEnum) {
    throw std::logic_error("setShowAsActionFlags is not implemented");
}

MenuItem& WearableActionDrawerMenu::WearableActionDrawerMenuItem::setActionView(View* view) {
    throw std::logic_error("setActionView is not implemented");
}

MenuItem& WearableActionDrawerMenu::WearableActionDrawerMenuItem::setActionView(int resId) {
    throw std::logic_error("setActionView is not implemented");
}

View* WearableActionDrawerMenu::WearableActionDrawerMenuItem::getActionView() {
    return nullptr;
}

MenuItem& WearableActionDrawerMenu::WearableActionDrawerMenuItem::setActionProvider(
        ActionProvider* actionProvider) {
    throw std::logic_error("setActionProvider is not implemented");
}

ActionProvider* WearableActionDrawerMenu::WearableActionDrawerMenuItem::getActionProvider() {
    return nullptr;
}

bool WearableActionDrawerMenu::WearableActionDrawerMenuItem::expandActionView() {
    throw std::logic_error("expandActionView is not implemented");
}

bool WearableActionDrawerMenu::WearableActionDrawerMenuItem::collapseActionView() {
    throw std::logic_error("collapseActionView is not implemented");
}

bool WearableActionDrawerMenu::WearableActionDrawerMenuItem::isActionViewExpanded() const {
    throw std::logic_error("isActionViewExpanded is not implemented");
}

MenuItem& WearableActionDrawerMenu::WearableActionDrawerMenuItem::setOnActionExpandListener(
        const OnActionExpandListener& listener) {
    throw std::logic_error("setOnActionExpandListener is not implemented");
}

bool WearableActionDrawerMenu::WearableActionDrawerMenuItem::invoke() {
    return mClickListener ? mClickListener(*this) : false; // listener != null && onMenuItemClick
}

WearableActionDrawerMenu::WearableActionDrawerMenuItem::~WearableActionDrawerMenuItem() {
    delete mIconDrawable;
}

}/*endof namespace*/
