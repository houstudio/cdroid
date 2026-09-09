// C++ port of AOSP DeskClock RingtonePreviewKlaxon — see ringtonepreviewklaxon.h.
#include "ringtonepreviewklaxon.h"

#include <porting/cdlog.h>
#include <core/app.h>

#include "asyncringtoneplayer.h"

namespace cdroid {
namespace deskclock {

namespace {
AsyncRingtonePlayer* sAsyncRingtonePlayer = nullptr;   // process-scoped (AOSP)
AsyncRingtonePlayer* getAsyncRingtonePlayer(Context& context) {
    if (sAsyncRingtonePlayer == nullptr) {
        sAsyncRingtonePlayer = new AsyncRingtonePlayer(&App::getInstance());
    }
    return sAsyncRingtonePlayer;
}
}

void RingtonePreviewKlaxon::stop(Context& context) {
    LOGI("RingtonePreviewKlaxon.stop()");
    getAsyncRingtonePlayer(context)->stop();
}

void RingtonePreviewKlaxon::start(Context& context, const Uri& uri) {
    stop(context);
    LOGI("RingtonePreviewKlaxon.start()");
    getAsyncRingtonePlayer(context)->play(const_cast<Uri*>(&uri), 0);
}

} // namespace deskclock
} // namespace cdroid
