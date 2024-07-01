#include <navigation/navigator.h>
namespace cdroid{

Navigator::Navigator():NavDestination(nullptr){
}

//@Nullable
Bundle Navigator::onSaveState() {
    return Bundle();
}

/**
 * Restore any state previously saved in {@link #onSaveState()}. This will be called before
 * any calls to {@link #navigate(NavDestination, Bundle, NavOptions)} or
 * {@link #popBackStack()}.
 * <p>
 * Calls to {@link #createDestination()} should not be dependent on any state restored here as
 * {@link #createDestination()} can be called before the state is restored.
 *
 * @param savedState The state previously saved
 */
void Navigator::onRestoreState(/*@NonNull*/Bundle savedState) {
}

/**
 * Add a listener to be notified when this navigator changes navigation destinations.
 *
 * <p>Most application code should use
 * {@link NavController#addOnNavigatedListener(NavController.OnNavigatedListener)} instead.
 * </p>
 *
 * @param listener listener to add
 */
void Navigator::addOnNavigatorNavigatedListener(OnNavigatorNavigatedListener listener) {
    mOnNavigatedListeners.push_back(listener);//add(listener);
}

/**
 * Remove a listener so that it will no longer be notified when this navigator changes
 * navigation destinations.
 *
 * @param listener listener to remove
 */
void Navigator::removeOnNavigatorNavigatedListener(OnNavigatorNavigatedListener listener) {
    //mOnNavigatedListeners.remove(listener);
}

/**
 * Dispatch a navigated event to all registered {@link OnNavigatorNavigatedListener listeners}.
 * Utility for navigator implementations.
 *
 * @param destId id of the new destination
 * @param backStackEffect how the navigation event affects the back stack
 */
void Navigator::dispatchOnNavigatorNavigated(int destId,int backStackEffect) {
    for (OnNavigatorNavigatedListener listener : mOnNavigatedListeners) {
        listener(*this, destId, backStackEffect);//onNavigatorNavigated
    }
}

}
