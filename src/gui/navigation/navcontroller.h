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
#include <functional>
#include <lifecycle/lifecycle.h>
#include <lifecycle/lifecycleowner.h>
#include <lifecycle/viewmodelstore.h>
#include <core/bundle.h>
#include <navigation/navdestination.h>
#include <navigation/navbackstackentry.h>
namespace cdroid{
class Context;
class Navigator;
class NavigatorState;
class NavGraph;
class NavInflater;
class NavigatorProvider;
class NavBackStackEntry;
class NavOptions;
class NavDeepLinkRequest;

class NavController{
    friend class NavigatorState; // androidx NavControllerNavigatorState is an inner class with
                                 // access to push()/pop(); in C++ it calls back via these privates.
public:
    class OnDestinationChangedListener{
    public:
        virtual ~OnDestinationChangedListener() = default;
        virtual void onDestinationChanged(NavController* controller, NavDestination* destination, Bundle* arguments){}
    };

    NavController(Context* context);
    ~NavController();
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
    // Read-only access to the back stack (androidx exposes NavController.currentBackStack /
    // getBackStackEntryAt). Used by tests to assert size and per-entry lifecycle.
    const std::vector<NavBackStackEntry*>& getBackStack() const { return mBackStack; }

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

    // --- navigator-state pop model (androidx NavControllerImpl navigatorState + handlers) ---
    // One NavigatorState per registered Navigator, created/attached in onGraphCreated (setGraph).
    NavigatorState* getOrCreateNavigatorState(Navigator* navigator);
    // androidx NavControllerImpl.push(state, entry): the installed push handler mutates the merged
    // back queue (addEntryToBackStack), then the navigator's own state is appended (addInternal).
    void push(NavigatorState* state, NavBackStackEntry* entry);
    // androidx NavControllerImpl.pop(state, popUpTo, saveState, superCallback): the installed pop
    // handler mutates the merged back queue (popEntryFromBackStack), then the navigator's own state
    // is trimmed (popInternal).
    void pop(NavigatorState* state, NavBackStackEntry* popUpTo, bool saveState);
    // androidx NavControllerImpl.executePopOperations: pop each entry (top-first) via its own
    // Navigator's entry-based popBackStack, breaking on the first that doesn't fire the pop handler
    // (!receivedPop). Returns whether anything was popped. saveState captures each popped entry's
    // NavBackStackEntryState into mBackStackStates (Level A).
    bool executePopOperations(std::vector<Navigator*>& popOperations, bool saveState);
    // androidx NavControllerImpl.restoreStateInternal: rebuild + re-run a saved back-stack chain
    // (keyed by destination id in mBackStackMap) with its original entry ids, so the navigator-side
    // restore (FragmentNavigator.restoreBackStack via savedIds) fires. Returns false if none saved.
    bool restoreStateInternal(int destinationId, Bundle* args, NavOptions* options);
    std::unordered_map<Navigator*, NavigatorState*> mNavigatorStates;
    // Transient handler slots installed around a single Navigator.navigate / popBackStack call
    // (androidx addToBackStackHandler / popFromBackStackHandler). Null outside that scope.
    std::function<void(NavBackStackEntry*)> mAddToBackStackHandler;
    std::function<void(NavBackStackEntry*)> mPopFromBackStackHandler;
    // --- Level A saveState bookkeeping (androidx NavControllerImpl backStackMap / backStackStates) ---
    std::unordered_map<int, std::string> mBackStackMap;   // destinationId -> saved chain id (bottom entry id)
    std::unordered_map<std::string, std::vector<NavBackStackEntryState>> mBackStackStates; // chain id -> saved entry chain
};

}//namespace cdroid
#endif/*__NAV_CONTROLLER_H__*/
