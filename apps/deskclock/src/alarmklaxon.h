// C++ port of AOSP DeskClock AlarmKlaxon (alarms/AlarmKlaxon.kt). Manages
// playing alarm ringtones (and, upstream, vibrating the device — no vibrator
// backend on cdroid, kept as a logged DEFERRED like TimerKlaxon).
#ifndef __DESKCLOCK_ALARMKLAXON_H__
#define __DESKCLOCK_ALARMKLAXON_H__

#include <core/context.h>

namespace cdroid {
namespace deskclock {
namespace data {
class Alarminstance;
}

namespace alarms {

class AlarmKlaxon {
private:
    AlarmKlaxon() = default;

public:
    static void stop(Context& context);
    static void start(Context& context, data::Alarminstance& instance);
};

} // namespace alarms
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ALARMKLAXON_H__
