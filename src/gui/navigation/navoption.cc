#if 1
#include <navigation/navoptions.h>
namespace cdroid{
/*private static String KEY_NAV_OPTIONS = "android-support-nav:navOptions";
private static String KEY_LAUNCH_MODE = "launchMode";
private static String KEY_POP_UP_TO = "popUpTo";
private static String KEY_POP_UP_TO_INCLUSIVE = "popUpToInclusive";
private static String KEY_ENTER_ANIM = "enterAnim";
private static String KEY_EXIT_ANIM = "exitAnim";
private static String KEY_POP_ENTER_ANIM = "popEnterAnim";
private static String KEY_POP_EXIT_ANIM = "popExitAnim";*/

/**
 * Add the {@link #getPopEnterAnim() pop enter} and {@link #getPopExitAnim() pop exit}
 * animation to an Intent for later usage with
 * {@link #applyPopAnimationsToPendingTransition(Activity)}.
 * <p>
 * This is automatically called for you by {@link ActivityNavigator}.
 * </p>
 *
 * @param intent Intent being started with the given NavOptions
 * @param navOptions NavOptions containing the pop animations.
 * @see #applyPopAnimationsToPendingTransition(Activity)
 * @see #getPopEnterAnim()
 * @see #getPopExitAnim()
 */
void NavOptions::addPopAnimationsToIntent(Intent& intent,NavOptions* navOptions) {
    if (navOptions != nullptr) {
        //intent.putExtra(KEY_NAV_OPTIONS, navOptions->toBundle());
    }
}

/**
 * Apply any pop animations in the Intent of the given Activity to a pending transition.
 * This should be used in place of  {@link Activity#overridePendingTransition(int, int)}
 * to get the appropriate pop animations.
 * @param activity An activity started from the {@link ActivityNavigator}.
 * @see #addPopAnimationsToIntent(Intent, NavOptions)
 * @see #getPopEnterAnim()
 * @see #getPopExitAnim()
 */
void NavOptions::applyPopAnimationsToPendingTransition(Activity& activity) {
    /*Intent intent = activity.getIntent();
    if (intent == nullptr) {
        return;
    }
    Bundle bundle = intent.getBundleExtra(KEY_NAV_OPTIONS);
    if (bundle != nullptr) {
        NavOptions* navOptions = NavOptions.fromBundle(bundle);
        const std::string popEnterAnim = navOptions->getPopEnterAnim();
        const std::string popExitAnim = navOptions->getPopExitAnim();
        if (popEnterAnim != -1 || popExitAnim != -1) {
            popEnterAnim = popEnterAnim != -1 ? popEnterAnim : 0;
            popExitAnim = popExitAnim != -1 ? popExitAnim : 0;
            activity.overridePendingTransition(popEnterAnim, popExitAnim);
        }
    }*/
}

NavOptions::NavOptions(int launchMode, int popUpTo, bool popUpToInclusive,
        const std::string& enterAnim, const std::string& exitAnim,
        const std::string& popEnterAnim, const std::string& popExitAnim) {
    mLaunchMode = launchMode;
    mPopUpTo = popUpTo;
    mPopUpToInclusive = popUpToInclusive;
    mEnterAnim = enterAnim;
    mExitAnim = exitAnim;
    mPopEnterAnim = popEnterAnim;
    mPopExitAnim = popExitAnim;
}

/**
 * Whether this navigation action should launch as single-top (i.e., there will be at most
 * one copy of a given destination on the top of the back stack).
 * <p>
 * This functions similarly to how {@link android.content.Intent#FLAG_ACTIVITY_SINGLE_TOP}
 * works with activites.
 */
bool NavOptions::shouldLaunchSingleTop() const{
    return (mLaunchMode & LAUNCH_SINGLE_TOP) != 0;
}

/**
 * Whether this navigation action should launch the destination in a new document.
 * <p>
 * This functions similarly to how {@link android.content.Intent#FLAG_ACTIVITY_NEW_DOCUMENT}
 * works with activites.
 * @deprecated As per the {@link android.content.Intent#FLAG_ACTIVITY_NEW_DOCUMENT}
 * documentation, it is recommended to use {@link android.R.attr#documentLaunchMode} on an
 * Activity you wish to launch as a new document.
 */
bool NavOptions::shouldLaunchDocument() const{
    return (mLaunchMode & LAUNCH_DOCUMENT) != 0;
}

/**
 * Whether this navigation action should clear the entire back stack
 * <p>
 * This functions similarly to how {@link android.content.Intent#FLAG_ACTIVITY_CLEAR_TASK}
 * works with activites.
 * @deprecated This is synonymous with {@link #getPopUpTo()} with the root of the graph and
 * using {@link #isPopUpToInclusive()}.
 */
bool NavOptions::shouldClearTask() const{
    return (mLaunchMode & LAUNCH_CLEAR_TASK) != 0;
}

/**
 * The destination to pop up to before navigating. When set, all non-matching destinations
 * should be popped from the back stack.
 * @return the destinationId to pop up to, clearing all intervening destinations
 * @see Builder#setPopUpTo
 * @see #isPopUpToInclusive
 */
int NavOptions::getPopUpTo() const{
    return mPopUpTo;
}

/**
 * Whether the destination set in {@link #getPopUpTo} should be popped from the back stack.
 * @see Builder#setPopUpTo
 * @see #getPopUpTo
 */
bool NavOptions::isPopUpToInclusive() const{
    return mPopUpToInclusive;
}

/**
 * The custom enter Animation/Animator that should be run.
 * @return the resource id of a Animation or Animator or -1 if none.
 */
const std::string NavOptions::getEnterAnim() const{
    return mEnterAnim;
}

/**
 * The custom exit Animation/Animator that should be run.
 * @return the resource id of a Animation or Animator or -1 if none.
 */
const std::string NavOptions::getExitAnim() const{
    return mExitAnim;
}

/**
 * The custom enter Animation/Animator that should be run when this destination is
 * popped from the back stack.
 * @return the resource id of a Animation or Animator or -1 if none.
 * @see #applyPopAnimationsToPendingTransition(Activity)
 */
const std::string NavOptions::getPopEnterAnim() const{
    return mPopEnterAnim;
}

/**
 * The custom exit Animation/Animator that should be run when this destination is
 * popped from the back stack.
 * @return the resource id of a Animation or Animator or -1 if none.
 * @see #applyPopAnimationsToPendingTransition(Activity)
 */
const std::string NavOptions::getPopExitAnim() const{
    return mPopExitAnim;
}
#if 0
private Bundle toBundle() {
    Bundle b = new Bundle();
    b.putInt(KEY_LAUNCH_MODE, mLaunchMode);
    b.putInt(KEY_POP_UP_TO, mPopUpTo);
    b.putBoolean(KEY_POP_UP_TO_INCLUSIVE, mPopUpToInclusive);
    b.putInt(KEY_ENTER_ANIM, mEnterAnim);
    b.putInt(KEY_EXIT_ANIM, mExitAnim);
    b.putInt(KEY_POP_ENTER_ANIM, mPopEnterAnim);
    b.putInt(KEY_POP_EXIT_ANIM, mPopExitAnim);
    return b;
}

@NonNull
private static NavOptions fromBundle(@NonNull Bundle b) {
    return new NavOptions(b.getInt(KEY_LAUNCH_MODE, 0),
            b.getInt(KEY_POP_UP_TO, 0), b.getBoolean(KEY_POP_UP_TO_INCLUSIVE, false),
            b.getInt(KEY_ENTER_ANIM, -1), b.getInt(KEY_EXIT_ANIM, -1),
            b.getInt(KEY_POP_ENTER_ANIM, -1), b.getInt(KEY_POP_EXIT_ANIM, -1));
}

/**
 * Builder for constructing new instances of NavOptions.
 */
public static class Builder {
    int mLaunchMode;
    @IdRes
    int mPopUpTo;
    boolean mPopUpToInclusive;
    @AnimRes @AnimatorRes
    int mEnterAnim = -1;
    @AnimRes @AnimatorRes
    int mExitAnim = -1;
    @AnimRes @AnimatorRes
    int mPopEnterAnim = -1;
    @AnimRes @AnimatorRes
    int mPopExitAnim = -1;

