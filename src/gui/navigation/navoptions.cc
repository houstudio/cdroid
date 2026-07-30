#include <navigation/navoptions.h>
namespace cdroid{

void NavOptions::addPopAnimationsToIntent(Intent& intent,NavOptions* navOptions) {
    if (navOptions != nullptr) {
        //intent.putExtra(KEY_NAV_OPTIONS, navOptions->toBundle());
    }
}

void NavOptions::applyPopAnimationsToPendingTransition(Activity& activity) {
    /*Intent intent = activity.getIntent();
    ... (Android pending transition; stubbed in CDROID) */
}

NavOptions::NavOptions(int launchMode, const std::string& popUpTo, bool popUpToInclusive,
        const std::string& enterAnim, const std::string& exitAnim,
        const std::string& popEnterAnim, const std::string& popExitAnim)
    : NavOptions(launchMode, popUpTo, popUpToInclusive, enterAnim, exitAnim,
                 popEnterAnim, popExitAnim, false, false){}

NavOptions::NavOptions(int launchMode, const std::string& popUpTo, bool popUpToInclusive,
        const std::string& enterAnim, const std::string& exitAnim,
        const std::string& popEnterAnim, const std::string& popExitAnim,
        bool shouldRestoreState, bool shouldPopUpToSaveState) {
    mLaunchMode = launchMode;
    mPopUpTo = popUpTo;
    mPopUpToInclusive = popUpToInclusive;
    mEnterAnim = enterAnim;
    mExitAnim = exitAnim;
    mPopEnterAnim = popEnterAnim;
    mPopExitAnim = popExitAnim;
    mShouldRestoreState = shouldRestoreState;
    mShouldPopUpToSaveState = shouldPopUpToSaveState;
}

bool NavOptions::shouldLaunchSingleTop() const{ return (mLaunchMode & LAUNCH_SINGLE_TOP) != 0; }
bool NavOptions::shouldLaunchDocument() const{ return (mLaunchMode & LAUNCH_DOCUMENT) != 0; }
bool NavOptions::shouldClearTask() const{ return (mLaunchMode & LAUNCH_CLEAR_TASK) != 0; }
const std::string NavOptions::getPopUpTo() const{ return mPopUpTo; }
bool NavOptions::isPopUpToInclusive() const{ return mPopUpToInclusive; }
const std::string NavOptions::getEnterAnim() const{ return mEnterAnim; }
const std::string NavOptions::getExitAnim() const{ return mExitAnim; }
const std::string NavOptions::getPopEnterAnim() const{ return mPopEnterAnim; }
const std::string NavOptions::getPopExitAnim() const{ return mPopExitAnim; }

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
    return nullptr; // CDROID: Bundle getXxx throws on missing key; restore not used yet.
}

NavOptions::Builder::Builder() {}

NavOptions::Builder& NavOptions::Builder::setLaunchSingleTop(bool singleTop) {
    if (singleTop) mLaunchMode |= LAUNCH_SINGLE_TOP; else mLaunchMode &= ~LAUNCH_SINGLE_TOP;
    return *this;
}
NavOptions::Builder& NavOptions::Builder::setLaunchDocument(bool launchDocument) {
    if (launchDocument) mLaunchMode |= LAUNCH_DOCUMENT; else mLaunchMode &= ~LAUNCH_DOCUMENT;
    return *this;
}
NavOptions::Builder& NavOptions::Builder::setClearTask(bool clearTask) {
    if (clearTask) mLaunchMode |= LAUNCH_CLEAR_TASK; else mLaunchMode &= ~LAUNCH_CLEAR_TASK;
    return *this;
}
NavOptions::Builder& NavOptions::Builder::setPopUpTo(const std::string& destinationId, bool inclusive) {
    mPopUpTo = destinationId;
    mPopUpToInclusive = inclusive;
    return *this;
}
NavOptions::Builder& NavOptions::Builder::setPopUpTo(const std::string& route, bool inclusive, bool saveState) {
    mPopUpTo = route;
    mPopUpToInclusive = inclusive;
    mShouldPopUpToSaveState = saveState;
    return *this;
}
NavOptions::Builder& NavOptions::Builder::setRestoreState(bool restoreState) {
    mShouldRestoreState = restoreState;
    return *this;
}
NavOptions::Builder& NavOptions::Builder::setEnterAnim(const std::string& enterAnim) { mEnterAnim = enterAnim; return *this; }
NavOptions::Builder& NavOptions::Builder::setExitAnim(const std::string& exitAnim) { mExitAnim = exitAnim; return *this; }
NavOptions::Builder& NavOptions::Builder::setPopEnterAnim(const std::string& popEnterAnim) { mPopEnterAnim = popEnterAnim; return *this; }
NavOptions::Builder& NavOptions::Builder::setPopExitAnim(const std::string& popExitAnim) { mPopExitAnim = popExitAnim; return *this; }

NavOptions* NavOptions::Builder::build() {
    return new NavOptions(mLaunchMode, mPopUpTo, mPopUpToInclusive,
        mEnterAnim, mExitAnim, mPopEnterAnim, mPopExitAnim,
        mShouldRestoreState, mShouldPopUpToSaveState);
}

}/*endof namespace*/
