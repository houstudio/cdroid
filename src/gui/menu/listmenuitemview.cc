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
#include <menu/listmenuitemview.h>
#include <menu/menuitemimpl.h>
#include <widget/textview.h>
#include <widget/checkbox.h>
#include <widget/imageview.h>
#include <widget/radiobutton.h>
#include <widget/R.h>
namespace cdroid{
ListMenuItemView::ListMenuItemView(Context* context,const AttributeSet& attrs)
    :LinearLayout(context, attrs){

    mBackground = attrs.getDrawable("itemBackground");
    mTextAppearance = attrs.getResourceId("itemTextAppearance");
    mPreserveIconSpacing = attrs.getBoolean("preserveIconSpacing", false);
    mTextAppearanceContext = context;
    mSubMenuArrow = attrs.getDrawable("subMenuArrow");

    /*final TypedArray b = context.getTheme()
            .obtainStyledAttributes(null, new int[] { com.android.internal.R.attr.divider },
                    com.android.internal.R.attr.dropDownListViewStyle, 0);*/
    mHasListDivider = false;//b.hasValue(0);
}

void ListMenuItemView::onFinishInflate() {
    LinearLayout::onFinishInflate();

    setBackground(mBackground);

    mTitleView = (TextView*)findViewById(cdroid::R::id::title);
    if (!mTextAppearance.empty()) {
        mTitleView->setTextAppearance(mTextAppearanceContext,mTextAppearance);
    }

    mShortcutView = (TextView*)findViewById(cdroid::R::id::shortcut);
    mSubMenuArrowView = (ImageView*)findViewById(cdroid::R::id::submenuarrow);
    if (mSubMenuArrowView != nullptr) {
        mSubMenuArrowView->setImageDrawable(mSubMenuArrow);
    }
    mGroupDivider = (ImageView*)findViewById(cdroid::R::id::group_divider);

    mContent = (LinearLayout*)findViewById(cdroid::R::id::content);
}

void ListMenuItemView::initialize(MenuItemImpl* itemData, int menuType) {
    mItemData = itemData;
    mMenuType = menuType;

    setVisibility(itemData->isVisible() ? View::VISIBLE : View::GONE);

    setTitle(itemData->getTitleForItemView(this));
    setCheckable(itemData->isCheckable());
    setShortcut(itemData->shouldShowShortcut(), itemData->getShortcut());
    setIcon(itemData->getIcon());
    setEnabled(itemData->isEnabled());
    setSubMenuArrowVisible(itemData->hasSubMenu());
    setContentDescription(itemData->getContentDescription());
}

void ListMenuItemView::addContentView(View* v) {
    addContentView(v, -1);
}

void ListMenuItemView::addContentView(View* v, int index) {
    if (mContent != nullptr) {
        mContent->addView(v, index);
    } else {
        addView(v, index);
    }
}

void ListMenuItemView::setForceShowIcon(bool forceShow) {
    mPreserveIconSpacing = mForceShowIcon = forceShow;
}

void ListMenuItemView::setTitle(const std::string& title) {
    if (!title.empty()) {
        mTitleView->setText(title);

        if (mTitleView->getVisibility() != VISIBLE) mTitleView->setVisibility(VISIBLE);
    } else {
        if (mTitleView->getVisibility() != GONE) mTitleView->setVisibility(GONE);
    }
}

MenuItemImpl* ListMenuItemView::getItemData() {
    return mItemData;
}

void ListMenuItemView::setEnabled(bool v){
    LinearLayout::setEnabled(v);
}

void ListMenuItemView::setCheckable(bool checkable) {
    if (!checkable && mRadioButton == nullptr && mCheckBox == nullptr) {
        return;
    }

    // Depending on whether its exclusive check or not, the checkbox or
    // radio button will be the one in use (and the other will be otherCompoundButton)
    CompoundButton* compoundButton;
    CompoundButton* otherCompoundButton;

    if (mItemData->isExclusiveCheckable()) {
        if (mRadioButton == nullptr) {
            insertRadioButton();
        }
        compoundButton = mRadioButton;
        otherCompoundButton = mCheckBox;
    } else {
        if (mCheckBox == nullptr) {
            insertCheckBox();
        }
        compoundButton = mCheckBox;
        otherCompoundButton = mRadioButton;
    }

    if (checkable) {
        compoundButton->setChecked(mItemData->isChecked());

        const int newVisibility = checkable ? VISIBLE : GONE;
        if (compoundButton->getVisibility() != newVisibility) {
            compoundButton->setVisibility(newVisibility);
        }

        // Make sure the other compound button isn't visible
        if (otherCompoundButton != nullptr && otherCompoundButton->getVisibility() != GONE) {
            otherCompoundButton->setVisibility(GONE);
        }
    } else {
        if (mCheckBox != nullptr) mCheckBox->setVisibility(GONE);
        if (mRadioButton != nullptr) mRadioButton->setVisibility(GONE);
    }
}

void ListMenuItemView::setChecked(bool checked) {
    CompoundButton* compoundButton;

    if (mItemData->isExclusiveCheckable()) {
        if (mRadioButton == nullptr) {
            insertRadioButton();
        }
        compoundButton = mRadioButton;
    } else {
        if (mCheckBox == nullptr) {
            insertCheckBox();
        }
        compoundButton = mCheckBox;
    }

    compoundButton->setChecked(checked);
}

void ListMenuItemView::setSubMenuArrowVisible(bool hasSubmenu) {
    if (mSubMenuArrowView != nullptr) {
        mSubMenuArrowView->setVisibility(hasSubmenu ? View::VISIBLE : View::GONE);
    }
}

void ListMenuItemView::setShortcut(bool showShortcut, int shortcutKey) {
    const int newVisibility = (showShortcut && mItemData->shouldShowShortcut())
            ? VISIBLE : GONE;

    if (newVisibility == VISIBLE) {
        mShortcutView->setText(mItemData->getShortcutLabel());
    }

    if (mShortcutView->getVisibility() != newVisibility) {
        mShortcutView->setVisibility(newVisibility);
    }
}

void ListMenuItemView::setIcon(Drawable* icon) {
    const bool showIcon = mItemData->shouldShowIcon() || mForceShowIcon;
    if (!showIcon && !mPreserveIconSpacing) {
        return;
    }

    if (mIconView == nullptr && icon == nullptr && !mPreserveIconSpacing) {
        return;
    }

    if (mIconView == nullptr) {
        insertIconView();
    }

    if (icon != nullptr || mPreserveIconSpacing) {
        mIconView->setImageDrawable(showIcon ? icon : nullptr);

        if (mIconView->getVisibility() != VISIBLE) {
            mIconView->setVisibility(VISIBLE);
        }
    } else {
        mIconView->setVisibility(GONE);
    }
}

void ListMenuItemView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    if (mIconView != nullptr && mPreserveIconSpacing) {
        // Enforce minimum icon spacing
        ViewGroup::LayoutParams* lp = getLayoutParams();
        LayoutParams* iconLp = (LayoutParams*) mIconView->getLayoutParams();
        if (lp->height > 0 && iconLp->width <= 0) {
            iconLp->width = lp->height;
        }
    }
    LinearLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);
}

