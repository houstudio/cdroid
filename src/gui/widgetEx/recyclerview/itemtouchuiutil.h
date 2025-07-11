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
#ifndef __ITEMTOUCHUI_UTIL_H__
#define __ITEMTOUCHUI_UTIL_H__
#include <widgetEx/recyclerview/recyclerview.h>
namespace cdroid{
class ItemTouchUIUtil {
public:
    virtual void onDraw(Canvas& c, RecyclerView& recyclerView, View& view,
            float dX, float dY, int actionState, bool isCurrentlyActive)=0;

    virtual void onDrawOver(Canvas& c, RecyclerView& recyclerView, View& view,
            float dX, float dY, int actionState, bool isCurrentlyActive)=0;
    virtual void clearView(View& view)=0;
    virtual void onSelected(View& view)=0;
};

class ItemTouchUIUtilImpl:public ItemTouchUIUtil {
private:
    static float findMaxElevation(RecyclerView& recyclerView, View& itemView);
    friend class ItemTouchHelper;
public:
    void onDraw(Canvas& c, RecyclerView& recyclerView, View& view, float dX, float dY,
            int actionState, bool isCurrentlyActive)override;
    void onDrawOver(Canvas& c, RecyclerView& recyclerView, View& view, float dX, float dY,
            int actionState, bool isCurrentlyActive)override;
    void clearView(View& view)override;
    void onSelected(View& view)override;
};
}
#endif/*__ITEMTOUCHUI_UTIL_H__*/
