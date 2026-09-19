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
#include <widgetEx/navigationview/navigationbarmenuview.h>
#include <widgetEx/navigationview/navigationbarview.h>
#include <widgetEx/navigationview/navigationbarpresenter.h>
#include <menu/menubuilder.h>
#include <widget/internal_R.h>
#include <menu/menuitemimpl.h>
#include <cdlog.h>
#include <algorithm>

namespace cdroid{

NavigationBarMenuView::NavigationBarMenuView(Context* context, const AttributeSet* attrs)
    : ViewGroup(context, attrs)
    , mLabelVisibilityMode(NavigationBarView::LABEL_VISIBILITY_AUTO)
    , mItemIconGravity(NavigationBarView::ITEM_ICON_GRAVITY_TOP)
    , mItemIconSize(0)
    , mItemTextAppearanceInactive(0)
    , mItemTextAppearanceActive(0)
    , mItemTextAppearanceActiveBoldEnabled(false)
    , mItemBackground(nullptr)
    , mItemBackgroundRes(0)
    , mItemActiveIndicatorWidth(0)
    , mItemActiveIndicatorHeight(0)
    , mItemActiveIndicatorMarginHorizontal(0)
    , mItemGravity(NavigationBarView::ITEM_GRAVITY_TOP_CENTER)
    , mItemActiveIndicatorColor(0)
    , mPresenter(nullptr) {
    // The checked item's label scales past its bounds (material's item
    // layout disables clipping up the chain); let it overflow the bar row.
    setClipChildren(false);
    // AOSP: itemTextColorDefault = createDefaultColorStateList(textColorSecondary).
    mItemTextColorDefault = createDefaultColorStateList(
            (int)cdroid::internal::R::attr::textColorSecondary);

    mOnClickListener = [this](View& v){
        NavigationBarItemView* itemView = (NavigationBarItemView*)&v;
        MenuItem* item = itemView->getItemData();
        const bool result = mMenu->performItemAction(item, mPresenter, 0);
        if (item != nullptr && item->isCheckable() && (!result || item->isChecked())) {
            // If the item action was not invoked successfully (ie if there's no
            // listener) or if the item was checked through the action, update
            // the checked item.
            setCheckedItem(item);
        }
    };
}

NavigationBarMenuView::~NavigationBarMenuView() {
    // No-GC discipline: the pool and the live buttons are all owned here
    // (buildMenuView releases live buttons into the pool and deletes the
    // leftovers). ~ViewGroup frees the ATTACHED children; pooled (detached)
    // items are freed here - clear them from mButtons first so nothing is
    // double-referenced.
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr && item->getParent() == nullptr) {
            delete item;
        }
    }
    for (NavigationBarItemView* item : mItemPool) {
        delete item;
    }
    mButtons.clear();
    mItemPool.clear();
    delete mMenu;
    // mItemBackground is owned by NavigationBarView's setter side (shared
    // template); each item derived its own copy.
}

void NavigationBarMenuView::setCheckedItem(MenuItem* checkedItem) {
    if (mCheckedItem == checkedItem || !checkedItem->isCheckable()) {
        return;
    }
    // Unset the previous checked item
    if (mCheckedItem != nullptr && mCheckedItem->isChecked()) {
        mCheckedItem->setChecked(false);
    }
    checkedItem->setChecked(true);
    mCheckedItem = checkedItem;
}

void NavigationBarMenuView::setExpanded(bool expanded) {
    mExpanded = expanded;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setExpanded(expanded);
    }
}

bool NavigationBarMenuView::isExpanded() const {
    return mExpanded;
}

void NavigationBarMenuView::initialize(MenuBuilder* menu) {
    mMenu = new NavigationBarMenuBuilder(menu);
}

void NavigationBarMenuView::setIconTintList(const RefPtr<ColorStateList>& tint) {
    mItemIconTint = tint;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setIconTintList(tint);
    }
}

const RefPtr<ColorStateList> NavigationBarMenuView::getIconTintList() const {
    return mItemIconTint;
}

void NavigationBarMenuView::setItemIconSize(int iconSize) {
    mItemIconSize = iconSize;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setIconSize(iconSize);
    }
}

