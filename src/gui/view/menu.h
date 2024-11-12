#ifndef __MENU_H__
#define __MENU_H__
#include <string>
#include <core/callbackbase.h>
#include <view/keyevent.h>
#include <cairomm/surface.h>
namespace cdroid{
class View;
class SubMenu;
class MenuItem;
class Drawable;
#ifndef DECLARE_UIEVENT
#define DECLARE_UIEVENT(type, name, ...) using name = std::function<type(__VA_ARGS__)>
#endif
class Menu{
public:
    /**
     * This is the part of an order integer that the user can provide.
     * @hide
     */
    static constexpr int USER_MASK = 0x0000ffff;
    /**
     * Bit shift of the user portion of the order integer.
     * @hide
     */
    static constexpr int USER_SHIFT = 0;

    /**
     * This is the part of an order integer that supplies the category of the
     * item.
     * @hide
     */
    static constexpr int CATEGORY_MASK = 0xffff0000;
    /**
     * Bit shift of the category portion of the order integer.
     * @hide
     */
    static constexpr int CATEGORY_SHIFT = 16;

    /**
     * A mask of all supported modifiers for MenuItem's keyboard shortcuts
     */
    static constexpr int SUPPORTED_MODIFIERS_MASK = KeyEvent::META_META_ON | KeyEvent::META_CTRL_ON
            | KeyEvent::META_ALT_ON | KeyEvent::META_SHIFT_ON | KeyEvent::META_SYM_ON | KeyEvent::META_FUNCTION_ON;

    /**
     * Value to use for group and item identifier integers when you don't care
     * about them.
     */
    static constexpr int NONE = 0;

    /**
     * First value for group and item identifier integers.
     */
    static constexpr int FIRST = 1;

    // Implementation note: Keep these CATEGORY_* in sync with the category enum
    // in attrs.xml

    /**
     * Category code for the order integer for items/groups that are part of a
     * container -- or/add this with your base value.
     */
    static constexpr int CATEGORY_CONTAINER = 0x00010000;

    /**
     * Category code for the order integer for items/groups that are provided by
     * the system -- or/add this with your base value.
     */
    static constexpr int CATEGORY_SYSTEM = 0x00020000;

    /**
     * Category code for the order integer for items/groups that are
     * user-supplied secondary (infrequently used) options -- or/add this with
     * your base value.
     */
    static constexpr int CATEGORY_SECONDARY = 0x00030000;

    /**
     * Category code for the order integer for items/groups that are
     * alternative actions on the data that is currently displayed -- or/add
     * this with your base value.
     */
    static constexpr int CATEGORY_ALTERNATIVE = 0x00040000;

    /**
     * Flag for {@link #addIntentOptions}: if set, do not automatically remove
     * any existing menu items in the same group.
     */
    static constexpr int FLAG_APPEND_TO_GROUP = 0x0001;

    /**
     * Flag for {@link #performShortcut}: if set, do not close the menu after
     * executing the shortcut.
     */
    static constexpr int FLAG_PERFORM_NO_CLOSE = 0x0001;

    /**
     * Flag for {@link #performShortcut(int, KeyEvent, int)}: if set, always
     * close the menu after executing the shortcut. Closing the menu also resets
     * the prepared state.
     */
    static constexpr int FLAG_ALWAYS_PERFORM_CLOSE = 0x0002;
protected:
    std::vector<MenuItem*>mMenuItems;
    void *owner;/*menu owner/container*/
public:
    Menu();
    ~Menu();
    MenuItem* add(const std::string&title);
    MenuItem* add(int groupId, int itemId, int order,const std::string&title);
    SubMenu* addSubMenu(const std::string&title);
    SubMenu* addSubMenu(int groupId,int itemId,int order,const std::string& title);
    void popup(int x,int y);
    void close();
    void removeItem(int id);
    void removeGroup(int groupId);
    void clear();
    void setGroupCheckable(int group, bool checkable, bool exclusive);
    void setGroupEnabled(int group, bool enabled);
    bool hasVisibleItems()const;
    MenuItem*findItem(int id)const;
    int size()const;
    virtual MenuItem* getItem(int index)const;
    bool performShortcut(int keyCode, KeyEvent& event, int flags);
    bool isShortcutKey(int keyCode, KeyEvent& event);
    bool performIdentifierAction(int id, int flags);
    void setQwertyMode(bool isQwerty);
     void setGroupDividerEnabled(bool groupDividerEnabled);
};

class MenuItem{
public:
    /*
     * These should be kept in sync with attrs.xml enum constants for showAsAction
     */
    /** Never show this item as a button in an Action Bar. */
    static constexpr int SHOW_AS_ACTION_NEVER = 0;
    /** Show this item as a button in an Action Bar if the system decides there is room for it. */
    static constexpr int SHOW_AS_ACTION_IF_ROOM = 1;
    /**
     * Always show this item as a button in an Action Bar.
     * Use sparingly! If too many items are set to always show in the Action Bar it can
     * crowd the Action Bar and degrade the user experience on devices with smaller screens.
     * A good rule of thumb is to have no more than 2 items set to always show at a time.
     */
    static constexpr int SHOW_AS_ACTION_ALWAYS = 2;

