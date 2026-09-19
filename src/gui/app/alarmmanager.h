/*********************************************************************************
 * Copyright (C) 2019 houzh@msn.com
 *
 * C++ port of android.app.AlarmManager.
 * Reference: frameworks/base/apex/jobscheduler/framework/java/android/app/
 * AlarmManager.java (android-12).
 *
 * Upstream delegates to IAlarmManager in system_server, which batches alarms
 * system-wide and delivers them via binder PendingIntents. CDROID is single
 * process, so AlarmManager itself hosts the scheduling: alarms are kept in a
 * priority queue ordered by trigger time (the elapsedRealtime/CLOCK_BOOTTIME
 * domain, as AlarmManagerService does), and a dedicated alarm thread waits on
 * kernel timerfds (inline in alarmmanager.cc -- no porting HAL is added for
 * this: the board fleet has no wake-capable RTC, so a cd*.h interface would
 * be dead surface), and NO dedicated thread: the timerfds are registered on
 * the MAIN LOOPER's epoll via Looper::addFd (the mechanism MessageQueue uses
 * for its own fd events), so the looper itself is the waiter -- fd readiness
 * interrupts epoll_wait at expiry, and re-arming needs no kick. This stands
 * in for system_server's AlarmThread with identical observable semantics.
 * Wakeup-typed alarms try CLOCK_BOOTTIME_ALARM first, so they can break
 * suspend on a kernel with the alarmtimer framework (CONFIG_ALARMTIMER + a
 * wake-capable RTC); on kernels without it they degrade to CLOCK_BOOTTIME,
 * which still counts suspend time, so a missed alarm fires right after the
 * next resume. Non-Linux builds (the win32 backend has no timerfd) compile
 * none of the fd layer: the schedule is carried by a due-scan runnable
 * posted on the main looper (uptime domain -- fires while the process runs,
 * no suspend-time accounting). Delivery is in-process: the OnAlarmListener
 * path posts the callback on the target Handler (ListenerWrapper
 * semantics); the PendingIntent path invokes PendingIntent::send()
 * (in-process adaptation, see pendingintent.h).
 *
 * Supported: set/setRepeating/setWindow/setExact/setInexactRepeating/
 *   setAndAllowWhileIdle/setExactAndAllowWhileIdle/setAlarmClock (both the
 *   PendingIntent and the OnAlarmListener variants), cancel x2, setTime,
 *   setTimeZone, getNextWakeFromIdleTime, canScheduleExactAlarms,
 *   getNextAlarmClock.
 * Not ported: the @hide WorkSource/setPrioritized/setIdleUntil family
 *   (WorkSource and the doze/idle policy machinery do not exist here -- all
 *   ALLOW_WHILE_IDLE flags are accepted and behave like plain alarms);
 *   getNextAlarmClock(userId) (single process, single user); the
 *   ACTION_NEXT_ALARM_CLOCK_CHANGED broadcast (no broadcast dispatch; the
 *   change is observable via getNextAlarmClock()).
 *********************************************************************************/
#ifndef __CDROID_APP_ALARMMANAGER_H__
#define __CDROID_APP_ALARMMANAGER_H__

#include <core/callbackbase.h>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace cdroid{

class Handler;
class PendingIntent;

class AlarmManager{
public:
    /** Alarm types (AlarmManager.java:108-129). */
    static constexpr int RTC_WAKEUP          = 0;
    static constexpr int RTC                 = 1;
    static constexpr int ELAPSED_REALTIME_WAKEUP = 2;
    static constexpr int ELAPSED_REALTIME    = 3;

    /** Broadcast action sent after the next alarm clock changed (@hide side
     *  of getNextAlarmClock(); no broadcast dispatch exists, kept for apps). */
    static constexpr const char* ACTION_NEXT_ALARM_CLOCK_CHANGED =
            "android.app.action.NEXT_ALARM_CLOCK_CHANGED";
    static constexpr const char* ACTION_SCHEDULE_EXACT_ALARM_PERMISSION_STATE_CHANGED =
            "android.app.action.SCHEDULE_EXACT_ALARM_PERMISSION_STATE_CHANGED";