int NavigationBarMenuView::getItemIconSize() const {
    return mItemIconSize;
}

void NavigationBarMenuView::setItemTextColor(const RefPtr<ColorStateList>& color) {
    mItemTextColorFromUser = color;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setTextColor(color);
    }
}

const RefPtr<ColorStateList> NavigationBarMenuView::getItemTextColor() const {
    return mItemTextColorFromUser;
}

void NavigationBarMenuView::setItemTextAppearanceInactive(int textAppearanceRes) {
    mItemTextAppearanceInactive = textAppearanceRes;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setTextAppearanceInactive(textAppearanceRes);
    }
}

int NavigationBarMenuView::getItemTextAppearanceInactive() const {
    return mItemTextAppearanceInactive;
}

void NavigationBarMenuView::setItemTextAppearanceActive(int textAppearanceRes) {
    mItemTextAppearanceActive = textAppearanceRes;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setTextAppearanceActive(textAppearanceRes);
    }
}

void NavigationBarMenuView::setItemTextAppearanceActiveBoldEnabled(bool isBold) {
    mItemTextAppearanceActiveBoldEnabled = isBold;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setTextAppearanceActiveBoldEnabled(isBold);
    }
}

int NavigationBarMenuView::getItemTextAppearanceActive() const {
    return mItemTextAppearanceActive;
}

void NavigationBarMenuView::setItemBackgroundRes(int background) {
    mItemBackgroundRes = background;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setItemBackground(background);
    }
}

int NavigationBarMenuView::getItemBackgroundRes() const {
    return mItemBackgroundRes;
}

void NavigationBarMenuView::setItemBackground(Drawable* background) {
    mItemBackground = background;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setItemBackground(background);
    }
}

Drawable* NavigationBarMenuView::getItemBackground() const {
    if (!mButtons.empty() && mButtons.front() != nullptr) {
        return mButtons.front()->getBackground();
    }
    return mItemBackground;
}

void NavigationBarMenuView::setItemRippleColor(const RefPtr<ColorStateList>& itemRippleColor) {
    mItemRippleColor = itemRippleColor;
}

const RefPtr<ColorStateList> NavigationBarMenuView::getItemRippleColor() const {
    return mItemRippleColor;
}

void NavigationBarMenuView::setItemPaddingTop(int paddingTop) {
    mItemPaddingTop = paddingTop;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setItemPaddingTop(paddingTop);
    }
}

int NavigationBarMenuView::getItemPaddingTop() const {
    return mItemPaddingTop;
}

void NavigationBarMenuView::setItemPaddingBottom(int paddingBottom) {
    mItemPaddingBottom = paddingBottom;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setItemPaddingBottom(paddingBottom);
    }
}

int NavigationBarMenuView::getItemPaddingBottom() const {
    return mItemPaddingBottom;
}

void NavigationBarMenuView::setActiveIndicatorLabelPadding(int activeIndicatorLabelPadding) {
    mItemActiveIndicatorLabelPadding = activeIndicatorLabelPadding;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setActiveIndicatorLabelPadding(activeIndicatorLabelPadding);
    }
}

int NavigationBarMenuView::getActiveIndicatorLabelPadding() const {
    return mItemActiveIndicatorLabelPadding;
}

void NavigationBarMenuView::setIconLabelHorizontalSpacing(int iconLabelHorizontalSpacing) {
    mIconLabelHorizontalSpacing = iconLabelHorizontalSpacing;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setIconLabelHorizontalSpacing(iconLabelHorizontalSpacing);
    }
}

int NavigationBarMenuView::getIconLabelHorizontalSpacing() const {
    return mIconLabelHorizontalSpacing;
}

void NavigationBarMenuView::setItemActiveIndicatorEnabled(bool enabled) {
    mItemActiveIndicatorEnabled = enabled;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setActiveIndicatorEnabled(enabled);
    }
}

bool NavigationBarMenuView::getItemActiveIndicatorEnabled() const {
    return mItemActiveIndicatorEnabled;
}

void NavigationBarMenuView::setItemActiveIndicatorWidth(int width) {
    mItemActiveIndicatorWidth = width;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setActiveIndicatorWidth(width);
    }
}

