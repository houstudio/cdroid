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
#include <view/view.h>
#include <menu/menu.h>
#include <menu/submenu.h>
#include <menu/menuitemimpl.h>
#include <menu/submenubuilder.h>
#include <view/actionprovider.h>
#include <widget/linearlayout.h>

namespace cdroid{

MenuItemImpl::MenuItemImpl(MenuBuilder* menu, int group, int id, int categoryOrder, int ordering,
        const std::string& title, int showAsAction) {
    mSubMenu = nullptr;
    mMenuInfo = nullptr;
    mActionView = nullptr;
    mIconDrawable = nullptr;
    mIconTintList = nullptr;
    mActionProvider = nullptr;
    mMenu = menu;
    mId = id;
    mGroup = group;
    mCategoryOrder = categoryOrder;
    mOrdering = ordering;
    mTitle = title;
    mShowAsAction = showAsAction;
    mShortcutNumericChar = 0;
    mShortcutAlphabeticChar = 0;
}

bool MenuItemImpl::invoke() {
    if ((mClickListener != nullptr) && mClickListener/*onMenuItemClick*/(*this)){
        return true;
    }

    if (mMenu->dispatchMenuItemSelected(*mMenu, *this)) {
        return true;
    }

    if (mItemCallback != nullptr) {
        mItemCallback();
        return true;
    }

    if (mIntent != nullptr) {
        try {
            //mMenu->getContext().startActivity(mIntent);
            return true;
        } catch (std::exception& e) {
            LOGE("Can't find activity to handle intent; ignoring %s", e.what());
        }
    }

    if ((mActionProvider != nullptr) && mActionProvider->onPerformDefaultAction()) {
        return true;
    }

    return false;
}

bool MenuItemImpl::isEnabled() const{
    return (mFlags & ENABLED) != 0;
}

MenuItem& MenuItemImpl::setEnabled(bool enabled) {
    if (enabled) {
        mFlags |= ENABLED;
    } else {
        mFlags &= ~ENABLED;
    }
    mMenu->onItemsChanged(false);
    return *this;
}

int MenuItemImpl::getGroupId()const{
    return mGroup;
}

int MenuItemImpl::getItemId()const{
    return mId;
}

int MenuItemImpl::getOrder()const{
    return mCategoryOrder;
}

int MenuItemImpl::getOrdering()const{
    return mOrdering;
}

Intent* MenuItemImpl::getIntent() {
    return mIntent;
}

MenuItem& MenuItemImpl::setIntent(Intent* intent) {
    mIntent = intent;
    return *this;
}

Runnable MenuItemImpl::getCallback() {
    return mItemCallback;
}

MenuItem& MenuItemImpl::setCallback(Runnable callback) {
    mItemCallback = callback;
    return *this;
}

int MenuItemImpl::getAlphabeticShortcut()const{
    return mShortcutAlphabeticChar;
}

int MenuItemImpl::getAlphabeticModifiers()const{
    return mShortcutAlphabeticModifiers;
}

MenuItem& MenuItemImpl::setAlphabeticShortcut(int alphaChar){
    if (mShortcutAlphabeticChar == alphaChar) return *this;

    mShortcutAlphabeticChar = std::tolower(alphaChar);
    mMenu->onItemsChanged(false);
    return *this;
}

MenuItem& MenuItemImpl::setAlphabeticShortcut(int alphaChar, int alphaModifiers){
    if ((mShortcutAlphabeticChar == alphaChar) &&
            (mShortcutAlphabeticModifiers == alphaModifiers)) {
        return *this;
    }

    mShortcutAlphabeticChar = std::tolower(alphaChar);
    mShortcutAlphabeticModifiers = KeyEvent::normalizeMetaState(alphaModifiers);
    mMenu->onItemsChanged(false);
    return *this;
}

int MenuItemImpl::getNumericShortcut() const{
    return mShortcutNumericChar;
}

int MenuItemImpl::getNumericModifiers() const{
    return mShortcutNumericModifiers;
}

MenuItem& MenuItemImpl::setNumericShortcut(int numericChar) {
    if (mShortcutNumericChar == numericChar) return *this;

    mShortcutNumericChar = numericChar;
    mMenu->onItemsChanged(false);
    return *this;
}

MenuItem& MenuItemImpl::setNumericShortcut(int numericChar, int numericModifiers){
    if ((mShortcutNumericChar == numericChar) && (mShortcutNumericModifiers == numericModifiers)) {
        return *this;
    }

    mShortcutNumericChar = numericChar;
    mShortcutNumericModifiers = KeyEvent::normalizeMetaState(numericModifiers);
    mMenu->onItemsChanged(false);
    return *this;
}

MenuItem& MenuItemImpl::setShortcut(int numericChar, int alphaChar){
    mShortcutNumericChar = numericChar;
    mShortcutAlphabeticChar = std::tolower(alphaChar);
    mMenu->onItemsChanged(false);
    return *this;
}

MenuItem& MenuItemImpl::setShortcut(int numericChar, int alphaChar, int numericModifiers,
        int alphaModifiers) {
    mShortcutNumericChar = numericChar;
    mShortcutNumericModifiers = KeyEvent::normalizeMetaState(numericModifiers);
    mShortcutAlphabeticChar = std::tolower(alphaChar);
    mShortcutAlphabeticModifiers = KeyEvent::normalizeMetaState(alphaModifiers);
    mMenu->onItemsChanged(false);

    return *this;
}

/**
 * @return The active shortcut (based on QWERTY-mode of the menu).
 */
int MenuItemImpl::getShortcut() const{
    return (mMenu->isQwertyMode() ? mShortcutAlphabeticChar : mShortcutNumericChar);
}

/**
 * @return The label to show for the shortcut. This includes the chording
 *         key (for example 'Menu+a'). Also, any non-human readable
 *         characters should be human readable (for example 'Menu+enter').
 */
std::string MenuItemImpl::getShortcutLabel() {

    int shortcut = getShortcut();
    if (shortcut == 0) {
        return "";
    }

    std::string sb;
#if 0
    final Resources res = mMenu.getContext().getResources();
    if (ViewConfiguration.get(mMenu->getContext()).hasPermanentMenuKey()) {
        // Only prepend "Menu+" if there is a hardware menu key.
        sb.append(res.getString(com.android.internal.R.string.prepend_shortcut_label));
    }

    const int modifiers =
        mMenu->isQwertyMode() ? mShortcutAlphabeticModifiers : mShortcutNumericModifiers;
    appendModifier(sb, modifiers, KeyEvent::META_META_ON, res.getString(
        com.android.internal.R.string.menu_meta_shortcut_label));
    appendModifier(sb, modifiers, KeyEvent::META_CTRL_ON, res.getString(
        com.android.internal.R.string.menu_ctrl_shortcut_label));
    appendModifier(sb, modifiers, KeyEvent::META_ALT_ON, res.getString(
        com.android.internal.R.string.menu_alt_shortcut_label));
    appendModifier(sb, modifiers, KeyEvent::META_SHIFT_ON, res.getString(
        com.android.internal.R.string.menu_shift_shortcut_label));
    appendModifier(sb, modifiers, KeyEvent::META_SYM_ON, res.getString(
        com.android.internal.R.string.menu_sym_shortcut_label));
    appendModifier(sb, modifiers, KeyEvent::META_FUNCTION_ON, res.getString(
        com.android.internal.R.string.menu_function_shortcut_label));

    switch (shortcut) {
        case '\n':
            sb.append(res.getString(
                com.android.internal.R.string.menu_enter_shortcut_label));
            break;
        case '\b':
            sb.append(res.getString(
                com.android.internal.R.string.menu_delete_shortcut_label));
            break;
        case ' ':
            sb.append(res.getString(
                com.android.internal.R.string.menu_space_shortcut_label));
            break;
        default:
            sb.append(shortcut);
            break;
    }
#endif
    return sb;
}

void MenuItemImpl::appendModifier(std::string& sb, int mask, int modifier,const std::string& label) {
    if ((mask & modifier) == modifier) {
        sb.append(label);
    }
}

bool MenuItemImpl::shouldShowShortcut() {
    // Show shortcuts if the menu is supposed to show shortcuts AND this item has a shortcut
    return mMenu->isShortcutsVisible() && (getShortcut() != 0);
}

SubMenu* MenuItemImpl::getSubMenu() {
    return (SubMenu*)mSubMenu;
}

bool MenuItemImpl::hasSubMenu() const{
    return mSubMenu != nullptr;
}

void  MenuItemImpl::setSubMenu(SubMenuBuilder* subMenu) {
    mSubMenu = subMenu;
    subMenu->setHeaderTitle(getTitle());
}

std::string MenuItemImpl::getTitle() {
    return mTitle;
}

std::string MenuItemImpl::getTitleForItemView(MenuView::ItemView* itemView) {
    return ((itemView != nullptr) && itemView->prefersCondensedTitle())
            ? getTitleCondensed(): getTitle();
}

MenuItem& MenuItemImpl::setTitle(const std::string& title) {
    mTitle = title;
    mMenu->onItemsChanged(false);
    if (mSubMenu != nullptr) {
        mSubMenu->setHeaderTitle(title);
    }
    return *this;
}

std::string MenuItemImpl::getTitleCondensed() {
    return !mTitleCondensed.empty() ? mTitleCondensed : mTitle;
}

MenuItem& MenuItemImpl::setTitleCondensed(const std::string& title) {
    mTitleCondensed = title;
    // Could use getTitle() in the loop below, but just cache what it would do here
    mMenu->onItemsChanged(false);
    return *this;
}

Drawable* MenuItemImpl::getIcon() {
    if (mIconDrawable != nullptr) {
        return applyIconTintIfNecessary(mIconDrawable);
    }

    if (!mIconResId.empty()) {
        Drawable* icon =  mMenu->getContext()->getDrawable(mIconResId);
        mIconResId.clear();
        mIconDrawable = icon;
        return applyIconTintIfNecessary(icon);
    }
    return nullptr;
}

MenuItem& MenuItemImpl::setIcon(Drawable* icon) {
    mIconResId = NO_ICON;
    mIconDrawable = icon;
    mNeedToApplyIconTint = true;
    mMenu->onItemsChanged(false);
    return *this;
}

MenuItem& MenuItemImpl::setIcon(const std::string& iconResId) {
    mIconDrawable = nullptr;
    mIconResId = iconResId;
    mNeedToApplyIconTint = true;

    // If we have a view, we need to push the Drawable to them
    mMenu->onItemsChanged(false);
    return *this;
}

MenuItem& MenuItemImpl::setIconTintList(const ColorStateList* iconTintList) {
    mIconTintList = iconTintList;
    mHasIconTint = true;
    mNeedToApplyIconTint = true;
    mMenu->onItemsChanged(false);
    return *this;
}

const ColorStateList* MenuItemImpl::getIconTintList() {
    return mIconTintList;
}

MenuItem& MenuItemImpl::setIconTintMode(int iconTintMode) {
    mIconTintMode = iconTintMode;
    mHasIconTintMode = true;
    mNeedToApplyIconTint = true;
    mMenu->onItemsChanged(false);

    return *this;
}

int MenuItemImpl::getIconTintMode()const {
    return mIconTintMode;
}

Drawable* MenuItemImpl::applyIconTintIfNecessary(Drawable* icon) {
    if ((icon != nullptr) && mNeedToApplyIconTint && (mHasIconTint || mHasIconTintMode)) {
        icon = icon->mutate();
        if (mHasIconTint) {
            icon->setTintList(mIconTintList);
        }
        if (mHasIconTintMode) {
            icon->setTintMode(mIconTintMode);
        }
        mNeedToApplyIconTint = false;
    }

    return icon;
}

bool MenuItemImpl::isCheckable() const{
    return (mFlags & CHECKABLE) == CHECKABLE;
}

MenuItem& MenuItemImpl::setCheckable(bool checkable) {
    const int oldFlags = mFlags;
    mFlags = (mFlags & ~CHECKABLE) | (checkable ? CHECKABLE : 0);
    if (oldFlags != mFlags) {
        mMenu->onItemsChanged(false);
    }
    return *this;
}

void MenuItemImpl::setExclusiveCheckable(bool exclusive) {
    mFlags = (mFlags & ~EXCLUSIVE) | (exclusive ? EXCLUSIVE : 0);
}

bool MenuItemImpl::isExclusiveCheckable() const{
    return (mFlags & EXCLUSIVE) != 0;
}

bool MenuItemImpl::isChecked() const{
    return (mFlags & CHECKED) == CHECKED;
}

MenuItem& MenuItemImpl::setChecked(bool checked) {
    if ((mFlags & EXCLUSIVE) != 0) {
        // Call the method on the Menu since it knows about the others in this
        // exclusive checkable group
        mMenu->setExclusiveItemChecked(*this);
    } else {
        setCheckedInt(checked);
    }
    return *this;
}

void MenuItemImpl::setCheckedInt(bool checked) {
    const int oldFlags = mFlags;
    mFlags = (mFlags & ~CHECKED) | (checked ? CHECKED : 0);
    if (oldFlags != mFlags) {
        mMenu->onItemsChanged(false);
    }
}

bool MenuItemImpl::isVisible() const{
    if (mActionProvider != nullptr && mActionProvider->overridesItemVisibility()) {
        return (mFlags & HIDDEN) == 0 && mActionProvider->isVisible();
    }
    return (mFlags & HIDDEN) == 0;
}

/**
 * Changes the visibility of the item. This method DOES NOT notify the
 * parent menu of a change in this item, so this should only be called from
 * methods that will eventually trigger this change.  If unsure, use {@link #setVisible(bool)}
 * instead.
 *
 * @param shown Whether to show (true) or hide (false).
 * @return Whether the item's shown state was changed
 */
bool MenuItemImpl::setVisibleInt(bool shown) {
    const int oldFlags = mFlags;
    mFlags = (mFlags & ~HIDDEN) | (shown ? 0 : HIDDEN);
    return oldFlags != mFlags;
}

MenuItem& MenuItemImpl::setVisible(bool shown) {
    // Try to set the shown state to the given state. If the shown state was changed
    // (i.e. the previous state isn't the same as given state), notify the parent menu that
    // the shown state has changed for this item
    if (setVisibleInt(shown)) mMenu->onItemVisibleChanged(*this);

    return *this;
}

MenuItem& MenuItemImpl::setOnMenuItemClickListener(const MenuItem::OnMenuItemClickListener& clickListener) {
    mClickListener = clickListener;
    return *this;
}

void MenuItemImpl::setMenuInfo(ContextMenuInfo* menuInfo) {
    mMenuInfo = menuInfo;
}

ContextMenuInfo* MenuItemImpl::getMenuInfo() {
    return mMenuInfo;
}

void MenuItemImpl::actionFormatChanged() {
    mMenu->onItemActionRequestChanged(*this);
}

/**
 * @return Whether the menu should show icons for menu items.
 */
bool MenuItemImpl::shouldShowIcon() const{
    return mMenu->getOptionalIconsVisible();
}

bool MenuItemImpl::isActionButton() const{
    return (mFlags & IS_ACTION) == IS_ACTION;
}

bool MenuItemImpl::requestsActionButton() {
    return (mShowAsAction & SHOW_AS_ACTION_IF_ROOM) == SHOW_AS_ACTION_IF_ROOM;
}

bool MenuItemImpl::requiresActionButton() {
    return (mShowAsAction & SHOW_AS_ACTION_ALWAYS) == SHOW_AS_ACTION_ALWAYS;
}

bool MenuItemImpl::requiresOverflow() {
    return !requiresActionButton() && !requestsActionButton();
}

void MenuItemImpl::setIsActionButton(bool isActionButton) {
    if (isActionButton) {
        mFlags |= IS_ACTION;
    } else {
        mFlags &= ~IS_ACTION;
    }
}

bool MenuItemImpl::showsTextAsAction() {
    return (mShowAsAction & SHOW_AS_ACTION_WITH_TEXT) == SHOW_AS_ACTION_WITH_TEXT;
}

void MenuItemImpl::setShowAsAction(int actionEnum) {
    switch (actionEnum & SHOW_AS_ACTION_MASK) {
    case SHOW_AS_ACTION_ALWAYS:
    case SHOW_AS_ACTION_IF_ROOM:
    case SHOW_AS_ACTION_NEVER:
        break;// Looks good!
    default:
        // Mutually exclusive options selected!
        throw std::invalid_argument("SHOW_AS_ACTION_ALWAYS, SHOW_AS_ACTION_IF_ROOM,"
                " and SHOW_AS_ACTION_NEVER are mutually exclusive.");
    }
    mShowAsAction = actionEnum;
    mMenu->onItemActionRequestChanged(*this);
}

MenuItem& MenuItemImpl::setActionView(View* view) {
    mActionView = view;
    mActionProvider = nullptr;
    if (view != nullptr && view->getId() == View::NO_ID && mId > 0) {
        view->setId(mId);
    }
    mMenu->onItemActionRequestChanged(*this);
    return *this;
}

MenuItem& MenuItemImpl::setActionView(const std::string& resId) {
    Context* context = mMenu->getContext();
    LayoutInflater* inflater = LayoutInflater::from(context);
    LinearLayout*ll = new LinearLayout(context,AttributeSet(context,"cdroid"));
    setActionView(inflater->inflate(resId, ll, false));
    return *this;
}

View* MenuItemImpl::getActionView() {
    if (mActionView != nullptr) {
        return mActionView;
    } else if (mActionProvider != nullptr) {
        mActionView = mActionProvider->onCreateActionView(*this);
        return mActionView;
    } else {
        return nullptr;
    }
}

ActionProvider* MenuItemImpl::getActionProvider() {
    return mActionProvider;
}

MenuItem& MenuItemImpl::setActionProvider(ActionProvider* actionProvider) {
    if (mActionProvider != nullptr) {
        mActionProvider->reset();
    }
    mActionView = nullptr;
    mActionProvider = actionProvider;
    mMenu->onItemsChanged(true); // Measurement can be changed
    if (mActionProvider != nullptr) {
        ActionProvider::VisibilityListener ls=[this](bool isVisible){
            mMenu->onItemVisibleChanged(*this);
        };
        mActionProvider->setVisibilityListener(ls);
    }
    return *this;
}

MenuItem& MenuItemImpl::setShowAsActionFlags(int actionEnum) {
    setShowAsAction(actionEnum);
    return *this;
}

bool MenuItemImpl::expandActionView() {
    if (!hasCollapsibleActionView()) {
        return false;
    }

    if ((mOnActionExpandListener.onMenuItemActionExpand == nullptr) ||
            mOnActionExpandListener.onMenuItemActionExpand(*this)) {
        return mMenu->expandItemActionView(this);
    }

    return false;
}

bool MenuItemImpl::collapseActionView() {
    if ((mShowAsAction & SHOW_AS_ACTION_COLLAPSE_ACTION_VIEW) == 0) {
        return false;
    }
    if (mActionView == nullptr) {
        // We're already collapsed if we have no action view.
        return true;
    }

    if ((mOnActionExpandListener.onMenuItemActionCollapse == nullptr) ||
            mOnActionExpandListener.onMenuItemActionCollapse(*this)) {
        return mMenu->collapseItemActionView(this);
    }
    return false;
}

MenuItem& MenuItemImpl::setOnActionExpandListener(const OnActionExpandListener& listener) {
    mOnActionExpandListener = listener;
    return *this;
}

bool MenuItemImpl::hasCollapsibleActionView() {
    if ((mShowAsAction & SHOW_AS_ACTION_COLLAPSE_ACTION_VIEW) != 0) {
        if ((mActionView == nullptr) && (mActionProvider != nullptr)) {
            mActionView = mActionProvider->onCreateActionView(*this);
        }
        return mActionView != nullptr;
    }
    return false;
}

void MenuItemImpl::setActionViewExpanded(bool isExpanded) {
    mIsActionViewExpanded = isExpanded;
    mMenu->onItemsChanged(false);
}

bool MenuItemImpl::isActionViewExpanded()const{
    return mIsActionViewExpanded;
}

MenuItem& MenuItemImpl::setContentDescription(const std::string&contentDescription) {
    mContentDescription = contentDescription;
    mMenu->onItemsChanged(false);
    return *this;
}

std::string MenuItemImpl::getContentDescription() {
    return mContentDescription;
}

MenuItem& MenuItemImpl::setTooltipText(const std::string& tooltipText) {
    mTooltipText = tooltipText;
    mMenu->onItemsChanged(false);
    return *this;
}

std::string MenuItemImpl::getTooltipText() {
    return mTooltipText;
}
}/*endof namespace*/
