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
#ifndef __EXPLOREBY_TOUCH_HELPER_H__
#define __EXPLOREBY_TOUCH_HELPER_H__
#include <view/view.h>
namespace cdroid{
class ExploreByTouchHelper:public View::AccessibilityDelegate {
    /** Virtual node identifier value for invalid nodes. */
public:
    static constexpr int INVALID_ID = INT_MIN;

    /** Virtual node identifier value for the host view's node. */
    static constexpr int HOST_ID = View::NO_ID;
private:
    class ExploreByTouchNodeProvider;
    /** Default class name used for virtual views. */
    //static std::string DEFAULT_CLASS_NAME = View.class.getName();

    /** Default bounds used to determine if the client didn't set any. */
    //static final Rect INVALID_PARENT_BOUNDS = new Rect(Integer.MAX_VALUE, Integer.MAX_VALUE, Integer.MIN_VALUE, Integer.MIN_VALUE);

    // Lazily-created temporary data structures used when creating nodes.
    Rect mTempScreenRect;
    Rect mTempParentRect;
    int mTempGlobalRect[2];

    /** Lazily-created temporary data structure used to compute visibility. */
    Rect mTempVisibleRect;

    /** Lazily-created temporary data structure used to obtain child IDs. */
    std::vector<int> mTempArray;

    /** System accessibility manager, used to check state and send events. */
    AccessibilityManager* mManager;

    /** View whose internal structure is exposed through this helper. */
    View* mView;

    /** Context of the host view. **/
    Context* mContext;

    /** Node provider that handles creating nodes and performing actions. */
    ExploreByTouchNodeProvider* mNodeProvider;

    /** Virtual view id for the currently focused logical item. */
    int mFocusedVirtualViewId = INVALID_ID;

    /** Virtual view id for the currently hovered logical item. */
    int mHoveredVirtualViewId = INVALID_ID;
public:
    /**
     * Factory method to create a new {@link ExploreByTouchHelper}.
     *
     * @param forView View whose logical children are exposed by this helper.
     */
    ExploreByTouchHelper(View* forView);

    /**
     * Returns the {@link android.view.accessibility.AccessibilityNodeProvider} for this helper.
     *
     * @param host View whose logical children are exposed by this helper.
     * @return The accessibility node provider for this helper.
     */
    AccessibilityNodeProvider* getAccessibilityNodeProvider(View& host)override;

    /**
     * Dispatches hover {@link android.view.MotionEvent}s to the virtual view hierarchy when
     * the Explore by Touch feature is enabled.
     * <p>
     * This method should be called by overriding
     * {@link View#dispatchHoverEvent}:
     *
     * <pre>&#64;Override
     * public bool dispatchHoverEvent(MotionEvent event) {
     *   if (mHelper.dispatchHoverEvent(this, event) {
     *     return true;
     *   }
     *   return super.dispatchHoverEvent(event);
     * }
     * </pre>
     *
     * @param event The hover event to dispatch to the virtual view hierarchy.
     * @return Whether the hover event was handled.
     */
    bool dispatchHoverEvent(MotionEvent& event);
    /**
     * Populates an event of the specified type with information about an item
     * and attempts to send it up through the view hierarchy.
     * <p>
     * You should call this method after performing a user action that normally
     * fires an accessibility event, such as clicking on an item.
     *
     * <pre>public void performItemClick(T item) {
     *   ...
     *   sendEventForVirtualViewId(item.id, AccessibilityEvent.TYPE_VIEW_CLICKED);
     * }
     * </pre>
     *
     * @param virtualViewId The virtual view id for which to send an event.
     * @param eventType The type of event to send.
     * @return true if the event was sent successfully.
     */
    bool sendEventForVirtualView(int virtualViewId, int eventType);

