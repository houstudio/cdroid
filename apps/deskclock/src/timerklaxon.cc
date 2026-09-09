#include <timerklaxon.h>

#include <memory>

#include <porting/cdlog.h>
#include <core/app.h>

#include <asyncringtoneplayer.h>
#include <datamodel.h>

namespace cdroid {
namespace deskclock {
namespace timer {

using data::DataModel;

bool TimerKlaxon::sStarted = false;
AsyncRingtonePlayer* TimerKlaxon::sAsyncRingtonePlayer = nullptr;

AsyncRingtonePlayer* TimerKlaxon::getAsyncRingtonePlayer(Context& /*context*/) {
    // AOSP binds the application context; the App singleton is cdroid's.
    if (sAsyncRingtonePlayer == nullptr) {
        sAsyncRingtonePlayer = new AsyncRingtonePlayer(&App::getInstance());
    }
    return sAsyncRingtonePlayer;
}

void TimerKlaxon::stop(Context& context) {
    if (sStarted) {
        LOGI("TimerKlaxon.stop()");
        sStarted = false;
        getAsyncRingtonePlayer(context)->stop();
        // Vibrator.cancel(): no vibrator backend on cdroid — DEFERRED.
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
        std::unique_ptr<Uri> uri(DataModel::getDataModel().getTimerRingtoneUri());
        const int64_t crescendoDuration = DataModel::getDataModel().getTimerCrescendoDuration();
        getAsyncRingtonePlayer(context)->play(uri.get(), crescendoDuration);
    }

    if (DataModel::getDataModel().getTimerVibrate()) {
        // Vibrator.vibrate({500, 500}, 0, USAGE_ALARM): DEFERRED.
    }
    sStarted = true;
}

} // namespace timer
} // namespace deskclock
} // namespace cdroid
