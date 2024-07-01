#if 0
#include <navigation/navigation.h>

namespace cdroid{

Navigation::Navigation() {
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
NavController* Navigation::findNavController(Context*,int viewId) {
    View view = ActivityCompat.requireViewById(activity, viewId);
    NavController navController = findViewNavController(view);
    if (navController == null) {
        throw new IllegalStateException("Activity " + activity
                + " does not have a NavController set on " + viewId);
    }
    return navController;
}

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
NavController* Navigation::findNavController(View* view) {
    NavController* navController = findViewNavController(view);
    if (navController == null) {
        throw new IllegalStateException("View " + view + " does not have a NavController set");
    }
    return navController;
}

/**
 * Create an {@link android.view.View.OnClickListener} for navigating
 * to a destination. This supports both navigating via an
 * {@link NavDestination#getAction(int) action} and directly navigating to a destination.
 *
 * @param resId an {@link NavDestination#getAction(int) action} id or a destination id to
 *              navigate to when the view is clicked
 * @return a new click listener for setting on an arbitrary view
 */

View::OnClickListener Navigation::createNavigateOnClickListener(@IdRes final int resId) {
    return createNavigateOnClickListener(resId, null);
}

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
View::OnClickListener Navigation::createNavigateOnClickListener(int resId,Bundle* args) {
    return new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            findNavController(view).navigate(resId, args);
        }
    };
}

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
void Navigation::setViewNavController(View* view,/*@Nullable*/ NavController controller) {
    view.setTag(R.id.nav_controller_view_tag, controller);
}

/**
 * Recurse up the view hierarchy, looking for the NavController
 * @param view the view to search from
 * @return the locally scoped {@link NavController} to the given view, if found
 */
NavController* Navigation::findViewNavController(View* view) {
    while (view != nullptr) {
        NavController* controller = getViewNavController(view);
        if (controller != nullptr) {
            return controller;
        }
        ViewParent parent = view.getParent();
        view = parent instanceof View ? (View) parent : nullptr;
    }
    return nullptr;
}

NavController* Navigation::getViewNavController(View* view) {
    Object tag = view.getTag(R.id.nav_controller_view_tag);
    NavController* controller = nullptr;
    if (tag instanceof WeakReference) {
        controller = ((WeakReference<NavController>) tag).get();
    } else if (tag instanceof NavController) {
        controller = (NavController) tag;
    }
    return controller;
}
}/*endof namespace*/
#endif