    /** Window lengths (@hide; 0 = exact, -1 = OS free to batch). There is no
     *  system-wide batcher here: windowed alarms fire at window start. */
    static constexpr long WINDOW_EXACT    = 0;
    static constexpr long WINDOW_HEURISTIC = -1;

    /** Alarm flags (@hide); accepted and recorded, only informational without
     *  the doze/batching policy machinery. */
    static constexpr int FLAG_STANDALONE = 1<<0;
    static constexpr int FLAG_WAKE_FROM_IDLE = 1<<1;
    static constexpr int FLAG_ALLOW_WHILE_IDLE = 1<<2;
    static constexpr int FLAG_ALLOW_WHILE_IDLE_UNRESTRICTED = 1<<3;
    static constexpr int FLAG_IDLE_UNTIL = 1<<4;
    static constexpr int FLAG_ALLOW_WHILE_IDLE_COMPAT = 1<<5;
    static constexpr int FLAG_PRIORITIZE = 1<<6;

    /** Recurrence intervals recognized by setInexactRepeating (informational:
     *  since API 19 all repeating alarms are inexact). */
    static constexpr long INTERVAL_FIFTEEN_MINUTES = 15 * 60 * 1000;
    static constexpr long INTERVAL_HALF_HOUR  = 2 * INTERVAL_FIFTEEN_MINUTES;
    static constexpr long INTERVAL_HOUR       = 2 * INTERVAL_HALF_HOUR;
    static constexpr long INTERVAL_HALF_DAY   = 12 * INTERVAL_HOUR;
    static constexpr long INTERVAL_DAY        = 2 * INTERVAL_HALF_DAY;

    /** Direct-notification alarms: the requester must be running from set to
     *  delivery. One-shot only (repeating must use the PendingIntent path,
     *  exactly as upstream). Listener identity is the CallbackBase handle, so
     *  re-setting with the same OnAlarmListener replaces the previous alarm. */
    using OnAlarmListener = CallbackBase<void>;

    /** Immutable description of a scheduled "alarm clock" event
     *  (AlarmManager.java:1389-1472; Parcelable plumbing omitted). */
    class AlarmClockInfo{
    public:
        AlarmClockInfo(int64_t triggerTime, PendingIntent* showIntent);
        int64_t getTriggerTime() const;
        PendingIntent* getShowIntent() const;
    private:
        int64_t mTriggerTime;
        PendingIntent* mShowIntent;
    };

    /** CDROID seam: upstream obtains the instance from
     *  Context.getSystemService(Context.ALARM_SERVICE); CDROID's Context has
     *  no getSystemService, so this follows the InputMethodManager::getInstance
     *  process-wide singleton pattern. */
    static AlarmManager& getInstance();

    /** AlarmManager.java:432 -- inexact one-shot. */
    void set(int type, long triggerAtMillis, PendingIntent* operation);
    /** AlarmManager.java:459 -- direct-callback set(). targetHandler==nullptr
     *  delivers onAlarm() on the main looper. */
    void set(int type, long triggerAtMillis, const std::string& tag,
             const OnAlarmListener& listener, Handler* targetHandler);

    /** AlarmManager.java:524 -- repeating (drift-free: a repeat missed while
     *  suspended is collapsed; the schedule realigns to the original grid). */
    void setRepeating(int type, long triggerAtMillis, long intervalMillis, PendingIntent* operation);

    /** AlarmManager.java:580/603. No batcher exists: windowed alarms are
     *  delivered at windowStartMillis. */
    void setWindow(int type, long windowStartMillis, long windowLengthMillis, PendingIntent* operation);
    void setWindow(int type, long windowStartMillis, long windowLengthMillis,
                   const std::string& tag, const OnAlarmListener& listener, Handler* targetHandler);

    /** AlarmManager.java:704/735. */
    void setExact(int type, long triggerAtMillis, PendingIntent* operation);
    void setExact(int type, long triggerAtMillis, const std::string& tag,
                  const OnAlarmListener& listener, Handler* targetHandler);

