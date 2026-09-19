#include <alarminstance.h>

#include <R.h>

#include <core/context.h>

namespace cdroid {
namespace deskclock {
namespace data {

namespace {
constexpr const char* INSTANCE_IDS = "instances_list";
constexpr const char* NEXT_INSTANCE_ID = "next_instance_id";
constexpr const char* YEAR = "instance_year_";
constexpr const char* MONTH = "instance_month_";
constexpr const char* DAY = "instance_day_";
constexpr const char* HOUR = "instance_hour_";
constexpr const char* MINUTE = "instance_minute_";
constexpr const char* LABEL = "instance_label_";
constexpr const char* VIBRATE = "instance_vibrate_";
constexpr const char* RINGTONE = "instance_ringtone_";
constexpr const char* ALARM_ID = "instance_alarm_id_";
constexpr const char* STATE = "instance_state_";

std::string key(const char* prefix, int64_t id) {
    return std::string(prefix) + std::to_string(id);
}

void writeInstance(SharedPreferences& prefs, const Alarminstance& instance) {
    SharedPreferences::Editor& e = prefs.edit();
    e.putInt(key(YEAR, instance.mId), instance.mYear);
    e.putInt(key(MONTH, instance.mId), instance.mMonth);
    e.putInt(key(DAY, instance.mId), instance.mDay);
    e.putInt(key(HOUR, instance.mId), instance.mHour);
    e.putInt(key(MINUTE, instance.mId), instance.mMinute);
    e.putString(key(LABEL, instance.mId), instance.mLabel);
    e.putBoolean(key(VIBRATE, instance.mId), instance.mVibrate);
    e.putString(key(RINGTONE, instance.mId), instance.mRingtone);
    e.putLong(key(ALARM_ID, instance.mId), instance.mAlarmId);
    e.putInt(key(STATE, instance.mId), instance.mAlarmState);
    e.apply();
}
} // namespace

Alarminstance::Alarminstance(Calendar& calendar) {
    mId = INVALID_ID;
    setAlarmTime(calendar);
    mLabel = "";
    mVibrate = false;
    mAlarmState = SILENT_STATE;
}

Alarminstance::Alarminstance(Calendar& calendar, int64_t alarmId)
    : Alarminstance(calendar) {
    mAlarmId = alarmId;
}

Calendar Alarminstance::getAlarmTime() const {
    auto calendar = Calendar::getInstance();
    calendar->set(Calendar::YEAR, mYear);
    calendar->set(Calendar::MONTH, mMonth);
    calendar->set(Calendar::DAY_OF_MONTH, mDay);
    calendar->set(Calendar::HOUR_OF_DAY, mHour);
    calendar->set(Calendar::MINUTE, mMinute);
    calendar->set(Calendar::SECOND, 0);
    calendar->set(Calendar::MILLISECOND, 0);
    return std::move(*calendar);
}

void Alarminstance::setAlarmTime(Calendar& calendar) {
    mYear = calendar.get(Calendar::YEAR);
    mMonth = calendar.get(Calendar::MONTH);
    mDay = calendar.get(Calendar::DAY_OF_MONTH);
    mHour = calendar.get(Calendar::HOUR_OF_DAY);
    mMinute = calendar.get(Calendar::MINUTE);
}

Calendar Alarminstance::getLowNotificationTime() const {
    Calendar calendar = getAlarmTime();
    calendar.add(Calendar::HOUR_OF_DAY, LOW_NOTIFICATION_HOUR_OFFSET);
    return calendar;
}

Calendar Alarminstance::getHighNotificationTime() const {
    Calendar calendar = getAlarmTime();
    calendar.add(Calendar::MINUTE, HIGH_NOTIFICATION_MINUTE_OFFSET);
    return calendar;
}

Calendar Alarminstance::getMissedTimeToLive() const {
    Calendar calendar = getAlarmTime();
    calendar.add(Calendar::HOUR, MISSED_TIME_TO_LIVE_HOUR_OFFSET);
    return calendar;
}

bool Alarminstance::getInstance(SharedPreferences& prefs, int64_t id, Alarminstance& out) {
    for (const Alarminstance& instance : getInstances(prefs)) {
        if (instance.mId == id) {
            out = instance;
            return true;
        }
    }
    return false;
}

std::vector<Alarminstance> Alarminstance::getInstances(SharedPreferences& prefs) {
    std::vector<Alarminstance> result;
    for (const std::string& idStr : prefs.getStringSet(INSTANCE_IDS, {})) {
        const int64_t id = atoll(idStr.c_str());
        Alarminstance instance;
        instance.mId = id;
        instance.mYear = prefs.getInt(key(YEAR, id), 0);
        instance.mMonth = prefs.getInt(key(MONTH, id), 0);
        instance.mDay = prefs.getInt(key(DAY, id), 0);
        instance.mHour = prefs.getInt(key(HOUR, id), 0);
        instance.mMinute = prefs.getInt(key(MINUTE, id), 0);
        instance.mLabel = prefs.getString(key(LABEL, id), "");
        instance.mVibrate = prefs.getBoolean(key(VIBRATE, id), false);
        instance.mRingtone = prefs.getString(key(RINGTONE, id), "");
        instance.mAlarmId = prefs.getLong(key(ALARM_ID, id), INVALID_ID);
        instance.mAlarmState = prefs.getInt(key(STATE, id), SILENT_STATE);
        result.push_back(instance);
    }
    return result;
}

std::vector<Alarminstance> Alarminstance::getInstancesByAlarmId(SharedPreferences& prefs,
                                                                int64_t alarmId) {
    std::vector<Alarminstance> result;
    for (const Alarminstance& instance : getInstances(prefs)) {
        if (instance.mAlarmId == alarmId) result.push_back(instance);
    }
    return result;
}

Alarminstance Alarminstance::addInstance(SharedPreferences& prefs, const Alarminstance& in) {
    Alarminstance instance = in;
    instance.mId = prefs.getLong(NEXT_INSTANCE_ID, 1);
    SharedPreferences::Editor& e = prefs.edit();
    e.putLong(NEXT_INSTANCE_ID, instance.mId + 1);
    std::set<std::string> ids = prefs.getStringSet(INSTANCE_IDS, {});
    ids.insert(std::to_string(instance.mId));
    e.putStringSet(INSTANCE_IDS, ids);
    e.apply();
    writeInstance(prefs, instance);
    return instance;
}

bool Alarminstance::updateInstance(SharedPreferences& prefs, const Alarminstance& instance) {
    if (instance.mId == INVALID_ID) return false;
    std::set<std::string> ids = prefs.getStringSet(INSTANCE_IDS, {});
    if (ids.find(std::to_string(instance.mId)) == ids.end()) return false;
    writeInstance(prefs, instance);
    return true;
}

bool Alarminstance::deleteInstance(SharedPreferences& prefs, int64_t instanceId) {
    std::set<std::string> ids = prefs.getStringSet(INSTANCE_IDS, {});
    const std::string idStr = std::to_string(instanceId);
    if (ids.find(idStr) == ids.end()) return false;
    ids.erase(idStr);
    SharedPreferences::Editor& e = prefs.edit();
    e.putStringSet(INSTANCE_IDS, ids);
    for (const char* prefix : {YEAR, MONTH, DAY, HOUR, MINUTE, LABEL, VIBRATE, RINGTONE,
                               ALARM_ID, STATE}) {
        e.remove(key(prefix, instanceId));
    }
    e.apply();
    return true;
}

void Alarminstance::deleteOtherInstances(SharedPreferences& prefs, int64_t alarmId,
                                         int64_t keepInstanceId) {
    for (const Alarminstance& instance : getInstancesByAlarmId(prefs, alarmId)) {
        if (instance.mId != keepInstanceId) {
            deleteInstance(prefs, instance.mId);
        }
    }
}

} // namespace data
} // namespace deskclock
} // namespace cdroid
