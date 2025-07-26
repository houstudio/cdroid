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
#include <view/viewgroup.h>
#include <widget/explorebytouchhelper.h>

namespace cdroid{
ExploreByTouchHelper::ExploreByTouchHelper(View* forView) {
    if (forView == nullptr) {
        throw std::runtime_error("View may not be null");
    }

    mView = forView;
    mContext = forView->getContext();
    mManager = &AccessibilityManager::getInstance(mContext);
}

AccessibilityNodeProvider* ExploreByTouchHelper::getAccessibilityNodeProvider(View& host) {
    if (mNodeProvider == nullptr) {
        mNodeProvider = new ExploreByTouchNodeProvider(this);
    }
    return mNodeProvider;
}

bool ExploreByTouchHelper::dispatchHoverEvent(MotionEvent& event) {
    if (!mManager->isEnabled() || !mManager->isTouchExplorationEnabled()) {
        return false;
    }
    int virtualViewId;
    switch (event.getAction()) {
    case MotionEvent::ACTION_HOVER_MOVE:
    case MotionEvent::ACTION_HOVER_ENTER:
        virtualViewId = getVirtualViewAt(event.getX(), event.getY());
        updateHoveredVirtualView(virtualViewId);
        return (virtualViewId != INVALID_ID);
    case MotionEvent::ACTION_HOVER_EXIT:
        if (mFocusedVirtualViewId != INVALID_ID) {
            updateHoveredVirtualView(INVALID_ID);
            return true;
        }
        return false;
    default:
        return false;
    }
}

bool ExploreByTouchHelper::sendEventForVirtualView(int virtualViewId, int eventType) {
    if ((virtualViewId == INVALID_ID) || !mManager->isEnabled()) {
        return false;
    }

    ViewGroup* parent = mView->getParent();
    if (parent == nullptr) {
        return false;
    }

    //AccessibilityEvent* event = createEvent(virtualViewId, eventType);
    //return parent->requestSendAccessibilityEvent(mView, *event);
    return false;
}

void ExploreByTouchHelper::invalidateRoot() {
    invalidateVirtualView(HOST_ID, AccessibilityEvent::CONTENT_CHANGE_TYPE_SUBTREE);
}

void ExploreByTouchHelper::invalidateVirtualView(int virtualViewId) {
    invalidateVirtualView(virtualViewId, AccessibilityEvent::CONTENT_CHANGE_TYPE_UNDEFINED);
}

void ExploreByTouchHelper::invalidateVirtualView(int virtualViewId, int changeTypes) {
    if (virtualViewId != INVALID_ID && mManager->isEnabled()) {
        ViewGroup* parent = mView->getParent();
        if (parent != nullptr) {
            AccessibilityEvent* event = createEvent(virtualViewId,
                    AccessibilityEvent::TYPE_WINDOW_CONTENT_CHANGED);
            event->setContentChangeTypes(changeTypes);
            //parent->requestSendAccessibilityEvent(mView, *event);
        }
    }
}

int ExploreByTouchHelper::getFocusedVirtualView() {
    return mFocusedVirtualViewId;
}

void ExploreByTouchHelper::updateHoveredVirtualView(int virtualViewId) {
    if (mHoveredVirtualViewId == virtualViewId) {
        return;
    }

    const int previousVirtualViewId = mHoveredVirtualViewId;
    mHoveredVirtualViewId = virtualViewId;

    // Stay consistent with framework behavior by sending ENTER/EXIT pairs
    // in reverse order. This is accurate as of API 18.
    sendEventForVirtualView(virtualViewId, AccessibilityEvent::TYPE_VIEW_HOVER_ENTER);
    sendEventForVirtualView(previousVirtualViewId, AccessibilityEvent::TYPE_VIEW_HOVER_EXIT);
}

AccessibilityEvent* ExploreByTouchHelper::createEvent(int virtualViewId, int eventType) {
    switch (virtualViewId) {
    case HOST_ID:return createEventForHost(eventType);
    default: return createEventForChild(virtualViewId, eventType);
    }
}

AccessibilityEvent* ExploreByTouchHelper::createEventForHost(int eventType) {
    AccessibilityEvent* event = AccessibilityEvent::obtain(eventType);
    mView->onInitializeAccessibilityEvent(*event);

    // Allow the client to populate the event.
    onPopulateEventForHost(*event);

    return event;
}
#define DEFAULT_CLASS_NAME "View"
AccessibilityEvent* ExploreByTouchHelper::createEventForChild(int virtualViewId, int eventType) {
    AccessibilityEvent* event = AccessibilityEvent::obtain(eventType);
    event->setEnabled(true);
    event->setClassName(DEFAULT_CLASS_NAME);

    // Allow the client to populate the event.
    onPopulateEventForVirtualView(virtualViewId, *event);

    // Make sure the developer is following the rules.
    if (event->getText().empty() && event->getContentDescription().empty()) {
        throw std::runtime_error("Callbacks must add text or a content description in populateEventForVirtualViewId()");
    }

    // Don't allow the client to override these properties.
    event->setPackageName(mView->getContext()->getPackageName());
    event->setSource(mView, virtualViewId);

    return event;
}

AccessibilityNodeInfo* ExploreByTouchHelper::createNode(int virtualViewId) {
    switch (virtualViewId) {
    case HOST_ID: return createNodeForHost();
    default:  return createNodeForChild(virtualViewId);
    }
}

/**
 * Constructs and returns an {@link AccessibilityNodeInfo} for the
 * host view populated with its virtual descendants.
 *
 * @return An {@link AccessibilityNodeInfo} for the parent node.
 */
AccessibilityNodeInfo* ExploreByTouchHelper::createNodeForHost() {
    AccessibilityNodeInfo* node = AccessibilityNodeInfo::obtain(mView);
    mView->onInitializeAccessibilityNodeInfo(*node);
    const int realNodeCount = node->getChildCount();

    // Allow the client to populate the host node.
    onPopulateNodeForHost(*node);

    std::vector<int> virtualViewIds;// = mTempArray;
    getVisibleVirtualViews(virtualViewIds);
    if (realNodeCount > 0 && virtualViewIds.size() > 0) {
        throw std::logic_error("Views cannot have both real and virtual children");
    }

    const int N = virtualViewIds.size();
    for (int i = 0; i < N; i++) {
        node->addChild(mView, virtualViewIds.at(i));
    }

    return node;
}

AccessibilityNodeInfo* ExploreByTouchHelper::createNodeForChild(int virtualViewId) {
    //ensureTempRects();
    Rect tempParentRect = mTempParentRect;
    int tempGlobalRect[2];// = mTempGlobalRect;
    Rect tempScreenRect = mTempScreenRect;

    AccessibilityNodeInfo* node = AccessibilityNodeInfo::obtain();

    // Ensure the client has good defaults.
    node->setEnabled(true);
    node->setClassName(DEFAULT_CLASS_NAME);
    Rect INVALID_PARENT_BOUNDS={INT_MAX,INT_MAX,INT_MIN,INT_MIN};
    node->setBoundsInParent(INVALID_PARENT_BOUNDS);

    // Allow the client to populate the node.
    onPopulateNodeForVirtualView(virtualViewId,*node);

    // Make sure the developer is following the rules.
    if (node->getText().empty() && node->getContentDescription().empty()) {
        throw std::runtime_error("Callbacks must add text or a content description in populateNodeForVirtualViewId()");
    }

    node->getBoundsInParent(tempParentRect);
    if (tempParentRect==INVALID_PARENT_BOUNDS) {
        throw std::runtime_error("Callbacks must set parent bounds in populateNodeForVirtualViewId()");
    }

    const int actions = node->getActions();
    if ((actions & AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS) != 0) {
        throw std::runtime_error("Callbacks must not add ACTION_ACCESSIBILITY_FOCUS in populateNodeForVirtualViewId()");
    }
    if ((actions & AccessibilityNodeInfo::ACTION_CLEAR_ACCESSIBILITY_FOCUS) != 0) {
        throw std::runtime_error("Callbacks must not add ACTION_CLEAR_ACCESSIBILITY_FOCUS in populateNodeForVirtualViewId()");
    }

    // Don't allow the client to override these properties.
    node->setPackageName(mView->getContext()->getPackageName());
    node->setSource(mView, virtualViewId);
    node->setParent(mView);

    // Manage internal accessibility focus state.
    if (mFocusedVirtualViewId == virtualViewId) {
        node->setAccessibilityFocused(true);
        node->addAction(AccessibilityNodeInfo::AccessibilityAction::ACTION_CLEAR_ACCESSIBILITY_FOCUS.getId());
    } else {
        node->setAccessibilityFocused(false);
        node->addAction(AccessibilityNodeInfo::AccessibilityAction::ACTION_ACCESSIBILITY_FOCUS.getId());
    }

    // Set the visibility based on the parent bound.
    if (intersectVisibleToUser(&tempParentRect)) {
        node->setVisibleToUser(true);
        node->setBoundsInParent(tempParentRect);
    }

    // Calculate screen-relative bound.
    mView->getLocationOnScreen(tempGlobalRect);
    const int offsetX = tempGlobalRect[0];
    const int offsetY = tempGlobalRect[1];
    tempScreenRect = tempParentRect;//set(tempParentRect);
    tempScreenRect.offset(offsetX, offsetY);
    node->setBoundsInScreen(tempScreenRect);

    return node;
}

/*void ExploreByTouchHelper::ensureTempRects() {
    mTempGlobalRect = new int[2];
    mTempParentRect = new Rect();
    mTempScreenRect = new Rect();
}*/

bool ExploreByTouchHelper::performAction(int virtualViewId, int action, Bundle* arguments) {
    switch (virtualViewId) {
        case HOST_ID:
            return performActionForHost(action, arguments);
        default:
            return performActionForChild(virtualViewId, action, arguments);
    }
}

bool ExploreByTouchHelper::performActionForHost(int action, Bundle* arguments) {
    return mView->performAccessibilityAction(action, arguments);
}

bool ExploreByTouchHelper::performActionForChild(int virtualViewId, int action, Bundle* arguments) {
    switch (action) {
    case AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS:
    case AccessibilityNodeInfo::ACTION_CLEAR_ACCESSIBILITY_FOCUS:
        return manageFocusForChild(virtualViewId, action);
    default:
        return onPerformActionForVirtualView(virtualViewId, action, arguments);
    }
}

bool ExploreByTouchHelper::manageFocusForChild(int virtualViewId, int action) {
    switch (action) {
    case AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS:
        return requestAccessibilityFocus(virtualViewId);
    case AccessibilityNodeInfo::ACTION_CLEAR_ACCESSIBILITY_FOCUS:
        return clearAccessibilityFocus(virtualViewId);
    default:  return false;
    }
}

bool ExploreByTouchHelper::intersectVisibleToUser(Rect* localRect) {
    // Missing or empty bounds mean this view is not visible.
    if ((localRect == nullptr) || localRect->empty()) {
        return false;
    }

    // Attached to invisible window means this view is not visible.
    if (mView->getWindowVisibility() != View::VISIBLE) {
        return false;
    }

    // An invisible predecessor means that this view is not visible.
    ViewGroup* viewParent = mView->getParent();
    while (viewParent) {
        View* view = (View*) viewParent;
        if ((view->getAlpha() <= 0) || (view->getVisibility() != View::VISIBLE)) {
            return false;
        }
        viewParent = view->getParent();
    }

    // A null parent implies the view is not visible.
    if (viewParent == nullptr) {
        return false;
    }

    Rect tempVisibleRect;// = mTempVisibleRect;
    if (!mView->getLocalVisibleRect(tempVisibleRect)) {
        return false;
    }

    // Check if the view intersects the visible portion of the parent.
    return localRect->intersect(tempVisibleRect);
}

/**
 * Returns whether this virtual view is accessibility focused.
 *
 * @return True if the view is accessibility focused.
 */
bool ExploreByTouchHelper::isAccessibilityFocused(int virtualViewId) {
    return (mFocusedVirtualViewId == virtualViewId);
}

bool ExploreByTouchHelper::requestAccessibilityFocus(int virtualViewId) {
    AccessibilityManager* accessibilityManager =&AccessibilityManager::getInstance(mContext);//.getSystemService(Context.ACCESSIBILITY_SERVICE);

    if (!mManager->isEnabled()
            || !accessibilityManager->isTouchExplorationEnabled()) {
        return false;
    }
    // TODO: Check virtual view visibility.
    if (!isAccessibilityFocused(virtualViewId)) {
        // Clear focus from the previously focused view, if applicable.
        if (mFocusedVirtualViewId != INVALID_ID) {
            sendEventForVirtualView(mFocusedVirtualViewId, AccessibilityEvent::TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED);
        }

        // Set focus on the new view.
        mFocusedVirtualViewId = virtualViewId;

        // TODO: Only invalidate virtual view bounds.
        mView->invalidate();
        sendEventForVirtualView(virtualViewId, AccessibilityEvent::TYPE_VIEW_ACCESSIBILITY_FOCUSED);
        return true;
    }
    return false;
}

bool ExploreByTouchHelper::clearAccessibilityFocus(int virtualViewId) {
    if (isAccessibilityFocused(virtualViewId)) {
        mFocusedVirtualViewId = INVALID_ID;
        mView->invalidate();
        sendEventForVirtualView(virtualViewId,AccessibilityEvent::TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED);
        return true;
    }
    return false;
}

void ExploreByTouchHelper::onPopulateEventForHost(AccessibilityEvent& event) {
    // Default implementation is no-op.
}

void ExploreByTouchHelper::onPopulateNodeForHost(AccessibilityNodeInfo& node) {
    // Default implementation is no-op.
}

}/*endof namespace*/

