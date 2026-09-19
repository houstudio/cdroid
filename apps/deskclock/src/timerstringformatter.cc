#include <timerstringformatter.h>

#include <R.h>

#include <core/context.h>
#include <content/resources.h>

#include <utils.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace data {

static constexpr int64_t SECOND_IN_MILLIS = 1000;
static constexpr int64_t MINUTE_IN_MILLIS = 60 * SECOND_IN_MILLIS;
static constexpr int64_t HOUR_IN_MILLIS = 60 * MINUTE_IN_MILLIS;

/** java String.format substitute for the %n$s positional placeholders these resources use. */
static std::string formatPositional(const std::string& pattern,
                                    const std::vector<std::string>& args) {
    std::string out;
    for (size_t i = 0; i < pattern.size(); i++) {
        if (pattern[i] == '%' && i + 1 < pattern.size() && pattern[i + 1] >= '1'
                && pattern[i + 1] <= '9') {
            const size_t arg = (size_t) (pattern[i + 1] - '1');
            if (i + 2 < pattern.size() && pattern[i + 2] == '$') {
                if (arg < args.size()) out += args[arg];
                i += 3; // skip %, digit, $
                // Optional type char (e.g. the 's' of "%1$s").
                if (i < pattern.size()) i++;
                i--; // the loop ++ lands back on the char after the spec
                continue;
            }
        }
        out += pattern[i];
    }
    return out;
}

std::string TimerStringFormatter::formatTimeRemaining(Context& context, int64_t remainingTime,
                                                      bool shouldShowSeconds) {
    int roundedHours = (int) (remainingTime / HOUR_IN_MILLIS);
    int roundedMinutes = (int) (remainingTime / MINUTE_IN_MILLIS % 60);
    int roundedSeconds = (int) (remainingTime / SECOND_IN_MILLIS % 60);

    int seconds;
    int minutes;
    int hours;
    if (remainingTime % SECOND_IN_MILLIS != 0 && shouldShowSeconds) {
        // Add 1 because there's a partial second.
        roundedSeconds += 1;
        if (roundedSeconds == 60) {
            // Wind back and fix the hours and minutes as needed.
            seconds = 0;
            roundedMinutes += 1;
            if (roundedMinutes == 60) {
                minutes = 0;
                roundedHours += 1;
                hours = roundedHours;
            } else {
                minutes = roundedMinutes;
                hours = roundedHours;
            }
        } else {
            seconds = roundedSeconds;
            minutes = roundedMinutes;
            hours = roundedHours;
        }
    } else {
        // Already perfect precision, or we don't want to consider seconds at all.
        seconds = roundedSeconds;
        minutes = roundedMinutes;
        hours = roundedHours;
    }

    Resources& r = context.getResources();
    const std::string minSeq = Utils::getNumberFormattedQuantityString(
            context, R::plurals::minutes, minutes);
    const std::string hourSeq = Utils::getNumberFormattedQuantityString(
            context, R::plurals::hours, hours);
    const std::string secSeq = Utils::getNumberFormattedQuantityString(
            context, R::plurals::seconds, seconds);

    // The verb "remaining" may have to change tense for singular subjects in some languages.
    const std::string remainingSuffix = r.getString(
            (minutes > 1 || hours > 1 || seconds > 1) ? R::string::timer_remaining_multiple
                                                      : R::string::timer_remaining_single);

    const bool showHours = hours > 0;
    const bool showMinutes = minutes > 0;
    const bool showSeconds = seconds > 0 && shouldShowSeconds;

    int formatStringId = -1;
    if (showHours) {
        if (showMinutes) {
            formatStringId = showSeconds ? R::string::timer_notifications_hours_minutes_seconds
                                         : R::string::timer_notifications_hours_minutes;
        } else if (showSeconds) {
            formatStringId = R::string::timer_notifications_hours_seconds;
        } else {
            formatStringId = R::string::timer_notifications_hours;
        }
    } else if (showMinutes) {
        formatStringId = showSeconds ? R::string::timer_notifications_minutes_seconds
                                     : R::string::timer_notifications_minutes;
    } else if (showSeconds) {
        formatStringId = R::string::timer_notifications_seconds;
    } else if (!shouldShowSeconds) {
        formatStringId = R::string::timer_notifications_less_min;
    }

    if (formatStringId == -1) {
        return std::string();
    }
    return formatPositional(r.getString(formatStringId),
            {hourSeq, minSeq, remainingSuffix, secSeq});
}

std::string TimerStringFormatter::formatString(Context& context, int stringResId,
                                               int64_t currentTime, bool shouldShowSeconds) {
    return formatPositional(context.getResources().getString(stringResId),
            {formatTimeRemaining(context, currentTime, shouldShowSeconds)});
}

} // namespace data
} // namespace deskclock
} // namespace cdroid
