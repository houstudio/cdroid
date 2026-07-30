#ifndef __SAVEDSTATEREGISTRYCONTROLLER_H__
#define __SAVEDSTATEREGISTRYCONTROLLER_H__
/*********************************************************************************
 * Port of androidx.savedstate.SavedStateRegistryController. Owns a
 * SavedStateRegistry and drives attach/restore/save for the owner.
 *********************************************************************************/
#include <savedstate/savedstateregistry.h>
#include <savedstate/savedstateregistryowner.h>
namespace cdroid{
namespace savedstate{

class SavedStateRegistryController{
public:
    explicit SavedStateRegistryController(SavedStateRegistryOwner* owner);
    SavedStateRegistry& getSavedStateRegistry(){ return mRegistry; }
    void performAttach();
    void performRestore(SavedState* savedState);
    void performSave(SavedState& outBundle);
private:
    SavedStateRegistryOwner* mOwner;
    SavedStateRegistry mRegistry;
};

}//namespace savedstate
}//namespace cdroid
#endif
