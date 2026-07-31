#include <navigation/navcontroller.h>
#include <navigation/navaction.h>
#include <navigation/navgraph.h>
#include <navigation/navinflater.h>
#include <navigation/navbackstackentry.h>
#include <navigation/navoptions.h>
#include <navigation/navdeeplinkrequest.h>
#include <navigation/simplenavigatorprovider.h>
#include <navigation/navgraphnavigator.h>
#include <algorithm>

namespace cdroid{

NavController::NavController(Context* context) : mContext(context){
    // Register the graph navigator by default.
    mNavigatorProvider = new SimpleNavigatorProvider();
    mNavigatorProvider->addNavigator(new NavGraphNavigator(context));
}

void NavController::setGraph(NavGraph* graph, Bundle* startDestinationArgs){
    LOGD("NavController.setGraph graph=%p startRoute='%s' startDestId=%d",
         graph, graph ? graph->getStartDestinationRoute().c_str() : "(null)",
         graph ? graph->getStartDestination() : 0);
    mGraph = graph;
    if(!mGraph) return;
    if(!mGraph->getStartDestinationRoute().empty()){
        navigate(mGraph->getStartDestinationRoute(), nullptr);
    }else if(mGraph->getStartDestination() != 0){
        navigate(mGraph->getStartDestination(), startDestinationArgs, nullptr);
    }
}

void NavController::setGraph(const std::string& graphRef, Bundle* startDestinationArgs){
    NavInflater inflater(mContext, mNavigatorProvider);
    NavGraph* graph = inflater.inflate(graphRef);
    setGraph(graph, startDestinationArgs);
}

// Search a graph and recurse into nested child <navigation> graphs for a route (androidx
// resolveDest traverses the hierarchy; NavGraph::findNode only walks searchParents upward).
static NavDestination* searchGraphForRoute(NavGraph* g, const std::string& route){
    NavDestination* d = g->findNode(route, false);
    if(d) return d;
    for(auto it = g->begin(); it != g->end(); ++it){
        NavGraph* child = dynamic_cast<NavGraph*>((*it).second);
        if(child){
            NavDestination* found = searchGraphForRoute(child, route);
            if(found) return found;
        }
    }
    return nullptr;
}

NavDestination* NavController::findDestination(const std::string& route){
    return mGraph ? searchGraphForRoute(mGraph, route) : nullptr;
}

NavDestination* NavController::getCurrentDestination(){
    NavBackStackEntry* entry = getCurrentBackStackEntry();
    return entry ? entry->getDestination() : nullptr;
}

NavBackStackEntry* NavController::getCurrentBackStackEntry() const{
    return mBackStack.empty() ? nullptr : mBackStack.back();
}

void NavController::navigate(const std::string& route, NavOptions* options){
    NavDestination* node = findDestination(route);
    if(!node) return;
    navigate(node, nullptr, options);
}

void NavController::navigate(int resId, Bundle* args, NavOptions* options){
    if(!mGraph) return;
    // resId may name an action declared on the current destination: resolve it first so the
    // action's destination id + NavOptions (e.g. popUpTo, singleTop) take effect (androidx).
    NavDestination* currentDest = getCurrentDestination();
    int destId = resId;
    NavAction* action = currentDest ? currentDest->getAction(resId) : nullptr;
    if(action){
        destId = action->getDestinationId();
        if(options == nullptr) options = action->getNavOptions();
        LOGD("NavController.navigate action 0x%x -> destId=0x%x", resId, destId);
    }
    NavDestination* node = mGraph->findNode(destId);
    if(!node) return;
    navigate(node, args, options);
}

void NavController::navigate(NavDestination* node, Bundle* args, NavOptions* options){
    LOGD("NavController.navigate route='%s'", node ? node->getRoute().c_str() : "(null)");
    // launchSingleTop: if this destination is already on top of the back stack, skip
    // navigation so we don't push a duplicate entry (androidx NavOptions singleTop).
    if(options && options->shouldLaunchSingleTop() && !mBackStack.empty()
       && mBackStack.back()->getDestination() == node){
        LOGD("NavController.navigate singleTop: '%s' already on top, skip", node->getRoute().c_str());
        return;
    }
    // popUpTo: pop the back stack up to (optionally including) the given destination before
    // navigating (androidx NavOptions popUpTo). The id form is set by <action> inflation; the
    // route form is the programmatic API. saveState/restoreState are deferred (SavedState ser).
    if(options){
        if(options->getPopUpToId() != -1){
            LOGD("NavController.navigate popUpTo id=0x%x inclusive=%d",
                 options->getPopUpToId(), options->isPopUpToInclusive());
            popBackStack(options->getPopUpToId(), options->isPopUpToInclusive(), false);
        } else if(!options->getPopUpToRoute().empty()){
            LOGD("NavController.navigate popUpTo route='%s' inclusive=%d",
                 options->getPopUpToRoute().c_str(), options->isPopUpToInclusive());
            popBackStack(options->getPopUpToRoute(), options->isPopUpToInclusive(), false);
        }
    }
    // V2: push the leaf + any parent-graph entries not already on the stack, link them, then
    // let updateBackStackLifecycle drive per-entry lifecycle (top RESUMED / graph STARTED / off CREATED).
    NavBackStackEntry* leafEntry = new NavBackStackEntry(node, args);
    addEntryToBackStack(node, args, leafEntry);
    node->navigate(args, options);
    dispatchOnDestinationChanged(node, args);
    updateBackStackLifecycle();
    (void)options;
}

void NavController::navigate(NavDeepLinkRequest* /*request*/, NavOptions* /*options*/){
    // MVP: deep-link navigation deferred.
}

bool NavController::popBackStack(){
    if(mBackStack.empty()) return false;
    NavBackStackEntry* entry = mBackStack.back();
    NavDestination* dest = entry->getDestination();
    if(dest) dest->getNavigator().popBackStack();
    popEntryFromBackStack(entry);
    if(!mBackStack.empty()){
        NavBackStackEntry* top = mBackStack.back();
        dispatchOnDestinationChanged(top->getDestination(), top->getArguments());
    }
    updateBackStackLifecycle();
    return true;
}

bool NavController::popBackStack(const std::string& route, bool inclusive, bool /*saveState*/){
    int targetIndex = -1;
    for(int i = (int)mBackStack.size() - 1; i >= 0; i--){
        NavDestination* d = mBackStack[i]->getDestination();
        if(d && (d->hasRoute(route) || d->matchRoute(route))){ targetIndex = i; break; }
    }
    if(targetIndex < 0) return false;
    int end = inclusive ? targetIndex : targetIndex + 1;
    while((int)mBackStack.size() > end){ popBackStack(); }
    return true;
}

bool NavController::popBackStack(int destinationId, bool inclusive, bool /*saveState*/){
    int targetIndex = -1;
    for(int i = (int)mBackStack.size() - 1; i >= 0; i--){
        NavDestination* d = mBackStack[i]->getDestination();
        if(d && d->getId() == destinationId){ targetIndex = i; break; }
    }
    if(targetIndex < 0) return false;
    int end = inclusive ? targetIndex : targetIndex + 1;
    while((int)mBackStack.size() > end){ popBackStack(); }
    return true;
}

bool NavController::navigateUp(){
    return popBackStack();
}

void NavController::addOnDestinationChangedListener(OnDestinationChangedListener* listener){
    if(!listener) return;
    mOnDestinationChangedListeners.push_back(listener);
    // Inform the new listener of the current destination, if any (androidx NavControllerImpl:
    // on add, dispatch the current top entry to just this listener).
    if(!mBackStack.empty()){
        NavBackStackEntry* top = mBackStack.back();
        listener->onDestinationChanged(this, top->getDestination(), top->getArguments());
    }
}
void NavController::removeOnDestinationChangedListener(OnDestinationChangedListener* listener){
    auto it = std::find(mOnDestinationChangedListeners.begin(), mOnDestinationChangedListeners.end(), listener);
    if(it != mOnDestinationChangedListeners.end()) mOnDestinationChangedListeners.erase(it);
}
void NavController::dispatchOnDestinationChanged(NavDestination* destination, Bundle* args){
    // Never leave a bare NavGraph on top of the back stack (androidx :677-681): when a leaf is
    // popped and its parent graph becomes top, pop the graph too.
    while(!mBackStack.empty() && dynamic_cast<NavGraph*>(mBackStack.back()->getDestination())
          && mBackStack.back()->getDestination() != mGraph){
        // Pop bare child graphs, but keep the root graph at the bottom of the stack even when
        // bare (androidx never pops the root graph entry here).
        popEntryFromBackStack(mBackStack.back());
    }
    // After popping bare graphs, dispatch the actual top destination (the passed `destination`
    // may itself be a graph that we just popped — use the final top, like androidx backQueue.last).
    if(!mBackStack.empty()){
        destination = mBackStack.back()->getDestination();
        args = mBackStack.back()->getArguments();
    }
    for(OnDestinationChangedListener* l : mOnDestinationChangedListeners){
        l->onDestinationChanged(this, destination, args);
    }
}

// --- V2 nested-graph back stack helpers (androidx NavControllerImpl) ---

void NavController::linkChildToParent(NavBackStackEntry* child, NavBackStackEntry* parent){
    if(!child || !parent) return;
    mChildToParent[child] = parent;
    mParentToChildCount[parent]++;
}

NavBackStackEntry* NavController::unlinkChildFromParent(NavBackStackEntry* child){
    auto it = mChildToParent.find(child);
    if(it == mChildToParent.end()) return nullptr;
    NavBackStackEntry* parent = it->second;
    mChildToParent.erase(it);
    auto cit = mParentToChildCount.find(parent);
    if(cit != mParentToChildCount.end()){
        cit->second--;
        if(cit->second <= 0) mParentToChildCount.erase(cit);
    }
    return parent;
}

NavBackStackEntry* NavController::findBackStackEntry(int destinationId){
    for(NavBackStackEntry* e : mBackStack){
        if(e && e->getDestination() && e->getDestination()->getId() == destinationId) return e;
    }
    return nullptr;
}

void NavController::addEntryToBackStack(NavDestination* /*node*/, Bundle* args, NavBackStackEntry* leafEntry){
    // Collect parent graph entries that need to be on the stack (root -> ... -> immediate parent).
    std::vector<NavBackStackEntry*> hierarchyEntries;
    NavDestination* dest = leafEntry->getDestination();
    while(dest){
        NavGraph* parent = dest->getParent();
        if(!parent) break;
        bool onStack = false;
        for(NavBackStackEntry* e : mBackStack){
            if(e->getDestination() == parent){ onStack = true; break; }
        }
        if(!onStack){
            hierarchyEntries.insert(hierarchyEntries.begin(), new NavBackStackEntry(parent, args));
        }
        dest = parent;
    }
    // Root graph guarantee: the root graph must be the bottom of the stack — but only if the
    // walk above didn't already collect it (a leaf directly under root => root is in hierarchy).
    bool rootInHierarchy = !hierarchyEntries.empty() && hierarchyEntries.front()->getDestination() == mGraph;
    bool rootOnStack = !mBackStack.empty() && mBackStack.front()->getDestination() == mGraph;
    if(mGraph && !rootInHierarchy && !rootOnStack){
        hierarchyEntries.insert(hierarchyEntries.begin(), new NavBackStackEntry(mGraph, args));
    }
    // Push hierarchy (parent graphs first), then the leaf on top.
    for(NavBackStackEntry* ge : hierarchyEntries) mBackStack.push_back(ge);
    mBackStack.push_back(leafEntry);
    // Link each newly-added entry to its parent graph's entry (increments parentToChildCount).
    auto linkIfHasParent = [this](NavBackStackEntry* e){
        NavGraph* p = e->getDestination()->getParent();
        if(p){
            NavBackStackEntry* pe = findBackStackEntry(p->getId());
            if(pe) linkChildToParent(e, pe);
        }
    };
    for(NavBackStackEntry* e : hierarchyEntries) linkIfHasParent(e);
    linkIfHasParent(leafEntry);
    LOGD("NavController.addEntryToBackStack: stack size=%d", (int)mBackStack.size());
    std::string routes;
    for(NavBackStackEntry* e : mBackStack){ if(!routes.empty()) routes += ","; routes += (e->getDestination() ? e->getDestination()->getRoute() : "?"); }
    LOGD("NavController.backStack: [%s]", routes.c_str());
}

void NavController::popEntryFromBackStack(NavBackStackEntry* entry){
    if(mBackStack.empty() || mBackStack.back() != entry) return;
    mBackStack.pop_back();
    unlinkChildFromParent(entry);
    entry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_DESTROY);
    delete entry;
}

void NavController::updateBackStackLifecycle(){
    if(mBackStack.empty()) return;
    using S = lifecycle::Lifecycle::State;
    for(int i = (int)mBackStack.size() - 1; i >= 0; --i){
        NavBackStackEntry* e = mBackStack[i];
        NavDestination* d = e->getDestination();
        bool isGraph = dynamic_cast<NavGraph*>(d) != nullptr;
        S target;
        if(i == (int)mBackStack.size() - 1){
            target = S::RESUMED;                              // top leaf
        } else if(isGraph && mParentToChildCount.count(e) > 0){
            target = S::STARTED;                              // graph with live children
        } else {
            target = S::CREATED;                              // off-path
        }
        e->setMaxLifecycle(target);
        e->setCurrentState(target);
    }
}

}//namespace cdroid
