#include <savedstate/savedstateviewmodelfactory.h>
#include <savedstate/savedstateregistry.h>
#include <porting/cdlog.h>
namespace cdroid{
namespace savedstate{

namespace{
// Bridges a SavedStateHandle into a SavedStateRegistry provider so its contents are persisted.
class HandleProvider : public SavedStateRegistry::SavedStateProvider{
    SavedStateHandle* mHandle;
public:
    explicit HandleProvider(SavedStateHandle* h) : mHandle(h){}
    SavedState* saveState() override{
        return new SavedState(mHandle->getSavedState());
    }
};
}//anonymous

SavedStateViewModelFactory::SavedStateViewModelFactory(SavedStateRegistryOwner* owner, SavedState* defaultArgs)
    : mOwner(owner), mDefaultArgs(defaultArgs){}

void SavedStateViewModelFactory::registerFactory(const std::string& className, VmFactory factory){
    mFactories[className] = std::move(factory);
}

lifecycle::ViewModel* SavedStateViewModelFactory::create(const std::string& modelClass, lifecycle::CreationExtras&){
    auto it = mFactories.find(modelClass);
    if(it == mFactories.end()){
        LOGE("SavedStateViewModelFactory: no factory registered for %s", modelClass.c_str());
        return nullptr;
    }
    // Restore any previously-saved state for this key, then build the handle.
    SavedState* restored = mOwner ? mOwner->getSavedStateRegistry().consumeRestoredStateForKey(modelClass) : nullptr;
    SavedStateHandle* handle = SavedStateHandle::createHandle(restored, mDefaultArgs);
    if(restored) delete restored;
    // Construct the VM with the handle.
    lifecycle::ViewModel* viewModel = it->second(handle);
    // Register the handle so its state is saved under this key.
    if(mOwner) mOwner->getSavedStateRegistry().registerSavedStateProvider(modelClass, new HandleProvider(handle));
    return viewModel;
}

}//namespace savedstate
}//namespace cdroid
