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
 * Port of androidx.navigation.NavigatorState. One instance per Navigator, owned by
 * the NavController (androidx NavControllerNavigatorState). Holds THIS navigator's back
 * stack of NavBackStackEntry and routes push/pop back to the NavController via transient
 * handler slots. This decouples the NavController's merged back queue from each Navigator's
 * own element stack — which is exactly what lets FragmentNavigator skip addToBackStack on the
 * initial navigation: the initial entry is removed from the logical stack through the same
 * handler path as any other, while its fragment lingers until the next navigate()'s replace().
 *********************************************************************************/
#include <vector>
#include <string>
#include <navigation/navbackstackentry.h>
namespace cdroid{
class NavDestination;
class NavController;
class Navigator;
class Bundle;

class NavigatorState{
public:
    // androidx NavControllerNavigatorState is an inner class of NavController with implicit access
    // to the controller; C++ has no inner classes, so the controller is passed explicitly.
    NavigatorState(NavController* controller, Navigator* navigator);
    virtual ~NavigatorState() = default;
    // androidx NavigatorState.push — overridden by NavControllerNavigatorState to route through the
    // controller's push handler (which mutates the merged back queue) then append here.
    virtual void push(NavBackStackEntry* entry);
    // androidx NavigatorState.pop — overridden to route through the controller's pop handler then
    // trim this state's own back stack.
    virtual void pop(NavBackStackEntry* popUpTo, bool saveState);
    // androidx NavigatorState.createBackStackEntry.
    virtual NavBackStackEntry* createBackStackEntry(NavDestination* destination, Bundle* arguments);

    // androidx NavigatorState.backStack (exposed as a getter — StateFlow is replaced by a vector).
    const std::vector<NavBackStackEntry*>& getBackStack() const { return mBackStack; }
    // androidx NavControllerNavigatorState.navigator.
    Navigator* getNavigator() const { return mNavigator; }

private:
    friend class NavController;
    // androidx NavControllerNavigatorState.addInternal (= super.push) and the super.pop equivalent:
    // mutate ONLY this state's own back stack, called by the controller after its handler has
    // updated the merged back queue. Kept private so the public surface matches androidx NavigatorState.
    void addInternal(NavBackStackEntry* entry);
    void popInternal(NavBackStackEntry* popUpTo);
protected:
    NavController* mController;
    Navigator* mNavigator;
    std::vector<NavBackStackEntry*> mBackStack;
};

}//namespace cdroid
#endif
