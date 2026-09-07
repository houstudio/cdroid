#ifndef __DESKCLOCK_TIMERKLAXON_H__
#define __DESKCLOCK_TIMERKLAXON_H__
/*********************************************************************************
 * Port of com.android.deskclock.timer.TimerKlaxon — plays the timer ringtone
 * and vibrates while expired timers exist. DEFERRED: cdroid has no audio or
 * vibrator backend yet, so both entry points are logging stubs that keep the
 * started flag; the ringtone/vibrate lookups stay wired so the seam is honest
 * when the backend lands.
 *********************************************************************************/
#include <string>

namespace cdroid {

class Context;

namespace deskclock {
namespace timer {

class TimerKlaxon {
private:
    static bool sStarted;

public:
    /** Make sure we are stopped before starting, then ring/vibrate (stubbed). */
    static void start(Context& context);

    /** Stop the ringtone and vibration if any were started. */
    static void stop(Context& context);
};

} // namespace timer
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_TIMERKLAXON_H__
