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

#include <islamiccalendar.h>

namespace {
static constexpr int64_t ONE_SECOND = 1000;
static constexpr int64_t ONE_MINUTE = 60 * ONE_SECOND;
static constexpr int64_t ONE_HOUR = 60 * ONE_MINUTE;
static constexpr int64_t ONE_DAY = 24 * ONE_HOUR;
static constexpr int64_t ISLAMIC_EPOCH = 1948439LL;

static int64_t gregorianToJdn(int year, int month, int day) {
    int a = (14 - month) / 12;
    int y = year + 4800 - a;
    int m = month + 12 * a - 3;
    return day + ((153 * m + 2) / 5) + 365LL * y + y / 4 - y / 100 + y / 400 - 32045;
}

static void jdnToGregorian(int64_t jdn, int& year, int& month, int& day) {
    int64_t a = jdn + 32044;
    int64_t b = (4 * a + 3) / 146097;
    int64_t c = a - (146097 * b) / 4;
    int64_t d = (4 * c + 3) / 1461;
    int64_t e = c - (1461 * d) / 4;
    int64_t m = (5 * e + 2) / 153;
    day = static_cast<int>(e - (153 * m + 2) / 5 + 1);
    month = static_cast<int>(m + 3 - 12 * (m / 10));
    year = static_cast<int>(100 * b + d - 4800 + (m / 10));
}

static bool isIslamicLeapYear(int year) {
    return ((11 * year + 14) % 30) < 11;
}

static int getIslamicMonthLength(int year, int month) {
    if ((month % 2) == 1) {
        return 30;
    }
    if (month != 12) {
        return 29;
    }
    return isIslamicLeapYear(year) ? 30 : 29;
}

static int getIslamicYearLength(int year) {
    return isIslamicLeapYear(year) ? 355 : 354;
}

static int64_t islamicToJdn(int year, int month, int day) {
    int64_t monthDays = ((month - 1) * 59 + 1) / 2;
    int64_t yearDays = 354LL * (year - 1) + (3 + 11LL * year) / 30;
    return day + monthDays + yearDays + ISLAMIC_EPOCH - 1;
}

static void jdnToIslamic(int64_t jdn, int& year, int& month, int& day) {
    year = static_cast<int>((30 * (jdn - ISLAMIC_EPOCH) + 10646) / 10631);
    while (jdn < islamicToJdn(year, 1, 1)) {
        year--;
    }
    while (jdn >= islamicToJdn(year + 1, 1, 1)) {
        year++;
    }
    month = 1;
    while (month < 12 && jdn >= islamicToJdn(year, month + 1, 1)) {
        month++;
    }
    day = static_cast<int>(jdn - islamicToJdn(year, month, 1) + 1);
}

static int getIslamicDayOfYear(int year, int month, int day) {
    int dayOfYear = day;
    for (int m = 1; m < month; ++m) {
        dayOfYear += getIslamicMonthLength(year, m);
    }
    return dayOfYear;
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
    int year = internalGet(YEAR);
    int month = internalGet(MONTH) + 1;
    int day = internalGet(DAY_OF_MONTH);
    if (month < 1 || month > 12 || day < 1 || day > getIslamicMonthLength(year, month)) {
        GregorianCalendar::computeTime();
        return;
    }
    int64_t jdn = islamicToJdn(year, month, day);
    int gYear, gMonth, gDay;
    jdnToGregorian(jdn, gYear, gMonth, gDay);
    struct tm tn = {};
    tn.tm_year = gYear - 1900;
    tn.tm_mon = gMonth - 1;
    tn.tm_mday = gDay;
    tn.tm_hour = isSet(HOUR_OF_DAY) ? internalGet(HOUR_OF_DAY) : 0;
    if (!isSet(HOUR_OF_DAY) && isSet(HOUR)) {
        tn.tm_hour = (internalGet(AM_PM) == PM ? 12 : 0) + internalGet(HOUR);
    }
    tn.tm_min = isSet(MINUTE) ? internalGet(MINUTE) : 0;
    tn.tm_sec = isSet(SECOND) ? internalGet(SECOND) : 0;
    time_t seconds = timegm(&tn);
    mTime = static_cast<int64_t>(seconds) * ONE_SECOND + (isSet(MILLISECOND) ? internalGet(MILLISECOND) : 0) - static_cast<int64_t>(getTimeZone()) * ONE_SECOND;
    isTimeSet = true;
}

void IslamicCalendar::computeFields() {
    int64_t utcMillis = getTime();
    int64_t seconds = utcMillis / ONE_SECOND;
    int64_t localSeconds = seconds + getTimeZone();
    time_t localTime = static_cast<time_t>(localSeconds);
    struct tm tn;
    gmtime_r(&localTime, &tn);
    int64_t jdn = gregorianToJdn(tn.tm_year + 1900, tn.tm_mon + 1, tn.tm_mday);
    int year, month, day;
    jdnToIslamic(jdn, year, month, day);
    internalSet(YEAR, year);
    internalSet(MONTH, month - 1);
    internalSet(DATE, day);
    internalSet(DAY_OF_MONTH, day);
    internalSet(DAY_OF_YEAR, getIslamicDayOfYear(year, month, day));
    internalSet(DAY_OF_WEEK, tn.tm_wday);
    internalSet(AM_PM, tn.tm_hour / 12);
    internalSet(HOUR_OF_DAY, tn.tm_hour);
    internalSet(HOUR, tn.tm_hour % 12);
    internalSet(MINUTE, tn.tm_min);
    internalSet(SECOND, tn.tm_sec);
    internalSet(MILLISECOND, static_cast<int>(utcMillis % ONE_SECOND));
    internalSet(ZONE_OFFSET, getTimeZone());
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
