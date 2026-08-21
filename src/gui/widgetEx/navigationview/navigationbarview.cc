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
#include <widgetEx/navigationview/navigationbarview.h>
#include <widget/internal_R.h>
#include <widgetEx/navigationview/bottomnavigationview.h>
#include <widgetEx/widgetex_styleable.h>
#include <menu/menubuilder.h>
#include <menu/menuinflater.h>
#include <menu/menuitem.h>
#include <menu/menuitemimpl.h>
#include <widget/imageview.h>
#include <widget/textview.h>
#include <drawable/colorstatelist.h>
#include <core/typedarray.h>

namespace cdroid{
using namespace cdroid::internal;

NavigationBarView::NavigationBarView(Context* context, const AttributeSet* attrs, int defStyleAttr)
    : FrameLayout(context, attrs, defStyleAttr) {
    mItemSelectedListener = nullptr;
    mItemReselectedListener = nullptr;
    mItemBackground = nullptr;
    mItemIconSize = 0;
    mLabelVisibilityMode = LABEL_VISIBILITY_AUTO;
    mItemGravity = ITEM_GRAVITY_TOP_CENTER;
    mItemIconGravity = ITEM_ICON_GRAVITY_TOP;

    // AOSP NavigationBarView ctor: menu, menu view, presenter, then attrs.
    mMenu = new MenuBuilder(context);
    MenuBuilder::Callback cb;
    cb.onMenuItemSelected = [this](MenuBuilder&, MenuItem& item)->bool{
        return onMenuItemClick(&item);
    };
    mMenu->setCallback(cb);

    mMenuView = new LinearLayout(context, nullptr, 0);
    mMenuView->setOrientation(LinearLayout::HORIZONTAL);
    mMenuView->setPadding(12, 8, 12, 8);
    addView(mMenuView, new ViewGroup::LayoutParams(
            ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::WRAP_CONTENT));

    // NavigationBarView styleable (0x02 attrs via the GENERATED styleable).
    auto ta = context->obtainStyledAttributes(attrs,
            cdroid::internal::R::styleable::NavigationBarView, defStyleAttr);
    mItemIconTint = ta->getColorStateList(cdroid::internal::R::styleable::NavigationBarView_itemIconTint);
    mItemIconSize = ta->getDimensionPixelSize(cdroid::internal::R::styleable::NavigationBarView_itemIconSize, 0);
    mItemTextColor = ta->getColorStateList(cdroid::internal::R::styleable::NavigationBarView_itemTextColor);
    const int bgRes = ta->getResourceId(cdroid::internal::R::styleable::NavigationBarView_itemBackground, 0);
    if (bgRes) mItemBackground = context->getDrawable(bgRes);

    // AOSP calls the setters here, but they trigger updateMenuView → the
    // pure-virtual getMaxItemCount() while this base ctor still runs (Java's
    // ctor virtual dispatch has no C++ counterpart) — assign the fields
    // directly instead; the first menu inflation rebuilds the items anyway.
    mLabelVisibilityMode = ta->getInt(
            cdroid::internal::R::styleable::NavigationBarView_labelVisibilityMode,
            LABEL_VISIBILITY_AUTO);
    mItemIconGravity = ta->getInt(
            cdroid::internal::R::styleable::NavigationBarView_itemIconGravity,
            ITEM_ICON_GRAVITY_TOP);
    mItemGravity = ta->getInt(
            cdroid::internal::R::styleable::NavigationBarView_itemGravity,
            ITEM_GRAVITY_TOP_CENTER);

    if (ta->hasValue(cdroid::internal::R::styleable::NavigationBarView_menu)) {
        MenuInflater inflater(context);
        inflater.inflate(ta->getResourceId(cdroid::internal::R::styleable::NavigationBarView_menu, 0), mMenu);
    }
}

NavigationBarView::~NavigationBarView() {
    delete mMenu;
}

void NavigationBarView::setOnItemSelectedListener(OnItemSelectedListener* listener) {
    mItemSelectedListener = listener;
}

void NavigationBarView::setOnItemReselectedListener(OnItemReselectedListener* listener) {
    mItemReselectedListener = listener;
}

Menu* NavigationBarView::getMenu() {
    return mMenu;
}

ViewGroup* NavigationBarView::getMenuViewGroup() {
    return mMenuView;
}

void NavigationBarView::refreshMenuView() {
    updateMenuView();
}

const RefPtr<ColorStateList> NavigationBarView::getItemIconTintList() const {
    return mItemIconTint;
}

void NavigationBarView::setItemIconTintList(const RefPtr<ColorStateList>& tint) {
    mItemIconTint = tint;
    updateMenuView();
}

int NavigationBarView::getItemIconSize() const {
    return mItemIconSize;
}

void NavigationBarView::setItemIconSize(int iconSize) {
    mItemIconSize = iconSize;
    updateMenuView();
}

void NavigationBarView::setItemIconSizeRes(int iconSizeRes) {
    setItemIconSize(getContext()->getDimensionPixelSize(iconSizeRes));
}

const RefPtr<ColorStateList> NavigationBarView::getItemTextColor() const {
    return mItemTextColor;
}

void NavigationBarView::setItemTextColor(const RefPtr<ColorStateList>& textColor) {
    mItemTextColor = textColor;
    updateMenuView();
}

Drawable* NavigationBarView::getItemBackground() const {
    return mItemBackground;
}

void NavigationBarView::setItemBackgroundResource(int resId) {
    setItemBackground(getContext()->getDrawable(resId));
}

void NavigationBarView::setItemBackground(Drawable* background) {
    mItemBackground = background;
    updateMenuView();
}

int NavigationBarView::getLabelVisibilityMode() const {
    return mLabelVisibilityMode;
}

void NavigationBarView::setLabelVisibilityMode(int labelVisibilityMode) {
    mLabelVisibilityMode = labelVisibilityMode;
    updateMenuView();
}

int NavigationBarView::getItemGravity() const {
    return mItemGravity;
}

void NavigationBarView::setItemGravity(int itemGravity) {
    if (mItemGravity != itemGravity) {
        mItemGravity = itemGravity;
        updateMenuView();
    }
}

int NavigationBarView::getItemIconGravity() const {
    return mItemIconGravity;
}

void NavigationBarView::setItemIconGravity(int itemIconGravity) {
    if (mItemIconGravity != itemIconGravity) {
        mItemIconGravity = itemIconGravity;
        updateMenuView();
    }
}

// Icon-over-label button (material NavigationBarItemView simplified: no
// active indicator, no badge, no item animation).
View* NavigationBarView::createItemView(MenuItem* item) {
    Context* context = getContext();
    LinearLayout* column = new LinearLayout(context, nullptr, 0);
    // itemIconGravity selects the item layout configuration: TOP stacks the
    // icon over the label, START lays them out side by side (material's
    // horizontal item).
    const bool iconAtStart = (mItemIconGravity == ITEM_ICON_GRAVITY_START);
    column->setOrientation(iconAtStart ? LinearLayout::HORIZONTAL : LinearLayout::VERTICAL);
    // itemGravity positions the item content inside the item bounds.
    column->setGravity(mItemGravity);
    column->setPadding(8, 6, 8, 6);
    if (mItemBackground) {
        // Each item owns its background through View::mBackground. Never share the
        // template drawable between items: View destruction deletes its background.
        std::shared_ptr<Drawable::ConstantState> constantState = mItemBackground->getConstantState();
        Drawable* background = constantState ? constantState->newDrawable() : mItemBackground->mutate();
        const std::vector<int> state = item->isChecked()
            ? std::vector<int>{R::attr::state_checked} : std::vector<int>{};
        background->setState(state);
        column->setBackground(background);
    }
    column->setLayoutParams(new LinearLayout::LayoutParams(
            0, 56, 1.f));

    Drawable* icon = item->getIcon();
    if (icon) {
        ImageView* iconView = new ImageView(context, nullptr, 0);
        iconView->setImageDrawable(icon);
        if (mItemIconSize > 0) {
            iconView->setLayoutParams(new LinearLayout::LayoutParams(mItemIconSize, mItemIconSize));
        }
        if (mItemIconTint) {
            const std::vector<int> state = item->isChecked()
                ? std::vector<int>{R::attr::state_checked} : std::vector<int>{};
            iconView->setImageTintList(ColorStateList::valueOf(
                mItemIconTint->getColorForState(state, mItemIconTint->getDefaultColor())));
        }
        column->addView(iconView);
    }
    // LABEL_VISIBILITY_UNLABELED(2) hides the label; AUTO/SELECTED/LABELED show it.
    if (mLabelVisibilityMode != LABEL_VISIBILITY_UNLABELED) {
        TextView* label = new TextView(context, nullptr, 0);
        label->setText(item->getTitle());
        if (mItemTextColor) {
            const std::vector<int> state = item->isChecked()
                ? std::vector<int>{R::attr::state_checked} : std::vector<int>{};
            label->setTextColor(mItemTextColor->getColorForState(
                state,
                    mItemTextColor->getDefaultColor()));
        }
        label->setGravity(iconAtStart ? (Gravity::START | Gravity::CENTER_VERTICAL)
                                      : Gravity::CENTER_HORIZONTAL);
        column->addView(label);
    }

    column->setOnClickListener([this, item](View&) {
        onMenuItemClick(item);
    });
    return column;
}

bool NavigationBarView::onMenuItemClick(MenuItem* item) {
    if (mItemSelectedListener) {
        return mItemSelectedListener->onNavigationItemSelected(item);
    }
    return false;
}

void NavigationBarView::updateMenuView() {
    mMenuView->removeAllViews();
    const std::vector<MenuItemImpl*>& items = mMenu->getVisibleItems();
    const int max = getMaxItemCount();
    int added = 0;
    for (MenuItemImpl* item : items) {
        if (added >= max) break;  // AOSP: menus are capped at maxItemCount
        mMenuView->addView(createItemView(item));
        added++;
    }
}

}//namespace cdroid
