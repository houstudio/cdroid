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
#ifndef __MULTI_PAGE_UI_H__
#define __MULTI_PAGE_UI_H__
#include <widgetEx/wear/multipagepresenter.h>
#include <widget/adapter.h>
namespace cdroid{

class ViewPager;
class PageIndicatorView;
class WearableNavigationDrawerView;
class WearableNavigationDrawerAdapter;

/** Handles view logic for the multi page style WearableNavigationDrawerView.
 *  androidx.wear.internal.widget.drawer.MultiPageUi.java (lines 43-171). */
class MultiPageUi : public MultiPagePresenter::Ui {
private:
    /** Borrowed — set by MultiPagePresenter's constructor via initialize. */
    WearableNavigationDrawerPresenter* mPresenter = nullptr;
    /** Borrowed (inflated children of the drawer content, owned by the drawer view). */
    ViewPager* mNavigationPager = nullptr;
    PageIndicatorView* mPageIndicatorView = nullptr;

public:
    /** Adapter for ViewPager used in the multi-page UI. (lines 125-170;
        private static final class upstream) */
    class NavigationPagerAdapter : public PagerAdapter {
    private:
        /** Borrowed — the app owns the WearableNavigationDrawerAdapter. */
        WearableNavigationDrawerAdapter* mAdapter;

    public:
        explicit NavigationPagerAdapter(WearableNavigationDrawerAdapter* adapter);

        void* instantiateItem(ViewGroup* container, int position) override;
        void destroyItem(ViewGroup* container, int position, void* object) override;
        int getCount() override;
        int getItemPosition(void* object) override;
        bool isViewFromObject(View* view, void* object) override;
    };

    MultiPageUi() = default;
    ~MultiPageUi() override;

    void initialize(WearableNavigationDrawerView* drawer,
            WearableNavigationDrawerPresenter* presenter) override;
    void setNavigationPagerAdapter(WearableNavigationDrawerAdapter* adapter) override;
    void notifyPageIndicatorDataChanged() override;
    void notifyNavigationPagerAdapterDataChanged() override;
    void setNavigationPagerSelectedItem(int index, bool smoothScrollTo) override;

private:
    /** Owned — CDROID's ViewPager borrows its PagerAdapter (upstream relies on
        GC); replaced (and deleted) on each setNavigationPagerAdapter call. */
    NavigationPagerAdapter* mNavigationPagerAdapter = nullptr;
};

}/*endof namespace*/
#endif/*__MULTI_PAGE_UI_H__*/
