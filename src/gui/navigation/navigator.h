#ifndef __NAVIGATOR_H__
#define __NAVIGATOR_H__
#include <core/callbackbase.h>
#include <navigation/navdestination.h>

namespace cdroid{

class Navigator:public NavDestination {
public:
    static constexpr int BACK_STACK_UNCHANGED = 0;

    /**
     * Indicator that the navigation event has added a new entry to the back stack. Only
     * destinations added with this flag will be handled by {@link NavController#navigateUp()}.
     *
     * @see #dispatchOnNavigatorNavigated
     */
    static constexpr int BACK_STACK_DESTINATION_ADDED = 1;

    /**
     * Indicator that the navigation event has popped an entry off the back stack.
     *
     * @see #dispatchOnNavigatorNavigated
     */
    static constexpr int BACK_STACK_DESTINATION_POPPED = 2;
    /**
     * Listener for observing navigation events for this specific navigator. Most app code
     * should use {@link NavController.OnNavigatedListener} instead.
     */
    typedef CallbackBase<void,Navigator&,int,int>OnNavigatorNavigatedListener;
private:
    std::vector<OnNavigatorNavigatedListener> mOnNavigatedListeners;
public:
    Navigator();
    /**
     * Construct a new NavDestination associated with this Navigator.
     *
     * <p>Any initialization of the destination should be done in the destination's constructor as
     * it is not guaranteed that every destination will be created through this method.</p>
     * @return a new NavDestination
     */
    //@NonNull
    virtual NavDestination* createDestination()=0;

    /**
     * Navigate to a destination.
     *
     * <p>Requests navigation to a given destination associated with this navigator in
     * the navigation graph. This method generally should not be called directly;
     * {@link NavController} will delegate to it when appropriate.</p>
     *
     * <p>Implementations should {@link #dispatchOnNavigatorNavigated} to notify
     * listeners of the resulting navigation destination.</p>
     *
     * @param destination destination node to navigate to
     * @param args arguments to use for navigation
     * @param navOptions additional options for navigation
     */
    virtual void navigate(NavDestination* destination, /*@Nullable*/ Bundle* args,
                                     /*@Nullable*/ NavOptions* navOptions)=0;

    /**
     * Attempt to pop this navigator's back stack, performing the appropriate navigation.
     *
     * <p>Implementations should {@link #dispatchOnNavigatorNavigated} to notify
     * listeners of the resulting navigation destination and return {@code true} if navigation
     * was successful. Implementations should return {@code false} if navigation could not
     * be performed, for example if the navigator's back stack was empty.</p>
     *
     * @return {@code true} if pop was successful
     */
    virtual bool popBackStack()=0;

    /**
     * Called to ask for a {@link Bundle} representing the Navigator's state. This will be
     * restored in {@link #onRestoreState(Bundle)}.
     */
    //@Nullable
    Bundle onSaveState();

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
    void onRestoreState(/*@NonNull*/Bundle savedState);
    /**
     * Add a listener to be notified when this navigator changes navigation destinations.
     *
     * <p>Most application code should use
     * {@link NavController#addOnNavigatedListener(NavController.OnNavigatedListener)} instead.
     * </p>
     *
     * @param listener listener to add
     */
    void addOnNavigatorNavigatedListener(OnNavigatorNavigatedListener listener);

    /**
     * Remove a listener so that it will no longer be notified when this navigator changes
     * navigation destinations.
     *
     * @param listener listener to remove
     */
    void removeOnNavigatorNavigatedListener(OnNavigatorNavigatedListener listener);
    /**
     * Dispatch a navigated event to all registered {@link OnNavigatorNavigatedListener listeners}.
     * Utility for navigator implementations.
     *
     * @param destId id of the new destination
     * @param backStackEffect how the navigation event affects the back stack
     */
    void dispatchOnNavigatorNavigated(int destId,int backStackEffect);
};
}/*endof namespace*/
#endif /*__NAVIGATOR_H__*/


