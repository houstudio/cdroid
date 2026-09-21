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
#ifndef __WEARABLE_NAVIGATION_DRAWER_PRESENTER_H__
#define __WEARABLE_NAVIGATION_DRAWER_PRESENTER_H__
#include <core/callbackbase.h>
#include <algorithm>
#include <vector>
namespace cdroid{

class WearableNavigationDrawerAdapter;

// androidx.wear.widget.drawer.WearableNavigationDrawerView.OnItemSelectedListener
// (WearableNavigationDrawerView.java lines 69-75): notified when the user
// selects an item. Single-callback add/remove listener → CDROID convention is a
// CallbackBase functor; it plays onItemSelected(int pos). Declared at namespace
// scope (not nested in the view) so the presenter can store it without a
// header cycle; WearableNavigationDrawerView re-exports it as a nested alias.
using OnItemSelectedListener = CallbackBase<void,int>;

/** Controls the behavior of this view where the behavior may differ between
 *  single and multi-page.
 *  androidx.wear.internal.widget.drawer.WearableNavigationDrawerPresenter.java
 *  (lines 33-94). */
class WearableNavigationDrawerPresenter {
private:
    // Upstream is a HashSet (identity set); CallbackBase shares its functor
    // handle across copies, which provides the same identity for removal.
    std::vector<OnItemSelectedListener> mOnItemSelectedListeners;

public:
    virtual ~WearableNavigationDrawerPresenter() = default;

    /** Indicates to the presenter that the underlying data has changed. */
    virtual void onDataSetChanged() = 0;

    /** Indicates to the presenter that the drawer has a new adapter. */
    virtual void onNewAdapter(WearableNavigationDrawerAdapter* adapter) = 0;

    /** Indicates to the presenter that the user has selected an item. */
    virtual void onSelected(int index) = 0;

    /** Indicates to the presenter that the developer wishes to change which item is selected. */
    virtual void onSetCurrentItemRequested(int index, bool smoothScrollTo) = 0;

    /** Indicates to the presenter that the user has tapped on the drawer.
        @return {@code true} if the touch event has been handled and should not propagate further. */
    virtual bool onDrawerTapped() = 0;

    /** Indicates to the presenter that a new OnItemSelectedListener has been added. */
    void onItemSelectedListenerAdded(const OnItemSelectedListener& listener) {
        // Set semantics: skip an already-registered (identical) listener.
        for (const auto& registered : mOnItemSelectedListeners) {
            if (registered == listener) {
                return;
            }
        }
        mOnItemSelectedListeners.push_back(listener);
    }

    /** Indicates to the presenter that an OnItemSelectedListener has been removed. */
    void onItemSelectedListenerRemoved(const OnItemSelectedListener& listener) {
        mOnItemSelectedListeners.erase(
                std::remove(mOnItemSelectedListeners.begin(), mOnItemSelectedListeners.end(),
                        listener),
                mOnItemSelectedListeners.end());
    }

    /** Notifies all listeners that the item at {@code selectedPos} has been selected.
        Package-private upstream; reachable by the presenter subclasses in C++. */
    void notifyItemSelectedListeners(int selectedPos) {
        for (OnItemSelectedListener& listener : mOnItemSelectedListeners) {
            listener(selectedPos);
        }
    }
};

}/*endof namespace*/
#endif/*__WEARABLE_NAVIGATION_DRAWER_PRESENTER_H__*/
