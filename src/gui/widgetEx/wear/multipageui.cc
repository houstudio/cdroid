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
#include <widgetEx/wear/multipageui.h>
#include <widgetEx/wear/wearablenavigationdrawerview.h>
#include <widgetEx/wear/wearablenavigationdraweradapter.h>
#include <widgetEx/wear/pageindicatorview.h>
#include <widgetEx/widgetex_styleable.h>
#include <widget/viewpager.h>
#include <widget/imageview.h>
#include <widget/textview.h>
#include <view/layoutinflater.h>
#include <porting/cdlog.h>
#include <stdexcept>

namespace cdroid{
using namespace cdroid::internal;

// androidx.wear.internal.widget.drawer.MultiPageUi.java (lines 43-171)
static constexpr const char* TAG = "MultiPageUi";

MultiPageUi::~MultiPageUi() {
    delete mNavigationPagerAdapter;
}

void MultiPageUi::initialize(WearableNavigationDrawerView* drawer,
        WearableNavigationDrawerPresenter* presenter) {
    if (drawer == nullptr) {
        throw std::invalid_argument("Received null drawer.");
    }
    if (presenter == nullptr) {
        throw std::invalid_argument("Received null presenter.");
    }
    mPresenter = presenter;

    LayoutInflater* inflater = LayoutInflater::from(drawer->getContext());
    View* content = inflater->inflate((int) R::layout::ws_navigation_drawer_view, drawer,
            false /* attachToRoot */);

    mNavigationPager =
            (ViewPager*) content->findViewById((int) R::id::ws_navigation_drawer_view_pager);
    mPageIndicatorView = (PageIndicatorView*) content->findViewById(
            (int) R::id::ws_navigation_drawer_page_indicator);

    drawer->setDrawerContent(content);
}

void MultiPageUi::setNavigationPagerAdapter(WearableNavigationDrawerAdapter* adapter) {
    if (mNavigationPager == nullptr || mPageIndicatorView == nullptr) {
        LOGW("%s: setNavigationPagerAdapter was called before initialize.", TAG);
        return;
    }

    // A replacement per adapter, as GC does upstream. CDROID's ViewPager borrows
    // its adapter and drains the old one inside setAdapter, so delete only after
    // the pager has switched over.
    NavigationPagerAdapter* oldPagerAdapter = mNavigationPagerAdapter;
    mNavigationPagerAdapter = new NavigationPagerAdapter(adapter);
    mNavigationPager->setAdapter(mNavigationPagerAdapter);
    delete oldPagerAdapter;

    // Clear out the old page listeners and add a new one for this adapter.
    mNavigationPager->clearOnPageChangeListeners();
    ViewPager::OnPageChangeListener pageChangeListener;
    pageChangeListener.onPageSelected = [this](int position) {
        mPresenter->onSelected(position);
    };
    mNavigationPager->addOnPageChangeListener(pageChangeListener);
    // PageIndicatorView adds itself as a page change listener here, so this must come after
    // they are cleared.
    mPageIndicatorView->setPager(mNavigationPager);
}

void MultiPageUi::notifyPageIndicatorDataChanged() {
    if (mPageIndicatorView != nullptr) {
        mPageIndicatorView->notifyDataSetChanged();
    }
}

void MultiPageUi::notifyNavigationPagerAdapterDataChanged() {
    if (mNavigationPager != nullptr) {
        PagerAdapter* adapter = mNavigationPager->getAdapter();
        if (adapter != nullptr) {
            adapter->notifyDataSetChanged();
        }
    }
}

void MultiPageUi::setNavigationPagerSelectedItem(int index, bool smoothScrollTo) {
    if (mNavigationPager != nullptr) {
        mNavigationPager->setCurrentItem(index, smoothScrollTo);
    }
}

void* MultiPageUi::NavigationPagerAdapter::instantiateItem(ViewGroup* container, int position) {
    // Do not attach to root in the inflate method. The view needs to returned at the end
    // of this method. Attaching to root will cause view to point to container instead.
    View* view = LayoutInflater::from(container->getContext())
            ->inflate((int) R::layout::ws_navigation_drawer_item_view, container, false);
    container->addView(view);
    ImageView* iconView =
            (ImageView*) view->findViewById((int) R::id::ws_navigation_drawer_item_icon);
    TextView* textView =
            (TextView*) view->findViewById((int) R::id::ws_navigation_drawer_item_text);
    iconView->setImageDrawable(mAdapter->getItemDrawable(position));
    textView->setText(mAdapter->getItemText(position));
    return view;
}

void MultiPageUi::NavigationPagerAdapter::destroyItem(ViewGroup* container, int position,
        void* object) {
    container->removeView((View*) object);
}

int MultiPageUi::NavigationPagerAdapter::getCount() {
    return mAdapter->getCount();
}

int MultiPageUi::NavigationPagerAdapter::getItemPosition(void* object) {
    return POSITION_NONE;
}

bool MultiPageUi::NavigationPagerAdapter::isViewFromObject(View* view, void* object) {
    return view == (View*) object;
}

MultiPageUi::NavigationPagerAdapter::NavigationPagerAdapter(WearableNavigationDrawerAdapter* adapter)
    : mAdapter(adapter) {
}

}/*endof namespace*/
