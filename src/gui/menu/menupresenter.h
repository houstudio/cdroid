#ifndef __MENU_PRESENTER_H__
#define __MENU_PRESENTER_H__
#include <functional>
#include <view/viewgroup.h>
namespace cdroid{
class MenuView;
class MenuBuilder;
class SubMenuBuilder;
class MenuPresenter {
public:
    /**
     * Called by menu implementation to notify another component of open/close events.
     */
    struct Callback {
        /**
         * Called when a menu is closing.
         * @param menu
         * @param allMenusAreClosing
         */
        std::function<void(MenuBuilder&/*menu*/,bool/*allMenusAreClosing*/)> onCloseMenu;

        /**
         * Called when a submenu opens. Useful for notifying the application
         * of menu state so that it does not attempt to hide the action bar
         * while a submenu is open or similar.
         *
         * @param subMenu Submenu currently being opened
         * @return true if the Callback will handle presenting the submenu, false if
         *         the presenter should attempt to do so.
         */
        std::function<bool(MenuBuilder&/*subMenu*/)> onOpenSubMenu;
    };

    /**
     * Initializes this presenter for the given context and menu.
     * <p>
     * This method is called by MenuBuilder when a presenter is added. See
     * {@link MenuBuilder#addMenuPresenter(MenuPresenter)}.
     *
     * @param context the context for this presenter; used for view creation
     *                and resource management, must be non-{@code null}
     * @param menu the menu to host, or {@code null} to clear the hosted menu
     */
    virtual void initForMenu(Context* context, MenuBuilder* menu)=0;

    /**
     * Retrieve a MenuView to display the menu specified in
     * {@link #initForMenu(Context, MenuBuilder)}.
     *
     * @param root Intended parent of the MenuView.
     * @return A freshly created MenuView.
     */
    virtual ViewGroup* getMenuView(ViewGroup* root)=0;

    /**
     * Update the menu UI in response to a change. Called by
     * MenuBuilder during the normal course of operation.
     *
     * @param cleared true if the menu was entirely cleared
     */
    virtual void updateMenuView(bool cleared)=0;

    /**
     * Set a callback object that will be notified of menu events
     * related to this specific presentation.
     * @param cb Callback that will be notified of future events
     */
    virtual void setCallback(const Callback& cb)=0;

    /**
     * Called by Menu implementations to indicate that a submenu item
     * has been selected. An active Callback should be notified, and
     * if applicable the presenter should present the submenu.
     *
     * @param subMenu SubMenu being opened
     * @return true if the the event was handled, false otherwise.
     */
    virtual bool onSubMenuSelected(SubMenuBuilder* subMenu)=0;

    /**
     * Called by Menu implementations to indicate that a menu or submenu is
     * closing. Presenter implementations should close the representation
     * of the menu indicated as necessary and notify a registered callback.
     *
     * @param menu the menu or submenu that is closing
     * @param allMenusAreClosing {@code true} if all displayed menus and
     *                           submenus are closing, {@code false} if only
     *                           the specified menu is closing
     */
    virtual void onCloseMenu(MenuBuilder* menu, bool allMenusAreClosing)=0;

    /**
     * Called by Menu implementations to flag items that will be shown as actions.
     * @return true if this presenter changed the action status of any items.
     */
    virtual bool flagActionItems()=0;

    /**
     * Called when a menu item with a collapsable action view should expand its action view.
     *
     * @param menu Menu containing the item to be expanded
     * @param item Item to be expanded
     * @return true if this presenter expanded the action view, false otherwise.
     */
    virtual bool expandItemActionView(MenuBuilder& menu, MenuItemImpl& item)=0;

    /**
     * Called when a menu item with a collapsable action view should collapse its action view.
     *
     * @param menu Menu containing the item to be collapsed
     * @param item Item to be collapsed
     * @return true if this presenter collapsed the action view, false otherwise.
     */
    virtual bool collapseItemActionView(MenuBuilder& menu, MenuItemImpl& item)=0;

    /**
     * Returns an ID for determining how to save/restore instance state.
     * @return a valid ID value.
     */
    virtual int getId()const =0;

    /**
     * Returns a Parcelable describing the current state of the presenter.
     * It will be passed to the {@link #onRestoreInstanceState(Parcelable)}
     * method of the presenter sharing the same ID later.
     * @return The saved instance state
     */
    virtual Parcelable* onSaveInstanceState()=0;

    /**
     * Supplies the previously saved instance state to be restored.
     * @param state The previously saved instance state
     */
    virtual void onRestoreInstanceState(Parcelable& state)=0;
};
}/*endof namespace*/
#endif/*__MENU_PRESENTER_H__*/
