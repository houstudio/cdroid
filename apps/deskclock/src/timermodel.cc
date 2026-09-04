#include <timermodel.h>

#include <R.h>
#include <algorithm>

#include <content/sharedpreferences.h>
#include <core/context.h>
#include <core/systemclock.h>

#include <settingsdao.h>
#include <timerdao.h>
#include <utils.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace data {

namespace {
// Timers shorter than this threshold are "missed" rather than "expired" after reboot.
constexpr int64_t MISSED_THRESHOLD = -(1000LL * 60 * 10);

struct TimerIdEquals {
    const Timer& timer;
    bool operator()(const Timer& other) const { return other == timer; }
};
} // namespace

TimerModel::TimerModel(Context& context, SharedPreferences& prefs,
                       SettingsModel& settingsModel, NotificationModel& notificationModel)
    : mContext(context), mPrefs(prefs), mSettingsModel(settingsModel),
      mNotificationModel(notificationModel), mHandler(Looper::getMainLooper()) {
    // Upstream: expireTimer(TimerService) via the scheduled intent.
    mExpireRunnable = [this]() {
        mScheduledExpiryTimerId = -1;
        Timer next;
        if (getTimerScheduledForExpiry(next)) {
            expireTimer(next);
        }
    };
}

bool TimerModel::getTimerScheduledForExpiry(Timer& outTimer) const {
    // Locate the next firing timer if one exists (mirror of updateExpirySchedule's pick).
    bool found = false;
    int64_t best = INT64_MAX;
    for (const Timer& timer : mTimers) {
        if (timer.isRunning()) {
            const int64_t expirationTime = timer.getExpirationTime();
            if (!found || expirationTime < best) {
                best = expirationTime;
                outTimer = timer;
                found = true;
            }
        }
    }
    return found;
}

void TimerModel::addTimerListener(const TimerListener& timerListener) {
    mTimerListeners.push_back(timerListener);
}

void TimerModel::removeTimerListener(const TimerListener& timerListener) {
    for (auto it = mTimerListeners.begin(); it != mTimerListeners.end(); ++it) {
        if (*it == timerListener) {
            mTimerListeners.erase(it);
            return;
        }
    }
}

std::vector<Timer>& TimerModel::getTimers() {
    if (!mTimersLoaded) {
        mTimers = TimerDAO::getTimers(mPrefs);
        std::stable_sort(mTimers.begin(), mTimers.end(), Timer::ID_COMPARATOR);
        mTimersLoaded = true;
    }
    return mTimers;
}

const std::vector<Timer>& TimerModel::getExpiredTimers() {
    if (!mExpiredTimersLoaded) {
        mExpiredTimers.clear();
        for (const Timer& timer : getTimers()) {
            if (timer.isExpired()) {
                mExpiredTimers.push_back(timer);
            }
        }
        std::stable_sort(mExpiredTimers.begin(), mExpiredTimers.end(), Timer::EXPIRY_COMPARATOR);
        mExpiredTimersLoaded = true;
    }
    return mExpiredTimers;
}

const std::vector<Timer>& TimerModel::getMissedTimers() {
    if (!mMissedTimersLoaded) {
        mMissedTimers.clear();
        for (const Timer& timer : getTimers()) {
            if (timer.isMissed()) {
                mMissedTimers.push_back(timer);
            }
        }
        std::stable_sort(mMissedTimers.begin(), mMissedTimers.end(), Timer::EXPIRY_COMPARATOR);
        mMissedTimersLoaded = true;
    }
    return mMissedTimers;
}

bool TimerModel::getTimer(int timerId, Timer& outTimer) const {
    for (const Timer& timer : mTimers) {
        if (timer.id == timerId) {
            outTimer = timer;
            return true;
        }
    }
    return false;
}

bool TimerModel::getMostRecentExpiredTimer(Timer& outTimer) {
    const std::vector<Timer>& timers = getExpiredTimers();
    if (timers.empty()) return false;
    outTimer = timers[timers.size() - 1];
    return true;
}

Timer TimerModel::addTimer(int64_t length, const std::string& label, bool deleteAfterUse) {
    // Create the timer instance.
    Timer timer(-1, Timer::State::RESET, length, length, Timer::UNUSED, Timer::UNUSED, length,
                label, deleteAfterUse);

    // Add the timer to permanent storage.
    timer = TimerDAO::addTimer(mPrefs, timer);

    // Add the timer to the cache.
    getTimers().insert(getTimers().begin(), timer);

    // Update the timer notification.
    updateNotification();

    // Notify listeners of the change.
    for (TimerListener& timerListener : mTimerListeners) {
        if (timerListener.timerAdded) timerListener.timerAdded(timer);
    }

    return timer;
}

void TimerModel::expireTimer(const Timer& timer) {
    updateTimer(timer.expire());
}

void TimerModel::updateTimer(const Timer& timer) {
    const Timer before = doUpdateTimer(timer);

    // Update the notification after updating the timer data.
    updateNotification();

    // If the timer started or stopped being expired, update the heads-up notification.
    if (before.state != timer.state) {
        if (before.isExpired() || timer.isExpired()) {
            updateHeadsUpNotification();
        }
    }
}

