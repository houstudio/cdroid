// C++ port of AOSP DeskClock RingtonePreviewKlaxon
// (RingtonePreviewKlaxon.kt): ringtone-pick previews ring immediately at a
// fixed volume (no crescendo) and stop when the selection changes.
#ifndef __DESKCLOCK_RINGTONEPREVIEWKLAXON_H__
#define __DESKCLOCK_RINGTONEPREVIEWKLAXON_H__

#include <core/context.h>
#include <core/uri.h>

namespace cdroid {
namespace deskclock {

class RingtonePreviewKlaxon {
private:
    RingtonePreviewKlaxon() = default;

public:
    static void stop(Context& context);
    static void start(Context& context, const Uri& uri);
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_RINGTONEPREVIEWKLAXON_H__