    public Builder() {
    }

    /**
     * Launch a navigation target as single-top if you are making a lateral navigation
     * between instances of the same target (e.g. detail pages about similar data items)
     * that should not preserve history.
     *
     * @param singleTop true to launch as single-top
     */
    @NonNull
    public Builder setLaunchSingleTop(boolean singleTop) {
        if (singleTop) {
            mLaunchMode |= LAUNCH_SINGLE_TOP;
        } else {
            mLaunchMode &= ~LAUNCH_SINGLE_TOP;
        }
        return this;
    }

    /**
     * Launch a navigation target as a document if you want it to appear as its own
     * entry in the system Overview screen. If the same document is launched multiple times
     * it will not create a new task, it will bring the existing document task to the front.
     *
     * <p>If the user presses the system Back key from a new document task they will land
     * on their previous task. If the user reached the document task from the system Overview
     * screen they will be taken to their home screen.</p>
     *
     * @param launchDocument true to launch a new document task
     * @deprecated As per the {@link android.content.Intent#FLAG_ACTIVITY_NEW_DOCUMENT}
     * documentation, it is recommended to use {@link android.R.attr#documentLaunchMode} on an
     * Activity you wish to launch as a new document.
     */
    @Deprecated
    @NonNull
    public Builder setLaunchDocument(boolean launchDocument) {
        if (launchDocument) {
            mLaunchMode |= LAUNCH_DOCUMENT;
        } else {
            mLaunchMode &= ~LAUNCH_DOCUMENT;
        }
        return this;
    }

