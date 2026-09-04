#include <alarm.h>

#include <R.h>

#include <algorithm>

#include <core/context.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace data {

namespace {
constexpr const char* ALARM_IDS = "alarms_list";
constexpr const char* NEXT_ALARM_ID = "next_alarm_id";
constexpr const char* ENABLED = "alarm_enabled_";
constexpr const char* HOUR = "alarm_hour_";
constexpr const char* MINUTES = "alarm_minutes_";
constexpr const char* DAYS_OF_WEEK = "alarm_days_of_week_";
constexpr const char* VIBRATE = "alarm_vibrate_";
constexpr const char* LABEL = "alarm_label_";
constexpr const char* RINGTONE = "alarm_ringtone_";
constexpr const char* DELETE_AFTER_USE = "alarm_delete_after_use_";

std::string key(const char* prefix, int64_t id) {
    return std::string(prefix) + std::to_string(id);
}

void writeAlarm(SharedPreferences& prefs, const Alarm& alarm) {
    SharedPreferences::Editor& e = prefs.edit();
    e.putBoolean(key(ENABLED, alarm.id), alarm.enabled);
    e.putInt(key(HOUR, alarm.id), alarm.hour);
    e.putInt(key(MINUTES, alarm.id), alarm.minutes);
    e.putInt(key(DAYS_OF_WEEK, alarm.id), alarm.daysOfWeek);
    e.putBoolean(key(VIBRATE, alarm.id), alarm.vibrate);
    e.putString(key(LABEL, alarm.id), alarm.label);
    e.putString(key(RINGTONE, alarm.id), alarm.alert);
    e.putBoolean(key(DELETE_AFTER_USE, alarm.id), alarm.deleteAfterUse);
    e.apply();
}
} // namespace

std::string Alarm::getLabelOrDefault(Context& context) const {
    return label.empty() ? context.getString(R::string::default_label) : label;
}

bool Alarm::canPreemptivelyDismiss() const {
    return instanceState == Alarminstance::SNOOZE_STATE
            || instanceState == Alarminstance::HIGH_NOTIFICATION_STATE
            || instanceState == Alarminstance::LOW_NOTIFICATION_STATE
            || instanceState == Alarminstance::HIDE_NOTIFICATION_STATE;
}

Alarminstance Alarm::createInstanceAfter(Calendar& time) const {
    Calendar nextInstanceTime = getNextAlarmTime(time);
    Alarminstance result(nextInstanceTime, id);
    result.mVibrate = vibrate;
    result.mLabel = label;
    result.mRingtone = alert;
    return result;
}

std::unique_ptr<Calendar> Alarm::getPreviousAlarmTime(Calendar& currentTime) const {
    auto previousInstanceTime = Calendar::getInstance();
    previousInstanceTime->set(Calendar::YEAR, currentTime.get(Calendar::YEAR));
    previousInstanceTime->set(Calendar::MONTH, currentTime.get(Calendar::MONTH));
    previousInstanceTime->set(Calendar::DAY_OF_MONTH, currentTime.get(Calendar::DAY_OF_MONTH));
    previousInstanceTime->set(Calendar::HOUR_OF_DAY, hour);
    previousInstanceTime->set(Calendar::MINUTE, minutes);
    previousInstanceTime->set(Calendar::SECOND, 0);
    previousInstanceTime->set(Calendar::MILLISECOND, 0);

    const int subtractDays = getWeekdays().getDistanceToPreviousDay(*previousInstanceTime);
    if (subtractDays > 0) {
        previousInstanceTime->add(Calendar::DAY_OF_WEEK, -subtractDays);
        return previousInstanceTime;
    }
    return nullptr;
}

Calendar Alarm::getNextAlarmTime(Calendar& currentTime) const {
    auto nextInstanceTime = Calendar::getInstance();
    nextInstanceTime->set(Calendar::YEAR, currentTime.get(Calendar::YEAR));
    nextInstanceTime->set(Calendar::MONTH, currentTime.get(Calendar::MONTH));
    nextInstanceTime->set(Calendar::DAY_OF_MONTH, currentTime.get(Calendar::DAY_OF_MONTH));
    nextInstanceTime->set(Calendar::HOUR_OF_DAY, hour);
    nextInstanceTime->set(Calendar::MINUTE, minutes);
    nextInstanceTime->set(Calendar::SECOND, 0);
    nextInstanceTime->set(Calendar::MILLISECOND, 0);

    // If we are still behind the passed in currentTime, then add a day.
    if (nextInstanceTime->getTimeInMillis() <= currentTime.getTimeInMillis()) {
        nextInstanceTime->add(Calendar::DAY_OF_YEAR, 1);
    }

    // The day of the week might be invalid, so find next valid one.
    const int addDays = getWeekdays().getDistanceToNextDay(*nextInstanceTime);
    if (addDays > 0) {
        nextInstanceTime->add(Calendar::DAY_OF_WEEK, addDays);
    }

    // Daylight Savings Time can alter the hours and minutes when adjusting the day above.
    // Reset the desired hour and minute now that the correct day has been chosen.
    nextInstanceTime->set(Calendar::HOUR_OF_DAY, hour);
    nextInstanceTime->set(Calendar::MINUTE, minutes);

    return std::move(*nextInstanceTime);
}

