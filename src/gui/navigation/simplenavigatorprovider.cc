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
#include <navigation/simplenavigatorprovider.h>

namespace cdroid{

Navigator*SimpleNavigatorProvider::getNavigator(const std::string& name) {
    LOGE_IF(!validateName(name),"navigator name cannot be an empty string");
    auto it = mNavigators.find(name);
    FATAL_IF(it == mNavigators.end(),"Could not find Navigator with name %s "
        "You must call NavController.addNavigator() for each navigation type.",name.c_str());
    return  it->second;
}

Navigator*SimpleNavigatorProvider::addNavigator(Navigator*navigator) {
    return addNavigator(navigator->getName(), navigator);
}

Navigator*SimpleNavigatorProvider::addNavigator(const std::string& name,Navigator*navigator) {
    FATAL_IF(!validateName(name),"navigator name cannot be an empty string");
    // androidx uses HashMap.put — a later addNavigator with the same name replaces
    // the previous navigator (std::map::insert would keep the old one forever).
    auto it = mNavigators.find(name);
    if(it != mNavigators.end()){
        if(it->second == navigator) return navigator; // re-adding same instance: no-op
        delete it->second; // provider owns every navigator added
        it->second = navigator;
    }else{
        mNavigators[name] = navigator;
    }
    return navigator;
}

const std::map<const std::string, Navigator*>& SimpleNavigatorProvider::getNavigators() {
    return mNavigators;
}

bool SimpleNavigatorProvider::validateName(const std::string& name) {
    return !name.empty();
}

}/*endof namespace*/

