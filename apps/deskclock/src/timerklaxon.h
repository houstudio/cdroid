#ifndef __DESKCLOCK_TIMERKLAXON_H__
#define __DESKCLOCK_TIMERKLAXON_H__
/*********************************************************************************
 * Port of com.android.deskclock.timer.TimerKlaxon — plays the timer ringtone
 * and vibrates while expired timers exist. Ringtone playback goes through the
 * AsyncRingtonePlayer port (FFmpeg+RtAudio behind DESKCLOCK_ENABLE_FFMPEG,
 * logging-stub otherwise); the vibrator stays a logged DEFERRED (no backend).
 *********************************************************************************/
#include <string>

namespace cdroid {

class Context;

namespace deskclock {

class AsyncRingtonePlayer;

namespace timer {

class TimerKlaxon {
private:
    static bool sStarted;
    static AsyncRingtonePlayer* sAsyncRingtonePlayer;   // process-scoped (AOSP)

    static AsyncRingtonePlayer* getAsyncRingtonePlayer(Context& context);

public:
    /** Make sure we are stopped before starting, then ring/vibrate. */
    static void start(Context& context);

    /** Stop the ringtone and vibration if any were started. */
    static void stop(Context& context);
};

} // namespace timer
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_TIMERKLAXON_H__
