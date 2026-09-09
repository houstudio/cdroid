#include <settingsdao.h>

#include <R.h>

#include <algorithm>
#include <cctype>

#include <content/resources.h>
#include <content/sharedpreferences.h>
#include <core/calendar.h>
#include <core/context.h>
#include <core/systemclock.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace data {

namespace {

constexpr const char* KEY_SORT_PREFERENCE = "sort_preference";
constexpr const char* KEY_DEFAULT_ALARM_RINGTONE_URI = "default_alarm_ringtone_uri";
constexpr const char* KEY_ALARM_GLOBAL_ID = "intent.extra.alarm.global.id";

constexpr int64_t SECOND_IN_MILLIS = 1000LL;

std::string toUpper(const std::string& s) {
    std::string out = s;
    for (char& c : out) c = toupper((unsigned char) c);
    return out;
}

/** Locale-aware name for the zone at the given offset, formatted like the upstream descriptor. */
struct TimeZoneDescriptor {
    int offset;
    std::string id;
    std::string name;

    bool operator<(const TimeZoneDescriptor& other) const {
        if (offset != other.offset) return offset < other.offset;
        return name < other.name;
    }
};

} // namespace

//
// TimeZones
//

std::string TimeZones::getTimeZoneName(const std::string& timeZoneId) const {
    for (size_t i = 0; i < timeZoneIds.size(); i++) {
        if (timeZoneId == timeZoneIds[i]) {
            return timeZoneNames[i];
        }
    }
    return std::string();
}

bool TimeZones::contains(const std::string& timeZoneId) const {
    return !getTimeZoneName(timeZoneId).empty();
}

//
// SettingsDAO
//

int SettingsDAO::getGlobalIntentId(SharedPreferences& prefs) {
    return prefs.getInt(KEY_ALARM_GLOBAL_ID, -1);
}

void SettingsDAO::updateGlobalIntentId(SharedPreferences& prefs) {
    const int globalId = prefs.getInt(KEY_ALARM_GLOBAL_ID, -1) + 1;
    prefs.edit().putInt(KEY_ALARM_GLOBAL_ID, globalId).apply();
}

CitySort SettingsDAO::getCitySort(SharedPreferences& prefs) {
    const int defaultSortOrdinal = (int) CitySort::NAME;
    const int citySortOrdinal = prefs.getInt(KEY_SORT_PREFERENCE, defaultSortOrdinal);
    if (citySortOrdinal == (int) CitySort::UTC_OFFSET) return CitySort::UTC_OFFSET;
    return CitySort::NAME;
}

void SettingsDAO::toggleCitySort(SharedPreferences& prefs) {
    const CitySort oldSort = getCitySort(prefs);
    const CitySort newSort = (oldSort == CitySort::NAME) ? CitySort::UTC_OFFSET : CitySort::NAME;
    prefs.edit().putInt(KEY_SORT_PREFERENCE, (int) newSort).apply();
}

bool SettingsDAO::getAutoShowHomeClock(SharedPreferences& prefs) {
    return prefs.getBoolean(KEY_AUTO_HOME_CLOCK, true);
}

TimeZone SettingsDAO::getHomeTimeZone(Context& context, SharedPreferences& prefs,
                                      const TimeZone& defaultTZ) {
    std::string timeZoneId = prefs.getString(KEY_HOME_TZ, "");

    const TimeZones timeZones = getTimeZones(context, SystemClock::currentTimeMillis());
    if (timeZones.contains(timeZoneId)) {
        return TimeZone::getTimeZone(timeZoneId);
    }

    timeZoneId = defaultTZ.getID();
    if (timeZones.contains(timeZoneId)) {
        prefs.edit().putString(KEY_HOME_TZ, timeZoneId).apply();
    }

    return defaultTZ;
}

ClockStyle SettingsDAO::getClockStyle(Context& context, SharedPreferences& prefs) {
    return getClockStyle(context, prefs, KEY_CLOCK_STYLE);
}

bool SettingsDAO::getDisplayClockSeconds(SharedPreferences& prefs) {
    return prefs.getBoolean(KEY_CLOCK_DISPLAY_SECONDS, false);
}

