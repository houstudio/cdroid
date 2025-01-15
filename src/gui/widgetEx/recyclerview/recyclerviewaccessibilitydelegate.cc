#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/recyclerview/recyclerviewaccessibilitydelegate.h>
namespace cdroid{
RecyclerViewAccessibilityDelegate::RecyclerViewAccessibilityDelegate(RecyclerView* recyclerView) {
    mRecyclerView = recyclerView;
    mItemDelegate = nullptr;
    AccessibilityDelegate* itemDelegate = getItemDelegate();
    if (itemDelegate && dynamic_cast<ItemDelegate*>(itemDelegate)) {
        mItemDelegate = (ItemDelegate*) itemDelegate;
    } else {
        mItemDelegate = new ItemDelegate(this);
    }
}

bool RecyclerViewAccessibilityDelegate::shouldIgnore() {
    return mRecyclerView->hasPendingAdapterUpdates();
}

bool RecyclerViewAccessibilityDelegate::performAccessibilityAction(View& host, int action, Bundle args) {
    if (AccessibilityDelegate::performAccessibilityAction(host, action, args)) {
        return true;
    }
    if (!shouldIgnore() && mRecyclerView->getLayoutManager() ) {
        return mRecyclerView->getLayoutManager()->performAccessibilityAction(action, args);
    }

    return false;
}

void RecyclerViewAccessibilityDelegate::onInitializeAccessibilityNodeInfo(View& host, AccessibilityNodeInfo& info) {
    AccessibilityDelegate::onInitializeAccessibilityNodeInfo(host, info);
    if (!shouldIgnore() && mRecyclerView->getLayoutManager() != nullptr) {
        mRecyclerView->getLayoutManager()->onInitializeAccessibilityNodeInfo(info);
    }
}

void RecyclerViewAccessibilityDelegate::onInitializeAccessibilityEvent(View& host, AccessibilityEvent& event) {
    AccessibilityDelegate::onInitializeAccessibilityEvent(host, event);
    if (dynamic_cast<RecyclerView*>(&host) && !shouldIgnore()) {
        RecyclerView* rv = (RecyclerView*) &host;
        if (rv->getLayoutManager() != nullptr) {
            rv->getLayoutManager()->onInitializeAccessibilityEvent(event);
        }
    }
}

/**
 * Gets the AccessibilityDelegate for an individual item in the RecyclerView.
 * A basic item delegate is provided by default, but you can override this
 * method to provide a custom per-item delegate.
 * For now, returning an {@code AccessibilityDelegateCompat} as opposed to an
 * {@code ItemDelegate} will prevent use of the {@code ViewCompat} accessibility API on
 * item views.
 */
View::AccessibilityDelegate* RecyclerViewAccessibilityDelegate::getItemDelegate() const{
    return mItemDelegate;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * The default implementation of accessibility delegate for the individual items of the
 * RecyclerView.
 * <p>
 * If you are overriding {@code RecyclerViewAccessibilityDelegate#getItemDelegate()} but still
 * want to keep some default behavior, you can create an instance of this class and delegate to
 * the parent as necessary.
 */

/**
 * Creates an item delegate for the given {@code RecyclerViewAccessibilityDelegate}.
 *
 * @param recyclerViewDelegate The parent RecyclerView's accessibility delegate.
 */
RecyclerViewAccessibilityDelegate::ItemDelegate::ItemDelegate(RecyclerViewAccessibilityDelegate* recyclerViewDelegate) {
    mRecyclerViewDelegate = recyclerViewDelegate;
}

/**
 * Saves a reference to the original delegate of the itemView so that it's behavior can be
 * combined with the ItemDelegate's behavior.
 */
void RecyclerViewAccessibilityDelegate::ItemDelegate::saveOriginalDelegate(View* itemView) {
    View::AccessibilityDelegate* delegate = itemView->getAccessibilityDelegate();
    if (delegate && delegate != this) {
        mOriginalItemDelegates.insert({itemView, delegate});
    }
}

/**
 * @return The delegate associated with itemView before the view was bound.
 */
View::AccessibilityDelegate* RecyclerViewAccessibilityDelegate::ItemDelegate::getAndRemoveOriginalDelegateForItem(View* itemView) {
    auto it = mOriginalItemDelegates.find(itemView);
    AccessibilityDelegate* delegate = nullptr;
    if(it!=mOriginalItemDelegates.end()){
        delegate = it->second;
        mOriginalItemDelegates.erase(it);
    }
    return delegate;
}

View::AccessibilityDelegate*RecyclerViewAccessibilityDelegate::ItemDelegate::getDelegateByHost(View&host)const{
    auto it = mOriginalItemDelegates.find(&host);
    if(it!=mOriginalItemDelegates.end())return it->second;
    return nullptr;
}

void RecyclerViewAccessibilityDelegate::ItemDelegate::onInitializeAccessibilityNodeInfo(View& host, AccessibilityNodeInfo& info) {
    if (!mRecyclerViewDelegate->shouldIgnore()
            && mRecyclerViewDelegate->mRecyclerView->getLayoutManager() != nullptr) {
        mRecyclerViewDelegate->mRecyclerView->getLayoutManager()
                ->onInitializeAccessibilityNodeInfoForItem(&host, info);
        AccessibilityDelegate* originalDelegate = getDelegateByHost(host);
        if (originalDelegate != nullptr) {
            originalDelegate->onInitializeAccessibilityNodeInfo(host, info);
        } else {
            AccessibilityDelegate::onInitializeAccessibilityNodeInfo(host, info);
        }
    } else {
        AccessibilityDelegate::onInitializeAccessibilityNodeInfo(host, info);
    }
}

bool RecyclerViewAccessibilityDelegate::ItemDelegate::performAccessibilityAction(View& host, int action, Bundle args) {
    if (!mRecyclerViewDelegate->shouldIgnore()
            && mRecyclerViewDelegate->mRecyclerView->getLayoutManager() != nullptr) {
        View::AccessibilityDelegate* originalDelegate = getDelegateByHost(host);
        if (originalDelegate != nullptr) {
            if (originalDelegate->performAccessibilityAction(host, action, args)) {
                return true;
            }
        } else if (AccessibilityDelegate::performAccessibilityAction(host, action, args)) {
            return true;
        }
        return mRecyclerViewDelegate->mRecyclerView->getLayoutManager()
                ->performAccessibilityActionForItem(host, action, args);
    } else {
        return AccessibilityDelegate::performAccessibilityAction(host, action, args);
    }
}

void RecyclerViewAccessibilityDelegate::ItemDelegate::sendAccessibilityEvent(View& host, int eventType) {
    View::AccessibilityDelegate* originalDelegate = getDelegateByHost(host);
    if (originalDelegate != nullptr) {
        originalDelegate->sendAccessibilityEvent(host, eventType);
    } else {
        AccessibilityDelegate::sendAccessibilityEvent(host, eventType);
    }
}

void RecyclerViewAccessibilityDelegate::ItemDelegate::sendAccessibilityEventUnchecked(View& host,AccessibilityEvent& event) {
    View::AccessibilityDelegate* originalDelegate = getDelegateByHost(host);
    if (originalDelegate != nullptr) {
        originalDelegate->sendAccessibilityEventUnchecked(host, event);
    } else {
        View::AccessibilityDelegate::sendAccessibilityEventUnchecked(host, event);
    }
}

bool RecyclerViewAccessibilityDelegate::ItemDelegate::dispatchPopulateAccessibilityEvent(View& host,AccessibilityEvent& event) {
    View::AccessibilityDelegate* originalDelegate = getDelegateByHost(host);
    if (originalDelegate != nullptr) {
        return originalDelegate->dispatchPopulateAccessibilityEvent(host, event);
    } else {
        return View::AccessibilityDelegate::dispatchPopulateAccessibilityEvent(host, event);
    }
}

void RecyclerViewAccessibilityDelegate::ItemDelegate::onPopulateAccessibilityEvent(View& host, AccessibilityEvent& event) {
    AccessibilityDelegate* originalDelegate = getDelegateByHost(host);
    if (originalDelegate != nullptr) {
        originalDelegate->onPopulateAccessibilityEvent(host, event);
    } else {
        AccessibilityDelegate::onPopulateAccessibilityEvent(host, event);
    }
}

void RecyclerViewAccessibilityDelegate::ItemDelegate::onInitializeAccessibilityEvent(View& host, AccessibilityEvent& event) {
    View::AccessibilityDelegate* originalDelegate = getDelegateByHost(host);
    if (originalDelegate != nullptr) {
        originalDelegate->onInitializeAccessibilityEvent(host, event);
    } else {
        AccessibilityDelegate::onInitializeAccessibilityEvent(host, event);
    }
}

bool RecyclerViewAccessibilityDelegate::ItemDelegate::onRequestSendAccessibilityEvent(ViewGroup& host,View& child, AccessibilityEvent& event) {
    View::AccessibilityDelegate* originalDelegate = getDelegateByHost(host);
    if (originalDelegate != nullptr) {
        return originalDelegate->onRequestSendAccessibilityEvent(host, child, event);
    } else {
        return AccessibilityDelegate::onRequestSendAccessibilityEvent(host, child, event);
    }
}

AccessibilityNodeProvider* RecyclerViewAccessibilityDelegate::ItemDelegate::getAccessibilityNodeProvider(View& host) {
    View::AccessibilityDelegate* originalDelegate = getDelegateByHost(host);
    if (originalDelegate != nullptr) {
        return originalDelegate->getAccessibilityNodeProvider(host);
    } else {
        return AccessibilityDelegate::getAccessibilityNodeProvider(host);
    }
}

}/*endof namespace*/

