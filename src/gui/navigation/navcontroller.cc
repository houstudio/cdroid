#if 0
#include <navigation/navoptions.h>
#include <navigation/navcontroller.h>
#include <navigation/navdestination.h>

namespace cdroid{
/*
public class NavController {
    private static final String KEY_NAVIGATOR_STATE = "android-support-nav:controller:navigatorState";
    private static final String KEY_NAVIGATOR_STATE_NAMES = "android-support-nav:controller:navigatorState:names";
    private static final String KEY_GRAPH_ID = "android-support-nav:controller:graphId";
    private static final String KEY_BACK_STACK_IDS = "android-support-nav:controller:backStackIds";
    static final String KEY_DEEP_LINK_IDS = "android-support-nav:controller:deepLinkIds";
    static final String KEY_DEEP_LINK_EXTRAS = "android-support-nav:controller:deepLinkExtras";
    //The {@link Intent} that triggered a deep link to the current destination.
    public static final String KEY_DEEP_LINK_INTENT = "android-support-nav:controller:deepLinkIntent";
*/

class MySimpleNavigatorProvider:public SimpleNavigatorProvider{//mNavigatorProvider = new SimpleNavigatorProvider() {
public:
    Navigator* addNavigator(const std::string& name,/*NonNull*/Navigator navigator) override{
        Navigator*reviousNavigator = addNavigator(name, navigator);
        if (previousNavigator != navigator) {
            if (previousNavigator) {
                previousNavigator->removeOnNavigatorNavigatedListener(mOnNavigatedListener);
            }
            navigator.addOnNavigatorNavigatedListener(mOnNavigatedListener);
        }
        return previousNavigator;
    }
};

NavController::onNavigatorNavigated(Navigator& navigator,/*IdRes*/int destId,
        /*Navigator.BackStackEffect*/int backStackEffect) {
    if (destId != 0) {
        // First remove popped destinations off the back stack
        if (backStackEffect == Navigator::BACK_STACK_DESTINATION_POPPED) {
            while (!mBackStack.empty()
                    && mBackStack.peekLast().getId() != destId) {
                mBackStack.pop_back();//removeLast();
            }
        }
        NavDestination* newDest = findDestination(destId);
        FATAL_IF(newDest == nullptr,"Navigator %p reported navigation to unknown destination id ",navigator);
                    //NavDestination::getDisplayName(mContext, destId);
        if (backStackEffect == Navigator::BACK_STACK_DESTINATION_ADDED) {
            // Add the new destination to the back stack
            mBackStack.add(newDest);
        }
        // Don't dispatchOnNavigated if nothing changed
        if (backStackEffect != Navigator::BACK_STACK_UNCHANGED) {
            dispatchOnNavigated(newDest);
        }
    }
}

/**
 * Constructs a new controller for a given {@link Context}. Controllers should not be
 * used outside of their context and retain a hard reference to the context supplied.
 * If you need a global controller, pass {@link Context#getApplicationContext()}.
 *
 * <p>Apps should generally not construct controllers, instead obtain a relevant controller
 * directly from a navigation host via {@link NavHost#getNavController()} or by using one of
 * the utility methods on the {@link Navigation} class.</p>
 *
 * <p>Note that controllers that are not constructed with an {@link Activity} context
 * (or a wrapped activity context) will only be able to navigate to
 * {@link android.content.Intent#FLAG_ACTIVITY_NEW_TASK new tasks} or
 * {@link android.content.Intent#FLAG_ACTIVITY_NEW_DOCUMENT new document tasks} when
 * navigating to new activities.</p>
 *
 * @param context context for this controller
 */
NavController::NavController(Context* context) {
    mContext = context;
    mOnNavigatedListener=std::bind(&NavController::onNavigatorNavigated,this,std::placeholders::_1,
            std::placeholders::_2,std::placeholders::_3);
    mNavigatorProvider = new MySimpleNavigatorProvider();
    while (context instanceof ContextWrapper) {
        if (context instanceof Activity) {
            mActivity = (Activity) context;
            break;
        }
        context = ((ContextWrapper) context).getBaseContext();
    }
    mNavigatorProvider->addNavigator(new NavGraphNavigator(mContext));
    mNavigatorProvider->addNavigator(new ActivityNavigator(mContext));
}

Context* NavController::getContext() const{
    return mContext;
}

/**
 * Retrieve the NavController's {@link NavigatorProvider}. All {@link Navigator Navigators} used
 * to construct the {@link NavGraph navigation graph} for this nav controller should be added
 * to this navigator provider before the graph is constructed.
 * <p>
 * Generally, the Navigators are set for you by the {@link NavHost} hosting this NavController
 * and you do not need to manually interact with the navigator provider.
 * </p>
 * @return The {@link NavigatorProvider} used by this NavController.
 */
NavigatorProvider* NavController::getNavigatorProvider() const{
    return mNavigatorProvider;
}

/**
 * Adds an {@link OnNavigatedListener} to this controller to receive events when
 * the controller navigates to a new destination.
 *
 * <p>The current destination, if any, will be immediately sent to your listener.</p>
 *
 * @param listener the listener to receive events
 */
void NavController::addOnNavigatedListener(OnNavigatedListener listener) {
    // Inform the new listener of our current state, if any
    if (!mBackStack.isEmpty()) {
        listener(*this, mBackStack.peekLast());
    }
    mOnNavigatedListeners.push_back(listener);
}

/**
 * Removes an {@link OnNavigatedListener} from this controller. It will no longer
 * receive navigation events.
 *
 * @param listener the listener to remove
 */
void NavController::removeOnNavigatedListener(OnNavigatedListener listener) {
    mOnNavigatedListeners.remove(listener);
}

/**
 * Attempts to pop the controller's back stack. Analogous to when the user presses
 * the system {@link android.view.KeyEvent#KEYCODE_BACK Back} button when the associated
 * navigation host has focus.
 *
 * @return true if the stack was popped, false otherwise
 */
bool NavController::popBackStack() {
    if (mBackStack.isEmpty()) {
        throw new IllegalArgumentException("NavController back stack is empty");
    }
    bool popped = false;
    while (!mBackStack.isEmpty()) {
        popped = mBackStack.removeLast().getNavigator().popBackStack();
        if (popped) {
            break;
        }
    }
    return popped;
}


/**
 * Attempts to pop the controller's back stack back to a specific destination.
 *
 * @param destinationId The topmost destination to retain
 * @param inclusive Whether the given destination should also be popped.
 *
 * @return true if the stack was popped at least once, false otherwise
 */
bool NavController::popBackStack(/*@IdRes*/int destinationId, bool inclusive) {
    if (mBackStack.isEmpty()) {
        throw new IllegalArgumentException("NavController back stack is empty");
    }
    ArrayList<NavDestination> destinationsToRemove = new ArrayList<>();
    Iterator<NavDestination> iterator = mBackStack.descendingIterator();
    while (iterator.hasNext()) {
        NavDestination destination = iterator.next();
        if (inclusive || destination.getId() != destinationId) {
            destinationsToRemove.add(destination);
        }
        if (destination.getId() == destinationId) {
            break;
        }
    }
    bool popped = false;
    iterator = destinationsToRemove.iterator();
    while (iterator.hasNext()) {
        NavDestination destination = iterator.next();
        // Skip destinations already removed by a previous popBackStack operation
        while (!mBackStack.isEmpty() && mBackStack.peekLast().getId() != destination.getId()) {
            if (iterator.hasNext()) {
                destination = iterator.next();
            } else {
                destination = nullptr;
                break;
            }
        }
        if (destination != nullptr) {
            popped = destination.getNavigator().popBackStack() || popped;
        }
    }
    return popped;
}

/**
 * Attempts to navigate up in the navigation hierarchy. Suitable for when the
 * user presses the "Up" button marked with a left (or start)-facing arrow in the upper left
 * (or starting) corner of the app UI.
 *
 * <p>The intended behavior of Up differs from {@link #popBackStack() Back} when the user
 * did not reach the current destination from the application's own task. e.g. if the user
 * is viewing a document or link in the current app in an activity hosted on another app's
 * task where the user clicked the link. In this case the current activity (determined by the
 * context used to create this NavController) will be {@link Activity#finish() finished} and
 * the user will be taken to an appropriate destination in this app on its own task.</p>
 *
 * @return true if navigation was successful, false otherwise
 */
bool NavController::navigateUp() {
    if (mBackStack.size() == 1) {
        // If there's only one entry, then we've deep linked into a specific destination
        // on another task so we need to find the parent and start our task from there
        NavDestination* currentDestination = getCurrentDestination();
        int destId = currentDestination->getId();
        NavGraph* parent = currentDestination->getParent();
        while (parent != nullptr) {
            if (parent->getStartDestination() != destId) {
                TaskStackBuilder parentIntents = new NavDeepLinkBuilder(NavController.this)
                        .setDestination(parent.getId())
                        .createTaskStackBuilder();
                parentIntents.startActivities();
                if (mActivity != nullptr) {
                    mActivity->finish();
                }
                return true;
            }
            destId = parent.getId();
            parent = parent.getParent();
        }
        // We're already at the startDestination of the graph so there's no 'Up' to go to
        return false;
    } else {
        return popBackStack();
    }
}

void NavController::dispatchOnNavigated(NavDestination destination) {
    for (OnNavigatedListener listener : mOnNavigatedListeners) {
        listener(*this, *destination);
    }
}

/**
 * Sets the {@link NavGraph navigation graph} as specified in the application manifest.
 *
 * <p>Applications may declare a graph resource in their manifest instead of declaring
 * or passing this data to each host or controller:</p>
 *
 * <pre class="prettyprint">
 *     <meta-data android:name="android.nav.graph" android:resource="@xml/my_nav_graph" />
 * </pre>
 *
 * <p>The inflated graph can be retrieved via {@link #getGraph()}. Calling this will have no
 * effect if there is no metadata graph specified.</p>
 *
 * @see NavInflater#METADATA_KEY_GRAPH
 * @see NavInflater#inflateMetadataGraph()
 * @see #getGraph
 */
void NavController::setMetadataGraph() {
    NavGraph* metadataGraph = getNavInflater().inflateMetadataGraph();
    if (metadataGraph != nullptr) {
        setGraph(metadataGraph);
    }
}

/**
 * Returns the {@link NavInflater inflater} for this controller.
 *
 * @return inflater for loading navigation resources
 */
NavInflater& NavController::getNavInflater() {
    if (mInflater == nullptr) {
        mInflater = new NavInflater(mContext, mNavigatorProvider);
    }
    return *mInflater;
}

/**
 * Sets the {@link NavGraph navigation graph} to the specified resource.
 * Any current navigation graph data will be replaced.
 *
 * <p>The inflated graph can be retrieved via {@link #getGraph()}.</p>
 *
 * @param graphResId resource id of the navigation graph to inflate
 *
 * @see #getNavInflater()
 * @see #setGraph(NavGraph)
 * @see #getGraph
 */
void NavController::setGraph(const std::string& graphResId) {
    mGraph = getNavInflater().inflate(graphResId);
    mGraphId = graphResId;
    onGraphCreated();
}

/**
 * Sets the {@link NavGraph navigation graph} to the specified graph.
 * Any current navigation graph data will be replaced.
 *
 * <p>The graph can be retrieved later via {@link #getGraph()}.</p>
 *
 * @param graph graph to set
 * @see #setGraph(int)
 * @see #getGraph
 */
void NavController::setGraph(NavGraph* graph) {
    mGraph = graph;
    mGraphId = 0;
    onGraphCreated();
}

void NavController::onGraphCreated() {
    if (mNavigatorStateToRestore != null) {
        ArrayList<String> navigatorNames = mNavigatorStateToRestore.getStringArrayList(
                KEY_NAVIGATOR_STATE_NAMES);
        if (navigatorNames != null) {
            for (String name : navigatorNames) {
                Navigator navigator = mNavigatorProvider.getNavigator(name);
                Bundle bundle = mNavigatorStateToRestore.getBundle(name);
                if (bundle != null) {
                    navigator.onRestoreState(bundle);
                }
            }
        }
    }
    if (mBackStackToRestore != null) {
        for (int destinationId : mBackStackToRestore) {
            NavDestination node = findDestination(destinationId);
            if (node == null) {
                throw new IllegalStateException("unknown destination during restore: "
                        + mContext.getResources().getResourceName(destinationId));
            }
            mBackStack.add(node);
        }
        mBackStackToRestore = null;
    }
    if (mGraph != null && mBackStack.isEmpty()) {
        bool deepLinked = mActivity != null && onHandleDeepLink(mActivity.getIntent());
        if (!deepLinked) {
            // Navigate to the first destination in the graph
            // if we haven't deep linked to a destination
            mGraph.navigate(null, null);
        }
    }
}

/**
 * Checks the given Intent for a Navigation deep link and navigates to the deep link if present.
 * This is called automatically for you the first time you set the graph if you've passed in an
 * {@link Activity} as the context when constructing this NavController, but should be manually
 * called if your Activity receives new Intents in {@link Activity#onNewIntent(Intent)}.
 * <p>
 * The types of Intents that are supported include:
 * <ul>
 *     <ol>Intents created by {@link NavDeepLinkBuilder} or
 *     {@link #createDeepLink()}. This assumes that the current graph shares
 *     the same hierarchy to get to the deep linked destination as when the deep link was
 *     constructed.</ol>
 *     <ol>Intents that include a {@link Intent#getData() data Uri}. This Uri will be checked
 *     against the Uri patterns added via {@link NavDestination#addDeepLink(String)}.</ol>
 * </ul>
 * <p>The {@link #getGraph() navigation graph} should be set before calling this method.</p>
 * @param intent The Intent that may contain a valid deep link
 * @return True if the navigation controller found a valid deep link and navigated to it.
 * @see NavDestination#addDeepLink(String)
 */
bool NavController::onHandleDeepLink(/*Nullable*/Intent* intent) {
    if (intent == nullptr) {
        return false;
    }
    Bundle extras = intent.getExtras();
    int[] deepLink = extras != null ? extras.getIntArray(KEY_DEEP_LINK_IDS) : null;
    Bundle bundle = new Bundle();
    Bundle deepLinkExtras = extras != null ? extras.getBundle(KEY_DEEP_LINK_EXTRAS) : null;
    if (deepLinkExtras != null) {
        bundle.putAll(deepLinkExtras);
    }
    if ((deepLink == null || deepLink.length == 0) && intent.getData() != null) {
        Pair<NavDestination, Bundle> matchingDeepLink = mGraph.matchDeepLink(intent.getData());
        if (matchingDeepLink != null) {
            deepLink = matchingDeepLink.first.buildDeepLinkIds();
            bundle.putAll(matchingDeepLink.second);
        }
    }
    if (deepLink == null || deepLink.length == 0) {
        return false;
    }
    bundle.putParcelable(KEY_DEEP_LINK_INTENT, intent);
    int flags = intent.getFlags();
    if ((flags & Intent.FLAG_ACTIVITY_NEW_TASK) != 0
            && (flags & Intent.FLAG_ACTIVITY_CLEAR_TASK) == 0) {
        // Someone called us with NEW_TASK, but we don't know what state our whole
        // task stack is in, so we need to manually restart the whole stack to
        // ensure we're in a predictably good state.
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
        TaskStackBuilder taskStackBuilder = TaskStackBuilder
                .create(mContext)
                .addNextIntentWithParentStack(intent);
        taskStackBuilder.startActivities();
        if (mActivity != null) {
            mActivity.finish();
        }
        return true;
    }
    if ((flags & Intent.FLAG_ACTIVITY_NEW_TASK) != 0) {
        // Start with a cleared task starting at our root when we're on our own task
        if (!mBackStack.isEmpty()) {
            navigate(mGraph.getStartDestination(), bundle, new NavOptions.Builder()
                    .setPopUpTo(mGraph.getId(), true)
                    .setEnterAnim(0).setExitAnim(0).build());
        }
        int index = 0;
        while (index < deepLink.length) {
            int destinationId = deepLink[index++];
            NavDestination* node = findDestination(destinationId);
            FATAL_IF(node == nullptr,"unknown destination during deep link: %s"，
                    NavDestination::getDisplayName(mContext, destinationId).c_str());
            }
            node->navigate(bundle, new NavOptions.Builder().setEnterAnim(0).setExitAnim(0).build());
        }
        return true;
    }
    // Assume we're on another apps' task and only start the final destination
    NavGraph* graph = mGraph;
    for (int i = 0; i < deepLink.length; i++) {
        int destinationId = deepLink[i];
        NavDestination* node = i == 0 ? mGraph : graph->findNode(destinationId);
        FATAL_IF(node == nullptr,"unknown destination during deep link: %s"
                 NavDestination::getDisplayName(mContext, destinationId).c_str());
        }
        if (i != deepLink.length - 1) {
            // We're not at the final NavDestination yet, so keep going through the chain
            graph = (NavGraph*) node;
        } else {
            // Navigate to the last NavDestination, clearing any existing destinations
            node->navigate(bundle, new NavOptions.Builder()
                    .setPopUpTo(mGraph.getId(), true)
                    .setEnterAnim(0).setExitAnim(0).build());
        }
    }
    return true;
}

