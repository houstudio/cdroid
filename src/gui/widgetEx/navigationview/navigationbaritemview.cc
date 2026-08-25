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
#include <widgetEx/navigationview/navigationbaritemview.h>
#include <widgetEx/navigationview/navigationbarview.h>
#include <menu/menuitemimpl.h>
#include <drawable/colordrawable.h>
#include <drawable/rippledrawable.h>
#include <drawable/statelistdrawable.h>
#include <cdlog.h>

namespace cdroid{

NavigationBarItemView::NavigationBarItemView(Context* context, const AttributeSet* attrs)
    : FrameLayout(context, attrs)
    , mLabelVisibilityMode(NavigationBarView::LABEL_VISIBILITY_AUTO)
    , mIsShifting(false)
    , mItemPosition(INVALID_ITEM_POSITION)
    , mTextAppearanceActive(0)
    , mTextAppearanceInactive(0)
    , mItemIconGravity(NavigationBarView::ITEM_ICON_GRAVITY_TOP)
    , mItemGravity(NavigationBarView::ITEM_GRAVITY_TOP_CENTER) {
    // Material inflates design_bottom_navigation_item:
    //   FrameLayout(this)
    //     +- View activeIndicatorView
    //     +- LinearLayout contentContainer (vertical, ITEM_GRAVITY centered)
    //          +- LinearLayout innerContentContainer
    //               +- FrameLayout iconContainer > ImageView icon
    //               +- BaselineLayout labelGroup > small/large TextView
    // CDROID builds the same hierarchy in code (the layout resource is not in
    // the framework pak; BaselineLayout is approximated by a centered
    // LinearLayout - the small/large pair still drives the shifting scale).
    mActiveIndicatorView = new View(context);
    mActiveIndicatorView->setVisibility(View::INVISIBLE);
    mActiveIndicatorView->setLayoutParams(new FrameLayout::LayoutParams(
            LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT, Gravity::CENTER));
    addView(mActiveIndicatorView);

    mContentContainer = new LinearLayout(context);
    mContentContainer->setOrientation(LinearLayout::VERTICAL);
    mContentContainer->setLayoutParams(new FrameLayout::LayoutParams(
            LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT, mItemGravity));
    addView(mContentContainer);

    mInnerContentContainer = new LinearLayout(context);
    mInnerContentContainer->setOrientation(LinearLayout::VERTICAL);
    mInnerContentContainer->setGravity(Gravity::CENTER_HORIZONTAL);
    mInnerContentContainer->setLayoutParams(new LinearLayout::LayoutParams(
            LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT));
    mContentContainer->addView(mInnerContentContainer);

    mIconContainer = new FrameLayout(context);
    mIconContainer->setLayoutParams(new LinearLayout::LayoutParams(
            LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT));
    mInnerContentContainer->addView(mIconContainer);

    mIcon = new ImageView(context);
    mIcon->setScaleType(CENTER_INSIDE);
    mIcon->setLayoutParams(new FrameLayout::LayoutParams(
            LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT, Gravity::CENTER));
    mIconContainer->addView(mIcon);

    mLabelGroup = new LinearLayout(context);
    mLabelGroup->setOrientation(LinearLayout::HORIZONTAL);
    mLabelGroup->setLayoutParams(new LinearLayout::LayoutParams(
            LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT));
    mInnerContentContainer->addView(mLabelGroup);

    mSmallLabel = new TextView(context);
    mSmallLabel->setMaxLines(1);
    mSmallLabel->setIncludeFontPadding(false);
    mSmallLabel->setGravity(Gravity::CENTER);
    mSmallLabel->setLayoutParams(new LinearLayout::LayoutParams(
            LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT));
    mLabelGroup->addView(mSmallLabel);

    // The large label sits over the small one (both centered in the group);
    // only one is visible at a time (refreshChecked).
    mLargeLabel = new TextView(context);
    mLargeLabel->setMaxLines(1);
    mLargeLabel->setIncludeFontPadding(false);
    mLargeLabel->setGravity(Gravity::CENTER);
    mLargeLabel->setVisibility(View::INVISIBLE);
    mLargeLabel->setLayoutParams(new LinearLayout::LayoutParams(
            LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT));
    mLabelGroup->addView(mLargeLabel);

    // design_bottom_navigation_margin (12dp in material).
    mItemPaddingTop = 12;
    mItemPaddingBottom = 10;
    mActiveIndicatorLabelPadding = 0;
    mIconLabelHorizontalSpacing = 0;

    setFocusable(true);
    calculateTextScaleFactors();
}

NavigationBarItemView::~NavigationBarItemView() {
    // mItemBackground is shared from the presenter side (NavigationBarView's
    // setter semantics); each view's OWN background is freed by ~View.
}

void NavigationBarItemView::calculateTextScaleFactors() {
    // Material: shiftAmount  = paddingTop + activeIndicatorLabelPadding
    //           scale factors = label text sizes ratio (default 12sp/14sp).
    mShiftAmountY = -mItemPaddingTop * 0.5f;
    mScaleUpFactor = 1.f + (14.f - 12.f) / 12.f;
    mScaleDownFactor = 1.f;
}

void NavigationBarItemView::initialize(MenuItemImpl* itemData, int menuType) {
    mItemData = itemData;
    setCheckable(itemData->isCheckable());
    setChecked(itemData->isChecked());
    View::setEnabled(itemData->isEnabled());
    setIcon(itemData->getIcon());
    setTitle(itemData->getTitle());
    setId(itemData->getItemId());
    updateVisibility();
    mInitialized = true;
}

void NavigationBarItemView::setEnabled(bool enabled) {
    // MenuView::ItemView passthrough (AOSP relies on View.setEnabled).
    View::setEnabled(enabled);
}

void NavigationBarItemView::clear() {
    mItemData = nullptr;
    mInitialized = false;
}

void NavigationBarItemView::setItemPosition(int position) {
    mItemPosition = position;
}

int NavigationBarItemView::getItemPosition() const {
    return mItemPosition;
}

LinearLayout* NavigationBarItemView::getLabelGroup() {
    return mLabelGroup;
}

void NavigationBarItemView::setShifting(bool shifting) {
    if (mIsShifting != shifting) {
        mIsShifting = shifting;
        refreshChecked();
    }
}

void NavigationBarItemView::setItemBackground(Drawable* background) {
    // AOSP mutates the shared drawable; CDROID's View owns its background, so
    // derive a private instance from the constant state when possible.
    Drawable* own = background;
    if (own != nullptr) {
        auto cs = own->getConstantState();
        if (cs) own = cs->newDrawable();
        else own = own->mutate();
    }
    setBackground(own);
}

void NavigationBarItemView::setItemBackground(int backgroundResId) {
    if (backgroundResId != 0) {
        setBackgroundResource(backgroundResId);
    } else {
        setBackground(nullptr);
    }
}

void NavigationBarItemView::setIconTintList(const RefPtr<ColorStateList>& tint) {
    mIconTint = tint;
    if (mIconTint != nullptr) {
        mIcon->setImageTintList(mIconTint);
    }
}

void NavigationBarItemView::setIconSize(int iconSize) {
    FrameLayout::LayoutParams* lp = (FrameLayout::LayoutParams*)mIcon->getLayoutParams();
    lp->width = lp->height = iconSize;
    mIcon->setLayoutParams(lp);
}

void NavigationBarItemView::setTextColor(const RefPtr<ColorStateList>& color) {
    mTextColor = color;
    mSmallLabel->setTextColor(color);
    mLargeLabel->setTextColor(color);
}

void NavigationBarItemView::setTextAppearanceInactive(int textAppearanceRes) {
    mTextAppearanceInactive = textAppearanceRes;
    mSmallLabel->setTextAppearance(textAppearanceRes);
    calculateTextScaleFactors();
}

void NavigationBarItemView::setTextAppearanceActive(int textAppearanceRes) {
    mTextAppearanceActive = textAppearanceRes;
    mLargeLabel->setTextAppearance(textAppearanceRes);
    calculateTextScaleFactors();
}

void NavigationBarItemView::setTextAppearanceActiveBoldEnabled(bool isBold) {
    mBoldText = isBold;
    // Bold reaches the active label through the text appearance in material;
    // the substrate keeps the flag for createMenuItem propagation only.
}

void NavigationBarItemView::setItemPaddingTop(int paddingTop) {
    mItemPaddingTop = paddingTop;
    updateItemIconGravity();
}

void NavigationBarItemView::setItemPaddingBottom(int paddingBottom) {
    mItemPaddingBottom = paddingBottom;
    updateItemIconGravity();
}

void NavigationBarItemView::setActiveIndicatorLabelPadding(int activeIndicatorLabelPadding) {
    mActiveIndicatorLabelPadding = activeIndicatorLabelPadding;
    updateItemIconGravity();
}

void NavigationBarItemView::setIconLabelHorizontalSpacing(int iconLabelHorizontalSpacing) {
    mIconLabelHorizontalSpacing = iconLabelHorizontalSpacing;
    updateItemIconGravity();
}

void NavigationBarItemView::setItemGravity(int itemGravity) {
    mItemGravity = itemGravity;
    updateItemIconGravity();
}

void NavigationBarItemView::setActiveIndicatorWidth(int width) {
    mActiveIndicatorDesiredWidth = width;
    updateActiveIndicatorLayoutParams(getWidth());
}

void NavigationBarItemView::setActiveIndicatorHeight(int height) {
    mActiveIndicatorDesiredHeight = height;
    updateActiveIndicatorLayoutParams(getWidth());
}

void NavigationBarItemView::setActiveIndicatorMarginHorizontal(int marginHorizontal) {
    mActiveIndicatorMarginHorizontal = marginHorizontal;
    updateActiveIndicatorLayoutParams(getWidth());
}

void NavigationBarItemView::setActiveIndicatorResizeable(bool resizeable) {
    mActiveIndicatorResizeable = resizeable;
}

void NavigationBarItemView::setActiveIndicatorEnabled(bool enabled) {
    mActiveIndicatorEnabled = enabled;
    updateActiveIndicatorLayoutParams(getWidth());
}

void NavigationBarItemView::setActiveIndicatorColor(int color) {
    mActiveIndicatorColor = color;
}

void NavigationBarItemView::setLabelVisibilityMode(int mode) {
    if (mLabelVisibilityMode != mode) {
        mLabelVisibilityMode = mode;
        refreshChecked();
    }
}

void NavigationBarItemView::updateItemIconGravity() {
    // TOP: vertical stack (icon over labels), the classic bottom-nav item.
    // START: side-by-side (the expanded/navigation-rail item).
    const bool iconAtStart = (mItemIconGravity == NavigationBarView::ITEM_ICON_GRAVITY_START);
    mContentContainer->setOrientation(iconAtStart ? LinearLayout::HORIZONTAL : LinearLayout::VERTICAL);
    mInnerContentContainer->setOrientation(iconAtStart ? LinearLayout::HORIZONTAL : LinearLayout::VERTICAL);
    mInnerContentContainer->setGravity(iconAtStart ? Gravity::CENTER_VERTICAL : Gravity::CENTER_HORIZONTAL);
    FrameLayout::LayoutParams* contentLp = (FrameLayout::LayoutParams*)mContentContainer->getLayoutParams();
    contentLp->gravity = mItemGravity;
    mContentContainer->setLayoutParams(contentLp);
    LinearLayout::LayoutParams* iconLp = (LinearLayout::LayoutParams*)mIconContainer->getLayoutParams();
    iconLp->rightMargin = iconAtStart ? mIconLabelHorizontalSpacing : 0;
    mIconContainer->setLayoutParams(iconLp);
    const int side = iconAtStart ? mActiveIndicatorMarginHorizontal : 0;
    setPadding(side, 0, side, 0);
}

void NavigationBarItemView::setItemIconGravity(int iconGravity) {
    if (mItemIconGravity != iconGravity) {
        mItemIconGravity = iconGravity;
        updateItemIconGravity();
    }
}

void NavigationBarItemView::setExpanded(bool expanded) {
    mExpanded = expanded;
    updateVisibility();
}

bool NavigationBarItemView::isExpanded() const {
    return mExpanded;
}

void NavigationBarItemView::setOnlyShowWhenExpanded(bool onlyShowWhenExpanded) {
    mOnlyShowWhenExpanded = onlyShowWhenExpanded;
    updateVisibility();
}

void NavigationBarItemView::updateVisibility() {
    if (mItemData != nullptr) {
        setVisibility(mItemData->isVisible() && (mExpanded || !mOnlyShowWhenExpanded)
                ? View::VISIBLE : View::GONE);
    }
}

void NavigationBarItemView::setTitle(const std::string& title) {
    mSmallLabel->setText(title);
    mLargeLabel->setText(title);
    if (mItemData == nullptr || mItemData->getContentDescription().empty()) {
        setContentDescription(title);
    }
}

void NavigationBarItemView::setCheckable(bool checkable) {
    refreshChecked();
}

void NavigationBarItemView::setChecked(bool checked) {
    refreshChecked();
    mLargeLabel->setVisibility(checked ? View::VISIBLE : View::INVISIBLE);
    mSmallLabel->setVisibility(checked ? View::INVISIBLE : View::VISIBLE);
    maybeSetActiveIndicatorPill(mActiveIndicatorView, checked);
}

void NavigationBarItemView::refreshChecked() {
    // The shifting mode scales the checked item's label up and shifts the
    // icon down; material animates this (AutoTransition in the menu view +
    // per-label scale factors) - the substrate applies the end state.
    const bool labeled = (mLabelVisibilityMode != NavigationBarView::LABEL_VISIBILITY_UNLABELED);
    mLabelGroup->setVisibility(labeled ? View::VISIBLE : View::GONE);
    if (labeled) {
        const float scale = mIsShifting ? (mItemData && mItemData->isChecked()
                ? mScaleUpFactor : mScaleDownFactor) : 1.f;
        mLabelGroup->setScaleX(scale);
        mLabelGroup->setScaleY(scale);
    }
}

void NavigationBarItemView::maybeSetActiveIndicatorPill(View* indicatorView, bool checked) {
    if (!mActiveIndicatorEnabled || mActiveIndicatorDesiredWidth <= 0
            || mActiveIndicatorDesiredHeight <= 0) {
        indicatorView->setVisibility(View::INVISIBLE);
        return;
    }
    if (checked) {
        GradientDrawable* pill = new GradientDrawable();
        pill->setShape(GradientDrawable::RECTANGLE);
        pill->setColor(mActiveIndicatorColor);
        pill->setCornerRadius(mActiveIndicatorDesiredHeight / 2.f);
        indicatorView->setBackground(pill);
        FrameLayout::LayoutParams* lp =
                (FrameLayout::LayoutParams*)indicatorView->getLayoutParams();
        lp->width = mActiveIndicatorDesiredWidth;
        lp->height = mActiveIndicatorDesiredHeight;
        lp->gravity = Gravity::CENTER;
        indicatorView->setLayoutParams(lp);
        indicatorView->setVisibility(View::VISIBLE);
    } else {
        indicatorView->setVisibility(View::INVISIBLE);
    }
}

void NavigationBarItemView::updateActiveIndicatorLayoutParams(int availableWidth) {
    // Resizeable indicators clamp to the item width (material's
    // ActiveIndicatorTransform width fraction at 1 for the labeled case).
    if (!mActiveIndicatorResizeable || availableWidth <= 0
            || mActiveIndicatorDesiredWidth <= 0) {
        return;
    }
    const int maxWidth = availableWidth - 2 * mActiveIndicatorMarginHorizontal;
    FrameLayout::LayoutParams* lp = (FrameLayout::LayoutParams*)mActiveIndicatorView->getLayoutParams();
    lp->width = std::min(mActiveIndicatorDesiredWidth, maxWidth);
    mActiveIndicatorView->setLayoutParams(lp);
}

void NavigationBarItemView::setIcon(Drawable* icon) {
    mOriginalIconDrawable = icon;
    if (icon != nullptr) {
        mIcon->setImageDrawable(icon);
    }
}

void NavigationBarItemView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    // AOSP measures the label at both text sizes and takes the max as the
    // group's measured size (BaselineLayout); the small/large pair centered in
    // one LinearLayout yields the same maxima via child measurement.
    const int specWidth = MeasureSpec::getSize(widthMeasureSpec);
    const int maxWidth = specWidth - getPaddingLeft() - getPaddingRight();
    if (maxWidth > 0) {
        mLargeLabel->measure(
                MeasureSpec::makeMeasureSpec(maxWidth, MeasureSpec::AT_MOST),
                MeasureSpec::makeMeasureSpec(0, MeasureSpec::UNSPECIFIED));
        mSmallLabel->measure(
                MeasureSpec::makeMeasureSpec(maxWidth, MeasureSpec::AT_MOST),
                MeasureSpec::makeMeasureSpec(0, MeasureSpec::UNSPECIFIED));
        const int labelWidth = std::max(mLargeLabel->getMeasuredWidth(),
                                        mSmallLabel->getMeasuredWidth());
        LinearLayout::LayoutParams* lp = (LinearLayout::LayoutParams*)mLabelGroup->getLayoutParams();
        lp->width = labelWidth;
        mLabelGroup->setLayoutParams(lp);
    }
    FrameLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);
}

} // namespace cdroid
