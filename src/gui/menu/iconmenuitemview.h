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
#ifndef __ICON_MENUITEM_VIEW_H__
#define __ICON_MENUITEM_VIEW_H__
#include <menu/iconmenuview.h>
#include <menu/menubuilder.h>
#include <widget/textview.h>
namespace cdroid{
class IconMenuItemView:public TextView,public MenuView::ItemView {
private:
    static constexpr int NO_ALPHA = 0xFF;
    IconMenuView* mIconMenuView;
    MenuBuilder::ItemInvoker mItemInvoker;
    MenuItemImpl* mItemData;

    Drawable* mIcon;
    std::string mTextAppearance;
    Context* mTextAppearanceContext;
    float mDisabledAlpha;

    Rect mPositionIconAvailable;
    Rect mPositionIconOutput;

    bool mShortcutCaptionMode;
    std::string mShortcutCaption;
    static std::string sPrependShortcutLabel;
private:
    friend IconMenuView;
    void positionIcon();
protected:
    void setCaptionMode(bool shortcut);

    void drawableStateChanged()override;
    void onLayout(bool changed, int left, int top, int right, int bottom)override;
    void onTextChanged(const std::wstring& text, int start, int before, int after)override;
public:
    IconMenuItemView(Context* context,const AttributeSet& attrs);
    void initialize(const std::string& title, Drawable* icon);
    void initialize(MenuItemImpl* itemData, int menuType);
    void setItemData(MenuItemImpl* data);
    bool performClick()override;
    void setTitle(const std::string& title);
    void setIcon(Drawable* icon);
    void setItemInvoker(const MenuBuilder::ItemInvoker& itemInvoker);

    MenuItemImpl* getItemData()override;
    void setEnabled(bool)override;
    void setVisibility(int v)override;
    void setIconMenuView(IconMenuView* iconMenuView);
    IconMenuView::LayoutParams* getTextAppropriateLayoutParams();
    void setCheckable(bool checkable);
    void setChecked(bool checked);
    void setShortcut(bool showShortcut, char shortcutKey);
    bool prefersCondensedTitle();
    bool showsIcon();
};
}/*endof namespace*/
#endif/*__ICON_MENUITEM_VIEW_H__*/
