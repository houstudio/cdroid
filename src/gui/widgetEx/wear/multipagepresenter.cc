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
#include <widgetEx/wear/multipagepresenter.h>
#include <widgetEx/wear/wearablenavigationdraweradapter.h>
#include <widgetEx/wear/wearablenavigationdrawerview.h>
#include <widgetEx/wear/wearabledrawercontroller.h>
#include <widgetEx/wear/wearabledrawerview.h>
#include <stdexcept>

namespace cdroid{

// androidx.wear.internal.widget.drawer.MultiPagePresenter.java (lines 32-125)

MultiPagePresenter::MultiPagePresenter(WearableNavigationDrawerView* drawer, Ui* ui,
        bool isAccessibilityEnabled)
    : mIsAccessibilityEnabled(isAccessibilityEnabled) {
    if (drawer == nullptr) {
        throw std::invalid_argument("Received null drawer.");
    }
    if (ui == nullptr) {
        throw std::invalid_argument("Received null ui.");
    }
    mDrawer = drawer;
    mUi = ui;
    mUi->initialize(drawer, this);
}

MultiPagePresenter::~MultiPagePresenter() {
    delete mUi;
}

void MultiPagePresenter::onDataSetChanged() {
    mUi->notifyNavigationPagerAdapterDataChanged();
    mUi->notifyPageIndicatorDataChanged();
}

void MultiPagePresenter::onNewAdapter(WearableNavigationDrawerAdapter* adapter) {
    if (adapter == nullptr) {
        throw std::invalid_argument("Received null adapter.");
    }
    mAdapter = adapter;
    mAdapter->setPresenter(this);
    mUi->setNavigationPagerAdapter(adapter);
}

void MultiPagePresenter::onSelected(int index) {
    notifyItemSelectedListeners(index);
}

void MultiPagePresenter::onSetCurrentItemRequested(int index, bool smoothScrollTo) {
    mUi->setNavigationPagerSelectedItem(index, smoothScrollTo);
}

bool MultiPagePresenter::onDrawerTapped() {
    if (mDrawer->isOpened()) {
        // Java dereferences getController() unconditionally (NPE when the drawer is
        // detached); keep an unattached tap inert instead.
        WearableDrawerController* controller = mDrawer->getController();
        if (controller != nullptr) {
            if (mIsAccessibilityEnabled) {
                // When accessibility gestures are enabled, the user can't access a closed nav
                // drawer, so peek it instead.
                controller->peekDrawer();
            } else {
                controller->closeDrawer();
            }
        }
        return true;
    }
    return false;
}

}/*endof namespace*/
