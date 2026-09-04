#include <timerfragment.h>

#include <R.h>

#include <algorithm>

#include <animation/animator.h>
#include <animation/animatorset.h>
#include <animation/interpolators.h>
#include <animation/objectanimator.h>
#include <content/resources.h>
#include <core/bundle.h>
#include <core/context.h>
#include <core/intent.h>
#include <core/systemclock.h>
#include <fragment/fragmentfactory.h>
#include <widget/cdwindow.h>

#include <animatorutils.h>
#include <datamodel.h>
#include <timerstringformatter.h>
#include <uidata.h>
#include <utils.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace timer {

namespace {
constexpr const char* EXTRA_TIMER_SETUP = "com.android.deskclock.action.TIMER_SETUP";
// TimerService.EXTRA_TIMER_ID (the service itself is a recorded stub).
constexpr const char* EXTRA_TIMER_ID = "com.android.deskclock.timer.extra.TIMMER_ID";
constexpr const char* KEY_TIMER_SETUP_STATE = "timer_setup_input";
} // namespace

TimerFragment::TimerFragment() : DeskClockFragment(uidata::Tab::TIMERS) {
    // The one-shot pre-draw body of animateToView (upstream: anonymous OnPreDrawListener).
    mPreDrawListener = [this]() -> bool {
        if (!mPreDrawArmed) return true;
        mPreDrawArmed = false;

        View* toView = mAnimToView;
        const bool toTimers = (toView == mTimersView);
        const data::Timer* timerToRemove = mAnimTimerToRemove;
        const int64_t animationDuration =
                uidata::UiDataModel::getUiDataModel().getLongAnimationDuration();

        View* view = mTimersView->findViewById(R::id::timer_time);
        const float distanceY = (view != nullptr) ? view->getHeight() + view->getY() : 0.0f;
        const float translationDistance = mAnimAnimateDown ? distanceY : -distanceY;

        toView->setTranslationY(-translationDistance);
        if (mCurrentView != nullptr) mCurrentView->setTranslationY(0.0f);
        toView->setAlpha(0.0f);
        if (mCurrentView != nullptr) mCurrentView->setAlpha(1.0f);

        ObjectAnimator* translateCurrent = ObjectAnimator::ofFloat(mCurrentView,
                View::TRANSLATION_Y, {translationDistance});
        ObjectAnimator* translateNew = ObjectAnimator::ofFloat(toView,
                View::TRANSLATION_Y, {0.0f});
        AnimatorSet* translationAnimatorSet = new AnimatorSet();
        translationAnimatorSet->playTogether({translateCurrent, translateNew});
        translationAnimatorSet->setDuration(animationDuration);
        translationAnimatorSet->setInterpolator(AnimatorUtils::INTERPOLATOR_FAST_OUT_SLOW_IN());

        ObjectAnimator* fadeOutAnimator = ObjectAnimator::ofFloat(mCurrentView,
                View::ALPHA, {0.0f});
        fadeOutAnimator->setDuration(animationDuration / 2);
        Animator::AnimatorListener fadeOutListener;
        fadeOutListener.onAnimationStart = [this](Animator&, bool) {
            // The fade-out animation and fab-shrinking animation should run together.
            updateFab(FabContainer::FAB_AND_BUTTONS_SHRINK);
        };
        fadeOutListener.onAnimationEnd = [this, toTimers, timerToRemove](Animator&, bool) {
            if (toTimers) {
                showTimersView(FabContainer::FAB_AND_BUTTONS_EXPAND);

                // Reset the state of the create view.
                mCreateTimerView->reset();
            } else {
                showCreateTimerView(FabContainer::FAB_AND_BUTTONS_EXPAND);
            }
            if (timerToRemove != nullptr) {
                data::DataModel::getDataModel().removeTimer(*timerToRemove);
            }

            // Update the fab and button states now that the correct view is visible and
            // before the animation to expand the fab and buttons starts.
            updateFab(FabContainer::FAB_AND_BUTTONS_IMMEDIATE);
        };
        fadeOutAnimator->addListener(fadeOutListener);

        ObjectAnimator* fadeInAnimator = ObjectAnimator::ofFloat(toView, View::ALPHA, {1.0f});
        fadeInAnimator->setDuration(animationDuration / 2);
        fadeInAnimator->setStartDelay(animationDuration / 2);

        AnimatorSet* animatorSet = new AnimatorSet();
        animatorSet->playTogether({fadeOutAnimator, fadeInAnimator, translationAnimatorSet});
        Animator::AnimatorListener setListener;
        setListener.onAnimationEnd = [this](Animator&, bool) {
            mTimersView->setTranslationY(0.0f);
            mCreateTimerView->setTranslationY(0.0f);
            mTimersView->setAlpha(1.0f);
            mCreateTimerView->setAlpha(1.0f);
        };
        animatorSet->addListener(setListener);
        animatorSet->start();

        return true;
    };

    mTimeUpdateRunnable = [this]() {
        const int64_t startTime = SystemClock::elapsedRealtime();
        // If no timers require continuous updates, avoid scheduling the next update.
        if (!mAdapter->updateTime()) {
            return;
        }
        const int64_t endTime = SystemClock::elapsedRealtime();

        // Try to maintain a consistent period of time between redraws.
        const int64_t delay = std::max((int64_t) 0, startTime + 20 - endTime);
        if (mTimersView != nullptr) mTimersView->postDelayed(mTimeUpdateRunnable, delay);
    };

    // TimerPageChangeListener.
    mTimerPageChangeListener.onPageSelected = [this](int) {
        updatePageIndicators();
        updateFab(FabContainer::FAB_AND_BUTTONS_IMMEDIATE);

        // Showing a new timer page may introduce a timer requiring continuous updates.
        startUpdatingTime();
    };
    mTimerPageChangeListener.onPageScrollStateChanged = [this](int state) {
        // Teasing a neighboring timer may introduce a timer requiring continuous updates.
        if (state == ViewPager::SCROLL_STATE_DRAGGING) {
            startUpdatingTime();
        }
    };

    // TimerWatcher.
    mTimerWatcher.timerAdded = [this](const data::Timer&) {
        updatePageIndicators();
        // If the timer is being created via this fragment avoid adjusting the fab.
        // Timer setup view is about to be animated away in response to this timer
        // creation; changes to the fab immediately preceding that animation are jarring.
        if (!mCreatingTimer) {
            updateFab(FabContainer::FAB_AND_BUTTONS_IMMEDIATE);
        }
    };
    mTimerWatcher.timerUpdated = [this](const data::Timer& before, const data::Timer& after) {
        // If the timer started, animate the timers.
        if (before.isReset() && !after.isReset()) {
            startUpdatingTime();
        }

        // Fetch the index of the change.
        const std::vector<data::Timer>& timers = data::DataModel::getDataModel().getTimers();
        int index = -1;
        for (size_t i = 0; i < timers.size(); i++) {
            if (timers[i].id == after.id) {
                index = (int) i;
                break;
            }
        }

        // If the timer just expired but is not displayed, display it now.
        if (!before.isExpired() && after.isExpired() && index != mViewPager->getCurrentItem()) {
            mViewPager->setCurrentItem(index, true);
        } else if (mCurrentView == mTimersView && index == mViewPager->getCurrentItem()) {
            // Morph the fab from its old state to new state if necessary.
            if (before.state != after.state && !(before.isPaused() && after.isReset())) {
                updateFab(FabContainer::FAB_MORPH);
            }
        }
    };
    mTimerWatcher.timerRemoved = [this](const data::Timer&) {
        updatePageIndicators();
        updateFab(FabContainer::FAB_AND_BUTTONS_IMMEDIATE);

        if (mCurrentView == mTimersView && mAdapter->getCount() == 0) {
            animateToView(mCreateTimerView, nullptr, false);
        }
    };
}

