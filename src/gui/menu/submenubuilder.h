#ifndef __SUBMENU_BUILDER_H__
#define __SUBMENU_BUILDER_H__
#include <menu/submenu.h>
#include <menu/menuitemimpl.h>
#include <menu/menubuilder.h>
namespace cdroid{
class SubMenuBuilder:public MenuBuilder,public SubMenu {
private:
    MenuBuilder* mParentMenu;
    MenuItemImpl* mItem;
public:
    SubMenuBuilder(Context*context, MenuBuilder* parentMenu, MenuItemImpl* item):MenuBuilder(context){
        mParentMenu = parentMenu;
        mItem = item;
    }

    void setQwertyMode(bool isQwerty) override{
        mParentMenu->setQwertyMode(isQwerty);
    }

    bool isQwertyMode() const override{
        return mParentMenu->isQwertyMode();
    }

    void setShortcutsVisible(bool shortcutsVisible) override{
        mParentMenu->setShortcutsVisible(shortcutsVisible);
    }

    bool isShortcutsVisible() override{
        return mParentMenu->isShortcutsVisible();
    }

    Menu* getParentMenu() {
        return mParentMenu;
    }

    MenuItem* getInvokerItem()override {
        return mItem;
    }

    void setCallback(const Callback& callback) override{
        mParentMenu->setCallback(callback);
    }

    MenuBuilder* getRootMenu() override{
        return mParentMenu->getRootMenu();
    }

    bool dispatchMenuItemSelected(MenuBuilder& menu, MenuItem& item) override{
        return MenuBuilder::dispatchMenuItemSelected(menu, item) ||
                mParentMenu->dispatchMenuItemSelected(menu, item);
    }

    SubMenu& setIcon(Drawable* icon) override{
        mItem->setIcon(icon);
        return *this;
    }

    SubMenu& setIcon(const std::string& iconRes) override{
        mItem->setIcon(iconRes);
        return *this;
    }

    SubMenu& setHeaderIcon(Drawable* icon) override{
        return (SubMenu&) MenuBuilder::setHeaderIconInt(icon);
    }

    SubMenu& setHeaderIcon(const std::string& iconRes) override{
        return (SubMenu&) MenuBuilder::setHeaderIconInt(iconRes);
    }

    SubMenu& setHeaderTitle(const std::string& title) override{
        return (SubMenu&) MenuBuilder::setHeaderTitleInt(title);
    }

    SubMenu& setHeaderView(View* view)override{
        return (SubMenu&) MenuBuilder::setHeaderViewInt(view);
    }
    void clearHeader()override{}
    bool expandItemActionView(MenuItemImpl* item) override{
        return mParentMenu->expandItemActionView(item);
    }

    bool collapseItemActionView(MenuItemImpl* item) override{
        return mParentMenu->collapseItemActionView(item);
    }

    std::string getActionViewStatesKey() override{
        const int itemId = mItem != nullptr ? mItem->getItemId() : 0;
        if (itemId == 0) {
            return "";
        }
        return MenuBuilder::getActionViewStatesKey() + ":" + std::to_string(itemId);
    }

    void setGroupDividerEnabled(bool groupDividerEnabled) override{
        mParentMenu->setGroupDividerEnabled(groupDividerEnabled);
    }

    bool isGroupDividerEnabled() override{
        return mParentMenu->isGroupDividerEnabled();
    }
};
}/*endof namespace*/
#endif/*__SUBMENU_BUILDER_H__*/