/**
 * Gets the topmost navigation graph associated with this NavController.
 *
 * @see #setGraph(int)
 * @see #setGraph(NavGraph)
 * @see #setMetadataGraph()
 */
NavGraph* NavController::getGraph() const{
    return mGraph;
}

/**
 * Gets the current destination.
 */
NavDestination* NavController::getCurrentDestination() {
    return mBackStack.peekLast();
}

NavDestination* NavController::findDestination(/*@IdRes*/ int destinationId) {
    if (mGraph == nullptr) {
        return nullptr;
    }
    if (mGraph->getId() == destinationId) {
        return mGraph;
    }
    NavDestination* currentNode = mBackStack.empty() ? mGraph : mBackStack.peekLast();
    NavGraph* currentGraph = currentNode instanceof NavGraph
            ? (NavGraph*) currentNode
            : currentNode->getParent();
    return currentGraph->findNode(destinationId);
}

/**
 * Navigate to a destination from the current navigation graph. This supports both navigating
 * via an {@link NavDestination#getAction(int) action} and directly navigating to a destination.
 *
 * @param resId an {@link NavDestination#getAction(int) action} id or a destination id to
 *              navigate to
 */
void NavController::navigate(/*@IdRes*/ int resId) {
    navigate(resId, nullptr);
}

