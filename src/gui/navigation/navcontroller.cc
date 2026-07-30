#include <navigation/navcontroller.h>
#include <navigation/navgraph.h>
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
    mGraph = graph;
    if(!mGraph) return;
    if(!mGraph->getStartDestinationRoute().empty()){
        navigate(mGraph->getStartDestinationRoute(), nullptr);
    }else if(mGraph->getStartDestination() != 0){
        navigate(mGraph->getStartDestination(), startDestinationArgs, nullptr);
    }
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
