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
#ifndef __NAVIGATION_BAR_MENU_VIEW_H__
#define __NAVIGATION_BAR_MENU_VIEW_H__
// Port of com.google.android.material.navigation.NavigationBarMenuView - the
// abstract ViewGroup rendering a menu inside a NavigationBarView. Owns the
// button array, the item pool (Pools.SynchronizedPool in material; a plain
// vector here) and the presentation fields propagated into every item.
// Substrate adaptations: badges (BadgeDrawable), the label AutoTransition
// (TransitionManager), and submenu subheader/divider views are not ported.
#include <view/viewgroup.h>
#include <menu/menuview.h>
#include <widgetEx/navigationview/navigationbarmenubuilder.h>
#include <widgetEx/navigationview/navigationbaritemview.h>
#include <vector>

namespace cdroid{

class NavigationBarPresenter;
class ColorStateList;

class NavigationBarMenuView : public ViewGroup, public MenuView {
private:
    static constexpr int NO_PADDING = -1;
    static constexpr int NO_SELECTED_ITEM = -1;

    View::OnClickListener mOnClickListener;
    std::vector<NavigationBarItemView*> mItemPool;

    int mLabelVisibilityMode;
    int mItemIconGravity;

    std::vector<NavigationBarItemView*> mButtons;

    int mSelectedItemId = 0;
    int mSelectedItemPosition = NO_SELECTED_ITEM;

    RefPtr<ColorStateList> mItemIconTint;
    int mItemIconSize;
    RefPtr<ColorStateList> mItemTextColorFromUser;
    RefPtr<ColorStateList> mItemTextColorDefault;
    int mItemTextAppearanceInactive;
    int mItemTextAppearanceActive;
    bool mItemTextAppearanceActiveBoldEnabled;
    Drawable* mItemBackground;
    RefPtr<ColorStateList> mItemRippleColor;
    int mItemBackgroundRes;
    int mItemPaddingTop = NO_PADDING;
    int mItemPaddingBottom = NO_PADDING;
    int mItemActiveIndicatorLabelPadding = NO_PADDING;
    int mIconLabelHorizontalSpacing = NO_PADDING;
    bool mItemActiveIndicatorEnabled = false;
    int mItemActiveIndicatorWidth;
    int mItemActiveIndicatorHeight;
    int mItemActiveIndicatorMarginHorizontal;
    int mItemGravity;
    bool mItemActiveIndicatorResizeable = false;
    int mItemActiveIndicatorColor;

    NavigationBarPresenter* mPresenter;
    NavigationBarMenuBuilder* mMenu = nullptr;
    bool mExpanded = false;
    MenuItem* mCheckedItem = nullptr;
    int mItemPoolSize = 0;
    static constexpr int DEFAULT_COLLAPSED_MAX_COUNT = 7;
    int mCollapsedMaxItemCount = DEFAULT_COLLAPSED_MAX_COUNT;

    void releaseItemPool();
    NavigationBarItemView* createMenuItem(int index, MenuItemImpl* item, bool shifting);
    NavigationBarItemView* getNewItem();
    bool isMenuStructureSame() const;
    int getCollapsedVisibleItemCount() const;
protected:
    NavigationBarMenuView(Context* context, const AttributeSet* attrs);
    virtual NavigationBarItemView* createNavigationBarItemView(Context* context) = 0;
    bool isShifting(int labelVisibilityMode, int childCount) const;
    int getSelectedItemPosition() const;
    NavigationBarMenuBuilder* getMenu() const;
public:
    ~NavigationBarMenuView() override;

    void setCheckedItem(MenuItem* checkedItem);
    void setExpanded(bool expanded);
    bool isExpanded() const;

    // MenuView
    void initialize(MenuBuilder* menu) override;
    int getWindowAnimations() override { return 0; }

    void setIconTintList(const RefPtr<ColorStateList>& tint);
    const RefPtr<ColorStateList> getIconTintList() const;
    void setItemIconSize(int iconSize);
    int getItemIconSize() const;
    void setItemTextColor(const RefPtr<ColorStateList>& color);
    const RefPtr<ColorStateList> getItemTextColor() const;
    void setItemTextAppearanceInactive(int textAppearanceRes);
    int getItemTextAppearanceInactive() const;
    void setItemTextAppearanceActive(int textAppearanceRes);
    void setItemTextAppearanceActiveBoldEnabled(bool isBold);
    int getItemTextAppearanceActive() const;
    void setItemBackgroundRes(int background);
    int getItemBackgroundRes() const;
    void setItemBackground(Drawable* background);
    Drawable* getItemBackground() const;
    void setItemRippleColor(const RefPtr<ColorStateList>& itemRippleColor);
    const RefPtr<ColorStateList> getItemRippleColor() const;
    void setItemPaddingTop(int paddingTop);
    int getItemPaddingTop() const;
    void setItemPaddingBottom(int paddingBottom);
    int getItemPaddingBottom() const;
    void setActiveIndicatorLabelPadding(int activeIndicatorLabelPadding);
    int getActiveIndicatorLabelPadding() const;
    void setIconLabelHorizontalSpacing(int iconLabelHorizontalSpacing);
    int getIconLabelHorizontalSpacing() const;
    void setItemActiveIndicatorEnabled(bool enabled);
    bool getItemActiveIndicatorEnabled() const;
    void setItemActiveIndicatorWidth(int width);
    void setItemActiveIndicatorHeight(int height);
    void setItemActiveIndicatorMarginHorizontal(int marginHorizontal);
    void setItemGravity(int itemGravity);
    int getItemGravity() const;
    void setItemActiveIndicatorResizeable(bool resizeable);
    void setItemActiveIndicatorColor(int color);
    void setLabelVisibilityMode(int labelVisibilityMode);
    int getLabelVisibilityMode() const;
    void setItemIconGravity(int itemIconGravity);
    int getItemIconGravity() const;

    void setPresenter(NavigationBarPresenter* presenter);
    void buildMenuView();
    void updateMenuView();
    int getCurrentVisibleContentItemCount() const;
    int getSelectedItemId() const;
    void tryRestoreSelectedItemId(int itemId);
};

} // namespace cdroid
#endif /* __NAVIGATION_BAR_MENU_VIEW_H__ */
