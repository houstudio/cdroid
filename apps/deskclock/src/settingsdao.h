#ifndef __DESKCLOCK_SETTINGSDAO_H__
#define __DESKCLOCK_SETTINGSDAO_H__
/*********************************************************************************
 * Port of com.android.deskclock.data.{SettingsDAO,SettingsModel,TimeZones} —
 * settings access over SharedPreferences. The KEY_* constants mirror the
 * SettingsActivity / ScreensaverSettingsActivity companion objects upstream.
 *********************************************************************************/
#include <string>
#include <vector>

#include <core/uri.h>
#include <timezone.h>
#include <weekdays.h>

namespace cdroid {

class Context;
class SharedPreferences;

namespace deskclock {
namespace data {

// --- ClockStyle / CitySort / AlarmVolumeButtonBehavior (DataModel enums) ---
enum class ClockStyle { ANALOG, DIGITAL };
enum class CitySort { NAME, UTC_OFFSET };
enum class AlarmVolumeButtonBehavior { NOTHING, SNOOZE, DISMISS };

// SettingsActivity.KEY_* / ScreensaverSettingsActivity.KEY_* companions.
constexpr const char* KEY_AUTO_HOME_CLOCK = "automatic_home_clock";
constexpr const char* KEY_HOME_TZ = "home_time_zone";
constexpr const char* KEY_CLOCK_STYLE = "clock_style";
constexpr const char* KEY_CLOCK_DISPLAY_SECONDS = "display_clock_seconds";
constexpr const char* KEY_TIMER_RINGTONE = "timer_ringtone";
constexpr const char* KEY_TIMER_VIBRATE = "timer_vibrate";
constexpr const char* KEY_TIMER_CRESCENDO = "timer_crescendo_duration";
constexpr const char* KEY_ALARM_CRESCENDO = "alarm_crescendo_duration";
constexpr const char* KEY_WEEK_START = "week_start";
constexpr const char* KEY_VOLUME_BUTTONS = "volume_button_setting";
constexpr const char* KEY_AUTO_SILENCE = "auto_silence";
constexpr const char* KEY_ALARM_SNOOZE = "snooze_duration";
// ScreensaverSettingsActivity.KEY_* companions.
constexpr const char* SCREENSAVER_KEY_CLOCK_STYLE = "screensaver_clock_style";
constexpr const char* SCREENSAVER_KEY_NIGHT_MODE = "screensaver_night_mode";
// SettingsActivity volume-behavior values.
constexpr const char* DEFAULT_VOLUME_BEHAVIOR = "0";
constexpr const char* VOLUME_BEHAVIOR_SNOOZE = "1";
constexpr const char* VOLUME_BEHAVIOR_DISMISS = "2";

/** A read-only domain object representing the timezones from which to choose a home zone. */
class TimeZones {
public:
    std::vector<std::string> timeZoneIds;
    std::vector<std::string> timeZoneNames;

    TimeZones(std::vector<std::string> ids, std::vector<std::string> names)
        : timeZoneIds(std::move(ids)), timeZoneNames(std::move(names)) {}

    /** @return the timezone name with the given id; empty if it does not exist. */
    std::string getTimeZoneName(const std::string& timeZoneId) const;

    /** @return true iff the timezone with the given id is present. */
    bool contains(const std::string& timeZoneId) const;
};

class SettingsDAO {
public:
    static int getGlobalIntentId(SharedPreferences& prefs);
    static void updateGlobalIntentId(SharedPreferences& prefs);

    static CitySort getCitySort(SharedPreferences& prefs);
    static void toggleCitySort(SharedPreferences& prefs);

    static bool getAutoShowHomeClock(SharedPreferences& prefs);
    static TimeZone getHomeTimeZone(Context& context, SharedPreferences& prefs,
                                    const TimeZone& defaultTZ);

    static ClockStyle getClockStyle(Context& context, SharedPreferences& prefs);
    static bool getDisplayClockSeconds(SharedPreferences& prefs);
    static void setDisplayClockSeconds(SharedPreferences& prefs, bool displaySeconds);
    static void setDefaultDisplayClockSeconds(Context& context, SharedPreferences& prefs);

    static ClockStyle getScreensaverClockStyle(Context& context, SharedPreferences& prefs);
    static bool getScreensaverNightModeOn(SharedPreferences& prefs);

    /** @return a heap Uri owned by the caller (Uri is abstract; cdroid Intent-style). */
    static Uri* getTimerRingtoneUri(SharedPreferences& prefs, const Uri* defaultUri);
    static bool getTimerVibrate(SharedPreferences& prefs);
    static void setTimerVibrate(SharedPreferences& prefs, bool enabled);
    static void setTimerRingtoneUri(SharedPreferences& prefs, const Uri* uri);

    static Uri* getDefaultAlarmRingtoneUri(SharedPreferences& prefs);
    static void setDefaultAlarmRingtoneUri(SharedPreferences& prefs, const Uri* uri);

    static int64_t getAlarmCrescendoDuration(SharedPreferences& prefs);
    static int64_t getTimerCrescendoDuration(SharedPreferences& prefs);

    static Weekdays::Order::Value getWeekdayOrder(SharedPreferences& prefs);

    static AlarmVolumeButtonBehavior getAlarmVolumeButtonBehavior(SharedPreferences& prefs);
    static int getAlarmTimeout(SharedPreferences& prefs);
    static int getSnoozeLength(SharedPreferences& prefs);

    static TimeZones getTimeZones(Context& context, int64_t currentTime);

private:
    static ClockStyle getClockStyle(Context& context, SharedPreferences& prefs,
                                    const char* key);
};

/** All settings data is accessed via this model. */
class SettingsModel {
private:
    Context& mContext;
    SharedPreferences& mPrefs;

    /** The default timer-ringtone uri string until the user explicitly chooses one. */
    std::string mDefaultTimerRingtoneUriString;

public:
    SettingsModel(Context& context, SharedPreferences& prefs);

    int getGlobalIntentId() const;
    void updateGlobalIntentId();

    CitySort getCitySort() const;
    void toggleCitySort();

    TimeZone getHomeTimeZone() const;

    ClockStyle getClockStyle() const;
    bool getDisplayClockSeconds() const;
    void setDisplayClockSeconds(bool shouldDisplaySeconds);

    ClockStyle getScreensaverClockStyle() const;
    bool getScreensaverNightModeOn() const;

    bool getShowHomeClock() const;

    /** @return a heap Uri owned by the caller. */
    Uri* getDefaultTimerRingtoneUri();
    Uri* getTimerRingtoneUri();
    void setTimerRingtoneUri(const Uri* uri);

    bool getTimerVibrate() const;
    void setTimerVibrate(bool enabled);

    AlarmVolumeButtonBehavior getAlarmVolumeButtonBehavior() const;
    int getAlarmTimeout() const;
    int getSnoozeLength() const;

    int64_t getAlarmCrescendoDuration() const;
    int64_t getTimerCrescendoDuration() const;

    Weekdays::Order::Value getWeekdayOrder() const;
};

} // namespace data
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_SETTINGSDAO_H__
