#ifndef __SAVEDSTATEHANDLESUPPORT_H__
#define __SAVEDSTATEHANDLESUPPORT_H__
/*********************************************************************************
 * Port of androidx.lifecycle.SavedStateHandleSupport. Helpers to obtain a SavedStateHandle
 * bound to a SavedStateRegistryOwner (used by VMs that want SavedStateHandle without the
 * SavedStateViewModelFactory indirection).
 *********************************************************************************/
#include <string>
#include <savedstate/savedstateregistryowner.h>
#include <savedstate/savedstatehandle.h>
namespace cdroid{
namespace savedstate{

class SavedStateHandleSupport{
public:
    // Create (or restore) a SavedStateHandle for the given key from the owner's registry.
    static SavedStateHandle* createSavedStateHandle(SavedStateRegistryOwner* owner,
                                                    const std::string& key,
                                                    SavedState* defaultArgs = nullptr);
};

}//namespace savedstate
}//namespace cdroid
#endif
