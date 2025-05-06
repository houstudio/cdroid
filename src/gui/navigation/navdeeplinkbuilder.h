#ifndef __NAV_DEEPLINK_BUILDER_H__
#define __NAV_DEEPLINK_BUILDER_H__
namespace cdroid{
class TaskStackBuilder;
class NavDeepLinkBuilder {
private:
    Context* mContext;
    Intent mIntent;
    NavGraph* mGraph;
    int mDestId;
private:
    class PermissiveNavigatorProvider;
    void fillInIntent();
public:
    /**
     * Construct a new NavDeepLinkBuilder.
     *
     * If the context passed in here is not an {@link Activity}, this method will use
     * {@link android.content.pm.PackageManager#getLaunchIntentForPackage(String)} as the
     * default activity to launch, if available.
     *
     * @param context Context used to create deep links
     * @see #setComponentName
     */
    NavDeepLinkBuilder(Context* context);

    /**
     * @see NavController#createDeepLink()
     */
    NavDeepLinkBuilder(NavController* navController);

    /**
     * Sets an explicit Activity to be started by the deep link created by this class.
     *
     * @param componentName The Activity to start. This Activity should have a {@link NavController}
     *                      which uses the same {@link NavGraph} used to construct this
     *                      deep link.
     * @return this object for chaining
     */
    NavDeepLinkBuilder& setComponentName(const std::string& componentName);

    /**
     * Sets the graph that contains the {@link #setDestination(int) deep link destination}.
     *
     * @param navGraphId ID of the {@link NavGraph} containing the deep link destination
     * @return this object for chaining
     */
    NavDeepLinkBuilder& setGraph(int navGraphId);

    /**
     * Sets the graph that contains the {@link #setDestination(int) deep link destination}.
     *
     * @param navGraph The {@link NavGraph} containing the deep link destination
     * @return this object for chaining
     */
    NavDeepLinkBuilder& setGraph(NavGraph* navGraph);

    /**
     * Sets the destination id to deep link to.
     *
     * @param destId destination ID to deep link to.
     * @return this object for chaining
     */
    NavDeepLinkBuilder& setDestination(int destId);


    /**
     * Set optional arguments to send onto the destination
     * @param args arguments to pass to the destination
     * @return this object for chaining
     */
    NavDeepLinkBuilder& setArguments(Bundle* args);

    /**
     * Construct the full {@link TaskStackBuilder task stack} needed to deep link to the given
     * destination.
     * <p>
     * You must have {@link #setGraph set a NavGraph} and {@link #setDestination set a destination}
     * before calling this method.
     * </p>
     *
     * @return a {@link TaskStackBuilder} which can be used to
     * {@link TaskStackBuilder#startActivities() send the deep link} or
     * {@link TaskStackBuilder#getPendingIntent(int, int) create a PendingIntent} to deep link to
     * the given destination.
     */
    TaskStackBuilder& createTaskStackBuilder();

    /**
     * Construct a {@link PendingIntent} to the {@link #setDestination(int) deep link destination}.
     * <p>
     * This constructs the entire {@link #createTaskStackBuilder() task stack} needed.
     * <p>
     * You must have {@link #setGraph set a NavGraph} and {@link #setDestination set a destination}
     * before calling this method.
     * </p>
     *
     * @return a PendingIntent constructed with
     * {@link TaskStackBuilder#getPendingIntent(int, int)} to deep link to the
     * given destination
     */
    class PendingIntent createPendingIntent();
};/*endof NavDeepLinkBuilder*/
/**
 * A {@link NavigatorProvider} that only parses the basics: {@link NavGraph navigation graphs}
 * and {@link NavDestination destinations}, effectively only getting the base destination
 * information.
 */
#if 0
class NavDeepLinkBuilder::PermissiveNavigatorProvider:public SimpleNavigatorProvider {
    /**
     * A Navigator that only parses the {@link NavDestination} attributes.
     */
    private final Navigator<NavDestination> mDestNavigator = new Navigator<NavDestination>() {
        @NonNull
        @Override
        public NavDestination createDestination() {
            return new NavDestination(this);
        }

        @Override
        public void navigate(@NonNull NavDestination destination, @Nullable Bundle args,
                @Nullable NavOptions navOptions) {
            throw new IllegalStateException("navigate is not supported");
        }

        @Override
        public boolean popBackStack() {
            throw new IllegalStateException("popBackStack is not supported");
        }
    };

    PermissiveNavigatorProvider(Context context) {
        addNavigator(new NavGraphNavigator(context));
    }

    @NonNull
    @Override
    public Navigator<? extends NavDestination> getNavigator(@NonNull String name) {
        try {
            return super.getNavigator(name);
        } catch (IllegalStateException e) {
            return mDestNavigator;
        }
    }
};
#endif
}/*endof namespace*/
#endif
