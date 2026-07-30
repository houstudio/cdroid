#include <navigation/navigationui.h>
#include <navigation/appbarconfiguration.h>
#include <navigation/navdestination.h>
#include <widget/toolbar.h>
#include <view/view.h>
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

void NavigationUI::setupWithNavController(Toolbar* toolbar, NavController* navController,
                                          AppBarConfiguration* configuration){
    if(!toolbar || !navController) return;
    // OnDestinationChangedListener: update the Toolbar title + wire the navigation icon to
    // navigateUp() unless the destination is a top-level one.
    class ToolbarListener : public NavController::OnDestinationChangedListener{
        Toolbar* mToolbar;
        NavController* mController;
        AppBarConfiguration* mConfig;
    public:
        ToolbarListener(Toolbar* t, NavController* c, AppBarConfiguration* cfg)
            : mToolbar(t), mController(c), mConfig(cfg){}
        void onDestinationChanged(NavController*, NavDestination* destination, Bundle*) override{
            if(!destination) return;
            mToolbar->setTitle(destination->getLabel());
            bool isTopLevel = mConfig && mConfig->isTopLevelDestination(destination->getRoute());
            if(!isTopLevel){
                mToolbar->setNavigationOnClickListener([this](View&){ mController->navigateUp(); });
            }
        }
    };
    navController->addOnDestinationChangedListener(new ToolbarListener(toolbar, navController, configuration));
}

bool NavigationUI::onNavDestinationSelected(MenuItem* /*item*/, NavController* /*navController*/){
    // TODO: resolve the menu item's itemId to a destination and navigate.
    return false;
}

}//namespace cdroid