    /**
     * Clear the entire task before launching this target. If you are launching as a
     * {@link #setLaunchDocument(boolean) document}, this will clear the document task.
     * Otherwise it will clear the current task.
     *
     * @param clearTask
     * @return
     * @deprecated Use {@link #setPopUpTo(int, boolean)} with the
     * {@link NavDestination#getId() id} of the
     * {@link androidx.navigation.NavController#getGraph() NavController's graph}
     * and set inclusive to true.
     */
    @Deprecated
    @NonNull
    public Builder setClearTask(boolean clearTask) {
        if (clearTask) {
            mLaunchMode |= LAUNCH_CLEAR_TASK;
        } else {
            mLaunchMode &= ~LAUNCH_CLEAR_TASK;
        }
        return this;
    }

    /**
     * Pop up to a given destination before navigating. This pops all non-matching destinations
     * from the back stack until this destination is found.
     *
     * @param destinationId The destination to pop up to, clearing all intervening destinations.
     * @param inclusive true to also pop the given destination from the back stack.
     * @return this Builder
     * @see NavOptions#getPopUpTo
     * @see NavOptions#isPopUpToInclusive
     */
    @NonNull
    public Builder setPopUpTo(@IdRes int destinationId, boolean inclusive) {
        mPopUpTo = destinationId;
        mPopUpToInclusive = inclusive;
        return this;
    }

    /**
     * Sets a custom Animation or Animator resource for the enter animation.
     *
     * <p>Note: Animator resources are not supported for navigating to a new Activity</p>
     * @param enterAnim Custom animation to run
     * @return this Builder
     * @see NavOptions#getEnterAnim()
     */
    @NonNull
    public Builder setEnterAnim(@AnimRes @AnimatorRes int enterAnim) {
        mEnterAnim = enterAnim;
        return this;
    }

    /**
     * Sets a custom Animation or Animator resource for the exit animation.
     *
     * <p>Note: Animator resources are not supported for navigating to a new Activity</p>
     * @param exitAnim Custom animation to run
     * @return this Builder
     * @see NavOptions#getExitAnim()
     */
    @NonNull
    public Builder setExitAnim(@AnimRes @AnimatorRes int exitAnim) {
        mExitAnim = exitAnim;
        return this;
    }

    /**
     * Sets a custom Animation or Animator resource for the enter animation
     * when popping off the back stack.
     *
     * <p>Note: Animator resources are not supported for navigating to a new Activity</p>
     * @param popEnterAnim Custom animation to run
     * @return this Builder
     * @see NavOptions#getPopEnterAnim()
     */
    @NonNull
    public Builder setPopEnterAnim(@AnimRes @AnimatorRes int popEnterAnim) {
        mPopEnterAnim = popEnterAnim;
        return this;
    }

    /**
     * Sets a custom Animation or Animator resource for the exit animation
     * when popping off the back stack.
     *
     * <p>Note: Animator resources are not supported for navigating to a new Activity</p>
     * @param popExitAnim Custom animation to run
     * @return this Builder
     * @see NavOptions#getPopExitAnim()
     */
    @NonNull
    public Builder setPopExitAnim(@AnimRes @AnimatorRes int popExitAnim) {
        mPopExitAnim = popExitAnim;
        return this;
    }

    /**
     * @return a constructed NavOptions
     */
    @NonNull
    public NavOptions build() {
        return new NavOptions(mLaunchMode, mPopUpTo, mPopUpToInclusive,
                mEnterAnim, mExitAnim, mPopEnterAnim, mPopExitAnim);
    }
}
#endif
}/*endof namespace*/
#endif
