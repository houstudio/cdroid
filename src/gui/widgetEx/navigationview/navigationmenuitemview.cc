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
#include <widgetEx/navigationview/navigationmenuitemview.h>
#include <widget/internal_R.h>
#include <menu/menuitemimpl.h>
#include <widget/textview.h>
#include <drawable/statelistdrawable.h>
#include <drawable/colordrawable.h>
#include <core/typedarray.h>
#include <core/typeface.h>
#include <porting/cdlog.h>

namespace cdroid{
using namespace cdroid::internal;

namespace{
// Framework theme attrs resolved for the code-built layouts (the XML layouts
// reference them through ?attr/?android:attr).
const uint32_t ITEM_THEME_ATTRS[] = {
    R::attr::listPreferredItemHeightSmall,
    R::attr::listPreferredItemPaddingStart,
    R::attr::listPreferredItemPaddingEnd,
};
constexpr int IDX_HEIGHT_SMALL   = 0;
constexpr int IDX_PADDING_START  = 1;
constexpr int IDX_PADDING_END    = 2;

int dp(Context* context,int dps){
    return (int)(context->getDisplayMetrics().density * dps + 0.5f);
}
}//namespace

NavigationMenuItemView::NavigationMenuItemView(Context* context)
    : LinearLayout(context, nullptr, 0){
    setOrientation(LinearLayout::HORIZONTAL);

    // design_navigation_item.xml root attributes: minHeight/paddings from the
    // theme, focusable, default-focus-highlight off (no such highlight here).
    auto ta = context->obtainStyledAttributes(ITEM_THEME_ATTRS);
    const int heightSmall  = ta->getDimensionPixelSize(IDX_HEIGHT_SMALL,  dp(context, 48));
    const int paddingStart = ta->getDimensionPixelSize(IDX_PADDING_START, dp(context, 16));
    const int paddingEnd   = ta->getDimensionPixelSize(IDX_PADDING_END,   dp(context, 16));
    setMinimumHeight(heightSmall);
    setPaddingRelative(paddingStart, 0, paddingEnd, 0);
    setFocusable(true);

    // design_navigation_menu_item.xml: CheckedTextView + ViewStub action area.
    mTextView = new CheckedTextView(context, nullptr, 0);
    mTextView->setGravity(Gravity::CENTER_VERTICAL | Gravity::START);
    mTextView->setMaxLines(1);
    mTextView->setCompoundDrawablePadding(dp(context, ICON_PADDING_DP));
    mTextView->setTextAppearance(R::style::TextAppearance_Material_Body2);
    LinearLayout::LayoutParams* lp = new LinearLayout::LayoutParams(
            0, ViewGroup::LayoutParams::MATCH_PARENT);
    lp->weight = 1;
    addView(mTextView, lp);

    setIconSize(dp(context, DEFAULT_ICON_SIZE_DP));
}

NavigationMenuItemView::~NavigationMenuItemView(){
    delete mOwnedIcon;
    delete mEmptyDrawable;
}

void NavigationMenuItemView::initialize(MenuItemImpl* itemData,int menuType){
    (void)menuType;
    mItemData = itemData;
    if (itemData->getItemId() > 0) {
        setId(itemData->getItemId());
    }

    setVisibility(itemData->isVisible() ? VISIBLE : GONE);

    if (getBackground() == nullptr) {
        setBackground(createDefaultBackground());
    }

    setCheckable(itemData->isCheckable());
    setChecked(itemData->isChecked());
    setEnabled(itemData->isEnabled());
    setTitle(itemData->getTitle());
    setIcon(itemData->getIcon());
    setActionView(itemData->getActionView());
    // AOSP also sets the content description and the tooltip text here; CDROID
    // views carry no tooltip, the content description is still applied.
    setContentDescription(itemData->getContentDescription());
    adjustAppearance();
}

void NavigationMenuItemView::initialize(MenuItemImpl* itemData,bool isBold){
    mIsBold = isBold;
    initialize(itemData, 0);
}

bool NavigationMenuItemView::shouldExpandActionArea(){
    return mItemData->getTitle().empty()
        && mItemData->getIcon() == nullptr
        && mItemData->getActionView() != nullptr;
}

void NavigationMenuItemView::adjustAppearance(){
    if (shouldExpandActionArea()) {
        // Expand the actionView area
        mTextView->setVisibility(GONE);
        if (mActionArea != nullptr) {
            LayoutParams* params = (LayoutParams*)mActionArea->getLayoutParams();
            params->width = LayoutParams::MATCH_PARENT;
            mActionArea->setLayoutParams(params);
        }
    } else {
        mTextView->setVisibility(VISIBLE);
        if (mActionArea != nullptr) {
            LayoutParams* params = (LayoutParams*)mActionArea->getLayoutParams();
            params->width = LayoutParams::WRAP_CONTENT;
            mActionArea->setLayoutParams(params);
        }
    }
}

void NavigationMenuItemView::recycle(){
    if (mActionArea != nullptr) {
        mActionArea->removeAllViews();
    }
    mTextView->setCompoundDrawables(nullptr, nullptr, nullptr, nullptr);
    // The compound drawables were cleared above, the owned tinted icon (if
    // any) can be freed now (AOSP leaves this to the GC).
    delete mOwnedIcon;
    mOwnedIcon = nullptr;
}

void NavigationMenuItemView::setActionView(View* actionView){
    if (actionView != nullptr) {
        if (mActionArea == nullptr) {
            // material inflates design_menu_item_action_area through a ViewStub.
            mActionArea = new FrameLayout(getContext(), nullptr, 0);
            LinearLayout::LayoutParams* lp = new LinearLayout::LayoutParams(
                    LayoutParams::WRAP_CONTENT, LayoutParams::MATCH_PARENT);
            addView(mActionArea, lp);
        }
        // Make sure to remove the existing parent if the View is reused
        if (actionView->getParent() != nullptr) {
            ((ViewGroup*)actionView->getParent())->removeView(actionView);
        }
        mActionArea->removeAllViews();
        mActionArea->addView(actionView);
    }
}

StateListDrawable* NavigationMenuItemView::createDefaultBackground(){
    TypedValue value;
    if (getContext()->getTheme().resolveAttribute(R::attr::colorControlHighlight, &value, true)) {
        StateListDrawable* drawable = new StateListDrawable();
        drawable->addState(std::vector<int>{R::attr::state_checked},
                new ColorDrawable(value.data));
        drawable->addState(std::vector<int>{}, new ColorDrawable(0));
        return drawable;
    }
    return nullptr;
}

MenuItemImpl* NavigationMenuItemView::getItemData(){
    return mItemData;
}

void NavigationMenuItemView::setTitle(const std::string& title){
    mTextView->setText(title);
}

void NavigationMenuItemView::setEnabled(bool enabled){
    // Satisfies both View::setEnabled and MenuView::ItemView::setEnabled.
    LinearLayout::setEnabled(enabled);
}

void NavigationMenuItemView::setCheckable(bool checkable){
    refreshDrawableState();
    mCheckable = checkable;
}

void NavigationMenuItemView::setChecked(bool checked){
    refreshDrawableState();
    mTextView->setChecked(checked);
    mTextView->setTypeface(mTextView->getTypeface(),
            (checked && mIsBold) ? Typeface::BOLD : Typeface::NORMAL);
}

void NavigationMenuItemView::setShortcut(bool showShortcut,int shortcutKey){
    (void)showShortcut; (void)shortcutKey;
}

void NavigationMenuItemView::setIcon(Drawable* icon){
    if (icon != nullptr) {
        if (mHasIconTintList) {
            std::shared_ptr<Drawable::ConstantState> state = icon->getConstantState();
            delete mOwnedIcon;
            mOwnedIcon = nullptr;
            if (state != nullptr) {
                // Own the mutated copy (AOSP: wrap(newDrawable).mutate(), GC-owned).
                icon = state->newDrawable()->mutate();
                mOwnedIcon = icon;
            } else {
                icon = icon->mutate();
            }
            icon->setTintList(mIconTintList);
        }
        icon->setBounds(0, 0, mIconSize, mIconSize);
    } else if (mNeedsEmptyIcon) {
        if (mEmptyDrawable == nullptr) {
            // navigation_empty_icon: a transparent icon-size rectangle.
            mEmptyDrawable = new ColorDrawable(0);
            mEmptyDrawable->setBounds(0, 0, mIconSize, mIconSize);
        }
        icon = mEmptyDrawable;
    }
    mTextView->setCompoundDrawables(icon, nullptr, nullptr, nullptr);
}

bool NavigationMenuItemView::prefersCondensedTitle()const{
    return false;
}

bool NavigationMenuItemView::showsIcon(){
    return true;
}

std::vector<int> NavigationMenuItemView::onCreateDrawableState(int extraSpace){
    std::vector<int> drawableState = LinearLayout::onCreateDrawableState(extraSpace + 1);
    if (mItemData != nullptr && mItemData->isCheckable() && mItemData->isChecked()) {
        const std::vector<int> checked{R::attr::state_checked};
        mergeDrawableStates(drawableState, checked);
    }
    return drawableState;
}

void NavigationMenuItemView::setIconTintList(const RefPtr<ColorStateList>& tintList){
    mIconTintList = tintList;
    mHasIconTintList = mIconTintList != nullptr;
    if (mItemData != nullptr) {
        // Update the icon so that the tint takes effect
        setIcon(mItemData->getIcon());
    }
}

void NavigationMenuItemView::setTextAppearance(int textAppearance){
    mTextView->setTextAppearance(textAppearance);
}

void NavigationMenuItemView::setTextColor(const RefPtr<ColorStateList>& colors){
    mTextView->setTextColor(colors);
}

void NavigationMenuItemView::setNeedsEmptyIcon(bool needsEmptyIcon){
    mNeedsEmptyIcon = needsEmptyIcon;
}

void NavigationMenuItemView::setHorizontalPadding(int padding){
    setPaddingRelative(padding, getPaddingTop(), padding, getPaddingBottom());
}

void NavigationMenuItemView::setIconPadding(int padding){
    mTextView->setCompoundDrawablePadding(padding);
}

void NavigationMenuItemView::setMaxLines(int maxLines){
    mTextView->setMaxLines(maxLines);
}

void NavigationMenuItemView::setIconSize(int iconSize){
    mIconSize = iconSize;
}

}//namespace cdroid
