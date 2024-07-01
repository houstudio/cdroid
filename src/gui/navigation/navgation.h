#include <view/view.h>

namespace cdroid{
class NavController;
class Navigation {
private:
    // No instances. Static utilities only.
    Navigation() {
    }

    /**
     * Find a {@link NavController} given the id of a View and its containing
     * {@link Activity}. This is a convenience wrapper around {@link #findNavController(View)}.
     *
     * <p>This method will locate the {@link NavController} associated with this view.
     * This is automatically populated for the id of a {@link NavHost} and its children.</p>
     *
     * @param activity The Activity hosting the view
     * @param viewId The id of the view to search from
     * @return the {@link NavController} associated with the view referenced by id
     * @throws IllegalStateException if the given viewId does not correspond with a
     * {@link NavHost} or is not within a NavHost.
     */
public:
    static NavController* findNavController(Context*,  int viewId);

    /**
     * Find a {@link NavController} given a local {@link View}.
     *
     * <p>This method will locate the {@link NavController} associated with this view.
     * This is automatically populated for views that are managed by a {@link NavHost}
     * and is intended for use by various {@link android.view.View.OnClickListener listener}
     * interfaces.</p>
     *
     * @param view the view to search from
     * @return the locally scoped {@link NavController} to the given view
     * @throws IllegalStateException if the given view does not correspond with a
     * {@link NavHost} or is not within a NavHost.
     */
    static NavController* findNavController(View* view);

    /**
     * Create an {@link android.view.View.OnClickListener} for navigating
     * to a destination. This supports both navigating via an
     * {@link NavDestination#getAction(int) action} and directly navigating to a destination.
     *
     * @param resId an {@link NavDestination#getAction(int) action} id or a destination id to
     *              navigate to when the view is clicked
     * @return a new click listener for setting on an arbitrary view
     */
    static View::OnClickListener createNavigateOnClickListener(int resId);

    /**
     * Create an {@link android.view.View.OnClickListener} for navigating
     * to a destination. This supports both navigating via an
     * {@link NavDestination#getAction(int) action} and directly navigating to a destination.
     *
     * @param resId an {@link NavDestination#getAction(int) action} id or a destination id to
     *              navigate to when the view is clicked
     * @param args arguments to pass to the final destination
     * @return a new click listener for setting on an arbitrary view
     */
    static View::OnClickListener createNavigateOnClickListener(int resId, Bundle* args);

    /**
     * Associates a NavController with the given View, allowing developers to use
     * {@link #findNavController(View)} and {@link #findNavController(Activity, int)} with that
     * View or any of its children to retrieve the NavController.
     * <p>
     * This is generally called for you by the hosting {@link NavHost}.
     * @param view View that should be associated with the given NavController
     * @param controller The controller you wish to later retrieve via
     *                   {@link #findNavController(View)}
     */
    static void setViewNavController(View* view,/*@Nullable*/ NavController* controller);

    /**
     * Recurse up the view hierarchy, looking for the NavController
     * @param view the view to search from
     * @return the locally scoped {@link NavController} to the given view, if found
     */
private:
    static NavController* findViewNavController(View* view);
    static NavController* getViewNavController(View* view);
};
}/*endof namespace*/
