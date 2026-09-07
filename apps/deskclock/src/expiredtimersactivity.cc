#include <expiredtimersactivity.h>

#include <R.h>

#include <algorithm>

#include <porting/cdlog.h>

#include <core/activityfactory.h>
#include <core/context.h>
#include <view/keyevent.h>
#include <view/layoutinflater.h>
#include <widget/framelayout.h>
#include <widget/linearlayout.h>
#include <widget/textview.h>

#include <transition/transitionmanager.h>

#include <datamodel.h>
#include <timeritem.h>
#include <utils.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace timer {

using data::DataModel;

ExpiredTimersActivity::ExpiredTimersActivity()
    : Window(0, 0, -1, -1) {
    // TimeUpdateRunnable: periodically refreshes the state of each timer.
    mTimeUpdateRunnable = [this]() {
        const int64_t startTime = Utils::now();

        const int count = mExpiredTimersView->getChildCount();
        for (int i = 0; i < count; i++) {
            TimerItem* timerItem = (TimerItem*) mExpiredTimersView->getChildAt(i);
            data::Timer t;
            if (DataModel::getDataModel().getTimer(timerItem->getId(), t)) {
                timerItem->update(t);
            }
        }

        const int64_t endTime = Utils::now();

        // Try to maintain a consistent period of time between redraws.
        const int64_t delay = std::max((int64_t) 0, startTime + 20 - endTime);
        mExpiredTimersView->postDelayed(mTimeUpdateRunnable, delay);
    };

    // TimerChangeWatcher: adds and removes expired timers based on their state changes.
    mTimerChangeWatcher.timerAdded = [this](const data::Timer& timer) {
        if (timer.isExpired()) {
            addTimer(timer);
        }
    };
    mTimerChangeWatcher.timerUpdated = [this](const data::Timer& before, const data::Timer& after) {
        if (!before.isExpired() && after.isExpired()) {
            addTimer(after);
        } else if (before.isExpired() && !after.isExpired()) {
            removeTimer(before);
        }
    };
    mTimerChangeWatcher.timerRemoved = [this](const data::Timer& timer) {
        if (timer.isExpired()) {
            removeTimer(timer);
        }
    };
}

void ExpiredTimersActivity::onCreate(Bundle* savedInstanceState) {
    Window::onCreate(savedInstanceState);

    // If no expired timers, finish.
    if (expiredTimers().empty()) {
        LOGI("No expired timers, skipping display.");
        close();
        return;
    }

    View* content = LayoutInflater::from(getContext())
            ->inflate(R::layout::expired_timers_activity, nullptr, false);
    // Opaque base layer: without one, SRC_OVER smears whatever was under the window.
    Utils::setDefaultBackground(content);
    addView(content);

    mExpiredTimersView = (ViewGroup*) content->findViewById(R::id::expired_timers_list);
    mExpiredTimersScrollView = (ViewGroup*) content->findViewById(R::id::expired_timers_scroll);

    // Clicking the fab resets all expired timers (FabClickListener).
    content->findViewById(R::id::fab)->setOnClickListener([this](View&) {
        stopUpdatingTime();
        DataModel::getDataModel().removeTimerListener(mTimerChangeWatcher);
        DataModel::getDataModel().resetOrDeleteExpiredTimers(R::string::label_deskclock);
        close();
    });

    // Upstream also: SYSTEM_UI_FLAG_LOW_PROFILE, FLAG_KEEP_SCREEN_ON /
    // FLAG_ALLOW_LOCK_WHILE_SCREEN_ON, setTurnScreenOn + setShowWhenLocked,
    // ACTION_CLOSE_SYSTEM_DIALOGS, and NOSENSOR orientation on phones — no
    // window-manager flag surface on cdroid; the takeover just shows full-screen.

    // Create views for each of the expired timers.
    for (const data::Timer& timer : expiredTimers()) {
        addTimer(timer);
    }

    // Update views in response to timer data changes.
    DataModel::getDataModel().addTimerListener(mTimerChangeWatcher);
}

void ExpiredTimersActivity::onResume() {
    Window::onResume();
    startUpdatingTime();
}

void ExpiredTimersActivity::onPause() {
    Window::onPause();
    stopUpdatingTime();
}

void ExpiredTimersActivity::onDestroy() {
    Window::onDestroy();
    DataModel::getDataModel().removeTimerListener(mTimerChangeWatcher);
}

bool ExpiredTimersActivity::dispatchKeyEvent(KeyEvent& event) {
    if (event.getAction() == KeyEvent::ACTION_UP) {
        switch (event.getKeyCode()) {
            case KeyEvent::KEYCODE_VOLUME_UP:
            case KeyEvent::KEYCODE_VOLUME_DOWN:
            case KeyEvent::KEYCODE_VOLUME_MUTE:
            case KeyEvent::KEYCODE_CAMERA:
            case KeyEvent::KEYCODE_FOCUS:
                DataModel::getDataModel().resetOrDeleteExpiredTimers(
                        R::string::label_hardware_button);
                return true;
        }
    }
    return Window::dispatchKeyEvent(event);
}

