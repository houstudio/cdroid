#ifndef __DESKCLOCK_EXPIREDTIMERSACTIVITY_H__
#define __DESKCLOCK_EXPIREDTIMERSACTIVITY_H__
/*********************************************************************************
 * Port of com.android.deskclock.timer.ExpiredTimersActivity — the takeover
 * shown when timers expire (upstream: over the lock screen, via the heads-up
 * notification's fullScreenIntent). Displays the expired timers and a single
 * button to reset them all; each expired timer can also be reset to one minute
 * with its "Add 1 Minute" button. All other timer operations are disabled.
 *********************************************************************************/
#include <core/handler.h>
#include <widget/cdwindow.h>

#include <datalisteners.h>
#include <timer.h>

namespace cdroid {
namespace deskclock {
namespace timer {

class ExpiredTimersActivity : public Window {
private:
    /** Scheduled to update the timers while at least one is expired. */
    Runnable mTimeUpdateRunnable;

    /** Updates the timers displayed in this activity as the backing data changes. */
    data::TimerListener mTimerChangeWatcher;

    /** The scene root for transitions when expired timers are added/removed from this container. */
    ViewGroup* mExpiredTimersScrollView = nullptr;

    /** Displays the expired timers. */
    ViewGroup* mExpiredTimersView = nullptr;

public:
    ExpiredTimersActivity();

    void onCreate(Bundle* savedInstanceState) override;
    void onResume() override;
    void onPause() override;
    void onDestroy() override;
    bool dispatchKeyEvent(KeyEvent& event) override;

private:
    /** Post the first runnable to update times within the UI. It will reschedule itself as needed. */
    void startUpdatingTime();

    /** Remove the runnable that updates times within the UI. */
    void stopUpdatingTime();

    /** Create and add a new view that corresponds with the given `timer`. */
    void addTimer(const data::Timer& timer);

    /** Remove an existing view that corresponds with the given `timer`. */
    void removeTimer(const data::Timer& timer);

    /** Center the single timer. */
    void centerFirstTimer();

    /** Display the multiple timers as a scrollable list. */
    void uncenterFirstTimer();

    /** Page-size each list child against the scroll viewport (cdroid-only; see .cc). */
    void sizeListChildren();

    const std::vector<data::Timer>& expiredTimers() const;
};

} // namespace timer
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_EXPIREDTIMERSACTIVITY_H__
