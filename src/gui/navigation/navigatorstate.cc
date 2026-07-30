#include <navigation/navigatorstate.h>
#include <algorithm>

namespace cdroid{

void NavigatorState::push(NavBackStackEntry* entry){
    if(entry) mBackStack.push_back(entry);
}

void NavigatorState::pop(NavBackStackEntry* popUpTo, bool /*saveState*/){
    if(mBackStack.empty()) return;
    if(popUpTo == nullptr){
        mBackStack.clear();
        return;
    }
    // Pop entries above popUpTo (inclusive semantics handled by caller via flag elsewhere;
    // here we pop everything strictly above popUpTo, keeping popUpTo).
    auto it = std::find(mBackStack.begin(), mBackStack.end(), popUpTo);
    if(it != mBackStack.end()){
        mBackStack.erase(it + 1, mBackStack.end());
    }
}

}//namespace cdroid
