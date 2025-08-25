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
#include <core/bundle.h>
#include <core/context.h>
#include <view/keyevent.h>
#include <menu/menuitemimpl.h>
#include <menu/menubuilder.h>
#include <menu/submenubuilder.h>
#include <menu/menupresenter.h>
#include <view/actionprovider.h>
#include <view/viewconfiguration.h>
namespace cdroid{

MenuBuilder::MenuBuilder(Context* context) {
    mContext = context;
    mCurrentMenuInfo = nullptr;
    mIsVisibleItemsStale = true;
    mIsActionItemsStale = true;
    mIsClosing = false;
    mGroupDividerEnabled  = false;
    mOptionalIconsVisible = false;
    mPreventDispatchingItemsChanged = false;
    mItemsChangedWhileDispatchPrevented = false;
    mDefaultShowAsAction = MenuItem::SHOW_AS_ACTION_NEVER;
    setShortcutsVisibleInner(true);
}

MenuBuilder::~MenuBuilder(){
    for(auto item:mItems){
        delete item;
    }
    mItems.clear();
}

MenuBuilder& MenuBuilder::setDefaultShowAsAction(int defaultShowAsAction) {
    mDefaultShowAsAction = defaultShowAsAction;
    return *this;
}

void MenuBuilder::addMenuPresenter(MenuPresenter* presenter) {
    addMenuPresenter(presenter, mContext);
}

void MenuBuilder::addMenuPresenter(MenuPresenter* presenter, Context* menuContext) {
    mPresenters.push_back(presenter);
    presenter->initForMenu(menuContext, this);
    mIsActionItemsStale = true;
}

void MenuBuilder::removeMenuPresenter(MenuPresenter* presenter) {
    for (auto it=mPresenters.begin();it!=mPresenters.end();){
        MenuPresenter* item=*it;
        if ((item == nullptr) || (item == presenter)) {
            it = mPresenters.erase(it);
        }else {
            it++;
        }
    }
}

void MenuBuilder::dispatchPresenterUpdate(bool cleared) {
    if (mPresenters.empty()) return;
    stopDispatchingItemsChanged();
    for (auto it = mPresenters.begin();it!=mPresenters.end();) {
        MenuPresenter* presenter =*it;
        if (presenter == nullptr) {
            it = mPresenters.erase(it);
        } else {
            presenter->updateMenuView(cleared);
            it++;
        }
    }
    startDispatchingItemsChanged();
}

bool MenuBuilder::dispatchSubMenuSelected(SubMenuBuilder* subMenu,MenuPresenter* preferredPresenter) {
    if (mPresenters.empty()) return false;
    bool result = false;
    // Try the preferred presenter first.
    if (preferredPresenter != nullptr) {
        result = preferredPresenter->onSubMenuSelected(subMenu);
    }

    for (auto  it = mPresenters.begin();it!=mPresenters.end();) {
        MenuPresenter* presenter = *it;
        if (presenter == nullptr) {
            it = mPresenters.erase(it);
        } else if (!result) {
            result = presenter->onSubMenuSelected(subMenu);
            it++;
        }
    }
    return result;
}
#if 0
void MenuBuilder::dispatchSaveInstanceState(Bundle outState) {
    if (mPresenters.empty()) return;

    SparseArray<Parcelable> presenterStates = new SparseArray<Parcelable>();
    for (auto it = mPresenters.begin();it!=mPresenters.end();) {
        MenuPresenter* presenter = *it;
        if (presenter == nullptr) {
            it = mPresenters.erase(it);
        } else {
            const int id = presenter.getId();
            it++;
            if (id > 0) {
                Parcelable state = presenter.onSaveInstanceState();
                if (state != nullptr) {
                    presenterStates.put(id, state);
                }
            }
        }
    }
    outState.putSparseParcelableArray(PRESENTER_KEY, presenterStates);
}

void MenuBuilder::dispatchRestoreInstanceState(Bundle state) {
    SparseArray<Parcelable> presenterStates = state.getSparseParcelableArray(PRESENTER_KEY);

    if (presenterStates == null || mPresenters.iempty()) return;
    for (auto it = mPresenters.begin();it!=mPresenters.end();) {
        MenuPresenter* presenter = *it;
        if (presenter == null) {
            it = mPresenters.erase(it);
        } else {
            const int id = presenter->getId();
            it++;
            if (id > 0) {
                Parcelable parcel = presenterStates.get(id);
                if (parcel != nullptr) {
                    presenter->onRestoreInstanceState(parcel);
                }
            }
        }
    }
}

void MenuBuilder::savePresenterStates(Bundle outState) {
    dispatchSaveInstanceState(outState);
}

void MenuBuilder::restorePresenterStates(Bundle state) {
    dispatchRestoreInstanceState(state);
}

void MenuBuilder::saveActionViewStates(Bundle outStates) {
    SparseArray<Parcelable> viewStates = null;

    const int itemCount = size();
    for (int i = 0; i < itemCount; i++) {
        const MenuItem* item = getItem(i);
        View* v = item->getActionView();
        if (v != nullptr && v->getId() != View::NO_ID) {
            if (viewStates == nullptr) {
                viewStates = new SparseArray<Parcelable>();
            }
            v->saveHierarchyState(viewStates);
            if (item->isActionViewExpanded()) {
                outStates.putInt(EXPANDED_ACTION_VIEW_ID, item.getItemId());
            }
        }
        if (item.hasSubMenu()) {
            SubMenuBuilder* subMenu = (SubMenuBuilder*) item->getSubMenu();
            subMenu->saveActionViewStates(outStates);
        }
    }
    if (viewStates != null) {
        outStates.putSparseParcelableArray(getActionViewStatesKey(), viewStates);
    }
}

void MenuBuilder::restoreActionViewStates(Bundle states) {
    if (states == null) {
        return;
    }
    SparseArray<Parcelable> viewStates = states.getSparseParcelableArray(getActionViewStatesKey());

    const int itemCount = size();
    for (int i = 0; i < itemCount; i++) {
        MenuItem* item = getItem(i);
        View* v = item->getActionView();
        if (v != nullptr && v->getId() != View::NO_ID) {
            v->restoreHierarchyState(viewStates);
        }
        if (item->hasSubMenu()) {
            SubMenuBuilder* subMenu = (SubMenuBuilder*) item->getSubMenu();
            subMenu->restoreActionViewStates(states);
        }
    }

    const int expandedId = states.getInt(EXPANDED_ACTION_VIEW_ID);
    if (expandedId > 0) {
        MenuItem* itemToExpand = findItem(expandedId);
        if (itemToExpand != nullptr) {
            itemToExpand->expandActionView();
        }
    }
}
#endif
std::string MenuBuilder::getActionViewStatesKey() {
    return ACTION_VIEW_STATES_KEY;
}

void MenuBuilder::setCallback(const Callback& cb) {
    mCallback = cb;
}

MenuItem* MenuBuilder::addInternal(int group, int id, int categoryOrder,const std::string& title) {
    const int ordering = getOrdering(categoryOrder);
    MenuItemImpl* item = createNewMenuItem(group, id, categoryOrder, ordering, title,mDefaultShowAsAction);
    if (mCurrentMenuInfo != nullptr) {
        // Pass along the current menu info
        item->setMenuInfo(mCurrentMenuInfo);
    }
    mItems.insert(mItems.begin()+findInsertIndex(mItems, ordering), item);
    onItemsChanged(true);
    return item;
}

MenuItemImpl* MenuBuilder::createNewMenuItem(int group, int id, int categoryOrder, int ordering, const std::string& title, int defaultShowAsAction) {
    return new MenuItemImpl(this, group, id, categoryOrder, ordering, title, defaultShowAsAction);
}

MenuItem* MenuBuilder::add(const std::string& title) {
    return addInternal(0, 0, 0, title);
}

MenuItem* MenuBuilder::add(int group, int id, int categoryOrder, const std::string& title) {
    return addInternal(group, id, categoryOrder, title);
}

SubMenu* MenuBuilder::addSubMenu(const std::string& title) {
    return addSubMenu(0, 0, 0, title);
}

SubMenu* MenuBuilder::addSubMenu(int group, int id, int categoryOrder, const std::string& title) {
    MenuItemImpl* item = (MenuItemImpl*) addInternal(group, id, categoryOrder, title);
    SubMenuBuilder* subMenu = new SubMenuBuilder(mContext, this, item);
    item->setSubMenu(subMenu);
    return subMenu;
}

void MenuBuilder::setGroupDividerEnabled(bool groupDividerEnabled) {
    mGroupDividerEnabled = groupDividerEnabled;
}

bool MenuBuilder::isGroupDividerEnabled() {
    return mGroupDividerEnabled;
}

/*int MenuBuilder::addIntentOptions(int group, int id, int categoryOrder, ComponentName caller,
        Intent[] specifics, Intent intent, int flags, MenuItem[] outSpecificItems) {
    PackageManager pm = mContext.getPackageManager();
    final List<ResolveInfo> lri = pm.queryIntentActivityOptions(caller, specifics, intent, 0);
    const int N = lri != null ? lri.size() : 0;

    if ((flags & FLAG_APPEND_TO_GROUP) == 0) {
        removeGroup(group);
    }

    for (int i=0; i<N; i++) {
        ResolveInfo ri = lri.get(i);
        Intent rintent = new Intent(ri.specificIndex < 0 ? intent : specifics[ri.specificIndex]);
        rintent.setComponent(new ComponentName( ri.activityInfo.applicationInfo.packageName,ri.activityInfo.name));
        MenuItem* item = add(group, id, categoryOrder, ri.loadLabel(pm))
                .setIcon(ri.loadIcon(pm))
                .setIntent(rintent);
        if (outSpecificItems != null && ri.specificIndex >= 0) {
            outSpecificItems[ri.specificIndex] = item;
        }
    }
    return N;
}*/

void MenuBuilder::removeItem(int id) {
    removeItemAtInt(findItemIndex(id), true);
}

void MenuBuilder::removeGroup(int group) {
    const int i = findGroupIndex(group);
    if (i >= 0) {
        const int maxRemovable = mItems.size() - i;
        int numRemoved = 0;
        while ((numRemoved++ < maxRemovable) && (mItems.at(i)->getGroupId() == group)) {
            // Don't force update for each one, this method will do it at the end
            removeItemAtInt(i, false);
        }
        // Notify menu views
        onItemsChanged(true);
    }
}

void MenuBuilder::removeItemAtInt(int index, bool updateChildrenOnMenuViews) {
    if ((index < 0) || (index >= mItems.size())) return;
    mItems.erase(mItems.begin()+index);
    if (updateChildrenOnMenuViews) onItemsChanged(true);
}

void MenuBuilder::removeItemAt(int index) {
    removeItemAtInt(index, true);
}

void MenuBuilder::clearAll() {
    mPreventDispatchingItemsChanged = true;
    clear();
    clearHeader();
    mPresenters.clear();
    mPreventDispatchingItemsChanged = false;
    mItemsChangedWhileDispatchPrevented = false;
    onItemsChanged(true);
}

void MenuBuilder::clear() {
    if (mExpandedItem != nullptr) {
        collapseItemActionView(mExpandedItem);
    }
    mItems.clear();
    onItemsChanged(true);
}

void MenuBuilder::setExclusiveItemChecked(MenuItem& item) {
    const int group = item.getGroupId();
    const int N = mItems.size();
    for (int i = 0; i < N; i++) {
        MenuItemImpl* curItem = mItems.at(i);
        if (curItem->getGroupId() == group) {
            if (!curItem->isExclusiveCheckable()) continue;
            if (!curItem->isCheckable()) continue;
            // Check the item meant to be checked, uncheck the others (that are in the group)
            curItem->setCheckedInt(curItem == &item);
        }
    }
}

void MenuBuilder::setGroupCheckable(int group, bool checkable, bool exclusive) {
    const int N = mItems.size();
    for (int i = 0; i < N; i++) {
        MenuItemImpl* item = mItems.at(i);
        if (item->getGroupId() == group) {
            item->setExclusiveCheckable(exclusive);
            item->setCheckable(checkable);
        }
    }
}

void MenuBuilder::setGroupVisible(int group, bool visible) {
    const int N = mItems.size();

    // We handle the notification of items being changed ourselves, so we use setVisibleInt rather
    // than setVisible and at the end notify of items being changed
    bool changedAtLeastOneItem = false;
    for (int i = 0; i < N; i++) {
        MenuItemImpl* item = mItems.at(i);
        if (item->getGroupId() == group) {
            if (item->setVisibleInt(visible)) changedAtLeastOneItem = true;
        }
    }
    if (changedAtLeastOneItem) onItemsChanged(true);
}

void MenuBuilder::setGroupEnabled(int group, bool enabled) {
    const int N = mItems.size();
    for (int i = 0; i < N; i++) {
        MenuItemImpl* item = mItems.at(i);
        if (item->getGroupId() == group) {
            item->setEnabled(enabled);
        }
    }
}

bool MenuBuilder::hasVisibleItems() const{
    const int _size = size();
    for (int i = 0; i < _size; i++) {
        MenuItemImpl* item = mItems.at(i);
        if (item->isVisible()) {
            return true;
        }
    }
    return false;
}

MenuItem* MenuBuilder::findItem(int id) const{
    const int _size = size();
    for (int i = 0; i < _size; i++) {
        MenuItemImpl* item = mItems.at(i);
        if (item->getItemId() == id) {
            return item;
        } else if (item->hasSubMenu()) {
            MenuItem* possibleItem = item->getSubMenu()->findItem(id);

            if (possibleItem != nullptr) {
                return possibleItem;
            }
        }
    }
    return nullptr;
}

int MenuBuilder::findItemIndex(int id) {
    const int _size = size();
    for (int i = 0; i < _size; i++) {
        MenuItemImpl* item = mItems.at(i);
        if (item->getItemId() == id) {
            return i;
        }
    }
    return -1;
}

int MenuBuilder::findGroupIndex(int group) {
    return findGroupIndex(group, 0);
}

int MenuBuilder::findGroupIndex(int group, int start) {
    const int _size = size();
    if (start < 0) start = 0;
    for (int i = start; i < _size; i++) {
        MenuItemImpl* item = mItems.at(i);
        if (item->getGroupId() == group) {
            return i;
        }
    }
    return -1;
}

int MenuBuilder::size() const{
    return mItems.size();
}

MenuItem* MenuBuilder::getItem(int index) {
    return mItems.at(index);
}

bool MenuBuilder::isShortcutKey(int keyCode,const KeyEvent& event) {
    return findItemWithShortcutForKey(keyCode, event) != nullptr;
}

void MenuBuilder::setQwertyMode(bool isQwerty) {
    mQwertyMode = isQwerty;
    onItemsChanged(false);
}

static const int sCategoryToOrder[]={
    1, /* No category */
    4, /* CONTAINER */
    5, /* SYSTEM */
    3, /* SECONDARY */
    2, /* ALTERNATIVE */
    0, /* SELECTED_ALTERNATIVE */
};

int MenuBuilder::getOrdering(int categoryOrder) {
    const int index = (categoryOrder & CATEGORY_MASK) >> CATEGORY_SHIFT;
    if (index < 0 || index >= sizeof(sCategoryToOrder)/sizeof(sCategoryToOrder[0])) {
        throw std::invalid_argument("order does not contain a valid category.");
    }
    return (sCategoryToOrder[index] << CATEGORY_SHIFT) | (categoryOrder & USER_MASK);
}

bool MenuBuilder::isQwertyMode() const{
    return mQwertyMode;
}

void MenuBuilder::setShortcutsVisible(bool shortcutsVisible) {
    if (mShortcutsVisible == shortcutsVisible) return;
    setShortcutsVisibleInner(shortcutsVisible);
    onItemsChanged(false);
}

void MenuBuilder::setShortcutsVisibleInner(bool shortcutsVisible) {
    mShortcutsVisible = shortcutsVisible
            //&& mResources.getConfiguration().keyboard != Configuration.KEYBOARD_NOKEYS
            && ViewConfiguration::get(mContext).shouldShowMenuShortcutsWhenKeyboardPresent();
}

bool MenuBuilder::isShortcutsVisible() {
    return mShortcutsVisible;
}

Context* MenuBuilder::getContext() {
    return mContext;
}

bool MenuBuilder::dispatchMenuItemSelected(MenuBuilder& menu, MenuItem& item) {
    return mCallback.onMenuItemSelected != nullptr && mCallback.onMenuItemSelected(menu, item);
}

void MenuBuilder::changeMenuMode() {
    if (mCallback.onMenuModeChange != nullptr) {
        mCallback.onMenuModeChange(*this);
    }
}

int MenuBuilder::findInsertIndex(const std::vector<MenuItemImpl*>& items, int ordering) {
    for (int i = items.size() - 1; i >= 0; i--) {
        MenuItemImpl* item = items.at(i);
        if (item->getOrdering() <= ordering) {
            return i + 1;
        }
    }
    return 0;
}

bool MenuBuilder::performShortcut(int keyCode,KeyEvent& event, int flags) {
    MenuItemImpl* item = findItemWithShortcutForKey(keyCode, event);
    bool handled = false;
    if (item != nullptr) {
        handled = performItemAction(item, flags);
    }
    if ((flags & FLAG_ALWAYS_PERFORM_CLOSE) != 0) {
        close(true /* closeAllMenus */);
    }
    return handled;
}

void MenuBuilder::findItemsWithShortcutForKey(std::vector<MenuItemImpl*>& items, int keyCode,const KeyEvent& event) {
    const bool qwerty = isQwertyMode();
    const int modifierState = event.getModifiers();
#if 0
    KeyCharacterMap.KeyData possibleChars = new KeyCharacterMap.KeyData();
    // Get the chars associated with the keyCode (i.e using any chording combo)
    const bool isKeyCodeMapped = event.getKeyData(possibleChars);
    // The delete key is not mapped to '\b' so we treat it specially
    if (!isKeyCodeMapped && (keyCode != KeyEvent::KEYCODE_DEL)) {
        return;
    }

    // Look for an item whose shortcut is this key.
    const int N = mItems.size();
    for (int i = 0; i < N; i++) {
        MenuItemImpl* item = mItems.at(i);
        if (item->hasSubMenu()) {
            ((MenuBuilder*)item->getSubMenu())->findItemsWithShortcutForKey(items, keyCode, event);
        }
        const int shortcutChar =  qwerty ? item->getAlphabeticShortcut() : item->getNumericShortcut();
        const int shortcutModifiers = qwerty ? item->getAlphabeticModifiers() : item->getNumericModifiers();
        const bool isModifiersExactMatch = (modifierState & SUPPORTED_MODIFIERS_MASK)
                == (shortcutModifiers & SUPPORTED_MODIFIERS_MASK);
        if (isModifiersExactMatch && (shortcutChar != 0) &&
              (shortcutChar == possibleChars.meta[0]|| shortcutChar == possibleChars.meta[2]
                  || (qwerty && shortcutChar == '\b' && keyCode == KeyEvent::KEYCODE_DEL)) &&
              item->isEnabled()) {
            items.push_back(item);
        }
    }
#endif
}

MenuItemImpl* MenuBuilder::findItemWithShortcutForKey(int keyCode,const KeyEvent& event) {
    // Get all items that can be associated directly or indirectly with the keyCode
    std::vector<MenuItemImpl*>& items = mTempShortcutItemList;
    items.clear();
    findItemsWithShortcutForKey(items, keyCode, event);

    if (items.empty()) {
        return nullptr;
    }
#if 0
    const int metaState = event.getMetaState();
    KeyCharacterMap.KeyData possibleChars = new KeyCharacterMap.KeyData();
    // Get the chars associated with the keyCode (i.e using any chording combo)
    event.getKeyData(possibleChars);

    // If we have only one element, we can safely returns it
    const int size = items.size();
    if (size == 1) {
        return items.at(0);
    }
    const bool qwerty = isQwertyMode();
    // If we found more than one item associated with the key,
    // we have to return the exact match
    for (int i = 0; i < size; i++) {
        MenuItemImpl* item = items.at(i);
        const int shortcutChar = qwerty ? item->getAlphabeticShortcut() :item->getNumericShortcut();
        if ((shortcutChar == possibleChars.meta[0] &&
                (metaState & KeyEvent::META_ALT_ON) == 0)
            || (shortcutChar == possibleChars.meta[2] && (metaState & KeyEvent::META_ALT_ON) != 0)
            || (qwerty && shortcutChar == '\b' && keyCode == KeyEvent::KEYCODE_DEL)) {
            return item;
        }
    }
#endif
    return nullptr;
}

bool MenuBuilder::performIdentifierAction(int id, int flags) {
    // Look for an item whose identifier is the id.
    return performItemAction(findItem(id), flags);
}

bool MenuBuilder::performItemAction(MenuItem* item, int flags) {
    return performItemAction(item, nullptr, flags);
}

bool MenuBuilder::performItemAction(MenuItem* item, MenuPresenter* preferredPresenter, int flags) {
    MenuItemImpl* itemImpl = (MenuItemImpl*) item;
    if (itemImpl == nullptr || !itemImpl->isEnabled()) {
        return false;
    }

    bool invoked = itemImpl->invoke();
    ActionProvider* provider = item->getActionProvider();
    const bool providerHasSubMenu = provider != nullptr && provider->hasSubMenu();
    if (itemImpl->hasCollapsibleActionView()) {
        invoked |= itemImpl->expandActionView();
        if (invoked) close(true /* closeAllMenus */);
    } else if (itemImpl->hasSubMenu() || providerHasSubMenu) {
        if (!itemImpl->hasSubMenu()) {
            itemImpl->setSubMenu(new SubMenuBuilder(getContext(), this, itemImpl));
        }
        SubMenuBuilder* subMenu = (SubMenuBuilder*)itemImpl->getSubMenu();
        if (providerHasSubMenu) {
            provider->onPrepareSubMenu(*subMenu);
        }
        invoked |= dispatchSubMenuSelected(subMenu, preferredPresenter);
        if (!invoked) close(true /* closeAllMenus */);
    } else {
        if ((flags & FLAG_PERFORM_NO_CLOSE) == 0) {
            close(true /* closeAllMenus */);
        }
    }
    return invoked;
}

void MenuBuilder::close(bool closeAllMenus) {
    if (mIsClosing) return;
    mIsClosing = true;
    for (int i = 0;i<mPresenters.size();i++) {
        MenuPresenter* presenter = mPresenters.at(i);
        if (presenter == nullptr) {
            mPresenters.erase(mPresenters.begin()+i);
        } else {
            presenter->onCloseMenu(this, closeAllMenus);
        }
    }
    mIsClosing = false;
}

void MenuBuilder::close() {
    close(true /* closeAllMenus */);
}

void MenuBuilder::onItemsChanged(bool structureChanged) {
    if (!mPreventDispatchingItemsChanged) {
        if (structureChanged) {
            mIsVisibleItemsStale = true;
            mIsActionItemsStale = true;
        }
        dispatchPresenterUpdate(structureChanged);
    } else {
        mItemsChangedWhileDispatchPrevented = true;
    }
}

void MenuBuilder::stopDispatchingItemsChanged() {
    if (!mPreventDispatchingItemsChanged) {
        mPreventDispatchingItemsChanged = true;
        mItemsChangedWhileDispatchPrevented = false;
    }
}

void MenuBuilder::startDispatchingItemsChanged() {
    mPreventDispatchingItemsChanged = false;
    if (mItemsChangedWhileDispatchPrevented) {
        mItemsChangedWhileDispatchPrevented = false;
        onItemsChanged(true);
    }
}

void MenuBuilder::onItemVisibleChanged(MenuItemImpl& item) {
    // Notify of items being changed
    mIsVisibleItemsStale = true;
    onItemsChanged(true);
}

void MenuBuilder::onItemActionRequestChanged(MenuItemImpl& item) {
    // Notify of items being changed
    mIsActionItemsStale = true;
    onItemsChanged(true);
}

std::vector<MenuItemImpl*> MenuBuilder::getVisibleItems() {
    if (!mIsVisibleItemsStale) return mVisibleItems;
    // Refresh the visible items
    mVisibleItems.clear();
    const int itemsSize = mItems.size();
    for (int i = 0; i < itemsSize; i++) {
        MenuItemImpl*item = mItems.at(i);
        if (item->isVisible()) mVisibleItems.push_back(item);
    }
    mIsVisibleItemsStale = false;
    mIsActionItemsStale = true;
    return mVisibleItems;
}

void MenuBuilder::flagActionItems() {
    std::vector<MenuItemImpl*> visibleItems = getVisibleItems();
    if (!mIsActionItemsStale) {
        return;
    }

    // Presenters flag action items as needed.
    bool flagged = false;
    for (auto it = mPresenters.begin();it!=mPresenters.end();) {
        MenuPresenter* presenter = *it;
        if (presenter == nullptr) {
            it = mPresenters.erase(it);
        } else {
            it++;
            flagged |= presenter->flagActionItems();
        }
    }
    if (flagged) {
        mActionItems.clear();
        mNonActionItems.clear();
        const int itemsSize = visibleItems.size();
        for (int i = 0; i < itemsSize; i++) {
            MenuItemImpl* item = visibleItems.at(i);
            if (item->isActionButton()) {
                mActionItems.push_back(item);
            } else {
                mNonActionItems.push_back(item);
            }
        }
    } else {
        // Nobody flagged anything, everything is a non-action item.
        // (This happens during a first pass with no action-item presenters.)
        mActionItems.clear();
        mNonActionItems.clear();
        mNonActionItems=getVisibleItems();
    }
    mIsActionItemsStale = false;
}

std::vector<MenuItemImpl*> MenuBuilder::getActionItems() {
    flagActionItems();
    return mActionItems;
}

std::vector<MenuItemImpl*> MenuBuilder::getNonActionItems() {
    flagActionItems();
    return mNonActionItems;
}

void MenuBuilder::clearHeader() {
    mHeaderIcon = nullptr;
    mHeaderTitle.clear();
    mHeaderView = nullptr;
    onItemsChanged(false);
}

void MenuBuilder::setHeaderInternal(const std::string& titleRes, const std::string& title, const std::string& iconRes,Drawable* icon, View* view) {
    if (view != nullptr) {
        mHeaderView = view;
        // If using a custom view, then the title and icon aren't used
        mHeaderTitle.clear();
        mHeaderIcon = nullptr;
    } else {
        if (!titleRes.empty()) {
            //mHeaderTitle = r.getText(titleRes);
        } else if (!title.empty()) {
            mHeaderTitle = title;
        }
        if (!iconRes.empty()) {
            mHeaderIcon = getContext()->getDrawable(iconRes);
        } else if (icon != nullptr) {
            mHeaderIcon = icon;
        }
        // If using the title or icon, then a custom view isn't used
        mHeaderView = nullptr;
    }

    // Notify of change
    onItemsChanged(false);
}

MenuBuilder& MenuBuilder::setHeaderTitleInt(const std::string& title) {
    setHeaderInternal("", title, "", nullptr, nullptr);
    return *this;
}

/*MenuBuilder& MenuBuilder::setHeaderTitleInt(const std::string& titleRes) {
    setHeaderInternal(titleRes, nullptr, "", nullptr, nullptr);
    return *this;
}*/

MenuBuilder& MenuBuilder::setHeaderIconInt(Drawable* icon) {
    setHeaderInternal("", nullptr, "", icon, nullptr);
    return *this;
}

MenuBuilder& MenuBuilder::setHeaderIconInt(const std::string& iconRes) {
    setHeaderInternal("", nullptr, iconRes, nullptr, nullptr);
    return *this;
}

MenuBuilder& MenuBuilder::setHeaderViewInt(View* view) {
    setHeaderInternal("", nullptr, "", nullptr, view);
    return *this;
}

std::string MenuBuilder::getHeaderTitle() {
    return mHeaderTitle;
}

Drawable* MenuBuilder::getHeaderIcon() {
    return mHeaderIcon;
}

View* MenuBuilder::getHeaderView() {
    return mHeaderView;
}

MenuBuilder* MenuBuilder::getRootMenu() {
    return this;
}

void MenuBuilder::setCurrentMenuInfo(ContextMenuInfo* menuInfo) {
    mCurrentMenuInfo = menuInfo;
}

void MenuBuilder::setOptionalIconsVisible(bool visible) {
    mOptionalIconsVisible = visible;
}

bool MenuBuilder::getOptionalIconsVisible() {
    return mOptionalIconsVisible;
}

bool MenuBuilder::expandItemActionView(MenuItemImpl* item) {
    bool expanded = false;
    if (mPresenters.empty()) return false;
    stopDispatchingItemsChanged();
    for (auto it = mPresenters.begin();it!=mPresenters.end();) {
        MenuPresenter* presenter = *it;
        if (presenter == nullptr) {
            it = mPresenters.erase(it);
        } else if ((expanded = presenter->expandItemActionView(*this, *item))) {
            break;
        }else{
            it++;
        }
    }
    startDispatchingItemsChanged();
    if (expanded) mExpandedItem = item;
    return expanded;
}

bool MenuBuilder::collapseItemActionView(MenuItemImpl* item) {
    if (mPresenters.empty() || (mExpandedItem != item)) return false;

    bool collapsed = false;

    stopDispatchingItemsChanged();
    for (auto it = mPresenters.begin();it!=mPresenters.end();) {
        MenuPresenter* presenter = *it;
        if (presenter == nullptr) {
            it = mPresenters.erase(it);
        } else if ((collapsed = presenter->collapseItemActionView(*this, *item))) {
            break;
        }else{
            it++;
        }
    }
    startDispatchingItemsChanged();
    if (collapsed) mExpandedItem = nullptr;
    return collapsed;
}

MenuItemImpl* MenuBuilder::getExpandedItem() {
    return mExpandedItem;
}
}/*endof namespace*/