void ExpiredTimersActivity::startUpdatingTime() {
    // Ensure only one copy of the runnable is ever scheduled by first stopping updates.
    stopUpdatingTime();
    mExpiredTimersView->post(mTimeUpdateRunnable);
}

void ExpiredTimersActivity::stopUpdatingTime() {
    mExpiredTimersView->removeCallbacks(mTimeUpdateRunnable);
}

void ExpiredTimersActivity::addTimer(const data::Timer& timer) {
    TransitionManager::beginDelayedTransition(mExpiredTimersScrollView);

    const int timerId = timer.id;
    TimerItem* timerItem = (TimerItem*) LayoutInflater::from(getContext())
            ->inflate(R::layout::timer_item, mExpiredTimersView, false);
    // Store the timer id as a tag on the view so it can be located on delete.
    timerItem->setId(timerId);
    mExpiredTimersView->addView(timerItem);

    // Hide the label hint for expired timers.
    TextView* labelView = (TextView*) timerItem->findViewById(R::id::timer_label);
    labelView->setHint(std::string());
    labelView->setVisibility(timer.label.empty() ? View::GONE : View::VISIBLE);

    // Add logic to the "Add 1 Minute" button.
    timerItem->findViewById(R::id::reset_add)->setOnClickListener([timerId](View&) {
        data::Timer t;
        if (DataModel::getDataModel().getTimer(timerId, t)) {
            DataModel::getDataModel().addTimerMinute(t);
        }
    });

    // If the first timer was just added, center it.
    const std::vector<data::Timer>& expired = expiredTimers();
    if (expired.size() == 1) {
        centerFirstTimer();
    } else if (expired.size() == 2) {
        uncenterFirstTimer();
    }

    // Upstream sizes each timer_item through the percent layout against the
    // scroll viewport, so every expired timer renders page-sized (circle capped
    // at max_timer_circle_size, time text auto-fit inside). The cdroid layout
    // has no percent layout port, and the wrap-content list chain collapses the
    // match_parent items to ~content height, clipping the circle; page-size the
    // children explicitly once the viewport is laid out.
    mExpiredTimersView->post([this]() { sizeListChildren(); });
}

void ExpiredTimersActivity::removeTimer(const data::Timer& timer) {
    TransitionManager::beginDelayedTransition(mExpiredTimersScrollView);

    const int timerId = timer.id;
    const int count = mExpiredTimersView->getChildCount();
    for (int i = 0; i < count; i++) {
        View* timerView = mExpiredTimersView->getChildAt(i);
        if (timerView->getId() == timerId) {
            mExpiredTimersView->removeView(timerView);
            break;
        }
    }

    // If the second last timer was just removed, center the last timer.
    const std::vector<data::Timer>& expired = expiredTimers();
    if (expired.empty()) {
        close();
    } else if (expired.size() == 1) {
        centerFirstTimer();
    }
}

void ExpiredTimersActivity::centerFirstTimer() {
    // Upstream mutates the existing FrameLayout.LayoutParams' gravity; cdroid's
    // setLayoutParams takes ownership, so rebuild the params with the same size.
    const ViewGroup::LayoutParams* lp = mExpiredTimersView->getLayoutParams();
    mExpiredTimersView->setLayoutParams(new FrameLayout::LayoutParams(
            lp->width, lp->height, Gravity::CENTER));
    mExpiredTimersView->requestLayout();
}

void ExpiredTimersActivity::uncenterFirstTimer() {
    const ViewGroup::LayoutParams* lp = mExpiredTimersView->getLayoutParams();
    mExpiredTimersView->setLayoutParams(new FrameLayout::LayoutParams(
            lp->width, lp->height, Gravity::NO_GRAVITY));
    mExpiredTimersView->requestLayout();
}

void ExpiredTimersActivity::sizeListChildren() {
    const int viewport = mExpiredTimersScrollView->getHeight();
    if (viewport <= 0) return;

    const int count = mExpiredTimersView->getChildCount();
    for (int i = 0; i < count; i++) {
        View* child = mExpiredTimersView->getChildAt(i);
        ViewGroup::LayoutParams* lp = child->getLayoutParams();
        if (lp->height != viewport) {
            lp->height = viewport;
            child->setLayoutParams(lp);
        }
    }
}

const std::vector<data::Timer>& ExpiredTimersActivity::expiredTimers() const {
    return DataModel::getDataModel().getExpiredTimers();
}

} // namespace timer

// Registered under the bare class name the expiry Intent carries (manifest:
// ".timer.ExpiredTimersActivity", launchMode singleInstance).
static const int _cdroid_act_reg_expired_timers =
    (::cdroid::ActivityFactory::registerActivity("ExpiredTimersActivity",
        []() -> ::cdroid::Window* {
            ::cdroid::Window* w = new ::cdroid::deskclock::timer::ExpiredTimersActivity();
            w->setActivityName("ExpiredTimersActivity");
            return w;
        }), 0);

} // namespace deskclock
} // namespace cdroid
