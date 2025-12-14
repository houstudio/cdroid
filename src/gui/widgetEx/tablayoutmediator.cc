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
 *********************************************************************************/
#include <widgetEx/tablayoutmediator.h>
namespace cdroid{

TabLayoutMediator::TabLayoutMediator(TabLayout* tabLayout, ViewPager2* viewPager,const TabConfigurationStrategy& tabConfigurationStrategy)
    :TabLayoutMediator(tabLayout, viewPager, true, tabConfigurationStrategy){
}

TabLayoutMediator::TabLayoutMediator(TabLayout* tabLayout, ViewPager2* viewPager, bool autoRefresh,const TabConfigurationStrategy& tabConfigurationStrategy)
    :TabLayoutMediator(tabLayout, viewPager, autoRefresh, true, tabConfigurationStrategy){
}

TabLayoutMediator::TabLayoutMediator(TabLayout* tabLayout, ViewPager2* viewPager, bool autoRefresh, bool smoothScroll,const TabConfigurationStrategy& tabConfigurationStrategy) {
    mAttached = false;
    mViewPager= nullptr;
    mAdapter  = nullptr;
    mPagerAdapterObserver = nullptr;
    mTabLayout = tabLayout;
    mViewPager = viewPager;
    mAutoRefresh = autoRefresh;
    mSmoothScroll= smoothScroll;
    mTabConfigurationStrategy = tabConfigurationStrategy;
}

TabLayoutMediator::~TabLayoutMediator(){
    delete mPagerAdapterObserver;
    delete mOnPageChangeCallback;
}

void TabLayoutMediator::attach() {
    if (mAttached) {
        throw std::runtime_error("TabLayoutMediator is already attached");
    } else {
        mAdapter = mViewPager->getAdapter();
        if (mAdapter == nullptr) {
            throw std::runtime_error("TabLayoutMediator attached before ViewPager2 has an adapter");
        } else {
            mAttached = true;
            mOnPageChangeCallback = new TabLayoutOnPageChangeCallback(mTabLayout);
            mViewPager->registerOnPageChangeCallback(*mOnPageChangeCallback);

            //mOnTabSelectedListener = new ViewPagerOnTabSelectedListener(mViewPager, mSmoothScroll);
            mOnTabSelectedListener.onTabSelected=[this](TabLayout::Tab& tab){
                mViewPager->setCurrentItem(tab.getPosition(), mSmoothScroll);
            };
            mTabLayout->addOnTabSelectedListener(mOnTabSelectedListener);

            if (mAutoRefresh) {
                mPagerAdapterObserver = new PagerAdapterObserver(this);
                mAdapter->registerAdapterDataObserver(mPagerAdapterObserver);
            }
            populateTabsFromPagerAdapter();
            mTabLayout->setScrollPosition(mViewPager->getCurrentItem(), 0.0f, true);
        }
    }
}

void TabLayoutMediator::detach() {
    if (mAutoRefresh && mAdapter != nullptr) {
        mAdapter->unregisterAdapterDataObserver(mPagerAdapterObserver);
        delete mPagerAdapterObserver;
        mPagerAdapterObserver = nullptr;
    }

    mTabLayout->removeOnTabSelectedListener(mOnTabSelectedListener);
    mViewPager->unregisterOnPageChangeCallback(*mOnPageChangeCallback);
    mOnTabSelectedListener= {};
    delete mOnPageChangeCallback;
    mOnPageChangeCallback = nullptr;
    mAdapter = nullptr;
    mAttached= false;
}

void TabLayoutMediator::populateTabsFromPagerAdapter() {
    mTabLayout->removeAllTabs();
    if (mAdapter != nullptr) {
        const int adapterCount = mAdapter->getItemCount();
        for(int i = 0; i < adapterCount; ++i) {
            TabLayout::Tab* tab = mTabLayout->newTab();
            mTabConfigurationStrategy/*.onConfigureTab*/(*tab, i);
            mTabLayout->addTab(tab, false);
        }
        if (adapterCount > 0) {
            const int lastItem = mTabLayout->getTabCount() - 1;
            const int currItem = std::min(mViewPager->getCurrentItem(), lastItem);
            if (currItem != mTabLayout->getSelectedTabPosition()) {
                mTabLayout->selectTab(mTabLayout->getTabAt(currItem));
            }
        }
    }
}

//class TabLayoutOnPageChangeCallback extends ViewPager2.OnPageChangeCallback {
TabLayoutMediator::TabLayoutOnPageChangeCallback::TabLayoutOnPageChangeCallback(TabLayout* tabLayout) {
    mTabLayout = tabLayout;
    onPageScrollStateChanged = [this](int state){
        doPageScrollStateChanged(state);
    };
    onPageScrolled = [this](int position, float positionOffset, int positionOffsetPixels){
        doPageScrolled(position,positionOffset,positionOffsetPixels);
    };
    onPageSelected=[this](int position){
        doPageSelected(position);
    };
    reset();
}

void TabLayoutMediator::TabLayoutOnPageChangeCallback::doPageScrollStateChanged(int state) {
    mPreviousScrollState = mScrollState;
    mScrollState = state;
    if (mTabLayout != nullptr) {
        mTabLayout->updateViewPagerScrollState(mScrollState);
    }
}

void TabLayoutMediator::TabLayoutOnPageChangeCallback::doPageScrolled(int position, float positionOffset, int positionOffsetPixels) {
    if (mTabLayout != nullptr) {
        const bool updateSelectedTabView = (mScrollState != ViewPager2::SCROLL_STATE_SETTLING)
            || (mPreviousScrollState == ViewPager2::SCROLL_STATE_DRAGGING);
        const bool updateIndicator = (mScrollState != ViewPager2::SCROLL_STATE_SETTLING)
            || (mPreviousScrollState != ViewPager2::SCROLL_STATE_IDLE);
        mTabLayout->setScrollPosition(position, positionOffset, updateSelectedTabView, updateIndicator, false);
    }
}

void TabLayoutMediator::TabLayoutOnPageChangeCallback::doPageSelected(int position) {
    if (mTabLayout != nullptr && mTabLayout->getSelectedTabPosition() != position && position < mTabLayout->getTabCount()) {
        const bool updateIndicator = ((mScrollState == ViewPager2::SCROLL_STATE_IDLE)
                || (mScrollState == ViewPager2::SCROLL_STATE_SETTLING))
            && (mPreviousScrollState == ViewPager2::SCROLL_STATE_IDLE);
        mTabLayout->selectTab(mTabLayout->getTabAt(position), updateIndicator);
    }
}

void TabLayoutMediator::TabLayoutOnPageChangeCallback::reset() {
    mPreviousScrollState = mScrollState = ViewPager2::SCROLL_STATE_IDLE;
}

}/*endof namespace*/