TimerFragment::~TimerFragment() {
    delete mAdapter;
}

View* TimerFragment::onCreateView(LayoutInflater* inflater, ViewGroup* container,
                                  Bundle* savedInstanceState) {
    View* view = inflater->inflate(R::layout::timer_fragment, container, false);
    Utils::setDefaultBackground(view);

    mAdapter = new TimerPagerAdapter(getParentFragmentManager());
    mViewPager = (ViewPager*) view->findViewById(R::id::vertical_view_pager);
    mViewPager->setAdapter(mAdapter);
    mViewPager->addOnPageChangeListener(mTimerPageChangeListener);

    mTimersView = view->findViewById(R::id::timer_view);
    mCreateTimerView = (TimerSetupView*) view->findViewById(R::id::timer_setup);
    mCreateTimerView->setFabContainer(this);
    mPageIndicators[0] = (ImageView*) view->findViewById(R::id::page_indicator0);
    mPageIndicators[1] = (ImageView*) view->findViewById(R::id::page_indicator1);
    mPageIndicators[2] = (ImageView*) view->findViewById(R::id::page_indicator2);
    mPageIndicators[3] = (ImageView*) view->findViewById(R::id::page_indicator3);

    data::DataModel::getDataModel().addTimerListener(mAdapter->mTimerListener);
    data::DataModel::getDataModel().addTimerListener(mTimerWatcher);

    // If timer setup state is present, retrieve it to be later honored.
    if (savedInstanceState != nullptr) {
        // Upstream stores the IntArray Serializable; cdroid Bundle has no int-array
        // face, so the digits travel as a comma-separated string.
        const std::string saved = savedInstanceState->getString(KEY_TIMER_SETUP_STATE);
        if (!saved.empty()) {
            mTimerSetupState.clear();
            size_t pos = 0;
            while (pos <= saved.size()) {
                const size_t comma = saved.find(',', pos);
                mTimerSetupState.push_back(
                        atoi(saved.substr(pos, comma - pos).c_str()));
                if (comma == std::string::npos) break;
                pos = comma + 1;
            }
            mHasTimerSetupState = !mTimerSetupState.empty();
        }
    }

    return view;
}