void SettingsDAO::setDisplayClockSeconds(SharedPreferences& prefs, bool displaySeconds) {
    prefs.edit().putBoolean(KEY_CLOCK_DISPLAY_SECONDS, displaySeconds).apply();
}

void SettingsDAO::setDefaultDisplayClockSeconds(Context& context, SharedPreferences& prefs) {
    // Set the user's default display seconds preference if one has not yet been chosen.
    if (!prefs.contains(KEY_CLOCK_DISPLAY_SECONDS)) {
        const bool isAnalog = getClockStyle(context, prefs) == ClockStyle::ANALOG;
        setDisplayClockSeconds(prefs, isAnalog);
    }
}

ClockStyle SettingsDAO::getScreensaverClockStyle(Context& context, SharedPreferences& prefs) {
    return getClockStyle(context, prefs, SCREENSAVER_KEY_CLOCK_STYLE);
}

bool SettingsDAO::getScreensaverNightModeOn(SharedPreferences& prefs) {
    return prefs.getBoolean(SCREENSAVER_KEY_NIGHT_MODE, false);
}

Uri* SettingsDAO::getTimerRingtoneUri(SharedPreferences& prefs, const Uri* defaultUri) {
    const std::string uriString = prefs.getString(KEY_TIMER_RINGTONE, "");
    return uriString.empty() ? Uri::parse(defaultUri->toString()) : Uri::parse(uriString);
}

bool SettingsDAO::getTimerVibrate(SharedPreferences& prefs) {
    return prefs.getBoolean(KEY_TIMER_VIBRATE, false);
}

void SettingsDAO::setTimerVibrate(SharedPreferences& prefs, bool enabled) {
    prefs.edit().putBoolean(KEY_TIMER_VIBRATE, enabled).apply();
}

void SettingsDAO::setTimerRingtoneUri(SharedPreferences& prefs, const Uri* uri) {
    prefs.edit().putString(KEY_TIMER_RINGTONE, uri->toString()).apply();
}

Uri* SettingsDAO::getDefaultAlarmRingtoneUri(SharedPreferences& prefs) {
    const std::string uriString = prefs.getString(KEY_DEFAULT_ALARM_RINGTONE_URI, "");
    return uriString.empty()
            ? Uri::parse("content://settings/system/alarm_alert") // Settings.System.DEFAULT_ALARM_ALERT_URI
            : Uri::parse(uriString);
}

void SettingsDAO::setDefaultAlarmRingtoneUri(SharedPreferences& prefs, const Uri* uri) {
    prefs.edit().putString(KEY_DEFAULT_ALARM_RINGTONE_URI, uri->toString()).apply();
}

int64_t SettingsDAO::getAlarmCrescendoDuration(SharedPreferences& prefs) {
    const std::string crescendoSeconds = prefs.getString(KEY_ALARM_CRESCENDO, "0");
    return atoll(crescendoSeconds.c_str()) * SECOND_IN_MILLIS;
}

int64_t SettingsDAO::getTimerCrescendoDuration(SharedPreferences& prefs) {
    const std::string crescendoSeconds = prefs.getString(KEY_TIMER_CRESCENDO, "0");
    return atoll(crescendoSeconds.c_str()) * SECOND_IN_MILLIS;
}

Weekdays::Order::Value SettingsDAO::getWeekdayOrder(SharedPreferences& prefs) {
    std::unique_ptr<Calendar> cal = Calendar::getInstance();
    const int defaultValue = cal->getFirstDayOfWeek();
    const std::string value = prefs.getString(KEY_WEEK_START, std::to_string(defaultValue));
    const int firstCalendarDay = atoi(value.c_str());
    switch (firstCalendarDay) {
        case Calendar::SATURDAY: return Weekdays::Order::SAT_TO_FRI;
        case Calendar::SUNDAY:   return Weekdays::Order::SUN_TO_SAT;
        case Calendar::MONDAY:   return Weekdays::Order::MON_TO_SUN;
        default: throw std::invalid_argument("Unknown weekday: " + std::to_string(firstCalendarDay));
    }
}

