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
#ifndef __SCROLL_VIEW_FLING_WATCHER_H__
#define __SCROLL_VIEW_FLING_WATCHER_H__
#include <widgetEx/wear/flingwatcherfactory.h>
#include <core/handler.h>
#include <widget/scrollview.h>
namespace cdroid{

// Ported from androidx.wear.widget.drawer.ScrollViewFlingWatcher
// (ScrollViewFlingWatcher.java:45-117). ScrollView has no scroll-state callback, so scroll
// events are watched: the fling is assumed finished at the top/bottom of the view or when
// no event arrives within MAX_WAIT_TIME_MS. Upstream "implements OnScrollChangeListener"
// becomes a by-value View::OnScrollChangeListener member wired back to this class.
class ScrollViewFlingWatcher: public FlingWatcherFactory::FlingWatcher{
private:
    static constexpr int MAX_WAIT_TIME_MS = 100;
    Handler mMainThreadHandler;
    FlingWatcherFactory::FlingListener& mListener;
    ScrollView* mScrollView; /* borrowed; upstream WeakReference<ScrollView> */
    View::OnScrollChangeListener mScrollListener;
    Runnable mNotifyListenerRunnable;
public:
    ScrollViewFlingWatcher(FlingWatcherFactory::FlingListener& listener, ScrollView& scrollView);

    void watch() override;
private:
    static bool isViewAtTopOrBottom(View& view);
    void onScrollChange(View& v, int scrollX, int scrollY, int oldScrollX, int oldScrollY);
    void onEndOfFlingFound();
    void scheduleNext();
};
}//endof namespace
#endif
