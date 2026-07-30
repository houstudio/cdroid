#ifndef __SAVEDSTATEREGISTRYOWNER_H__
#define __SAVEDSTATEREGISTRYOWNER_H__
/*********************************************************************************
 * Port of androidx.savedstate.SavedStateRegistryOwner. Extends LifecycleOwner.
 *********************************************************************************/
#include <lifecycle/lifecycleowner.h>
namespace cdroid{
namespace savedstate{

class SavedStateRegistry;

class SavedStateRegistryOwner : public lifecycle::LifecycleOwner{
public:
    virtual ~SavedStateRegistryOwner() = default;
    virtual SavedStateRegistry& getSavedStateRegistry() = 0;
};

}//namespace savedstate
}//namespace cdroid
#endif
