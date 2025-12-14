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
#ifndef __TABLAYOUT_MEDIATOR_H__
#define __TABLAYOUT_MEDIATOR_H__
#include <widget/tablayout.h>
#include <widgetEx/viewpager2.h>
#include <widgetEx/recyclerview/recyclerview.h>
namespace cdroid{
class TabLayoutMediator {
public:
    DECLARE_UIEVENT(void,TabConfigurationStrategy,TabLayout::Tab&,int);//void onConfigureTab(TabLayout::Tab& var1, int var2);
private:
    class PagerAdapterObserver;
    class TabLayoutOnPageChangeCallback;
    TabLayout* mTabLayout;
    ViewPager2* mViewPager;
    bool mAutoRefresh;
    bool mSmoothScroll;
    bool mAttached;
    TabConfigurationStrategy mTabConfigurationStrategy;
    RecyclerView::Adapter* mAdapter;
    TabLayoutOnPageChangeCallback* mOnPageChangeCallback;
    TabLayout::OnTabSelectedListener mOnTabSelectedListener;
    RecyclerView::AdapterDataObserver* mPagerAdapterObserver;
public:
    TabLayoutMediator(TabLayout* tabLayout, ViewPager2* viewPager, const TabConfigurationStrategy& tabConfigurationStrategy);
    TabLayoutMediator(TabLayout* tabLayout, ViewPager2* viewPager, bool autoRefresh, const TabConfigurationStrategy& tabConfigurationStrategy);
    TabLayoutMediator(TabLayout* tabLayout, ViewPager2* viewPager, bool autoRefresh, bool smoothScroll, const TabConfigurationStrategy& tabConfigurationStrategy);
    virtual ~TabLayoutMediator();
    void attach();
    void detach();
    bool isAttached() const{
        return mAttached;
    }

    void populateTabsFromPagerAdapter();
};

class TabLayoutMediator::TabLayoutOnPageChangeCallback :public ViewPager2::OnPageChangeCallback {
private:
    TabLayout* mTabLayout;
    int mPreviousScrollState;
    int mScrollState;
protected:
    void doPageScrollStateChanged(int state);
    void doPageScrolled(int position, float positionOffset, int positionOffsetPixels);
    void doPageSelected(int position);
public:
    TabLayoutOnPageChangeCallback(TabLayout* tabLayout);
    void reset();
};

class TabLayoutMediator::PagerAdapterObserver :public RecyclerView::AdapterDataObserver {
    TabLayoutMediator*mTabLayoutMediator;
public:
    PagerAdapterObserver(TabLayoutMediator*lm):mTabLayoutMediator(lm) {
    }

    void onChanged() override{
        mTabLayoutMediator->populateTabsFromPagerAdapter();
    }

    void onItemRangeChanged(int positionStart, int itemCount) override{
        mTabLayoutMediator->populateTabsFromPagerAdapter();
    }

    void onItemRangeChanged(int positionStart, int itemCount,Object* payload) override{
        mTabLayoutMediator->populateTabsFromPagerAdapter();
    }

    void onItemRangeInserted(int positionStart, int itemCount) override{
        mTabLayoutMediator->populateTabsFromPagerAdapter();
    }

    void onItemRangeRemoved(int positionStart, int itemCount) override{
        mTabLayoutMediator->populateTabsFromPagerAdapter();
    }

    void onItemRangeMoved(int fromPosition, int toPosition, int itemCount) override{
        mTabLayoutMediator->populateTabsFromPagerAdapter();
    }
};
}/*endof namespace*/
#endif/*__TABLAYOUT_MEDIATOR_H__*/

