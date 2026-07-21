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
#include <core/islamiccalendar.h>
#include <core/calendarutils.h>

namespace {
static const int64_t ONE_SECOND = 1000;
static const int64_t ONE_DAY    = 24 * 60 * 60 * ONE_SECOND;
static const int EPOCH_JULIAN_DAY = 2440588;
static const int64_t ISLAMIC_EPOCH = 1948439LL; // tabular (Thursday) epoch

static bool isIslamicLeapYear(int year) {
    return ((11 * year + 14) % 30) < 11;
}

static int getIslamicMonthLength(int year, int month) {
    if ((month % 2) == 1) return 30;
    if (month != 12) return 29;
    return isIslamicLeapYear(year) ? 30 : 29;
}

static int getIslamicYearLength(int year) {
    return isIslamicLeapYear(year) ? 355 : 354;
}

// Tabular Islamic date (1-based month) -> Julian day.
static int islamicToJdn(int year, int month, int day) {
    int monthDays = ((month - 1) * 59 + 1) / 2;
    int yearDays = 354 * (year - 1) + (3 + 11 * year) / 30;
    return day + monthDays + yearDays + static_cast<int>(ISLAMIC_EPOCH) - 1;
}

// Julian day -> tabular Islamic date (1-based month).
static void jdnToIslamic(int jdn, int& year, int& month, int& day) {
    year = (30 * (jdn - ISLAMIC_EPOCH) + 10646) / 10631;
    while (jdn < islamicToJdn(year, 1, 1)) year--;
    while (jdn >= islamicToJdn(year + 1, 1, 1)) year++;
    month = 1;
    while (month < 12 && jdn >= islamicToJdn(year, month + 1, 1)) month++;
    day = jdn - islamicToJdn(year, month, 1) + 1;
}

static int getIslamicDayOfYear(int year, int month, int day) {
    int dayOfYear = day;
    for (int m = 1; m < month; ++m) {
        dayOfYear += getIslamicMonthLength(year, m);
    }
    return dayOfYear;
}

static int julianDayToDayOfWeek(int jd) {
    int dow = (jd + cdroid::Calendar::MONDAY) % 7;
    if (dow < cdroid::Calendar::SUNDAY) dow += 7;
    return dow;
}
} // namespace

namespace cdroid {

IslamicCalendar::IslamicCalendar() : GregorianCalendar() {
}

IslamicCalendar::IslamicCalendar(int year, int month, int date)
        : GregorianCalendar(year, month, date) {
}

IslamicCalendar::IslamicCalendar(int year, int month, int date, int hourOfDay, int minute, int second)
        : GregorianCalendar(year, month, date, hourOfDay, minute, second) {
}

void IslamicCalendar::computeTime() {
    int year = isSet(YEAR) ? internalGet(YEAR) : 1;
    int month = internalGet(MONTH) + 1; // 1-based for islamicToJdn
    int day = isSet(DAY_OF_MONTH) ? internalGet(DAY_OF_MONTH) : 1;
    int jd = islamicToJdn(year, month, day);

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

void IslamicCalendar::computeFields() {
    int64_t zoneMillis = static_cast<int64_t>(getTimeZone()) * ONE_SECOND;
    int64_t localMillis = getTime() + zoneMillis;
    int64_t days = cdroid::CalendarUtils::floorDivide(localMillis, ONE_DAY);
    int jd = static_cast<int>(days) + EPOCH_JULIAN_DAY;

    int year, month, day;
    jdnToIslamic(jd, year, month, day);

    internalSet(YEAR, year);
    internalSet(MONTH, month - 1); // 0-based field
    internalSet(DATE, day);
    internalSet(DAY_OF_MONTH, day);
    internalSet(DAY_OF_YEAR, getIslamicDayOfYear(year, month, day));
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

int IslamicCalendar::handleGetMonthLength(int extendedYear, int month) const {
    return getIslamicMonthLength(extendedYear, month + 1);
}

int IslamicCalendar::handleGetYearLength(int extendedYear) const {
    return getIslamicYearLength(extendedYear);
}

} // namespace cdroid
