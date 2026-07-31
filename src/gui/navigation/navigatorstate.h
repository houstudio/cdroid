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
#ifndef __NAVIGATORSTATE_H__
#define __NAVIGATORSTATE_H__
/*********************************************************************************
 * Port of androidx.navigation.NavigatorState. The bridge between a Navigator and
 * its NavController: holds this navigator's back stack of NavBackStackEntry and
 * provides push/pop/createBackStackEntry. (StateFlow is replaced by a plain vector;
 * transitionsInProgress/special-effects are omitted for MVP.)
 *********************************************************************************/
#include <vector>
#include <string>
#include <navigation/navbackstackentry.h>
namespace cdroid{
class NavDestination;

class NavigatorState{
public:
    virtual ~NavigatorState() = default;
    // Push a new entry onto this navigator's back stack.
    virtual void push(NavBackStackEntry* entry);
    // Pop everything above (and optionally) popUpTo from the back stack.
    virtual void pop(NavBackStackEntry* popUpTo, bool saveState);
    // Build a new NavBackStackEntry for a destination + args (NavController implements).
    virtual NavBackStackEntry* createBackStackEntry(NavDestination* destination, Bundle* arguments) = 0;

    const std::vector<NavBackStackEntry*>& getBackStack() const { return mBackStack; }
protected:
    std::vector<NavBackStackEntry*> mBackStack;
};

}//namespace cdroid
#endif
