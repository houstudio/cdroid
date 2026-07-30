#include <savedstate/savedstatehandle.h>
namespace cdroid{
namespace savedstate{

SavedStateHandle* SavedStateHandle::createHandle(SavedState* restoredState, SavedState* defaultState){
    if(restoredState) return new SavedStateHandle(restoredState);
    if(defaultState) return new SavedStateHandle(defaultState);
    return new SavedStateHandle();
}

}//namespace savedstate
}//namespace cdroid