void TimerFragment::onStart() {
    DeskClockFragment::onStart();

    // Initialize the page indicators.
    updatePageIndicators();
    bool createTimer = false;
    int showTimerId = -1;

    // Examine the intent of the parent activity to determine which view to display.
    Window* activity = getActivity();
    if (activity != nullptr && activity->getIntent().hasExtra(EXTRA_TIMER_SETUP)) {
        // These extras are single-use; remove them after honoring them.
        createTimer = activity->getIntent().getBooleanExtra(EXTRA_TIMER_SETUP, false);
        activity->getIntent().removeExtra(EXTRA_TIMER_SETUP);
    }
    if (activity != nullptr && activity->getIntent().hasExtra(EXTRA_TIMER_ID)) {
        showTimerId = activity->getIntent().getIntExtra(EXTRA_TIMER_ID, -1);
        activity->getIntent().removeExtra(EXTRA_TIMER_ID);
    }

    // Choose the view to display in this fragment.
    if (showTimerId != -1) {
        // A specific timer must be shown; show the list of timers.
        showTimersView(FabContainer::FAB_AND_BUTTONS_IMMEDIATE);
    } else if (!hasTimers() || createTimer || mHasTimerSetupState) {
        // No timers exist, a timer is being created, or the last view was timer setup;
        // show the timer setup view.
        showCreateTimerView(FabContainer::FAB_AND_BUTTONS_IMMEDIATE);

        if (mHasTimerSetupState) {
            mCreateTimerView->setState(mTimerSetupState);
            mTimerSetupState.clear();
            mHasTimerSetupState = false;
        }
    } else {
        // Otherwise, default to showing the list of timers.
        showTimersView(FabContainer::FAB_AND_BUTTONS_IMMEDIATE);
    }

    // If the intent did not specify a timer to show, show the last timer that expired.
    if (showTimerId == -1) {
        data::Timer expired;
        if (data::DataModel::getDataModel().getMostRecentExpiredTimer(expired)) {
            showTimerId = expired.id;
        }
    }

    // If a specific timer should be displayed, display the corresponding timer tab.
    if (showTimerId != -1) {
        const std::vector<data::Timer>& timers = data::DataModel::getDataModel().getTimers();
        for (size_t i = 0; i < timers.size(); i++) {
            if (timers[i].id == showTimerId) {
                mViewPager->setCurrentItem((int) i);
                break;
            }
        }
    }
}

