#include <navigation/navcontroller.h>
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

NavDestination* NavController::findDestination(const std::string& route){
    return mGraph ? mGraph->findNode(route) : nullptr;
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
    NavDestination* node = mGraph->findNode(resId);
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
    // popUpTo: pop the back stack up to (optionally including) the given route before
    // navigating (androidx NavOptions popUpTo). saveState/restoreState are deferred
    // (need SavedState serialization).
    if(options && !options->getPopUpToRoute().empty()){
        LOGD("NavController.navigate popUpTo '%s' inclusive=%d",
             options->getPopUpToRoute().c_str(), options->isPopUpToInclusive());
        popBackStack(options->getPopUpToRoute(), options->isPopUpToInclusive(), false);
    }
    // Demote the current top entry to STARTED so only one entry is RESUMED at a time
    // (androidx updateBackStackLifecycle: top = RESUMED, the rest = STARTED).
    if(!mBackStack.empty()){
        NavBackStackEntry* prev = mBackStack.back();
        LOGD("NavController: demote '%s' RESUMED->STARTED",
             prev->getDestination() ? prev->getDestination()->getRoute().c_str() : "?");
        prev->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_PAUSE);
    }
    NavBackStackEntry* entry = new NavBackStackEntry(node, args);
    mBackStack.push_back(entry);
    // Drive the entry's lifecycle up to RESUMED (MVP: no per-position maxLifecycle).
    entry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_CREATE);
    entry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_START);
    entry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_RESUME);
    // Execute via the destination's navigator (legacy 3-arg form).
    node->navigate(args, options);
    dispatchOnDestinationChanged(node, args);
    (void)options;
}

void NavController::navigate(NavDeepLinkRequest* /*request*/, NavOptions* /*options*/){
    // MVP: deep-link navigation deferred.
}

bool NavController::popBackStack(){
    if(mBackStack.empty()) return false;
    NavBackStackEntry* entry = mBackStack.back();
    NavDestination* dest = entry->getDestination();
    // Drive the destination's navigator (e.g. FragmentNavigator -> FM pop, restoring prior Fragment)
    if(dest) dest->getNavigator().popBackStack();
    mBackStack.pop_back();
    entry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_DESTROY);
    delete entry;
    if(!mBackStack.empty()){
        NavBackStackEntry* top = mBackStack.back();
        // Re-promote the new top to RESUMED (it was demoted to STARTED when pushed over).
        LOGD("NavController: promote '%s' STARTED->RESUMED",
             top->getDestination() ? top->getDestination()->getRoute().c_str() : "?");
        top->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_RESUME);
        dispatchOnDestinationChanged(top->getDestination(), top->getArguments());
    }
    return true;
}

bool NavController::popBackStack(const std::string& route, bool inclusive, bool /*saveState*/){
    int targetIndex = -1;
    for(int i = (int)mBackStack.size() - 1; i >= 0; i--){
        NavDestination* d = mBackStack[i]->getDestination();
        if(d && d->hasRoute(route)){ targetIndex = i; break; }
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
    if(listener) mOnDestinationChangedListeners.push_back(listener);
}
void NavController::removeOnDestinationChangedListener(OnDestinationChangedListener* listener){
    auto it = std::find(mOnDestinationChangedListeners.begin(), mOnDestinationChangedListeners.end(), listener);
    if(it != mOnDestinationChangedListeners.end()) mOnDestinationChangedListeners.erase(it);
}
void NavController::dispatchOnDestinationChanged(NavDestination* destination, Bundle* args){
    for(OnDestinationChangedListener* l : mOnDestinationChangedListeners){
        l->onDestinationChanged(this, destination, args);
    }
}

}//namespace cdroid
