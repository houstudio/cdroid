#include <navigation/navaction.h>
namespace cdroid{
NavAction::NavAction(int destinationId)
    :NavAction(destinationId, nullptr){
}

NavAction::NavAction(int destinationId, NavOptions* navOptions) {
    mDestinationId = destinationId;
    mNavOptions = navOptions;
}

int NavAction::getDestinationId() const{
    return mDestinationId;
}

void NavAction::setNavOptions(NavOptions* navOptions) {
    mNavOptions = navOptions;
}

NavOptions* NavAction::getNavOptions() const{
    return mNavOptions;
}

}/*endof namespace*/
