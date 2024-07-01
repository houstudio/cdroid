namespace cdroid{
class NavOptions;

class NavAction {
private:
    int mDestinationId;
    NavOptions*mNavOptions;
public:
    /**
     * Creates a new NavAction for the given destination.
     *
     * @param destinationId the ID of the destination that should be navigated to when this
     *                      action is used.
     */
    NavAction(int destinationId);

    /**
     * Creates a new NavAction for the given destination.
     *
     * @param destinationId the ID of the destination that should be navigated to when this
     *                      action is used.
     * @param navOptions special options for this action that should be used by default
     */
    NavAction(int destinationId, NavOptions* navOptions);

    /**
     * Gets the ID of the destination that should be navigated to when this action is used
     */
    int getDestinationId() const;

    /**
     * Sets the NavOptions to be used by default when navigating to this action.
     *
     * @param navOptions special options for this action that should be used by default
     */
    void setNavOptions(NavOptions* navOptions);

    /**
     * Gets the NavOptions to be used by default when navigating to this action.
     */
    NavOptions* getNavOptions()const;
};
}/*endof namaespace*/
