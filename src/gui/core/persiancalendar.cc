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
#include <core/persiancalendar.h>
#include <core/calendarutils.h>

namespace {
static const int64_t ONE_SECOND = 1000;
static const int64_t ONE_DAY    = 24 * 60 * 60 * ONE_SECOND;
static const int EPOCH_JULIAN_DAY = 2440588;

// Persian (Solar Hijri, Birashk 2820-cycle) date -> Julian day.
static int persianToJdn(int year, int month, int day) {
    int epbase = year - ((year >= 0) ? 474 : 473);
    int epyear = 474 + (epbase % 2820);
    int mdays = (month <= 7) ? ((month - 1) * 31) : (((month - 1) * 30) + 6);
    return day + mdays + ((epyear * 682 - 110) / 2816) + (epyear - 1) * 365
         + (epbase / 2820) * 1029983 + 1948320 - 1;
}

// Julian day -> Persian date (1-based month).
static void jdnToPersian(int jdn, int& year, int& month, int& day) {
    int depoch = jdn - persianToJdn(475, 1, 1);
    int cycle = depoch / 1029983;
    int cyear = depoch % 1029983;
    int ycycle;
    if (cyear == 1029982) {
        ycycle = 2820;
    } else {
        int aux1 = cyear / 366;
        int aux2 = cyear % 366;
        ycycle = ((2134 * aux1 + 2816 * aux2 + 2815) / 1028522) + aux1 + 1;
    }
    year = ycycle + 2820 * cycle + 474;
    if (year <= 0) year -= 1;
    int yday = jdn - persianToJdn(year, 1, 1) + 1;
    month = (yday <= 186) ? ((yday - 1) / 31 + 1) : (((yday - 187) / 30) + 7);
    if (month <= 6) {
        day = yday - 31 * (month - 1);
    } else {
        day = yday - 186 - 30 * (month - 7);
    }
}

static int persianDayOfYear(int year, int month, int day) {
    if (month <= 6) return (month - 1) * 31 + day;
    return 186 + (month - 7) * 30 + day;
}

static bool isPersianLeapYear(int year) {
    return (persianToJdn(year + 1, 1, 1) - persianToJdn(year, 1, 1)) == 366;
}

static int julianDayToDayOfWeek(int jd) {
    int dow = (jd + cdroid::Calendar::MONDAY) % 7;
    if (dow < cdroid::Calendar::SUNDAY) dow += 7;
    return dow;
}
} // namespace

namespace cdroid {

PersianCalendar::PersianCalendar() : Calendar() {
}

PersianCalendar::PersianCalendar(int year, int month, int date) : Calendar() {
    set(YEAR, year);
    set(MONTH, month);
    set(DAY_OF_MONTH, date);
}

PersianCalendar::PersianCalendar(int year, int month, int date, int hourOfDay, int minute, int second)
        : Calendar() {
    set(YEAR, year);
    set(MONTH, month);
    set(DAY_OF_MONTH, date);
    set(HOUR_OF_DAY, hourOfDay);
    set(MINUTE, minute);
    set(SECOND, second);
}

void PersianCalendar::computeTime() {
    int year = isSet(YEAR) ? internalGet(YEAR) : 1;
    int month = internalGet(MONTH) + 1; // 1-based for persianToJdn
    int day = isSet(DAY_OF_MONTH) ? internalGet(DAY_OF_MONTH) : 1;
    int jd = persianToJdn(year, month, day);

    int64_t timeOfDay = 0;
    if (isSet(HOUR_OF_DAY)) {
        timeOfDay += internalGet(HOUR_OF_DAY);
    } else if (isSet(HOUR)) {
        timeOfDay += internalGet(HOUR);
        if (isSet(AM_PM)) timeOfDay += 12 * internalGet(AM_PM);
    }
    timeOfDay = (timeOfDay * 60 + (isSet(MINUTE) ? internalGet(MINUTE) : 0)) * 60
              + (isSet(SECOND) ? internalGet(SECOND) : 0);
    timeOfDay = timeOfDay * 1000 + (isSet(MILLISECOND) ? internalGet(MILLISECOND) : 0);

    int64_t zoneMillis = static_cast<int64_t>(getTimeZone()) * ONE_SECOND;
    mTime = (static_cast<int64_t>(jd) - EPOCH_JULIAN_DAY) * ONE_DAY + timeOfDay - zoneMillis;
    isTimeSet = true;
}

void PersianCalendar::computeFields() {
    int64_t zoneMillis = static_cast<int64_t>(getTimeZone()) * ONE_SECOND;
    int64_t localMillis = getTime() + zoneMillis;
    int64_t days = cdroid::CalendarUtils::floorDivide(localMillis, ONE_DAY);
    int jd = static_cast<int>(days) + EPOCH_JULIAN_DAY;

    int year, month, day;
    jdnToPersian(jd, year, month, day);

    internalSet(YEAR, year);
    internalSet(MONTH, month - 1); // 0-based field
    internalSet(DATE, day);
    internalSet(DAY_OF_MONTH, day);
    internalSet(DAY_OF_YEAR, persianDayOfYear(year, month, day));
    internalSet(DAY_OF_WEEK, julianDayToDayOfWeek(jd));

    int64_t t = localMillis - days * ONE_DAY;
    internalSet(MILLISECOND, static_cast<int>(t % 1000));
    t /= 1000;
    internalSet(SECOND, static_cast<int>(t % 60));
    t /= 60;
    internalSet(MINUTE, static_cast<int>(t % 60));
    t /= 60;
    internalSet(HOUR_OF_DAY, static_cast<int>(t));
    internalSet(AM_PM, static_cast<int>(t / 12));
    internalSet(HOUR, static_cast<int>(t % 12));
    internalSet(ZONE_OFFSET, static_cast<int>(zoneMillis));
    internalSet(DST_OFFSET, 0);
    internalSet(ERA, 1);
    computeWeekFields();
    setFieldsComputed(ALL_FIELDS);
}

int PersianCalendar::handleGetMonthLength(int extendedYear, int month) const {
    month += 1; // 0-based field -> 1-based
    if (month <= 6) return 31;
    if (month <= 11) return 30;
    return isPersianLeapYear(extendedYear) ? 30 : 29;
}

int PersianCalendar::handleGetYearLength(int extendedYear) const {
    return isPersianLeapYear(extendedYear) ? 366 : 365;
}

} // namespace cdroid
