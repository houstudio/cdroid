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
#include <widgetEx/recyclerview/scrollbarhelper.h>
#include <widgetEx/recyclerview/orientationhelper.h>
namespace cdroid{

ScrollbarHelper::ScrollbarHelper(){
}

int ScrollbarHelper::computeScrollOffset(RecyclerView::State& state, OrientationHelper& orientation,
        View* startChild, View* endChild, RecyclerView::LayoutManager& lm,
        bool smoothScrollbarEnabled, bool reverseLayout) {
    if (lm.getChildCount() == 0 || state.getItemCount() == 0 || startChild == nullptr
            || endChild == nullptr) {
        return 0;
    }
    const int minPosition = std::min(lm.getPosition(startChild), lm.getPosition(endChild));
    const int maxPosition = std::max(lm.getPosition(startChild), lm.getPosition(endChild));
    const int itemsBefore = reverseLayout
            ? std::max(0, state.getItemCount() - maxPosition - 1)
            : std::max(0, minPosition);
    if (!smoothScrollbarEnabled) {
        return itemsBefore;
    }
    const int laidOutArea = std::abs(orientation.getDecoratedEnd(endChild)
            - orientation.getDecoratedStart(startChild));
    const int itemRange = std::abs(lm.getPosition(startChild)
            - lm.getPosition(endChild)) + 1;
    const float avgSizePerRow = (float) laidOutArea / itemRange;

    return std::round(itemsBefore * avgSizePerRow + (orientation.getStartAfterPadding()
            - orientation.getDecoratedStart(startChild)));
}

int ScrollbarHelper::computeScrollExtent(RecyclerView::State& state, OrientationHelper& orientation,
        View* startChild, View* endChild, RecyclerView::LayoutManager& lm, bool smoothScrollbarEnabled) {
    if (lm.getChildCount() == 0 || state.getItemCount() == 0 || startChild == nullptr
            || endChild == nullptr) {
        return 0;
    }
    if (!smoothScrollbarEnabled) {
        return std::abs(lm.getPosition(startChild) - lm.getPosition(endChild)) + 1;
    }
    const int extend = orientation.getDecoratedEnd(endChild)
            - orientation.getDecoratedStart(startChild);
    return std::min(orientation.getTotalSpace(), extend);
}

int ScrollbarHelper::computeScrollRange(RecyclerView::State& state, OrientationHelper& orientation,
        View* startChild, View* endChild, RecyclerView::LayoutManager& lm,bool smoothScrollbarEnabled) {
    if (lm.getChildCount() == 0 || state.getItemCount() == 0 || startChild == nullptr
            || endChild == nullptr) {
        return 0;
    }
    if (!smoothScrollbarEnabled) {
        return state.getItemCount();
    }
    // smooth scrollbar enabled. try to estimate better.
    const int laidOutArea = orientation.getDecoratedEnd(endChild)
            - orientation.getDecoratedStart(startChild);
    const int laidOutRange = std::abs(lm.getPosition(startChild)
            - lm.getPosition(endChild)) + 1;
    // estimate a size for full list.
    return (int) ((float) laidOutArea / laidOutRange * state.getItemCount());
}
}/*endof namespace*/
