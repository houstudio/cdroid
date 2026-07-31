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
#ifndef __FRAGMENTVIEWLIFECYCLEOWNER_H__
#define __FRAGMENTVIEWLIFECYCLEOWNER_H__
/*********************************************************************************
 * Port of androidx.fragment.app.FragmentViewLifecycleOwner. The lifecycle scoped to a
 * Fragment's view (created in performCreateView, destroyed in performDestroyView). A
 * LifecycleOwner + ViewModelStoreOwner + SavedStateRegistryOwner for the view.
 *********************************************************************************/
#include <lifecycle/lifecycle.h>
#include <lifecycle/lifecycleowner.h>
#include <lifecycle/lifecycleregistry.h>
#include <lifecycle/viewmodelstore.h>
#include <lifecycle/viewmodelstoreowner.h>
#include <lifecycle/hasdefaultviewmodelproviderfactory.h>
#include <savedstate/savedstateregistryowner.h>
#include <savedstate/savedstateregistrycontroller.h>
namespace cdroid{
namespace fragment{

class Fragment;

class FragmentViewLifecycleOwner : public savedstate::SavedStateRegistryOwner,
                                   public lifecycle::ViewModelStoreOwner,
                                   public lifecycle::HasDefaultViewModelProviderFactory{
public:
    explicit FragmentViewLifecycleOwner(Fragment* fragment);
    ~FragmentViewLifecycleOwner() override;

    lifecycle::Lifecycle& getLifecycle() override;
    lifecycle::ViewModelStore& getViewModelStore() override;
    savedstate::SavedStateRegistry& getSavedStateRegistry() override;
    lifecycle::ViewModelProvider::Factory& getDefaultViewModelProviderFactory() override;

    void handleLifecycleEvent(lifecycle::Lifecycle::Event event);
private:
    lifecycle::LifecycleRegistry* mLifecycleRegistry;
    lifecycle::ViewModelStore* mViewModelStore;
    savedstate::SavedStateRegistryController* mSavedStateRegistryController;
};

}//namespace fragment
}//namespace cdroid
#endif
