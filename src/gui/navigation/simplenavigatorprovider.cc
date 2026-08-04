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
    Navigator* /*<? extends NavDestination>*/navigator = mNavigators.find(name)->second;
    FATAL_IF(navigator == nullptr,"Could not find Navigator with name "
        "You must call NavController.addNavigator() for each navigation type.",name.c_str());
    return  navigator;
}

Navigator*SimpleNavigatorProvider::addNavigator(Navigator*navigator) {
    return addNavigator(navigator->getName(), navigator);
}

Navigator*SimpleNavigatorProvider::addNavigator(const std::string& name,Navigator*navigator) {
    FATAL_IF(!validateName(name),"navigator name cannot be an empty string");
    mNavigators.insert({name, navigator});
    return navigator;
}

const std::map<const std::string, Navigator*>& SimpleNavigatorProvider::getNavigators() {
    return mNavigators;
}

bool SimpleNavigatorProvider::validateName(const std::string& name) {
    return !name.empty();
}

}/*endof namespace*/

