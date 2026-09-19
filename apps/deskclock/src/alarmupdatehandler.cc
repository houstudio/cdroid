#include <alarmupdatehandler.h>

#include <porting/cdlog.h>

#include <core/context.h>

#include <alarmstatemanager.h>
#include <alarmutils.h>
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

AlarmUpdateHandler::AlarmUpdateHandler(Context* context, ScrollHandler* scrollHandler,
                                       ViewGroup* snackbarAnchor)
    : mContext(context), mScrollHandler(scrollHandler), mSnackbarAnchor(snackbarAnchor) {
}

data::Alarminstance AlarmUpdateHandler::setupAlarmInstance(const Alarm& alarm) {
    auto now = DataModel::getDataModel().getCalendar();
    Alarminstance newInstance = alarm.createInstanceAfter(*now);
    newInstance = Alarminstance::addInstance(*prefs(*mContext), newInstance);
    // Register instance to state manager
    AlarmStateManager::registerInstance(*mContext, newInstance, true);
    return newInstance;
}

void AlarmUpdateHandler::asyncAddAlarm(const Alarm& alarm) {
    Alarm newAlarm = Alarm::addAlarm(*prefs(*mContext), alarm);

    // Be ready to scroll to this alarm on UI later.
    if (mScrollHandler != nullptr) mScrollHandler->setSmoothScrollStableId(newAlarm.id);

    if (newAlarm.enabled) {
        Alarminstance instance = setupAlarmInstance(newAlarm);
        // (popAlarmSetSnackbar stubbed — announce via log.)
        LOGI("alarm set for %lld", (long long) instance.getAlarmTime().getTimeInMillis());
    }
}

void AlarmUpdateHandler::asyncUpdateAlarm(const Alarm& alarm, bool popToast, bool minorUpdate) {
    // Update alarm
    Alarm::updateAlarm(*prefs(*mContext), alarm);
    if (minorUpdate) {
        // Just update the instances in storage (notifications stubbed).
        for (const Alarminstance& existing :
                Alarminstance::getInstancesByAlarmId(*prefs(*mContext), alarm.id)) {
            Alarminstance newInstance = existing;
            newInstance.mVibrate = alarm.vibrate;
            newInstance.mRingtone = alarm.alert;
            newInstance.mLabel = alarm.label;
            Alarminstance::updateInstance(*prefs(*mContext), newInstance);
        }
        return;
    }
    // Otherwise, this is a major update and we're going to re-create the alarm
    AlarmStateManager::deleteAllInstances(*mContext, alarm.id);

    if (alarm.enabled) {
        Alarminstance instance = setupAlarmInstance(alarm);
        if (popToast) {
            LOGI("alarm set for %lld", (long long) instance.getAlarmTime().getTimeInMillis());
        }
    }
}

void AlarmUpdateHandler::asyncDeleteAlarm(const Alarm& alarm) {
    AlarmStateManager::deleteAllInstances(*mContext, alarm.id);
    if (Alarm::deleteAlarm(*prefs(*mContext), alarm.id)) {
        mDeletedAlarm = alarm;
        mHasDeletedAlarm = true;
        showUndoBar();
    }
}

void AlarmUpdateHandler::showPredismissToast(const Alarminstance& /*instance*/) {
    // (Snackbar stub.)
}

void AlarmUpdateHandler::hideUndoBar() {
    mHasDeletedAlarm = false;
    // (SnackbarManager.dismiss stub.)
}

void AlarmUpdateHandler::showUndoBar() {
    // (Undo snackbar stub; the undo action would re-run asyncAddAlarm(mDeletedAlarm).)
}

} // namespace alarms
} // namespace deskclock
} // namespace cdroid
