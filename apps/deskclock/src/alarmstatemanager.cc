#include <alarmstatemanager.h>

#include <porting/cdlog.h>

#include <core/app.h>
#include <core/looper.h>
#include <core/intent.h>

#include <alarmklaxon.h>
#include <datamodel.h>

namespace cdroid {
namespace deskclock {
namespace alarms {

using data::Alarm;
using data::DataModel;
using data::Alarminstance;

namespace {
std::shared_ptr<SharedPreferences> prefs(Context& context) {
    return context.getSharedPreferences("DeskClock", Context::MODE_PRIVATE);
}
} // namespace

struct AlarmStateManager::ScheduledChange {
    Handler handler; // binds the main looper
    data::Alarminstance instance;
    int newState;
};

std::map<int64_t, AlarmStateManager::ScheduledChange*>& AlarmStateManager::scheduledChanges() {
    static std::map<int64_t, ScheduledChange*> sChanges;
    return sChanges;
}

std::unique_ptr<Calendar> AlarmStateManager::currentTime() {
    return DataModel::getDataModel().getCalendar();
}

void AlarmStateManager::applyInstanceData(Alarm& alarm) {
    alarm.instanceState = 0;
    alarm.instanceId = Alarminstance::INVALID_ID;
    // Upstream joins the instances table in the alarms query.
    Context& context = App::getInstance();
    for (const Alarminstance& instance :
            Alarminstance::getInstancesByAlarmId(*prefs(context), alarm.id)) {
        alarm.instanceState = instance.mAlarmState;
        alarm.instanceId = instance.mId;
        break;
    }
}

void AlarmStateManager::scheduleInstanceStateChange(Context& context, Calendar& time,
                                                     const Alarminstance& instance,
                                                     int newState) {
    const int64_t delay = std::max((int64_t) 0,
            time.getTimeInMillis() - currentTime()->getTimeInMillis());
    LOGI("Scheduling state change %d to instance %lld in %lldms",
         newState, (long long) instance.mId, (long long) delay);

    ScheduledChange* change = new ScheduledChange();
    change->instance = instance;
    change->newState = newState;
    ScheduledChange* raw = change;
    change->handler.postDelayed([context = &context, raw]() {
        scheduledChanges().erase(raw->instance.mId);
        Context& ctx = *context;
        Alarminstance current = raw->instance;
        int state = raw->newState;
        delete raw;
        // Re-read: the instance may have been mutated/deleted since scheduling.
        if (!Alarminstance::getInstance(*prefs(ctx), current.mId, current)) {
            LOGW("Scheduled instance %lld no longer exists", (long long) current.mId);
            return;
        }
        setAlarmState(ctx, current, state);
    }, (long) delay);
    scheduledChanges()[instance.mId] = change;
}

void AlarmStateManager::cancelScheduledInstanceStateChange(const Alarminstance& instance) {
    auto it = scheduledChanges().find(instance.mId);
    if (it != scheduledChanges().end()) {
        it->second->handler.removeCallbacks(nullptr);
        delete it->second;
        scheduledChanges().erase(it);
    }
}

void AlarmStateManager::setSilentState(Context& context, Alarminstance& instance) {
    LOGI("Setting silent state to instance %lld", (long long) instance.mId);

    instance.mAlarmState = Alarminstance::SILENT_STATE;
    Alarminstance::updateInstance(*prefs(context), instance);

    // (AlarmNotifications stubbed.)
    Calendar low = instance.getLowNotificationTime();
    scheduleInstanceStateChange(context, low, instance, Alarminstance::LOW_NOTIFICATION_STATE);
}

void AlarmStateManager::setLowNotificationState(Context& context, Alarminstance& instance) {
    LOGI("Setting low notification state to instance %lld", (long long) instance.mId);

    instance.mAlarmState = Alarminstance::LOW_NOTIFICATION_STATE;
    Alarminstance::updateInstance(*prefs(context), instance);

    Calendar high = instance.getHighNotificationTime();
    scheduleInstanceStateChange(context, high, instance, Alarminstance::HIGH_NOTIFICATION_STATE);
}

void AlarmStateManager::setHideNotificationState(Context& context, Alarminstance& instance) {
    LOGI("Setting hide notification state to instance %lld", (long long) instance.mId);

    instance.mAlarmState = Alarminstance::HIDE_NOTIFICATION_STATE;
    Alarminstance::updateInstance(*prefs(context), instance);

    Calendar high = instance.getHighNotificationTime();
    scheduleInstanceStateChange(context, high, instance, Alarminstance::HIGH_NOTIFICATION_STATE);
}

void AlarmStateManager::setHighNotificationState(Context& context, Alarminstance& instance) {
    LOGI("Setting high notification state to instance %lld", (long long) instance.mId);

    instance.mAlarmState = Alarminstance::HIGH_NOTIFICATION_STATE;
    Alarminstance::updateInstance(*prefs(context), instance);

    Calendar alarmTime = instance.getAlarmTime();
    scheduleInstanceStateChange(context, alarmTime, instance, Alarminstance::FIRED_STATE);
}

void AlarmStateManager::setFiredState(Context& context, Alarminstance& instance) {
    LOGI("Setting fire state to instance %lld", (long long) instance.mId);

    instance.mAlarmState = Alarminstance::FIRED_STATE;
    Alarminstance::updateInstance(*prefs(context), instance);

    // Upstream: the FIRE broadcast starts AlarmService (klaxon) and the
    // heads-up notification's fullScreenIntent shows AlarmActivity over
    // everything. No service/notification layer on cdroid — start both here.
    AlarmKlaxon::start(context, instance);
    Intent alarmIntent;
    alarmIntent.setClassName("cdroid.deskclock", "AlarmActivity")
              .setAction(Intent::ACTION_MAIN)
              .setFlags(Intent::FLAG_ACTIVITY_NEW_TASK)
              .putExtra("alarmInstanceId", (int64_t) instance.mId);
    context.startActivity(alarmIntent);

    // if the time changed *backward* and pushed an instance from missed back to fired,
    // remove any other scheduled instances that may exist
    if (instance.mAlarmId != Alarminstance::INVALID_ID) {
        Alarminstance::deleteOtherInstances(*prefs(context), instance.mAlarmId, instance.mId);
    }

    // (Alarm timeout: upstream schedules MISSED_STATE at instance.timeout; the
    //  DataModel alarmTimeout preference is honored when set.)
    const int timeoutMinutes = DataModel::getDataModel().getAlarmTimeout();
    if (timeoutMinutes >= 0) {
        Calendar timeout = instance.getAlarmTime();
        timeout.add(Calendar::MINUTE, timeoutMinutes);
        scheduleInstanceStateChange(context, timeout, instance, Alarminstance::MISSED_STATE);
    }

    updateNextAlarm(context);
}

void AlarmStateManager::setSnoozeState(Context& context, Alarminstance& instance) {
    // (AlarmService.stopAlarm stubbed.)

    const int snoozeMinutes = DataModel::getDataModel().getSnoozeLength();
    auto newAlarmTime = Calendar::getInstance();
    newAlarmTime->add(Calendar::MINUTE, snoozeMinutes);

    LOGI("Setting snoozed state to instance %lld", (long long) instance.mId);
    instance.setAlarmTime(*newAlarmTime);
    instance.mAlarmState = Alarminstance::SNOOZE_STATE;
    Alarminstance::updateInstance(*prefs(context), instance);

    Calendar alarmTime = instance.getAlarmTime();
    scheduleInstanceStateChange(context, alarmTime, instance, Alarminstance::FIRED_STATE);

    // (Snooze toast stubbed.)

    updateNextAlarm(context);
}

void AlarmStateManager::setMissedState(Context& context, Alarminstance& instance) {
    LOGI("Setting missed state to instance %lld", (long long) instance.mId);

    if (instance.mAlarmId != Alarminstance::INVALID_ID) {
        updateParentAlarm(context, instance);
    }

    instance.mAlarmState = Alarminstance::MISSED_STATE;
    Alarminstance::updateInstance(*prefs(context), instance);

    Calendar ttl = instance.getMissedTimeToLive();
    scheduleInstanceStateChange(context, ttl, instance, Alarminstance::DISMISSED_STATE);

    updateNextAlarm(context);
}

void AlarmStateManager::setPreDismissState(Context& context, Alarminstance& instance) {
    LOGI("Setting predismissed state to instance %lld", (long long) instance.mId);

    instance.mAlarmState = Alarminstance::PREDISMISSED_STATE;
    Alarminstance::updateInstance(*prefs(context), instance);

    Calendar alarmTime = instance.getAlarmTime();
    scheduleInstanceStateChange(context, alarmTime, instance, Alarminstance::DISMISSED_STATE);

    if (instance.mAlarmId != Alarminstance::INVALID_ID) {
        updateParentAlarm(context, instance);
    }

    updateNextAlarm(context);
}

void AlarmStateManager::setDismissState(Context& context, Alarminstance& instance) {
    LOGI("Setting dismissed state to instance %lld", (long long) instance.mId);
    instance.mAlarmState = Alarminstance::DISMISSED_STATE;
    Alarminstance::updateInstance(*prefs(context), instance);
}

void AlarmStateManager::updateParentAlarm(Context& context, const Alarminstance& instance) {
    Alarm alarm;
    if (!Alarm::getAlarm(*prefs(context), instance.mAlarmId, alarm)) {
        LOGE("Parent has been deleted with instance: %lld", (long long) instance.mId);
        return;
    }

    if (!alarm.getWeekdays().isRepeating()) {
        if (alarm.deleteAfterUse) {
            LOGI("Deleting parent alarm: %lld", (long long) alarm.id);
            Alarm::deleteAlarm(*prefs(context), alarm.id);
        } else {
            LOGI("Disabling parent alarm: %lld", (long long) alarm.id);
            alarm.enabled = false;
            Alarm::updateAlarm(*prefs(context), alarm);
        }
    } else {
        // Schedule the next repeating instance.
        auto now = currentTime();
        Alarminstance nextRepeatedInstance = alarm.createInstanceAfter(*now);
        if (instance.mAlarmState > Alarminstance::FIRED_STATE) {
            Calendar instanceTimeCal = instance.getAlarmTime();
            Alarminstance next = alarm.createInstanceAfter(instanceTimeCal);
            if (next.getAlarmTime().getTimeInMillis()
                    == nextRepeatedInstance.getAlarmTime().getTimeInMillis()) {
                nextRepeatedInstance = next;
            }
        }

        LOGI("Creating new instance for repeating alarm %lld", (long long) alarm.id);
        Alarminstance added = Alarminstance::addInstance(*prefs(context), nextRepeatedInstance);
        registerInstance(context, added, true);
    }
}

void AlarmStateManager::deleteInstanceAndUpdateParent(Context& context,
                                                      Alarminstance& instance) {
    LOGI("Deleting instance %lld and updating parent alarm.", (long long) instance.mId);

    unregisterInstance(context, instance);

    if (instance.mAlarmId != Alarminstance::INVALID_ID) {
        updateParentAlarm(context, instance);
    }

    Alarminstance::deleteInstance(*prefs(context), instance.mId);

    updateNextAlarm(context);
}

void AlarmStateManager::unregisterInstance(Context& context, Alarminstance& instance) {
    LOGI("Unregistering instance %lld", (long long) instance.mId);
    cancelScheduledInstanceStateChange(instance);
    setDismissState(context, instance);
}

void AlarmStateManager::registerInstance(Context& context, Alarminstance& instance,
                                         bool updateNextAlarmFlag) {
    LOGI("Registering instance: %lld", (long long) instance.mId);
    auto currentTimeCal = currentTime();
    Calendar alarmTime = instance.getAlarmTime();
    Calendar lowNotificationTime = instance.getLowNotificationTime();
    Calendar highNotificationTime = instance.getHighNotificationTime();
    Calendar missedTTL = instance.getMissedTimeToLive();

    // Handle special use cases here
    if (instance.mAlarmState == Alarminstance::DISMISSED_STATE) {
        // This should never happen, but add a quick check here
        LOGE("Alarm Instance is dismissed, but never deleted");
        deleteInstanceAndUpdateParent(context, instance);
        return;
    } else if (instance.mAlarmState == Alarminstance::FIRED_STATE) {
        const int timeoutMinutes = DataModel::getDataModel().getAlarmTimeout();
        if (timeoutMinutes < 0) {
            setFiredState(context, instance);
            return;
        }
        Calendar timeout = alarmTime;
        timeout.add(Calendar::MINUTE, timeoutMinutes);
        if (currentTimeCal->getTimeInMillis() <= timeout.getTimeInMillis()) {
            setFiredState(context, instance);
            return;
        }
    } else if (instance.mAlarmState == Alarminstance::MISSED_STATE) {
        if (currentTimeCal->getTimeInMillis() < alarmTime.getTimeInMillis()) {
            if (instance.mAlarmId == Alarminstance::INVALID_ID) {
                LOGI("Cannot restore missed instance for one-time alarm");
                deleteInstanceAndUpdateParent(context, instance);
                return;
            }
            // Make sure we re-enable the parent alarm of the instance.
            Alarm alarm;
            if (Alarm::getAlarm(*prefs(context), instance.mAlarmId, alarm)) {
                alarm.enabled = true;
                Alarm::updateAlarm(*prefs(context), alarm);
            }
        }
    } else if (instance.mAlarmState == Alarminstance::PREDISMISSED_STATE) {
        if (currentTimeCal->getTimeInMillis() < alarmTime.getTimeInMillis()) {
            setPreDismissState(context, instance);
        } else {
            deleteInstanceAndUpdateParent(context, instance);
        }
        return;
    }

    // Fix states that are time sensitive
    if (currentTimeCal->getTimeInMillis() > missedTTL.getTimeInMillis()) {
        // Alarm is so old, just dismiss it
        deleteInstanceAndUpdateParent(context, instance);
    } else if (currentTimeCal->getTimeInMillis() > alarmTime.getTimeInMillis()) {
        // There is a chance that the TIME_SET occurred right when the alarm should go off,
        // so we need to add a check to see if we should fire the alarm instead of marking
        // it missed.
        Calendar alarmBuffer = alarmTime;
        alarmBuffer.add(Calendar::SECOND, ALARM_FIRE_BUFFER);
        if (currentTimeCal->getTimeInMillis() < alarmBuffer.getTimeInMillis()) {
            setFiredState(context, instance);
        } else {
            setMissedState(context, instance);
        }
    } else if (instance.mAlarmState == Alarminstance::SNOOZE_STATE) {
        Calendar snoozeTime = instance.getAlarmTime();
        scheduleInstanceStateChange(context, snoozeTime, instance, Alarminstance::FIRED_STATE);
    } else if (currentTimeCal->getTimeInMillis() > highNotificationTime.getTimeInMillis()) {
        setHighNotificationState(context, instance);
    } else if (currentTimeCal->getTimeInMillis() > lowNotificationTime.getTimeInMillis()) {
        if (instance.mAlarmState == Alarminstance::HIDE_NOTIFICATION_STATE) {
            setHideNotificationState(context, instance);
        } else {
            setLowNotificationState(context, instance);
        }
    } else {
        // Alarm is still active, so initialize as a silent alarm
        setSilentState(context, instance);
    }

    if (updateNextAlarmFlag) {
        updateNextAlarm(context);
    }
}

void AlarmStateManager::deleteAllInstances(Context& context, int64_t alarmId) {
    LOGI("Deleting all instances of alarm: %lld", (long long) alarmId);
    for (const Alarminstance& instance :
            Alarminstance::getInstancesByAlarmId(*prefs(context), alarmId)) {
        Alarminstance copy = instance;
        unregisterInstance(context, copy);
        Alarminstance::deleteInstance(*prefs(context), instance.mId);
    }
    updateNextAlarm(context);
}

void AlarmStateManager::deleteNonSnoozeInstances(Context& context, int64_t alarmId) {
    LOGI("Deleting all non-snooze instances of alarm: %lld", (long long) alarmId);
    for (const Alarminstance& instance :
            Alarminstance::getInstancesByAlarmId(*prefs(context), alarmId)) {
        if (instance.mAlarmState == Alarminstance::SNOOZE_STATE) {
            continue;
        }
        Alarminstance copy = instance;
        unregisterInstance(context, copy);
        Alarminstance::deleteInstance(*prefs(context), instance.mId);
    }
    updateNextAlarm(context);
}

bool AlarmStateManager::getNextFiringAlarm(Context& context, Alarminstance& outInstance) {
    bool found = false;
    int64_t best = INT64_MAX;
    for (const Alarminstance& instance : Alarminstance::getInstances(*prefs(context))) {
        if (instance.mAlarmState < Alarminstance::FIRED_STATE) {
            const int64_t t = instance.getAlarmTime().getTimeInMillis();
            if (t < best) {
                best = t;
                outInstance = instance;
                found = true;
            }
        }
    }
    return found;
}

void AlarmStateManager::updateNextAlarm(Context& context) {
    // Upstream mirrors the next alarm into the framework AlarmManager/AlarmClockInfo
    // (status-bar indicator); cdroid has no such system face — the clock tab reads
    // the next alarm from the DataModel instead. Logged for parity.
    Alarminstance next;
    if (getNextFiringAlarm(context, next)) {
        LOGI("Next alarm set for instance %lld", (long long) next.mId);
    } else {
        LOGI("Canceling upcoming alarm");
    }
}

void AlarmStateManager::setAlarmState(Context& context, Alarminstance& instance, int state) {
    switch (state) {
        case Alarminstance::SILENT_STATE: setSilentState(context, instance); break;
        case Alarminstance::LOW_NOTIFICATION_STATE:
            setLowNotificationState(context, instance);
            break;
        case Alarminstance::HIDE_NOTIFICATION_STATE:
            setHideNotificationState(context, instance);
            break;
        case Alarminstance::HIGH_NOTIFICATION_STATE:
            setHighNotificationState(context, instance);
            break;
        case Alarminstance::FIRED_STATE: setFiredState(context, instance); break;
        case Alarminstance::SNOOZE_STATE: setSnoozeState(context, instance); break;
        case Alarminstance::MISSED_STATE: setMissedState(context, instance); break;
        case Alarminstance::PREDISMISSED_STATE: setPreDismissState(context, instance); break;
        case Alarminstance::DISMISSED_STATE:
            deleteInstanceAndUpdateParent(context, instance);
            break;
        default: LOGE("Trying to change to unknown alarm state: %d", state); break;
    }
}

void AlarmStateManager::fixAlarmInstances(Context& context) {
    LOGI("Fixing alarm instances");
    auto currentTimeCal = currentTime();
    std::vector<Alarminstance> instances = Alarminstance::getInstances(*prefs(context));
    // Reverse chronological order (see upstream).
    std::sort(instances.begin(), instances.end(),
              [](const Alarminstance& lhs, const Alarminstance& rhs) {
        return rhs.getAlarmTime().getTimeInMillis() < lhs.getAlarmTime().getTimeInMillis();
    });

    for (Alarminstance& instance : instances) {
        Alarm alarm;
        if (!Alarm::getAlarm(*prefs(context), instance.mAlarmId, alarm)) {
            unregisterInstance(context, instance);
            Alarminstance::deleteInstance(*prefs(context), instance.mId);
            LOGE("Found instance without matching alarm; deleting instance");
            continue;
        }
        Calendar instanceAlarmTime = instance.getAlarmTime();
        auto priorAlarmTime = alarm.getPreviousAlarmTime(instanceAlarmTime);
        Calendar missedTTLTime = instance.getMissedTimeToLive();
        const bool before = priorAlarmTime != nullptr
                && currentTimeCal->getTimeInMillis() < priorAlarmTime->getTimeInMillis();
        if (before || currentTimeCal->getTimeInMillis() > missedTTLTime.getTimeInMillis()) {
            // The time change is so dramatic the AlarmInstance doesn't make any sense;
            // remove it and schedule the new appropriate instance.
            deleteInstanceAndUpdateParent(context, instance);
        } else {
            registerInstance(context, instance, false /* updateNextAlarm */);
        }
    }

    updateNextAlarm(context);
}

} // namespace alarms
} // namespace deskclock
} // namespace cdroid