void NavigationBarMenuView::setItemActiveIndicatorHeight(int height) {
    mItemActiveIndicatorHeight = height;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setActiveIndicatorHeight(height);
    }
}

void NavigationBarMenuView::setItemActiveIndicatorMarginHorizontal(int marginHorizontal) {
    mItemActiveIndicatorMarginHorizontal = marginHorizontal;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setActiveIndicatorMarginHorizontal(marginHorizontal);
    }
}

void NavigationBarMenuView::setItemGravity(int itemGravity) {
    mItemGravity = itemGravity;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setItemGravity(itemGravity);
    }
}

int NavigationBarMenuView::getItemGravity() const {
    return mItemGravity;
}

void NavigationBarMenuView::setItemActiveIndicatorResizeable(bool resizeable) {
    mItemActiveIndicatorResizeable = resizeable;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setActiveIndicatorResizeable(resizeable);
    }
}

void NavigationBarMenuView::setItemActiveIndicatorColor(int color) {
    mItemActiveIndicatorColor = color;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setActiveIndicatorColor(color);
    }
}

void NavigationBarMenuView::setLabelVisibilityMode(int labelVisibilityMode) {
    mLabelVisibilityMode = labelVisibilityMode;
}

int NavigationBarMenuView::getLabelVisibilityMode() const {
    return mLabelVisibilityMode;
}

void NavigationBarMenuView::setItemIconGravity(int itemIconGravity) {
    mItemIconGravity = itemIconGravity;
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) item->setItemIconGravity(itemIconGravity);
    }
}

int NavigationBarMenuView::getItemIconGravity() const {
    return mItemIconGravity;
}

void NavigationBarMenuView::setPresenter(NavigationBarPresenter* presenter) {
    mPresenter = presenter;
}

void NavigationBarMenuView::releaseItemPool() {
    for (NavigationBarItemView* item : mButtons) {
        if (item != nullptr) {
            mItemPool.push_back(item);
            item->clear();
        }
    }
}

NavigationBarItemView* NavigationBarMenuView::createMenuItem(int index, MenuItemImpl* item, bool shifting) {
    mPresenter->setUpdateSuspended(true);
    item->setCheckable(true);
    mPresenter->setUpdateSuspended(false);
    NavigationBarItemView* child = getNewItem();
    child->setShifting(shifting);
    child->setIconTintList(mItemIconTint);
    child->setIconSize(mItemIconSize);
    child->setTextColor(mItemTextColorDefault);
    child->setTextAppearanceInactive(mItemTextAppearanceInactive);
    child->setTextAppearanceActive(mItemTextAppearanceActive);
    child->setTextAppearanceActiveBoldEnabled(mItemTextAppearanceActiveBoldEnabled);
    child->setTextColor(mItemTextColorFromUser);
    if (mItemPaddingTop != NO_PADDING) {
        child->setItemPaddingTop(mItemPaddingTop);
    }
    if (mItemPaddingBottom != NO_PADDING) {
        child->setItemPaddingBottom(mItemPaddingBottom);
    }
    if (mItemActiveIndicatorLabelPadding != NO_PADDING) {
        child->setActiveIndicatorLabelPadding(mItemActiveIndicatorLabelPadding);
    }
    if (mIconLabelHorizontalSpacing != NO_PADDING) {
        child->setIconLabelHorizontalSpacing(mIconLabelHorizontalSpacing);
    }
    child->setActiveIndicatorWidth(mItemActiveIndicatorWidth);
    child->setActiveIndicatorHeight(mItemActiveIndicatorHeight);
    child->setActiveIndicatorMarginHorizontal(mItemActiveIndicatorMarginHorizontal);
    child->setItemGravity(mItemGravity);
    child->setActiveIndicatorColor(mItemActiveIndicatorColor);
    child->setActiveIndicatorResizeable(mItemActiveIndicatorResizeable);
    child->setActiveIndicatorEnabled(mItemActiveIndicatorEnabled);
    if (mItemBackground != nullptr) {
        child->setItemBackground(mItemBackground);
    } else {
        child->setItemBackground(mItemBackgroundRes);
    }
    child->setLabelVisibilityMode(mLabelVisibilityMode);
    child->setItemIconGravity(mItemIconGravity);
    child->setExpanded(mExpanded);
    child->initialize(item, 0);
    child->setItemPosition(index);
    const int itemId = item->getItemId();
    child->setOnClickListener(mOnClickListener);
    if (mSelectedItemId != Menu::NONE && itemId == mSelectedItemId) {
        mSelectedItemPosition = index;
    }
    return child;
}