    /** AlarmManager.java:1050. */
    void setInexactRepeating(int type, long triggerAtMillis, long intervalMillis, PendingIntent* operation);

    /** AlarmManager.java:1100/1173. Without doze these behave like set/setExact. */
    void setAndAllowWhileIdle(int type, long triggerAtMillis, PendingIntent* operation);
    void setExactAndAllowWhileIdle(int type, long triggerAtMillis, PendingIntent* operation);

    /** AlarmManager.java:807 -- implies RTC_WAKEUP + exact. */
    void setAlarmClock(AlarmClockInfo* info, PendingIntent* operation);

    /** AlarmManager.java:1189 -- remove the alarm matching this PendingIntent. */
    void cancel(PendingIntent* operation);
    /** AlarmManager.java:1212 -- remove the alarm targeting this listener. */
    void cancel(const OnAlarmListener& listener);

    /** AlarmManager.java:1242 -- sets the wall clock (needs root). Pending
     *  RTC-domain alarms are re-derived against the new wall time. */
    void setTime(long millis);

    /** AlarmManager.java:1266 -- sets TZ for the process (tzset); upstream
     *  additionally validates an Olson id via ZoneInfoDb, which is not
     *  ported, so the id is applied as given. */
    void setTimeZone(const std::string& timeZone);

    /** @hide AlarmManager.java:1287 -- earliest wakeup alarm in the elapsed
     *  domain (LLONG_MAX when none); with no idle mode this is simply the
     *  next time a wakeup alarm can fire. */
    int64_t getNextWakeFromIdleTime() const;

    /** No permission subsystem exists: always true. */
    bool canScheduleExactAlarms() const { return true; }

    /** AlarmManager.java:1355 -- next scheduled alarm clock, or nullptr. */
    AlarmClockInfo* getNextAlarmClock();

private:
    AlarmManager();
    ~AlarmManager();
    AlarmManager(const AlarmManager&) = delete;
    AlarmManager& operator=(const AlarmManager&) = delete;

    /** Internal record (one scheduled alarm); owned by the manager. */
    struct Alarm;
    /** Linux: forwards timerfd readiness on the main looper's epoll. */
    struct FdWatcher;

    /** AlarmManager.java:902-952 -- the single funnel every public setter
     *  goes through (negative triggers clamp to 0; same PendingIntent /
     *  listener replaces its previous alarm). */
    void setImpl(int type, long triggerAtMillis, long windowMillis, long intervalMillis,
                 int flags, PendingIntent* operation, const OnAlarmListener& listener,
                 const std::string& listenerTag, Handler* targetHandler,
                 AlarmClockInfo* alarmClock);

    /** RTC-domain trigger -> elapsed domain (AlarmManagerService
     *  convertToElapsed: rtc - (currentTimeMillis - elapsedRealtime)). */
    static int64_t elapsedForTrigger(int type, int64_t triggerAtMillis);

    /** Lazily create the timerfds and register them on the main looper
     *  (Linux); a no-op on platforms without the fd backend. */
    void ensureWatchersLocked();
    /** Collect everything due, re-arm, and deliver. Always runs on the main
     *  thread: the looper's fd callback on Linux, the posted scan runnable
     *  elsewhere. */
    void deliverDueAlarms();
    void rescheduleLocked();
    void removeLocked(PendingIntent* operation, const OnAlarmListener* listener);
    static Handler& mainThreadHandler();

    mutable std::recursive_mutex mLock;
    std::vector<Alarm*> mAlarms;     /* sorted ascending by whenElapsed */
    bool mHasNextAlarmClock;
    AlarmClockInfo* mNextAlarmClock; /* owned scratch copy, or nullptr */

    FdWatcher* mWatcher;             /* owned; null until the first alarm */
    int mWakeupFd;                   /* Linux timerfd backend; -1 elsewhere */
    int mNonWakeupFd;
    bool mWatchersReady;             /* guarded by mLock */
    Runnable mScanRun;               /* non-Linux fallback: due-scan runnable
                                        posted on the main looper (uptime
                                        domain, no suspend accounting) */
};

} // namespace cdroid
#endif/*__CDROID_APP_ALARMMANAGER_H__*/