void TimerModel::removeTimer(const Timer& timer) {
    doRemoveTimer(timer);

    // Update the timer notifications after removing the timer data.
    if (timer.isExpired()) {
        updateHeadsUpNotification();
    } else {
        updateNotification();
    }
}

bool TimerModel::resetTimer(const Timer& timer, bool allowDelete, int eventLabelId,
                            Timer& outTimer) {
    const bool result = doResetOrDeleteTimer(timer, allowDelete, eventLabelId, outTimer);

    // Update the notification after updating the timer data.
    if (timer.isMissed()) {
        updateMissedNotification();
    } else if (timer.isExpired()) {
        updateHeadsUpNotification();
    } else {
        updateNotification();
    }

    return result;
}

void TimerModel::updateTimersAfterReboot() {
    for (const Timer& timer : getTimers()) {
        doUpdateAfterRebootTimer(timer);
    }

    updateNotification();
    updateMissedNotification();
    updateHeadsUpNotification();
}

void TimerModel::updateTimersAfterTimeSet() {
    for (const Timer& timer : getTimers()) {
        doUpdateAfterTimeSetTimer(timer);
    }

    updateNotification();
    updateMissedNotification();
    updateHeadsUpNotification();
}

void TimerModel::resetOrDeleteExpiredTimers(int eventLabelId) {
    for (const Timer& timer : getTimers()) {
        if (timer.isExpired()) {
            Timer out;
            doResetOrDeleteTimer(timer, true /* allowDelete */, eventLabelId, out);
        }
    }

    updateHeadsUpNotification();
}

void TimerModel::resetMissedTimers(int eventLabelId) {
    for (const Timer& timer : getTimers()) {
        if (timer.isMissed()) {
            Timer out;
            doResetOrDeleteTimer(timer, true /* allowDelete */, eventLabelId, out);
        }
    }

    updateMissedNotification();
}

void TimerModel::resetUnexpiredTimers(int eventLabelId) {
    for (const Timer& timer : getTimers()) {
        if (timer.isRunning() || timer.isPaused()) {
            Timer out;
            doResetOrDeleteTimer(timer, true /* allowDelete */, eventLabelId, out);
        }
    }

    updateNotification();
}

Uri* TimerModel::getDefaultTimerRingtoneUri() const {
    return mSettingsModel.getDefaultTimerRingtoneUri();
}

bool TimerModel::isTimerRingtoneSilent() {
    std::unique_ptr<Uri> uri(getTimerRingtoneUri());
    // Uri.EMPTY.equals(timerRingtoneUri)
    return uri->toString().empty();
}

Uri* TimerModel::getTimerRingtoneUri() {
    if (!mTimerRingtoneUriCached) {
        std::unique_ptr<Uri> uri(mSettingsModel.getTimerRingtoneUri());
        mTimerRingtoneUriString = uri->toString();
        mTimerRingtoneUriCached = true;
    }
    return Uri::parse(mTimerRingtoneUriString);
}

void TimerModel::setTimerRingtoneUri(const Uri* uri) {
    mSettingsModel.setTimerRingtoneUri(uri);
    mTimerRingtoneUriCached = false;
    mTimerRingtoneUriString.clear();
}

std::string TimerModel::getTimerRingtoneTitle() {
    if (mTimerRingtoneTitle.empty()) {
        if (isTimerRingtoneSilent()) {
            // Special case: no ringtone has a title of "Silent".
            mTimerRingtoneTitle = mContext.getString(R::string::silent_ringtone_title);
        } else {
            std::unique_ptr<Uri> defaultUri(getDefaultTimerRingtoneUri());
            std::unique_ptr<Uri> uri(getTimerRingtoneUri());
            if (defaultUri->toString() == uri->toString()) {
                // Special case: default ringtone has a title of "Timer Expired".
                mTimerRingtoneTitle = mContext.getString(R::string::default_timer_ringtone_title);
            } else {
                // Upstream queries RingtoneManager for the title on a worker.
                mTimerRingtoneTitle = uri->toString();
            }
        }
    }
    return mTimerRingtoneTitle;
}

int64_t TimerModel::getTimerCrescendoDuration() const {
    return mSettingsModel.getTimerCrescendoDuration();
}

bool TimerModel::getTimerVibrate() const {
    return mSettingsModel.getTimerVibrate();
}

void TimerModel::setTimerVibrate(bool enabled) {
    mSettingsModel.setTimerVibrate(enabled);
}

