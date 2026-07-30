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
