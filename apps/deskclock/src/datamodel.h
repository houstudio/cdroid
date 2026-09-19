#ifndef __DESKCLOCK_DATAMODEL_H__
#define __DESKCLOCK_DATAMODEL_H__
/*********************************************************************************
 * Port of com.android.deskclock.data.DataModel — the application-wide data
 * facade. Cut from upstream: WidgetModel (app widgets), AlarmModel notification
 * bookkeeping, RingtoneModel title lookups (uri string stands in) and the
 * background-thread ringtone loading; SilentSettings polling never fires (no
 * system audio/notification state on cdroid).
 *********************************************************************************/
#include <memory>
#include <string>
#include <vector>

#include <core/calendar.h>
#include <core/handler.h>

#include <city.h>
#include <citymodel.h>
#include <datalisteners.h>
#include <notificationmodel.h>
#include <ringtonemodel.h>
#include <settingsdao.h>
#include <stopwatchmodel.h>
#include <timermodel.h>

namespace cdroid {

class Context;
class SharedPreferences;

namespace deskclock {
namespace data {

class DataModel {
private:
    Context* mContext = nullptr;

    SettingsModel* mSettingsModel = nullptr;
    CityModel* mCityModel = nullptr;
    TimerModel* mTimerModel = nullptr;
    StopwatchModel* mStopwatchModel = nullptr;
    NotificationModel* mNotificationModel = nullptr;
    RingtoneModel* mRingtoneModel = nullptr;

    std::vector<OnSilentSettingsListener> mSilentSettingsListeners;

    std::unique_ptr<Handler> mHandler;

    DataModel() = default;

public:
    static DataModel& getDataModel();

    void init(Context& context, SharedPreferences& prefs);

    /** Executes a runnable on the main thread. */
    void run(const Runnable& runnable);

    /** Executes a runnable on the main thread after the given delay. */
    void run(const Runnable& runnable, int64_t waitMillis);

    /** @return the main thread handler. */
    Handler& getHandler();

    /** Updates the times to reflect the passing of a reboot. */
    void updateAfterReboot();

    /** Updates the times to reflect a time set. */
    void updateAfterTimeSet();

    /** Notifications are cut on cdroid; this is a no-op. */
    void updateAllNotifications();

    // --- Cities ---
    const std::vector<City>& getAllCities();
    const City& getHomeCity();
    const std::vector<City>& getUnselectedCities();
    const std::vector<City>& getSelectedCities();
    void setSelectedCities(const std::vector<City>& cities);
    std::function<int(const City&, const City&)> getCityIndexComparator();
    CitySort getCitySort() const;
    void toggleCitySort();
    void addCityListener(const CityListener& cityListener);
    void removeCityListener(const CityListener& cityListener);

    // --- Timers ---
    void addTimerListener(const TimerListener& timerListener);
    void removeTimerListener(const TimerListener& timerListener);
    const std::vector<Timer>& getTimers();
    const std::vector<Timer>& getExpiredTimers();
    bool getTimer(int timerId, Timer& outTimer);
    bool getMostRecentExpiredTimer(Timer& outTimer);
    Timer addTimer(int64_t length, const std::string& label, bool deleteAfterUse);
    void removeTimer(const Timer& timer);
    void startTimer(const Timer& timer);
    void pauseTimer(const Timer& timer);
    void expireTimer(const Timer& timer);
    bool resetTimer(const Timer& timer, Timer& outTimer);
    bool resetOrDeleteTimer(const Timer& timer, int eventLabelId, Timer& outTimer);
    void resetOrDeleteExpiredTimers(int eventLabelId);
    void resetUnexpiredTimers(int eventLabelId);
    void resetMissedTimers(int eventLabelId);
    void addTimerMinute(const Timer& timer);
    void setTimerLabel(const Timer& timer, const std::string& label);
    void setTimerLength(const Timer& timer, int64_t length);
    void setRemainingTime(const Timer& timer, int64_t remainingTime);
    Uri* getDefaultTimerRingtoneUri() const;
    bool isTimerRingtoneSilent();
    Uri* getTimerRingtoneUri();
    std::string getTimerRingtoneTitle();
    int64_t getTimerCrescendoDuration() const;
    void setTimerRingtoneUri(const Uri* uri);
    bool getTimerVibrate() const;
    void setTimerVibrate(bool enabled);

    // --- Ringtone (AOSP "ringtone data" surface) ---
    /** @return a heap Uri owned by the caller. */
    Uri* getDefaultAlarmRingtoneUri();
    void setDefaultAlarmRingtoneUri(const Uri* uri);
    const std::vector<CustomRingtone>& getCustomRingtones();
    const CustomRingtone* addCustomRingtone(const std::string& uri, const std::string& title);
    void removeCustomRingtone(const std::string& uri);
    const std::vector<std::pair<std::string, std::string>>& getSystemRingtones();
    void loadRingtoneTitles();
    void loadRingtonePermissions();
    std::string getRingtoneTitle(const std::string& uri);

    // --- Stopwatch ---
    void addStopwatchListener(const StopwatchListener& stopwatchListener);
    void removeStopwatchListener(const StopwatchListener& stopwatchListener);
    const Stopwatch& getStopwatch();
    Stopwatch startStopwatch();
    Stopwatch pauseStopwatch();
    Stopwatch resetStopwatch();
    const std::vector<Lap>& getLaps();
    bool canAddMoreLaps();
    bool addLap(Lap& outLap);
    int64_t getLongestLapTime();
    int64_t getCurrentLapTime(int64_t time) const;

    // --- Time ---
    int64_t currentTimeMillis() const;
    int64_t elapsedRealtime() const;
    bool is24HourFormat() const;
    std::unique_ptr<Calendar> getCalendar() const;

    // --- Settings ---
    ClockStyle getClockStyle() const;
    ClockStyle getScreensaverClockStyle() const;
    bool getDisplayClockSeconds() const;
    void setDisplayClockSeconds(bool displaySeconds);
    bool getScreensaverNightModeOn() const;
    TimeZone getHomeTimeZone() const;
    bool getShowHomeClock() const;
    AlarmVolumeButtonBehavior getAlarmVolumeButtonBehavior() const;
    int getAlarmTimeout() const;
    int getSnoozeLength() const;
    int64_t getAlarmCrescendoDuration() const;
    Weekdays::Order::Value getWeekdayOrder() const;

    // --- Notifications (flag only) ---
    bool isApplicationInForeground() const;
    void setApplicationInForeground(bool inForeground) const;

    // --- Silent settings (listeners stored; never fire on cdroid) ---
    void addSilentSettingsListener(const OnSilentSettingsListener& listener);
    void removeSilentSettingsListener(const OnSilentSettingsListener& listener);
};

} // namespace data
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_DATAMODEL_H__
