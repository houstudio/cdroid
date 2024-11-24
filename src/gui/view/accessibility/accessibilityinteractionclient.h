#ifndef __ACCESSIBILITY_INTERACTION_CLIENT_H__
#define __ACCESSIBILITY_INTERACTION_CLIENT_H__
namespace cdroid{
class AccessibilityInteractionClient{//extends IAccessibilityInteractionConnectionCallback.Stub {

public:
    static constexpr int NO_ID = -1;

private:
    static constexpr bool Debug = false;

    static constexpr bool CHECK_INTEGRITY = true;

    static constepr long TIMEOUT_INTERACTION_MILLIS = 5000;

    static final Object sStaticLock = new Object();

    static final LongSparseArray<AccessibilityInteractionClient> sClients;

    static final SparseArray<IAccessibilityServiceConnection> sConnectionCache;

    static AccessibilityCache sAccessibilityCache;//=new AccessibilityCache(new AccessibilityCache.AccessibilityNodeRefresher());

    AtomicInteger mInteractionIdCounter = new AtomicInteger();

    final Object mInstanceLock = new Object();

    int mInteractionId = -1;

    AccessibilityNodeInfo mFindAccessibilityNodeInfoResult;

    List<AccessibilityNodeInfo> mFindAccessibilityNodeInfosResult;

    bool mPerformAccessibilityActionResult;

    Message mSameThreadMessage;
public:
    /**
     * @return The client for the current thread.
     */
    static AccessibilityInteractionClient getInstance();

    /**
     * <strong>Note:</strong> We keep one instance per interrogating thread since
     * the instance contains state which can lead to undesired thread interleavings.
     * We do not have a thread local variable since other threads should be able to
     * look up the correct client knowing a thread id. See ViewRootImpl for details.
     *
     * @return The client for a given <code>threadId</code>.
     */
    static AccessibilityInteractionClient getInstanceForThread(long threadId);

    /**
     * Gets a cached accessibility service connection.
     *
     * @param connectionId The connection id.
     * @return The cached connection if such.
     */
    static IAccessibilityServiceConnection getConnection(int connectionId);

    /**
     * Adds a cached accessibility service connection.
     *
     * @param connectionId The connection id.
     * @param connection The connection.
     */
    static void addConnection(int connectionId, IAccessibilityServiceConnection connection);

    /**
     * Removes a cached accessibility service connection.
     *
     * @param connectionId The connection id.
     */
    static void removeConnection(int connectionId);

    /**
     * This method is only for testing. Replacing the cache is a generally terrible idea, but
     * tests need to be able to verify this class's interactions with the cache
     */
    static void setCache(AccessibilityCache cache);

    private AccessibilityInteractionClient();

    /**
     * Sets the message to be processed if the interacted view hierarchy
     * and the interacting client are running in the same thread.
     *
     * @param message The message.
     */
    public void setSameThreadMessage(Message message);

    /**
     * Gets the root {@link AccessibilityNodeInfo} in the currently active window.
     *
     * @param connectionId The id of a connection for interacting with the system.
     * @return The root {@link AccessibilityNodeInfo} if found, null otherwise.
     */
    public AccessibilityNodeInfo* getRootInActiveWindow(int connectionId);

    /**
     * Gets the info for a window.
     *
     * @param connectionId The id of a connection for interacting with the system.
     * @param accessibilityWindowId A unique window id. Use
     *     {@link android.view.accessibility.AccessibilityWindowInfo#ACTIVE_WINDOW_ID}
     *     to query the currently active window.
     * @return The {@link AccessibilityWindowInfo}.
     */
    public AccessibilityWindowInfo getWindow(int connectionId, int accessibilityWindowId);

    /**
     * Gets the info for all windows.
     *
     * @param connectionId The id of a connection for interacting with the system.
     * @return The {@link AccessibilityWindowInfo} list.
     */
    public List<AccessibilityWindowInfo> getWindows(int connectionId);