/**
 * Navigate to a destination from the current navigation graph. This supports both navigating
 * via an {@link NavDestination#getAction(int) action} and directly navigating to a destination.
 *
 * @param resId an {@link NavDestination#getAction(int) action} id or a destination id to
 *              navigate to
 * @param args arguments to pass to the destination
 */
void NavController::navigate(/*@IdRes*/ int resId,/*Nullable*/Bundle* args) {
    navigate(resId, args, nullptr);
}

/**
 * Navigate to a destination from the current navigation graph. This supports both navigating
 * via an {@link NavDestination#getAction(int) action} and directly navigating to a destination.
 *
 * @param resId an {@link NavDestination#getAction(int) action} id or a destination id to
 *              navigate to
 * @param args arguments to pass to the destination
 * @param navOptions special options for this navigation operation
 */
void NavController::navigate(/*@IdRes*/ int resId,/*Nullable*/Bundle* args,NavOptions* navOptions) {
    NavDestination* currentNode = mBackStack.empty() ? mGraph : mBackStack.back();
    FATAL_IF(currentNode == nullptr,"no current navigation node");
    int destId = resId;
    NavAction* navAction = currentNode->getAction(resId);
    if (navAction != nullptr) {
        if (navOptions == nullptr) {
            navOptions = navAction->getNavOptions();
        }
        destId = navAction.getDestinationId();
    }
    if (destId == 0 && navOptions && navOptions->getPopUpTo() != 0) {
        popBackStack(navOptions->getPopUpTo(), navOptions->isPopUpToInclusive());
        return;
    }

    FATAL_IF(destId == 0,"Destination id == 0 can only be used" " in conjunction with navOptions.popUpTo != 0");

    NavDestination* node = findDestination(destId);
    if (node == nullptr) {
        const std::string dest = NavDestination::getDisplayName(mContext, destId);
        /*FATAL("navigation destination %s " + dest
                + (navAction != null
                ? " referenced from action " + NavDestination.getDisplayName(mContext, resId)
                : "")
                + " is unknown to this NavController");*/
    }
    if (navOptions != nullptr) {
        if (navOptions->shouldClearTask()) {
            // Start with a clean slate
            popBackStack(mGraph->getId(), true);
        } else if (navOptions->getPopUpTo() != 0) {
            popBackStack(navOptions->getPopUpTo(), navOptions->isPopUpToInclusive());
        }
    }
    node->navigate(args, navOptions);
}

