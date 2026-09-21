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
#include <widgetEx/wear/abslistviewflingwatcher.h>
namespace cdroid{

AbsListViewFlingWatcher::AbsListViewFlingWatcher(FlingWatcherFactory::FlingListener& listener,
        AbsListView& listView)
    : mListener(listener), mListView(&listView) {
    mScrollListener.onScrollStateChanged = [this](AbsListView& view, int scrollState) {
        onScrollStateChanged(view, scrollState);
    };
    // Upstream leaves onScroll() empty, so mScrollListener.onScroll stays unset.
}

void AbsListViewFlingWatcher::watch() {
    if (mListView != nullptr) {
        mListView->setOnScrollListener(mScrollListener);
    }
}

void AbsListViewFlingWatcher::onScrollStateChanged(AbsListView& view, int scrollState) {
    if (scrollState != AbsListView::OnScrollListener::SCROLL_STATE_FLING) {
        // Upstream clears the View-level scroll-change listener here (not the AbsListView
        // OnScrollListener); kept verbatim.
        view.setOnScrollChangeListener(nullptr);
        mListener.onFlingComplete(view);
    }
}
}//endof namespace
