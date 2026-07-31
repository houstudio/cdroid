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
#include <algorithm>

namespace cdroid{

void NavigatorState::push(NavBackStackEntry* entry){
    if(entry) mBackStack.push_back(entry);
}

void NavigatorState::pop(NavBackStackEntry* popUpTo, bool /*saveState*/){
    if(mBackStack.empty()) return;
    if(popUpTo == nullptr){
        mBackStack.clear();
        return;
    }
    // Pop entries above popUpTo (inclusive semantics handled by caller via flag elsewhere;
    // here we pop everything strictly above popUpTo, keeping popUpTo).
    auto it = std::find(mBackStack.begin(), mBackStack.end(), popUpTo);
    if(it != mBackStack.end()){
        mBackStack.erase(it + 1, mBackStack.end());
    }
}

}//namespace cdroid