void TimerFragment::onResume() {
    DeskClockFragment::onResume();

    // We may have received a new intent while paused.
    Window* activity = getActivity();
    if (activity != nullptr && activity->getIntent().hasExtra(EXTRA_TIMER_ID)) {
        // This extra is single-use; remove after honoring it.
        const int showTimerId = activity->getIntent().getIntExtra(EXTRA_TIMER_ID, -1);
        activity->getIntent().removeExtra(EXTRA_TIMER_ID);

        const std::vector<data::Timer>& timers = data::DataModel::getDataModel().getTimers();
        for (size_t i = 0; i < timers.size(); i++) {
            if (timers[i].id == showTimerId) {
                // A specific timer must be shown; show the list of timers.
                mViewPager->setCurrentItem((int) i);
                animateToView(mTimersView, nullptr, false);
                break;
            }
        }
    }
}

void TimerFragment::onStop() {
    DeskClockFragment::onStop();

    // Stop updating the timers when this fragment is no longer visible.
    stopUpdatingTime();
}

void TimerFragment::onDestroyView() {
    DeskClockFragment::onDestroyView();

    data::DataModel::getDataModel().removeTimerListener(mAdapter->mTimerListener);
    data::DataModel::getDataModel().removeTimerListener(mTimerWatcher);
}

void TimerFragment::onSaveInstanceState(Bundle* outState) {
    DeskClockFragment::onSaveInstanceState(outState);

    // If the timer creation view is visible, store the input for later restoration.
    if (mCurrentView == mCreateTimerView) {
        mTimerSetupState = mCreateTimerView->getState();
        mHasTimerSetupState = true;
        // The IntArray travels as a comma-separated string (see onCreateView).
        std::string joined;
        for (size_t i = 0; i < mTimerSetupState.size(); i++) {
            if (i > 0) joined += ',';
            joined += std::to_string(mTimerSetupState[i]);
        }
        outState->putString(KEY_TIMER_SETUP_STATE, joined);
    }
}

void TimerFragment::updateFab(ImageView& fab, bool animate) {
    if (mCurrentView == mTimersView) {
        data::Timer timer;
        if (!getCurrentTimer(timer)) {
            fab.setVisibility(View::INVISIBLE);
            return;
        }

        fab.setVisibility(View::VISIBLE);
        Resources& res = fab.getContext()->getResources();
        switch (timer.state) {
            case data::Timer::State::RUNNING:
                fab.setImageResource(animate ? R::drawable::ic_play_pause_animation
                                             : R::drawable::ic_play_pause);
                fab.setContentDescription(res.getString(R::string::timer_stop));
                break;
            case data::Timer::State::RESET:
                fab.setImageResource(animate ? R::drawable::ic_stop_play_animation
                                             : R::drawable::ic_pause_play);
                fab.setContentDescription(res.getString(R::string::timer_start));
                break;
            case data::Timer::State::PAUSED:
                fab.setImageResource(animate ? R::drawable::ic_pause_play_animation
                                             : R::drawable::ic_pause_play);
                fab.setContentDescription(res.getString(R::string::timer_start));
                break;
            case data::Timer::State::MISSED:
            case data::Timer::State::EXPIRED:
                fab.setImageResource(R::drawable::ic_stop_white_24dp);
                fab.setContentDescription(res.getString(R::string::timer_stop));
                break;
        }
    } else if (mCurrentView == mCreateTimerView) {
        if (mCreateTimerView->hasValidInput()) {
            fab.setImageResource(R::drawable::ic_start_white_24dp);
            fab.setContentDescription(
                    fab.getContext()->getResources().getString(R::string::timer_start));
            fab.setVisibility(View::VISIBLE);
        } else {
            // Upstream sets contentDescription = null; cdroid's string-based face has
            // no null state, so "" is the canonical "no description".
            fab.setContentDescription(std::string());
            fab.setVisibility(View::INVISIBLE);
        }
    }
}

