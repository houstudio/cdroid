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
#include <navigation/navaction.h>
#include <navigation/navoptions.h>
namespace cdroid{
NavAction::NavAction(int destinationId)
    :NavAction(destinationId, nullptr){
}

NavAction::NavAction(int destinationId, NavOptions* navOptions) {
    mDestinationId = destinationId;
    mNavOptions = navOptions;
}

NavAction::~NavAction(){
    delete mNavOptions;
}

int NavAction::getDestinationId() const{
    return mDestinationId;
}

void NavAction::setNavOptions(NavOptions* navOptions) {
    delete mNavOptions;
    mNavOptions = navOptions;
}

NavOptions* NavAction::getNavOptions() const{
    return mNavOptions;
}

}/*endof namespace*/