    /**
     * Finds an {@link AccessibilityNodeInfo} by accessibility id.
     *
     * @param connectionId The id of a connection for interacting with the system.
     * @param accessibilityWindowId A unique window id. Use
     *     {@link android.view.accessibility.AccessibilityWindowInfo#ACTIVE_WINDOW_ID}
     *     to query the currently active window.
     * @param accessibilityNodeId A unique view id or virtual descendant id from
     *     where to start the search. Use
     *     {@link android.view.accessibility.AccessibilityNodeInfo#ROOT_NODE_ID}
     *     to start from the root.
     * @param bypassCache Whether to bypass the cache while looking for the node.
     * @param prefetchFlags flags to guide prefetching.
     * @return An {@link AccessibilityNodeInfo} if found, null otherwise.
     */
    public AccessibilityNodeInfo findAccessibilityNodeInfoByAccessibilityId(int connectionId,
            int accessibilityWindowId, long accessibilityNodeId, boolean bypassCache,
            int prefetchFlags, Bundle arguments);
    private static String idToString(int accessibilityWindowId, long accessibilityNodeId);

    /**
     * Finds an {@link AccessibilityNodeInfo} by View id. The search is performed in
     * the window whose id is specified and starts from the node whose accessibility
     * id is specified.
     *
     * @param connectionId The id of a connection for interacting with the system.
     * @param accessibilityWindowId A unique window id. Use
     *     {@link android.view.accessibility.AccessibilityWindowInfo#ACTIVE_WINDOW_ID}
     *     to query the currently active window.
     * @param accessibilityNodeId A unique view id or virtual descendant id from
     *     where to start the search. Use
     *     {@link android.view.accessibility.AccessibilityNodeInfo#ROOT_NODE_ID}
     *     to start from the root.
     * @param viewId The fully qualified resource name of the view id to find.
     * @return An list of {@link AccessibilityNodeInfo} if found, empty list otherwise.
     */
    public List<AccessibilityNodeInfo> findAccessibilityNodeInfosByViewId(int connectionId,
            int accessibilityWindowId, long accessibilityNodeId, String viewId);

    /**
     * Finds {@link AccessibilityNodeInfo}s by View text. The match is case
     * insensitive containment. The search is performed in the window whose
     * id is specified and starts from the node whose accessibility id is
     * specified.
     *
     * @param connectionId The id of a connection for interacting with the system.
     * @param accessibilityWindowId A unique window id. Use
     *     {@link android.view.accessibility.AccessibilityWindowInfo#ACTIVE_WINDOW_ID}
     *     to query the currently active window.
     * @param accessibilityNodeId A unique view id or virtual descendant id from
     *     where to start the search. Use
     *     {@link android.view.accessibility.AccessibilityNodeInfo#ROOT_NODE_ID}
     *     to start from the root.
     * @param text The searched text.
     * @return A list of found {@link AccessibilityNodeInfo}s.
     */
    public List<AccessibilityNodeInfo> findAccessibilityNodeInfosByText(int connectionId,
            int accessibilityWindowId, long accessibilityNodeId, String text);

    /**
     * Finds the {@link android.view.accessibility.AccessibilityNodeInfo} that has the
     * specified focus type. The search is performed in the window whose id is specified
     * and starts from the node whose accessibility id is specified.
     *
     * @param connectionId The id of a connection for interacting with the system.
     * @param accessibilityWindowId A unique window id. Use
     *     {@link android.view.accessibility.AccessibilityWindowInfo#ACTIVE_WINDOW_ID}
     *     to query the currently active window.
     * @param accessibilityNodeId A unique view id or virtual descendant id from
     *     where to start the search. Use
     *     {@link android.view.accessibility.AccessibilityNodeInfo#ROOT_NODE_ID}
     *     to start from the root.
     * @param focusType The focus type.
     * @return The accessibility focused {@link AccessibilityNodeInfo}.
     */
    public AccessibilityNodeInfo findFocus(int connectionId, int accessibilityWindowId,
            long accessibilityNodeId, int focusType);
    /**
     * Finds the accessibility focused {@link android.view.accessibility.AccessibilityNodeInfo}.
     * The search is performed in the window whose id is specified and starts from the
     * node whose accessibility id is specified.
     *
     * @param connectionId The id of a connection for interacting with the system.
     * @param accessibilityWindowId A unique window id. Use
     *     {@link android.view.accessibility.AccessibilityWindowInfo#ACTIVE_WINDOW_ID}
     *     to query the currently active window.
     * @param accessibilityNodeId A unique view id or virtual descendant id from
     *     where to start the search. Use
     *     {@link android.view.accessibility.AccessibilityNodeInfo#ROOT_NODE_ID}
     *     to start from the root.
     * @param direction The direction in which to search for focusable.
     * @return The accessibility focused {@link AccessibilityNodeInfo}.
     */
    public AccessibilityNodeInfo focusSearch(int connectionId, int accessibilityWindowId,
            long accessibilityNodeId, int direction);

