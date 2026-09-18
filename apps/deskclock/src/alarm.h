#ifndef __DESKCLOCK_ALARM_H__
#define __DESKCLOCK_ALARM_H__
/*********************************************************************************
 * Port of com.android.deskclock.provider.Alarm — the alarm domain object and
 * its SharedPreferences-backed DAO (upstream persists via SQLite/ClockProvider;
 * the cdroid port follows the TimerDAO prefs pattern). Lives in namespace data
 * with the other domain objects (upstream package: provider).
 *
 * CDROID note: Weekdays has a const `bits` member so it cannot be copy-assigned;
 * the alarm stores the raw bits and hands out Weekdays views via getWeekdays().
 *********************************************************************************/
#include <cstdint>
#include <string>
#include <vector>

#include <core/calendar.h>
#include <content/sharedpreferences.h>

#include <alarminstance.h>
#include <weekdays.h>

namespace cdroid {
namespace deskclock {
namespace data {

class Alarm {
public:
    /** Alarms start with an invalid id when not yet saved. */
    static constexpr int64_t INVALID_ID = -1;

    int64_t id = INVALID_ID;
    bool enabled = false;
    int hour = 0;
    int minutes = 0;
    /** 7-bit repeat schedule (Weekdays bits; kept raw for assignability). */
    int daysOfWeek = 0;
    bool vibrate = true;
    std::string label;
    /** Ringtone uri string; empty mirrors upstream's null = follow the default. */
    std::string alert;
    bool deleteAfterUse = false;

    /** The state of the alarm's next firing instance (joined-table column upstream). */
    int instanceState = 0;
    int64_t instanceId = Alarminstance::INVALID_ID;

    /** Creates a default alarm at the current time. */
    Alarm() = default;
    Alarm(int hour, int minutes) : hour(hour), minutes(minutes) {}

    Weekdays getWeekdays() const { return Weekdays::fromBits(daysOfWeek); }
    void setWeekdays(const Weekdays& weekdays) { daysOfWeek = weekdays.bits; }

    std::string getLabelOrDefault(Context& context) const;

    /** Whether the alarm is in a state to show preemptive dismiss. */
    bool canPreemptivelyDismiss() const;

    Alarminstance createInstanceAfter(Calendar& time) const;

    /** @return previous firing time, or nullptr if this is a one-time alarm. */
    std::unique_ptr<Calendar> getPreviousAlarmTime(Calendar& currentTime) const;

    Calendar getNextAlarmTime(Calendar& currentTime) const;

    bool operator==(const Alarm& other) const { return id == other.id; }

    //
    // AlarmDAO (upstream Alarm.getAlarm/addAlarm/updateAlarm/deleteAlarm via
    // ContentResolver).
    //
    static bool getAlarm(SharedPreferences& prefs, int64_t alarmId, Alarm& outAlarm);
    static std::vector<Alarm> getAlarms(SharedPreferences& prefs);
    static Alarm addAlarm(SharedPreferences& prefs, const Alarm& alarm);
    static bool updateAlarm(SharedPreferences& prefs, const Alarm& alarm);
    static bool deleteAlarm(SharedPreferences& prefs, int64_t alarmId);

    /** Upstream Alarm.isTomorrow. */
    static bool isTomorrow(const Alarm& alarm, Calendar& now);
};

} // namespace data
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ALARM_H__
