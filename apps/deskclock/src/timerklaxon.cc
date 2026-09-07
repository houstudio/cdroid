#include <timerklaxon.h>

#include <memory>

#include <porting/cdlog.h>

#include <datamodel.h>

namespace cdroid {
namespace deskclock {
namespace timer {

using data::DataModel;

bool TimerKlaxon::sStarted = false;

void TimerKlaxon::stop(Context& /*context*/) {
    if (sStarted) {
        LOGI("TimerKlaxon.stop()");
        sStarted = false;
        // AsyncRingtonePlayer.stop() + Vibrator.cancel(): no audio/vibrator
        // backend on cdroid — DEFERRED until one lands.
    }
}

void TimerKlaxon::start(Context& context) {
    // Make sure we are stopped before starting.
    stop(context);
    LOGI("TimerKlaxon.start()");

    // Look up user-selected timer ringtone.
    if (DataModel::getDataModel().isTimerRingtoneSilent()) {
        // Special case: Silent ringtone.
        LOGI("Playing silent ringtone for timer");
    } else {
        // AsyncRingtonePlayer.play(uri, crescendoDuration): DEFERRED (no audio
        // backend); log what would ring so the settings chain stays observable.
        std::unique_ptr<Uri> uri(DataModel::getDataModel().getTimerRingtoneUri());
        LOGI("TimerKlaxon: would play ringtone %s with crescendo %lldms",
             uri ? uri->toString().c_str() : "(none)",
             (long long) DataModel::getDataModel().getTimerCrescendoDuration());
    }

    if (DataModel::getDataModel().getTimerVibrate()) {
        // Vibrator.vibrate({500, 500}, 0, USAGE_ALARM): DEFERRED.
    }
    sStarted = true;
}

} // namespace timer
} // namespace deskclock
} // namespace cdroid