    /**
     * Notifies the accessibility framework that the properties of the parent
     * view have changed.
     * <p>
     * You <b>must</b> call this method after adding or removing items from the
     * parent view.
     */
    void invalidateRoot();
    /**
     * Notifies the accessibility framework that the properties of a particular
     * item have changed.
     * <p>
     * You <b>must</b> call this method after changing any of the properties set
     * in {@link #onPopulateNodeForVirtualView}.
     *
     * @param virtualViewId The virtual view id to invalidate, or
     *                      {@link #HOST_ID} to invalidate the root view.
     * @see #invalidateVirtualView(int, int)
     */
    void invalidateVirtualView(int virtualViewId);
    /**
     * Notifies the accessibility framework that the properties of a particular
     * item have changed.
     * <p>
     * You <b>must</b> call this method after changing any of the properties set
     * in {@link #onPopulateNodeForVirtualView}.
     *
     * @param virtualViewId The virtual view id to invalidate, or
     *                      {@link #HOST_ID} to invalidate the root view.
     * @param changeTypes The bit mask of change types. May be {@code 0} for the
     *                    default (undefined) change type or one or more of:
     *         <ul>
     *         <li>{@link AccessibilityEvent#CONTENT_CHANGE_TYPE_CONTENT_DESCRIPTION}
     *         <li>{@link AccessibilityEvent#CONTENT_CHANGE_TYPE_SUBTREE}
     *         <li>{@link AccessibilityEvent#CONTENT_CHANGE_TYPE_TEXT}
     *         <li>{@link AccessibilityEvent#CONTENT_CHANGE_TYPE_UNDEFINED}
     *         </ul>
     */
    void invalidateVirtualView(int virtualViewId, int changeTypes);

    /**
     * Returns the virtual view id for the currently focused item,
     *
     * @return A virtual view id, or {@link #INVALID_ID} if no item is
     *         currently focused.
     */
    int getFocusedVirtualView();
    /**
     * Sets the currently hovered item, sending hover accessibility events as
     * necessary to maintain the correct state.
     *
     * @param virtualViewId The virtual view id for the item currently being
     *            hovered, or {@link #INVALID_ID} if no item is hovered within
     *            the parent view.
     */
private:
    void updateHoveredVirtualView(int virtualViewId);

    /**
     * Constructs and returns an {@link AccessibilityEvent} for the specified
     * virtual view id, which includes the host view ({@link #HOST_ID}).
     *
     * @param virtualViewId The virtual view id for the item for which to
     *            construct an event.
     * @param eventType The type of event to construct.
     * @return An {@link AccessibilityEvent} populated with information about
     *         the specified item.
     */
    AccessibilityEvent* createEvent(int virtualViewId, int eventType);
    /**
     * Constructs and returns an {@link AccessibilityEvent} for the host node.
     *
     * @param eventType The type of event to construct.
     * @return An {@link AccessibilityEvent} populated with information about
     *         the specified item.
     */
    AccessibilityEvent* createEventForHost(int eventType);

    /**
     * Constructs and returns an {@link AccessibilityEvent} populated with
     * information about the specified item.
     *
     * @param virtualViewId The virtual view id for the item for which to
     *            construct an event.
     * @param eventType The type of event to construct.
     * @return An {@link AccessibilityEvent} populated with information about
     *         the specified item.
     */
    AccessibilityEvent* createEventForChild(int virtualViewId, int eventType);
    /**
     * Constructs and returns an {@link android.view.accessibility.AccessibilityNodeInfo} for the
     * specified virtual view id, which includes the host view
     * ({@link #HOST_ID}).
     *
     * @param virtualViewId The virtual view id for the item for which to
     *            construct a node.
     * @return An {@link android.view.accessibility.AccessibilityNodeInfo} populated with information
     *         about the specified item.
     */
    AccessibilityNodeInfo* createNode(int virtualViewId);
    /**
     * Constructs and returns an {@link AccessibilityNodeInfo} for the
     * host view populated with its virtual descendants.
     *
     * @return An {@link AccessibilityNodeInfo} for the parent node.
     */
    AccessibilityNodeInfo* createNodeForHost();
    /**
     * Constructs and returns an {@link AccessibilityNodeInfo} for the
     * specified item. Automatically manages accessibility focus actions.
     * <p>
     * Allows the implementing class to specify most node properties, but
     * overrides the following:
     * <ul>
     * <li>{@link AccessibilityNodeInfo#setPackageName}
     * <li>{@link AccessibilityNodeInfo#setClassName}
     * <li>{@link AccessibilityNodeInfo#setParent(View)}
     * <li>{@link AccessibilityNodeInfo#setSource(View, int)}
     * <li>{@link AccessibilityNodeInfo#setVisibleToUser}
     * <li>{@link AccessibilityNodeInfo#setBoundsInScreen(Rect)}
     * </ul>
     * <p>
     * Uses the bounds of the parent view and the parent-relative bounding
     * rectangle specified by
     * {@link AccessibilityNodeInfo#getBoundsInParent} to automatically
     * update the following properties:
     * <ul>
     * <li>{@link AccessibilityNodeInfo#setVisibleToUser}
     * <li>{@link AccessibilityNodeInfo#setBoundsInParent}
     * </ul>
     *
     * @param virtualViewId The virtual view id for item for which to construct
     *            a node.
     * @return An {@link AccessibilityNodeInfo} for the specified item.
     */
    AccessibilityNodeInfo* createNodeForChild(int virtualViewId);