bool Alarm::isTomorrow(const Alarm& alarm, Calendar& now) {
    if (alarm.instanceState == Alarminstance::SNOOZE_STATE) {
        return false;
    }

    const int totalAlarmMinutes = alarm.hour * 60 + alarm.minutes;
    const int totalNowMinutes = now.get(Calendar::HOUR_OF_DAY) * 60 + now.get(Calendar::MINUTE);
    return totalAlarmMinutes <= totalNowMinutes;
}

bool Alarm::getAlarm(SharedPreferences& prefs, int64_t alarmId, Alarm& out) {
    for (const Alarm& alarm : getAlarms(prefs)) {
        if (alarm.id == alarmId) {
            out = alarm;
            return true;
        }
    }
    return false;
}

std::vector<Alarm> Alarm::getAlarms(SharedPreferences& prefs) {
    // Fetch the set of alarms and sort by the upstream DEFAULT_SORT_ORDER
    // (hour, minutes ASC, id DESC).
    std::vector<Alarm> alarms;
    for (const std::string& idStr : prefs.getStringSet(ALARM_IDS, {})) {
        const int64_t id = atoll(idStr.c_str());
        Alarm alarm;
        alarm.id = id;
        alarm.enabled = prefs.getBoolean(key(ENABLED, id), false);
        alarm.hour = prefs.getInt(key(HOUR, id), 0);
        alarm.minutes = prefs.getInt(key(MINUTES, id), 0);
        alarm.daysOfWeek = prefs.getInt(key(DAYS_OF_WEEK, id), 0);
        alarm.vibrate = prefs.getBoolean(key(VIBRATE, id), true);
        alarm.label = prefs.getString(key(LABEL, id), "");
        alarm.alert = prefs.getString(key(RINGTONE, id), "");
        alarm.deleteAfterUse = prefs.getBoolean(key(DELETE_AFTER_USE, id), false);
        alarms.push_back(alarm);
    }
    std::sort(alarms.begin(), alarms.end(), [](const Alarm& lhs, const Alarm& rhs) {
        if (lhs.hour != rhs.hour) return lhs.hour < rhs.hour;
        if (lhs.minutes != rhs.minutes) return lhs.minutes < rhs.minutes;
        return lhs.id > rhs.id;
    });
    return alarms;
}

Alarm Alarm::addAlarm(SharedPreferences& prefs, const Alarm& in) {
    Alarm alarm = in;
    alarm.id = prefs.getLong(NEXT_ALARM_ID, 1);
    SharedPreferences::Editor& e = prefs.edit();
    e.putLong(NEXT_ALARM_ID, alarm.id + 1);
    std::set<std::string> ids = prefs.getStringSet(ALARM_IDS, {});
    ids.insert(std::to_string(alarm.id));
    e.putStringSet(ALARM_IDS, ids);
    e.apply();
    writeAlarm(prefs, alarm);
    return alarm;
}

bool Alarm::updateAlarm(SharedPreferences& prefs, const Alarm& alarm) {
    if (alarm.id == INVALID_ID) return false;
    std::set<std::string> ids = prefs.getStringSet(ALARM_IDS, {});
    if (ids.find(std::to_string(alarm.id)) == ids.end()) return false;
    writeAlarm(prefs, alarm);
    return true;
}

bool Alarm::deleteAlarm(SharedPreferences& prefs, int64_t alarmId) {
    std::set<std::string> ids = prefs.getStringSet(ALARM_IDS, {});
    const std::string idStr = std::to_string(alarmId);
    if (ids.find(idStr) == ids.end()) return false;
    ids.erase(idStr);
    SharedPreferences::Editor& e = prefs.edit();
    e.putStringSet(ALARM_IDS, ids);
    for (const char* prefix : {ENABLED, HOUR, MINUTES, DAYS_OF_WEEK, VIBRATE, LABEL, RINGTONE,
                               DELETE_AFTER_USE}) {
        e.remove(key(prefix, alarmId));
    }
    e.apply();
    return true;
}

} // namespace data
} // namespace deskclock
} // namespace cdroid
