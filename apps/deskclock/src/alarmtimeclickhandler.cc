#include <alarmtimeclickhandler.h>

#include <porting/cdlog.h>

#include <core/context.h>
#include <fragment/fragment.h>

#include <alarmclockfragment.h>
#include <alarmitemholder.h>
#include <alarmstatemanager.h>
#include <ringtonepickeractivity.h>
#include <datamodel.h>
#include <labeldialogfragment.h>
#include <timepickerdialogfragment.h>
#include <utils.h>

namespace cdroid {
namespace deskclock {
namespace alarms {

using data::Alarm;
using data::Alarminstance;

AlarmTimeClickHandler::AlarmTimeClickHandler(Fragment* fragment, Bundle* savedState,
                                             AlarmUpdateHandler* alarmUpdateHandler,
                                             ScrollHandler* scrollHandler)
    : mFragment(fragment), mAlarmUpdateHandler(alarmUpdateHandler),
      mScrollHandler(scrollHandler) {
    Context* context = fragment->getContext();
    mContext = context;
    (void) savedState; // the previous-days map is process-lifetime on cdroid
}

void AlarmTimeClickHandler::setSelectedAlarm(const Alarm* selectedAlarm) {
    if (selectedAlarm != nullptr) {
        mSelectedAlarm = *selectedAlarm;
        mHasSelectedAlarm = true;
    } else {
        mHasSelectedAlarm = false;
    }
}

void AlarmTimeClickHandler::saveInstance(Bundle& /*outState*/) {
    // (the previous-days map is process-lifetime on cdroid)
}

void AlarmTimeClickHandler::setAlarmEnabled(const Alarm& alarm, bool newState) {
    if (newState != alarm.enabled) {
        Alarm mutableAlarm = alarm;
        mutableAlarm.enabled = newState;
        mAlarmUpdateHandler->asyncUpdateAlarm(mutableAlarm, newState, false);
    }
}

void AlarmTimeClickHandler::setAlarmVibrationEnabled(const Alarm& alarm, bool newState) {
    if (newState != alarm.vibrate) {
        Alarm mutableAlarm = alarm;
        mutableAlarm.vibrate = newState;
        mAlarmUpdateHandler->asyncUpdateAlarm(mutableAlarm, false, true);
        // (vibration preview stubbed)
    }
}

void AlarmTimeClickHandler::setAlarmRepeatEnabled(const Alarm& alarm, bool isEnabled) {
    auto now = data::DataModel::getDataModel().getCalendar();
    Alarm mutableAlarm = alarm;
    const int64_t oldNextAlarmTime =
            mutableAlarm.getNextAlarmTime(*now).getTimeInMillis();
    const std::string alarmId = std::to_string(mutableAlarm.id);
    if (isEnabled) {
        // Set all previously set days or all days if no previous.
        auto it = mPreviousDaysOfWeekMap.find(mutableAlarm.id);
        const int bitSet = (it != mPreviousDaysOfWeekMap.end()) ? it->second : 0;
        mutableAlarm.daysOfWeek = bitSet;
        if (!data::Weekdays::fromBits(mutableAlarm.daysOfWeek).isRepeating()) {
            mutableAlarm.daysOfWeek = data::Weekdays::fromBits(0x7F).bits; // Weekdays.ALL
        }
    } else {
        // Remember the set days in case the user wants it back.
        mPreviousDaysOfWeekMap[mutableAlarm.id] = mutableAlarm.daysOfWeek;

        // Remove all repeat days
        mutableAlarm.daysOfWeek = 0;
    }

    const int64_t newNextAlarmTime =
            mutableAlarm.getNextAlarmTime(*now).getTimeInMillis();
    const bool popupToast = oldNextAlarmTime != newNextAlarmTime;
    mAlarmUpdateHandler->asyncUpdateAlarm(mutableAlarm, popupToast, false);
}

void AlarmTimeClickHandler::setDayOfWeekEnabled(const Alarm& alarm, bool checked, int index) {
    auto now = data::DataModel::getDataModel().getCalendar();
    Alarm mutableAlarm = alarm;
    const int64_t oldNextAlarmTime =
            mutableAlarm.getNextAlarmTime(*now).getTimeInMillis();

    const std::vector<int>& calendarDays = data::Weekdays::Order::calendarDays(
            data::DataModel::getDataModel().getWeekdayOrder());
    const int weekday = calendarDays[index];
    mutableAlarm.setWeekdays(mutableAlarm.getWeekdays().setBit(weekday, checked));

    const int64_t newNextAlarmTime =
            mutableAlarm.getNextAlarmTime(*now).getTimeInMillis();
    const bool popupToast = oldNextAlarmTime != newNextAlarmTime;
    mAlarmUpdateHandler->asyncUpdateAlarm(mutableAlarm, popupToast, false);
}

void AlarmTimeClickHandler::onDeleteClicked(AlarmItemHolder* itemHolder) {
    AlarmClockFragment* fragment = dynamic_cast<AlarmClockFragment*>(mFragment);
    if (fragment != nullptr) {
        fragment->removeItem(itemHolder);
    }
    const Alarm& alarm = itemHolder->item;
    mAlarmUpdateHandler->asyncDeleteAlarm(alarm);
}

void AlarmTimeClickHandler::onClockClicked(const Alarm& alarm) {
    mSelectedAlarm = alarm;
    mHasSelectedAlarm = true;
    TimePickerDialogFragment::show(mFragment, alarm.hour, alarm.minutes);
}

void AlarmTimeClickHandler::dismissAlarmInstance(const Alarminstance& alarmInstance) {
    // Upstream posts a predismiss intent through AlarmService; apply synchronously.
    Alarminstance instance = alarmInstance;
    AlarmStateManager::setPreDismissState(*mContext, instance);
    mAlarmUpdateHandler->showPredismissToast(instance);
}

void AlarmTimeClickHandler::onRingtoneClicked(Context& context, const Alarm& alarm) {
    mSelectedAlarm = alarm;
    mHasSelectedAlarm = true;
    // Upstream: startActivity(RingtonePickerActivity.createAlarmRingtonePickerIntent(...)).
    context.startActivity(ringtone::RingtonePickerActivity::createAlarmRingtonePickerIntent(
            context, alarm));
}

void AlarmTimeClickHandler::onEditLabelClicked(const Alarm& alarm) {
    // Upstream: activity (AlarmClockFragment) is the AlarmLabelDialogHandler.
    AlarmClockFragment* fragment = dynamic_cast<AlarmClockFragment*>(mFragment);
    LabelDialogFragment::OnAlarmLabelSet handler;
    if (fragment != nullptr) {
        handler = [fragment](const data::Alarm& a, const std::string& label) {
            fragment->setLabel(a, label);
        };
    }
    LabelDialogFragment::showDialog(mFragment->getParentFragmentManager(),
            LabelDialogFragment::newInstance(alarm, alarm.label, handler));
}

void AlarmTimeClickHandler::onTimeSet(int hourOfDay, int minute) {
    if (!mHasSelectedAlarm) {
        // If no selected alarm then we're creating a new alarm.
        Alarm a;
        a.hour = hourOfDay;
        a.minutes = minute;
        a.enabled = true;
        mAlarmUpdateHandler->asyncAddAlarm(a);
    } else {
        mSelectedAlarm.hour = hourOfDay;
        mSelectedAlarm.minutes = minute;
        mSelectedAlarm.enabled = true;
        if (mScrollHandler != nullptr) mScrollHandler->setSmoothScrollStableId(mSelectedAlarm.id);
        mAlarmUpdateHandler->asyncUpdateAlarm(mSelectedAlarm, true, false);
        mHasSelectedAlarm = false;
    }
}

} // namespace alarms
} // namespace deskclock
} // namespace cdroid
