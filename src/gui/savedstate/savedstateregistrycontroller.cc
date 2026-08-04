#include <savedstate/savedstateregistrycontroller.h>
namespace cdroid{
namespace savedstate{

SavedStateRegistryController::SavedStateRegistryController(SavedStateRegistryOwner* owner)
    : mOwner(owner){}

void SavedStateRegistryController::performAttach(){
    mRegistry.performAttach(mOwner);
}

void SavedStateRegistryController::performRestore(SavedState* savedState){
    mRegistry.performAttach(mOwner);
    mRegistry.performRestore(savedState);
}

void SavedStateRegistryController::performSave(SavedState& outBundle){
    mRegistry.performSave(outBundle);
}

}//namespace savedstate
}//namespace cdroid
