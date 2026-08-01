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
#ifndef __NAVBACKSTACKENTRY_H__
#define __NAVBACKSTACKENTRY_H__
/*********************************************************************************
 * Port of androidx.navigation.NavBackStackEntry. A back-stack entry bundles a
 * NavDestination + immutable args and is itself a LifecycleOwner /
 * ViewModelStoreOwner / SavedStateRegistryOwner (reusing the stage-1 foundation).
 *********************************************************************************/
#include <string>
#include <lifecycle/lifecycle.h>
#include <lifecycle/lifecycleowner.h>
#include <lifecycle/lifecycleregistry.h>
#include <lifecycle/viewmodelstore.h>
#include <lifecycle/viewmodelstoreowner.h>
#include <lifecycle/hasdefaultviewmodelproviderfactory.h>
#include <savedstate/savedstateregistryowner.h>
#include <savedstate/savedstateregistrycontroller.h>
#include <core/bundle.h>
namespace cdroid{

class NavDestination;

class NavBackStackEntry : public savedstate::SavedStateRegistryOwner,
                          public lifecycle::ViewModelStoreOwner,
                          public lifecycle::HasDefaultViewModelProviderFactory{
    friend class NavController; // restore: set mId to a saved id (androidx NavBackStackEntry.create(id, ...))
public:
    NavBackStackEntry(NavDestination* destination, Bundle* arguments);
    ~NavBackStackEntry() override;

    lifecycle::Lifecycle& getLifecycle() override;
    lifecycle::ViewModelStore& getViewModelStore() override;
    savedstate::SavedStateRegistry& getSavedStateRegistry() override;
    lifecycle::ViewModelProvider::Factory& getDefaultViewModelProviderFactory() override;

    NavDestination* getDestination() const { return mDestination; }
    const std::string& getId() const { return mId; }
    Bundle* getArguments() const { return mArguments; }

    void handleLifecycleEvent(lifecycle::Lifecycle::Event event);
    // Drive the LifecycleRegistry straight to `s` (state-based; used by updateBackStackLifecycle).
    void setCurrentState(lifecycle::Lifecycle::State s);
    lifecycle::Lifecycle::State getMaxLifecycle() const { return mMaxLifecycle; }
    void setMaxLifecycle(lifecycle::Lifecycle::State s){ mMaxLifecycle = s; }

private:
    NavDestination* mDestination;
    std::string mId;
    Bundle* mArguments;
    lifecycle::LifecycleRegistry* mLifecycleRegistry;
    lifecycle::ViewModelStore* mViewModelStore;
    savedstate::SavedStateRegistryController* mSavedStateRegistryController;
    lifecycle::Lifecycle::State mMaxLifecycle = lifecycle::Lifecycle::State::RESUMED;
    static std::string generateId();
};

// Port of androidx.navigation.NavBackStackEntryState — a saved NavBackStackEntry's identity +
// destination + arguments (CDROID in-memory: omits the SavedStateHandle slice for now). Held in
// NavController.mBackStackStates; instantiate() rebuilds a NavBackStackEntry preserving the id so
// the navigator-side (FragmentNavigator.savedIds) can match it for restoreBackStack.
struct NavBackStackEntryState{
    std::string id;            // the NavBackStackEntry mId (preserved across save/restore)
    int destinationId = 0;     // NavDestination id
    Bundle* arguments = nullptr; // owned copy
    NavBackStackEntryState() = default;
    NavBackStackEntryState(const std::string& i, int destId, Bundle* args)
        : id(i), destinationId(destId), arguments(args ? new Bundle(*args) : nullptr){}
    ~NavBackStackEntryState(){ delete arguments; }
    NavBackStackEntryState(const NavBackStackEntryState&) = delete;
    NavBackStackEntryState& operator=(const NavBackStackEntryState&) = delete;
    // Move-only (owns arguments) so it can live in / be inserted into a std::vector.
    NavBackStackEntryState(NavBackStackEntryState&& o) noexcept
        : id(std::move(o.id)), destinationId(o.destinationId), arguments(o.arguments){ o.arguments = nullptr; }
    NavBackStackEntryState& operator=(NavBackStackEntryState&& o) noexcept {
        if(this != &o){ delete arguments; id = std::move(o.id); destinationId = o.destinationId; arguments = o.arguments; o.arguments = nullptr; }
        return *this;
    }
};

}//namespace cdroid
#endif