AlarmVolumeButtonBehavior SettingsDAO::getAlarmVolumeButtonBehavior(SharedPreferences& prefs) {
    const std::string value = prefs.getString(KEY_VOLUME_BUTTONS, DEFAULT_VOLUME_BEHAVIOR);
    if (value == VOLUME_BEHAVIOR_SNOOZE) return AlarmVolumeButtonBehavior::SNOOZE;
    if (value == VOLUME_BEHAVIOR_DISMISS) return AlarmVolumeButtonBehavior::DISMISS;
    return AlarmVolumeButtonBehavior::NOTHING;
}

int SettingsDAO::getAlarmTimeout(SharedPreferences& prefs) {
    // Default value must match the one in res/xml/settings.xml
    const std::string string = prefs.getString(KEY_AUTO_SILENCE, "10");
    return atoi(string.c_str());
}

int SettingsDAO::getSnoozeLength(SharedPreferences& prefs) {
    // Default value must match the one in res/xml/settings.xml
    const std::string string = prefs.getString(KEY_ALARM_SNOOZE, "10");
    return atoi(string.c_str());
}

TimeZones SettingsDAO::getTimeZones(Context& context, int64_t currentTime) {
    Resources& resources = context.getResources();
    const std::vector<std::string> timeZoneIds = resources.getStringArray(R::array::timezone_values);
    std::vector<std::string> timeZoneNames = resources.getStringArray(R::array::timezone_labels);

    // Verify the data is consistent.
    if (timeZoneIds.size() != timeZoneNames.size()) {
        throw std::logic_error("id count (" + std::to_string(timeZoneIds.size())
                + ") does not match name count (" + std::to_string(timeZoneNames.size()) + ")");
    }

    // Create TimeZoneDescriptors for each TimeZone so they can be sorted.
    std::vector<TimeZoneDescriptor> descriptors;
    descriptors.reserve(timeZoneIds.size());
    for (size_t i = 0; i < timeZoneIds.size(); i++) {
        const std::string& id = timeZoneIds[i];
        std::string name = timeZoneNames[i];
        name.erase(std::remove(name.begin(), name.end(), '"'), name.end());
        const TimeZone tz = TimeZone::getTimeZone(id);
        descriptors.push_back(TimeZoneDescriptor{tz.getOffset(currentTime), id, name});
    }
    std::sort(descriptors.begin(), descriptors.end());

    // Transfer the TimeZoneDescriptors into parallel arrays for easy consumption by the caller.
    std::vector<std::string> tzIds;
    std::vector<std::string> tzNames;
    tzIds.reserve(descriptors.size());
    tzNames.reserve(descriptors.size());
    for (const TimeZoneDescriptor& descriptor : descriptors) {
        tzIds.push_back(descriptor.id);
        tzNames.push_back(descriptor.name);
    }

    return TimeZones(std::move(tzIds), std::move(tzNames));
}

ClockStyle SettingsDAO::getClockStyle(Context& context, SharedPreferences& prefs, const char* key) {
    const std::string defaultStyle = context.getString(R::string::default_clock_style);
    const std::string clockStyle = prefs.getString(key, defaultStyle);
    // Use hardcoded locale to perform toUpperCase, because in some languages toUpperCase adds
    // accent to character, which breaks the enum conversion.
    const std::string upper = toUpper(clockStyle);
    if (upper == "ANALOG") return ClockStyle::ANALOG;
    return ClockStyle::DIGITAL;
}

//
// SettingsModel
//

SettingsModel::SettingsModel(Context& context, SharedPreferences& prefs)
    : mContext(context), mPrefs(prefs) {
    // Set the user's default display seconds preference if one has not yet been chosen.
    SettingsDAO::setDefaultDisplayClockSeconds(mContext, mPrefs);
}

int SettingsModel::getGlobalIntentId() const {
    return SettingsDAO::getGlobalIntentId(mPrefs);
}

void SettingsModel::updateGlobalIntentId() {
    SettingsDAO::updateGlobalIntentId(mPrefs);
}

CitySort SettingsModel::getCitySort() const {
    return SettingsDAO::getCitySort(mPrefs);
}