void ListMenuItemView::insertIconView() {
    LayoutInflater* inflater = getInflater();
    mIconView = (ImageView*) inflater->inflate("cdroid:layout/list_menu_item_icon", this, false);
    addContentView(mIconView, 0);
}

void ListMenuItemView::insertRadioButton() {
    LayoutInflater* inflater = getInflater();
    mRadioButton =(RadioButton*) inflater->inflate("android/layout/list_menu_item_radio", this, false);
    addContentView(mRadioButton);
}

void ListMenuItemView::insertCheckBox() {
    LayoutInflater* inflater = getInflater();
    mCheckBox =(CheckBox*) inflater->inflate("android:/layout/list_menu_item_checkbox", this, false);
    addContentView(mCheckBox);
}

bool ListMenuItemView::prefersCondensedTitle() {
    return false;
}

bool ListMenuItemView::showsIcon() {
    return mForceShowIcon;
}

LayoutInflater* ListMenuItemView::getInflater() {
    if (mInflater == nullptr) {
        mInflater = LayoutInflater::from(mContext);
    }
    return mInflater;
}

void ListMenuItemView::onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info) {
    LinearLayout::onInitializeAccessibilityNodeInfoInternal(info);

    if (mItemData != nullptr && mItemData->hasSubMenu()) {
        info.setCanOpenPopup(true);
    }
}

void ListMenuItemView::setGroupDividerEnabled(bool groupDividerEnabled) {
    // If mHasListDivider is true, disabling the groupDivider.
    // Otherwise, checking enbling it according to groupDividerEnabled flag.
    if (mGroupDivider != nullptr) {
        mGroupDivider->setVisibility(!mHasListDivider
                && groupDividerEnabled ? View::VISIBLE : View::GONE);
    }
}

void ListMenuItemView::adjustListItemSelectionBounds(Rect& rect) {
    if (mGroupDivider != nullptr && mGroupDivider->getVisibility() == View::VISIBLE) {
        // groupDivider is a part of MenuItemListView.
        // If ListMenuItem with divider enabled is hovered/clicked, divider also gets selected.
        // Clipping the selector bounds from the top divider portion when divider is enabled,
        // so that divider does not get selected on hover or click.
        const LayoutParams* lp = (const LayoutParams*) mGroupDivider->getLayoutParams();
        rect.top += mGroupDivider->getHeight() + lp->topMargin + lp->bottomMargin;
    }
}
}
