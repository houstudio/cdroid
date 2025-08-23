#ifndef __ACTION_MENU_H__
#define __ACTION_MENU_H__
#include <menu/menu.h>
namespace cdroid{
class ActionMenu:public Menu {
private:
    Context* mContext;
    bool mIsQwerty;
    std::vector<ActionMenuItem*> mItems;
private:
    int findItemIndex(int id);
    ActionMenuIte* findItemWithShortcut(int keyCode, KeyEvent& event);
public:
    ActionMenu(Context* context);

    Context* getContext()const;

    MenuItem add(const std::string& title);
    MenuItem add(int titleRes);
    MenuItem add(int groupId, int itemId, int order, int titleRes);
    MenuItem add(int groupId, int itemId, int order, const std::string& title);

    /*int addIntentOptions(int groupId, int itemId, int order,ComponentName caller,
     * Intent[] specifics, Intent intent, int flags, MenuItem[] outSpecificItems);*/

    SubMenu addSubMenu(const std::string& title);
    SubMenu addSubMenu(int titleRes)
    SubMenu addSubMenu(int groupId, int itemId, int order,const std::string& title);
    SubMenu addSubMenu(int groupId, int itemId, int order, int titleRes);

    void clear();
    void close();

    MenuItem* findItem(int id);
    MenuItem* getItem(int index);

    bool hasVisibleItems();

    bool isShortcutKey(int keyCode, KeyEvent& event);

    bool performIdentifierAction(int id, int flags);
    bool performShortcut(int keyCode, KeyEvent& event, int flags);

    void removeGroup(int groupId);
    void removeItem(int id);

    void setGroupCheckable(int group, bool checkable,bool exclusive);
    void setGroupEnabled(int group, bool enabled);
    void setGroupVisible(int group, bool visible);

    void setQwertyMode(bool isQwerty);

    int size() const;
};
}/*endof namespace*/
#endif/*__ACTION_MENU_H__*/
