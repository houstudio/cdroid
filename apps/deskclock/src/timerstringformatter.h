#ifndef __DESKCLOCK_TIMERSTRINGFORMATTER_H__
#define __DESKCLOCK_TIMERSTRINGFORMATTER_H__
/*********************************************************************************
 * Port of com.android.deskclock.data.TimerStringFormatter — formats
 * "N hours M minutes remaining" style strings for accessibility and the
 * (stubbed) notifications.
 *********************************************************************************/
#include <cstdint>
#include <string>

namespace cdroid {
class Context;

namespace deskclock {
namespace data {

class TimerStringFormatter {
public:
    /**
     * Format "7 hours 52 minutes 14 seconds remaining".
     * @return the formatted string; empty when there is nothing to show
     *         (upstream returns null in that case).
     */
    static std::string formatTimeRemaining(Context& context, int64_t remainingTime,
                                           bool shouldShowSeconds);

    static std::string formatString(Context& context, int stringResId, int64_t currentTime,
                                    bool shouldShowSeconds);
};

} // namespace data
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_TIMERSTRINGFORMATTER_H__
