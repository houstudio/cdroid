#ifndef __DESKCLOCK_ALARMINSTANCE_H__
#define __DESKCLOCK_ALARMINSTANCE_H__
/*********************************************************************************
 * Port of com.android.deskclock.provider.AlarmInstance — one firing of an
 * alarm, with the state machine constants from ClockContract.InstancesColumns.
 * Persisted via SharedPreferences (see Alarm).
 *********************************************************************************/
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <core/calendar.h>
#include <content/sharedpreferences.h>

namespace cdroid {
namespace deskclock {
namespace data {

/** Upstream class name is AlarmInstance; renamed to avoid clashing with the
 *  android.widget.AlarmClock-adjacent naming rule of one-class-per-file. */
class Alarminstance {
public:
    // ClockContract.InstancesColumns states.
    static constexpr int SILENT_STATE = 0;
    static constexpr int LOW_NOTIFICATION_STATE = 1;
    static constexpr int HIDE_NOTIFICATION_STATE = 2;
    static constexpr int HIGH_NOTIFICATION_STATE = 3;
    static constexpr int SNOOZE_STATE = 4;
    static constexpr int FIRED_STATE = 5;
    static constexpr int MISSED_STATE = 6;
    static constexpr int DISMISSED_STATE = 7;
    static constexpr int PREDISMISSED_STATE = 8;

    /** Offset from alarm time to show low priority notification. */
    static constexpr int LOW_NOTIFICATION_HOUR_OFFSET = -2;
    /** Offset from alarm time to show high priority notification. */
    static constexpr int HIGH_NOTIFICATION_MINUTE_OFFSET = -30;
    /** Offset from alarm time to stop showing missed notification. */
    static constexpr int MISSED_TIME_TO_LIVE_HOUR_OFFSET = 12;

    static constexpr int64_t INVALID_ID = -1;

    int mYear = 0;
    int mMonth = 0;
    int mDay = 0;
    int mHour = 0;
    int mMinute = 0;

    int64_t mId = INVALID_ID;
    std::string mLabel;
    bool mVibrate = false;
    std::string mRingtone;
    int64_t mAlarmId = INVALID_ID;
    int mAlarmState = SILENT_STATE;

    Alarminstance() = default;
    Alarminstance(Calendar& calendar, int64_t alarmId);
    explicit Alarminstance(Calendar& calendar);

    /** @return the time when the alarm should fire. */
    Calendar getAlarmTime() const;
    void setAlarmTime(Calendar& calendar);

    /** @return the time when a low priority notification should be shown. */
    Calendar getLowNotificationTime() const;

    /** @return the time when a high priority notification should be shown. */
    Calendar getHighNotificationTime() const;

    /** @return the time when a missed notification should be removed. */
    Calendar getMissedTimeToLive() const;

    bool operator==(const Alarminstance& other) const { return mId == other.mId; }

    //
    // AlarmInstanceDAO (upstream ContentResolver CRUD).
    //
    static bool getInstance(SharedPreferences& prefs, int64_t id, Alarminstance& outInstance);
    static std::vector<Alarminstance> getInstances(SharedPreferences& prefs);
    static std::vector<Alarminstance> getInstancesByAlarmId(SharedPreferences& prefs,
                                                            int64_t alarmId);
    static Alarminstance addInstance(SharedPreferences& prefs, const Alarminstance& instance);
    static bool updateInstance(SharedPreferences& prefs, const Alarminstance& instance);
    static bool deleteInstance(SharedPreferences& prefs, int64_t instanceId);

    /** Removes any other scheduled instances that may exist for the alarm. */
    static void deleteOtherInstances(SharedPreferences& prefs, int64_t alarmId,
                                     int64_t keepInstanceId);
};

} // namespace data
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ALARMINSTANCE_H__
