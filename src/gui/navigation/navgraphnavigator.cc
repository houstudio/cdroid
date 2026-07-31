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
#include <navigation/navgraph.h>
#include <navigation/navdestination.h>
#include <navigation/navgraphnavigator.h>

namespace cdroid{

/**
    @Override
 * Construct a Navigator capable of routing incoming navigation requests to the proper
 * destination within a {@link NavGraph}.
 * @param context
 */
NavGraphNavigator::NavGraphNavigator(Context* context):Navigator() {
    mContext = context;
    mName = "navigation";
}

/**
 * Creates a new {@link NavGraph} associated with this navigator.
 * @return
 */
NavGraph* NavGraphNavigator::createDestination() {
    return new NavGraph(this);
}

void NavGraphNavigator::navigate(/*@NonNull NavGraph*/NavDestination* destination, /*@Nullable*/Bundle* args,
        /*@Nullable*/ NavOptions* navOptions) {
    const int startId = ((NavGraph*)destination)->getStartDestination();
    if (startId == 0) {
        /*throw new IllegalStateException("no start destination defined via"
                + " app:startDestination for "
                + (destination.getId() != 0
                        ? NavDestination.getDisplayName(mContext, destination.getId())
                        : "the root navigation"));*/
    }
    NavDestination* startDestination = ((NavGraph*)destination)->findNode(startId, false);
    if (startDestination == nullptr) {
        const std::string dest = NavDestination::getDisplayName(mContext, startId);
        /*throw std::logic_error("navigation destination %s is not a direct child of this NavGraph",dest.c_str());*/
    }
    startDestination->navigate(args, navOptions);
}

bool NavGraphNavigator::popBackStack() {
    return false;
}
}/*endof namespace*/
