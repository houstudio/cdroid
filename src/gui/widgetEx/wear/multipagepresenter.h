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
#ifndef __MULTI_PAGE_PRESENTER_H__
#define __MULTI_PAGE_PRESENTER_H__
#include <widgetEx/wear/wearablenavigationdrawerpresenter.h>
namespace cdroid{

class WearableNavigationDrawerView;
class WearableNavigationDrawerAdapter;

/** Provides a WearableNavigationDrawerPresenter implementation that is designed
 *  for the multi-page navigation drawer.
 *  androidx.wear.internal.widget.drawer.MultiPagePresenter.java (lines 32-125). */
class MultiPagePresenter : public WearableNavigationDrawerPresenter {
public:
    /** Controls the user interface of a multi-page WearableNavigationDrawerView.
        (lines 42-69) */
    class Ui {
    public:
        virtual ~Ui() = default;

        /** Initializes the Ui. */
        virtual void initialize(WearableNavigationDrawerView* drawer,
                WearableNavigationDrawerPresenter* presenter) = 0;

        /** Should notify the NavigationPagerAdapter that the underlying data has changed. */
        virtual void notifyNavigationPagerAdapterDataChanged() = 0;

        /** Should notify the Page Indicator that the underlying data has changed. */
        virtual void notifyPageIndicatorDataChanged() = 0;

        /** Associates the given {@code adapter} with this Ui. */
        virtual void setNavigationPagerAdapter(WearableNavigationDrawerAdapter* adapter) = 0;

        /** Sets which item is selected and optionally smooth scrolls to it. */
        virtual void setNavigationPagerSelectedItem(int index, bool smoothScrollTo) = 0;
    };

private:
    /** Owned (upstream holds a final reference; the presenter outlives it). */
    Ui* mUi;
    /** Borrowed — the view owns the presenter. */
    WearableNavigationDrawerView* mDrawer;
    const bool mIsAccessibilityEnabled;
    /** @Nullable upstream (null until onNewAdapter). Borrowed — the app owns the adapter. */
    WearableNavigationDrawerAdapter* mAdapter = nullptr;

public:
    /** @throw std::invalid_argument if drawer or ui is null
        (upstream IllegalArgumentException). */
    MultiPagePresenter(WearableNavigationDrawerView* drawer, Ui* ui, bool isAccessibilityEnabled);
    ~MultiPagePresenter() override;

    void onDataSetChanged() override;
    void onNewAdapter(WearableNavigationDrawerAdapter* adapter) override;
    void onSelected(int index) override;
    void onSetCurrentItemRequested(int index, bool smoothScrollTo) override;
    bool onDrawerTapped() override;
};

}/*endof namespace*/
#endif/*__MULTI_PAGE_PRESENTER_H__*/
