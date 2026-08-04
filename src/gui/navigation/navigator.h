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
#ifndef __NAVIGATOR_H__
#define __NAVIGATOR_H__
/*********************************************************************************
 * Port of androidx.navigation.Navigator<D> — rewritten as an INDEPENDENT abstract
 * base class (the old `class Navigator : public NavDestination` was a structural
 * error: it modeled the navigation strategy as if it were a destination node).
 *
 * Keeps the legacy navigate(NavDestination*, Bundle*, NavOptions*) signature so
 * existing NavGraphNavigator compiles; adds the modern navigate(entries) /
 * popBackStack(entry) + NavigatorState plumbing.
 *********************************************************************************/
#include <vector>
#include <navigation/navdestination.h>
namespace cdroid{

class Bundle;
class NavOptions;
class NavBackStackEntry;
class NavigatorState;

class Navigator{
public:
    class Extras{ public: virtual ~Extras(){} };

    Navigator() = default;
    virtual ~Navigator() = default;

    // Navigator name (androidx @Navigator.Name). Subclasses set mName in their ctor.
    const std::string& getName() const { return mName; }

    // Construct a new NavDestination associated with this Navigator.
    virtual NavDestination* createDestination() = 0;

    // Legacy navigate (kept for NavGraphNavigator). Default: no-op.
    virtual void navigate(NavDestination* destination, Bundle* args, NavOptions* navOptions){ (void)destination; (void)args; (void)navOptions; }
    virtual bool popBackStack(){ return true; }

    // Modern navigate over NavBackStackEntry list. Default: delegate to legacy per entry
    // and push each onto the navigator state.
    virtual void navigate(std::vector<NavBackStackEntry*>& entries, NavOptions* navOptions, Extras* extras);
    virtual void popBackStack(NavBackStackEntry* popUpTo, bool savedState);
    virtual void onLaunchSingleTop(NavBackStackEntry* /*entry*/){}

    // Lifecycle: bound to a NavigatorState by the NavController.
    virtual void onAttach(NavigatorState* state);
    NavigatorState* getState() const { return mState; }
    bool isAttached() const { return mState != nullptr; }

    virtual Bundle onSaveState(){ return Bundle(); }
    virtual void onRestoreState(const Bundle& /*savedState*/){}

protected:
    NavigatorState* mState = nullptr;
    std::string mName;
};

}//namespace cdroid
#endif /*__NAVIGATOR_H__*/
