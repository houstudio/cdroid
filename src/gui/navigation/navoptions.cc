#include <navigation/navoptions.h>
namespace cdroid{

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

NavOptions::NavOptions(int launchMode, const std::string& popUpTo, bool popUpToInclusive,
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
const std::string NavOptions::getPopUpTo() const{
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

Bundle* NavOptions::toBundle() {
    Bundle* b = new Bundle();
    b->putInt(KEY_LAUNCH_MODE, mLaunchMode);
    b->putString(KEY_POP_UP_TO, mPopUpTo);
    b->putBoolean(KEY_POP_UP_TO_INCLUSIVE, mPopUpToInclusive);
    b->putString(KEY_ENTER_ANIM, mEnterAnim);
    b->putString(KEY_EXIT_ANIM, mExitAnim);
    b->putString(KEY_POP_ENTER_ANIM, mPopEnterAnim);
    b->putString(KEY_POP_EXIT_ANIM, mPopExitAnim);
    return b;
}

NavOptions* NavOptions::fromBundle(const Bundle& b) {
    return nullptr;/*new NavOptions(b.getInt(KEY_LAUNCH_MODE, 0),
            b.getInt(KEY_POP_UP_TO, 0), b.getBoolean(KEY_POP_UP_TO_INCLUSIVE, false),
            b.getInt(KEY_ENTER_ANIM, -1), b.getInt(KEY_EXIT_ANIM, -1),
            b.getInt(KEY_POP_ENTER_ANIM, -1), b.getInt(KEY_POP_EXIT_ANIM, -1));*/
}

/**
 * Builder for constructing new instances of NavOptions.
 */

NavOptions::Builder::Builder() {
}

NavOptions::Builder& NavOptions::Builder::setLaunchSingleTop(bool singleTop) {
    if (singleTop) {
        mLaunchMode |= LAUNCH_SINGLE_TOP;
    } else {
        mLaunchMode &= ~LAUNCH_SINGLE_TOP;
    }
    return *this;
}

NavOptions::Builder& NavOptions::Builder::setLaunchDocument(bool launchDocument) {
    if (launchDocument) {
        mLaunchMode |= LAUNCH_DOCUMENT;
    } else {
        mLaunchMode &= ~LAUNCH_DOCUMENT;
    }
    return *this;
}

NavOptions::Builder& NavOptions::Builder::setClearTask(bool clearTask) {
    if (clearTask) {
        mLaunchMode |= LAUNCH_CLEAR_TASK;
    } else {
        mLaunchMode &= ~LAUNCH_CLEAR_TASK;
    }
    return *this;
}

NavOptions::Builder& NavOptions::Builder::setPopUpTo(const std::string& destinationId, bool inclusive) {
    mPopUpTo = destinationId;
    mPopUpToInclusive = inclusive;
    return *this;
}

NavOptions::Builder& NavOptions::Builder::setEnterAnim(const std::string&enterAnim) {
    mEnterAnim = enterAnim;
    return *this;
}

NavOptions::Builder& NavOptions::Builder::setExitAnim(const std::string& exitAnim) {
    mExitAnim = exitAnim;
    return *this;
}

NavOptions::Builder& NavOptions::Builder::setPopEnterAnim(const std::string& popEnterAnim) {
    mPopEnterAnim = popEnterAnim;
    return *this;
}

NavOptions::Builder& NavOptions::Builder::setPopExitAnim(const std::string& popExitAnim) {
    mPopExitAnim = popExitAnim;
    return *this;
}

NavOptions* NavOptions::Builder::build() {
    return new NavOptions(mLaunchMode, mPopUpTo, mPopUpToInclusive,
            mEnterAnim, mExitAnim, mPopEnterAnim, mPopExitAnim);
}

}/*endof namespace*/
