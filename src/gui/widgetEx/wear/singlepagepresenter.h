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
#ifndef __SINGLE_PAGE_PRESENTER_H__
#define __SINGLE_PAGE_PRESENTER_H__
#include <widgetEx/wear/wearablenavigationdrawerpresenter.h>
#include <string>
namespace cdroid{

class WearableNavigationDrawerView;
class WearableNavigationDrawerAdapter;
class Drawable;

/** Provides a WearableNavigationDrawerPresenter implementation that is designed
 *  for the single page navigation drawer.
 *  androidx.wear.internal.widget.drawer.SinglePagePresenter.java (lines 34-167). */
class SinglePagePresenter : public WearableNavigationDrawerPresenter {
public:
    /** Controls the user interface of a single-page WearableNavigationDrawerView.
        (lines 47-90) */
    class Ui {
    public:
        virtual ~Ui() = default;

        /** Associates a WearableNavigationDrawerPresenter with this Ui. */
        virtual void setPresenter(WearableNavigationDrawerPresenter* presenter) = 0;

        /** Initializes the Ui with {@code count} items. */
        virtual void initialize(int count) = 0;

        /** Sets the item's Drawable icon and its contentDescription. */
        virtual void setIcon(int index, Drawable* drawable, const std::string& contentDescription) = 0;

        /** Displays {@code itemText} in a TextView used to indicate which item is selected.
            When the Ui doesn't have space, it should show a Toast if
            {@code showToastIfNoTextView} is {@code true}. */
        virtual void setText(const std::string& itemText, bool showToastIfNoTextView) = 0;

        /** Indicates that the item at {@code index} has been selected. */
        virtual void selectItem(int index) = 0;

        /** Removes the indication that the item at {@code index} has been selected. */
        virtual void deselectItem(int index) = 0;

        /** Closes the drawer after the given delay. */
        virtual void closeDrawerDelayed(long delayMs) = 0;

        /** Peeks the WearableNavigationDrawerView. */
        virtual void peekDrawer() = 0;
    };

private:
    static constexpr long DRAWER_CLOSE_DELAY_MS = 500;

    /** Owned (upstream holds a final reference; the presenter outlives it). */
    Ui* mUi;
    const bool mIsAccessibilityEnabled;
    /** @Nullable upstream (null until onNewAdapter). Borrowed — the app owns the adapter. */
    WearableNavigationDrawerAdapter* mAdapter = nullptr;
    int mCount = 0;
    int mSelected = 0;

public:
    /** @throw std::invalid_argument if ui is null (upstream IllegalArgumentException). */
    SinglePagePresenter(Ui* ui, bool isAccessibilityEnabled);
    ~SinglePagePresenter() override;

    void onDataSetChanged() override;
    void onNewAdapter(WearableNavigationDrawerAdapter* adapter) override;
    void onSelected(int index) override;
    void onSetCurrentItemRequested(int index, bool smoothScrollTo) override;
    bool onDrawerTapped() override;
};

}/*endof namespace*/
#endif/*__SINGLE_PAGE_PRESENTER_H__*/
