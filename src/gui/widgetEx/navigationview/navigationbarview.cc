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
#include <widgetEx/navigationview/navigationbarmenuview.h>
#include <widgetEx/navigationview/navigationbarpresenter.h>
#include <widget/internal_R.h>
#include <widgetEx/widgetex_styleable.h>
#include <menu/menubuilder.h>
#include <menu/menuinflater.h>
#include <menu/menuitem.h>
#include <drawable/colorstatelist.h>
#include <content/typedarray.h>

namespace cdroid{
using namespace cdroid::internal;

NavigationBarView::NavigationBarView(Context* context, const AttributeSet* attrs, int defStyleAttr)
    : FrameLayout(context, attrs, defStyleAttr) {
    // Checked labels scale past item bounds; don't clip at the bar either.
    setClipChildren(false);
    mItemSelectedListener = nullptr;
    mItemReselectedListener = nullptr;
    mPresenter = nullptr;
    mMenuView = nullptr;
    mItemBackground = nullptr;
    mItemBackgroundRes = 0;
    mItemIconSize = 0;
    mLabelVisibilityMode = LABEL_VISIBILITY_AUTO;
    mItemGravity = ITEM_GRAVITY_TOP_CENTER;
    mItemIconGravity = ITEM_ICON_GRAVITY_TOP;
    mItemTextAppearanceInactive = 0;
    mItemTextAppearanceActive = 0;
    mHorizontalItemTextAppearanceInactive = 0;
    mHorizontalItemTextAppearanceActive = 0;
    mItemTextAppearanceActiveBoldEnabled = true;
    mItemRippleColor = nullptr;
    mItemPaddingTop = -1;
    mItemPaddingBottom = -1;
    mActiveIndicatorLabelPadding = -1;
    mIconLabelHorizontalSpacing = -1;
    mActiveIndicatorWidth = 0;
    mActiveIndicatorHeight = 0;
    mActiveIndicatorMarginX = 0;
    mActiveIndicatorColor = 0;

    // AOSP NavigationBarView ctor: menu, presenter, menu view, then attrs.
    mMenu = new MenuBuilder(context);
    MenuBuilder::Callback cb;
    cb.onMenuItemSelected = [this](MenuBuilder&, MenuItem& item)->bool{
        return onMenuItemClick(&item);
    };
    mMenu->setCallback(cb);
    mPresenter = new NavigationBarPresenter();

    // NavigationBarView styleable (0x02 attrs via the GENERATED styleable).
    auto ta = context->obtainStyledAttributes(attrs,
            cdroid::internal::R::styleable::NavigationBarView, defStyleAttr);
    mItemIconTint = ta->getColorStateList(cdroid::internal::R::styleable::NavigationBarView_itemIconTint);
    mItemIconSize = ta->getDimensionPixelSize(cdroid::internal::R::styleable::NavigationBarView_itemIconSize, 0);
    mItemTextColor = ta->getColorStateList(cdroid::internal::R::styleable::NavigationBarView_itemTextColor);
    const int bgRes = ta->getResourceId(cdroid::internal::R::styleable::NavigationBarView_itemBackground, 0);
    if (bgRes) mItemBackground = context->getDrawable(bgRes);

    // AOSP calls the setters here; C++ cannot dispatch to the derived menu
    // view from this base ctor, so cache the fields - installMenuView() pushes
    // them in AOSP setter order once the subclass provides the view.
    mLabelVisibilityMode = ta->getInt(
            cdroid::internal::R::styleable::NavigationBarView_labelVisibilityMode,
            LABEL_VISIBILITY_AUTO);
    mItemIconGravity = ta->getInt(
            cdroid::internal::R::styleable::NavigationBarView_itemIconGravity,
            ITEM_ICON_GRAVITY_TOP);
    mItemGravity = ta->getInt(
            cdroid::internal::R::styleable::NavigationBarView_itemGravity,
            ITEM_GRAVITY_TOP_CENTER);

    // Text appearances (0 = none). The horizontal variants only apply to the
    // icon-start layout and fall back to the vertical ones.
    mItemTextAppearanceInactive = ta->getResourceId(
            cdroid::internal::R::styleable::NavigationBarView_itemTextAppearanceInactive, 0);
    mItemTextAppearanceActive = ta->getResourceId(
            cdroid::internal::R::styleable::NavigationBarView_itemTextAppearanceActive, 0);
    mHorizontalItemTextAppearanceInactive = ta->getResourceId(
            cdroid::internal::R::styleable::NavigationBarView_horizontalItemTextAppearanceInactive,
            mItemTextAppearanceInactive);
    mHorizontalItemTextAppearanceActive = ta->getResourceId(
            cdroid::internal::R::styleable::NavigationBarView_horizontalItemTextAppearanceActive,
            mItemTextAppearanceActive);
    mItemTextAppearanceActiveBoldEnabled = ta->getBoolean(
            cdroid::internal::R::styleable::NavigationBarView_itemTextAppearanceActiveBoldEnabled, true);
    mItemRippleColor = ta->getColorStateList(
            cdroid::internal::R::styleable::NavigationBarView_itemRippleColor);
    mItemPaddingTop = ta->getDimensionPixelSize(
            cdroid::internal::R::styleable::NavigationBarView_itemPaddingTop, -1);
    mItemPaddingBottom = ta->getDimensionPixelSize(
            cdroid::internal::R::styleable::NavigationBarView_itemPaddingBottom, -1);
    mActiveIndicatorLabelPadding = ta->getDimensionPixelSize(
            cdroid::internal::R::styleable::NavigationBarView_activeIndicatorLabelPadding, -1);
    mIconLabelHorizontalSpacing = ta->getDimensionPixelSize(
            cdroid::internal::R::styleable::NavigationBarView_iconLabelHorizontalSpacing, -1);

    // itemActiveIndicatorStyle: a style holding the pill geometry (simplified
    // from material's shape-based BottomNavigationActiveIndicator).
    const int activeIndicatorStyle = ta->getResourceId(
            cdroid::internal::R::styleable::NavigationBarView_itemActiveIndicatorStyle, 0);
    if (activeIndicatorStyle != 0) {
        auto ai = context->obtainStyledAttributes(activeIndicatorStyle,
                cdroid::internal::R::styleable::BottomNavigationActiveIndicator);
        mActiveIndicatorWidth = ai->getDimensionPixelSize(
                cdroid::internal::R::styleable::BottomNavigationActiveIndicator_width, 0);
        mActiveIndicatorHeight = ai->getDimensionPixelSize(
                cdroid::internal::R::styleable::BottomNavigationActiveIndicator_height, 0);
        mActiveIndicatorMarginX = ai->getDimensionPixelSize(
                cdroid::internal::R::styleable::BottomNavigationActiveIndicator_marginX, 0);
        mActiveIndicatorColor = ai->getColor(
                cdroid::internal::R::styleable::BottomNavigationActiveIndicator_color, 0);
    }

    if (ta->hasValue(cdroid::internal::R::styleable::NavigationBarView_menu)) {
        MenuInflater inflater(context);
        inflater.inflate(ta->getResourceId(cdroid::internal::R::styleable::NavigationBarView_menu, 0), mMenu);
    }
}

void NavigationBarView::installMenuView(NavigationBarMenuView* menuView) {
    mMenuView = menuView;
    mMenuView->setPresenter(mPresenter);
    mPresenter->setMenuView(mMenuView);
    mMenu->addMenuPresenter(mPresenter, getContext());   // initForMenu -> initialize
    addView(mMenuView, new FrameLayout::LayoutParams(
            LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT, Gravity::CENTER));

    // Push the ctor-cached presentation in AOSP setter order (material's ctor
    // calls these setters directly on the menu view).
    const int iconAtStart = (mItemIconGravity == ITEM_ICON_GRAVITY_START);
    mMenuView->setIconTintList(mItemIconTint);
    mMenuView->setItemIconSize(mItemIconSize);
    mMenuView->setItemTextAppearanceInactive(iconAtStart
            ? mHorizontalItemTextAppearanceInactive : mItemTextAppearanceInactive);
    mMenuView->setItemTextAppearanceActive(iconAtStart
            ? mHorizontalItemTextAppearanceActive : mItemTextAppearanceActive);
    mMenuView->setItemTextAppearanceActiveBoldEnabled(mItemTextAppearanceActiveBoldEnabled);
    mMenuView->setItemTextColor(mItemTextColor);
    if (mItemPaddingTop != -1) mMenuView->setItemPaddingTop(mItemPaddingTop);
    if (mItemPaddingBottom != -1) mMenuView->setItemPaddingBottom(mItemPaddingBottom);
    if (mActiveIndicatorLabelPadding != -1) mMenuView->setActiveIndicatorLabelPadding(mActiveIndicatorLabelPadding);
    if (mIconLabelHorizontalSpacing != -1) mMenuView->setIconLabelHorizontalSpacing(mIconLabelHorizontalSpacing);
    mMenuView->setItemActiveIndicatorWidth(mActiveIndicatorWidth);
    mMenuView->setItemActiveIndicatorHeight(mActiveIndicatorHeight);
    mMenuView->setItemActiveIndicatorMarginHorizontal(mActiveIndicatorMarginX);
    mMenuView->setItemActiveIndicatorColor(mActiveIndicatorColor);
    mMenuView->setItemActiveIndicatorEnabled(
            mActiveIndicatorWidth > 0 && mActiveIndicatorHeight > 0);
    if (mItemBackground != nullptr) mMenuView->setItemBackground(mItemBackground);
    else mMenuView->setItemBackgroundRes(mItemBackgroundRes);
    mMenuView->setItemGravity(mItemGravity);
    mMenuView->setItemIconGravity(mItemIconGravity);
    mMenuView->setLabelVisibilityMode(mLabelVisibilityMode);

    // AOSP: the ctor ends with the menu inflated; the first build renders it.
    if (mMenu->size() > 0) {
        mMenuView->buildMenuView();
    }
}

NavigationBarView::~NavigationBarView() {
    // mMenuView is a child view (freed with the tree); the presenter is the
    // only extra owner object on this side.
    delete mPresenter;
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
    if (mMenuView) mMenuView->setIconTintList(tint);
}

int NavigationBarView::getItemIconSize() const {
    return mItemIconSize;
}

void NavigationBarView::setItemIconSize(int iconSize) {
    mItemIconSize = iconSize;
    if (mMenuView) mMenuView->setItemIconSize(iconSize);
}

void NavigationBarView::setItemIconSizeRes(int iconSizeRes) {
    setItemIconSize(getContext()->getDimensionPixelSize(iconSizeRes));
}

const RefPtr<ColorStateList> NavigationBarView::getItemTextColor() const {
    return mItemTextColor;
}

void NavigationBarView::setItemTextColor(const RefPtr<ColorStateList>& textColor) {
    mItemTextColor = textColor;
    if (mMenuView) mMenuView->setItemTextColor(textColor);
}

Drawable* NavigationBarView::getItemBackground() const {
    return mItemBackground;
}

void NavigationBarView::setItemBackgroundResource(int resId) {
    mItemBackgroundRes = resId;
    setItemBackground(getContext()->getDrawable(resId));
}

void NavigationBarView::setItemBackground(Drawable* background) {
    mItemBackground = background;
    if (mMenuView) mMenuView->setItemBackground(background);
}

int NavigationBarView::getLabelVisibilityMode() const {
    return mLabelVisibilityMode;
}

void NavigationBarView::setLabelVisibilityMode(int labelVisibilityMode) {
    mLabelVisibilityMode = labelVisibilityMode;
    if (mMenuView) mMenuView->setLabelVisibilityMode(labelVisibilityMode);
}

int NavigationBarView::getItemGravity() const {
    return mItemGravity;
}

void NavigationBarView::setItemGravity(int itemGravity) {
    if (mItemGravity != itemGravity) {
        mItemGravity = itemGravity;
        if (mMenuView) mMenuView->setItemGravity(itemGravity);
    }
}

int NavigationBarView::getItemIconGravity() const {
    return mItemIconGravity;
}

void NavigationBarView::setItemIconGravity(int itemIconGravity) {
    if (mItemIconGravity != itemIconGravity) {
        mItemIconGravity = itemIconGravity;
        if (mMenuView) mMenuView->setItemIconGravity(itemIconGravity);
    }
}

int NavigationBarView::getItemTextAppearanceInactive() const {
    return mItemTextAppearanceInactive;
}

void NavigationBarView::setItemTextAppearanceInactive(int textAppearanceRes) {
    if (mItemTextAppearanceInactive != textAppearanceRes) {
        mItemTextAppearanceInactive = textAppearanceRes;
        if (mMenuView) mMenuView->setItemTextAppearanceInactive(textAppearanceRes);
    }
}

int NavigationBarView::getItemTextAppearanceActive() const {
    return mItemTextAppearanceActive;
}

void NavigationBarView::setItemTextAppearanceActive(int textAppearanceRes) {
    if (mItemTextAppearanceActive != textAppearanceRes) {
        mItemTextAppearanceActive = textAppearanceRes;
        if (mMenuView) mMenuView->setItemTextAppearanceActive(textAppearanceRes);
    }
}

void NavigationBarView::setItemTextAppearanceActiveBoldEnabled(bool isBold) {
    if (mItemTextAppearanceActiveBoldEnabled != isBold) {
        mItemTextAppearanceActiveBoldEnabled = isBold;
        if (mMenuView) mMenuView->setItemTextAppearanceActiveBoldEnabled(isBold);
    }
}

const RefPtr<ColorStateList> NavigationBarView::getItemRippleColor() const {
    return mItemRippleColor;
}

void NavigationBarView::setItemRippleColor(const RefPtr<ColorStateList>& itemRippleColor) {
    if (mItemRippleColor != itemRippleColor) {
        mItemRippleColor = itemRippleColor;
        if (mMenuView) mMenuView->setItemRippleColor(itemRippleColor);
    }
}

int NavigationBarView::getItemPaddingTop() const {
    return mItemPaddingTop;
}

void NavigationBarView::setItemPaddingTop(int paddingTop) {
    if (mItemPaddingTop != paddingTop) {
        mItemPaddingTop = paddingTop;
        if (mMenuView) mMenuView->setItemPaddingTop(paddingTop);
    }
}

int NavigationBarView::getItemPaddingBottom() const {
    return mItemPaddingBottom;
}

void NavigationBarView::setItemPaddingBottom(int paddingBottom) {
    if (mItemPaddingBottom != paddingBottom) {
        mItemPaddingBottom = paddingBottom;
        if (mMenuView) mMenuView->setItemPaddingBottom(paddingBottom);
    }
}

int NavigationBarView::getIconLabelHorizontalSpacing() const {
    return mIconLabelHorizontalSpacing;
}

void NavigationBarView::setIconLabelHorizontalSpacing(int spacing) {
    if (mIconLabelHorizontalSpacing != spacing) {
        mIconLabelHorizontalSpacing = spacing;
        if (mMenuView) mMenuView->setIconLabelHorizontalSpacing(spacing);
    }
}

int NavigationBarView::getActiveIndicatorLabelPadding() const {
    return mActiveIndicatorLabelPadding;
}

void NavigationBarView::setActiveIndicatorLabelPadding(int activeIndicatorLabelPadding) {
    if (mActiveIndicatorLabelPadding != activeIndicatorLabelPadding) {
        mActiveIndicatorLabelPadding = activeIndicatorLabelPadding;
        if (mMenuView) mMenuView->setActiveIndicatorLabelPadding(activeIndicatorLabelPadding);
    }
}

bool NavigationBarView::onMenuItemClick(MenuItem* item) {
    if (mItemSelectedListener) {
        return mItemSelectedListener->onNavigationItemSelected(item);
    }
    return false;
}

void NavigationBarView::updateMenuView() {
    if (mMenuView == nullptr) {
        return;
    }
    // The presenter route (updateSuspended guard); a direct call is equivalent
    // and keeps the pre-port refreshMenuView() contract (rebuild the items).
    mMenuView->updateMenuView();
}

}//namespace cdroid
