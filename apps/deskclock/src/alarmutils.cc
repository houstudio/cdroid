#include <alarmutils.h>

#include <R.h>

#include <core/context.h>
#include <content/resources.h>

#include <datamodel.h>
#include <utils.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {

namespace {
constexpr int64_t MINUTE_IN_MILLIS = 60 * 1000;
constexpr int64_t HOUR_IN_MILLIS = 60 * MINUTE_IN_MILLIS;

const char* SHORT_WEEKDAYS[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
} // namespace

std::string AlarmUtils::getFormattedTime(Context& context, Calendar& time) {
    const bool is24 = data::DataModel::getDataModel().is24HourFormat();
    const int hour = time.get(Calendar::HOUR_OF_DAY);
    const int minute = time.get(Calendar::MINUTE);
    const int dayOfWeek = time.get(Calendar::DAY_OF_WEEK); // SUNDAY=1..SATURDAY=7
    const std::string weekday = SHORT_WEEKDAYS[(dayOfWeek + 5) % 7];
    char buf[64];
    if (is24) {
        snprintf(buf, sizeof(buf), "%s %02d:%02d", weekday, hour, minute);
    } else {
        const int hour12 = hour % 12;
        snprintf(buf, sizeof(buf), "%s %d:%02d %s", weekday, hour12 == 0 ? 12 : hour12, minute,
                 hour < 12 ? "AM" : "PM");
    }
    return buf;
}

std::string AlarmUtils::getFormattedTime(Context& context, int64_t timeInMillis) {
    auto c = Calendar::getInstance();
    c->setTimeInMillis(timeInMillis);
    return getFormattedTime(context, *c);
}

std::string AlarmUtils::getAlarmText(Context& context, const data::Alarminstance& instance,
                                     bool includeLabel) {
    auto alarmTime = instance.getAlarmTime();
    const std::string alarmTimeStr = getFormattedTime(context, alarmTime);
    if (instance.mLabel.empty() || !includeLabel) {
        return alarmTimeStr;
    }
    return alarmTimeStr + " - " + instance.mLabel;
}

std::string AlarmUtils::formatElapsedTimeUntilAlarm(Context& context, int64_t delta) {
    // If the alarm will ring within 60 seconds, just report "less than a minute."
    const std::vector<std::string> formats =
            context.getResources().getStringArray(R::array::alarm_set);
    if (delta < MINUTE_IN_MILLIS) {
        return formats.empty() ? std::string() : formats[0];
    }

    // Round delta upwards to the nearest whole minute. (e.g. 7m 58s -> 8m)
    const int64_t remainder = delta % MINUTE_IN_MILLIS;
    delta += (remainder == 0) ? 0 : MINUTE_IN_MILLIS - remainder;
    int hours = (int) (delta / HOUR_IN_MILLIS);
    const int minutes = (int) (delta / MINUTE_IN_MILLIS) % 60;
    const int days = hours / 24;
    hours %= 24;

    const std::string daySeq = Utils::getNumberFormattedQuantityString(
            context, R::plurals::days, days);
    const std::string minSeq = Utils::getNumberFormattedQuantityString(
            context, R::plurals::minutes, minutes);
    const std::string hourSeq = Utils::getNumberFormattedQuantityString(
            context, R::plurals::hours, hours);

    const bool showDays = days > 0;
    const bool showHours = hours > 0;
    const bool showMinutes = minutes > 0;

    const int index = (showDays ? 1 : 0) | (showHours ? 2 : 0) | (showMinutes ? 4 : 0);

    if (index >= (int) formats.size()) return std::string();
    // These resources use %n$s positional placeholders.
    std::string out;
    const std::string& pattern = formats[index];
    for (size_t i = 0; i < pattern.size(); i++) {
        if (pattern[i] == '%' && i + 3 < pattern.size() && pattern[i + 2] == '$'
                && pattern[i + 3] == 's' && pattern[i + 1] >= '1' && pattern[i + 1] <= '3') {
            const int arg = pattern[i + 1] - '1';
            out += (arg == 0) ? daySeq : (arg == 1) ? hourSeq : minSeq;
            i += 3;
            continue;
        }
        out += pattern[i];
    }
    return out;
}

} // namespace deskclock
} // namespace cdroid
