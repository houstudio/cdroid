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
#ifndef __RECYCLERVIEW_ACCESSIBILITY_DELEGATE_H__
#define __RECYCLERVIEW_ACCESSIBILITY_DELEGATE_H__
#include <view/view.h>
namespace cdroid{
class RecyclerViewAccessibilityDelegate:public View::AccessibilityDelegate{
protected:
    RecyclerView*mRecyclerView;
    bool shouldIgnore();
public:
    class ItemDelegate;
    RecyclerViewAccessibilityDelegate(RecyclerView* recyclerView);
    virtual ~RecyclerViewAccessibilityDelegate();
    bool performAccessibilityAction(View& host, int action, Bundle* args) override;

    void onInitializeAccessibilityNodeInfo(View& host, AccessibilityNodeInfo& info) override;

    void onInitializeAccessibilityEvent(View& host, AccessibilityEvent& event) override;

    /**
     * Gets the AccessibilityDelegate for an individual item in the RecyclerView.
     * A basic item delegate is provided by default, but you can override this
     * method to provide a custom per-item delegate.
     * For now, returning an {@code AccessibilityDelegateCompat} as opposed to an
     * {@code ItemDelegate} will prevent use of the {@code ViewCompat} accessibility API on
     * item views.
     */
    AccessibilityDelegate* getItemDelegate()const;
private:
    ItemDelegate* mItemDelegate;
};

/**
 * The default implementation of accessibility delegate for the individual items of the
 * RecyclerView.
 * <p>
 * If you are overriding {@code RecyclerViewAccessibilityDelegate#getItemDelegate()} but still
 * want to keep some default behavior, you can create an instance of this class and delegate to
 * the parent as necessary.
 */
class RecyclerViewAccessibilityDelegate::ItemDelegate:public View::AccessibilityDelegate{
    RecyclerViewAccessibilityDelegate* mRecyclerViewDelegate;
private:
    std::unordered_map<View*, AccessibilityDelegate*> mOriginalItemDelegates;

    /**
     * Creates an item delegate for the given {@code RecyclerViewAccessibilityDelegate}.
     *
     * @param recyclerViewDelegate The parent RecyclerView's accessibility delegate.
     */
    AccessibilityDelegate*getDelegateByHost(View&)const;
public:
    ItemDelegate(RecyclerViewAccessibilityDelegate* recyclerViewDelegate);

    /**
     * Saves a reference to the original delegate of the itemView so that it's behavior can be
     * combined with the ItemDelegate's behavior.
     */
    void saveOriginalDelegate(View* itemView);

    /**
     * @return The delegate associated with itemView before the view was bound.
     */
    AccessibilityDelegate* getAndRemoveOriginalDelegateForItem(View* itemView);

    void onInitializeAccessibilityNodeInfo(View& host, AccessibilityNodeInfo& info) override;
    bool performAccessibilityAction(View& host, int action, Bundle* args) override;
    void sendAccessibilityEvent(View& host, int eventType) override;
    void sendAccessibilityEventUnchecked(View& host,AccessibilityEvent& event) override;
    bool dispatchPopulateAccessibilityEvent(View& host,AccessibilityEvent& event) override;
    void onPopulateAccessibilityEvent(View& host, AccessibilityEvent& event) override;
    void onInitializeAccessibilityEvent(View& host, AccessibilityEvent& event) override;
    bool onRequestSendAccessibilityEvent(ViewGroup& host,View& child, AccessibilityEvent& event) override;
    AccessibilityNodeProvider* getAccessibilityNodeProvider(View& host)override;
};
};/*endof namespace*/
#endif/*__RECYCLERVIEW_ACCESSIBILITY_DELEGATE_H__*/
