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
#include <widgetEx/wear/singlepagepresenter.h>
#include <widgetEx/wear/wearablenavigationdraweradapter.h>
#include <algorithm>
#include <stdexcept>

namespace cdroid{

// androidx.wear.internal.widget.drawer.SinglePagePresenter.java (lines 34-167)

SinglePagePresenter::SinglePagePresenter(Ui* ui, bool isAccessibilityEnabled)
    : mIsAccessibilityEnabled(isAccessibilityEnabled) {
    if (ui == nullptr) {
        throw std::invalid_argument("Received null ui.");
    }

    mUi = ui;
    mUi->setPresenter(this);
    onDataSetChanged();
}

SinglePagePresenter::~SinglePagePresenter() {
    delete mUi;
}

void SinglePagePresenter::onDataSetChanged() {
    if (mAdapter == nullptr) {
        return;
    }
    const int count = mAdapter->getCount();
    if (mCount != count) {
        mCount = count;
        mSelected = std::min(mSelected, count - 1);
        mUi->initialize(count);
    }
    for (int i = 0; i < count; i++) {
        mUi->setIcon(i, mAdapter->getItemDrawable(i), mAdapter->getItemText(i));
    }

    mUi->setText(mAdapter->getItemText(mSelected), false /* showToastIfNoTextView */);
    mUi->selectItem(mSelected);
}

void SinglePagePresenter::onNewAdapter(WearableNavigationDrawerAdapter* adapter) {
    if (adapter == nullptr) {
        throw std::invalid_argument("Received null adapter.");
    }
    mAdapter = adapter;
    mAdapter->setPresenter(this);
    onDataSetChanged();
}

void SinglePagePresenter::onSelected(int index) {
    mUi->deselectItem(mSelected);
    mUi->selectItem(index);
    mSelected = index;
    if (mIsAccessibilityEnabled) {
        // When accessibility gestures are enabled, the user can't access a closed nav drawer,
        // so peek it instead.
        mUi->peekDrawer();
    } else {
        mUi->closeDrawerDelayed(DRAWER_CLOSE_DELAY_MS);
    }

    if (mAdapter != nullptr) {
        mUi->setText(mAdapter->getItemText(index), true /* showToastIfNoTextView */);
    }
    notifyItemSelectedListeners(index);
}

void SinglePagePresenter::onSetCurrentItemRequested(int index, bool smoothScrollTo) {
    mUi->deselectItem(mSelected);
    mUi->selectItem(index);
    mSelected = index;
    if (mAdapter != nullptr) {
        mUi->setText(mAdapter->getItemText(index), false /* showToastIfNoTextView */);
    }
    notifyItemSelectedListeners(index);
}

bool SinglePagePresenter::onDrawerTapped() {
    // Do nothing. Use onSelected as our tap trigger so that we get which index was tapped on.
    return false;
}

}/*endof namespace*/
