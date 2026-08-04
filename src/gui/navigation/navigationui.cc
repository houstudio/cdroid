/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <navigation/navigationui.h>
#include <navigation/appbarconfiguration.h>
#include <navigation/navdestination.h>
#include <navigation/navgraph.h>
#include <widget/actionbar.h>
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

void NavigationUI::setupActionBarWithNavController(ActionBar* actionBar,
                                                   NavController* navController,
                                                   AppBarConfiguration* configuration){
    if(!actionBar || !navController) return;
    // OnDestinationChangedListener: update the ActionBar title and Up affordance. The Up
    // button click flows home -> Activity.onOptionsItemSelected -> onNavigateUp; the host
    // Activity should override onNavigateUp() to call NavigationUI::navigateUp(navController).
    // The listener is a CallbackBase value owned by NavController (no new/delete); pointers are
    // captured by value, matching the prior subclass's borrowed-field lifetime.
    navController->addOnDestinationChangedListener(
        [actionBar, configuration](NavController*, NavDestination* destination, Bundle*){
            if(!destination) return;
            actionBar->setTitle(destination->getLabel());
            bool isTopLevel = configuration && configuration->isTopLevelDestination(destination->getRoute());
            actionBar->setDisplayHomeAsUpEnabled(!isTopLevel);
        });
}

void NavigationUI::setupWithNavController(Toolbar* toolbar, NavController* navController,
                                          AppBarConfiguration* configuration){
    if(!toolbar || !navController) return;
    // androidx NavigationUI.setupWithNavController(Toolbar, NavController, AppBarConfiguration)
    // attaches a ToolbarOnDestinationChangedListener (title + Up indicator, via
    // AbstractAppBarOnDestinationChangedListener) and wires the navigation click UNCONDITIONALLY
    // to navigateUp(navController, configuration). With no Openable/drawer configured (CDROID
    // wires no drawer here) the icon logic clears the nav icon on top-level destinations and
    // shows the Up indicator otherwise — androidx uses a DrawerArrowDrawable at progress 1; CDROID
    // has no DrawerArrowDrawable, so the built-in homeAsUpIndicator asset stands in.
    // The 2-arg overload passes no AppBarConfiguration; androidx then builds a default whose sole
    // top-level destination is the graph's start destination. Mirror that so the start screen
    // shows no Up arrow (and the existing 2-arg callers don't regress to an arrow on home).
    const bool useDefaultConfig = (configuration == nullptr);
    const std::string startRoute = (useDefaultConfig && navController->getGraph())
        ? navController->getGraph()->getStartDestinationRoute() : std::string();
    navController->addOnDestinationChangedListener(
        [toolbar, configuration, startRoute](NavController*, NavDestination* destination, Bundle*){
            if(!destination) return;
            toolbar->setTitle(destination->getLabel());
            const std::string& route = destination->getRoute();
            const bool isTopLevel = (configuration && configuration->isTopLevelDestination(route))
                || (!configuration && !startRoute.empty() && route == startRoute);
            if(isTopLevel){
                toolbar->setNavigationIcon(nullptr);
            }else{
                toolbar->setNavigationIcon(
                    toolbar->getContext()->getDrawable("cdroid:drawable/ic_ab_back_holo_dark"));
            }
        });
    // Wired once, unconditionally — navigateUp itself decides drawer-vs-pop from the configuration.
    toolbar->setNavigationOnClickListener([navController, configuration](View&){
        navigateUp(navController, configuration);
    });
}

bool NavigationUI::onNavDestinationSelected(MenuItem* /*item*/, NavController* /*navController*/){
    // TODO: resolve the menu item's itemId to a destination and navigate.
    return false;
}

}//namespace cdroid
