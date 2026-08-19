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
#ifndef __OPENABLE_H__
#define __OPENABLE_H__
// androidx.customview.widget.Openable — implemented by drawer-like layouts
// (DrawerLayout) so NavigationUI can open them from the Up button on a
// top-level destination.
namespace cdroid{

class Openable{
public:
    virtual ~Openable() = default;
    /** Whether the layout is currently in the opened state. */
    virtual bool isOpen() = 0;
    /** Move the layout to the opened state. */
    virtual void open() = 0;
    /** Move the layout to the closed state. */
    virtual void close() = 0;
};

}//namespace cdroid
#endif/*__OPENABLE_H__*/
