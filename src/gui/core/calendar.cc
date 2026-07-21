/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <calendar.h>
#include <gregoriancalendar.h>
#include <calendarutils.h>
#include <stdarg.h>
#include <climits>
#include <algorithm>
#include <assert.h>
#include <cdtypes.h>
#include <cdlog.h>
using namespace cdroid;

static int getLocalTimeZoneOffsetSeconds() {
    time_t now = time(nullptr);
    struct tm localTm = {};
    struct tm utcTm = {};
#ifdef _WIN32
    localtime_s(&localTm, &now);
    gmtime_s(&utcTm, &now);
#else
    localtime_r(&now, &localTm);
    gmtime_r(&now, &utcTm);
#endif
    time_t localTime = mktime(&localTm);
    time_t utcTime = timegm(&utcTm);
    return static_cast<int>(difftime(localTime, utcTime));
}


#define INTERNALLY_SET -1
#define ONE_SECOND 1000
#define ONE_MINUTE (60*ONE_SECOND)
#define ONE_HOUR   (60*ONE_MINUTE)
#define ONE_DAY    (24*ONE_HOUR)
#define ONE_WEEK   (7*ONE_DAY)
#define JAN_1_1_JULIAN_DAY 1721426
#define EPOCH_JULIAN_DAY   2440588
#define STAMP_MAX  10000

// Gregorian month table ported from ICU android.icu.util.Calendar.
// Columns: [month length, leap-month length, days-before-month, days-before-month-leap].
static const int GREGORIAN_MONTH_COUNT[12][4] = {
    {  31,  31,   0,   0 }, // Jan
    {  28,  29,  31,  31 }, // Feb
    {  31,  31,  59,  60 }, // Mar
    {  30,  30,  90,  91 }, // Apr
    {  31,  31, 120, 121 }, // May
    {  30,  30, 151, 152 }, // Jun
    {  31,  31, 181, 182 }, // Jul
    {  31,  31, 212, 213 }, // Aug
    {  30,  30, 243, 244 }, // Sep
    {  31,  31, 273, 274 }, // Oct
    {  30,  30, 304, 305 }, // Nov
    {  31,  31, 334, 335 }  // Dec
};

// Julian-day helpers ported from ICU android.icu.util.Calendar.
static int millisToJulianDay(int64_t millis) {
    return static_cast<int>(EPOCH_JULIAN_DAY + CalendarUtils::floorDivide(millis, static_cast<int64_t>(ONE_DAY)));
}

// Julian day 0 is Monday. Returns 1..7 (SUNDAY..SATURDAY).
static int julianDayToDayOfWeek(int julian) {
    int dayOfWeek = (julian + Calendar::MONDAY) % 7;
    if (dayOfWeek < Calendar::SUNDAY) {
        dayOfWeek += 7;
    }
    return dayOfWeek;
}

// Julian day of the day BEFORE the first day of the given extended Gregorian
// year/month. Ported from ICU android.icu.util.Calendar.computeGregorianMonthStart.
static int gregorianMonthStart(int year, int month) {
    if (month < 0 || month > 11) {
        int rem;
        year += CalendarUtils::floorDivide(month, 12, rem);
        month = rem;
    }
    int y = year - 1;
    int julianDay = 365 * y + CalendarUtils::floorDivide(y, 4) - CalendarUtils::floorDivide(y, 100)
                  + CalendarUtils::floorDivide(y, 400) + JAN_1_1_JULIAN_DAY - 1;
    if (month != 0) {
        bool isLeap = CalendarUtils::isGregorianLeapYear(year);
        julianDay += GREGORIAN_MONTH_COUNT[month][isLeap ? 3 : 2];
    }
    return julianDay;
}

static bool isGregorianLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int getDaysInGregorianMonth(int year, int month) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == Calendar::FEBRUARY) {
        return days[month] + (isGregorianLeapYear(year) ? 1 : 0);
    }
    return days[month];
}

static int getDaysInGregorianYear(int year) {
    return isGregorianLeapYear(year) ? 366 : 365;
}

static int getWeekCount(int year, int firstDayOfWeek, int minimalDaysInFirstWeek) {
    int jan1 = gregorianMonthStart(year, 0) + 1;
    int firstDayOfYear = julianDayToDayOfWeek(jan1);
    int offset = (firstDayOfYear - firstDayOfWeek + 7) % 7;
    int firstWeekDays = 7 - offset;
    int yearDays = isGregorianLeapYear(year) ? 366 : 365;
    if (firstWeekDays >= minimalDaysInFirstWeek) {
        return (yearDays + offset + 6) / 7;
    }
    return (yearDays + offset - firstWeekDays + 6) / 7;
}

