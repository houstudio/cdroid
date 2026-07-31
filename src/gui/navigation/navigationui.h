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
