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
#include <widgetEx/wear/wearablenavigationdraweradapter.h>
#include <widgetEx/wear/wearablenavigationdrawerpresenter.h>
#include <porting/cdlog.h>

namespace cdroid{

// WearableNavigationDrawerView.TAG (private static upstream; the inner adapter
// class reads the outer constant).
static constexpr const char* TAG = "WearableNavDrawer";

// WearableNavigationDrawerView.java lines 273-282
void WearableNavigationDrawerAdapter::notifyDataSetChanged() {
    // If this method is called before drawer.setAdapter, then we will not yet have a
    // presenter.
    if (mPresenter != nullptr) {
        mPresenter->onDataSetChanged();
    } else {
        LOGW("adapter.notifyDataSetChanged called before drawer.setAdapter; ignoring.");
    }
}

// WearableNavigationDrawerView.java lines 286-289
void WearableNavigationDrawerAdapter::setPresenter(WearableNavigationDrawerPresenter* presenter) {
    mPresenter = presenter;
}

}/*endof namespace*/
