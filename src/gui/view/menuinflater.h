#ifndef __MENU_INFLATER_H__
#define __MENU_INFLATER_H__
#include <view/menu.h>
namespace cdroid{ 
class Context;
class XmlPullParser;
class AttributeSet;
class ColorStateList;
class ActionProvider;
class MenuInflater {
private:
    class MenuState;
    static constexpr int NO_ID = 0;
    //static final Class<?>[] ACTION_VIEW_CONSTRUCTOR_SIGNATURE = new Class[] {Context.class};
    //static final Class<?>[] ACTION_PROVIDER_CONSTRUCTOR_SIGNATURE = ACTION_VIEW_CONSTRUCTOR_SIGNATURE;
    //Object[] mActionViewConstructorArguments;
    //Object[] mActionProviderConstructorArguments;

    Context* mContext;
    void* mRealOwner;
private:
    void parseMenu(XmlPullParser& parser, AttributeSet& attrs, Menu* menu);
    void registerMenu(MenuItem& item,AttributeSet& set);
    void registerMenu(SubMenu& subMenu,AttributeSet& set);
    Object* getRealOwner();
    Object* findRealOwner(Object owner);
public:
    MenuInflater(Context* context);
    /**
     * Constructs a menu inflater.
     *
     * @see Activity#getMenuInflater()
     * @hide
     */
    MenuInflater(Context* context, Object realOwner);

    /**
     * Inflate a menu hierarchy from the specified XML resource. Throws
     * {@link InflateException} if there is an error.
     *
     * @param menuRes Resource ID for an XML layout resource to load (e.g.,
     *            <code>R.menu.main_activity</code>)
     * @param menu The Menu to inflate into. The items and submenus will be
     *            added to this Menu.
     */
    void inflate(const std::string&menuRes, Menu* menu);

    Context* getContext();
};/*endof MenuInflater*/

/**
 * State for the current menu.
 * <p>
 * Groups can not be nested unless there is another menu (which will have
 * its state class).
 */
class MenuInflater::MenuState {
private:
    friend MenuInflater;
    Menu* menu;
    /*
     * Group state is set on items as they are added, allowing an item to
     * override its group state. (As opposed to set on items at the group end tag.)
     */
    int groupId;
    int groupCategory;
    int groupOrder;
    int groupCheckable;
    bool groupVisible;
    bool groupEnabled;

    bool itemAdded;
    int itemId;
    int itemCategoryOrder;
    std::string itemTitle;
    std::string itemTitleCondensed;
    int itemIconResId;
    ColorStateList* itemIconTintList;
    int mItemIconBlendMode;
    char itemAlphabeticShortcut;
    int itemAlphabeticModifiers;
    char itemNumericShortcut;
    int itemNumericModifiers;
    int itemCheckable;
    bool itemChecked;
    bool itemVisible;
    bool itemEnabled;

    int itemShowAsAction;
    int itemActionViewLayout;
    std::string itemActionViewClassName;
    std::string itemActionProviderClassName;

    std::string itemListenerMethodName;

    ActionProvider* itemActionProvider;

    std::string itemContentDescription;
    std::string itemTooltipText;

    static constexpr int defaultGroupId = NO_ID;
    static constexpr int defaultItemId = NO_ID;
    static constexpr int defaultItemCategory = 0;
    static constexpr int defaultItemOrder = 0;
    static constexpr int defaultItemCheckable = 0;
    static constexpr bool defaultItemChecked = false;
    static constexpr bool defaultItemVisible = true;
    static constexpr bool defaultItemEnabled = true;
private:
    char getShortcut(const std::string& shortcutString);
    void setItem(MenuItem* item);
public:
    MenuState(Menu* menu);
    virtual ~MenuState();
    void resetGroup();

    /**
     * Called when the parser is pointing to a group tag.
     */
    void readGroup(const AttributeSet& attrs);

    /**
     * Called when the parser is pointing to an item tag.
     */
    void readItem(const AttributeSet& attrs);

    MenuItem* addItem();

    SubMenu* addSubMenuItem();

    bool hasAddedItem() const;
};
}/*endof namespace*/
#endif 
