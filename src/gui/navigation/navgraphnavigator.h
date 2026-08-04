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
#ifndef __NAVGRAPH_NAVIGATOR_H__
#define __NAVGRAPH_NAVIGATOR_H__
#include <navigation/navigator.h>
#include <navigation/navgraph.h>

namespace cdroid{
class NavGraphNavigator :public Navigator{
private:
    Context* mContext;

    /**
     * Construct a Navigator capable of routing incoming navigation requests to the proper
     * destination within a {@link NavGraph}.
     * @param context
     */
public:
    NavGraphNavigator(Context* context);

    /**
     * Creates a new {@link NavGraph} associated with this navigator.
     * @return
     */
    NavGraph* createDestination() override;
    void navigate(/*@NonNull NavGraph*/NavDestination* destination, /*@Nullable*/Bundle* args,
            /*@Nullable*/ NavOptions* navOptions) override;

    bool popBackStack() override;
};
}/*endof namespace*/
#endif /*__NAVGRAPH_NAVIGATOR_H__*/


