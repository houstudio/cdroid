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
#include <unordered_map>
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
    // V2 nested-graph back stack (androidx NavControllerImpl.addEntryToBackStack etc.).
    void addEntryToBackStack(NavDestination* node, Bundle* args, NavBackStackEntry* leafEntry);
    void popEntryFromBackStack(NavBackStackEntry* entry);
    void updateBackStackLifecycle();
    void linkChildToParent(NavBackStackEntry* child, NavBackStackEntry* parent);
    NavBackStackEntry* unlinkChildFromParent(NavBackStackEntry* child);
    NavBackStackEntry* findBackStackEntry(int destinationId);
    std::unordered_map<NavBackStackEntry*, NavBackStackEntry*> mChildToParent;
    std::unordered_map<NavBackStackEntry*, int> mParentToChildCount;
};

}//namespace cdroid
#endif/*__NAV_CONTROLLER_H__*/
