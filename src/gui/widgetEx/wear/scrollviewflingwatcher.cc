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
#include <widgetEx/wear/scrollviewflingwatcher.h>
namespace cdroid{

ScrollViewFlingWatcher::ScrollViewFlingWatcher(FlingWatcherFactory::FlingListener& listener,
        ScrollView& scrollView)
    : mMainThreadHandler(Looper::getMainLooper())
    , mListener(listener)
    , mScrollView(&scrollView) {
    mScrollListener = [this](View& v, int scrollX, int scrollY, int oldScrollX, int oldScrollY) {
        onScrollChange(v, scrollX, scrollY, oldScrollX, oldScrollY);
    };
    mNotifyListenerRunnable = [this]() {
        onEndOfFlingFound();
    };
}

bool ScrollViewFlingWatcher::isViewAtTopOrBottom(View& view) {
    return !view.canScrollVertically(-1 /* up */) || !view.canScrollVertically(1 /* down */);
}

void ScrollViewFlingWatcher::watch() {
    if (mScrollView != nullptr) {
        mScrollView->setOnScrollChangeListener(mScrollListener);
        scheduleNext();
    }
}

void ScrollViewFlingWatcher::onScrollChange(View& v, int scrollX, int scrollY,
        int oldScrollX, int oldScrollY) {
    if (isViewAtTopOrBottom(v)) {
        onEndOfFlingFound();
    } else {
        scheduleNext();
    }
}

void ScrollViewFlingWatcher::onEndOfFlingFound() {
    mMainThreadHandler.removeCallbacks(mNotifyListenerRunnable);

    if (mScrollView != nullptr) {
        mScrollView->setOnScrollChangeListener(nullptr);
        mListener.onFlingComplete(*mScrollView);
    }
}

void ScrollViewFlingWatcher::scheduleNext() {
    mMainThreadHandler.removeCallbacks(mNotifyListenerRunnable);
    mMainThreadHandler.postDelayed(mNotifyListenerRunnable, MAX_WAIT_TIME_MS);
}
}//endof namespace
