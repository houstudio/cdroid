#include <alarmitemholder.h>

#include <R.h>

namespace cdroid {
namespace deskclock {
namespace alarms {

int alarmCollapsedViewType() { return ::deskclock::R::layout::alarm_time_collapsed; }
int alarmExpandedViewType() { return ::deskclock::R::layout::alarm_time_expanded; }

Object* AlarmItemHolder::ANIMATE_REPEAT_DAYS() {
    static Object sMarker;   // process-lifetime; compared by pointer
    return &sMarker;
}

} // namespace alarms
} // namespace deskclock
} // namespace cdroid