Timer TimerModel::doUpdateTimer(const Timer& timer) {
    // Retrieve the cached form of the timer.
    std::vector<Timer>& timers = getTimers();
    auto it = std::find_if(timers.begin(), timers.end(), TimerIdEquals{timer});
    const size_t index = it - timers.begin();
    const Timer before = timers[index];

    // If no change occurred, ignore this update.
    if (&timer == &before) {
        return timer;
    }

    // Update the timer in permanent storage.
    TimerDAO::updateTimer(mPrefs, timer);

    // Update the timer in the cache.
    const Timer oldTimer = timers[index] = timer;

    // Clear the cache of expired timers if the timer changed to/from expired.
    if (before.isExpired() || timer.isExpired()) {
        mExpiredTimersLoaded = false;
    }
    // Clear the cache of missed timers if the timer changed to/from missed.
    if (before.isMissed() || timer.isMissed()) {
        mMissedTimersLoaded = false;
    }

    // Update the timer expiration callback.
    updateExpirySchedule();

    // Update the timer ringer.
    updateRinger(&before, &timer);

    // Notify listeners of the change.
    for (TimerListener& timerListener : mTimerListeners) {
        if (timerListener.timerUpdated) timerListener.timerUpdated(before, timer);
    }

    return oldTimer;
}

void TimerModel::doRemoveTimer(const Timer& timer) {
    // Remove the timer from permanent storage.
    TimerDAO::removeTimer(mPrefs, timer);

    // Remove the timer from the cache.
    std::vector<Timer>& timers = getTimers();
    auto it = std::find_if(timers.begin(), timers.end(), TimerIdEquals{timer});

    // If the timer cannot be located there is nothing to remove.
    if (it == timers.end()) {
        return;
    }
    const Timer removed = *it;
    timers.erase(it);

    // Clear the cache of expired timers if a new expired timer was added.
    if (removed.isExpired()) {
        mExpiredTimersLoaded = false;
    }

    // Clear the cache of missed timers if a new missed timer was added.
    if (removed.isMissed()) {
        mMissedTimersLoaded = false;
    }

    // Update the timer expiration callback.
    updateExpirySchedule();

    // Update the timer ringer.
    updateRinger(&removed, nullptr);

    // Notify listeners of the change.
    for (TimerListener& timerListener : mTimerListeners) {
        if (timerListener.timerRemoved) timerListener.timerRemoved(removed);
    }
}

bool TimerModel::doResetOrDeleteTimer(const Timer& timer, bool allowDelete, int eventLabelId,
                                      Timer& outTimer) {
    if (allowDelete && (timer.isExpired() || timer.isMissed()) && timer.deleteAfterUse) {
        doRemoveTimer(timer);
        // Events.sendTimerEvent(R.string.action_delete, eventLabelId) — analytics cut.
        return false;
    } else if (!timer.isReset()) {
        const Timer reset = timer.reset();
        doUpdateTimer(reset);
        outTimer = reset;
        // Events.sendTimerEvent(R.string.action_reset, eventLabelId) — analytics cut.
        return true;
    }
    outTimer = timer;
    return true;
}

void TimerModel::doUpdateAfterRebootTimer(const Timer& timer) {
    Timer updated = timer.updateAfterReboot();
    if (updated.getRemainingTime() < MISSED_THRESHOLD && updated.isRunning()) {
        updated = updated.miss();
    }
    doUpdateTimer(updated);
}

void TimerModel::doUpdateAfterTimeSetTimer(const Timer& timer) {
    doUpdateTimer(timer.updateAfterTimeSet());
}

void TimerModel::updateExpirySchedule() {
    // Locate the next firing timer if one exists.
    Timer next;
    bool found = false;
    int64_t best = INT64_MAX;
    for (const Timer& timer : getTimers()) {
        if (timer.isRunning()) {
            const int64_t expirationTime = timer.getExpirationTime();
            if (!found || expirationTime < best) {
                best = expirationTime;
                next = timer;
                found = true;
            }
        }
    }

    mHandler.removeCallbacks(mExpireRunnable);
    mScheduledExpiryTimerId = -1;

    if (found) {
        // AlarmManager.setExact(ELAPSED_REALTIME_WAKEUP, expirationTime, pi):
        // a main-looper runnable scheduled to the expiration time stands in.
        mScheduledExpiryTimerId = next.id;
        const int64_t delay = std::max((int64_t) 0, best - Utils::now());
        mHandler.postDelayed(mExpireRunnable, delay);
    }
}

void TimerModel::updateRinger(const Timer* before, const Timer* after) {
    // If the timer state did not change, the ringer state is unchanged.
    const bool beforeExpired = before && before->isExpired();
    const bool afterExpired = after && after->isExpired();
    if (beforeExpired == afterExpired) {
        return;
    }

    // If the timer is the first to expire, start ringing.
    if (afterExpired && mRingingIds.insert(after->id).second && mRingingIds.size() == 1) {
        // AlarmAlertWakeLock.acquireScreenCpuWakeLock + TimerKlaxon.start:
        // no wake locks or audio backend on cdroid; the expiry UI drives instead.
    }

    // If the expired timer was the last to reset, stop ringing.
    if (beforeExpired && mRingingIds.erase(before->id) > 0 && mRingingIds.empty()) {
        // TimerKlaxon.stop + AlarmAlertWakeLock.release: stubs.
    }
}

void TimerModel::updateNotification() {
    // No notification surface on cdroid.
}

void TimerModel::updateHeadsUpNotification() {
    // No notification surface on cdroid.
}

void TimerModel::updateMissedNotification() {
    // No notification surface on cdroid.
}

} // namespace data
} // namespace deskclock
} // namespace cdroid
