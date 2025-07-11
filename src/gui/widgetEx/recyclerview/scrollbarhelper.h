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
#ifndef __SCROLLBAR_HELPER_H__
#define __SCROLLBAR_HELPER_H__
#include <widgetEx/recyclerview/recyclerview.h>
namespace cdroid{
class OrientationHelper;
class ScrollbarHelper {
private:
     ScrollbarHelper();
public:
    static int computeScrollOffset(RecyclerView::State& state, OrientationHelper& orientation,
            View* startChild, View* endChild, RecyclerView::LayoutManager& lm,
            bool smoothScrollbarEnabled, bool reverseLayout);
    static int computeScrollExtent(RecyclerView::State& state, OrientationHelper& orientation,
            View* startChild, View* endChild, RecyclerView::LayoutManager& lm, bool smoothScrollbarEnabled);
    static int computeScrollRange(RecyclerView::State& state, OrientationHelper& orientation,
            View* startChild, View* endChild, RecyclerView::LayoutManager& lm, bool smoothScrollbarEnabled);
};
}/*endof namespace*/
#endif/*__SCROLLBAR_HELPER_H__*/
