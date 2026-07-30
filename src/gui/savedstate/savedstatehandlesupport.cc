#include <savedstate/savedstatehandlesupport.h>
#include <savedstate/savedstateregistry.h>
namespace cdroid{
namespace savedstate{

SavedStateHandle* SavedStateHandleSupport::createSavedStateHandle(SavedStateRegistryOwner* owner,
                                                                  const std::string& key,
                                                                  SavedState* defaultArgs){
    SavedState* restored = owner ? owner->getSavedStateRegistry().consumeRestoredStateForKey(key) : nullptr;
    SavedStateHandle* handle = SavedStateHandle::createHandle(restored, defaultArgs);
    if(restored) delete restored;
    return handle;
}

}//namespace savedstate
}//namespace cdroid
