#include <actionbarmenu.h>

#include <R.h>

#include <core/context.h>
#include <core/intent.h>
#include <widget/cdwindow.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace actionbarmenu {

std::vector<MenuItemController*> MenuItemControllerFactory::buildMenuItemControllers(
        Context* /*activity*/) {
    // No MenuItemProviders are registered upstream on this build either.
    return std::vector<MenuItemController*>();
}

OptionsMenuManager& OptionsMenuManager::addMenuItemController(
        const std::vector<MenuItemController*>& controllers) {
    for (MenuItemController* controller : controllers) {
        mControllers.push_back(controller);
    }
    return *this;
}

void OptionsMenuManager::onCreateOptionsMenu(Menu& menu) {
    for (MenuItemController* controller : mControllers) {
        controller->onCreateOptionsItem(menu);
    }
}

void OptionsMenuManager::onPrepareOptionsMenu(Menu& menu) {
    for (MenuItemController* controller : mControllers) {
        if (MenuItem* menuItem = menu.findItem(controller->getId())) {
            controller->onPrepareOptionsItem(*menuItem);
        }
    }
}

bool OptionsMenuManager::onOptionsItemSelected(MenuItem& item) {
    const int itemId = item.getItemId();
    for (MenuItemController* controller : mControllers) {
        if (controller->getId() == itemId && controller->onOptionsItemSelected(item)) {
            return true;
        }
    }
    return false;
}

//
// NightModeMenuItemController
//

int NightModeMenuItemController::getId() const {
    return R::id::menu_item_night_mode;
}

void NightModeMenuItemController::onCreateOptionsItem(Menu& menu) {
    menu.add(Menu::NONE, getId(), Menu::NONE,
             mContext.getString(R::string::menu_item_night_mode))
            ->setShowAsAction(MenuItem::SHOW_AS_ACTION_NEVER);
}

bool NightModeMenuItemController::onOptionsItemSelected(MenuItem& /*item*/) {
    Intent intent;
    intent.setClassName("cdroid.deskclock", "ScreensaverActivity")
          .setAction(Intent::ACTION_MAIN)
          .setFlags(Intent::FLAG_ACTIVITY_NEW_TASK);
    mContext.startActivity(intent);
    return true;
}

//
// SettingsMenuItemController
//

int SettingsMenuItemController::getId() const {
    return R::id::menu_item_settings;
}

void SettingsMenuItemController::onCreateOptionsItem(Menu& menu) {
    menu.add(Menu::NONE, getId(), Menu::NONE,
             mActivity->getContext()->getString(R::string::menu_item_settings))
            ->setShowAsAction(MenuItem::SHOW_AS_ACTION_NEVER);
}

bool SettingsMenuItemController::onOptionsItemSelected(MenuItem& /*item*/) {
    Intent settingIntent;
    settingIntent.setClassName("cdroid.deskclock", "SettingsActivity")
                 .setAction(Intent::ACTION_MAIN);
    mActivity->startActivityForResult(settingIntent, REQUEST_CHANGE_SETTINGS);
    return true;
}

} // namespace actionbarmenu
} // namespace deskclock
} // namespace cdroid
