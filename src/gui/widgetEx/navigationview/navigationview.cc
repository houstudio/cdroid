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
#include <widgetEx/navigationview/navigationview.h>
#include <widgetEx/widgetex_styleable.h>
#include <widget/internal_R.h>
#include <menu/menuinflater.h>
#include <menu/menuitemimpl.h>
#include <content/typedarray.h>
#include <stdexcept>

namespace cdroid{
using namespace cdroid::internal;

DECLARE_WIDGET2(NavigationView, "com.google.android.material.navigation.NavigationView");

NavigationView::NavigationView(Context* context, const AttributeSet* attrs)
    : NavigationView(context, attrs, 0) {}

NavigationView::NavigationView(Context* context, const AttributeSet* attrs, int defStyleAttr)
    : FrameLayout(context, attrs, defStyleAttr) {
    init(context, attrs, defStyleAttr);
}

NavigationView::~NavigationView() {
    delete mPresenter;
    delete mMenu;
}

void NavigationView::init(Context* context, const AttributeSet* attrs, int defStyleAttr) {
    (void)defStyleAttr;
    // android:maxWidth is a framework attr; read it by its fw id through the
    // tag AttributeSet (AOSP reads it via the same styleable).
    mMaxWidth = attrs ? attrs->getAttributeIntValue(std::string(), "maxWidth", 0) : 0;

    // Create the menu
    this->mMenu = new NavigationMenu(context);

    mPresenter = new NavigationMenuPresenter();

    // Custom attributes (DEF_STYLE_RES = Widget_Design_NavigationView, the
    // upstream MaterialThemeOverlay.wrap idiom's style-resolution effect).
    auto ta = context->obtainStyledAttributes(attrs,
            cdroid::internal::R::styleable::NavigationView, defStyleAttr,
            cdroid::internal::R::style::Widget_Design_NavigationView);

    RefPtr<ColorStateList> subheaderColor;
    if (ta->hasValue(cdroid::internal::R::styleable::NavigationView_subheaderColor)) {
        subheaderColor = ta->getColorStateList(
                cdroid::internal::R::styleable::NavigationView_subheaderColor);
    }

    int subheaderTextAppearance = NavigationMenuPresenter::NO_TEXT_APPEARANCE_SET;
    if (ta->hasValue(cdroid::internal::R::styleable::NavigationView_subheaderTextAppearance)) {
        subheaderTextAppearance = ta->getResourceId(
                cdroid::internal::R::styleable::NavigationView_subheaderTextAppearance, 0);
    }

    if (subheaderTextAppearance == NavigationMenuPresenter::NO_TEXT_APPEARANCE_SET
            && subheaderColor == nullptr) {
        // If there isn't a text appearance set, we'll use a default text color
        subheaderColor = createDefaultColorStateList(R::attr::textColorSecondary);
    }

    RefPtr<ColorStateList> itemIconTint;
    if (ta->hasValue(cdroid::internal::R::styleable::NavigationView_itemIconTint)) {
        itemIconTint = ta->getColorStateList(
                cdroid::internal::R::styleable::NavigationView_itemIconTint);
    } else {
        itemIconTint = createDefaultColorStateList(R::attr::textColorSecondary);
    }

    int textAppearance = NavigationMenuPresenter::NO_TEXT_APPEARANCE_SET;
    if (ta->hasValue(cdroid::internal::R::styleable::NavigationView_itemTextAppearance)) {
        textAppearance = ta->getResourceId(
                cdroid::internal::R::styleable::NavigationView_itemTextAppearance, 0);
    }

    const bool textAppearanceActiveBoldEnabled = ta->getBoolean(
            cdroid::internal::R::styleable::NavigationView_itemTextAppearanceActiveBoldEnabled, true);

    if (ta->hasValue(cdroid::internal::R::styleable::NavigationView_itemIconSize)) {
        setItemIconSize(ta->getDimensionPixelSize(
                cdroid::internal::R::styleable::NavigationView_itemIconSize, 0));
    }

    RefPtr<ColorStateList> itemTextColor;
    if (ta->hasValue(cdroid::internal::R::styleable::NavigationView_itemTextColor)) {
        itemTextColor = ta->getColorStateList(
                cdroid::internal::R::styleable::NavigationView_itemTextColor);
    }

    if (textAppearance == NavigationMenuPresenter::NO_TEXT_APPEARANCE_SET && itemTextColor == nullptr) {
        // If there isn't a text appearance set, we'll use a default text color
        itemTextColor = createDefaultColorStateList(R::attr::textColorPrimary);
    }

    Drawable* itemBackground = ta->getDrawable(
            cdroid::internal::R::styleable::NavigationView_itemBackground);

    if (ta->hasValue(cdroid::internal::R::styleable::NavigationView_itemHorizontalPadding)) {
        setItemHorizontalPadding(ta->getDimensionPixelSize(
                cdroid::internal::R::styleable::NavigationView_itemHorizontalPadding, 0));
    }

    if (ta->hasValue(cdroid::internal::R::styleable::NavigationView_itemVerticalPadding)) {
        setItemVerticalPadding(ta->getDimensionPixelSize(
                cdroid::internal::R::styleable::NavigationView_itemVerticalPadding, 0));
    }

    setDividerInsetStart(ta->getDimensionPixelSize(
            cdroid::internal::R::styleable::NavigationView_dividerInsetStart, 0));

    setDividerInsetEnd(ta->getDimensionPixelSize(
            cdroid::internal::R::styleable::NavigationView_dividerInsetEnd, 0));

    setSubheaderInsetStart(ta->getDimensionPixelSize(
            cdroid::internal::R::styleable::NavigationView_subheaderInsetStart, 0));

    setSubheaderInsetEnd(ta->getDimensionPixelSize(
            cdroid::internal::R::styleable::NavigationView_subheaderInsetEnd, 0));

    setTopInsetScrimEnabled(ta->getBoolean(
            cdroid::internal::R::styleable::NavigationView_topInsetScrimEnabled, mTopInsetScrimEnabled));

    setBottomInsetScrimEnabled(ta->getBoolean(
            cdroid::internal::R::styleable::NavigationView_bottomInsetScrimEnabled, mBottomInsetScrimEnabled));

    const int itemIconPadding = ta->getDimensionPixelSize(
            cdroid::internal::R::styleable::NavigationView_itemIconPadding, 0);

    setItemMaxLines(ta->getInt(cdroid::internal::R::styleable::NavigationView_itemMaxLines, 1));

    MenuBuilder::Callback cb;
    cb.onMenuItemSelected = [this](MenuBuilder&, MenuItem& item)->bool{
        return onMenuItemSelected(item);
    };
    cb.onMenuModeChange = [](MenuBuilder&){};
    mMenu->setCallback(cb);
    mPresenter->setId(PRESENTER_NAVIGATION_VIEW_ID);
    mPresenter->initForMenu(context, mMenu);
    if (subheaderTextAppearance != NavigationMenuPresenter::NO_TEXT_APPEARANCE_SET) {
        mPresenter->setSubheaderTextAppearance(subheaderTextAppearance);
    }
    mPresenter->setSubheaderColor(subheaderColor);
    mPresenter->setItemIconTintList(itemIconTint);
    mPresenter->setOverScrollMode(getOverScrollMode());
    if (textAppearance != NavigationMenuPresenter::NO_TEXT_APPEARANCE_SET) {
        mPresenter->setItemTextAppearance(textAppearance);
    }
    mPresenter->setItemTextAppearanceActiveBoldEnabled(textAppearanceActiveBoldEnabled);
    mPresenter->setItemTextColor(itemTextColor);
    mPresenter->setItemBackground(itemBackground);
    mPresenter->setItemIconPadding(itemIconPadding);
    mMenu->addMenuPresenter(mPresenter);
    addView(mPresenter->getMenuView(this), new ViewGroup::LayoutParams(
            ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::MATCH_PARENT));

    if (ta->hasValue(cdroid::internal::R::styleable::NavigationView_menu)) {
        inflateMenu(ta->getResourceId(cdroid::internal::R::styleable::NavigationView_menu, 0));
    }

    if (ta->hasValue(cdroid::internal::R::styleable::NavigationView_headerLayout)) {
        inflateHeaderView(ta->getResourceId(
                cdroid::internal::R::styleable::NavigationView_headerLayout, 0));
    }
}

void NavigationView::setOverScrollMode(int overScrollMode) {
    FrameLayout::setOverScrollMode(overScrollMode);
    if (mPresenter != nullptr) {
        mPresenter->setOverScrollMode(overScrollMode);
    }
}

NavigationView::SavedState::SavedState(Parcelable* superState)
    : AbsSavedState(superState){}

Parcelable* NavigationView::onSaveInstanceState() {
    Parcelable* superState = FrameLayout::onSaveInstanceState();
    SavedState* state = new SavedState(superState);
    mMenu->savePresenterStates(state->menuState);
    return state;
}

void NavigationView::onRestoreInstanceState(Parcelable& savedState) {
    SavedState* state = dynamic_cast<SavedState*>(&savedState);
    if (state == nullptr) {
        FrameLayout::onRestoreInstanceState(savedState);
        return;
    }
    if (state->getSuperState() != nullptr) {
        FrameLayout::onRestoreInstanceState(*state->getSuperState());
    }
    mMenu->restorePresenterStates(state->menuState);
}

void NavigationView::setNavigationItemSelectedListener(OnNavigationItemSelectedListener* listener) {
    mListener = listener;
}

void NavigationView::inflateMenu(int resId) {
    mPresenter->setUpdateSuspended(true);
    MenuInflater inflater(getContext());
    inflater.inflate(resId, mMenu);
    mPresenter->setUpdateSuspended(false);
    mPresenter->updateMenuView(false);
}

Menu* NavigationView::getMenu() {
    return mMenu;
}

View* NavigationView::inflateHeaderView(int res) {
    return mPresenter->inflateHeaderView(res);
}

void NavigationView::addHeaderView(View* view) {
    mPresenter->addHeaderView(view);
}

void NavigationView::removeHeaderView(View* view) {
    mPresenter->removeHeaderView(view);
}

int NavigationView::getHeaderCount() const {
    return mPresenter->getHeaderCount();
}

View* NavigationView::getHeaderView(int index) const {
    return mPresenter->getHeaderView(index);
}

const RefPtr<ColorStateList> NavigationView::getItemIconTintList() const {
    return mPresenter->getItemTintList();
}

void NavigationView::setItemIconTintList(const RefPtr<ColorStateList>& tint) {
    mPresenter->setItemIconTintList(tint);
}

const RefPtr<ColorStateList> NavigationView::getItemTextColor() const {
    return mPresenter->getItemTextColor();
}

void NavigationView::setItemTextColor(const RefPtr<ColorStateList>& textColor) {
    mPresenter->setItemTextColor(textColor);
}

Drawable* NavigationView::getItemBackground() const {
    return mPresenter->getItemBackground();
}

void NavigationView::setItemBackgroundResource(int resId) {
    setItemBackground(getContext()->getDrawable(resId));
}

void NavigationView::setItemBackground(Drawable* itemBackground) {
    mPresenter->setItemBackground(itemBackground);
}

int NavigationView::getItemHorizontalPadding() const {
    return mPresenter->getItemHorizontalPadding();
}

void NavigationView::setItemHorizontalPadding(int padding) {
    mPresenter->setItemHorizontalPadding(padding);
}

void NavigationView::setItemHorizontalPaddingResource(int paddingResource) {
    setItemHorizontalPadding(getContext()->getDimensionPixelSize(paddingResource));
}

int NavigationView::getItemVerticalPadding() const {
    return mPresenter->getItemVerticalPadding();
}

void NavigationView::setItemVerticalPadding(int padding) {
    mPresenter->setItemVerticalPadding(padding);
}

void NavigationView::setItemVerticalPaddingResource(int paddingResource) {
    setItemVerticalPadding(getContext()->getDimensionPixelSize(paddingResource));
}

int NavigationView::getItemIconPadding() const {
    return mPresenter->getItemIconPadding();
}

void NavigationView::setItemIconPadding(int padding) {
    mPresenter->setItemIconPadding(padding);
}

void NavigationView::setItemIconPaddingResource(int paddingResource) {
    setItemIconPadding(getContext()->getDimensionPixelSize(paddingResource));
}

void NavigationView::setCheckedItem(int id) {
    MenuItem* item = mMenu->findItem(id);
    if (item != nullptr) {
        mPresenter->setCheckedItem((MenuItemImpl*)item);
    }
}

void NavigationView::setCheckedItem(MenuItem* checkedItem) {
    MenuItem* item = mMenu->findItem(checkedItem->getItemId());
    if (item != nullptr) {
        mPresenter->setCheckedItem((MenuItemImpl*)item);
    } else {
        throw std::invalid_argument(
                "Called setCheckedItem(MenuItem) with an item that is not in the current menu.");
    }
}

MenuItem* NavigationView::getCheckedItem() {
    return mPresenter->getCheckedItem();
}

void NavigationView::setItemTextAppearance(int resId) {
    mPresenter->setItemTextAppearance(resId);
}

void NavigationView::setItemTextAppearanceActiveBoldEnabled(bool isBold) {
    mPresenter->setItemTextAppearanceActiveBoldEnabled(isBold);
}

void NavigationView::setItemIconSize(int iconSize) {
    mPresenter->setItemIconSize(iconSize);
}

void NavigationView::setItemMaxLines(int itemMaxLines) {
    mPresenter->setItemMaxLines(itemMaxLines);
}

int NavigationView::getItemMaxLines() {
    return mPresenter->getItemMaxLines();
}

bool NavigationView::isTopInsetScrimEnabled() const {
    return mTopInsetScrimEnabled;
}

void NavigationView::setTopInsetScrimEnabled(bool enabled) {
    mTopInsetScrimEnabled = enabled;
}

bool NavigationView::isBottomInsetScrimEnabled() const {
    return mBottomInsetScrimEnabled;
}

void NavigationView::setBottomInsetScrimEnabled(bool enabled) {
    mBottomInsetScrimEnabled = enabled;
}

int NavigationView::getDividerInsetStart() const {
    return mPresenter->getDividerInsetStart();
}

void NavigationView::setDividerInsetStart(int dividerInsetStart) {
    mPresenter->setDividerInsetStart(dividerInsetStart);
}

int NavigationView::getDividerInsetEnd() const {
    return mPresenter->getDividerInsetEnd();
}

void NavigationView::setDividerInsetEnd(int dividerInsetEnd) {
    mPresenter->setDividerInsetEnd(dividerInsetEnd);
}

int NavigationView::getSubheaderInsetStart() const {
    return mPresenter->getSubheaderInsetStart();
}

void NavigationView::setSubheaderInsetStart(int subheaderInsetStart) {
    mPresenter->setSubheaderInsetStart(subheaderInsetStart);
}

int NavigationView::getSubheaderInsetEnd() const {
    return mPresenter->getSubheaderInsetEnd();
}

void NavigationView::setSubheaderInsetEnd(int subheaderInsetEnd) {
    mPresenter->setSubheaderInsetEnd(subheaderInsetEnd);
}

void NavigationView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    switch (MeasureSpec::getMode(widthMeasureSpec)) {
        case MeasureSpec::EXACTLY:
            // Nothing to do
            break;
        case MeasureSpec::AT_MOST:
            widthMeasureSpec = MeasureSpec::makeMeasureSpec(
                    std::min(MeasureSpec::getSize(widthMeasureSpec), mMaxWidth), MeasureSpec::EXACTLY);
            break;
        default:
            widthMeasureSpec = MeasureSpec::makeMeasureSpec(mMaxWidth, MeasureSpec::EXACTLY);
            break;
    }
    // Let super sort out the height
    FrameLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);
}

RefPtr<ColorStateList> NavigationView::createDefaultColorStateList(int baseColorThemeAttr) {
    TypedValue value;
    if (!getContext()->getTheme().resolveAttribute(baseColorThemeAttr, &value, true)) {
        return nullptr;
    }
    RefPtr<ColorStateList> baseColor = getContext()->getColorStateList(value.resourceId);
    if (!getContext()->getTheme().resolveAttribute(R::attr::colorPrimary, &value, true)) {
        return nullptr;
    }
    const int colorPrimary = value.data;
    const int defaultColor = baseColor->getDefaultColor();
    const std::vector<int> disabled{-R::attr::state_enabled};
    const std::vector<int> checked{R::attr::state_checked};
    const std::vector<int> empty;
    return RefPtr<ColorStateList>(new ColorStateList(
            {disabled, checked, empty},
            {baseColor->getColorForState(disabled, defaultColor), colorPrimary, defaultColor}));
}

bool NavigationView::onMenuItemSelected(MenuItem& item) {
    return mListener && mListener->onNavigationItemSelected(&item);
}

}//namespace cdroid