    bool performAction(int virtualViewId, int action, Bundle* arguments);
    bool performActionForHost(int action, Bundle* arguments);
    bool performActionForChild(int virtualViewId, int action, Bundle* arguments);
    bool manageFocusForChild(int virtualViewId, int action);
    /**
     * Computes whether the specified {@link Rect} intersects with the visible
     * portion of its parent {@link View}. Modifies {@code localRect} to contain
     * only the visible portion.
     *
     * @param localRect A rectangle in local (parent) coordinates.
     * @return Whether the specified {@link Rect} is visible on the screen.
     */
    bool intersectVisibleToUser(Rect* localRect);
    /**
     * Returns whether this virtual view is accessibility focused.
     *
     * @return True if the view is accessibility focused.
     */
    bool isAccessibilityFocused(int virtualViewId);

    /**
     * Attempts to give accessibility focus to a virtual view.
     * <p>
     * A virtual view will not actually take focus if
     * {@link AccessibilityManager#isEnabled()} returns false,
     * {@link AccessibilityManager#isTouchExplorationEnabled()} returns false,
     * or the view already has accessibility focus.
     *
     * @param virtualViewId The id of the virtual view on which to place
     *            accessibility focus.
     * @return Whether this virtual view actually took accessibility focus.
     */
    bool requestAccessibilityFocus(int virtualViewId);
    /**
     * Attempts to clear accessibility focus from a virtual view.
     *
     * @param virtualViewId The id of the virtual view from which to clear
     *            accessibility focus.
     * @return Whether this virtual view actually cleared accessibility focus.
     */
    bool clearAccessibilityFocus(int virtualViewId);
protected:
    /**
     * Provides a mapping between view-relative coordinates and logical
     * items.
     *
     * @param x The view-relative x coordinate
     * @param y The view-relative y coordinate
     * @return virtual view identifier for the logical item under
     *         coordinates (x,y)
     */
    virtual int getVirtualViewAt(float x, float y)=0;

    /**
     * Populates a list with the view's visible items. The ordering of items
     * within {@code virtualViewIds} specifies order of accessibility focus
     * traversal.
     *
     * @param virtualViewIds The list to populate with visible items
     */
    virtual void getVisibleVirtualViews(std::vector<int>& virtualViewIds)=0;

    /**
     * Populates an {@link AccessibilityEvent} with information about the
     * specified item.
     * <p>
     * Implementations <b>must</b> populate the following required fields:
     * <ul>
     * <li>event text, see {@link AccessibilityEvent#getText} or
     * {@link AccessibilityEvent#setContentDescription}
     * </ul>
     * <p>
     * The helper class automatically populates the following fields with
     * default values, but implementations may optionally override them:
     * <ul>
     * <li>item class name, set to android.view.View, see
     * {@link AccessibilityEvent#setClassName}
     * </ul>
     * <p>
     * The following required fields are automatically populated by the
     * helper class and may not be overridden:
     * <ul>
     * <li>package name, set to the package of the host view's
     * {@link Context}, see {@link AccessibilityEvent#setPackageName}
     * <li>event source, set to the host view and virtual view identifier,
     * see {@link android.view.accessibility.AccessibilityRecord#setSource(View, int)}
     * </ul>
     *
     * @param virtualViewId The virtual view id for the item for which to
     *            populate the event
     * @param event The event to populate
     */
    virtual void onPopulateEventForVirtualView(int virtualViewId, AccessibilityEvent& event)=0;

    /**
     * Populates an {@link AccessibilityEvent} with information about the host
     * view.
     * <p>
     * The default implementation is a no-op.
     *
     * @param event the event to populate with information about the host view
     */
    virtual void onPopulateEventForHost(AccessibilityEvent& event);