void NavigationBarMenuView::buildMenuView() {
    // CDROID no-GC discipline on top of the AOSP flow: removeAllViews
    // detaches only, releaseItemPool parks the old buttons, and every pooled
    // item beyond the NEW pool size is deleted here (material relies on GC).
    // This is the exact leak fixed by this port: a full item-view set used to
    // leak on every refreshMenuView (window resume).
    std::vector<View*> oldChildren;
    for (int i = 0; i < getChildCount(); i++) {
        oldChildren.push_back(getChildAt(i));
    }
    removeAllViews();
    releaseItemPool();

    mPresenter->setUpdateSuspended(true);
    mMenu->refreshItems();
    mPresenter->setUpdateSuspended(false);

    const int contentItemCount = mMenu->getContentItemCount();
    if (contentItemCount == 0) {
        mSelectedItemId = 0;
        mSelectedItemPosition = 0;
        mButtons.clear();
        for (NavigationBarItemView* pooled : mItemPool) delete pooled;
        mItemPool.clear();
        mItemPoolSize = 0;
        for (View* old : oldChildren) delete old;
        return;
    }

    if ((int)mItemPool.size() > contentItemCount) {
        // Shrink the pool: delete the excess, keep the newest contentItemCount.
        const int excess = (int)mItemPool.size() - contentItemCount;
        for (int i = 0; i < excess; i++) {
            delete mItemPool.back();
            mItemPool.pop_back();
        }
    }
    mItemPoolSize = contentItemCount;

    const int menuSize = mMenu->size();
    mButtons.assign(menuSize, nullptr);
    const bool shifting = isShifting(mLabelVisibilityMode, getCurrentVisibleContentItemCount());
    for (int i = 0; i < menuSize; i++) {
        MenuItem* menuItem = mMenu->getItemAt(i);
        if (menuItem->hasSubMenu()) {
            // Subheader/submenu items: substrate renders the flattened
            // children as plain items (dividers/subheaders not ported).
            continue;
        }
        NavigationBarItemView* child = createMenuItem(i, (MenuItemImpl*)menuItem, shifting);
        if (menuItem->isCheckable() && mSelectedItemPosition == NO_SELECTED_ITEM) {
            mSelectedItemPosition = i;
        }
        mButtons[i] = child;
        addView(child);
    }
    mSelectedItemPosition = std::min(menuSize - 1, mSelectedItemPosition);
    if (mSelectedItemPosition >= 0 && mButtons[mSelectedItemPosition] != nullptr) {
        setCheckedItem(mButtons[mSelectedItemPosition]->getItemData());
    }
    // An old child is still live if it was REUSED this build (getNewItem
    // popped it from the pool and addView re-attached it - it is no longer in
    // mItemPool but has a parent again) or remains parked in the pool; only
    // the leftovers (detached and pool-less) are freed.
    for (View* old : oldChildren) {
        bool pooled = false;
        for (NavigationBarItemView* p : mItemPool) pooled |= (p == old);
        if (!pooled && old->getParent() == nullptr) delete old;
    }
}

bool NavigationBarMenuView::isMenuStructureSame() const {
    if (mButtons.empty() || mMenu == nullptr || mMenu->size() != (int)mButtons.size()) {
        return false;
    }
    for (int i = 0; i < (int)mButtons.size(); i++) {
        if (mButtons[i] == nullptr) return false;
    }
    return true;
}

