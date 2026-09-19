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
#ifndef __NAVIGATION_BAR_ITEM_VIEW_H__
#define __NAVIGATION_BAR_ITEM_VIEW_H__
// Port of com.google.android.material.navigation.NavigationBarItemView - the
// destination item inside a NavigationBarMenuView. Material inflates
// design_bottom_navigation_item (content/active-indicator/icon containers plus
// a small/large label pair for the shifting scale animation); CDROID builds
// the same hierarchy in code. Substrate adaptations: the active indicator is a
// GradientDrawable pill applied instantly on check state (material animates it
// with an ActiveIndicatorTransform + ValueAnimator), and badges
// (BadgeDrawable) are not ported.
#include <widget/framelayout.h>
#include <widget/linearlayout.h>
#include <widget/imageview.h>
#include <widget/textview.h>
#include <drawable/gradientdrawable.h>
#include <menu/menuview.h>

namespace cdroid{

class MenuItemImpl;
class ColorStateList;

class NavigationBarItemView : public FrameLayout, public MenuView::ItemView {
private:
    static constexpr int INVALID_ITEM_POSITION = -1;

    bool mInitialized = false;
    RefPtr<ColorStateList> mItemRippleColor;
    Drawable* mItemBackground = nullptr;
    int mItemPaddingTop;
    int mItemPaddingBottom;
    int mActiveIndicatorLabelPadding;
    int mIconLabelHorizontalSpacing;
    float mShiftAmountY;
    float mScaleUpFactor;
    float mScaleDownFactor;

    int mLabelVisibilityMode;
    bool mIsShifting;

    LinearLayout* mContentContainer;
    LinearLayout* mInnerContentContainer;
    View* mActiveIndicatorView;
    FrameLayout* mIconContainer;
    ImageView* mIcon;
    FrameLayout* mLabelGroup;    // BaselineLayout in material: small/large
    TextView* mSmallLabel;       // LinearLayout serves the same role here.
    TextView* mLargeLabel;

    int mItemPosition;
    int mTextAppearanceActive;
    int mTextAppearanceInactive;
    RefPtr<ColorStateList> mTextColor;
    bool mBoldText = false;

    MenuItemImpl* mItemData = nullptr;
    RefPtr<ColorStateList> mIconTint;
    Drawable* mOriginalIconDrawable = nullptr;

    // Active indicator geometry (the pill behind the checked item's icon).
    bool mActiveIndicatorEnabled = false;
    int mActiveIndicatorDesiredWidth = 0;
    int mActiveIndicatorDesiredHeight = 0;
    bool mActiveIndicatorResizeable = false;
    int mActiveIndicatorMarginHorizontal = 0;
    int mActiveIndicatorColor = 0;

    int mItemIconGravity;   // NavigationBarView::ITEM_ICON_GRAVITY_*
    int mItemGravity;       // NavigationBarView::ITEM_GRAVITY_*
    bool mExpanded = false;
    bool mOnlyShowWhenExpanded = false;

    void calculateTextScaleFactors();
    void updateItemIconGravity();
    void updateVisibility();
    void updateActiveIndicatorLayoutParams(int availableWidth);
    void refreshItemBackground();
    void refreshChecked();
    int getSuggestedIconWidth() const;
    void maybeSetActiveIndicatorPill(View* indicatorView, bool checked);
public:
    NavigationBarItemView(Context* context, const AttributeSet* attrs = nullptr);
    ~NavigationBarItemView() override;

    // MenuView::ItemView
    void initialize(MenuItemImpl* itemData, int menuType) override;
    MenuItemImpl* getItemData() override { return mItemData; }
    void setTitle(const std::string& title) override;
    void setCheckable(bool checkable) override;
    void setChecked(bool checked) override;
    std::vector<int> onCreateDrawableState(int extraSpace) override;
    void setEnabled(bool enabled) override;
    void setShortcut(bool showShortcut, int shortcutKey) override {}
    void setIcon(Drawable* icon) override;
    bool prefersCondensedTitle() const override { return false; }
    bool showsIcon() override { return true; }

    /** Reset state so this view can be reused from the item pool. */
    void clear();
    void setItemPosition(int position);
    int getItemPosition() const;

    FrameLayout* getLabelGroup();
    void setShifting(bool shifting);
    void setItemBackground(Drawable* background);
    void setItemBackground(int backgroundResId);
    void setIconTintList(const RefPtr<ColorStateList>& tint);
    void setIconSize(int iconSize);
    void setTextColor(const RefPtr<ColorStateList>& color);
    void setTextAppearanceInactive(int textAppearanceRes);
    void setTextAppearanceActive(int textAppearanceRes);
    void setTextAppearanceActiveBoldEnabled(bool isBold);
    void setItemPaddingTop(int paddingTop);
    void setItemPaddingBottom(int paddingBottom);
    void setActiveIndicatorLabelPadding(int activeIndicatorLabelPadding);
    void setIconLabelHorizontalSpacing(int iconLabelHorizontalSpacing);
    void setItemGravity(int itemGravity);
    void setActiveIndicatorWidth(int width);
    void setActiveIndicatorHeight(int height);
    void setActiveIndicatorMarginHorizontal(int marginHorizontal);
    void setActiveIndicatorResizeable(bool resizeable);
    void setActiveIndicatorEnabled(bool enabled);
    void setActiveIndicatorColor(int color);
    void setLabelVisibilityMode(int mode);
    void setItemIconGravity(int iconGravity);
    void setExpanded(bool expanded);
    bool isExpanded() const;
    void setOnlyShowWhenExpanded(bool onlyShowWhenExpanded);

    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
};

} // namespace cdroid
#endif /* __NAVIGATION_BAR_ITEM_VIEW_H__ */
