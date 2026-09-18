#ifndef __DESKCLOCK_TIMEZONE_H__
#define __DESKCLOCK_TIMEZONE_H__
/*********************************************************************************
 * App-local java.util.TimeZone facade (subset used by DeskClock).
 *
 * CDROID has no Olson/IANA zone engine (see TextClock::createTime): only custom
 * "GMT±hh[:mm]" ids resolve to a real offset; Olson ids resolve to the host's
 * local zone offset. World-clock accuracy for foreign Olson ids is a known gap
 * until a zone-info module lands in cdroid.
 *********************************************************************************/
#include <string>
#include <climits>

namespace cdroid {
namespace deskclock {

class TimeZone {
private:
    std::string mId;
    int mRawOffsetMillis;

    explicit TimeZone(const std::string& id, int rawOffsetMillis)
        : mId(id), mRawOffsetMillis(rawOffsetMillis) {}

public:
    /** java.util.TimeZone.getTimeZone(id) — custom GMT ids or the local zone. */
    static TimeZone getTimeZone(const std::string& id);

    /** java.util.TimeZone.getDefault(). */
    static TimeZone getDefault();

    const std::string& getID() const { return mId; }

    /** Returns the amount of time in milliseconds to add to UTC to get standard time. */
    int getRawOffset() const { return mRawOffsetMillis; }

    /** Returns the offset at the given time; DST is not modeled (== raw offset). */
    int getOffset(int64_t /*date*/) const { return mRawOffsetMillis; }
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_TIMEZONE_H__
