#ifndef __DESKCLOCK_ALARMUTILS_H__
#define __DESKCLOCK_ALARMUTILS_H__
/*********************************************************************************
 * Port of com.android.deskclock.AlarmUtils — static utility methods for Alarms.
 *********************************************************************************/
#include <string>

#include <core/calendar.h>

#include <alarminstance.h>

namespace cdroid {
class Context;

namespace deskclock {

class AlarmUtils {
public:
    /** Formats "Tue 8:00 AM"-style strings (EHm/Ehma skeleton approximation). */
    static std::string getFormattedTime(Context& context, Calendar& time);
    static std::string getFormattedTime(Context& context, int64_t timeInMillis);

    static std::string getAlarmText(Context& context, const data::Alarminstance& instance,
                                    bool includeLabel);

    /** format "Alarm set for 2 days, 7 hours, and 53 minutes from now." */
    static std::string formatElapsedTimeUntilAlarm(Context& context, int64_t delta);
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ALARMUTILS_H__
