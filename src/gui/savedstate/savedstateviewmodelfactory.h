#ifndef __SAVEDSTATEVIEWMODELFACTORY_H__
#define __SAVEDSTATEVIEWMODELFACTORY_H__
/*********************************************************************************
 * Port of androidx.lifecycle.SavedStateViewModelFactory. CDROID has no reflection, so
 * instead of reflectively instantiating VMs it holds a user-registered factory map:
 * className -> lambda(SavedStateHandle*) -> ViewModel. create() resolves the handle from
 * the owner's SavedStateRegistry (restore) + default args, builds the VM, and registers a
 * provider so the handle's state is saved.
 *********************************************************************************/
#include <string>
#include <unordered_map>
#include <functional>
#include <lifecycle/viewmodel.h>
#include <lifecycle/viewmodelprovider.h>
#include <savedstate/savedstateregistryowner.h>
#include <savedstate/savedstatehandle.h>
namespace cdroid{
namespace savedstate{

class SavedStateViewModelFactory : public lifecycle::ViewModelProvider::Factory{
public:
    using VmFactory = std::function<lifecycle::ViewModel*(SavedStateHandle*)>;

    SavedStateViewModelFactory(SavedStateRegistryOwner* owner, SavedState* defaultArgs = nullptr);

    // Register a VM constructor that accepts a SavedStateHandle (replaces reflection).
    void registerFactory(const std::string& className, VmFactory factory);

    lifecycle::ViewModel* create(const std::string& modelClass, lifecycle::CreationExtras& extras) override;

private:
    SavedStateRegistryOwner* mOwner;
    SavedState* mDefaultArgs;
    std::unordered_map<std::string, VmFactory> mFactories;
};

}//namespace savedstate
}//namespace cdroid
#endif
