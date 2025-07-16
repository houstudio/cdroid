#ifndef __ACTION_PROVIDER_H__
#define __ACTION_PROVIDER_H__
namespace cdroid{
class Context;
class MenuItem;
class ActionProvider {
public:
    /**
     * @hide Internal use only
     */
    using SubUiVisibilityListener=std::function<void(bool)>;

    /**
     * Listens to changes in visibility as reported by {@link ActionProvider#refreshVisibility()}.
     *
     * @see ActionProvider#overridesItemVisibility()
     * @see ActionProvider#isVisible()
     */
    using VisibilityListener =std::function<void(bool isVisible)>;
private:
    SubUiVisibilityListener mSubUiVisibilityListener;
    VisibilityListener mVisibilityListener;
public:
    /**
     * Creates a new instance. ActionProvider classes should always implement a
     * constructor that takes a single Context parameter for inflating from menu XML.
     *
     * @param context Context for accessing resources.
     */
    ActionProvider(Context* context);

    /**
     * Factory method called by the Android framework to create new action views.
     * This method returns a new action view for the given MenuItem.
     *
     * <p>If your ActionProvider implementation overrides the deprecated no-argument overload
     * {@link #onCreateActionView()}, overriding this method for devices running API 16 or later
     * is recommended but optional. The default implementation calls {@link #onCreateActionView()}
     * for compatibility with applications written for older platform versions.</p>
     *
     * @param forItem MenuItem to create the action view for
     * @return the new action view
     */
    virtual View* onCreateActionView(MenuItem& forItem)=0;

    /**
     * The result of this method determines whether or not {@link #isVisible()} will be used
     * by the {@link MenuItem} this ActionProvider is bound to help determine its visibility.
     *
     * @return true if this ActionProvider overrides the visibility of the MenuItem
     *         it is bound to, false otherwise. The default implementation returns false.
     * @see #isVisible()
     */
    virtual bool overridesItemVisibility();

    /**
     * If {@link #overridesItemVisibility()} returns true, the return value of this method
     * will help determine the visibility of the {@link MenuItem} this ActionProvider is bound to.
     *
     * <p>If the MenuItem's visibility is explicitly set to false by the application,
     * the MenuItem will not be shown, even if this method returns true.</p>
     *
     * @return true if the MenuItem this ActionProvider is bound to is visible, false if
     *         it is invisible. The default implementation returns true.
     */
    virtual bool isVisible();

    /**
     * If this ActionProvider is associated with an item in a menu,
     * refresh the visibility of the item based on {@link #overridesItemVisibility()} and
     * {@link #isVisible()}. If {@link #overridesItemVisibility()} returns false, this call
     * will have no effect.
     */
    void refreshVisibility();

    /**
     * Performs an optional default action.
     * <p>
     * For the case of an action provider placed in a menu item not shown as an action this
     * method is invoked if previous callbacks for processing menu selection has handled
     * the event.
     * </p>
     * <p>
     * A menu item selection is processed in the following order:
     * <ul>
     * <li>
     * Receiving a call to {@link MenuItem.OnMenuItemClickListener#onMenuItemClick
     *  MenuItem.OnMenuItemClickListener.onMenuItemClick}.
     * </li>
     * <li>
     * Receiving a call to {@link android.app.Activity#onOptionsItemSelected(MenuItem)
     *  Activity.onOptionsItemSelected(MenuItem)}
     * </li>
     * <li>
     * Receiving a call to {@link android.app.Fragment#onOptionsItemSelected(MenuItem)
     *  Fragment.onOptionsItemSelected(MenuItem)}
     * </li>
     * <li>
     * Launching the {@link android.content.Intent} set via
     * {@link MenuItem#setIntent(android.content.Intent) MenuItem.setIntent(android.content.Intent)}
     * </li>
     * <li>
     * Invoking this method.
     * </li>
     * </ul>
     * </p>
     * <p>
     * The default implementation does not perform any action and returns false.
     * </p>
     */
    virtual bool onPerformDefaultAction();

    /**
     * Determines if this ActionProvider has a submenu associated with it.
     *
     * <p>Associated submenus will be shown when an action view is not. This
     * provider instance will receive a call to {@link #onPrepareSubMenu(SubMenu)}
     * after the call to {@link #onPerformDefaultAction()} and before a submenu is
     * displayed to the user.
     *
     * @return true if the item backed by this provider should have an associated submenu
     */
    virtual bool hasSubMenu();

    /**
     * Called to prepare an associated submenu for the menu item backed by this ActionProvider.
     *
     * <p>if {@link #hasSubMenu()} returns true, this method will be called when the
     * menu item is selected to prepare the submenu for presentation to the user. Apps
     * may use this to create or alter submenu content right before display.
     *
     * @param subMenu Submenu that will be displayed
     */
    virtual void onPrepareSubMenu(SubMenu& subMenu);

    /**
     * Notify the system that the visibility of an action view's sub-UI such as
     * an anchored popup has changed. This will affect how other system
     * visibility notifications occur.
     *
     * @hide Pending future API approval
     */
    void subUiVisibilityChanged(bool isVisible);

    void setSubUiVisibilityListener(const SubUiVisibilityListener& listener);

    /**
     * Set a listener to be notified when this ActionProvider's overridden visibility changes.
     * This should only be used by MenuItem implementations.
     *
     * @param listener listener to set
     */
    void setVisibilityListener(const VisibilityListener& listener);
    void reset();
};
}/*endof namespace*/
#endif/*__ACTION_PROVIDER_H__*/

