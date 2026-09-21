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
#include <widgetEx/wear/singlepageui.h>
#include <widgetEx/wear/wearablenavigationdrawerview.h>
#include <widgetEx/wear/wearabledrawercontroller.h>
#include <widgetEx/wear/circledimageview.h>
#include <widgetEx/widgetex_styleable.h>
#include <widget/textview.h>
#include <widget/toast.h>
#include <view/gravity.h>
#include <view/layoutinflater.h>
#include <stdexcept>

namespace cdroid{
using namespace cdroid::internal;

// androidx.wear.internal.widget.drawer.SinglePageUi.java (lines 43-183)

namespace {
// (lines 46-55)
const uint32_t SINGLE_PAGE_BUTTON_IDS[] = {
    R::id::ws_nav_drawer_icon_0,
    R::id::ws_nav_drawer_icon_1,
    R::id::ws_nav_drawer_icon_2,
    R::id::ws_nav_drawer_icon_3,
    R::id::ws_nav_drawer_icon_4,
    R::id::ws_nav_drawer_icon_5,
    R::id::ws_nav_drawer_icon_6,
};

// (lines 58-68)
const uint32_t SINGLE_PAGE_LAYOUT_RES[] = {
    0,
    R::layout::ws_single_page_nav_drawer_1_item,
    R::layout::ws_single_page_nav_drawer_2_item,
    R::layout::ws_single_page_nav_drawer_3_item,
    R::layout::ws_single_page_nav_drawer_4_item,
    R::layout::ws_single_page_nav_drawer_5_item,
    R::layout::ws_single_page_nav_drawer_6_item,
    R::layout::ws_single_page_nav_drawer_7_item,
};
} // namespace

SinglePageUi::SinglePageUi(WearableNavigationDrawerView* navigationDrawer) {
    if (navigationDrawer == nullptr) {
        throw std::invalid_argument("Received null navigationDrawer.");
    }
    mDrawer = navigationDrawer;
    // Upstream's close runnable dereferences getController() unconditionally; a
    // posted runnable can fire after the drawer was detached (controller null,
    // Java NPE), so it stays inert then.
    mCloseDrawerRunnable = [this]() {
        WearableDrawerController* controller = mDrawer->getController();
        if (controller != nullptr) {
            controller->closeDrawer();
        }
    };
}

SinglePageUi::~SinglePageUi() {
    // The runnable is captured by this pointer; make sure a pending delayed
    // close cannot run after the presenter (and this Ui) are gone. View
    // removeCallbacks matches by functor identity.
    mDrawer->removeCallbacks(mCloseDrawerRunnable);
}

void SinglePageUi::setPresenter(WearableNavigationDrawerPresenter* presenter) {
    mPresenter = presenter;
}

void SinglePageUi::initialize(int count) {
    if (count < 0 || count >= (int) (sizeof(SINGLE_PAGE_LAYOUT_RES) / sizeof(SINGLE_PAGE_LAYOUT_RES[0]))
            || SINGLE_PAGE_LAYOUT_RES[count] == 0) {
        // Count > 7 leaves the drawer without content views; the per-index
        // methods below no-op instead of dereferencing a missing view (upstream
        // would throw here — the class doc calls this "displayed as empty").
        mDrawer->setDrawerContent(nullptr);
        return;
    }

    LayoutInflater* inflater = LayoutInflater::from(mDrawer->getContext());
    View* content = inflater->inflate((int) SINGLE_PAGE_LAYOUT_RES[count], mDrawer, false);
    View* peek = inflater->inflate((int) R::layout::ws_single_page_nav_drawer_peek_view,
            mDrawer, false);

    mTextView = (TextView*) content->findViewById((int) R::id::ws_nav_drawer_text);
    mSinglePageImageViews.assign(count, nullptr);
    for (int i = 0; i < count; i++) {
        mSinglePageImageViews[i] =
                (CircledImageView*) content->findViewById((int) SINGLE_PAGE_BUTTON_IDS[i]);
        // OnSelectedClickHandler (lines 168-182): notifies the presenter that the
        // item at the given index has been selected.
        WearableNavigationDrawerPresenter* presenter = mPresenter;
        const int index = i;
        mSinglePageImageViews[i]->setOnClickListener([presenter, index](View&) {
            presenter->onSelected(index);
        });
        mSinglePageImageViews[i]->setCircleHidden(true);
    }

    mDrawer->setDrawerContent(content);
    mDrawer->setPeekContent(peek);
}

void SinglePageUi::setIcon(int index, Drawable* drawable, const std::string& contentDescription) {
    if (index < 0 || index >= (int) mSinglePageImageViews.size()) {
        return;
    }
    mSinglePageImageViews[index]->setImageDrawable(drawable);
    mSinglePageImageViews[index]->setContentDescription(contentDescription);
}

void SinglePageUi::setText(const std::string& itemText, bool showToastIfNoTextView) {
    if (mTextView != nullptr) {
        mTextView->setText(itemText);
    } else if (showToastIfNoTextView) {
        Toast* toast = Toast::makeText(mDrawer->getContext(), itemText, Toast::LENGTH_SHORT);
        toast->setGravity(Gravity::CENTER, 0 /* xOffset */, 0 /* yOffset */);
        toast->show();
    }
}

void SinglePageUi::selectItem(int index) {
    if (index < 0 || index >= (int) mSinglePageImageViews.size()) {
        return;
    }
    mSinglePageImageViews[index]->setCircleHidden(false);
}

void SinglePageUi::deselectItem(int index) {
    if (index < 0 || index >= (int) mSinglePageImageViews.size()) {
        return;
    }
    mSinglePageImageViews[index]->setCircleHidden(true);
}

void SinglePageUi::closeDrawerDelayed(long delayMs) {
    mDrawer->removeCallbacks(mCloseDrawerRunnable);
    mDrawer->postDelayed(mCloseDrawerRunnable, delayMs);
}

void SinglePageUi::peekDrawer() {
    WearableDrawerController* controller = mDrawer->getController();
    if (controller != nullptr) { // Java NPEs when the drawer is detached; keep inert
        controller->peekDrawer();
    }
}

}/*endof namespace*/