void NavigationBarMenuView::updateMenuView() {
    if (mMenu == nullptr || mButtons.empty()) {
        return;
    }
    mPresenter->setUpdateSuspended(true);
    mMenu->refreshItems();
    mPresenter->setUpdateSuspended(false);

    if (!isMenuStructureSame()) {
        buildMenuView();
        return;
    }

    const int previousSelectedId = mSelectedItemId;
    const int menuSize = mMenu->size();
    for (int i = 0; i < menuSize; i++) {
        MenuItem* item = mMenu->getItemAt(i);
        if (item->isChecked()) {
            setCheckedItem(item);
            mSelectedItemId = item->getItemId();
            mSelectedItemPosition = i;
        }
    }

    const bool shifting = isShifting(mLabelVisibilityMode, getCurrentVisibleContentItemCount());
    for (int i = 0; i < menuSize; i++) {
        mPresenter->setUpdateSuspended(true);
        NavigationBarItemView* itemView = mButtons[i];
        itemView->setExpanded(mExpanded);
        itemView->setLabelVisibilityMode(mLabelVisibilityMode);
        itemView->setItemIconGravity(mItemIconGravity);
        itemView->setItemGravity(mItemGravity);
        itemView->setShifting(shifting);
        itemView->initialize((MenuItemImpl*)mMenu->getItemAt(i), 0);
        mPresenter->setUpdateSuspended(false);
    }
    (void)previousSelectedId; // material begins a label AutoTransition on
                              // selection change; the transition is not ported.
}

NavigationBarItemView* NavigationBarMenuView::getNewItem() {
    NavigationBarItemView* item = nullptr;
    if (!mItemPool.empty()) {
        item = mItemPool.back();
        mItemPool.pop_back();
    }
    if (item == nullptr) {
        item = createNavigationBarItemView(getContext());
    }
    return item;
}

int NavigationBarMenuView::getCollapsedVisibleItemCount() const {
    return std::min(mCollapsedMaxItemCount, mMenu->getVisibleMainContentItemCount());
}

int NavigationBarMenuView::getCurrentVisibleContentItemCount() const {
    return mExpanded ? mMenu->getVisibleContentItemCount() : getCollapsedVisibleItemCount();
}

int NavigationBarMenuView::getSelectedItemId() const {
    return mSelectedItemId;
}

void NavigationBarMenuView::tryRestoreSelectedItemId(int itemId) {
    const int size = mMenu->size();
    for (int i = 0; i < size; i++) {
        MenuItem* item = mMenu->getItemAt(i);
        if (itemId == item->getItemId()) {
            mSelectedItemId = itemId;
            mSelectedItemPosition = i;
            setCheckedItem(item);
            break;
        }
    }
}

RefPtr<ColorStateList> NavigationBarMenuView::createDefaultColorStateList(int baseColorThemeAttr) {
    TypedValue value;
    if (!getContext()->getTheme().resolveAttribute(baseColorThemeAttr, &value, true)) {
        return nullptr;
    }
    RefPtr<ColorStateList> baseColor = getContext()->getColorStateList(value.resourceId);
    if (baseColor == nullptr) {
        return nullptr;
    }
    if (!getContext()->getTheme().resolveAttribute(
            (int)cdroid::internal::R::attr::colorPrimary, &value, true)) {
        return nullptr;
    }
    const int colorPrimary = value.data;
    const int defaultColor = baseColor->getDefaultColor();
    const std::vector<std::vector<int>> states = {
        {-cdroid::internal::R::attr::state_enabled},              // DISABLED_STATE_SET
        { cdroid::internal::R::attr::state_checked},              // CHECKED_STATE_SET
        {},                                                         // EMPTY_STATE_SET
    };
    const std::vector<int> colors = {
        baseColor->getColorForState(states[0], defaultColor),
        colorPrimary,
        defaultColor,
    };
    return RefPtr<ColorStateList>(new ColorStateList(states, colors));
}

bool NavigationBarMenuView::isShifting(int labelVisibilityMode, int childCount) const {
    return labelVisibilityMode == NavigationBarView::LABEL_VISIBILITY_AUTO
        ? childCount > 3
        : labelVisibilityMode == NavigationBarView::LABEL_VISIBILITY_SELECTED;
}

int NavigationBarMenuView::getSelectedItemPosition() const {
    return mSelectedItemPosition;
}

NavigationBarMenuBuilder* NavigationBarMenuView::getMenu() const {
    return mMenu;
}

} // namespace cdroid