void SettingsModel::toggleCitySort() {
    SettingsDAO::toggleCitySort(mPrefs);
}

TimeZone SettingsModel::getHomeTimeZone() const {
    return SettingsDAO::getHomeTimeZone(mContext, mPrefs, TimeZone::getDefault());
}

ClockStyle SettingsModel::getClockStyle() const {
    return SettingsDAO::getClockStyle(mContext, mPrefs);
}

bool SettingsModel::getDisplayClockSeconds() const {
    return SettingsDAO::getDisplayClockSeconds(mPrefs);
}

void SettingsModel::setDisplayClockSeconds(bool shouldDisplaySeconds) {
    SettingsDAO::setDisplayClockSeconds(mPrefs, shouldDisplaySeconds);
}

ClockStyle SettingsModel::getScreensaverClockStyle() const {
    return SettingsDAO::getScreensaverClockStyle(mContext, mPrefs);
}

bool SettingsModel::getScreensaverNightModeOn() const {
    return SettingsDAO::getScreensaverNightModeOn(mPrefs);
}

bool SettingsModel::getShowHomeClock() const {
    if (!SettingsDAO::getAutoShowHomeClock(mPrefs)) {
        return false;
    }

    // Show the home clock if the current time and home time differ.
    // (By using UTC offset for this comparison the various DST rules are considered)
    const TimeZone defaultTZ = TimeZone::getDefault();
    const TimeZone homeTimeZone = SettingsDAO::getHomeTimeZone(mContext, mPrefs, defaultTZ);
    const int64_t now = SystemClock::currentTimeMillis();
    return homeTimeZone.getOffset(now) != defaultTZ.getOffset(now);
}

Uri* SettingsModel::getDefaultAlarmRingtoneUri() {
    return SettingsDAO::getDefaultAlarmRingtoneUri(mPrefs);
}

void SettingsModel::setDefaultAlarmRingtoneUri(const Uri* uri) {
    SettingsDAO::setDefaultAlarmRingtoneUri(mPrefs, uri);
}

Uri* SettingsModel::getDefaultTimerRingtoneUri() {
    if (mDefaultTimerRingtoneUriString.empty()) {
        // Utils.getResourceUri(mContext, R.raw.timer_expire)
        mDefaultTimerRingtoneUriString = "android.resource://cdroid.deskclock/"
                + std::to_string(R::raw::timer_expire);
    }
    return Uri::parse(mDefaultTimerRingtoneUriString);
}

Uri* SettingsModel::getTimerRingtoneUri() {
    std::unique_ptr<Uri> defaultUri(getDefaultTimerRingtoneUri());
    return SettingsDAO::getTimerRingtoneUri(mPrefs, defaultUri.get());
}

void SettingsModel::setTimerRingtoneUri(const Uri* uri) {
    SettingsDAO::setTimerRingtoneUri(mPrefs, uri);
}

bool SettingsModel::getTimerVibrate() const {
    return SettingsDAO::getTimerVibrate(mPrefs);
}

void SettingsModel::setTimerVibrate(bool enabled) {
    SettingsDAO::setTimerVibrate(mPrefs, enabled);
}

AlarmVolumeButtonBehavior SettingsModel::getAlarmVolumeButtonBehavior() const {
    return SettingsDAO::getAlarmVolumeButtonBehavior(mPrefs);
}

int SettingsModel::getAlarmTimeout() const {
    return SettingsDAO::getAlarmTimeout(mPrefs);
}

int SettingsModel::getSnoozeLength() const {
    return SettingsDAO::getSnoozeLength(mPrefs);
}

int64_t SettingsModel::getAlarmCrescendoDuration() const {
    return SettingsDAO::getAlarmCrescendoDuration(mPrefs);
}

int64_t SettingsModel::getTimerCrescendoDuration() const {
    return SettingsDAO::getTimerCrescendoDuration(mPrefs);
}

Weekdays::Order::Value SettingsModel::getWeekdayOrder() const {
    return SettingsDAO::getWeekdayOrder(mPrefs);
}

} // namespace data
} // namespace deskclock
} // namespace cdroid
