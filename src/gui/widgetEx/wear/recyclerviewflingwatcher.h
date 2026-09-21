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
#ifndef __RECYCLER_VIEW_FLING_WATCHER_H__
#define __RECYCLER_VIEW_FLING_WATCHER_H__
#include <widgetEx/wear/flingwatcherfactory.h>
#include <widgetEx/recyclerview/recyclerview.h>
namespace cdroid{

// Ported from androidx.wear.widget.drawer.RecyclerViewFlingWatcher
// (RecyclerViewFlingWatcher.java:36-89). Upstream "extends OnScrollListener" becomes a
// by-value OnScrollListener member wired back to this class (value-semantics listener model).
class RecyclerViewFlingWatcher: public FlingWatcherFactory::FlingWatcher{
private:
    FlingWatcherFactory::FlingListener& mListener;
    RecyclerView* mRecyclerView; /* borrowed; upstream WeakReference<RecyclerView> */
    RecyclerView::OnScrollListener mScrollListener;
public:
    RecyclerViewFlingWatcher(FlingWatcherFactory::FlingListener& listener, RecyclerView& view);

    void watch() override;

    void onScrollStateChanged(RecyclerView& recyclerView, int newState);
};
}//endof namespace
#endif
