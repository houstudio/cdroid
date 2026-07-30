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
