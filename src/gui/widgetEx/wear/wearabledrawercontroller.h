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
*/
#ifndef __WEARABLE_DRAWER_CONTROLLER_H__
#define __WEARABLE_DRAWER_CONTROLLER_H__

namespace cdroid{

class WearableDrawerLayout;
class WearableDrawerView;

/** Provides the ability to manipulate a WearableDrawerView's position within a
 *  WearableDrawerLayout.
 *  androidx.wear.widget.drawer.WearableDrawerController.java (lines 23-53).
 *  The constructor is package-private upstream; it is created solely by
 *  WearableDrawerLayout, which owns this handle. */
class WearableDrawerController {
private:
    WearableDrawerLayout* mDrawerLayout;
    WearableDrawerView* mDrawerView;
public:
    WearableDrawerController(WearableDrawerLayout* drawerLayout, WearableDrawerView* drawerView);

    /** Requests that the WearableDrawerView be opened. */
    void openDrawer();

    /** Requests that the WearableDrawerView be closed. */
    void closeDrawer();

    /** Requests that the WearableDrawerView be peeked. */
    void peekDrawer();
};

}/*endof namespace*/
#endif/*__WEARABLE_DRAWER_CONTROLLER_H__*/
