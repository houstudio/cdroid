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
#include <navigation/navcontroller.h>
#include <navigation/navaction.h>
#include <navigation/navgraph.h>
#include <navigation/navinflater.h>
#include <navigation/navbackstackentry.h>
#include <navigation/navigatorstate.h>
#include <navigation/navigator.h>
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

NavController::~NavController(){
    for(auto& kv : mNavigatorStates) delete kv.second;
    mNavigatorStates.clear();
    // Owns the inflated NavGraph (set via setGraph) and, transitively, every NavDestination in it
    // (~NavGraph frees its nodes; NavDestination's virtual dtor frees deep links/actions/arguments
    // and recurses into nested NavGraph children). NavHostFragment owns this NavController.
    delete mGraph;
}

// --- navigator-state pop model (androidx NavControllerImpl navigatorState + handlers) ---

NavigatorState* NavController::getOrCreateNavigatorState(Navigator* navigator){
    if(!navigator) return nullptr;
    auto it = mNavigatorStates.find(navigator);
    if(it != mNavigatorStates.end()) return it->second;
    NavigatorState* state = new NavigatorState(this, navigator);
    mNavigatorStates[navigator] = state;
    navigator->onAttach(state);
    return state;
}

void NavController::push(NavigatorState* state, NavBackStackEntry* entry){
    // androidx NavControllerImpl.push: the handler mutates the merged back queue first
    // (addEntryToBackStack), then the navigator's own state is appended (addInternal). The handler
    // is installed only during navigate(); a push outside that scope is ignored.
    if(mAddToBackStackHandler){
        mAddToBackStackHandler(entry);
        state->addInternal(entry);
    } else {
        LOGD("NavController.push: ignoring push of entry %p outside navigate()", (void*)entry);
    }
}

void NavController::pop(NavigatorState* state, NavBackStackEntry* popUpTo, bool saveState){
    // androidx NavControllerImpl.pop: the handler mutates the merged back queue first
    // (popEntryFromBackStack), then the navigator's own state is trimmed (popInternal). The handler
    // is installed only during popBackStack().
    if(mPopFromBackStackHandler){
        mPopFromBackStackHandler(popUpTo);
        state->popInternal(popUpTo);
    }
    (void)saveState;
}