    /**
     * When this item is in the action bar, always show it with a text label even if
     * it also has an icon specified.
     */
    static constexpr int SHOW_AS_ACTION_WITH_TEXT = 4;

    /**
     * This item's action view collapses to a normal menu item.
     * When expanded, the action view temporarily takes over
     * a larger segment of its container.
     */
    static constexpr int SHOW_AS_ACTION_COLLAPSE_ACTION_VIEW = 8;
public:
    DECLARE_UIEVENT(void,OnMenuItemClickListener,MenuItem&);
    struct OnActionExpandListener{
        CallbackBase<bool,MenuItem&>onMenuItemActionExpand;
        CallbackBase<bool,MenuItem&>onMenuItemActionCollapse;
    };
protected:
    std::string mTitle;
    std::string mTooltip;
    int mItemId;
    int mGroupId;
    int mOrder;
    bool mCheckable;
    bool mChecked;
    bool mVisible;
    bool mEnabled;
    Cairo::RefPtr<Cairo::ImageSurface>mIcon;
    SubMenu*mSubMenu;
    friend class Menu;
public:
    MenuItem();
    MenuItem& setEnabled(bool v);
    MenuItem& setVisible(bool v);
    MenuItem& setTitle(const std::string&);
    std::string getTitle()const{return mTitle;}
    MenuItem& setTooltipText(const std::string&tips);
    std::string getTooltipText()const{return mTooltip;}
    SubMenu* getSubMenu()const;
    bool hasSubMenu()const;

    bool isEnabled()const{return mEnabled;}
    bool isVisible()const{return mVisible;}
    bool isCheckable()const{return mCheckable;}
    bool isChecked()const{return mChecked;}
    int getGroupId()const{return mGroupId;}
    int getItemId()const{return mItemId;}
    int getOrder()const{return mOrder;}
};

class SubMenu:public Menu{
public:
    SubMenu& setHeaderTitle(const std::string&);
    SubMenu& setHeaderIcon(const std::string&iconRes);
    SubMenu& setHeaderIcon(Drawable*);
    SubMenu& setHeaderView(View*);
    void clearHeader();
    SubMenu& setIcon(const std::string&iconRes);
    SubMenu& setIcon(Drawable*);
    MenuItem*getItem()const;
};

class ContextMenu:public Menu {
public:

    /**
     * Sets the context menu header's title to the title given in <var>title</var>.
     *
     * @param title The character sequence used for the title.
     * @return This ContextMenu so additional setters can be called.
     */
    virtual ContextMenu& setHeaderTitle(const std::string& title)=0;

    /**
     * Sets the context menu header's icon to the icon given in <var>iconRes</var>
     * resource id.
     *
     * @param iconRes The resource identifier used for the icon.
     * @return This ContextMenu so additional setters can be called.
     */
    virtual ContextMenu& setHeaderIcon(const std::string& iconRes)=0;

    /**
     * Sets the context menu header's icon to the icon given in <var>icon</var>
     * {@link Drawable}.
     *
     * @param icon The {@link Drawable} used for the icon.
     * @return This ContextMenu so additional setters can be called.
     */
    virtual ContextMenu& setHeaderIcon(Drawable* icon)=0;

    /**
     * Sets the header of the context menu to the {@link View} given in
     * <var>view</var>. This replaces the header title and icon (and those
     * replace this).
     *
     * @param view The {@link View} used for the header.
     * @return This ContextMenu so additional setters can be called.
     */
    virtual ContextMenu& setHeaderView(View* view)=0;

    /**
     * Clears the header of the context menu.
     */
    virtual void clearHeader()=0;

    /**
     * Additional information regarding the creation of the context menu.  For example,
     * {@link AdapterView}s use this to pass the exact item position within the adapter
     * that initiated the context menu.
     */
    struct ContextMenuInfo {
    };
};
}

#endif
