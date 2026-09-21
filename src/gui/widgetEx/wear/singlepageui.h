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
#ifndef __SINGLE_PAGE_UI_H__
#define __SINGLE_PAGE_UI_H__
#include <widgetEx/wear/singlepagepresenter.h>
#include <core/callbackbase.h>
#include <string>
#include <vector>
namespace cdroid{

class CircledImageView;
class TextView;
class WearableNavigationDrawerView;
class Drawable;

/** Handles view logic for the single page style WearableNavigationDrawerView.
 *  androidx.wear.internal.widget.drawer.SinglePageUi.java (lines 43-183). */
class SinglePageUi : public SinglePagePresenter::Ui {
private:
    /** Borrowed — the drawer view owns its presenter, which owns this Ui. */
    WearableNavigationDrawerView* mDrawer;
    /** Borrowed — set by SinglePagePresenter's constructor via setPresenter. */
    WearableNavigationDrawerPresenter* mPresenter = nullptr;
    /** Inflated children of the drawer content; borrowed (the drawer view owns them). */
    std::vector<CircledImageView*> mSinglePageImageViews;
    /** Indicates currently selected item. {@code null} when the layout lacks space to
        display it. Borrowed (child of the drawer content). */
    TextView* mTextView = nullptr;
    /** Upstream posts this on a main-looper Handler; CDROID posts it through the
        drawer view (View::postDelayed runs on the main looper too). */
    Runnable mCloseDrawerRunnable;

public:
    /** @throw std::invalid_argument if navigationDrawer is null
        (upstream IllegalArgumentException). */
    explicit SinglePageUi(WearableNavigationDrawerView* navigationDrawer);
    ~SinglePageUi() override;

    void setPresenter(WearableNavigationDrawerPresenter* presenter) override;
    void initialize(int count) override;
    void setIcon(int index, Drawable* drawable, const std::string& contentDescription) override;
    void setText(const std::string& itemText, bool showToastIfNoTextView) override;
    void selectItem(int index) override;
    void deselectItem(int index) override;
    void closeDrawerDelayed(long delayMs) override;
    void peekDrawer() override;
};

}/*endof namespace*/
#endif/*__SINGLE_PAGE_UI_H__*/
