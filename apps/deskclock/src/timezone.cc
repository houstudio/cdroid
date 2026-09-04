#include <timezone.h>

#include <ctime>

#include <porting/cdlog.h>

namespace cdroid {
namespace deskclock {

namespace {

/** TextClock::parseCustomZoneSeconds twin: "GMT±hh[:mm]" -> millis, else INT_MIN sentinel. */
int parseCustomZoneMillis(const std::string& id) {
    // Reuse the same grammar as cdroid's TextClock: GMT+hh / GMT-hh[:mm].
    if (id.size() < 7 || id.compare(0, 3, "GMT") != 0) return INT_MIN;
    const char sign = id[3];
    if (sign != '+' && sign != '-') return INT_MIN;
    int hours = 0, minutes = 0;
    const std::string rest = id.substr(4);
    size_t colon = rest.find(':');
    try {
        if (colon != std::string::npos) {
            hours = std::stoi(rest.substr(0, colon));
            minutes = std::stoi(rest.substr(colon + 1));
        } else {
            hours = std::stoi(rest);
        }
    } catch (...) {
        return INT_MIN;
    }
    const int millis = (hours * 3600 + minutes * 60) * 1000;
    return sign == '-' ? -millis : millis;
}

int localZoneOffsetMillis() {
    // tm_gmtoff is seconds east of UTC — the same source cdroid's TextClock uses.
    time_t now = time(nullptr);
    struct tm local;
    localtime_r(&now, &local);
    return (int) (local.tm_gmtoff * 1000);
}

} // namespace

TimeZone TimeZone::getTimeZone(const std::string& id) {
    if (id.empty()) return getDefault();
    const int custom = parseCustomZoneMillis(id);
    if (custom != INT_MIN) return TimeZone(id, custom);
    LOGW("deskclock TimeZone: Olson id '%s' has no zone engine; using the local zone",
         id.c_str());
    return TimeZone(id, localZoneOffsetMillis());
}

TimeZone TimeZone::getDefault() {
    // Empty id = the host default zone (java.util.TimeZone.getDefault).
    return TimeZone("", localZoneOffsetMillis());
}

} // namespace deskclock
} // namespace cdroid
