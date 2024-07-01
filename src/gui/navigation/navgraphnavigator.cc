#include <navigation/navgraph.h>
#include <navigation/navdestination.h>
#include <navigation/navgraphnavigator.h>

namespace cdroid{

/**
    @Override
 * Construct a Navigator capable of routing incoming navigation requests to the proper
 * destination within a {@link NavGraph}.
 * @param context
 */
NavGraphNavigator::NavGraphNavigator(Context* context):Navigator() {
    mContext = context;
}

/**
 * Creates a new {@link NavGraph} associated with this navigator.
 * @return
 */
NavGraph* NavGraphNavigator::createDestination() {
    return new NavGraph(this);
}

void NavGraphNavigator::navigate(/*@NonNull NavGraph*/NavDestination* destination, /*@Nullable*/Bundle* args,
        /*@Nullable*/ NavOptions* navOptions) {
    const int startId = ((NavGraph*)destination)->getStartDestination();
    if (startId == 0) {
        /*throw new IllegalStateException("no start destination defined via"
                + " app:startDestination for "
                + (destination.getId() != 0
                        ? NavDestination.getDisplayName(mContext, destination.getId())
                        : "the root navigation"));*/
    }
    NavDestination* startDestination = ((NavGraph*)destination)->findNode(startId, false);
    if (startDestination == nullptr) {
        const std::string dest = NavDestination::getDisplayName(mContext, startId);
        /*throw new IllegalArgumentException("navigation destination " + dest
                + " is not a direct child of this NavGraph");*/
    }
    dispatchOnNavigatorNavigated(destination->getId(), BACK_STACK_DESTINATION_ADDED);
    startDestination->navigate(args, navOptions);
}

bool NavGraphNavigator::popBackStack() {
    return false;
}
}/*endof namespace*/
