#include <navigation/navbackstackentry.h>
#include <atomic>

namespace cdroid{

std::string NavBackStackEntry::generateId(){
    static std::atomic<long> sCounter{0};
    return "nav-back-stack-entry:" + std::to_string(++sCounter);
}

NavBackStackEntry::NavBackStackEntry(NavDestination* destination, Bundle* arguments)
    : mDestination(destination)
    , mArguments(arguments){
    mId = generateId();
    mLifecycleRegistry = new lifecycle::LifecycleRegistry(this);
    mViewModelStore = new lifecycle::ViewModelStore();
    mSavedStateRegistryController = new savedstate::SavedStateRegistryController(this);
}

NavBackStackEntry::~NavBackStackEntry(){
    delete mLifecycleRegistry;
    if(mViewModelStore) mViewModelStore->clear();
    delete mViewModelStore;
    delete mSavedStateRegistryController;
}

lifecycle::Lifecycle& NavBackStackEntry::getLifecycle(){
    return *mLifecycleRegistry;
}

lifecycle::ViewModelStore& NavBackStackEntry::getViewModelStore(){
    return *mViewModelStore;
}

savedstate::SavedStateRegistry& NavBackStackEntry::getSavedStateRegistry(){
    return mSavedStateRegistryController->getSavedStateRegistry();
}

lifecycle::ViewModelProvider::Factory& NavBackStackEntry::getDefaultViewModelProviderFactory(){
    return lifecycle::HasDefaultViewModelProviderFactory::getDefaultViewModelProviderFactory();
}

void NavBackStackEntry::handleLifecycleEvent(lifecycle::Lifecycle::Event event){
    mLifecycleRegistry->handleLifecycleEvent(event);
}

void NavBackStackEntry::setCurrentState(lifecycle::Lifecycle::State s){
    mLifecycleRegistry->setCurrentState(s);
}

}//namespace cdroid
