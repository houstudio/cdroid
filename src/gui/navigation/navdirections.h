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
#ifndef __NAV_DIRECTIONS_H__
#define __NAV_DIRECTIONS_H__
namespace cdroid{
class NavDirections {
public:
    /**
     * Returns a action id to navigate with.
     *
     * @return id of an action
     */
    virtual int getActionId()=0;

    /**
     * Returns arguments to pass to the destination
     */
    virtual Bundle* getArguments()=0;
};
}
#endif/*__NAV_DIRECTIONS_H__*/
