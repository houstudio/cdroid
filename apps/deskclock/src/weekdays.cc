#include <weekdays.h>
#include <cstdint>
#include <R.h>

#include <core/calendar.h>
#include <content/dateformatsymbols.h>
#include <core/context.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace data {

namespace {
constexpr int ALL_DAYS = 0x7F;
} // namespace

const std::vector<int>& Weekdays::Order::calendarDays(Value order) {
    static const std::vector<int> kSatToFri = {Calendar::SATURDAY, Calendar::SUNDAY,
        Calendar::MONDAY, Calendar::TUESDAY, Calendar::WEDNESDAY, Calendar::THURSDAY,
        Calendar::FRIDAY};
    static const std::vector<int> kSunToSat = {Calendar::SUNDAY, Calendar::MONDAY,
        Calendar::TUESDAY, Calendar::WEDNESDAY, Calendar::THURSDAY, Calendar::FRIDAY,
        Calendar::SATURDAY};
    static const std::vector<int> kMonToSun = {Calendar::MONDAY, Calendar::TUESDAY,
        Calendar::WEDNESDAY, Calendar::THURSDAY, Calendar::FRIDAY, Calendar::SATURDAY,
        Calendar::SUNDAY};
    switch (order) {
        case SAT_TO_FRI: return kSatToFri;
        case SUN_TO_SAT: return kSunToSat;
        case MON_TO_SUN: return kMonToSun;
    }
    return kSunToSat;
}

Weekdays::Weekdays(int bits) : bits(ALL_DAYS & bits) {
}

Weekdays Weekdays::fromBits(int bits) {
    return Weekdays(bits);
}

Weekdays Weekdays::fromCalendarDays(const std::vector<int>& calendarDays) {
    int bits = 0;
    for (int calendarDay : calendarDays) {
        const int bit = calendarDayToBit(calendarDay);
        if (bit != 0) bits |= bit;
    }
    return Weekdays(bits);
}

const Weekdays& Weekdays::ALL() {
    static Weekdays ALL(fromBits(ALL_DAYS));
    return ALL;
}

const Weekdays& Weekdays::NONE() {
    static Weekdays NONE(fromBits(0));
    return NONE;
}

int Weekdays::calendarDayToBit(int calendarDay) {
    switch (calendarDay) {
        case Calendar::MONDAY:    return 0x01;
        case Calendar::TUESDAY:   return 0x02;
        case Calendar::WEDNESDAY: return 0x04;
        case Calendar::THURSDAY:  return 0x08;
        case Calendar::FRIDAY:    return 0x10;
        case Calendar::SATURDAY:  return 0x20;
        case Calendar::SUNDAY:    return 0x40;
        default:                  return 0;
    }
}

Weekdays Weekdays::setBit(int calendarDay, bool on) const {
    const int bit = calendarDayToBit(calendarDay);
    if (bit == 0) return *this;
    return Weekdays(on ? bits | bit : bits & ~bit);
}

bool Weekdays::isBitOn(int calendarDay) const {
    const int bit = calendarDayToBit(calendarDay);
    if (bit == 0) {
        throw std::invalid_argument(std::to_string(calendarDay) + " is not a valid weekday");
    }
    return (bits & bit) > 0;
}

int Weekdays::getDistanceToPreviousDay(Calendar& time) const {
    int calendarDay = time.get(Calendar::DAY_OF_WEEK);
    for (int count = 1; count <= 7; count++) {
        calendarDay--;
        if (calendarDay < Calendar::SUNDAY) {
            calendarDay = Calendar::SATURDAY;
        }
        if (isBitOn(calendarDay)) {
            return count;
        }
    }

    return -1;
}

int Weekdays::getDistanceToNextDay(Calendar& time) const {
    int calendarDay = time.get(Calendar::DAY_OF_WEEK);
    for (int count = 0; count <= 6; count++) {
        if (isBitOn(calendarDay)) {
            return count;
        }

        calendarDay++;
        if (calendarDay > Calendar::SATURDAY) {
            calendarDay = Calendar::SUNDAY;
        }
    }

    return -1;
}

std::string Weekdays::toString(Context& context, Order::Value order) const {
    return toString(context, order, false /* forceLongNames */);
}

std::string Weekdays::toAccessibilityString(Context& context, Order::Value order) const {
    return toString(context, order, true /* forceLongNames */);
}

int Weekdays::count() const {
    int count = 0;
    for (int calendarDay = Calendar::SUNDAY; calendarDay <= Calendar::SATURDAY; calendarDay++) {
        if (isBitOn(calendarDay)) {
            count++;
        }
    }
    return count;
}

std::string Weekdays::toString(Context& context, Order::Value order,
                               bool forceLongNames) const {
    if (!isRepeating()) {
        return "";
    }

    if (bits == ALL_DAYS) {
        return context.getString(R::string::every_day);
    }

    const bool longNames = forceLongNames || count() <= 1;
    DateFormatSymbols dfs;
    const std::vector<std::string>& weekdays =
            longNames ? dfs.getWeekdays() : dfs.getShortWeekdays();

    const std::string separator = context.getString(R::string::day_concat);

    std::string builder;
    for (int calendarDay : Order::calendarDays(order)) {
        if (isBitOn(calendarDay)) {
            if (!builder.empty()) {
                builder.append(separator);
            }
            builder.append(weekdays[calendarDay]);
        }
    }
    return builder;
}

} // namespace data
} // namespace deskclock
} // namespace cdroid
