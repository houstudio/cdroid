// C++ port of AOSP DeskClock AlarmKlaxon — see alarmklaxon.h.
#include "alarmklaxon.h"

#include <porting/cdlog.h>
#include <core/app.h>

#include "alarminstance.h"
#include "asyncringtoneplayer.h"
#include "datamodel.h"

namespace cdroid {
namespace deskclock {
namespace alarms {

using data::DataModel;

namespace {
// AOSP holds these on the object singleton; process-scoped and never freed
// (the player's thread is the AOSP "ringtone-player" HandlerThread).
bool sStarted = false;
AsyncRingtonePlayer* sAsyncRingtonePlayer = nullptr;

AsyncRingtonePlayer* getAsyncRingtonePlayer(Context& context) {
    if (sAsyncRingtonePlayer == nullptr) {
        sAsyncRingtonePlayer = new AsyncRingtonePlayer(&App::getInstance());
    }
    return sAsyncRingtonePlayer;
}
}

void AlarmKlaxon::stop(Context& context) {
    if (sStarted) {
        LOGV("AlarmKlaxon.stop()");
        sStarted = false;
        getAsyncRingtonePlayer(context)->stop();
        // Vibrator.cancel(): no vibrator backend on cdroid — DEFERRED.
    }
}

void AlarmKlaxon::start(Context& context, data::Alarminstance& instance) {
    // Make sure we are stopped before starting.
    stop(context);
    LOGV("AlarmKlaxon.start()");

    if (!instance.mRingtone.empty()) {   // !NO_RINGTONE_URI.equals(mRingtone)
        const int64_t crescendoDuration = DataModel::getDataModel().getAlarmCrescendoDuration();
        std::unique_ptr<Uri> uri(Uri::parse(instance.mRingtone));
        getAsyncRingtonePlayer(context)->play(uri.get(), crescendoDuration);
    }

    if (instance.mVibrate) {
        // Vibrator.vibrate({500, 500}, 0, USAGE_ALARM): DEFERRED (no backend).
    }
    sStarted = true;
}

} // namespace alarms
} // namespace deskclock
} // namespace cdroid