namespace cdroid{

Calendar::Calendar(){
    zone  = 0;
    mTime = 0;
    isTimeSet = false;
    areFieldsSet = false;
    areAllFieldsSet = false;
    clear();
    firstDayOfWeek = SUNDAY;
    minimalDaysInFirstWeek = 1;
    setTime(static_cast<int64_t>(time(nullptr)) * ONE_SECOND);
}

std::unique_ptr<Calendar> Calendar::getInstance() {
    auto cal = std::make_unique<GregorianCalendar>();
    cal->setTimeZone(getLocalTimeZoneOffsetSeconds());
    return cal;
}

std::unique_ptr<Calendar> Calendar::getInstance(int zoneOffsetSeconds) {
    auto cal = std::make_unique<GregorianCalendar>();
    cal->setTimeZone(zoneOffsetSeconds);
    return cal;
}

std::unique_ptr<Calendar> Calendar::getInstance(int64_t instantMillis) {
    auto cal = getInstance();
    cal->setTime(instantMillis);
    return cal;
}

std::unique_ptr<Calendar> Calendar::getInstance(int64_t instantMillis, int zoneOffsetSeconds) {
    auto cal = std::make_unique<GregorianCalendar>();
    cal->setTimeZone(zoneOffsetSeconds);
    cal->setTime(instantMillis);
    return cal;
}

int Calendar::getMinimum(int field) const {
    return getLimit(field, LIMIT_MINIMUM);
}

int Calendar::getMaximum(int field) const {
    return getLimit(field, LIMIT_MAXIMUM);
}

int Calendar::getGreatestMinimum(int field) const {
    return getLimit(field, LIMIT_GREATEST_MINIMUM);
}

int Calendar::getLeastMaximum(int field) const {
    return getLimit(field, LIMIT_LEAST_MAXIMUM);
}

static const int LIMITS[Calendar::FIELD_COUNT][4] = {
    { 0, 0, 1, 1 },
    { 1, 292278994, 1, 292278994 },
    { 0, 11, 0, 11 },
    { 1, 1, 53, 53 },
    { 0, 0, 0, 0 },
    { 1, 1, 28, 31 },
    { 1, 1, 365, 366 },
    { 1, 1, 7, 7 },
    { -1, -1, 5, 5 },
    { 0, 0, 1, 1 },
    { 0, 0, 11, 11 },
    { 0, 0, 23, 23 },
    { 0, 0, 59, 59 },
    { 0, 0, 59, 59 },
    { 0, 0, 999, 999 },
    { -24 * ONE_HOUR, -16 * ONE_HOUR, 12 * ONE_HOUR, 30 * ONE_HOUR },
    { 0, 0, 2 * ONE_HOUR, 2 * ONE_HOUR },
};

int Calendar::getLimit(int field, int limitType) const {
    switch (field) {
        case DAY_OF_WEEK:
        case AM_PM:
        case HOUR:
        case HOUR_OF_DAY:
        case MINUTE:
        case SECOND:
        case MILLISECOND:
        case ZONE_OFFSET:
        case DST_OFFSET:
            return LIMITS[field][limitType];

        case WEEK_OF_MONTH: {
            int limit;
            if (limitType == LIMIT_MINIMUM) {
                limit = (getMinimalDaysInFirstWeek() == 1) ? 1 : 0;
            } else if (limitType == LIMIT_GREATEST_MINIMUM) {
                limit = 1;
            } else {
                int minDaysInFirst = getMinimalDaysInFirstWeek();
                int daysInMonth = handleGetLimit(DAY_OF_MONTH, limitType);
                if (limitType == LIMIT_LEAST_MAXIMUM) {
                    limit = (daysInMonth + (7 - minDaysInFirst)) / 7;
                } else {
                    limit = (daysInMonth + 6 + (7 - minDaysInFirst)) / 7;
                }
            }
            return limit;
        }
    }
    return handleGetLimit(field, limitType);
}

int Calendar::handleGetLimit(int field, int limitType) const {
    if (field < 0 || field >= FIELD_COUNT) {
        return 0;
    }
    if (limitType < LIMIT_MINIMUM || limitType > LIMIT_MAXIMUM) {
        return 0;
    }
    return LIMITS[field][limitType];
}

void  Calendar::internalSet(int field, int value){
    fields[field] = value;
}

int Calendar::internalGet(int field){
    return fields[field];
}

void Calendar::updateTime() {
    computeTime();
    // The areFieldsSet and areAllFieldsSet values are no longer
    // controlled here (as of 1.5).
    isTimeSet = true;
}

Calendar& Calendar::set(int field, int value){
    if (field < 0 || field >= FIELD_COUNT) {
        return *this;  // reject out-of-range fields (WEEK_YEAR uses setWeekDate)
    }
    if (areFieldsSet && !areAllFieldsSet) {
         computeFields();
    }
    internalSet(field,value);
    isTimeSet = false;
    areFieldsSet =false;
    bisSet[field] = true;
    stamp[field] = nextStamp++;
    if(nextStamp==INT_MAX)
        adjustStamp();
    return *this;
}

void Calendar::setTime(int64_t millis){
    if (mTime == millis && isTimeSet && areFieldsSet && areAllFieldsSet)
        return;
    mTime = millis;
    isTimeSet = true;
    areFieldsSet = false;
    computeFields();
    areAllFieldsSet = areFieldsSet = true;
}

void Calendar::add(int field, int amount){
    if (amount == 0) return;

    // Ported from ICU android.icu.util.Calendar.add. CDROID has no DST, so the
    // wall-time-invariant dance is omitted; the path is pure-integer (no libc
    // timegm), which avoids the 2038 overflow on 32-bit ARM time_t.
    switch (field) {
        case ERA:
            set(field, get(field) + amount);
            pinField(ERA);
            return;
        case YEAR: {
            // Add Rule 2: clamp DAY_OF_MONTH to the target year/month length
            // (e.g. Feb 29 + 1 year -> Feb 28 in a non-leap year).
            int year = get(YEAR) + amount;
            int month = get(MONTH);
            int dom = get(DAY_OF_MONTH);
            int monthLen = handleGetMonthLength(year, month);
            if (dom > monthLen) dom = monthLen;
            set(YEAR, year);
            set(DAY_OF_MONTH, dom);
            return;
        }
        case MONTH: {
            // Add Rule 2: normalize the month (with year carry), then clamp
            // DAY_OF_MONTH to the target month's length (e.g. Jan 31 + 1 month
            // -> Feb 28/29, not Mar 2/3).
            int year = get(YEAR);
            int month = get(MONTH) + amount;
            int dom = get(DAY_OF_MONTH);
            int rem;
            year += CalendarUtils::floorDivide(month, 12, rem);
            month = rem;
            int monthLen = handleGetMonthLength(year, month);
            if (dom > monthLen) dom = monthLen;
            set(YEAR, year);
            set(MONTH, month);
            set(DAY_OF_MONTH, dom);
            return;
        }
        case WEEK_OF_YEAR:
        case WEEK_OF_MONTH:
        case DAY_OF_WEEK_IN_MONTH:
            setTime(getTime() + static_cast<int64_t>(amount) * ONE_WEEK);
            return;
        case AM_PM:
            setTime(getTime() + static_cast<int64_t>(amount) * 12 * ONE_HOUR);
            return;
        case DAY_OF_MONTH:
        case DAY_OF_YEAR:
        case DAY_OF_WEEK:
            setTime(getTime() + static_cast<int64_t>(amount) * ONE_DAY);
            return;
        case HOUR_OF_DAY:
        case HOUR:
            setTime(getTime() + static_cast<int64_t>(amount) * ONE_HOUR);
            return;
        case MINUTE:
            setTime(getTime() + static_cast<int64_t>(amount) * ONE_MINUTE);
            return;
        case SECOND:
            setTime(getTime() + static_cast<int64_t>(amount) * ONE_SECOND);
            return;
        case MILLISECOND:
            setTime(getTime() + amount);
            return;
        default:
            return;
    }
}

void Calendar::roll(int field, bool up){
    roll(field, up ? 1 : -1);
}

void Calendar::roll(int field, int amount){
    if (amount == 0) {
        return;
    }

    complete();

    switch (field) {
        case DAY_OF_MONTH:
        case AM_PM:
        case MINUTE:
        case SECOND:
        case MILLISECOND:
        case ERA: {
            int min = getActualMinimum(field);
            int max = getActualMaximum(field);
            int gap = max - min + 1;
            int value = get(field) + amount;
            value = (value - min) % gap;
            if (value < 0) {
                value += gap;
            }
            value += min;
            set(field, value);
            return;
        }

        case HOUR:
        case HOUR_OF_DAY: {
            int oldHour = get(field);
            int max = getMaximum(field);
            int newHour = (oldHour + amount) % (max + 1);
            if (newHour < 0) {
                newHour += max + 1;
            }
            setTimeInMillis(getTimeInMillis() + ONE_HOUR * static_cast<int64_t>(newHour - oldHour));
            return;
        }

        case MONTH: {
            int max = getActualMaximum(MONTH);
            int mon = (get(MONTH) + amount) % (max + 1);
            if (mon < 0) {
                mon += max + 1;
            }
            // Clamp DAY_OF_MONTH to the target month length (not via pinField,
            // which would normalize the overflowed date first).
            int year = get(YEAR);
            int dom = get(DAY_OF_MONTH);
            int monthLen = handleGetMonthLength(year, mon);
            if (dom > monthLen) dom = monthLen;
            set(MONTH, mon);
            set(DAY_OF_MONTH, dom);
            return;
        }

        case YEAR: {
            int newYear = get(YEAR) + amount;
            int maxYear = getActualMaximum(YEAR);
            if (maxYear < 32768) {
                if (newYear < 1) {
                    newYear = maxYear - ((-newYear) % maxYear);
                } else if (newYear > maxYear) {
                    newYear = ((newYear - 1) % maxYear) + 1;
                }
            } else if (newYear < 1) {
                newYear = 1;
            }
            int month = get(MONTH);
            int dom = get(DAY_OF_MONTH);
            int monthLen = handleGetMonthLength(newYear, month);
            if (dom > monthLen) dom = monthLen;
            set(YEAR, newYear);
            set(DAY_OF_MONTH, dom);
            return;
        }

        case WEEK_OF_MONTH: {
            int dow = internalGet(DAY_OF_WEEK) - getFirstDayOfWeek();
            if (dow < 0) dow += 7;

            int fdm = (dow - internalGet(DAY_OF_MONTH) + 1) % 7;
            if (fdm < 0) fdm += 7;

            int start;
            if ((7 - fdm) < getMinimalDaysInFirstWeek()) {
                start = 8 - fdm;
            } else {
                start = 1 - fdm;
            }

            int monthLen = getActualMaximum(DAY_OF_MONTH);
            int ldm = (monthLen - internalGet(DAY_OF_MONTH) + dow) % 7;
            int limit = monthLen + 7 - ldm;
            int gap = limit - start;
            int dayOfMonth = (internalGet(DAY_OF_MONTH) + amount * 7 - start) % gap;
            if (dayOfMonth < 0) {
                dayOfMonth += gap;
            }
            dayOfMonth += start;
            if (dayOfMonth < 1) {
                dayOfMonth = 1;
            }
            if (dayOfMonth > monthLen) {
                dayOfMonth = monthLen;
            }
            set(DAY_OF_MONTH, dayOfMonth);
            return;
        }

        case WEEK_OF_YEAR: {
            int dow = internalGet(DAY_OF_WEEK) - getFirstDayOfWeek();
            if (dow < 0) dow += 7;

            int fdy = (dow - internalGet(DAY_OF_YEAR) + 1) % 7;
            if (fdy < 0) fdy += 7;

            int start;
            if ((7 - fdy) < getMinimalDaysInFirstWeek()) {
                start = 8 - fdy;
            } else {
                start = 1 - fdy;
            }

            int yearLen = getActualMaximum(DAY_OF_YEAR);
            int ldy = (yearLen - internalGet(DAY_OF_YEAR) + dow) % 7;
            int limit = yearLen + 7 - ldy;
            int gap = limit - start;
            int dayOfYear = (internalGet(DAY_OF_YEAR) + amount * 7 - start) % gap;
            if (dayOfYear < 0) {
                dayOfYear += gap;
            }
            dayOfYear += start;
            if (dayOfYear < 1) {
                dayOfYear = 1;
            }
            if (dayOfYear > yearLen) {
                dayOfYear = yearLen;
            }
            set(DAY_OF_YEAR, dayOfYear);
            clear(MONTH);
            return;
        }

        case DAY_OF_YEAR: {
            int64_t delta = amount * ONE_DAY;
            int64_t time = getTimeInMillis();
            int64_t min2 = time - static_cast<int64_t>(internalGet(DAY_OF_YEAR) - 1) * ONE_DAY;
            int yearLength = getActualMaximum(DAY_OF_YEAR);
            int64_t newTime = (time + delta - min2) % (static_cast<int64_t>(yearLength) * ONE_DAY);
            if (newTime < 0) {
                newTime += static_cast<int64_t>(yearLength) * ONE_DAY;
            }
            setTimeInMillis(newTime + min2);
            return;
        }

        case DAY_OF_WEEK: {
            int64_t delta = amount * ONE_DAY;
            int leadDays = internalGet(DAY_OF_WEEK) - getFirstDayOfWeek();
            if (leadDays < 0) {
                leadDays += 7;
            }
            int64_t time = getTimeInMillis();
            int64_t min2 = time - static_cast<int64_t>(leadDays) * ONE_DAY;
            int64_t newTime = (time + delta - min2) % ONE_WEEK;
            if (newTime < 0) {
                newTime += ONE_WEEK;
            }
            setTimeInMillis(newTime + min2);
            return;
        }

        case DAY_OF_WEEK_IN_MONTH: {
            int preWeeks = (internalGet(DAY_OF_MONTH) - 1) / 7;
            int postWeeks = (getActualMaximum(DAY_OF_MONTH) - internalGet(DAY_OF_MONTH)) / 7;
            int64_t min2 = getTimeInMillis() - static_cast<int64_t>(preWeeks) * ONE_WEEK;
            int64_t gap2 = static_cast<int64_t>(ONE_WEEK) * (preWeeks + postWeeks + 1);
            int64_t time = getTimeInMillis();
            int64_t newTime = (time + static_cast<int64_t>(amount) * ONE_WEEK - min2) % gap2;
            if (newTime < 0) {
                newTime += gap2;
            }
            setTimeInMillis(newTime + min2);
            return;
        }

        default:
            // Only ZONE_OFFSET/DST_OFFSET reach here; rolling a zone offset is
            // meaningless, and calling add() would wrongly shift larger fields
            // (roll contract: larger fields must not change). No-op.
            return;
    }
}

int64_t Calendar::getTime() const {
    if (!isTimeSet) {
        const_cast<Calendar*>(this)->updateTime();
    }
    return mTime;
}

int64_t Calendar::internalGetTimeInMillis() const {
    return mTime;
}

void Calendar::setTimeZone(int zone){
    this->zone=zone;
    /* Recompute the fields from the time using the new zone.  This also
     * works if isTimeSet is false (after a call to set()).  In that case
     * the time will be computed from the fields using the new zone, then
     * the fields will get recomputed from that.  Consider the sequence of
     * calls: cal.setTimeZone(EST); cal.set(HOUR, 1); cal.setTimeZone(PST).
     * Is cal set to 1 o'clock EST or 1 o'clock PST?  Answer: PST.  More
     * generally, a call to setTimeZone() affects calls to set() BEFORE AND
     * AFTER it up to the next call to complete().*/
    areAllFieldsSet = areFieldsSet = false;
}

int Calendar::getTimeZone()const{
    return zone;
}

void Calendar::set(int year, int month, int date){
    set(YEAR, year);
    set(MONTH, month);
    set(DATE, date);    
}

void Calendar::set(int year, int month, int date, int hourOfDay, int minute,int second,int millis){
    set(YEAR, year);
    set(MONTH, month);
    set(DATE, date);
    set(HOUR_OF_DAY, hourOfDay);
    set(MINUTE, minute);
    set(SECOND, second);
    set(MILLISECOND,millis);
}

void Calendar::clear(int field){
    fields[field] = 0;
    stamp[field] = UNSET;
    bisSet[field] = false;
    areAllFieldsSet = areFieldsSet = false;
    isTimeSet = false;
}

void Calendar::clear(){
    for (int i = 0; i < FIELD_COUNT/*fields.length*/; ) {
        stamp[i] = fields[i] = 0; // UNSET == 0
        bisSet[i++] = false;
    }
    areAllFieldsSet = areFieldsSet = false;
    isTimeSet = false;
}

bool Calendar::isSet(int field) const {
    return stamp[field] != UNSET;
}

bool Calendar::isEquivalentTo(const Calendar& other) const{
    return isLenient() == other.isLenient() &&
            getFirstDayOfWeek() == other.getFirstDayOfWeek() &&
            getMinimalDaysInFirstWeek() == other.getMinimalDaysInFirstWeek() &&
            getTimeZone()==other.getTimeZone() /*&&
            getRepeatedWallTimeOption() == other.getRepeatedWallTimeOption() &&
            getSkippedWallTimeOption() == other.getSkippedWallTimeOption()*/;
}

void Calendar::complete(){
    if (!isTimeSet) {
        updateTime();
    }
    if (!areFieldsSet || !areAllFieldsSet) {
        computeFields(); // fills in unset fields
        areAllFieldsSet = areFieldsSet = true;
    }
}


void Calendar::adjustStamp(){
    int max = MINIMUM_USER_STAMP;
    int newStamp = MINIMUM_USER_STAMP;

    for (;;) {
        int min = INT_MAX;
        for (int i = 0; i <FIELD_COUNT/*stamp.length*/; i++) {
            int v = stamp[i];
            if (v >= newStamp && min > v) min = v;
            if (max < v)  max = v;
        }
        if (max != min && min == INT_MAX) {
            break;
        }
        for (int i = 0; i < FIELD_COUNT/*stamp.length*/; i++) {
            if (stamp[i] == min) {
                stamp[i] = newStamp;
            }
        }
        newStamp++;
        if (min == max) {
            break;
        }
    }
    nextStamp = newStamp;
}

int Calendar::get(int field){
    if (field == WEEK_YEAR) { complete(); return mWeekYear; }
    complete();
    return fields[field];
}

int Calendar::get(int field) const {
    if (field == WEEK_YEAR) { const_cast<Calendar*>(this)->complete(); return mWeekYear; }
    const_cast<Calendar*>(this)->complete();
    return fields[field];
}

Calendar& Calendar::setFields(const std::vector<int>&values){
    for(int i=0;i<values.size();i+=2){
        set(values[i],values[i+1]);
    }
    return *this;
}

#if defined(_WIN32)||defined(_WIN64)||defined(_MSVC_VER)
static void gmtime_r(const time_t* timer, struct tm* result) {
    //std::lock_guard<std::mutex> lock(gmtime_mutex);
    struct tm* tmp = ::gmtime(timer);
    if (tmp) {
        *result = *tmp;
    }
}
static time_t timegm(struct tm* tm) {
    return _mkgmtime(tm);
}
#endif
void Calendar::computeFields(){
    if (!isTimeSet) {
        return;
    }

    // Local wall-clock millis = UTC millis + zone offset. DST is unsupported,
    // so the DST component is always 0 (matches the legacy behavior).
    int64_t zoneMillis = static_cast<int64_t>(zone) * ONE_SECOND;
    int64_t localMillis = mTime + zoneMillis;

    int64_t days = CalendarUtils::floorDivide(localMillis, static_cast<int64_t>(ONE_DAY));
    int julianDay = static_cast<int>(days) + EPOCH_JULIAN_DAY;

    // Gregorian year/month/day-of-month/day-of-year from the julian day.
    // Ported from ICU android.icu.util.Calendar.computeGregorianFields.
    int64_t gregorianEpochDay = julianDay - JAN_1_1_JULIAN_DAY;
    int rem;
    int n400 = CalendarUtils::floorDivide(gregorianEpochDay, 146097, rem); // 400-yr cycle
    int n100 = CalendarUtils::floorDivide(rem, 36524, rem);                // 100-yr cycle
    int n4   = CalendarUtils::floorDivide(rem, 1461, rem);                 // 4-yr cycle
    int n1   = CalendarUtils::floorDivide(rem, 365, rem);
    int year = 400 * n400 + 100 * n100 + 4 * n4 + n1;
    int dayOfYear = rem; // zero-based
    if (n100 == 4 || n1 == 4) {
        dayOfYear = 365; // Dec 31 at the end of a 4- or 400-year cycle.
    } else {
        ++year;
    }

    bool isLeap = CalendarUtils::isGregorianLeapYear(year);
    int correction = 0;
    int march1 = isLeap ? 60 : 59; // zero-based DOY for March 1
    if (dayOfYear >= march1) correction = isLeap ? 1 : 2;
    int month = (12 * (dayOfYear + correction) + 6) / 367; // zero-based month
    int dayOfMonth = dayOfYear - GREGORIAN_MONTH_COUNT[month][isLeap ? 3 : 2] + 1;

    internalSet(MONTH, month);
    internalSet(DATE, dayOfMonth);
    internalSet(DAY_OF_MONTH, dayOfMonth);
    internalSet(DAY_OF_YEAR, dayOfYear + 1);

    // year is the proleptic Gregorian (extended) year; map BC (year < 1) to
    // the positive YEAR + ERA=BC representation, like GregorianCalendar.
    int era = AD;
    int eyear = year;
    if (year < 1) {
        era = BC;
        eyear = 1 - year;
    }
    internalSet(ERA, era);
    internalSet(YEAR, eyear);

    // Julian day 0 is Monday; helper returns 1..7 (SUNDAY..SATURDAY).
    internalSet(DAY_OF_WEEK, julianDayToDayOfWeek(julianDay));

    // Time-of-day fields, derived from the wall millis in the day.
    int millisInDay = static_cast<int>(localMillis - days * ONE_DAY);
    internalSet(MILLISECOND, millisInDay % 1000);
    millisInDay /= 1000;
    internalSet(SECOND, millisInDay % 60);
    millisInDay /= 60;
    internalSet(MINUTE, millisInDay % 60);
    millisInDay /= 60;
    internalSet(HOUR_OF_DAY, millisInDay);
    internalSet(AM_PM, millisInDay / 12);
    internalSet(HOUR, millisInDay % 12);
    internalSet(ZONE_OFFSET, zoneMillis);
    internalSet(DST_OFFSET, 0);

    computeWeekFields();
    setFieldsComputed(ALL_FIELDS);
}

void Calendar::computeWeekFields() {
    int year = fields[YEAR];
    int month = fields[MONTH];
    int dayOfMonth = fields[DAY_OF_MONTH];
    int dayOfYear = fields[DAY_OF_YEAR];
    int dow = fields[DAY_OF_WEEK]; // 1..7 (SUNDAY..SATURDAY)

    // Day of week of Jan 1 of this year, derived from the current date's
    // DAY_OF_WEEK and DAY_OF_YEAR (no libc; JD 0 = Monday convention).
    int firstDayOfYear = dow - (dayOfYear - 1);
    firstDayOfYear = ((firstDayOfYear - 1) % 7 + 7) % 7 + 1;

    int offset = (firstDayOfYear - firstDayOfWeek + 7) % 7;
    int firstWeekDays = 7 - offset;
    int yearWeeks = getWeekCount(year, firstDayOfWeek, minimalDaysInFirstWeek);

    int weekOfYear;
    int weekYear = year;
    if (firstWeekDays >= minimalDaysInFirstWeek) {
        weekOfYear = (dayOfYear + offset - 1) / 7 + 1;
        if (weekOfYear > yearWeeks) {
            weekOfYear = 1;
            weekYear = year + 1;
        }
    } else {
        if (dayOfYear <= firstWeekDays) {
            weekYear = year - 1;
            weekOfYear = getWeekCount(weekYear, firstDayOfWeek, minimalDaysInFirstWeek);
        } else {
            weekOfYear = (dayOfYear + offset - 1 - firstWeekDays) / 7 + 1;
            if (weekOfYear > yearWeeks) {
                weekOfYear = 1;
                weekYear = year + 1;
            }
        }
    }

    int firstDayOfMonth = dow - (dayOfMonth - 1);
    firstDayOfMonth = ((firstDayOfMonth - 1) % 7 + 7) % 7 + 1;
    int monthOffset = (firstDayOfMonth - firstDayOfWeek + 7) % 7;
    int firstMonthWeekDays = 7 - monthOffset;
    int weekOfMonth;
    if (firstMonthWeekDays >= minimalDaysInFirstWeek) {
        weekOfMonth = (dayOfMonth + monthOffset - 1) / 7 + 1;
    } else if (dayOfMonth <= firstMonthWeekDays) {
        weekOfMonth = 1;
    } else {
        weekOfMonth = (dayOfMonth + monthOffset - 1 - firstMonthWeekDays) / 7 + 1;
    }
    if (weekOfMonth < 1) {
        weekOfMonth = 1;
    }

    internalSet(WEEK_OF_YEAR, weekOfYear);
    mWeekYear = weekYear;
    internalSet(WEEK_OF_MONTH, weekOfMonth);
    internalSet(DAY_OF_WEEK_IN_MONTH, (dayOfMonth - 1) / 7 + 1);
}

void Calendar::setFieldsComputed(int fieldMask){
    if (fieldMask == ALL_FIELDS) {
        for (int i = 0; i < FIELD_COUNT/*fields.length*/; i++) {
            stamp[i] = COMPUTED;
            bisSet[i] = true;
        }
        areFieldsSet = areAllFieldsSet = true;
    } else {
        for (int i = 0; i < FIELD_COUNT/*fields.length*/; i++) {
            if ((fieldMask & 1) == 1) {
                stamp[i] = COMPUTED;
                bisSet[i] = true;
            } else {
                if (areAllFieldsSet && !bisSet[i]) {
                    areAllFieldsSet = false;
                }
            }
            fieldMask >>= 1;
        }
    }
}

void Calendar::pinField(int field) {
    int value = get(field);
    int min = getActualMinimum(field);
    int max = getActualMaximum(field);
    if (value < min) {
        value = min;
    } else if (value > max) {
        value = max;
    }
    set(field, value);
}

void Calendar::setFieldsNormalized(int fieldMask){
    if (fieldMask != ALL_FIELDS) {
        for (int i = 0; i < FIELD_COUNT/*fields.length*/; i++) {
            if ((fieldMask & 1) == 0) {
                stamp[i] = fields[i] = 0; // UNSET == 0
                bisSet[i] = false;
            }
            fieldMask >>= 1;
        }
    }

    // Some or all of the fields are in sync with the
    // milliseconds, but the stamp values are not normalized yet.
    areFieldsSet = true;
    areAllFieldsSet = false;
}

bool Calendar::isPartiallyNormalized()const{
    return areFieldsSet && !areAllFieldsSet;
}

bool Calendar::isFullyNormalized()const{
    return areFieldsSet && areAllFieldsSet;
}

void Calendar::setUnnormalized() {
    areFieldsSet = areAllFieldsSet = false;
}

bool Calendar::isFieldSet(int fieldMask, int field){
    return (fieldMask & (1 << field)) != 0;
}

int Calendar::selectFields(){
    int fieldMask = YEAR_MASK;

    if (stamp[ERA] != UNSET) {
        fieldMask |= ERA_MASK;
    }
    // Find the most recent group of fields specifying the day within
    // the year.  These may be any of the following combinations:
    //   MONTH + DAY_OF_MONTH
    //   MONTH + WEEK_OF_MONTH + DAY_OF_WEEK
    //   MONTH + DAY_OF_WEEK_IN_MONTH + DAY_OF_WEEK
    //   DAY_OF_YEAR
    //   WEEK_OF_YEAR + DAY_OF_WEEK
    // We look for the most recent of the fields in each group to determine
    // the age of the group.  For groups involving a week-related field such
    // as WEEK_OF_MONTH, DAY_OF_WEEK_IN_MONTH, or WEEK_OF_YEAR, both the
    // week-related field and the DAY_OF_WEEK must be set for the group as a
    // whole to be considered.  (See bug 4153860 - liu 7/24/98.)
    int dowStamp = stamp[DAY_OF_WEEK];
    int monthStamp = stamp[MONTH];
    int domStamp = stamp[DAY_OF_MONTH];
    int womStamp = aggregateStamp(stamp[WEEK_OF_MONTH], dowStamp);
    int dowimStamp = aggregateStamp(stamp[DAY_OF_WEEK_IN_MONTH], dowStamp);
    int doyStamp = stamp[DAY_OF_YEAR];
    int woyStamp = aggregateStamp(stamp[WEEK_OF_YEAR], dowStamp);

    int bestStamp = domStamp;
    if ( womStamp > bestStamp ) bestStamp = womStamp;
    if (dowimStamp > bestStamp) bestStamp = dowimStamp;
    if ( doyStamp > bestStamp ) bestStamp = doyStamp;
    if ( woyStamp > bestStamp ) bestStamp = woyStamp;

    /* No complete combination exists.  Look for WEEK_OF_MONTH,
     * DAY_OF_WEEK_IN_MONTH, or WEEK_OF_YEAR alone.  Treat DAY_OF_WEEK alone
     * as DAY_OF_WEEK_IN_MONTH.*/
    if (bestStamp == UNSET) {
        womStamp = stamp[WEEK_OF_MONTH];
        dowimStamp = std::max(stamp[DAY_OF_WEEK_IN_MONTH], dowStamp);
        woyStamp = stamp[WEEK_OF_YEAR];
        bestStamp = std::max(std::max(womStamp, dowimStamp), woyStamp);

        /* Treat MONTH alone or no fields at all as DAY_OF_MONTH.  This may
         * result in bestStamp = domStamp = UNSET if no fields are set,
         * which indicates DAY_OF_MONTH. */
        if (bestStamp == UNSET) {
            bestStamp = domStamp = monthStamp;
        }
    }

    if (bestStamp == domStamp ||
       (bestStamp == womStamp && stamp[WEEK_OF_MONTH] >= stamp[WEEK_OF_YEAR]) ||
       (bestStamp == dowimStamp && stamp[DAY_OF_WEEK_IN_MONTH] >= stamp[WEEK_OF_YEAR])) {
        fieldMask |= MONTH_MASK;
        if (bestStamp == domStamp) {
            fieldMask |= DAY_OF_MONTH_MASK;
        } else {
            assert (bestStamp == womStamp || bestStamp == dowimStamp);
            if (dowStamp != UNSET) {
                fieldMask |= DAY_OF_WEEK_MASK;
            }
            if (womStamp == dowimStamp) {
                // When they are equal, give the priority to
                // WEEK_OF_MONTH for compatibility.
                if (stamp[WEEK_OF_MONTH] >= stamp[DAY_OF_WEEK_IN_MONTH]) {
                    fieldMask |= WEEK_OF_MONTH_MASK;
                } else {
                    fieldMask |= DAY_OF_WEEK_IN_MONTH_MASK;
                }
            } else {
                if (bestStamp == womStamp) {
                    fieldMask |= WEEK_OF_MONTH_MASK;
                } else {
                    assert (bestStamp == dowimStamp);
                    if (stamp[DAY_OF_WEEK_IN_MONTH] != UNSET) {
                        fieldMask |= DAY_OF_WEEK_IN_MONTH_MASK;
                    }
                }
            }
        }
    } else {
        assert (bestStamp == doyStamp || bestStamp == woyStamp ||
                bestStamp == UNSET);
        if (bestStamp == doyStamp) {
            fieldMask |= DAY_OF_YEAR_MASK;
        } else {
            assert (bestStamp == woyStamp);
            if (dowStamp != UNSET) {
                fieldMask |= DAY_OF_WEEK_MASK;
            }
            fieldMask |= WEEK_OF_YEAR_MASK;
        }
    }

    // Find the best set of fields specifying the time of day.  There
    // are only two possibilities here; the HOUR_OF_DAY or the
    // AM_PM and the HOUR.
    int hourOfDayStamp = stamp[HOUR_OF_DAY];
    int hourStamp = aggregateStamp(stamp[HOUR], stamp[AM_PM]);
    bestStamp = (hourStamp > hourOfDayStamp) ? hourStamp : hourOfDayStamp;
    // if bestStamp is still UNSET, then take HOUR or AM_PM. (See 4846659)
    if (bestStamp == UNSET) {
        bestStamp = std::max(stamp[HOUR], stamp[AM_PM]);
    }
    // Hours
    if (bestStamp != UNSET) {
        if (bestStamp == hourOfDayStamp) {
            fieldMask |= HOUR_OF_DAY_MASK;
        } else {
            fieldMask |= HOUR_MASK;
            if (stamp[AM_PM] != UNSET) {
                fieldMask |= AM_PM_MASK;
            }
        }
    }
    if (stamp[MINUTE] != UNSET) fieldMask |= MINUTE_MASK;
    if (stamp[SECOND] != UNSET) fieldMask |= SECOND_MASK;
    if (stamp[MILLISECOND] != UNSET) fieldMask |= MILLISECOND_MASK;
    if (stamp[ZONE_OFFSET] >= MINIMUM_USER_STAMP)fieldMask |= ZONE_OFFSET_MASK;
    if (stamp[DST_OFFSET] >= MINIMUM_USER_STAMP) fieldMask |= DST_OFFSET_MASK;

    return fieldMask;
}

int Calendar::aggregateStamp(int stamp_a, int stamp_b){
    if (stamp_a == UNSET || stamp_b == UNSET) {
        return UNSET;
    }
    return (stamp_a > stamp_b) ? stamp_a : stamp_b;
}

int Calendar::handleGetMonthLength(int extendedYear, int month) const {
    return getDaysInGregorianMonth(extendedYear, month);
}

int Calendar::handleGetYearLength(int extendedYear) const {
    return getDaysInGregorianYear(extendedYear);
}

int Calendar::getActualMinimum(int field) const {
    switch (field) {
        case ERA: return 0;
        case YEAR: return 1;
        case MONTH: return JANUARY;
        case WEEK_OF_YEAR: return 1;
        case WEEK_OF_MONTH: return 1;
        case DAY_OF_MONTH: return 1;
        case DAY_OF_YEAR: return 1;
        case DAY_OF_WEEK: return SUNDAY;
        case DAY_OF_WEEK_IN_MONTH: return 1;
        case AM_PM: return AM;
        case HOUR: return 0;
        case HOUR_OF_DAY: return 0;
        case MINUTE: case SECOND: return 0;
        case MILLISECOND: return 0;
        case ZONE_OFFSET: return INT_MIN;
        case DST_OFFSET: return INT_MIN;
        default: return 0;
    }
}

int Calendar::getActualMaximum(int field) const {
    switch (field) {
        case DAY_OF_MONTH:
            return handleGetMonthLength(get(YEAR), get(MONTH));
        case DAY_OF_YEAR:
            return handleGetYearLength(get(YEAR));
        case ERA: return 1;
        case YEAR: return INT_MAX;
        case MONTH: return DECEMBER;
        case WEEK_OF_YEAR: {
            int weekYear = mWeekYear;
            return getWeekCount(weekYear, firstDayOfWeek, minimalDaysInFirstWeek);
        }
        case WEEK_OF_MONTH: {
            int year = get(YEAR);
            int month = get(MONTH);
            int daysInMonth = getDaysInGregorianMonth(year, month);
            int firstDayOfMonth = julianDayToDayOfWeek(gregorianMonthStart(year, month) + 1);
            int monthOffset = (firstDayOfMonth - firstDayOfWeek + 7) % 7;
            int firstMonthWeekDays = 7 - monthOffset;
            if (firstMonthWeekDays >= minimalDaysInFirstWeek) {
                return (daysInMonth + monthOffset - 1) / 7 + 1;
            }
            return (daysInMonth + monthOffset - 1 - firstMonthWeekDays) / 7 + 1;
        }
        case DAY_OF_WEEK: return SATURDAY;
        case DAY_OF_WEEK_IN_MONTH: {
            int year = get(YEAR);
            int month = get(MONTH);
            int daysInMonth = getDaysInGregorianMonth(year, month);
            int firstDayOfMonth = julianDayToDayOfWeek(gregorianMonthStart(year, month) + 1);
            int monthOffset = (firstDayOfMonth - firstDayOfWeek + 7) % 7;
            return (daysInMonth + monthOffset - 1) / 7 + 1;
        }
        case AM_PM: return PM;
        case HOUR: return 11;
        case HOUR_OF_DAY: return 23;
        case MINUTE: case SECOND: return 59;
        case MILLISECOND: return 999;
        case ZONE_OFFSET: return INT_MAX;
        case DST_OFFSET: return INT_MAX;
        default: return INT_MAX;
    }
}

bool Calendar::isLeapYear(int year) {
    return isGregorianLeapYear(year);
}

int Calendar::getDaysInMonth(int year, int month) {
    return getDaysInGregorianMonth(year, month);
}

int Calendar::getDaysInYear(int year) {
    return getDaysInGregorianYear(year);
}

/*Converts the current calendar field values in {@link #fields fields[]} to the millisecond time value*/
/*Converts the current calendar field values in fields[] to the millisecond
 time value. Ported from java.util.GregorianCalendar.computeTime (proleptic
 Gregorian, no Julian cutover).*/
void Calendar::computeTime(){
    int fieldMask = selectFields();

    // YEAR is mandatory to determine the date; default to the epoch year.
    int year = isSet(YEAR) ? internalGet(YEAR) : 1970 /*EPOCH_YEAR*/;
    int era = (stamp[ERA] != UNSET) ? internalGet(ERA) : AD;
    if (era == BC) {
        year = 1 - year; // fields[YEAR] holds the positive BC year; flip to extended.
    }

    // Time of day from the selected time fields (UNSET fields are 0).
    int64_t timeOfDay = 0;
    if (isFieldSet(fieldMask, HOUR_OF_DAY)) {
        timeOfDay += internalGet(HOUR_OF_DAY);
    } else {
        timeOfDay += internalGet(HOUR);
        if (isFieldSet(fieldMask, AM_PM)) {
            timeOfDay += 12 * internalGet(AM_PM);
        }
    }
    timeOfDay = (timeOfDay * 60 + internalGet(MINUTE)) * 60 + internalGet(SECOND);
    timeOfDay = timeOfDay * 1000 + internalGet(MILLISECOND);

    int64_t dayCarry = CalendarUtils::floorDivide(timeOfDay, static_cast<int64_t>(ONE_DAY));
    int64_t tod = timeOfDay - dayCarry * ONE_DAY; // remainder in [0, ONE_DAY)

    // Resolve the julian day at local midnight from the selected date-field
    // combination. DAY_OF_MONTH is the default baseline.
    int month = internalGet(MONTH);
    int dayOfMonth = isSet(DAY_OF_MONTH) ? internalGet(DAY_OF_MONTH) : 1;
    int jd = gregorianMonthStart(year, month) + dayOfMonth;

    if (isFieldSet(fieldMask, DAY_OF_YEAR)) {
        int jan1 = gregorianMonthStart(year, 0) + 1;
        jd = jan1 + internalGet(DAY_OF_YEAR) - 1;
    } else if (isFieldSet(fieldMask, DAY_OF_WEEK)) {
        int dow = internalGet(DAY_OF_WEEK); // 1..7
        int firstDow = firstDayOfWeek;
        if (isFieldSet(fieldMask, WEEK_OF_YEAR)) {
            int weekOfYear = internalGet(WEEK_OF_YEAR);
            int jan1 = gregorianMonthStart(year, 0) + 1;
            int first = (julianDayToDayOfWeek(jan1) - firstDow + 7) % 7;
            int week1Start = jan1 - first;
            if ((7 - first) < minimalDaysInFirstWeek) week1Start += 7;
            jd = week1Start + (weekOfYear - 1) * 7 + (dow - firstDow);
        } else {
            int month1 = gregorianMonthStart(year, month) + 1;
            int month1Dow = julianDayToDayOfWeek(month1);
            if (isFieldSet(fieldMask, DAY_OF_WEEK_IN_MONTH)) {
                int dowim = internalGet(DAY_OF_WEEK_IN_MONTH);
                int firstOccur = 1 + (dow - month1Dow + 7) % 7;
                int date;
                if (dowim > 0) {
                    date = firstOccur + 7 * (dowim - 1);
                } else if (dowim < 0) {
                    int monthLen = handleGetMonthLength(year, month);
                    int count = (monthLen - firstOccur) / 7 + 1;
                    date = firstOccur + 7 * (count + dowim);
                } else {
                    date = firstOccur;
                }
                jd = month1 + (date - 1);
            } else { // WEEK_OF_MONTH
                int weekOfMonth = internalGet(WEEK_OF_MONTH);
                int first = (month1Dow - firstDow + 7) % 7;
                int week1Start = month1 - first;
                if ((7 - first) < minimalDaysInFirstWeek) week1Start += 7;
                jd = week1Start + (weekOfMonth - 1) * 7 + (dow - firstDow);
            }
        }
    }

    int64_t fixedJd = static_cast<int64_t>(jd) + dayCarry;
    int64_t millis = (fixedJd - EPOCH_JULIAN_DAY) * ONE_DAY + tod;

    // Zone offset: honor an explicitly set ZONE_OFFSET/DST_OFFSET, else the
    // calendar's zone. DST data is unavailable, so DST_OFFSET defaults to 0.
    int64_t zoneOffset = static_cast<int64_t>(zone) * ONE_SECOND;
    if (stamp[ZONE_OFFSET] >= MINIMUM_USER_STAMP) zoneOffset = internalGet(ZONE_OFFSET);
    if (stamp[DST_OFFSET] >= MINIMUM_USER_STAMP) zoneOffset += internalGet(DST_OFFSET);
    millis -= zoneOffset;

    mTime = millis;
    isTimeSet = true;
}

Calendar& Calendar::setDate(int year, int month, int dayOfMonth){
    return setFields({YEAR, year, MONTH, month, DAY_OF_MONTH, dayOfMonth});
}

Calendar& Calendar::setTimeOfDay(int hourOfDay, int minute, int second,int millis){
    return setFields({HOUR_OF_DAY, hourOfDay, MINUTE, minute,
                    SECOND, second, MILLISECOND, millis});
}

Calendar& Calendar::setWeekDate(int weekYear, int weekOfYear, int dayOfWeek){
    mWeekYear = weekYear;
    return setFields({WEEK_OF_YEAR, weekOfYear, DAY_OF_WEEK, dayOfWeek});
}

Calendar& Calendar::setWeekDefinition(int firstDayOfWeek, int minimalDaysInFirstWeek){
    this->firstDayOfWeek = firstDayOfWeek;
    this->minimalDaysInFirstWeek = minimalDaysInFirstWeek;
    return *this;
}

bool Calendar::isValidWeekParameter(int value){
    return value > 0 && value <= 7;
}

Calendar& Calendar::setInstant(int64_t instant){
    setTime(instant);
    return *this;
}

Calendar& Calendar::setLenient(bool lenient){
    this->lenient = lenient;
    return *this;
}

bool Calendar::isLenient()const{
    return lenient;
}

void Calendar::invalidateWeekFields(){
    if (stamp[WEEK_OF_MONTH] != COMPUTED &&
        stamp[WEEK_OF_YEAR] != COMPUTED) {
        return;
    }

    // We have to check the new values of these fields after changing
    // firstDayOfWeek and/or minimalDaysInFirstWeek. If the field values
    // have been changed, then set the new values. (4822110)
    Calendar cal ;//= (Calendar) clone();
    cal.setLenient(true);
    cal.clear(WEEK_OF_MONTH);
    cal.clear(WEEK_OF_YEAR);

    if (stamp[WEEK_OF_MONTH] == COMPUTED) {
        int weekOfMonth = cal.get(WEEK_OF_MONTH);
        if (fields[WEEK_OF_MONTH] != weekOfMonth) {
            fields[WEEK_OF_MONTH] = weekOfMonth;
        }
    }

    if (stamp[WEEK_OF_YEAR] == COMPUTED) {
        int weekOfYear = cal.get(WEEK_OF_YEAR);
        if (fields[WEEK_OF_YEAR] != weekOfYear) {
            fields[WEEK_OF_YEAR] = weekOfYear;
        }
    }
}

void Calendar::setFirstDayOfWeek(int value){
    if (firstDayOfWeek == value) return;
    firstDayOfWeek = value;
    areFieldsSet = false;//invalidateWeekFields();
}

int Calendar::getFirstDayOfWeek()const{
    return firstDayOfWeek;
}

int Calendar::getMinimalDaysInFirstWeek() const {
    return minimalDaysInFirstWeek;
}

void Calendar::setMinimalDaysInFirstWeek(int value){
    if (minimalDaysInFirstWeek == value) {
        return;
    }
    minimalDaysInFirstWeek = value;
    areFieldsSet = false;
    //invalidateWeekFields();
}

int64_t Calendar::getTimeInMillis(){
    if (!isTimeSet) {
        updateTime();
    }
    return mTime;
}

void Calendar::setTimeInMillis(int64_t millis){
    if (mTime == millis && isTimeSet && areFieldsSet && areAllFieldsSet) {
        return;
    }
    mTime = millis;
    isTimeSet = true;
    areFieldsSet = false;
    computeFields();
    areAllFieldsSet = areFieldsSet = true;
}

bool Calendar::after(const Calendar&other)const{
    return getTime() > other.getTime();
}

bool Calendar::before(const Calendar&other)const{
    return getTime() < other.getTime();
}
}//namespace
