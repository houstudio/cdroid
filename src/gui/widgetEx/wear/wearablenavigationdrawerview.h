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
#ifndef __WEARABLE_NAVIGATION_DRAWER_VIEW_H__
#define __WEARABLE_NAVIGATION_DRAWER_VIEW_H__
#include <widgetEx/wear/wearabledrawerview.h>
#include <widgetEx/wear/wearablenavigationdrawerpresenter.h>
#include <view/gesturedetector.h>
namespace cdroid{

class WearableNavigationDrawerAdapter;

/** Ease of use class for creating a Wearable navigation drawer. This can be used
 *  with WearableDrawerLayout to create a drawer for users to easily navigate a
 *  wearable app.
 *  There are two ways this information may be presented: as a single page and as
 *  multiple pages. The single page navigation drawer will display 1-7 items to
 *  the user representing different navigation verticals. If more than 7 items
 *  are provided to a single page navigation drawer, the navigation drawer will
 *  be displayed as empty. The multiple page navigation drawer will display 1 or
 *  more pages to the user, each representing different navigation verticals.
 *  androidx.wear.widget.drawer.WearableNavigationDrawerView.java (lines 62-292). */
class WearableNavigationDrawerView : public WearableDrawerView {
public:
    /** Listener which is notified when the user selects an item. (lines 69-75)
        Upstream interface with a single onItemSelected(int pos); the CDROID
        listener convention maps it onto the namespace-scope CallbackBase functor
        declared in wearablenavigationdrawerpresenter.h (breaks the header cycle). */
    using OnItemSelectedListener = cdroid::OnItemSelectedListener;

    /** Adapter for specifying the contents of WearableNavigationDrawer. (lines
        250-290) Upstream abstract inner class, ported as the top-level
        WearableNavigationDrawerAdapter (wearablenavigationdraweradapter.h). */
    using WearableNavigationDrawerAdapter = cdroid::WearableNavigationDrawerAdapter;

    /** Single page navigation drawer style. This is the default drawer style. It
        is ideal for 1-5 items, but works with up to 7 items. If more than 7 items
        exist, then the drawer will be displayed as empty. */
    static constexpr int SINGLE_PAGE = 0;

    /** Multi-page navigation drawer style. Each item is on its own page. Useful
        when more than 7 items exist. */
    static constexpr int MULTI_PAGE = 1;

private:
    static constexpr int DEFAULT_STYLE = SINGLE_PAGE;
    static constexpr long AUTO_CLOSE_DRAWER_DELAY_MS = 5000; // TimeUnit.SECONDS.toMillis(5)

    const bool mIsAccessibilityEnabled;
    /** Owned — upstream keeps the presenter as a final field (GC'd with the view). */
    WearableNavigationDrawerPresenter* mPresenter = nullptr;
    // Java keeps this final; it is assigned once in the constructor body (after
    // the styled-attribute read), so it cannot be const here.
    int mNavigationStyle = DEFAULT_STYLE;
    /** Listens for single taps on the drawer. (lines 112-121) */
    GestureDetector* mGestureDetector = nullptr;
    GestureDetector::OnGestureListener mOnGestureListener;
    /** Upstream posts this on a main-looper Handler; CDROID posts it through
        View::postDelayed/removeCallbacks (also the main looper). */
    Runnable mCloseDrawerRunnable;

    void autoCloseDrawerAfterDelay();

public:
    WearableNavigationDrawerView(Context* context);
    WearableNavigationDrawerView(Context* context, const AttributeSet* attrs);
    WearableNavigationDrawerView(Context* context, const AttributeSet* attrs, int defStyleAttr);
    WearableNavigationDrawerView(Context* context, const AttributeSet* attrs, int defStyleAttr,
            int defStyleRes);
    ~WearableNavigationDrawerView() override;

    /** Set a WearableNavigationDrawerAdapter that will supply data for this drawer. */
    void setAdapter(WearableNavigationDrawerAdapter* adapter);

    /** Add an OnItemSelectedListener that will be notified when the user selects an item. */
    void addOnItemSelectedListener(const OnItemSelectedListener& listener);

    /** Remove an OnItemSelectedListener. */
    void removeOnItemSelectedListener(const OnItemSelectedListener& listener);

    /** Changes which index is selected. OnItemSelectedListener#onItemSelected will
        be called when the specified {@code index} is reached, but it won't be called
        for items between the current index and the destination index. */
    void setCurrentItem(int index, bool smoothScrollTo);

    /** Returns the style this drawer is using, either SINGLE_PAGE or MULTI_PAGE. */
    int getNavigationStyle() const;

    bool onInterceptTouchEvent(MotionEvent& ev) override;

    bool canScrollHorizontally(int direction) override;

    void onDrawerOpened() override;

    void onDrawerClosed() override;

    // package-private upstream; WearableDrawerLayout queries it
    int preferGravity() const override;
};

}/*endof namespace*/
#endif/*__WEARABLE_NAVIGATION_DRAWER_VIEW_H__*/