    /**
     * Performs an accessibility action on an {@link AccessibilityNodeInfo}.
     *
     * @param connectionId The id of a connection for interacting with the system.
     * @param accessibilityWindowId A unique window id. Use
     *     {@link android.view.accessibility.AccessibilityWindowInfo#ACTIVE_WINDOW_ID}
     *     to query the currently active window.
     * @param accessibilityNodeId A unique view id or virtual descendant id from
     *     where to start the search. Use
     *     {@link android.view.accessibility.AccessibilityNodeInfo#ROOT_NODE_ID}
     *     to start from the root.
     * @param action The action to perform.
     * @param arguments Optional action arguments.
     * @return Whether the action was performed.
     */
    public bool performAccessibilityAction(int connectionId, int accessibilityWindowId,
            long accessibilityNodeId, int action, Bundle arguments);

    public void clearCache() {

    public void onAccessibilityEvent(AccessibilityEvent event);

    /**
     * Gets the the result of an async request that returns an {@link AccessibilityNodeInfo}.
     *
     * @param interactionId The interaction id to match the result with the request.
     * @return The result {@link AccessibilityNodeInfo}.
     */
    private AccessibilityNodeInfo getFindAccessibilityNodeInfoResultAndClear(int interactionId);

    /**
     * {@inheritDoc}
     */
    public void setFindAccessibilityNodeInfoResult(AccessibilityNodeInfo info,
                int interactionId);

    /**
     * Gets the the result of an async request that returns {@link AccessibilityNodeInfo}s.
     *
     * @param interactionId The interaction id to match the result with the request.
     * @return The result {@link AccessibilityNodeInfo}s.
     */
    private List<AccessibilityNodeInfo> getFindAccessibilityNodeInfosResultAndClear(
                int interactionId);

    /**
     * {@inheritDoc}
     */
    public void setFindAccessibilityNodeInfosResult(List<AccessibilityNodeInfo> infos,
                int interactionId);

    /**
     * Gets the result of a request to perform an accessibility action.
     *
     * @param interactionId The interaction id to match the result with the request.
     * @return Whether the action was performed.
     */
    private boolean getPerformAccessibilityActionResultAndClear(int interactionId) {

    /**
     * {@inheritDoc}
     */
    public void setPerformAccessibilityActionResult(bool succeeded, int interactionId);

    /**
     * Clears the result state.
     */
    private void clearResultLocked();

    /**
     * Waits up to a given bound for a result of a request and returns it.
     *
     * @param interactionId The interaction id to match the result with the request.
     * @return Whether the result was received.
     */
    private bool waitForResultTimedLocked(int interactionId);

    /**
     * Finalize an {@link AccessibilityNodeInfo} before passing it to the client.
     *
     * @param info The info.
     * @param connectionId The id of the connection to the system.
     * @param bypassCache Whether or not to bypass the cache. The node is added to the cache if
     *                    this value is {@code false}
     * @param packageNames The valid package names a node can come from.
     */
    private void finalizeAndCacheAccessibilityNodeInfo(AccessibilityNodeInfo info,
            int connectionId, boolean bypassCache, String[] packageNames);

    /**
     * Finalize {@link AccessibilityNodeInfo}s before passing them to the client.
     *
     * @param infos The {@link AccessibilityNodeInfo}s.
     * @param connectionId The id of the connection to the system.
     * @param bypassCache Whether or not to bypass the cache. The nodes are added to the cache if
     *                    this value is {@code false}
     * @param packageNames The valid package names a node can come from.
     */
    private void finalizeAndCacheAccessibilityNodeInfos(List<AccessibilityNodeInfo> infos,
            int connectionId, boolean bypassCache, String[] packageNames);

    /**
     * Gets the message stored if the interacted and interacting
     * threads are the same.
     *
     * @return The message.
     */
    private Message getSameProcessMessageAndClear();

    /**
     * Checks whether the infos are a fully connected tree with no duplicates.
     *
     * @param infos The result list to check.
     */
    private void checkFindAccessibilityNodeInfoResultIntegrity(List<AccessibilityNodeInfo> infos);
};
}/*endof namespace*/
#endif/*__ACCESSIBILITY_INTERACTION_CLIENT_H__*/