/**
 * Navigate via the given {@link NavDirections}
 *
 * @param directions directions that describe this navigation operation
 */
void NavController::navigate(/*@NonNull*/ NavDirections& directions) {
    navigate(directions.getActionId(), directions.getArguments());
}

/**
 * Navigate via the given {@link NavDirections}
 *
 * @param directions directions that describe this navigation operation
 */
void NavController::navigate(/*@NonNull*/ NavDirections& directions,/*Nullable*/ NavOptions* navOptions) {
    navigate(directions.getActionId(), directions.getArguments(), navOptions);
}
/**
 * Create a deep link to a destination within this NavController.
 *
 * @return a {@link NavDeepLinkBuilder} suitable for constructing a deep link
 */
NavDeepLinkBuilder* NavController::createDeepLink() {
    return new NavDeepLinkBuilder(this);
}

/**
 * Saves all navigation controller state to a Bundle.
 *
 * <p>State may be restored from a bundle returned from this method by calling
 * {@link #restoreState(Bundle)}. Saving controller state is the responsibility
 * of a {@link NavHost}.</p>
 *
 * @return saved state for this controller
 */
Bundle* NavController::saveState() {
    Bundle* b = nullptr;
#if 0
    if (mGraphId != 0) {
        b = new Bundle();
        b.putInt(KEY_GRAPH_ID, mGraphId);
    }
    ArrayList<String> navigatorNames = new ArrayList<>();
    Bundle navigatorState = new Bundle();
    for (Map.Entry<String, Navigator<? extends NavDestination>> entry :
            mNavigatorProvider.getNavigators().entrySet()) {
        String name = entry.getKey();
        Bundle savedState = entry.getValue().onSaveState();
        if (savedState != null) {
            navigatorNames.add(name);
            navigatorState.putBundle(name, entry.getValue().onSaveState());
        }
    }
    if (!navigatorNames.isEmpty()) {
        if (b == null) {
            b = new Bundle();
        }
        navigatorState.putStringArrayList(KEY_NAVIGATOR_STATE_NAMES, navigatorNames);
        b.putBundle(KEY_NAVIGATOR_STATE, navigatorState);
    }
    if (!mBackStack.isEmpty()) {
        if (b == null) {
            b = new Bundle();
        }
        int[] backStack = new int[mBackStack.size()];
        int index = 0;
        for (NavDestination destination : mBackStack) {
            backStack[index++] = destination.getId();
        }
        b.putIntArray(KEY_BACK_STACK_IDS, backStack);
    }
#endif
    return b;
}

/**
 * Restores all navigation controller state from a bundle.
 *
 * <p>State may be saved to a bundle by calling {@link #saveState()}.
 * Restoring controller state is the responsibility of a {@link NavHost}.</p>
 *
 * @param navState state bundle to restore
 */
void NavController::restoreState(/*@Nullable*/Bundle* navState) {
    if (navState == nullptr) {
        return;
    }

    mGraphId = navState->getInt(KEY_GRAPH_ID);
    mNavigatorStateToRestore = navState.getBundle(KEY_NAVIGATOR_STATE);
    mBackStackToRestore = navState.getIntArray(KEY_BACK_STACK_IDS);
    if (mGraphId != 0) {
        // Set the graph right away, onGraphCreated will handle restoring the
        // rest of the saved state
        setGraph(mGraphId);
    }
}
}/*endof namespace*/
#endif