void TimerFragment::onUpdateFab(ImageView& fab) {
    updateFab(fab, false);
}

void TimerFragment::onMorphFab(ImageView& fab) {
    // Update the fab's drawable to match the current timer state.
    updateFab(fab, Utils::isNOrLater);
    // Animate the drawable.
    AnimatorUtils::startDrawableAnimation(fab);
}

void TimerFragment::onUpdateFabButtons(Button& left, Button& right) {
    Resources& resources = getContext()->getResources();
    if (mCurrentView == mTimersView) {
        left.setClickable(true);
        left.setText(R::string::timer_delete);
        left.setContentDescription(resources.getString(R::string::timer_delete));
        left.setVisibility(View::VISIBLE);

        right.setClickable(true);
        right.setText(R::string::timer_add_timer);
        right.setContentDescription(resources.getString(R::string::timer_add_timer));
        right.setVisibility(View::VISIBLE);
    } else if (mCurrentView == mCreateTimerView) {
        left.setClickable(true);
        left.setText(R::string::timer_cancel);
        left.setContentDescription(resources.getString(R::string::timer_cancel));
        // If no timers yet exist, the user is forced to create the first one.
        left.setVisibility(hasTimers() ? View::VISIBLE : View::INVISIBLE);

        right.setVisibility(View::INVISIBLE);
    }
}

void TimerFragment::onFabClick(ImageView& fab) {
    if (mCurrentView == mTimersView) {
        // If no timer is currently showing a fab action is meaningless.
        data::Timer timer;
        if (!getCurrentTimer(timer)) return;

        Context& context = *fab.getContext();
        const int64_t currentTime = timer.getRemainingTime();

        switch (timer.state) {
            case data::Timer::State::RUNNING:
                data::DataModel::getDataModel().pauseTimer(timer);
                if (currentTime > 0 && mTimersView != nullptr) {
                    mTimersView->announceForAccessibility(
                            data::TimerStringFormatter::formatString(context,
                                    R::string::timer_accessibility_stopped, currentTime, true));
                }
                break;
            case data::Timer::State::PAUSED:
            case data::Timer::State::RESET:
                data::DataModel::getDataModel().startTimer(timer);
                if (currentTime > 0 && mTimersView != nullptr) {
                    mTimersView->announceForAccessibility(
                            data::TimerStringFormatter::formatString(context,
                                    R::string::timer_accessibility_started, currentTime, true));
                }
                break;
            case data::Timer::State::MISSED:
            case data::Timer::State::EXPIRED: {
                data::Timer outTimer;
                data::DataModel::getDataModel().resetOrDeleteTimer(timer,
                        R::string::label_deskclock, outTimer);
                break;
            }
        }
    } else if (mCurrentView == mCreateTimerView) {
        mCreatingTimer = true;
        // Create the new timer.
        const int64_t timerLength = mCreateTimerView->getTimeInMillis();
        data::Timer timer = data::DataModel::getDataModel().addTimer(timerLength, "", false);

        // Start the new timer.
        data::DataModel::getDataModel().startTimer(timer);

        mCreatingTimer = false;

        // Display the freshly created timer view.
        mViewPager->setCurrentItem(0);

        // Return to the list of timers.
        animateToView(mTimersView, nullptr, true);
    }
}

