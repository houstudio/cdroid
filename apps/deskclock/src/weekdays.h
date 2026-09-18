#ifndef __DESKCLOCK_WEEKDAYS_H__
#define __DESKCLOCK_WEEKDAYS_H__
/*********************************************************************************
 * Port of com.android.deskclock.data.Weekdays — an encoded form of a weekly
 * repeat schedule (7 bits, MON=0x01..SUN=0x40) with Order-aware formatting.
 *********************************************************************************/
#include <string>
#include <vector>

namespace cdroid {

class Context;
class Calendar;

namespace deskclock {
namespace data {

class Weekdays {
public:
    /** The orders in which the weekdays can be presented. */
    struct Order {
        enum Value { SAT_TO_FRI, SUN_TO_SAT, MON_TO_SUN };
        static const std::vector<int>& calendarDays(Value order);
    };

    /** An encoded form of a weekly repeat schedule. */
    const int bits;

private:
    explicit Weekdays(int bits);

public:
    /** @return a Weekdays with the given bits set. */
    static Weekdays fromBits(int bits);

    /** @return a Weekdays with the given calendar days set. */
    static Weekdays fromCalendarDays(const std::vector<int>& calendarDays);

    static const Weekdays& ALL();
    static const Weekdays& NONE();

    /** @return a copy with the given calendarDay's bit set or cleared. */
    Weekdays setBit(int calendarDay, bool on) const;

    /** @return true iff the bit for the given calendarDay is set. */
    bool isBitOn(int calendarDay) const;

    /** @return true if any day is set. */
    bool isRepeating() const { return bits != 0; }

    /**
     * @param time a point in time
     * @return the number of days to the previous enabled day; -1 if no days are enabled
     */
    int getDistanceToPreviousDay(Calendar& time) const;

    /**
     * @param time a point in time
     * @return the number of days to the next enabled day; -1 if no days are enabled
     */
    int getDistanceToNextDay(Calendar& time) const;

    bool equals(const Weekdays& other) const { return bits == other.bits; }

    /** @return the enabled weekdays in the given order. */
    std::string toString(Context& context, Order::Value order) const;

    /** @return the enabled weekdays with unabbreviated names (for talkback). */
    std::string toAccessibilityString(Context& context, Order::Value order) const;

    /** @return the number of enabled days. */
    int count() const;

private:
    std::string toString(Context& context, Order::Value order, bool forceLongNames) const;

    /** Maps calendar weekdays (Calendar::MONDAY..) to bit masks. */
    static int calendarDayToBit(int calendarDay);
};

} // namespace data
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_WEEKDAYS_H__
