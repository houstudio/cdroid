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
 * to a NavController. setupWithNavController(Toolbar) updates the title and the Up
 * indicator per destination (Up arrow on sub-pages, none on the start/top-level
 * destination) and routes the navigation click to navigateUp; setupActionBarWithNavController
 * drives an ActionBar's title + DISPLAY_HOME_AS_UP the same way. navigateUp works fully.
 *********************************************************************************/
#include <navigation/navcontroller.h>
namespace cdroid{
class Context;
class Drawable;
class ActionBar;
class Toolbar;
class AppBarConfiguration;
class MenuItem;
class Openable;

/** androidx AbstractAppBarOnDestinationChangedListener: title + Up affordance
 *  on destination change. CDROID's NavController listener face is a CallbackBase
 *  VALUE (functor), so attach(NavController*) registers a forwarding callback;
 *  the subclass object itself is owned by the caller (typically stack/成员). */
class AbstractAppBarOnDestinationChangedListener{
protected:
    Context* mContext;
    AppBarConfiguration* mConfiguration;
public:
    AbstractAppBarOnDestinationChangedListener(Context* context, AppBarConfiguration* configuration);
    virtual ~AbstractAppBarOnDestinationChangedListener() = default;
    void attach(NavController* controller);
    void detach(NavController* controller);
    virtual void onDestinationChanged(NavController* controller, NavDestination* destination, Bundle* arguments);
protected:
    virtual void setTitle(const std::string& title) = 0;
    virtual void setNavigationIcon(Drawable* icon) = 0;
};

/** androidx ToolbarOnDestinationChangedListener. */
class ToolbarOnDestinationChangedListener : public AbstractAppBarOnDestinationChangedListener{
    Toolbar* mToolbar;
public:
    ToolbarOnDestinationChangedListener(Toolbar* toolbar, AppBarConfiguration* configuration);
    void onDestinationChanged(NavController* controller, NavDestination* destination, Bundle* arguments) override;
protected:
    void setTitle(const std::string& title) override;
    void setNavigationIcon(Drawable* icon) override;
};

/** androidx ActionBarOnDestinationChangedListener. */
class ActionBarOnDestinationChangedListener : public AbstractAppBarOnDestinationChangedListener{
    ActionBar* mActionBar;
public:
    ActionBarOnDestinationChangedListener(Context* context, ActionBar* actionBar, AppBarConfiguration* configuration);
    void onDestinationChanged(NavController* controller, NavDestination* destination, Bundle* arguments) override;
protected:
    void setTitle(const std::string& title) override;
    void setNavigationIcon(Drawable* icon) override;
};

class NavigationView;
class NavigationBarView;

class NavigationUI{
public:
    static void setupActionBarWithNavController(ActionBar* actionBar, NavController* navController,
                                                AppBarConfiguration* configuration = nullptr);
    static void setupWithNavController(Toolbar* toolbar, NavController* navController,
                                       AppBarConfiguration* configuration = nullptr);
    static void setupWithNavController(NavigationView* navigationView, NavController* navController);
    static void setupWithNavController(NavigationBarView* navigationBarView, NavController* navController);
    // androidx navigateUp(NavController, Openable): opens the Openable on a
    // top-level destination instead of popping.
    static bool navigateUp(NavController* navController, Openable* openableLayout);
    static bool navigateUp(NavController* navController, AppBarConfiguration* configuration);
    static bool onNavDestinationSelected(MenuItem* item, NavController* navController);
};

}//namespace cdroid
#endif
