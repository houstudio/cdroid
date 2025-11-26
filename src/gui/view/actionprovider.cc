#include <view/actionprovider.h>
#include <porting/cdlog.h>
namespace cdroid{

ActionProvider::ActionProvider(Context* context) {
}

/**
 * The result of this method determines whether or not {@link #isVisible()} will be used
 * by the {@link MenuItem} this ActionProvider is bound to help determine its visibility.
 *
 * @return true if this ActionProvider overrides the visibility of the MenuItem
 *         it is bound to, false otherwise. The default implementation returns false.
 * @see #isVisible()
 */
bool ActionProvider::overridesItemVisibility() {
    return false;
}

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
bool ActionProvider::isVisible() {
    return true;
}

/**
 * If this ActionProvider is associated with an item in a menu,
 * refresh the visibility of the item based on {@link #overridesItemVisibility()} and
 * {@link #isVisible()}. If {@link #overridesItemVisibility()} returns false, this call
 * will have no effect.
 */
void ActionProvider::refreshVisibility() {
    if (mVisibilityListener != nullptr && overridesItemVisibility()) {
        mVisibilityListener(isVisible());//onActionProviderVisibilityChanged(bool)
    }
}

bool ActionProvider::onPerformDefaultAction() {
    return false;
}

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
bool ActionProvider::hasSubMenu() {
    return false;
}

/**
 * Called to prepare an associated submenu for the menu item backed by this ActionProvider.
 *
 * <p>if {@link #hasSubMenu()} returns true, this method will be called when the
 * menu item is selected to prepare the submenu for presentation to the user. Apps
 * may use this to create or alter submenu content right before display.
 *
 * @param subMenu Submenu that will be displayed
 */
void ActionProvider::onPrepareSubMenu(SubMenu& subMenu) {
}

/**
 * Notify the system that the visibility of an action view's sub-UI such as
 * an anchored popup has changed. This will affect how other system
 * visibility notifications occur.
 *
 * @hide Pending future API approval
 */
void ActionProvider::subUiVisibilityChanged(bool isVisible) {
    if (mSubUiVisibilityListener != nullptr) {
        mSubUiVisibilityListener(isVisible);//.onSubUiVisibilityChanged(isVisible);
    }
}

void ActionProvider::setSubUiVisibilityListener(const SubUiVisibilityListener& listener) {
    mSubUiVisibilityListener = listener;
}

/**
 * Set a listener to be notified when this ActionProvider's overridden visibility changes.
 * This should only be used by MenuItem implementations.
 *
 * @param listener listener to set
 */
void ActionProvider::setVisibilityListener(const VisibilityListener& listener) {
    if (mVisibilityListener != nullptr) {
        LOGW("setVisibilityListener: Setting a new ActionProvider.VisibilityListener "
                "when one is already set. Are you reusing this "
                " instance while it is still in use somewhere else?");
    }
    mVisibilityListener = listener;
}

void ActionProvider::reset() {
    mVisibilityListener = nullptr;
    mSubUiVisibilityListener = nullptr;
}
}/*endof namespace*/

