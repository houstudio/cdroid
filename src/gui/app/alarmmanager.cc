/*********************************************************************************
 * Copyright (C) 2019 houzh@msn.com
 *
 * C++ port of android.app.AlarmManager -- see alarmmanager.h for the
 * in-process architecture notes.
 *********************************************************************************/
#include <app/alarmmanager.h>
#include <app/pendingintent.h>
#include <core/handler.h>
#include <core/looper.h>
#include <core/systemclock.h>
#include <porting/cdlog.h>

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <ctime>
#ifdef __linux__
#include <sys/timerfd.h>
#include <unistd.h>
#endif

namespace cdroid{

#ifdef __linux__
// ============================================================================
// Inline timerfd layer (no porting HAL and no dedicated thread -- see the
// header notes). Clock selection per fd, probed once at create:
//   wakeup kind:     CLOCK_BOOTTIME_ALARM -> CLOCK_BOOTTIME (-> MONOTONIC)
//   non-wakeup kind: CLOCK_BOOTTIME                      (-> MONOTONIC)
// A kernel with the alarmtimer framework (CONFIG_ALARMTIMER + wake RTC) arms
// CLOCK_BOOTTIME_ALARM timers into the RTC before suspend, so wakeup alarms
// can break suspend; on kernels without it they degrade to CLOCK_BOOTTIME,
// which still counts suspend time, so missed alarms fire right after resume.
//
// The fds are registered on the MAIN LOOPER's epoll (Looper::addFd), the same
// mechanism MessageQueue uses for OnFileDescriptorEventListener: the kernel
// CLOCK_BOOTTIME accounting keeps running while suspended, and once the fd is
// readable (at expiry, or immediately on resume after a missed deadline) the
// looper wakes by itself. epoll_wait's timeout is only an upper bound -- fd
// readiness interrupts it any time -- so re-arming an earlier deadline needs
// no kick either. This replaces system_server's dedicated AlarmThread with
// the looper as the waiter, with identical observable semantics.
// ============================================================================

namespace {

struct FdClockEntry {
    int fd;
    clockid_t clock;
};
/* The manager owns at most two timer fds; a tiny linear table is plenty. */
constexpr int MAX_ALARM_FDS = 4;
FdClockEntry gFdClocks[MAX_ALARM_FDS];

void registerFdClock(int fd, clockid_t clock) {
    for (int i = 0; i < MAX_ALARM_FDS; i++) {
        if (gFdClocks[i].fd == 0) {
            gFdClocks[i].fd = fd;
            gFdClocks[i].clock = clock;
            return;
        }
    }
    LOGE("AlarmManager: fd clock table full");
}

clockid_t clockForFd(int fd) {
    for (int i = 0; i < MAX_ALARM_FDS; i++) {
        if (gFdClocks[i].fd == fd) return gFdClocks[i].clock;
    }
    return CLOCK_MONOTONIC;
}

int64_t clockMillis(clockid_t clock) {
    struct timespec ts;
    if (clock_gettime(clock, &ts) != 0) return -1;
    return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

bool timerfdUsable(clockid_t clock) {
    int fd = timerfd_create(clock, TFD_NONBLOCK | TFD_CLOEXEC);
    if (fd < 0) return false;
    close(fd);
    return true;
}

bool wakesFromSuspend() {
    static int cached = -1;
    if (cached < 0) cached = timerfdUsable(CLOCK_BOOTTIME_ALARM) ? 1 : 0;
    return cached != 0;
}

int createAlarmFd(bool wakeup) {
    clockid_t clocks[3];
    int count = 0;
    if (wakeup) clocks[count++] = CLOCK_BOOTTIME_ALARM;
    clocks[count++] = CLOCK_BOOTTIME;
    clocks[count++] = CLOCK_MONOTONIC;

    for (int i = 0; i < count; i++) {
        bool dup = false;
        for (int j = 0; j < i; j++) dup |= (clocks[j] == clocks[i]);
        if (dup) continue;

        int fd = timerfd_create(clocks[i], TFD_NONBLOCK | TFD_CLOEXEC);
        if (fd >= 0) {
            registerFdClock(fd, clocks[i]);
            LOGI("AlarmManager: fd=%d on clock %d (wakeup=%d, wakes-from-suspend=%d)",
                 fd, (int)clocks[i], (int)wakeup,
                 clocks[i] == CLOCK_BOOTTIME_ALARM ? 1 : 0);
            return fd;
        }
    }
    LOGE("AlarmManager: no usable timer clock (timerfd unavailable)");
    return -1;
}

/* Absolute CLOCK_BOOTTIME deadline -> the fd's own clock domain (the offset
 * is re-measured per arm: BOOTTIME keeps counting in suspend, MONOTONIC does
 * not, so it only moves across suspends). */
int armAlarmFd(int fd, int64_t abstimeBoottimeMillis, int64_t intervalMillis) {
    if (fd < 0 || abstimeBoottimeMillis < 0) return -1;
    const int64_t boottime = clockMillis(CLOCK_BOOTTIME);
    const int64_t own = clockMillis(clockForFd(fd));
    if (boottime < 0 || own < 0) return -1;

    const int64_t absolute = abstimeBoottimeMillis + (boottime - own);
    /* timerfd treats it_value == {0,0} as DISARM even with TFD_TIMER_ABSTIME,
     * so a deadline at (or before) the fd clock's epoch must clamp to a
     * positive past instant -- which expires immediately, matching AOSP's
     * "a trigger time in the past fires the alarm right away". */
    const int64_t clamped = (absolute > 0) ? absolute : 1;
    struct itimerspec its;
    memset(&its, 0, sizeof(its));
    its.it_value.tv_sec = clamped / 1000;
    its.it_value.tv_nsec = (clamped % 1000) * 1000000;
    if (intervalMillis > 0) {
        its.it_interval.tv_sec = intervalMillis / 1000;
        its.it_interval.tv_nsec = (intervalMillis % 1000) * 1000000;
    }
    return timerfd_settime(fd, TFD_TIMER_ABSTIME, &its, nullptr) == 0 ? 0 : -1;
}

int disarmAlarmFd(int fd) {
    if (fd < 0) return -1;
    struct itimerspec its;
    memset(&its, 0, sizeof(its));
    return timerfd_settime(fd, 0, &its, nullptr) == 0 ? 0 : -1;
}

int drainAlarmFd(int fd) {
    if (fd < 0) return -1;
    uint64_t expirations;
    ssize_t n = read(fd, &expirations, sizeof(expirations));
    if (n == (ssize_t)sizeof(expirations)) return 1;
    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return 0;
    return -1;
}

} // namespace
#endif /* __linux__ */

/** One scheduled alarm; owned by AlarmManager and only ever touched under
 *  mLock (delivery works on copies, see deliverDueAlarms). */
struct AlarmManager::Alarm {
    int type;
    long triggerAtMillis;      /* caller's value, in the type's clock domain */
    int64_t whenElapsed;       /* absolute deadline, elapsedRealtime domain */
    long windowLengthMillis;   /* WINDOW_EXACT / WINDOW_HEURISTIC (recorded) */
    long intervalMillis;       /* 0 = one-shot */
    int flags;
    std::string tag;
    PendingIntent* operation;  /* borrowed: PendingIntent-delivered alarm */
    OnAlarmListener listener;  /* listener-delivered alarm (identity = handle) */
    Handler* targetHandler;    /* borrowed; nullptr = main looper */
    bool isAlarmClock;         /* scheduled via setAlarmClock() */
    PendingIntent* alarmClockShow; /* setAlarmClock's show intent (borrowed) */
};

/* Everything a delivery needs, copied under the lock so a concurrent cancel()
 * can never yank a record out from under a pending delivery. */
struct AlarmDeliveryJob {
    AlarmManager::OnAlarmListener listener;   /* value copy: safe post-mortem */
    PendingIntent* operation;                 /* borrowed per caller contract */
    Handler* handler;                         /* borrowed per caller contract */
};

/* AOSP keeps mAlwaysExact = (targetSdkVersion < KITKAT); CDROID apps are
 * always current, so legacyExactLength() is permanently WINDOW_HEURISTIC. */
static long legacyExactLength() {
    return AlarmManager::WINDOW_HEURISTIC;
}

#ifdef __linux__
/* Forwards timerfd readiness on the main looper's epoll to the manager, the
 * role AlarmManagerService's AlarmThread plays upstream (libutils
 * convention: handleEvent returns 0 to unregister, nonzero to stay). */
struct AlarmManager::FdWatcher : public LooperCallback {
    AlarmManager& mOwner;
    explicit FdWatcher(AlarmManager& owner) : mOwner(owner) {}
    int handleEvent(int fd, int events, void* /*data*/) override {
        if (events & Looper::EVENT_INPUT) {
            drainAlarmFd(fd);
            mOwner.deliverDueAlarms();
        }
        return 1;  // keep the fd registered
    }
};
#endif

AlarmManager& AlarmManager::getInstance() {
    static AlarmManager sInstance;
    return sInstance;
}

AlarmManager::AlarmManager()
    : mHasNextAlarmClock(false)
    , mNextAlarmClock(nullptr)
    , mWatcher(nullptr)
    , mWakeupFd(-1)
    , mNonWakeupFd(-1)
    , mWatchersReady(false) {
    /* Non-Linux fallback carrier: a due-scan runnable posted on the main
     * looper (uptime domain; no suspend-time accounting on this platform). */
    mScanRun = [this]() { deliverDueAlarms(); };
}

AlarmManager::~AlarmManager() {
    {
        std::lock_guard<std::recursive_mutex> lock(mLock);
#ifdef __linux__
        Looper* looper = Looper::getMainLooper();
        if (looper) {
            if (mWakeupFd >= 0) looper->removeFd(mWakeupFd);
            if (mNonWakeupFd >= 0) looper->removeFd(mNonWakeupFd);
        }
        delete mWatcher;   /* complete type only under __linux__ */
        mWatcher = nullptr;
#else
        mainThreadHandler().removeCallbacks(mScanRun);
#endif
        for (Alarm* a : mAlarms) delete a;
        mAlarms.clear();
        delete mNextAlarmClock;
        mNextAlarmClock = nullptr;
    }
}

Handler& AlarmManager::mainThreadHandler() {
    /* AOSP field: mMainThreadHandler = new Handler(ctx.getMainLooper()) */
    static Handler sMainThreadHandler(Looper::getMainLooper());
    return sMainThreadHandler;
}

// ============================================================================
// AlarmClockInfo (AlarmManager.java:1389-1472)
// ============================================================================

AlarmManager::AlarmClockInfo::AlarmClockInfo(int64_t triggerTime, PendingIntent* showIntent)
    : mTriggerTime(triggerTime)
    , mShowIntent(showIntent) {
}

int64_t AlarmManager::AlarmClockInfo::getTriggerTime() const {
    return mTriggerTime;
}

PendingIntent* AlarmManager::AlarmClockInfo::getShowIntent() const {
    return mShowIntent;
}

// ============================================================================
// Public setters (each mirrors its AlarmManager.java counterpart's call into
// setImpl; only the javadoc'd lines differ).
// ============================================================================

void AlarmManager::set(int type, long triggerAtMillis, PendingIntent* operation) {
    setImpl(type, triggerAtMillis, legacyExactLength(), 0, 0, operation,
            nullptr, std::string(), nullptr, nullptr);
}

void AlarmManager::set(int type, long triggerAtMillis, const std::string& tag,
                       const OnAlarmListener& listener, Handler* targetHandler) {
    setImpl(type, triggerAtMillis, legacyExactLength(), 0, 0, nullptr,
            listener, tag, targetHandler, nullptr);
}

void AlarmManager::setRepeating(int type, long triggerAtMillis,
                                long intervalMillis, PendingIntent* operation) {
    setImpl(type, triggerAtMillis, legacyExactLength(), intervalMillis, 0, operation,
            nullptr, std::string(), nullptr, nullptr);
}

void AlarmManager::setWindow(int type, long windowStartMillis, long windowLengthMillis,
                             PendingIntent* operation) {
    setImpl(type, windowStartMillis, windowLengthMillis, 0, 0, operation,
            nullptr, std::string(), nullptr, nullptr);
}

void AlarmManager::setWindow(int type, long windowStartMillis, long windowLengthMillis,
                             const std::string& tag, const OnAlarmListener& listener,
                             Handler* targetHandler) {
    setImpl(type, windowStartMillis, windowLengthMillis, 0, 0, nullptr,
            listener, tag, targetHandler, nullptr);
}

void AlarmManager::setExact(int type, long triggerAtMillis, PendingIntent* operation) {
    setImpl(type, triggerAtMillis, WINDOW_EXACT, 0, 0, operation,
            nullptr, std::string(), nullptr, nullptr);
}

void AlarmManager::setExact(int type, long triggerAtMillis, const std::string& tag,
                            const OnAlarmListener& listener, Handler* targetHandler) {
    setImpl(type, triggerAtMillis, WINDOW_EXACT, 0, 0, nullptr,
            listener, tag, targetHandler, nullptr);
}

void AlarmManager::setInexactRepeating(int type, long triggerAtMillis,
                                       long intervalMillis, PendingIntent* operation) {
    setImpl(type, triggerAtMillis, WINDOW_HEURISTIC, intervalMillis, 0, operation,
            nullptr, std::string(), nullptr, nullptr);
}

void AlarmManager::setAndAllowWhileIdle(int type, long triggerAtMillis, PendingIntent* operation) {
    setImpl(type, triggerAtMillis, WINDOW_HEURISTIC, 0, FLAG_ALLOW_WHILE_IDLE, operation,
            nullptr, std::string(), nullptr, nullptr);
}

void AlarmManager::setExactAndAllowWhileIdle(int type, long triggerAtMillis, PendingIntent* operation) {
    setImpl(type, triggerAtMillis, WINDOW_EXACT, 0, FLAG_ALLOW_WHILE_IDLE, operation,
            nullptr, std::string(), nullptr, nullptr);
}

void AlarmManager::setAlarmClock(AlarmClockInfo* info, PendingIntent* operation) {
    setImpl(RTC_WAKEUP, info->getTriggerTime(), WINDOW_EXACT, 0, 0, operation,
            nullptr, std::string(), nullptr, info);
}

// ============================================================================
// setImpl (AlarmManager.java:902-952)
// ============================================================================

int64_t AlarmManager::elapsedForTrigger(int type, int64_t triggerAtMillis) {
    if (type == RTC_WAKEUP || type == RTC) {
        /* AlarmManagerService.convertToElapsed:
         * rtc - (currentTimeMillis - elapsedRealtime) */
        return triggerAtMillis
                - (SystemClock::currentTimeMillis() - SystemClock::elapsedRealtime());
    }
    return triggerAtMillis;  /* ELAPSED_* are already in this domain */
}

void AlarmManager::setImpl(int type, long triggerAtMillis, long windowMillis,
                           long intervalMillis, int flags, PendingIntent* operation,
                           const OnAlarmListener& listener, const std::string& listenerTag,
                           Handler* targetHandler, AlarmClockInfo* alarmClock) {
    if (triggerAtMillis < 0) {
        triggerAtMillis = 0;  // AlarmManager.java:923
    }
    if (type != RTC_WAKEUP && type != RTC
            && type != ELAPSED_REALTIME_WAKEUP && type != ELAPSED_REALTIME) {
        throw std::invalid_argument("bad alarm type " + std::to_string(type));
    }

    /* ListenerWrapper bookkeeping collapses to the CallbackBase identity:
     * a listener (or PendingIntent) can back exactly one alarm; re-setting
     * it replaces the previous one (AlarmManager.java:926-944 +
     * AlarmManagerService.set removal). */
    const bool hasListener = (listener != nullptr);

    Alarm* alarm = new Alarm();
    alarm->type = type;
    alarm->triggerAtMillis = triggerAtMillis;
    alarm->whenElapsed = elapsedForTrigger(type, triggerAtMillis);
    alarm->windowLengthMillis = windowMillis;
    alarm->intervalMillis = intervalMillis;
    alarm->flags = flags;
    alarm->tag = listenerTag;
    alarm->operation = operation;
    alarm->listener = hasListener ? listener : OnAlarmListener();
    alarm->targetHandler = targetHandler;
    alarm->isAlarmClock = (alarmClock != nullptr);
    alarm->alarmClockShow = alarmClock ? alarmClock->getShowIntent() : nullptr;

    {
        std::lock_guard<std::recursive_mutex> lock(mLock);
        ensureWatchersLocked();
        removeLocked(operation, hasListener ? &listener : nullptr);
        auto it = std::lower_bound(mAlarms.begin(), mAlarms.end(), alarm->whenElapsed,
                [](const Alarm* a, int64_t when) { return a->whenElapsed < when; });
        mAlarms.insert(it, alarm);
        rescheduleLocked();
    }
}

void AlarmManager::cancel(PendingIntent* operation) {
    if (operation == nullptr) {
        /* AOSP throws NPE for targetSdk >= N */
        throw std::invalid_argument("cancel() called with a null PendingIntent");
    }
    std::lock_guard<std::recursive_mutex> lock(mLock);
    removeLocked(operation, nullptr);
    rescheduleLocked();
}

void AlarmManager::cancel(const OnAlarmListener& listener) {
    if (listener == nullptr) {
        throw std::invalid_argument("cancel() called with a null OnAlarmListener");
    }
    std::lock_guard<std::recursive_mutex> lock(mLock);
    removeLocked(nullptr, &listener);
    rescheduleLocked();
}

void AlarmManager::removeLocked(PendingIntent* operation, const OnAlarmListener* listener) {
    for (size_t i = 0; i < mAlarms.size();) {
        Alarm* a = mAlarms[i];
        const bool matchesOperation = operation && a->operation == operation;
        const bool matchesListener = listener && (*listener != nullptr) && a->listener == *listener;
        if (matchesOperation || matchesListener) {
            mAlarms.erase(mAlarms.begin() + i);
            delete a;
        } else {
            i++;
        }
    }
}

// ============================================================================
// Wall clock / time zone (AlarmManager.java:1242-1284)
// ============================================================================

void AlarmManager::setTime(long millis) {
    if (!SystemClock::setCurrentTimeMillis(millis)) {
        LOGE("AlarmManager::setTime: setCurrentTimeMillis failed (%s) -- needs root",
             strerror(errno));
    }

    /* RTC-domain deadlines were derived from the old wall time; re-derive
     * them, as AlarmManagerService does on TIME_CHANGED. */
    std::lock_guard<std::recursive_mutex> lock(mLock);
    for (Alarm* a : mAlarms) {
        a->whenElapsed = elapsedForTrigger(a->type, a->triggerAtMillis);
    }
    std::stable_sort(mAlarms.begin(), mAlarms.end(),
            [](const Alarm* l, const Alarm* r) { return l->whenElapsed < r->whenElapsed; });
    rescheduleLocked();
}

void AlarmManager::setTimeZone(const std::string& timeZone) {
    if (timeZone.empty()) {
        return;  // TextUtils.isEmpty branch (AlarmManager.java:1267)
    }
#if defined(_WIN32) || defined(_WIN64)
    _putenv_s("TZ", timeZone.c_str());
    _tzset();
#else
    setenv("TZ", timeZone.c_str(), 1);
    tzset();
#endif
}

// ============================================================================
// Queries
// ============================================================================

int64_t AlarmManager::getNextWakeFromIdleTime() const {
    std::lock_guard<std::recursive_mutex> lock(mLock);
    int64_t next = LLONG_MAX;
    for (const Alarm* a : mAlarms) {
        if ((a->type == RTC_WAKEUP || a->type == ELAPSED_REALTIME_WAKEUP)
                && a->whenElapsed < next) {
            next = a->whenElapsed;
        }
    }
    return next;
}

AlarmManager::AlarmClockInfo* AlarmManager::getNextAlarmClock() {
    std::lock_guard<std::recursive_mutex> lock(mLock);
    return mHasNextAlarmClock ? mNextAlarmClock : nullptr;
}

// ============================================================================
// Scheduling core (the AlarmManagerService stand-in). Runs entirely on the
// main thread: the looper's epoll is the waiter.
// ============================================================================

void AlarmManager::ensureWatchersLocked() {
    if (mWatchersReady) return;
    mWatchersReady = true;

#ifdef __linux__
    mWakeupFd = createAlarmFd(true);
    mNonWakeupFd = createAlarmFd(false);
    if (mWakeupFd < 0 && mNonWakeupFd < 0) {
        LOGE("AlarmManager: no timer backend at all; alarms will never fire");
        return;
    }
    if (!wakesFromSuspend()) {
        LOGW("AlarmManager: kernel alarmtimer unavailable (CONFIG_ALARMTIMER); "
             "wakeup alarms degrade to fire-after-resume");
    }
    Looper* looper = Looper::getMainLooper();
    if (looper == nullptr) {
        LOGE("AlarmManager: no main looper; alarms will never fire");
        return;
    }
    mWatcher = new FdWatcher(*this);
    if (mWakeupFd >= 0) {
        int rc = looper->addFd(mWakeupFd, Looper::POLL_CALLBACK, Looper::EVENT_INPUT, mWatcher, nullptr);
        LOGI("AlarmManager: addFd(wakeup fd=%d) rc=%d", mWakeupFd, rc);
    }
    if (mNonWakeupFd >= 0) {
        int rc = looper->addFd(mNonWakeupFd, Looper::POLL_CALLBACK, Looper::EVENT_INPUT, mWatcher, nullptr);
        LOGI("AlarmManager: addFd(non-wakeup fd=%d) rc=%d", mNonWakeupFd, rc);
    }
#else
    LOGI("AlarmManager: portable postAtTime backend (no timerfd on this "
         "platform); alarms fire while the process runs, no suspend-time "
         "accounting");
#endif
}

void AlarmManager::rescheduleLocked() {
    bool hasAlarmClock = false;
    int64_t clockTrigger = 0;
    PendingIntent* clockShow = nullptr;
    int64_t earliest = LLONG_MAX;

#ifdef __linux__
    /* Earliest deadline per wakeup class -> its own timerfd
     * (AlarmManagerService arms CLOCK_BOOTTIME_ALARM for wakeup alarms,
     * CLOCK_BOOTTIME for the rest). */
    int64_t nextWakeup = LLONG_MAX;
    int64_t nextNonWakeup = LLONG_MAX;
#endif

    for (const Alarm* a : mAlarms) {
#ifdef __linux__
        const bool wakeup = (a->type == RTC_WAKEUP || a->type == ELAPSED_REALTIME_WAKEUP);
        if (wakeup) {
            if (a->whenElapsed < nextWakeup) nextWakeup = a->whenElapsed;
        } else if (a->whenElapsed < nextNonWakeup) {
            nextNonWakeup = a->whenElapsed;
        }
#endif
        if (a->whenElapsed < earliest) earliest = a->whenElapsed;
        if (a->isAlarmClock && (!hasAlarmClock || a->triggerAtMillis < clockTrigger)) {
            hasAlarmClock = true;
            clockTrigger = a->triggerAtMillis;
            clockShow = a->alarmClockShow;
        }
    }

#ifdef __linux__
    /* Arm (degrade to the other fd when one clock is absent, so alarms still
     * fire, just never out of suspend; two arms on a shared fd keep the
     * earlier deadline only if it is armed last -- acceptable degraded path).
     * No kick: an armed timerfd wakes the looper's epoll by itself at
     * expiry, and re-arming only moves the deadline. */
    const int wakeupFd = (mWakeupFd >= 0) ? mWakeupFd : mNonWakeupFd;
    const int nonWakeupFd = (mNonWakeupFd >= 0) ? mNonWakeupFd : mWakeupFd;
    if (nextWakeup != LLONG_MAX) {
        if (wakeupFd >= 0) armAlarmFd(wakeupFd, nextWakeup, 0);
    } else if (wakeupFd >= 0) {
        disarmAlarmFd(wakeupFd);
    }
    if (nextNonWakeup != LLONG_MAX) {
        if (nonWakeupFd >= 0) armAlarmFd(nonWakeupFd, nextNonWakeup, 0);
    } else if (nonWakeupFd >= 0) {
        disarmAlarmFd(nonWakeupFd);
    }
#else
    /* Portable fallback carrier: (re)post the due-scan runnable at the
     * earliest deadline, converted to the looper's uptime domain. */
    mainThreadHandler().removeCallbacks(mScanRun);
    if (earliest != LLONG_MAX) {
        int64_t when = earliest
                - (SystemClock::elapsedRealtime() - SystemClock::uptimeMillis());
        if (when < 0) when = 0;
        mainThreadHandler().postAtTime(mScanRun, (void*)this, when);
    }
#endif

    /* Next-alarm-clock cache (upstream: per-user earliest setAlarmClock). */
    if (hasAlarmClock) {
        if (mNextAlarmClock == nullptr) mNextAlarmClock = new AlarmClockInfo(clockTrigger, clockShow);
        else *mNextAlarmClock = AlarmClockInfo(clockTrigger, clockShow);
    } else {
        delete mNextAlarmClock;
        mNextAlarmClock = nullptr;
    }
    mHasNextAlarmClock = hasAlarmClock;
}

void AlarmManager::deliverDueAlarms() {
    /* Collect everything due under the lock, re-arm, then deliver on copies
     * (a cancel() racing on another thread must never yank a record out from
     * under a pending delivery). Runs on the main thread: the looper's fd
     * callback on Linux, the posted scan runnable elsewhere. */
    std::vector<AlarmDeliveryJob> jobs;
    {
        std::lock_guard<std::recursive_mutex> lock(mLock);
        const int64_t now = SystemClock::elapsedRealtime();
        for (size_t i = 0; i < mAlarms.size();) {
            Alarm* a = mAlarms[i];
            if (a->whenElapsed > now) { i++; continue; }

            AlarmDeliveryJob job;
            job.listener = a->listener;
            job.operation = a->operation;
            job.handler = a->targetHandler;
            jobs.push_back(job);

            if (a->intervalMillis > 0) {
                /* Drift-free repeat grid: misses collapse into the next
                 * future slot (docs of setRepeating), then reschedule. */
                do { a->whenElapsed += a->intervalMillis; }
                while (a->whenElapsed <= now);
                mAlarms.erase(mAlarms.begin() + i);
                auto it = std::lower_bound(mAlarms.begin(), mAlarms.end(), a->whenElapsed,
                        [](const Alarm* x, int64_t when) { return x->whenElapsed < when; });
                mAlarms.insert(it, a);
            } else {
                mAlarms.erase(mAlarms.begin() + i);
                delete a;
            }
        }
        rescheduleLocked();
    }

    for (const AlarmDeliveryJob& job : jobs) {
        if (job.operation) {
            job.operation->send();
        } else if (job.listener) {
            Handler* handler = job.handler ? job.handler : &mainThreadHandler();
            /* ListenerWrapper.doAlarm -> executor.execute(this): the
             * callback runs on the target handler's looper (posted even when
             * that is the current thread, matching HandlerExecutor). */
            handler->post(job.listener);
        }
    }
}

} // namespace cdroid