void TimerFragment::onLeftButtonClick(Button& left) {
    if (mCurrentView == mTimersView) {
        // Clicking the "delete" button.
        data::Timer timer;
        if (!getCurrentTimer(timer)) return;

        if (mAdapter->getCount() > 1) {
            animateTimerRemove(timer);
        } else {
            animateToView(mCreateTimerView, &timer, false);
        }

        left.announceForAccessibility(getContext()->getString(R::string::timer_deleted));
    } else if (mCurrentView == mCreateTimerView) {
        // Clicking the "cancel" button on the timer creation page returns to the timers list.
        mCreateTimerView->reset();

        animateToView(mTimersView, nullptr, false);

        left.announceForAccessibility(getContext()->getString(R::string::timer_canceled));
    }
}

void TimerFragment::onRightButtonClick(Button& /*right*/) {
    if (mCurrentView != mCreateTimerView) {
        animateToView(mCreateTimerView, nullptr, true);
    }
}

bool TimerFragment::onKeyDown(int keyCode, KeyEvent& event) {
    if (mCurrentView == mCreateTimerView) {
        return mCreateTimerView->onKeyDown(keyCode, event);
    }
    return DeskClockFragment::onKeyDown(keyCode, event);
}

void TimerFragment::updatePageIndicators() {
    const int page = mViewPager->getCurrentItem();
    const int pageIndicatorCount = (int) (sizeof(mPageIndicators) / sizeof(mPageIndicators[0]));
    const int pageCount = mAdapter->getCount();

    const std::vector<int> states =
            computePageIndicatorStates(page, pageIndicatorCount, pageCount);
    for (int i = 0; i < pageIndicatorCount; i++) {
        const int state = states[i];
        ImageView* pageIndicator = mPageIndicators[i];
        if (state == 0) {
            pageIndicator->setVisibility(View::GONE);
        } else {
            pageIndicator->setVisibility(View::VISIBLE);
            pageIndicator->setImageResource(state);
        }
    }
}

void TimerFragment::showCreateTimerView(int updateTypes) {
    // Stop animating the timers.
    stopUpdatingTime();

    // Show the creation view; hide the timer view.
    mTimersView->setVisibility(View::GONE);
    mCreateTimerView->setVisibility(View::VISIBLE);

    // Record the fact that the create view is visible.
    mCurrentView = mCreateTimerView;

    // Update the fab and buttons.
    updateFab(updateTypes);
}

void TimerFragment::showTimersView(int updateTypes) {
    // Clear any defunct timer creation state; the next timer creation starts fresh.
    mTimerSetupState.clear();
    mHasTimerSetupState = false;

    // Show the timer view; hide the creation view.
    mTimersView->setVisibility(View::VISIBLE);
    mCreateTimerView->setVisibility(View::GONE);

    // Record the fact that the timers view is visible.
    mCurrentView = mTimersView;

    // Update the fab and buttons.
    updateFab(updateTypes);

    // Start animating the timers.
    startUpdatingTime();
}

void TimerFragment::animateTimerRemove(const data::Timer& timerToRemove) {
    const int64_t duration = uidata::UiDataModel::getUiDataModel().getShortAnimationDuration();

    ObjectAnimator* fadeOut = ObjectAnimator::ofFloat(mViewPager, View::ALPHA, {1.0f, 0.0f});
    fadeOut->setDuration(duration);
    fadeOut->setInterpolator(new DecelerateInterpolator());
    Animator::AnimatorListener outListener;
    outListener.onAnimationEnd = [this, timerToRemove](Animator&, bool) {
        data::DataModel::getDataModel().removeTimer(timerToRemove);
    };
    fadeOut->addListener(outListener);

    ObjectAnimator* fadeIn = ObjectAnimator::ofFloat(mViewPager, View::ALPHA, {0.0f, 1.0f});
    fadeIn->setDuration(duration);
    fadeIn->setInterpolator(new AccelerateInterpolator());

    AnimatorSet* animatorSet = new AnimatorSet();
    AnimatorSet::Builder* builder = animatorSet->play(fadeOut);
    builder->before(fadeIn);
    animatorSet->start();
}

