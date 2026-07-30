#include <navigation/navigationui.h>
#include <navigation/appbarconfiguration.h>
#include <navigation/navdestination.h>
#include <porting/cdlog.h>

namespace cdroid{

bool NavigationUI::navigateUp(NavController* navController, AppBarConfiguration* configuration){
    if(!navController) return false;
    // At a top-level destination with a drawer configured, Up opens the drawer instead of popping.
    if(configuration && configuration->getDrawerLayout()){
        NavDestination* current = navController->getCurrentDestination();
        if(current && configuration->isTopLevelDestination(current->getRoute())){
            // TODO: configuration->getDrawerLayout()->openDrawer() once DrawerLayout open API is wired.
            LOGD("NavigationUI.navigateUp: top-level destination, would open drawer");
            return true;
        }
    }
    return navController->navigateUp();
}

void NavigationUI::setupActionBarWithNavController(ActionBar* /*actionBar*/,
                                                   NavController* /*navController*/,
                                                   AppBarConfiguration* /*configuration*/){
    // CDROID ActionBar is a shell (no setTitle/setDisplayHomeAsUpEnabled yet). The
    // destination-changed listener that updates title/Up-arrow is implemented once
    // ActionBar gains those APIs.
    LOGD("NavigationUI.setupActionBarWithNavController: ActionBar shell, no-op for now");
}

void NavigationUI::setupWithNavController(Toolbar* /*toolbar*/, NavController* /*navController*/,
                                          AppBarConfiguration* /*configuration*/){
    // TODO: wire Toolbar title + navigation icon via OnDestinationChangedListener once
    // Toolbar exposes setTitle/setNavigationIcon/setNavigationOnClickListener.
    LOGD("NavigationUI.setupWithNavController: Toolbar setters incomplete, no-op for now");
}

bool NavigationUI::onNavDestinationSelected(MenuItem* /*item*/, NavController* /*navController*/){
    // TODO: resolve the menu item's itemId to a destination and navigate.
    return false;
}

}//namespace cdroid
