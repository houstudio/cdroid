public final class AccessibilityInteractionClient
        extends IAccessibilityInteractionConnectionCallback.Stub {

    public static final int NO_ID = -1;

    private static final String LOG_TAG = "AccessibilityInteractionClient";

    private static final boolean DEBUG = false;

    private static final boolean CHECK_INTEGRITY = true;

    private static final long TIMEOUT_INTERACTION_MILLIS = 5000;

    private static final Object sStaticLock = new Object();

    private static final LongSparseArray<AccessibilityInteractionClient> sClients =
        new LongSparseArray<>();

    private static final SparseArray<IAccessibilityServiceConnection> sConnectionCache =
            new SparseArray<>();

    private static AccessibilityCache sAccessibilityCache =
            new AccessibilityCache(new AccessibilityCache.AccessibilityNodeRefresher());

    private final AtomicInteger mInteractionIdCounter = new AtomicInteger();

    private final Object mInstanceLock = new Object();

    private volatile int mInteractionId = -1;

    private AccessibilityNodeInfo mFindAccessibilityNodeInfoResult;

    private List<AccessibilityNodeInfo> mFindAccessibilityNodeInfosResult;

    private boolean mPerformAccessibilityActionResult;

    private Message mSameThreadMessage;

    /**
     * @return The client for the current thread.
     */
    public static AccessibilityInteractionClient getInstance() {
        final long threadId = Thread.currentThread().getId();
        return getInstanceForThread(threadId);
    }

    /**
     * <strong>Note:</strong> We keep one instance per interrogating thread since
     * the instance contains state which can lead to undesired thread interleavings.
     * We do not have a thread local variable since other threads should be able to
     * look up the correct client knowing a thread id. See ViewRootImpl for details.
     *
     * @return The client for a given <code>threadId</code>.
     */
    public static AccessibilityInteractionClient getInstanceForThread(long threadId) {
        synchronized (sStaticLock) {
            AccessibilityInteractionClient client = sClients.get(threadId);
            if (client == null) {
                client = new AccessibilityInteractionClient();
                sClients.put(threadId, client);
            }
            return client;
        }
    }

    /**
     * Gets a cached accessibility service connection.
     *
     * @param connectionId The connection id.
     * @return The cached connection if such.
     */
    public static IAccessibilityServiceConnection getConnection(int connectionId) {
        synchronized (sConnectionCache) {
            return sConnectionCache.get(connectionId);
        }
    }

    /**
     * Adds a cached accessibility service connection.
     *
     * @param connectionId The connection id.
     * @param connection The connection.
     */
    public static void addConnection(int connectionId, IAccessibilityServiceConnection connection) {
        synchronized (sConnectionCache) {
            sConnectionCache.put(connectionId, connection);
        }
    }

    /**
     * Removes a cached accessibility service connection.
     *
     * @param connectionId The connection id.
     */
    public static void removeConnection(int connectionId) {
        synchronized (sConnectionCache) {
            sConnectionCache.remove(connectionId);
        }
    }

    /**
     * This method is only for testing. Replacing the cache is a generally terrible idea, but
     * tests need to be able to verify this class's interactions with the cache
     */
    @VisibleForTesting
    public static void setCache(AccessibilityCache cache) {
        sAccessibilityCache = cache;
    }

    private AccessibilityInteractionClient() {
        /* reducing constructor visibility */
    }

    /**
     * Sets the message to be processed if the interacted view hierarchy
     * and the interacting client are running in the same thread.
     *
     * @param message The message.
     */
    public void setSameThreadMessage(Message message) {
        synchronized (mInstanceLock) {
            mSameThreadMessage = message;
            mInstanceLock.notifyAll();
        }
    }

    /**
     * Gets the root {@link AccessibilityNodeInfo} in the currently active window.
     *
     * @param connectionId The id of a connection for interacting with the system.
     * @return The root {@link AccessibilityNodeInfo} if found, null otherwise.
     */
    public AccessibilityNodeInfo getRootInActiveWindow(int connectionId) {
        return findAccessibilityNodeInfoByAccessibilityId(connectionId,
                AccessibilityWindowInfo.ACTIVE_WINDOW_ID, AccessibilityNodeInfo.ROOT_NODE_ID,
                false, AccessibilityNodeInfo.FLAG_PREFETCH_DESCENDANTS, null);
    }

    /**
     * Gets the info for a window.
     *
     * @param connectionId The id of a connection for interacting with the system.
     * @param accessibilityWindowId A unique window id. Use
     *     {@link android.view.accessibility.AccessibilityWindowInfo#ACTIVE_WINDOW_ID}
     *     to query the currently active window.
     * @return The {@link AccessibilityWindowInfo}.
     */
    public AccessibilityWindowInfo getWindow(int connectionId, int accessibilityWindowId) {
        try {
            IAccessibilityServiceConnection connection = getConnection(connectionId);
            if (connection != null) {
                AccessibilityWindowInfo window = sAccessibilityCache.getWindow(
                        accessibilityWindowId);
                if (window != null) {
                    if (DEBUG) {
                        Log.i(LOG_TAG, "Window cache hit");
                    }
                    return window;
                }
                if (DEBUG) {
                    Log.i(LOG_TAG, "Window cache miss");
                }
                final long identityToken = Binder.clearCallingIdentity();
                try {
                    window = connection.getWindow(accessibilityWindowId);
                } finally {
                    Binder.restoreCallingIdentity(identityToken);
                }
                if (window != null) {
                    sAccessibilityCache.addWindow(window);
                    return window;
                }
            } else {
                if (DEBUG) {
                    Log.w(LOG_TAG, "No connection for connection id: " + connectionId);
                }
            }
        } catch (RemoteException re) {
            Log.e(LOG_TAG, "Error while calling remote getWindow", re);
        }
        return null;
    }

    /**
     * Gets the info for all windows.
     *
     * @param connectionId The id of a connection for interacting with the system.
     * @return The {@link AccessibilityWindowInfo} list.
     */
    public List<AccessibilityWindowInfo> getWindows(int connectionId) {
        try {
            IAccessibilityServiceConnection connection = getConnection(connectionId);
            if (connection != null) {
                List<AccessibilityWindowInfo> windows = sAccessibilityCache.getWindows();
                if (windows != null) {
                    if (DEBUG) {
                        Log.i(LOG_TAG, "Windows cache hit");
                    }
                    return windows;
                }
                if (DEBUG) {
                    Log.i(LOG_TAG, "Windows cache miss");
                }
                final long identityToken = Binder.clearCallingIdentity();
                try {
                    windows = connection.getWindows();
                } finally {
                    Binder.restoreCallingIdentity(identityToken);
                }
                if (windows != null) {
                    sAccessibilityCache.setWindows(windows);
                    return windows;
                }
            } else {
                if (DEBUG) {
                    Log.w(LOG_TAG, "No connection for connection id: " + connectionId);
                }
            }
        } catch (RemoteException re) {
            Log.e(LOG_TAG, "Error while calling remote getWindows", re);
        }
        return Collections.emptyList();
    }

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
            int prefetchFlags, Bundle arguments) {
        if ((prefetchFlags & AccessibilityNodeInfo.FLAG_PREFETCH_SIBLINGS) != 0
                && (prefetchFlags & AccessibilityNodeInfo.FLAG_PREFETCH_PREDECESSORS) == 0) {
            throw new IllegalArgumentException("FLAG_PREFETCH_SIBLINGS"
                + " requires FLAG_PREFETCH_PREDECESSORS");
        }
        try {
            IAccessibilityServiceConnection connection = getConnection(connectionId);
            if (connection != null) {
                if (!bypassCache) {
                    AccessibilityNodeInfo cachedInfo = sAccessibilityCache.getNode(
                            accessibilityWindowId, accessibilityNodeId);
                    if (cachedInfo != null) {
                        if (DEBUG) {
                            Log.i(LOG_TAG, "Node cache hit for "
                                    + idToString(accessibilityWindowId, accessibilityNodeId));
                        }
                        return cachedInfo;
                    }
                    if (DEBUG) {
                        Log.i(LOG_TAG, "Node cache miss for "
                                + idToString(accessibilityWindowId, accessibilityNodeId));
                    }
                }
                final int interactionId = mInteractionIdCounter.getAndIncrement();
                final long identityToken = Binder.clearCallingIdentity();
                final String[] packageNames;
                try {
                    packageNames = connection.findAccessibilityNodeInfoByAccessibilityId(
                            accessibilityWindowId, accessibilityNodeId, interactionId, this,
                            prefetchFlags, Thread.currentThread().getId(), arguments);
                } finally {
                    Binder.restoreCallingIdentity(identityToken);
                }
                if (packageNames != null) {
                    List<AccessibilityNodeInfo> infos = getFindAccessibilityNodeInfosResultAndClear(
                            interactionId);
                    finalizeAndCacheAccessibilityNodeInfos(infos, connectionId,
                            bypassCache, packageNames);
                    if (infos != null && !infos.isEmpty()) {
                        for (int i = 1; i < infos.size(); i++) {
                            infos.get(i).recycle();
                        }
                        return infos.get(0);
                    }
                }
            } else {
                if (DEBUG) {
                    Log.w(LOG_TAG, "No connection for connection id: " + connectionId);
                }
            }
        } catch (RemoteException re) {
            Log.e(LOG_TAG, "Error while calling remote"
                    + " findAccessibilityNodeInfoByAccessibilityId", re);
        }
        return null;
    }

    private static String idToString(int accessibilityWindowId, long accessibilityNodeId) {
        return accessibilityWindowId + "/"
                + AccessibilityNodeInfo.idToString(accessibilityNodeId);
    }

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
            int accessibilityWindowId, long accessibilityNodeId, String viewId) {
        try {
            IAccessibilityServiceConnection connection = getConnection(connectionId);
            if (connection != null) {
                final int interactionId = mInteractionIdCounter.getAndIncrement();
                final long identityToken = Binder.clearCallingIdentity();
                final String[] packageNames;
                try {
                    packageNames = connection.findAccessibilityNodeInfosByViewId(
                            accessibilityWindowId, accessibilityNodeId, viewId, interactionId, this,
                            Thread.currentThread().getId());
                } finally {
                    Binder.restoreCallingIdentity(identityToken);
                }

                if (packageNames != null) {
                    List<AccessibilityNodeInfo> infos = getFindAccessibilityNodeInfosResultAndClear(
                            interactionId);
                    if (infos != null) {
                        finalizeAndCacheAccessibilityNodeInfos(infos, connectionId,
                                false, packageNames);
                        return infos;
                    }
                }
            } else {
                if (DEBUG) {
                    Log.w(LOG_TAG, "No connection for connection id: " + connectionId);
                }
            }
        } catch (RemoteException re) {
            Log.w(LOG_TAG, "Error while calling remote"
                    + " findAccessibilityNodeInfoByViewIdInActiveWindow", re);
        }
        return Collections.emptyList();
    }

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
            int accessibilityWindowId, long accessibilityNodeId, String text) {
        try {
            IAccessibilityServiceConnection connection = getConnection(connectionId);
            if (connection != null) {
                final int interactionId = mInteractionIdCounter.getAndIncrement();
                final long identityToken = Binder.clearCallingIdentity();
                final String[] packageNames;
                try {
                    packageNames = connection.findAccessibilityNodeInfosByText(
                            accessibilityWindowId, accessibilityNodeId, text, interactionId, this,
                            Thread.currentThread().getId());
                } finally {
                    Binder.restoreCallingIdentity(identityToken);
                }

                if (packageNames != null) {
                    List<AccessibilityNodeInfo> infos = getFindAccessibilityNodeInfosResultAndClear(
                            interactionId);
                    if (infos != null) {
                        finalizeAndCacheAccessibilityNodeInfos(infos, connectionId,
                                false, packageNames);
                        return infos;
                    }
                }
            } else {
                if (DEBUG) {
                    Log.w(LOG_TAG, "No connection for connection id: " + connectionId);
                }
            }
        } catch (RemoteException re) {
            Log.w(LOG_TAG, "Error while calling remote"
                    + " findAccessibilityNodeInfosByViewText", re);
        }
        return Collections.emptyList();
    }

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
            long accessibilityNodeId, int focusType) {
        try {
            IAccessibilityServiceConnection connection = getConnection(connectionId);
            if (connection != null) {
                final int interactionId = mInteractionIdCounter.getAndIncrement();
                final long identityToken = Binder.clearCallingIdentity();
                final String[] packageNames;
                try {
                    packageNames = connection.findFocus(accessibilityWindowId,
                            accessibilityNodeId, focusType, interactionId, this,
                            Thread.currentThread().getId());
                } finally {
                    Binder.restoreCallingIdentity(identityToken);
                }

                if (packageNames != null) {
                    AccessibilityNodeInfo info = getFindAccessibilityNodeInfoResultAndClear(
                            interactionId);
                    finalizeAndCacheAccessibilityNodeInfo(info, connectionId, false, packageNames);
                    return info;
                }
            } else {
                if (DEBUG) {
                    Log.w(LOG_TAG, "No connection for connection id: " + connectionId);
                }
            }
        } catch (RemoteException re) {
            Log.w(LOG_TAG, "Error while calling remote findFocus", re);
        }
        return null;
    }

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
            long accessibilityNodeId, int direction) {
        try {
            IAccessibilityServiceConnection connection = getConnection(connectionId);
            if (connection != null) {
                final int interactionId = mInteractionIdCounter.getAndIncrement();
                final long identityToken = Binder.clearCallingIdentity();
                final String[] packageNames;
                try {
                    packageNames = connection.focusSearch(accessibilityWindowId,
                            accessibilityNodeId, direction, interactionId, this,
                            Thread.currentThread().getId());
                } finally {
                    Binder.restoreCallingIdentity(identityToken);
                }

                if (packageNames != null) {
                    AccessibilityNodeInfo info = getFindAccessibilityNodeInfoResultAndClear(
                            interactionId);
                    finalizeAndCacheAccessibilityNodeInfo(info, connectionId, false, packageNames);
                    return info;
                }
            } else {
                if (DEBUG) {
                    Log.w(LOG_TAG, "No connection for connection id: " + connectionId);
                }
            }
        } catch (RemoteException re) {
            Log.w(LOG_TAG, "Error while calling remote accessibilityFocusSearch", re);
        }
        return null;
    }

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
    public boolean performAccessibilityAction(int connectionId, int accessibilityWindowId,
            long accessibilityNodeId, int action, Bundle arguments) {
        try {
            IAccessibilityServiceConnection connection = getConnection(connectionId);
            if (connection != null) {
                final int interactionId = mInteractionIdCounter.getAndIncrement();
                final long identityToken = Binder.clearCallingIdentity();
                final boolean success;
                try {
                    success = connection.performAccessibilityAction(
                            accessibilityWindowId, accessibilityNodeId, action, arguments,
                            interactionId, this, Thread.currentThread().getId());
                } finally {
                    Binder.restoreCallingIdentity(identityToken);
                }

                if (success) {
                    return getPerformAccessibilityActionResultAndClear(interactionId);
                }
            } else {
                if (DEBUG) {
                    Log.w(LOG_TAG, "No connection for connection id: " + connectionId);
                }
            }
        } catch (RemoteException re) {
            Log.w(LOG_TAG, "Error while calling remote performAccessibilityAction", re);
        }
        return false;
    }

    public void clearCache() {
        sAccessibilityCache.clear();
    }

    public void onAccessibilityEvent(AccessibilityEvent event) {
        sAccessibilityCache.onAccessibilityEvent(event);
    }

    /**
     * Gets the the result of an async request that returns an {@link AccessibilityNodeInfo}.
     *
     * @param interactionId The interaction id to match the result with the request.
     * @return The result {@link AccessibilityNodeInfo}.
     */
    private AccessibilityNodeInfo getFindAccessibilityNodeInfoResultAndClear(int interactionId) {
        synchronized (mInstanceLock) {
            final boolean success = waitForResultTimedLocked(interactionId);
            AccessibilityNodeInfo result = success ? mFindAccessibilityNodeInfoResult : null;
            clearResultLocked();
            return result;
        }
    }

    /**
     * {@inheritDoc}
     */
    public void setFindAccessibilityNodeInfoResult(AccessibilityNodeInfo info,
                int interactionId) {
        synchronized (mInstanceLock) {
            if (interactionId > mInteractionId) {
                mFindAccessibilityNodeInfoResult = info;
                mInteractionId = interactionId;
            }
            mInstanceLock.notifyAll();
        }
    }

    /**
     * Gets the the result of an async request that returns {@link AccessibilityNodeInfo}s.
     *
     * @param interactionId The interaction id to match the result with the request.
     * @return The result {@link AccessibilityNodeInfo}s.
     */
    private List<AccessibilityNodeInfo> getFindAccessibilityNodeInfosResultAndClear(
                int interactionId) {
        synchronized (mInstanceLock) {
            final boolean success = waitForResultTimedLocked(interactionId);
            final List<AccessibilityNodeInfo> result;
            if (success) {
                result = mFindAccessibilityNodeInfosResult;
            } else {
                result = Collections.emptyList();
            }
            clearResultLocked();
            if (Build.IS_DEBUGGABLE && CHECK_INTEGRITY) {
                checkFindAccessibilityNodeInfoResultIntegrity(result);
            }
            return result;
        }
    }

    /**
     * {@inheritDoc}
     */
    public void setFindAccessibilityNodeInfosResult(List<AccessibilityNodeInfo> infos,
                int interactionId) {
        synchronized (mInstanceLock) {
            if (interactionId > mInteractionId) {
                if (infos != null) {
                    // If the call is not an IPC, i.e. it is made from the same process, we need to
                    // instantiate new result list to avoid passing internal instances to clients.
                    final boolean isIpcCall = (Binder.getCallingPid() != Process.myPid());
                    if (!isIpcCall) {
                        mFindAccessibilityNodeInfosResult = new ArrayList<>(infos);
                    } else {
                        mFindAccessibilityNodeInfosResult = infos;
                    }
                } else {
                    mFindAccessibilityNodeInfosResult = Collections.emptyList();
                }
                mInteractionId = interactionId;
            }
            mInstanceLock.notifyAll();
        }
    }

    /**
     * Gets the result of a request to perform an accessibility action.
     *
     * @param interactionId The interaction id to match the result with the request.
     * @return Whether the action was performed.
     */
    private boolean getPerformAccessibilityActionResultAndClear(int interactionId) {
        synchronized (mInstanceLock) {
            final boolean success = waitForResultTimedLocked(interactionId);
            final boolean result = success ? mPerformAccessibilityActionResult : false;
            clearResultLocked();
            return result;
        }
    }

    /**
     * {@inheritDoc}
     */
    public void setPerformAccessibilityActionResult(boolean succeeded, int interactionId) {
        synchronized (mInstanceLock) {
            if (interactionId > mInteractionId) {
                mPerformAccessibilityActionResult = succeeded;
                mInteractionId = interactionId;
            }
            mInstanceLock.notifyAll();
        }
    }

    /**
     * Clears the result state.
     */
    private void clearResultLocked() {
        mInteractionId = -1;
        mFindAccessibilityNodeInfoResult = null;
        mFindAccessibilityNodeInfosResult = null;
        mPerformAccessibilityActionResult = false;
    }

    /**
     * Waits up to a given bound for a result of a request and returns it.
     *
     * @param interactionId The interaction id to match the result with the request.
     * @return Whether the result was received.
     */
    private boolean waitForResultTimedLocked(int interactionId) {
        long waitTimeMillis = TIMEOUT_INTERACTION_MILLIS;
        final long startTimeMillis = SystemClock.uptimeMillis();
        while (true) {
            try {
                Message sameProcessMessage = getSameProcessMessageAndClear();
                if (sameProcessMessage != null) {
                    sameProcessMessage.getTarget().handleMessage(sameProcessMessage);
                }

                if (mInteractionId == interactionId) {
                    return true;
                }
                if (mInteractionId > interactionId) {
                    return false;
                }
                final long elapsedTimeMillis = SystemClock.uptimeMillis() - startTimeMillis;
                waitTimeMillis = TIMEOUT_INTERACTION_MILLIS - elapsedTimeMillis;
                if (waitTimeMillis <= 0) {
                    return false;
                }
                mInstanceLock.wait(waitTimeMillis);
            } catch (InterruptedException ie) {
                /* ignore */
            }
        }
    }

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
            int connectionId, boolean bypassCache, String[] packageNames) {
        if (info != null) {
            info.setConnectionId(connectionId);
            // Empty array means any package name is Okay
            if (!ArrayUtils.isEmpty(packageNames)) {
                CharSequence packageName = info.getPackageName();
                if (packageName == null
                        || !ArrayUtils.contains(packageNames, packageName.toString())) {
                    // If the node package not one of the valid ones, pick the top one - this
                    // is one of the packages running in the introspected UID.
                    info.setPackageName(packageNames[0]);
                }
            }
            info.setSealed(true);
            if (!bypassCache) {
                sAccessibilityCache.add(info);
            }
        }
    }

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
            int connectionId, boolean bypassCache, String[] packageNames) {
        if (infos != null) {
            final int infosCount = infos.size();
            for (int i = 0; i < infosCount; i++) {
                AccessibilityNodeInfo info = infos.get(i);
                finalizeAndCacheAccessibilityNodeInfo(info, connectionId,
                        bypassCache, packageNames);
            }
        }
    }

    /**
     * Gets the message stored if the interacted and interacting
     * threads are the same.
     *
     * @return The message.
     */
    private Message getSameProcessMessageAndClear() {
        synchronized (mInstanceLock) {
            Message result = mSameThreadMessage;
            mSameThreadMessage = null;
            return result;
        }
    }

    /**
     * Checks whether the infos are a fully connected tree with no duplicates.
     *
     * @param infos The result list to check.
     */
    private void checkFindAccessibilityNodeInfoResultIntegrity(List<AccessibilityNodeInfo> infos) {
        if (infos.size() == 0) {
            return;
        }
        // Find the root node.
        AccessibilityNodeInfo root = infos.get(0);
        final int infoCount = infos.size();
        for (int i = 1; i < infoCount; i++) {
            for (int j = i; j < infoCount; j++) {
                AccessibilityNodeInfo candidate = infos.get(j);
                if (root.getParentNodeId() == candidate.getSourceNodeId()) {
                    root = candidate;
                    break;
                }
            }
        }
        if (root == null) {
            Log.e(LOG_TAG, "No root.");
        }
        // Check for duplicates.
        HashSet<AccessibilityNodeInfo> seen = new HashSet<>();
        Queue<AccessibilityNodeInfo> fringe = new LinkedList<>();
        fringe.add(root);
        while (!fringe.isEmpty()) {
            AccessibilityNodeInfo current = fringe.poll();
            if (!seen.add(current)) {
                Log.e(LOG_TAG, "Duplicate node.");
                return;
            }
            final int childCount = current.getChildCount();
            for (int i = 0; i < childCount; i++) {
                final long childId = current.getChildId(i);
                for (int j = 0; j < infoCount; j++) {
                    AccessibilityNodeInfo child = infos.get(j);
                    if (child.getSourceNodeId() == childId) {
                        fringe.add(child);
                    }
                }
            }
        }
        final int disconnectedCount = infos.size() - seen.size();
        if (disconnectedCount > 0) {
            Log.e(LOG_TAG, disconnectedCount + " Disconnected nodes.");
        }
    }
}