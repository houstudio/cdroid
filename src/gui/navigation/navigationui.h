#ifndef __NAVIGATIONUI_H__
#define __NAVIGATIONUI_H__
/*********************************************************************************
 * Port of androidx.navigation.ui.NavigationUI. Wires AppBar/Toolbar title + Up button
 * to a NavController. NOTE: CDROID's ActionBar is currently a shell and Toolbar lacks
 * some setters, so setupActionBarWithNavController/setupWithNavController register the
 * destination-changed logic but the ActionBar/Toolbar mutations are no-ops (TODO until
 * those widgets are fleshed out). navigateUp works fully.
 *********************************************************************************/
#include <navigation/navcontroller.h>
namespace cdroid{
class ActionBar;
class Toolbar;
class AppBarConfiguration;
class MenuItem;

class NavigationUI{
public:
    static void setupActionBarWithNavController(ActionBar* actionBar, NavController* navController,
                                                AppBarConfiguration* configuration = nullptr);
    static void setupWithNavController(Toolbar* toolbar, NavController* navController,
                                       AppBarConfiguration* configuration = nullptr);
    static bool navigateUp(NavController* navController, AppBarConfiguration* configuration);
    static bool onNavDestinationSelected(MenuItem* item, NavController* navController);
};

}//namespace cdroid
#endif
