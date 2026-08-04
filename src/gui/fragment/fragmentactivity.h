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
#ifndef __FRAGMENTACTIVITY_H__
#define __FRAGMENTACTIVITY_H__
/*********************************************************************************
 * CDROID port of androidx.fragment.app.FragmentActivity. Subclasses Activity
 * (= Window) and implements the owner interfaces FragmentManager probes for via
 * dynamic_cast: LifecycleOwner (via SavedStateRegistryOwner), ViewModelStoreOwner,
 * SavedStateRegistryOwner. Drives FragmentManager dispatch from the Activity
 * lifecycle hooks
 * (onCreate/onStart/onResume/onPause/onStop/onDestroy/onBackPressed).
 *********************************************************************************/
#include <widget/cdwindow.h>
#include <lifecycle/lifecycleregistry.h>
#include <lifecycle/viewmodelstore.h>
#include <lifecycle/viewmodelstoreowner.h>
#include <savedstate/savedstateregistryowner.h>
#include <savedstate/savedstateregistrycontroller.h>
namespace cdroid{
namespace fragment{

class FragmentManager;

class FragmentActivity : public Activity,
                         public savedstate::SavedStateRegistryOwner,
                         public lifecycle::ViewModelStoreOwner{
public:
    FragmentActivity(int x, int y, int w, int h);
    ~FragmentActivity() override;

    // --- owner interfaces ---
    lifecycle::Lifecycle& getLifecycle() override;
    lifecycle::ViewModelStore& getViewModelStore() override;
    savedstate::SavedStateRegistry& getSavedStateRegistry() override;

    // --- Activity lifecycle hooks drive the FragmentManager ---
    void onCreate(Bundle* savedInstanceState) override;
    void onStart() override;
    void onResume() override;
    void onPause() override;
    void onStop() override;
    void onDestroy() override;
    void onBackPressed() override;

    FragmentManager* getSupportFragmentManager();
    int getFragmentContainerId() const { return mContainerId; }

private:
    class HostCallbacks;
    int mContainerId;
    lifecycle::LifecycleRegistry* mLifecycleRegistry;
    lifecycle::ViewModelStore* mViewModelStore;
    savedstate::SavedStateRegistryController* mSavedStateRegistryController;
    FragmentManager* mFragmentManager;
    HostCallbacks* mHost;
};

}//namespace fragment
}//namespace cdroid
#endif