void TimerFragment::animateToView(View* toView, const data::Timer* timerToRemove,
                                  bool animateDown) {
    if (mCurrentView == toView) {
        return;
    }

    const bool toTimers = (toView == mTimersView);
    if (toTimers) {
        mTimersView->setVisibility(View::VISIBLE);
    } else {
        mCreateTimerView->setVisibility(View::VISIBLE);
    }
    // Avoid double-taps by enabling/disabling the set of buttons active on the new view.
    updateFab(FabContainer::BUTTONS_DISABLE);

    // Arm the one-shot pre-draw callback (see the header note).
    mAnimToView = toView;
    mAnimTimerToRemove = timerToRemove;
    mAnimAnimateDown = animateDown;
    mPreDrawArmed = true;
    toView->getViewTreeObserver()->addOnPreDrawListener(mPreDrawListener);
}

bool TimerFragment::hasTimers() const {
    return mAdapter != nullptr && mAdapter->getCount() > 0;
}

bool TimerFragment::getCurrentTimer(data::Timer& outTimer) const {
    if (mViewPager == nullptr || mAdapter == nullptr || mAdapter->getCount() == 0) {
        return false;
    }
    outTimer = mAdapter->getTimer(mViewPager->getCurrentItem());
    return true;
}

void TimerFragment::startUpdatingTime() {
    // Ensure only one copy of the runnable is ever scheduled by first stopping updates.
    stopUpdatingTime();
    mViewPager->post(mTimeUpdateRunnable);
}

void TimerFragment::stopUpdatingTime() {
    if (mViewPager != nullptr) mViewPager->removeCallbacks(mTimeUpdateRunnable);
}

Intent TimerFragment::createTimerSetupIntent() {
    Intent intent;
    intent.putExtra(EXTRA_TIMER_SETUP, true);
    return intent;
}

std::vector<int> TimerFragment::computePageIndicatorStates(int page, int pageIndicatorCount,
                                                           int pageCount) {
    // Compute the number of page indicators that will be visible.
    const int rangeSize = std::min(pageIndicatorCount, pageCount);

    // Compute the inclusive range of pages to indicate centered around the selected page.
    int rangeStart = page - rangeSize / 2;
    int rangeEnd = rangeStart + rangeSize - 1;

    // Clamp the range of pages if they extend beyond the last page.
    if (rangeEnd >= pageCount) {
        rangeEnd = pageCount - 1;
        rangeStart = rangeEnd - rangeSize + 1;
    }

    // Clamp the range of pages if they extend beyond the first page.
    if (rangeStart < 0) {
        rangeStart = 0;
        rangeEnd = rangeSize - 1;
    }

    // Build the result with all page indicators initially hidden.
    std::vector<int> states(pageIndicatorCount, 0);

    // If 0 or 1 total pages exist, all page indicators must remain hidden.
    if (rangeSize < 2) {
        return states;
    }

    // Initialize the visible page indicators to be dark.
    for (int i = 0; i < rangeSize; i++) {
        states[i] = R::drawable::ic_swipe_circle_dark;
    }

    // If more pages exist before the first page indicator, make it a fade-in gradient.
    if (rangeStart > 0) {
        states[0] = R::drawable::ic_swipe_circle_top;
    }

    // If more pages exist after the last page indicator, make it a fade-out gradient.
    if (rangeEnd < pageCount - 1) {
        states[rangeSize - 1] = R::drawable::ic_swipe_circle_bottom;
    }

    // Set the indicator of the selected page to be light.
    states[page - rangeStart] = R::drawable::ic_swipe_circle_light;
    return states;
}

} // namespace timer
} // namespace deskclock

// Registered under the bare name the UiDataModel TIMERS tab references.
static const int _cdroid_frag_reg_timer_real =
    (::cdroid::FragmentFactory::registerFragment("TimerFragment",
        []() -> ::cdroid::Fragment* {
            return new ::cdroid::deskclock::timer::TimerFragment();
        }), 0);

} // namespace cdroid
