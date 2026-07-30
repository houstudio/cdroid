#ifndef __FRAGMENTWINDOW_H__
#define __FRAGMENTWINDOW_H__
/*********************************************************************************
 * CDROID Fragment host (the FragmentActivity analogue). Subclasses Window and
 * implements the owner interfaces FragmentManager probes for via dynamic_cast:
 * LifecycleOwner (via SavedStateRegistryOwner), ViewModelStoreOwner,
 * SavedStateRegistryOwner. Drives FragmentManager dispatch from Window lifecycle
 * hooks (onCreate/onActive/onDeactive/onBackPressed).
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

class FragmentWindow : public Window,
                       public savedstate::SavedStateRegistryOwner,
                       public lifecycle::ViewModelStoreOwner{
public:
    FragmentWindow(int x, int y, int w, int h);
    ~FragmentWindow() override;

    // --- owner interfaces ---
    lifecycle::Lifecycle& getLifecycle() override;
    lifecycle::ViewModelStore& getViewModelStore() override;
    savedstate::SavedStateRegistry& getSavedStateRegistry() override;

    // --- Window lifecycle hooks drive the FragmentManager ---
    void onCreate() override;
    void onActive() override;
    void onDeactive() override;
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
