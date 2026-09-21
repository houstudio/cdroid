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
#ifndef __WEARABLE_NAVIGATION_DRAWER_ADAPTER_H__
#define __WEARABLE_NAVIGATION_DRAWER_ADAPTER_H__
#include <string>
namespace cdroid{

class Drawable;
class WearableNavigationDrawerPresenter;

/** Adapter for specifying the contents of WearableNavigationDrawer.
 *  Upstream this is the abstract inner class
 *  WearableNavigationDrawerView.WearableNavigationDrawerAdapter
 *  (WearableNavigationDrawerView.java lines 250-290); it is ported as a
 *  top-level class (C++ has no inner classes owning an outer instance). */
class WearableNavigationDrawerAdapter {
private:
    /** @Nullable upstream (null before drawer.setAdapter wires it). */
    WearableNavigationDrawerPresenter* mPresenter = nullptr;

public:
    virtual ~WearableNavigationDrawerAdapter() = default;

    /** Get the text associated with the item at {@code pos}. */
    virtual std::string getItemText(int pos) const = 0;

    /** Get the drawable associated with the item at {@code pos}.
        @return a Drawable the caller takes ownership of (Java relies on GC;
        the UIs hand it straight to ImageView/CircledImageView::setImageDrawable,
        which owns and eventually deletes it). */
    virtual Drawable* getItemDrawable(int pos) const = 0;

    /** Returns the number of items in this adapter. */
    virtual int getCount() const = 0;

    /** This method should be called by the application if the data backing this
        adapter has changed and associated views should update. */
    void notifyDataSetChanged();

    /** Library-internal upstream (@RestrictTo LIBRARY); wired by the drawer's
        presenter on setAdapter. */
    void setPresenter(WearableNavigationDrawerPresenter* presenter);
};

}/*endof namespace*/
#endif/*__WEARABLE_NAVIGATION_DRAWER_ADAPTER_H__*/
