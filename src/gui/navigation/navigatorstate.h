#ifndef __NAVIGATORSTATE_H__
#define __NAVIGATORSTATE_H__
/*********************************************************************************
 * Port of androidx.navigation.NavigatorState. The bridge between a Navigator and
 * its NavController: holds this navigator's back stack of NavBackStackEntry and
 * provides push/pop/createBackStackEntry. (StateFlow is replaced by a plain vector;
 * transitionsInProgress/special-effects are omitted for MVP.)
 *********************************************************************************/
#include <vector>
#include <string>
#include <navigation/navbackstackentry.h>
namespace cdroid{
class NavDestination;

class NavigatorState{
public:
    virtual ~NavigatorState() = default;
    // Push a new entry onto this navigator's back stack.
    virtual void push(NavBackStackEntry* entry);
    // Pop everything above (and optionally) popUpTo from the back stack.
    virtual void pop(NavBackStackEntry* popUpTo, bool saveState);
    // Build a new NavBackStackEntry for a destination + args (NavController implements).
    virtual NavBackStackEntry* createBackStackEntry(NavDestination* destination, Bundle* arguments) = 0;

    const std::vector<NavBackStackEntry*>& getBackStack() const { return mBackStack; }
protected:
    std::vector<NavBackStackEntry*> mBackStack;
};

}//namespace cdroid
#endif
