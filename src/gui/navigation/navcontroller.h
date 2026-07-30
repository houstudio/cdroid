#ifndef __NAV_CONTROLLER_H__
#define __NAV_CONTROLLER_H__
/*********************************************************************************
 * Port of androidx.navigation.NavController (rewritten, modern route model).
 * Holds a NavGraph + a back stack of NavBackStackEntry; navigate(route) resolves a
 * destination, pushes an entry, drives its Lifecycle, and delegates execution to the
 * destination's Navigator. Keeps navigate(int) legacy overload for Navigation helper.
 *********************************************************************************/
#include <vector>
#include <string>
#include <lifecycle/lifecycle.h>
#include <lifecycle/lifecycleowner.h>
#include <lifecycle/viewmodelstore.h>
#include <core/bundle.h>
#include <navigation/navdestination.h>
namespace cdroid{
class Context;
class NavGraph;
class NavInflater;
class NavigatorProvider;
class NavBackStackEntry;
class NavOptions;
class NavDeepLinkRequest;

class NavController{
public:
    class OnDestinationChangedListener{
    public:
        virtual ~OnDestinationChangedListener() = default;
        virtual void onDestinationChanged(NavController* controller, NavDestination* destination, Bundle* arguments){}
    };

    NavController(Context* context);
    Context* getContext() const { return mContext; }
    NavigatorProvider* getNavigatorProvider() const { return mNavigatorProvider; }

    NavGraph* getGraph() const { return mGraph; }
    void setGraph(NavGraph* graph, Bundle* startDestinationArgs = nullptr);
    // Inflate the graph from a resource ref (e.g. "@navigation/nav_graph") and set it —
    // mirrors androidx NavController#setGraph(@NavigationRes int). Used by NavHostFragment
    // so declaring the graph (with its startDestination) loads the first destination with
    // no app-side inflate/navigate code.
    void setGraph(const std::string& graphRef, Bundle* startDestinationArgs = nullptr);

    NavDestination* getCurrentDestination();
    NavBackStackEntry* getCurrentBackStackEntry() const;

    void setLifecycleOwner(lifecycle::LifecycleOwner* owner){ mLifecycleOwner = owner; }
    void setViewModelStore(lifecycle::ViewModelStore* store){ mViewModelStore = store; }

    // Modern route navigation.
    void navigate(const std::string& route, NavOptions* options = nullptr);
    // Legacy int-id navigation (kept for Navigation.createNavigateOnClickListener).
    void navigate(int resId, Bundle* args = nullptr, NavOptions* options = nullptr);
    void navigate(NavDeepLinkRequest* request, NavOptions* options = nullptr);

    bool popBackStack();
    bool popBackStack(const std::string& route, bool inclusive, bool saveState);
    bool popBackStack(int destinationId, bool inclusive, bool saveState);
    bool navigateUp();

    NavDestination* findDestination(const std::string& route);

    void addOnDestinationChangedListener(OnDestinationChangedListener* listener);
    void removeOnDestinationChangedListener(OnDestinationChangedListener* listener);

private:
    Context* mContext;
    NavigatorProvider* mNavigatorProvider = nullptr;
    NavGraph* mGraph = nullptr;
    std::vector<NavBackStackEntry*> mBackStack;
    lifecycle::LifecycleOwner* mLifecycleOwner = nullptr;
    lifecycle::ViewModelStore* mViewModelStore = nullptr;
    std::vector<OnDestinationChangedListener*> mOnDestinationChangedListeners;

    void navigate(NavDestination* node, Bundle* args, NavOptions* navOptions);
    void dispatchOnDestinationChanged(NavDestination* destination, Bundle* args);
};

}//namespace cdroid
#endif/*__NAV_CONTROLLER_H__*/
