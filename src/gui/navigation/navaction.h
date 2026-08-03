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
namespace cdroid{
class NavOptions;

class NavAction {
private:
    int mDestinationId;
    NavOptions*mNavOptions;
public:
    /**
     * Creates a new NavAction for the given destination.
     *
     * @param destinationId the ID of the destination that should be navigated to when this
     *                      action is used.
     */
    NavAction(int destinationId);

    /**
     * Creates a new NavAction for the given destination.
     *
     * @param destinationId the ID of the destination that should be navigated to when this
     *                      action is used.
     * @param navOptions special options for this action that should be used by default
     */
    NavAction(int destinationId, NavOptions* navOptions);
    // Owns mNavOptions (set via ctor or setNavOptions from NavInflater.inflateAction's
    // NavOptions::Builder::build()). CDROID has no GC: delete on destruction.
    ~NavAction();

    /**
     * Gets the ID of the destination that should be navigated to when this action is used
     */
    int getDestinationId() const;

    /**
     * Sets the NavOptions to be used by default when navigating to this action.
     *
     * @param navOptions special options for this action that should be used by default
     */
    void setNavOptions(NavOptions* navOptions);

    /**
     * Gets the NavOptions to be used by default when navigating to this action.
     */
    NavOptions* getNavOptions()const;
};
}/*endof namaespace*/
