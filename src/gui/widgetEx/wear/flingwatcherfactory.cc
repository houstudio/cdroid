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
#include <widgetEx/wear/flingwatcherfactory.h>
#include <widgetEx/wear/recyclerviewflingwatcher.h>
#include <widgetEx/wear/abslistviewflingwatcher.h>
#include <widgetEx/wear/scrollviewflingwatcher.h>
#include <widgetEx/wear/nestedscrollviewflingwatcher.h>
#include <widget/abslistview.h>
#include <widget/scrollview.h>
#include <widget/nestedscrollview.h>
#include <widgetEx/recyclerview/recyclerview.h>
namespace cdroid{

FlingWatcherFactory::FlingWatcherFactory(FlingListener& listener)
    : mListener(listener) {
}

FlingWatcherFactory::~FlingWatcherFactory() {
    for (auto& entry : mWatchers) {
        delete entry.second;
    }
    mWatchers.clear();
}

FlingWatcherFactory::FlingWatcher* FlingWatcherFactory::getFor(View& view) {
    FlingWatcher* watcher = nullptr;
    const auto it = mWatchers.find(&view);
    if (it != mWatchers.end()) {
        watcher = it->second;
    } else {
        watcher = createFor(view);
        if (watcher != nullptr) {
            mWatchers.emplace(&view, watcher);
        }
    }
    return watcher;
}

FlingWatcherFactory::FlingWatcher* FlingWatcherFactory::createFor(View& view) {
    if (RecyclerView* recyclerView = dynamic_cast<RecyclerView*>(&view)) {
        return new RecyclerViewFlingWatcher(mListener, *recyclerView);
    } else if (AbsListView* absListView = dynamic_cast<AbsListView*>(&view)) {
        return new AbsListViewFlingWatcher(mListener, *absListView);
    } else if (ScrollView* scrollView = dynamic_cast<ScrollView*>(&view)) {
        return new ScrollViewFlingWatcher(mListener, *scrollView);
    } else if (NestedScrollView* nestedScrollView = dynamic_cast<NestedScrollView*>(&view)) {
        return new NestedScrollViewFlingWatcher(mListener, *nestedScrollView);
    } else {
        return nullptr;
    }
}
}//endof namespace
