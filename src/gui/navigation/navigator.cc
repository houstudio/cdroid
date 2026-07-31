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
#include <navigation/navigator.h>
#include <navigation/navigatorstate.h>
#include <navigation/navbackstackentry.h>

namespace cdroid{

void Navigator::onAttach(NavigatorState* state){
    mState = state;
}

void Navigator::navigate(std::vector<NavBackStackEntry*>& entries, NavOptions* navOptions, Extras* extras){
    // Default modern navigate: fall back to the legacy per-destination navigate and push
    // each entry onto this navigator's state.
    for(NavBackStackEntry* entry : entries){
        if(!entry) continue;
        navigate(entry->getDestination(), entry->getArguments(), navOptions);
        if(mState) mState->push(entry);
    }
    (void)extras;
}

void Navigator::popBackStack(NavBackStackEntry* popUpTo, bool savedState){
    if(mState) mState->pop(popUpTo, savedState);
}

}//namespace cdroid
