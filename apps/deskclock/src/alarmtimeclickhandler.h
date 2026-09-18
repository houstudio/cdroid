#ifndef __DESKCLOCK_ALARMTIMECLICKHANDLER_H__
#define __DESKCLOCK_ALARMTIMECLICKHANDLER_H__
/*********************************************************************************
 * Port of com.android.deskclock.alarms.AlarmTimeClickHandler — click handler
 * for an alarm time item.
 *********************************************************************************/
#include <core/bundle.h>

#include <alarm.h>
#include <alarmupdatehandler.h>
#include <alarminstance.h>

namespace cdroid {
class Context;
class Fragment;


namespace deskclock {
class AlarmClockFragment;

namespace alarms {

class AlarmItemHolder;

class AlarmTimeClickHandler {
private:
    Fragment* mFragment;
    Context* mContext;
    AlarmUpdateHandler* mAlarmUpdateHandler;
    ScrollHandler* mScrollHandler;

    data::Alarm mSelectedAlarm;
    bool mHasSelectedAlarm = false;

    /** alarmId -> previously-set repeat bits (upstream Bundle map). */
    std::map<int64_t, int> mPreviousDaysOfWeekMap;

public:
    AlarmTimeClickHandler(Fragment* fragment, Bundle* savedState,
                          AlarmUpdateHandler* alarmUpdateHandler, ScrollHandler* scrollHandler);

    void setSelectedAlarm(const data::Alarm* selectedAlarm);
    void saveInstance(Bundle& outState);

    void setAlarmEnabled(const data::Alarm& alarm, bool newState);
    void setAlarmVibrationEnabled(const data::Alarm& alarm, bool newState);
    void setAlarmRepeatEnabled(const data::Alarm& alarm, bool isEnabled);
    void setDayOfWeekEnabled(const data::Alarm& alarm, bool checked, int index);
    void onDeleteClicked(AlarmItemHolder* itemHolder);
    void onClockClicked(const data::Alarm& alarm);
    void dismissAlarmInstance(const data::Alarminstance& alarmInstance);
    void onRingtoneClicked(Context& context, const data::Alarm& alarm);
    void onEditLabelClicked(const data::Alarm& alarm);
    void onTimeSet(int hourOfDay, int minute);
};

} // namespace alarms
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ALARMTIMECLICKHANDLER_H__
