#ifndef __DESKCLOCK_ACTIONBARMENU_H__
#define __DESKCLOCK_ACTIONBARMENU_H__
/*********************************************************************************
 * Port of com.android.deskclock.actionbarmenu — MenuItemController,
 * OptionsMenuManager, NightMode/Settings controllers and the (empty) factory.
 *********************************************************************************/
#include <vector>

#include <menu/menu.h>
#include <menu/menuitem.h>

namespace cdroid {

class Context;
class Window;

namespace deskclock {
namespace actionbarmenu {

/** Interface for handling a single menu item in action bar. */
class MenuItemController {
public:
    virtual ~MenuItemController() = default;

    /** Returns the menu item resource id that the controller manages. */
    virtual int getId() const = 0;

    /** Create the menu item. */
    virtual void onCreateOptionsItem(Menu& menu) = 0;

    /** Called immediately before the MenuItem is shown. */
    virtual void onPrepareOptionsItem(MenuItem& item) = 0;

    /** Attempts to handle the click action; true if handled. */
    virtual bool onOptionsItemSelected(MenuItem& item) = 0;
};

/** Factory that builds optional MenuItemController instances (no providers registered). */
class MenuItemControllerFactory {
public:
    static std::vector<MenuItemController*> buildMenuItemControllers(Context* activity);
};

/** Coordinates handling of context menu items. */
class OptionsMenuManager {
private:
    std::vector<MenuItemController*> mControllers;

public:
    OptionsMenuManager& addMenuItemController(const std::vector<MenuItemController*>& controllers);

    void onCreateOptionsMenu(Menu& menu);
    void onPrepareOptionsMenu(Menu& menu);
    bool onOptionsItemSelected(MenuItem& item);
};

/** MenuItemController for controlling night mode display. */
class NightModeMenuItemController : public MenuItemController {
private:
    Context& mContext;

public:
    explicit NightModeMenuItemController(Context& context) : mContext(context) {}

    int getId() const override;
    void onCreateOptionsItem(Menu& menu) override;
    void onPrepareOptionsItem(MenuItem& item) override {}
    bool onOptionsItemSelected(MenuItem& item) override;
};

/** MenuItemController for settings menu. */
class SettingsMenuItemController : public MenuItemController {
public:
    static constexpr int REQUEST_CHANGE_SETTINGS = 1;

private:
    Window* mActivity;

public:
    explicit SettingsMenuItemController(Window* activity) : mActivity(activity) {}

    int getId() const override;
    void onCreateOptionsItem(Menu& menu) override;
    void onPrepareOptionsItem(MenuItem& item) override {}
    bool onOptionsItemSelected(MenuItem& item) override;
};

} // namespace actionbarmenu
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ACTIONBARMENU_H__
