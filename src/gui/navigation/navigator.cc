#include <navigation/navigator.h>
#include <navigation/navigatorstate.h>
#include <navigation/navbackstackentry.h>

namespace cdroid{

void Navigator::onAttach(NavigatorState* state){
    mState = state;
}

void Navigator::navigate(std::vector<NavBackStackEntry*>& entries, NavOptions* navOptions, Extras* extras){
    // Default modern navigate: fall back to the legacy per-destination navigate and push
    // each entry onto this navigator's state.
    for(NavBackStackEntry* entry : entries){
        if(!entry) continue;
        navigate(entry->getDestination(), entry->getArguments(), navOptions);
        if(mState) mState->push(entry);
    }
    (void)extras;
}

void Navigator::popBackStack(NavBackStackEntry* popUpTo, bool savedState){
    if(mState) mState->pop(popUpTo, savedState);
}

}//namespace cdroid
