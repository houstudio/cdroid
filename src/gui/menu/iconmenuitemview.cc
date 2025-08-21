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
#include <menu/iconmenuitemview.h>
#include <menu/menuitemimpl.h>
namespace cdroid{
std::string IconMenuItemView::sPrependShortcutLabel;

IconMenuItemView::IconMenuItemView(Context* context,const AttributeSet& attrs)
    :TextView(context, attrs){

    if (sPrependShortcutLabel.empty()) {
        /*
         * Views should only be constructed from the UI thread, so no
         * synchronization needed
         */
        sPrependShortcutLabel = attrs.getString("android:string/prepend_shortcut_label");
    }

    mDisabledAlpha = attrs.getFloat("itemIconDisabledAlpha", 0.8f);
    mTextAppearance = attrs.getString("itemTextAppearance");
    mTextAppearanceContext = context;
}

void IconMenuItemView::initialize(const std::string& title, Drawable* icon) {
    setClickable(true);
    setFocusable(true);

    if (!mTextAppearance.empty()) {
        setTextAppearance(mTextAppearanceContext, mTextAppearance);
    }

    setTitle(title);
    setIcon(icon);

    if (mItemData != nullptr) {
        std::string contentDescription = mItemData->getContentDescription();
        if (contentDescription.empty()) {
            setContentDescription(title);
        } else {
            setContentDescription(contentDescription);
        }
        setTooltipText(mItemData->getTooltipText());
    }
}

void IconMenuItemView::initialize(MenuItemImpl* itemData, int menuType) {
    mItemData = itemData;

    initialize(itemData->getTitleForItemView(this), itemData->getIcon());

    setVisibility(itemData->isVisible() ? View::VISIBLE : View::GONE);
    setEnabled(itemData->isEnabled());
}

void IconMenuItemView::setItemData(MenuItemImpl* data) {
    mItemData = data;
}

bool IconMenuItemView::performClick() {
    // Let the view's click listener have top priority (the More button relies on this)
    if (TextView::performClick()) {
        return true;
    }

    if ((mItemInvoker != nullptr) && mItemInvoker(*mItemData)) {
        playSoundEffect(SoundEffectConstants::CLICK);
        return true;
    } else {
        return false;
    }
    return false;
}

void IconMenuItemView::setTitle(const std::string& title) {

    if (mShortcutCaptionMode) {
        /*
         * Don't set the title directly since it will replace the
         * shortcut+title being shown. Instead, re-set the shortcut caption
         * mode so the new title is shown.
         */
        setCaptionMode(true);

    } else if (!title.empty()) {
        setText(title);
    }
}

void IconMenuItemView::setCaptionMode(bool shortcut) {
    /*
     * If there is no item model, don't do any of the below (for example,
     * the 'More' item doesn't have a model)
     */
    if (mItemData == nullptr) {
        return;
    }

    mShortcutCaptionMode = shortcut && (mItemData->shouldShowShortcut());

    std::string text = mItemData->getTitleForItemView(this);

    if (mShortcutCaptionMode) {

        if (mShortcutCaption.empty()) {
            mShortcutCaption = mItemData->getShortcutLabel();
        }

        text = mShortcutCaption;
    }

    setText(text);
}

void IconMenuItemView::setIcon(Drawable* icon) {
    mIcon = icon;

    if (icon != nullptr) {

        /* Set the bounds of the icon since setCompoundDrawables needs it. */
        icon->setBounds(0, 0, icon->getIntrinsicWidth(), icon->getIntrinsicHeight());

        // Set the compound drawables
        setCompoundDrawables(nullptr, icon, nullptr, nullptr);

        // When there is an icon, make sure the text is at the bottom
        setGravity(Gravity::BOTTOM | Gravity::CENTER_HORIZONTAL);

        /*
         * Request a layout to reposition the icon. The positioning of icon
         * depends on this TextView's line bounds, which is only available
         * after a layout.
         */
        requestLayout();
    } else {
        setCompoundDrawables(nullptr, nullptr, nullptr, nullptr);

        // When there is no icon, make sure the text is centered vertically
        setGravity(Gravity::CENTER_VERTICAL | Gravity::CENTER_HORIZONTAL);
    }
}

void IconMenuItemView::setItemInvoker(const MenuBuilder::ItemInvoker& itemInvoker) {
    mItemInvoker = itemInvoker;
}

MenuItemImpl* IconMenuItemView::getItemData() {
    return mItemData;
}

void IconMenuItemView::setEnabled(bool v){
    TextView::setEnabled(v);
}

void IconMenuItemView::setVisibility(int v) {
    TextView::setVisibility(v);

    if (mIconMenuView != nullptr) {
        // On visibility change, mark the IconMenuView to refresh itself eventually
        mIconMenuView->markStaleChildren();
    }
}

void IconMenuItemView::setIconMenuView(IconMenuView* iconMenuView) {
    mIconMenuView = iconMenuView;
}

void IconMenuItemView::drawableStateChanged() {
    TextView::drawableStateChanged();

    if (mItemData != nullptr && mIcon != nullptr) {
        // When disabled, the not-focused state and the pressed state should
        // drop alpha on the icon
        const bool isInAlphaState = !mItemData->isEnabled() && (isPressed() || !isFocused());
        mIcon->setAlpha(isInAlphaState ? (int) (mDisabledAlpha * NO_ALPHA) : NO_ALPHA);
    }
}

void IconMenuItemView::onLayout(bool changed, int left, int top, int width, int height) {
    TextView::onLayout(changed, left, top, width, height);

    positionIcon();
}

void IconMenuItemView::onTextChanged(const std::wstring& text, int start, int before, int after) {
    TextView::onTextChanged(text, start, before, after);

    // our layout params depend on the length of the text
    setLayoutParams(getTextAppropriateLayoutParams());
}

IconMenuView::LayoutParams* IconMenuItemView::getTextAppropriateLayoutParams(){
    IconMenuView::LayoutParams* lp = (IconMenuView::LayoutParams*) getLayoutParams();
    if (lp == nullptr) {
        // Default layout parameters
        lp = new IconMenuView::LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::MATCH_PARENT);
    }

    // Set the desired width of item
    lp->desiredWidth = 120;//(int) Layout::getDesiredWidth(getText(), 0, getText().length(), getPaint(), getTextDirectionHeuristic());

    return lp;
}

void IconMenuItemView::positionIcon() {

    if (mIcon == nullptr) {
        return;
    }

    // We reuse the output rectangle as a temp rect
    Rect tmpRect = mPositionIconOutput;
    getLineBounds(0, tmpRect);
    mPositionIconAvailable.set(0, 0, getWidth(), tmpRect.top);
    const int layoutDirection = getLayoutDirection();
    Gravity::apply(Gravity::CENTER_VERTICAL | Gravity::START, mIcon->getIntrinsicWidth(), mIcon
            ->getIntrinsicHeight(), mPositionIconAvailable, mPositionIconOutput,
            layoutDirection);
    mIcon->setBounds(mPositionIconOutput);
}

void IconMenuItemView::setCheckable(bool checkable) {
}

void IconMenuItemView::setChecked(bool checked) {
}

void IconMenuItemView::setShortcut(bool showShortcut, char shortcutKey) {

    if (mShortcutCaptionMode) {
        /*
         * Shortcut has changed and we're showing it right now, need to
         * update (clear the old one first).
         */
        mShortcutCaption.clear();
        setCaptionMode(true);
    }
}

bool IconMenuItemView::prefersCondensedTitle() {
    return true;
}

bool IconMenuItemView::showsIcon() {
    return true;
}

}
