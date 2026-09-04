#ifndef __DESKCLOCK_TIMERMODEL_H__
#define __DESKCLOCK_TIMERMODEL_H__
/*********************************************************************************
 * Port of com.android.deskclock.data.TimerModel — timer CRUD + listener
 * notification + expiry scheduling.
 *
 * CDROID facades: AlarmManager/PendingIntent expiry → a main-looper Handler
 * runnable posted to the next expiration time (TimerService's timer-expired
 * intent is dropped); TimerKlaxon/AlarmAlertWakeLock are stubs until the audio
 * backend lands; the three notification updates are no-ops (notifications cut).
 *********************************************************************************/
#include <set>
#include <string>
#include <vector>

#include <core/handler.h>
#include <core/uri.h>

#include <datalisteners.h>
#include <timer.h>

namespace cdroid {

class Context;
class SharedPreferences;

namespace deskclock {
namespace data {

class NotificationModel;
class SettingsModel;

class TimerModel {
private:
    Context& mContext;
    SharedPreferences& mPrefs;
    SettingsModel& mSettingsModel;
    NotificationModel& mNotificationModel;

    std::vector<TimerListener> mTimerListeners;

    /** Ids of timers currently ringing (Kotlin mRingingIds). */
    std::set<int> mRingingIds;

    /** Cached timer ringtone uri string (empty = uncached). */
    std::string mTimerRingtoneUriString;
    bool mTimerRingtoneUriCached = false;

    std::string mTimerRingtoneTitle;

    std::vector<Timer> mTimers;
    bool mTimersLoaded = false;
    std::vector<Timer> mExpiredTimers;
    bool mExpiredTimersLoaded = false;
    std::vector<Timer> mMissedTimers;
    bool mMissedTimersLoaded = false;

    /** Main-looper schedule of the next timer expiry (replaces AlarmManager). */
    Handler mHandler;
    Runnable mExpireRunnable;
    int mScheduledExpiryTimerId = -1;

public:
    TimerModel(Context& context, SharedPreferences& prefs, SettingsModel& settingsModel,
               NotificationModel& notificationModel);

    void addTimerListener(const TimerListener& timerListener);
    void removeTimerListener(const TimerListener& timerListener);

    std::vector<Timer>& getTimers();
    const std::vector<Timer>& getExpiredTimers();
    const std::vector<Timer>& getMissedTimers();

    bool getTimer(int timerId, Timer& outTimer) const;

    /** @return the most recently expired timer; false if none. */
    bool getMostRecentExpiredTimer(Timer& outTimer);

    Timer addTimer(int64_t length, const std::string& label, bool deleteAfterUse);

    void expireTimer(const Timer& timer);
    void updateTimer(const Timer& timer);
    void removeTimer(const Timer& timer);
    /** @return false if the timer was deleted; the reset timer otherwise. */
    bool resetTimer(const Timer& timer, bool allowDelete, int eventLabelId, Timer& outTimer);

    void updateTimersAfterReboot();
    void updateTimersAfterTimeSet();

    void resetOrDeleteExpiredTimers(int eventLabelId);
    void resetMissedTimers(int eventLabelId);
    void resetUnexpiredTimers(int eventLabelId);

    Uri* getDefaultTimerRingtoneUri() const;
    bool isTimerRingtoneSilent();
    Uri* getTimerRingtoneUri();
    void setTimerRingtoneUri(const Uri* uri);
    std::string getTimerRingtoneTitle();

    int64_t getTimerCrescendoDuration() const;
    bool getTimerVibrate() const;
    void setTimerVibrate(bool enabled);

private:
    /** @return the timer the expiry runnable should expire (next running). */
    bool getTimerScheduledForExpiry(Timer& outTimer) const;

    Timer doUpdateTimer(const Timer& timer);
    void doRemoveTimer(const Timer& timer);
    bool doResetOrDeleteTimer(const Timer& timer, bool allowDelete, int eventLabelId,
                              Timer& outTimer);
    void doUpdateAfterRebootTimer(const Timer& timer);
    void doUpdateAfterTimeSetTimer(const Timer& timer);

    /** (Upstream updateAlarmManager: schedule/cancel the next expiry callback.) */
    void updateExpirySchedule();

    /** Start/stop the ringer as timers transition to/from expired. */
    void updateRinger(const Timer* before, const Timer* after);

    // Notification updates are no-ops on cdroid (no notification surface).
    void updateNotification();
    void updateHeadsUpNotification();
    void updateMissedNotification();
};

} // namespace data
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_TIMERMODEL_H__
