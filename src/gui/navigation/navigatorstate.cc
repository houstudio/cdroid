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
#include <navigation/navigatorstate.h>
#include <navigation/navcontroller.h>
#include <navigation/navdestination.h>
#include <algorithm>

namespace cdroid{

NavigatorState::NavigatorState(NavController* controller, Navigator* navigator)
    : mController(controller), mNavigator(navigator){
}

void NavigatorState::push(NavBackStackEntry* entry){
    // androidx NavControllerNavigatorState.push -> impl.push(this, entry): route through the
    // NavController so its installed push handler mutates the merged back queue, then this state's
    // own back stack is appended (addInternal) on the controller side.
    if(mController) mController->push(this, entry);
}

void NavigatorState::pop(NavBackStackEntry* popUpTo, bool saveState){
    // androidx NavControllerNavigatorState.pop -> impl.pop(this, popUpTo, saveState){ super.pop }:
    // route through the NavController's pop handler, then this state's own back stack is trimmed
    // (popInternal) on the controller side.
    if(mController) mController->pop(this, popUpTo, saveState);
}

NavBackStackEntry* NavigatorState::createBackStackEntry(NavDestination* destination, Bundle* arguments){
    return new NavBackStackEntry(destination, arguments);
}

void NavigatorState::addInternal(NavBackStackEntry* entry){
    // androidx addInternal = super.push: append to this navigator's own back stack only.
    if(entry) mBackStack.push_back(entry);
}

void NavigatorState::popInternal(NavBackStackEntry* popUpTo){
    // androidx super.pop(popUpTo, saveState): drop popUpTo and everything above it from this
    // navigator's own back stack.
    if(popUpTo == nullptr){ mBackStack.clear(); return; }
    auto it = std::find(mBackStack.begin(), mBackStack.end(), popUpTo);
    if(it != mBackStack.end()) mBackStack.erase(it, mBackStack.end());
}

}//namespace cdroid
