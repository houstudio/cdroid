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