void NavController::setGraph(NavGraph* graph, Bundle* startDestinationArgs){
    LOGD("NavController.setGraph graph=%p startRoute='%s' startDestId=%d",
         graph, graph ? graph->getStartDestinationRoute().c_str() : "(null)",
         graph ? graph->getStartDestination() : 0);
    mGraph = graph;
    if(!mGraph) return;
    // androidx NavControllerImpl.setGraph: clear saved back-stack chains from a previous graph
    // so stale state doesn't leak into the new graph's navigation.
    mBackStackMap.clear();
    mBackStackStates.clear();
    // onGraphCreated (androidx NavControllerImpl.kt:964-973): attach a NavigatorState to every
    // registered navigator so its navigate()/popBackStack() can drive this controller's back queue
    // through the push/pop handlers.
    if(auto* provider = dynamic_cast<SimpleNavigatorProvider*>(mNavigatorProvider)){
        for(auto& kv : provider->getNavigators()){
            getOrCreateNavigatorState(kv.second);
        }
    }
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

void NavController::navigate(const std::string& route, Bundle* args, NavOptions* options){
    NavDestination* node = findDestination(route);
    if(!node) return;
    // androidx: extract route params (e.g. "detail/42" → {id:42}) and merge with caller args.
    Bundle* routeArgs = node->matchRouteArgs(route);
    if(routeArgs && args) routeArgs->putAll(*args);  // caller overrides route params
    Bundle* merged = routeArgs ? routeArgs : args;
    navigate(node, merged, options);
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
    // androidx NavControllerImpl.navigate restoreState path (:1201-1202): if NavOptions asks to
    // restore AND a saved chain is keyed by this destination, restore it (re-runs the saved
    // transactions with original entry ids → FragmentNavigator.restoreBackStack) instead of pushing.
    if(options && options->shouldRestoreState() && mBackStackMap.count(node->getId())){
        if(restoreStateInternal(node->getId(), args, options)) return;
    }
    // launchSingleTop: skip if the destination is already on top (androidx NavOptions singleTop).
    // For a NavGraph target, singleTop only when backStack[nodeIndex..top] exactly matches the
    // graph's childHierarchy ids (androidx launchSingleTopInternal:1234).
    if(options && options->shouldLaunchSingleTop() && !mBackStack.empty()){
        NavGraph* graphTarget = dynamic_cast<NavGraph*>(node);
        if(graphTarget){
            int nodeIndex = -1;
            for(int i = (int)mBackStack.size() - 1; i >= 0; --i){
                if(mBackStack[i]->getDestination() == node){ nodeIndex = i; break; }
            }
            if(nodeIndex >= 0){
                std::vector<NavDestination*> children = graphTarget->childHierarchy();
                bool match = ((int)mBackStack.size() - nodeIndex == (int)children.size());
                for(size_t k = 0; match && k < children.size(); ++k){
                    if(mBackStack[nodeIndex + k]->getDestination() != children[k]) match = false;
                }
                if(match){
                    LOGD("NavController.navigate singleTop: graph '%s' childHierarchy on top, skip", node->getRoute().c_str());
                    return;
                }
            }
        } else if(mBackStack.back()->getDestination() == node){
            LOGD("NavController.navigate singleTop: '%s' already on top, skip", node->getRoute().c_str());
            return;
        }
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
    // androidx: merge this destination's default arguments (from <argument android:defaultValue>).
    args = node->addInDefaultArgs(args);
    NavBackStackEntry* leafEntry = new NavBackStackEntry(node, args);
    // androidx navigateInternal: install the push handler, dispatch through the destination's
    // Navigator (its navigate(entries) commits the fragment and calls state.push(leafEntry), which
    // fires the handler = addEntryToBackStack, then appends to the navigator's own state), clear it.
    Navigator& navigator = node->getNavigator();
    getOrCreateNavigatorState(&navigator);
    std::vector<NavBackStackEntry*> entries = { leafEntry };
    mAddToBackStackHandler = [this, node, args](NavBackStackEntry* e){ addEntryToBackStack(node, args, e); };
    navigator.navigate(entries, options, nullptr);
    mAddToBackStackHandler = nullptr;
    dispatchOnDestinationChanged(node, args);
    updateBackStackLifecycle();
    (void)options;
}

void NavController::navigate(NavDeepLinkRequest* /*request*/, NavOptions* /*options*/){
    // MVP: deep-link navigation deferred.
}

bool NavController::popBackStack(){
    // androidx popBackStack() = popBackStack(currentDestination.id, inclusive = true): pop the
    // single top entry via its navigator's entry-based popBackStack (user back — no save).
    if(mBackStack.empty()) return false;
    NavDestination* dest = mBackStack.back()->getDestination();
    if(!dest) return false;
    std::vector<Navigator*> popOperations = { &dest->getNavigator() };
    return executePopOperations(popOperations, false);
}

bool NavController::popBackStack(const std::string& route, bool inclusive, bool saveState){
    // androidx popBackStackInternal(route, inclusive, saveState): build popOperations walking the
    // back stack top-down to the topmost entry whose destination has the route.
    std::vector<Navigator*> popOperations;
    bool found = false;
    for(int i = (int)mBackStack.size() - 1; i >= 0; --i){
        NavDestination* d = mBackStack[i]->getDestination();
        if(d && (d->hasRoute(route) || d->matchRoute(route))){
            if(inclusive) popOperations.push_back(&d->getNavigator());
            found = true;
            break;
        }
        if(d) popOperations.push_back(&d->getNavigator());
    }
    if(!found) return false;
    return executePopOperations(popOperations, saveState);
}

bool NavController::popBackStack(int destinationId, bool inclusive, bool saveState){
    // androidx popBackStackInternal(destinationId, inclusive, saveState).
    std::vector<Navigator*> popOperations;
    bool found = false;
    for(int i = (int)mBackStack.size() - 1; i >= 0; --i){
        NavDestination* d = mBackStack[i]->getDestination();
        if(d && d->getId() == destinationId){
            if(inclusive) popOperations.push_back(&d->getNavigator());
            found = true;
            break;
        }
        if(d) popOperations.push_back(&d->getNavigator());
    }
    if(!found) return false;
    return executePopOperations(popOperations, saveState);
}

bool NavController::executePopOperations(std::vector<Navigator*>& popOperations, bool saveState){
    // androidx NavControllerImpl.executePopOperations (:478-546): for each navigator, pop the
    // current top entry via its entry-based popBackStack. The pop handler (popEntryFromBackStack)
    // runs only if the navigator actually popped (receivedPop); stop on the first that doesn't.
    // With saveState, each popped entry is captured into a NavBackStackEntryState chain (Level A).
    bool popped = false;
    std::vector<NavBackStackEntryState> savedChain; // built bottom-to-top (prepend as we pop top-down)
    for(Navigator* navigator : popOperations){
        if(!navigator || mBackStack.empty()) break;
        NavBackStackEntry* topEntry = mBackStack.back();
        bool receivedPop = false;
        mPopFromBackStackHandler = [this, &receivedPop, saveState, &savedChain](NavBackStackEntry* e){
            if(saveState){
                NavDestination* d = e->getDestination();
                NavBackStackEntryState st(e->getId(), d ? d->getId() : 0, e->getArguments());
                e->saveState(st.savedState); // capture SavedStateRegistry contents (androidx)
                savedChain.insert(savedChain.begin(), std::move(st));
            }
            popEntryFromBackStack(e);
            receivedPop = true;
        };
        navigator->popBackStack(topEntry, saveState);
        mPopFromBackStackHandler = nullptr;
        if(!receivedPop) break;   // navigator refused (did not call state.pop) — stop, no desync
        popped = true;
    }
    if(saveState && !savedChain.empty()){
        // androidx :518-541: index the saved chain by its bottom entry's id and map that entry's
        // destination id to it (so navigate(destId, restoreState) can find + restore the chain).
        const std::string chainId = savedChain.front().id;
        int chainDestId = savedChain.front().destinationId;
        mBackStackStates[chainId] = std::move(savedChain);
        mBackStackMap[chainDestId] = chainId;
    }
    if(popped){
        if(!mBackStack.empty()){
            NavBackStackEntry* top = mBackStack.back();
            dispatchOnDestinationChanged(top->getDestination(), top->getArguments());
        }
        updateBackStackLifecycle();
    }
    return popped;
}

bool NavController::restoreStateInternal(int destinationId, Bundle* /*args*/, NavOptions* options){
    // androidx NavControllerImpl.restoreStateInternal (:1282-1298) + instantiateBackStack +
    // executeRestoreState: consume the saved chain keyed by destinationId, rebuild each
    // NavBackStackEntry preserving its original id (so the navigator-side savedIds matches and
    // FragmentNavigator.restoreBackStack fires), then dispatch each rebuilt entry through its
    // navigator's navigate (Level B restore).
    auto mit = mBackStackMap.find(destinationId);
    if(mit == mBackStackMap.end()) return false;
    const std::string chainId = mit->second;
    // Consume-once: strip every map entry pointing at this chain, then take the chain.
    for(auto it = mBackStackMap.begin(); it != mBackStackMap.end(); ){
        if(it->second == chainId) it = mBackStackMap.erase(it); else ++it;
    }
    auto sit = mBackStackStates.find(chainId);
    if(sit == mBackStackStates.end()) return false;
    std::vector<NavBackStackEntryState> chain = std::move(sit->second);
    mBackStackStates.erase(sit);
    if(!mGraph) return false;
    // Rebuild entries (preserving ids), group by navigator, and dispatch each group forward — the
    // FragmentNavigator.navigate restoreState gate matches the saved id → restoreBackStack (Level B).
    for(NavBackStackEntryState& st : chain){
        NavDestination* dest = mGraph->findNode(st.destinationId);
        if(!dest) continue;
        NavBackStackEntry* entry = new NavBackStackEntry(dest, st.arguments);
        entry->mId = st.id; // preserve original id (friend access) — navigator savedIds match
        entry->restoreState(st.savedState); // restore SavedStateRegistry contents (androidx)
        Navigator& navigator = dest->getNavigator();
        getOrCreateNavigatorState(&navigator);
        std::vector<NavBackStackEntry*> entries = { entry };
        mAddToBackStackHandler = [this, dest, &st](NavBackStackEntry* e){ addEntryToBackStack(dest, st.arguments, e); };
        navigator.navigate(entries, options, nullptr); // options has shouldRestoreState → FragmentNavigator restoreState gate fires
        mAddToBackStackHandler = nullptr;
        dispatchOnDestinationChanged(dest, st.arguments);
    }
    updateBackStackLifecycle();
    return true;
}

bool NavController::navigateUp(){
    return popBackStack();
}

void NavController::addOnDestinationChangedListener(const OnDestinationChangedListener& listener){
    mOnDestinationChangedListeners.push_back(listener);
    // Inform the new listener of the current destination, if any (androidx NavControllerImpl:
    // on add, dispatch the current top entry to just this listener). Dispatch via the just-stored
    // copy: CallbackBase::operator() is non-const, and the stored element is a mutable value.
    if(!mBackStack.empty()){
        NavBackStackEntry* top = mBackStack.back();
        mOnDestinationChangedListeners.back()(this, top->getDestination(), top->getArguments());
    }
}
void NavController::removeOnDestinationChangedListener(const OnDestinationChangedListener& listener){
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
    for(auto& l : mOnDestinationChangedListeners){
        l(this, destination, args);
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
    for(NavBackStackEntry* e : mBackStack){
        if(!routes.empty())
            routes += ",";
        routes += (e->getDestination() ? e->getDestination()->getRoute() : "?");
    }
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
