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
#ifndef __LAYOUT_STATE_H__
#define __LAYOUT_STATE_H__
namespace cdroid{
class LayoutState {
public:
    static constexpr int LAYOUT_START = -1;
    static constexpr int LAYOUT_END = 1;
    static constexpr int INVALID_LAYOUT = INT_MIN;//Integer.MIN_VALUE;
    static constexpr int ITEM_DIRECTION_HEAD = -1;
    static constexpr int ITEM_DIRECTION_TAIL = 1;

    bool mRecycle = true;

    int mAvailable;
    int mCurrentPosition;
    int mItemDirection;
    int mLayoutDirection;
    int mStartLine = 0;
    int mEndLine = 0;
    bool mStopInFocusable;
    bool mInfinite;
    bool hasMore(RecyclerView::State& state) {
        return mCurrentPosition >= 0 && mCurrentPosition < state.getItemCount();
    }
    View* next(RecyclerView::Recycler& recycler) {
        View* view = recycler.getViewForPosition(mCurrentPosition);
        mCurrentPosition += mItemDirection;
        return view;
    }
};
}/*endof namespace */
#endif/*__LAYOUT_STATE_H__*/
