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
#ifndef __LIST_MENUITEM_VIEW_H__
#define __LIST_MENUITEM_VIEW_H__
#include <menu/menuview.h>
#include <widget/linearlayout.h>
namespace cdroid{
class TextView;
class CheckBox;
class ImageView;
class RadioButton;
class ListMenuItemView:public LinearLayout,public MenuView::ItemView{//, AbsListView::SelectionBoundsAdjuster {
private:
    MenuItemImpl* mItemData;
    ImageView* mIconView;
    RadioButton* mRadioButton;
    TextView* mTitleView;
    CheckBox* mCheckBox;
    TextView* mShortcutView;
    ImageView* mSubMenuArrowView;
    ImageView* mGroupDivider;
    LinearLayout* mContent;

    Drawable* mBackground;
    std::string mTextAppearance;
    int mMenuType;
    Context* mTextAppearanceContext;
    Drawable* mSubMenuArrow;
    LayoutInflater* mInflater;
    bool mPreserveIconSpacing;
    bool mHasListDivider;
    bool mForceShowIcon;
private:
    void addContentView(View* v);
    void addContentView(View* v, int index);
    void setSubMenuArrowVisible(bool hasSubmenu);
    void insertIconView();
    void insertRadioButton();
    void insertCheckBox();
    LayoutInflater* getInflater();
protected:
    void onFinishInflate() override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
public:
    ListMenuItemView(Context* context,const AttributeSet& attrs);

    void initialize(MenuItemImpl* itemData, int menuType)override;

    void setForceShowIcon(bool forceShow);

    void setTitle(const std::string& title)override;

    MenuItemImpl* getItemData()override;
    void setEnabled(bool)override;
    void setCheckable(bool checkable)override;

    void setChecked(bool checked)override;

    void setShortcut(bool showShortcut, int shortcutKey)override;

    void setIcon(Drawable* icon)override;

    bool prefersCondensedTitle()const override;

    bool showsIcon()override;

    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info)override;
    /**
     * Enable or disable group dividers for this view.
     */
    void setGroupDividerEnabled(bool groupDividerEnabled);
    void adjustListItemSelectionBounds(Rect& rect);
};
}/*endof namespace*/
#endif/*__LIST_MENUITEM_VIEW_H__*/
