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
#ifndef __FLING_WATCHER_FACTORY_H__
#define __FLING_WATCHER_FACTORY_H__
#include <view/view.h>
#include <unordered_map>
namespace cdroid{

// Ported from androidx.wear.widget.drawer.FlingWatcherFactory (FlingWatcherFactory.java:38-102).
class FlingWatcherFactory{
public:
    /** Notified when a fling completes and the view has settled. Polling may be used to determine
     *  when the fling has completed, so there may be up to a 100ms delay. */
    class FlingListener{
    public:
        virtual ~FlingListener() = default;
        virtual void onFlingComplete(View& view) = 0;
    };

    /** Watches a given view to detect the end of a fling. Will notify a FlingListener when the
     *  end is found. */
    class FlingWatcher{
    public:
        virtual ~FlingWatcher() = default;
        virtual void watch() = 0;
    };
private:
    FlingListener& mListener;
    // Upstream is a WeakHashMap<View, FlingWatcher>: values are owned here, keys are borrowed
    // View pointers (entries are only ever compared, never dereferenced).
    std::unordered_map<View*, FlingWatcher*> mWatchers;
public:
    explicit FlingWatcherFactory(FlingListener& listener);
    ~FlingWatcherFactory();

    /** Returns a FlingWatcher for the particular type of view, or null if the type is unsupported. */
    FlingWatcher* getFor(View& view);
private:
    FlingWatcher* createFor(View& view);

    FlingWatcherFactory(const FlingWatcherFactory&) = delete;
    FlingWatcherFactory& operator=(const FlingWatcherFactory&) = delete;
};
}//endof namespace
#endif