    /**
     * Populates an {@link AccessibilityNodeInfo} with information
     * about the specified item.
     * <p>
     * Implementations <b>must</b> populate the following required fields:
     * <ul>
     * <li>event text, see {@link AccessibilityNodeInfo#setText} or
     * {@link AccessibilityNodeInfo#setContentDescription}
     * <li>bounds in parent coordinates, see
     * {@link AccessibilityNodeInfo#setBoundsInParent}
     * </ul>
     * <p>
     * The helper class automatically populates the following fields with
     * default values, but implementations may optionally override them:
     * <ul>
     * <li>enabled state, set to true, see
     * {@link AccessibilityNodeInfo#setEnabled}
     * <li>item class name, identical to the class name set by
     * {@link #onPopulateEventForVirtualView}, see
     * {@link AccessibilityNodeInfo#setClassName}
     * </ul>
     * <p>
     * The following required fields are automatically populated by the
     * helper class and may not be overridden:
     * <ul>
     * <li>package name, identical to the package name set by
     * {@link #onPopulateEventForVirtualView}, see
     * {@link AccessibilityNodeInfo#setPackageName}
     * <li>node source, identical to the event source set in
     * {@link #onPopulateEventForVirtualView}, see
     * {@link AccessibilityNodeInfo#setSource(View, int)}
     * <li>parent view, set to the host view, see
     * {@link AccessibilityNodeInfo#setParent(View)}
     * <li>visibility, computed based on parent-relative bounds, see
     * {@link AccessibilityNodeInfo#setVisibleToUser}
     * <li>accessibility focus, computed based on internal helper state, see
     * {@link AccessibilityNodeInfo#setAccessibilityFocused}
     * <li>bounds in screen coordinates, computed based on host view bounds,
     * see {@link AccessibilityNodeInfo#setBoundsInScreen}
     * </ul>
     * <p>
     * Additionally, the helper class automatically handles accessibility
     * focus management by adding the appropriate
     * {@link AccessibilityNodeInfo#ACTION_ACCESSIBILITY_FOCUS} or
     * {@link AccessibilityNodeInfo#ACTION_CLEAR_ACCESSIBILITY_FOCUS}
     * action. Implementations must <b>never</b> manually add these actions.
     * <p>
     * The helper class also automatically modifies parent- and
     * screen-relative bounds to reflect the portion of the item visible
     * within its parent.
     *
     * @param virtualViewId The virtual view identifier of the item for
     *            which to populate the node
     * @param node The node to populate
     */
    virtual void onPopulateNodeForVirtualView(int virtualViewId, AccessibilityNodeInfo& node)=0;

    /**
     * Populates an {@link AccessibilityNodeInfo} with information about the
     * host view.
     * <p>
     * The default implementation is a no-op.
     *
     * @param node the node to populate with information about the host view
     */
     virtual void onPopulateNodeForHost(AccessibilityNodeInfo& node);
    /**
     * Performs the specified accessibility action on the item associated
     * with the virtual view identifier. See
     * {@link AccessibilityNodeInfo#performAction(int, Bundle)} for
     * more information.
     * <p>
     * Implementations <b>must</b> handle any actions added manually in
     * {@link #onPopulateNodeForVirtualView}.
     * <p>
     * The helper class automatically handles focus management resulting
     * from {@link AccessibilityNodeInfo#ACTION_ACCESSIBILITY_FOCUS}
     * and
     * {@link AccessibilityNodeInfo#ACTION_CLEAR_ACCESSIBILITY_FOCUS}
     * actions.
     *
     * @param virtualViewId The virtual view identifier of the item on which
     *            to perform the action
     * @param action The accessibility action to perform
     * @param arguments (Optional) A bundle with additional arguments, or
     *            null
     * @return true if the action was performed
     */
    virtual bool onPerformActionForVirtualView(int virtualViewId, int action, Bundle* arguments)=0;
};

/**
 * Exposes a virtual view hierarchy to the accessibility framework. Only
 * used in API 16+.
 */
class ExploreByTouchHelper::ExploreByTouchNodeProvider:public AccessibilityNodeProvider {
private:
    ExploreByTouchHelper*mExp;
public:
    ExploreByTouchNodeProvider(ExploreByTouchHelper*e){
        mExp=e;
    }
    AccessibilityNodeInfo* createAccessibilityNodeInfo(int virtualViewId)override {
        return mExp->createNode(virtualViewId);
    }

    bool performAction(int virtualViewId, int action, Bundle* arguments)override {
        return mExp->performAction(virtualViewId, action, arguments);
    }
};
}/*endof namespace*/
#endif/*__EXPLOREBY_TOUCH_HELPER_H__*/
