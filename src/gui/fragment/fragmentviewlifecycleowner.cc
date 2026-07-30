#include <fragment/fragmentviewlifecycleowner.h>
namespace cdroid{
namespace fragment{

FragmentViewLifecycleOwner::FragmentViewLifecycleOwner(Fragment* /*fragment*/){
    mLifecycleRegistry = new lifecycle::LifecycleRegistry(this);
    mViewModelStore = new lifecycle::ViewModelStore();
    mSavedStateRegistryController = new savedstate::SavedStateRegistryController(this);
}

FragmentViewLifecycleOwner::~FragmentViewLifecycleOwner(){
    delete mLifecycleRegistry;
    if(mViewModelStore) mViewModelStore->clear();
    delete mViewModelStore;
    delete mSavedStateRegistryController;
}

lifecycle::Lifecycle& FragmentViewLifecycleOwner::getLifecycle(){ return *mLifecycleRegistry; }
lifecycle::ViewModelStore& FragmentViewLifecycleOwner::getViewModelStore(){ return *mViewModelStore; }
savedstate::SavedStateRegistry& FragmentViewLifecycleOwner::getSavedStateRegistry(){
    return mSavedStateRegistryController->getSavedStateRegistry();
}
lifecycle::ViewModelProvider::Factory& FragmentViewLifecycleOwner::getDefaultViewModelProviderFactory(){
    return lifecycle::HasDefaultViewModelProviderFactory::getDefaultViewModelProviderFactory();
}

void FragmentViewLifecycleOwner::handleLifecycleEvent(lifecycle::Lifecycle::Event event){
    mLifecycleRegistry->handleLifecycleEvent(event);
}

}//namespace fragment
}//namespace cdroid
