#include <savedstate/savedstateregistry.h>
#include <savedstate/savedstateregistryowner.h>
#include <stdexcept>
#include <porting/cdlog.h>

namespace cdroid{
namespace savedstate{

SavedState* SavedStateRegistry::consumeRestoredStateForKey(const std::string& key){
    if(!mIsRestored){
        throw std::runtime_error("You can 'consumeRestoredStateForKey' only after the "
                                 "corresponding component has moved to the CREATED state");
    }
    if(mRestoredState.isEmpty()) return nullptr;
    if(!mRestoredState.contains(key)) return nullptr;
    const SavedState* sub = mRestoredState.getSavedState(key);
    SavedState* consumed = new SavedState(sub ? *sub : SavedState());
    mRestoredState.remove(key);
    if(mRestoredState.isEmpty()) mRestoredState.clear();
    return consumed;
}

void SavedStateRegistry::registerSavedStateProvider(const std::string& key, SavedStateProvider* provider){
    if(mKeyToProviders.find(key) != mKeyToProviders.end()){
        throw std::runtime_error("SavedStateProvider with the given key (" + key + ") is already registered");
    }
    mKeyToProviders[key] = provider;
}

SavedStateRegistry::SavedStateProvider* SavedStateRegistry::getSavedStateProvider(const std::string& key) const{
    auto it = mKeyToProviders.find(key);
    return it == mKeyToProviders.end() ? nullptr : it->second;
}

void SavedStateRegistry::unregisterSavedStateProvider(const std::string& key){
    mKeyToProviders.erase(key);
}

void SavedStateRegistry::performAttach(SavedStateRegistryOwner* owner){
    if(!owner || mAttached) return;
    lifecycle::Lifecycle::State state = owner->getLifecycle().getCurrentState();
    if(state != lifecycle::Lifecycle::State::INITIALIZED){
        LOGE("Restarter must be created only during owner's initialization stage");
    }
    // androidx also adds a LifecycleObserver toggling isAllowingSavingState on
    // ON_START/ON_STOP; that flag does not gate the core save/restore path, so it
    // is omitted here to avoid an owner-less observer leak.
    mAttached = true;
}

void SavedStateRegistry::performRestore(SavedState* savedState){
    mRestoredState.clear();
    if(savedState){
        const SavedState* components = savedState->getSavedState(SAVED_COMPONENTS_KEY());
        if(components) mRestoredState.putAll(*components);
    }
    mIsRestored = true;
}

void SavedStateRegistry::performSave(SavedState& outBundle){
    SavedState inState;
    inState.putAll(mRestoredState);
    for(auto& kv : mKeyToProviders){
        SavedStateProvider* provider = kv.second;
        if(!provider) continue;
        SavedState* saved = provider->saveState();
        if(saved){
            if(!saved->isEmpty()) inState.putSavedState(kv.first, *saved);
            delete saved;
        }
    }
    if(!inState.isEmpty()){
        outBundle.putSavedState(SAVED_COMPONENTS_KEY(), inState);
    }
}

}//namespace savedstate
}//namespace cdroid
