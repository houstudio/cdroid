#ifndef __DESKCLOCK_ALARMSTATEMANAGER_H__
#define __DESKCLOCK_ALARMSTATEMANAGER_H__
/*********************************************************************************
 * Port of com.android.deskclock.alarms.AlarmStateManager — handles all state
 * changes for alarm instances. Upstream schedules via AlarmManager/PendingIntent
 * broadcasts; cdroid schedules on a main-thread Handler (the TimerModel pattern).
 *
 * Stubs (recorded): AlarmNotifications (no notification stack on cdroid),
 * AlarmService/klaxon (no service stack), the full-screen AlarmActivity firing
 * UX (FIRED_STATE is reached and logged; a later round can port AlarmActivity).
 *********************************************************************************/
#include <map>

#include <core/handler.h>

#include <alarm.h>
#include <alarminstance.h>

namespace cdroid {
class Context;

namespace deskclock {
class AlarmUpdateHandler;

namespace alarms {

class AlarmStateManager {
public:
    /** Buffer time in seconds to fire alarm instead of marking it missed. */
    static constexpr int ALARM_FIRE_BUFFER = 15;

    /** Registers the instance and chooses the most appropriate state. */
    static void registerInstance(Context& context, data::Alarminstance& instance,
                                 bool updateNextAlarm);

    /** Sets SNOOZE_STATE with the DataModel snooze length. */
    static void setSnoozeState(Context& context, data::Alarminstance& instance);

    /** Sets PREDISMISSED_STATE (called from the alarm row's dismiss button). */
    static void setPreDismissState(Context& context, data::Alarminstance& instance);

    /** Deletes the instance, then reschedules/disables/deletes its parent alarm. */
    static void deleteInstanceAndUpdateParent(Context& context, data::Alarminstance& instance);

    /** DISMISSED_STATE + notifications/timers removed. */
    static void unregisterInstance(Context& context, data::Alarminstance& instance);

    /** Deletes and unregisters all instances of the alarm (major update/delete). */
    static void deleteAllInstances(Context& context, int64_t alarmId);

    /** Deletes and unregisters all non-snoozed instances (minor update). */
    static void deleteNonSnoozeInstances(Context& context, int64_t alarmId);

    /** Fixes all instances after major time changes / restart. */
    static void fixAlarmInstances(Context& context);

    /** @return the instance that will fire earliest, or nullptr. */
    static bool getNextFiringAlarm(Context& context, data::Alarminstance& outInstance);

    /**
     * Applies the alarm's live instance state to the alarm rows (the upstream
     * ALARMS_WITH_INSTANCES join): fills instanceState/instanceId.
     */
    static void applyInstanceData(data::Alarm& alarm);

    static std::unique_ptr<Calendar> currentTime();

private:
    static void setAlarmState(Context& context, data::Alarminstance& instance, int state);
    static void setSilentState(Context& context, data::Alarminstance& instance);
    static void setLowNotificationState(Context& context, data::Alarminstance& instance);
    static void setHideNotificationState(Context& context, data::Alarminstance& instance);
    static void setHighNotificationState(Context& context, data::Alarminstance& instance);
    static void setFiredState(Context& context, data::Alarminstance& instance);
    static void setMissedState(Context& context, data::Alarminstance& instance);
    static void setDismissState(Context& context, data::Alarminstance& instance);
    static void updateParentAlarm(Context& context, const data::Alarminstance& instance);
    static void updateNextAlarm(Context& context);

    static void scheduleInstanceStateChange(Context& context, Calendar& time,
                                            const data::Alarminstance& instance, int newState);
    static void cancelScheduledInstanceStateChange(const data::Alarminstance& instance);

    struct ScheduledChange;
    static std::map<int64_t, ScheduledChange*>& scheduledChanges();
};

} // namespace alarms
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ALARMSTATEMANAGER_H__
