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

#include <hebrewcalendar.h>
#include <algorithm>

namespace {
static const int ONE_SECOND = 1000;
static const int ONE_MINUTE = 60 * ONE_SECOND;
static const int ONE_HOUR = 60 * ONE_MINUTE;
static const int ONE_DAY = 24 * ONE_HOUR;
static const int64_t HEBREW_EPOCH = -1373429;

static int64_t floorDiv(int64_t a, int64_t b) {
    int64_t result = a / b;
    if ((a ^ b) < 0 && a % b) {
        result -= 1;
    }
    return result;
}

static bool isHebrewLeapYear(int year) {
    return ((7 * year) + 1) % 19 < 7;
}

static int64_t hebrewNewYear(int year) {
    int64_t monthsElapsed = (235 * ((year - 1) / 19)) + (12 * ((year - 1) % 19)) + (((7 * ((year - 1) % 19)) + 1) / 19);
    int64_t parts = 204 + 793 * (monthsElapsed % 1080);
    int64_t hours = 5 + 12 * monthsElapsed + 793 * (monthsElapsed / 1080) + parts / 1080;
    int64_t day = 1 + 29 * monthsElapsed + hours / 24;
    int64_t halakim = (hours % 24) * 1080 + (parts % 1080);

    if (halakim >= 19440 || ((day % 7) == 2 && halakim >= 9924 && !isHebrewLeapYear(year)) ||
        ((day % 7) == 1 && halakim >= 16789 && isHebrewLeapYear(year - 1))) {
        day++;
    }
    if (day % 7 == 0 || day % 7 == 3 || day % 7 == 5) {
        day++;
    }
    return HEBREW_EPOCH + day;
}

static int hebrewYearLength(int year) {
    return static_cast<int>(hebrewNewYear(year + 1) - hebrewNewYear(year));
}

static int hebrewMonthCount(int year) {
    return isHebrewLeapYear(year) ? 13 : 12;
}

static int hebrewMonthLength(int year, int monthIndex) {
    bool leap = isHebrewLeapYear(year);
    int yearLength = hebrewYearLength(year);
    switch (monthIndex) {
        case 0: return 30; // Tishri
        case 1: return (yearLength == 355 || yearLength == 385) ? 30 : 29; // Heshvan
        case 2: return (yearLength == 353 || yearLength == 383) ? 29 : 30; // Kislev
        case 3: return 29; // Tevet
        case 4: return 30; // Shevat
        case 5: return leap ? 30 : 29; // Adar I / Adar
        case 6: return leap ? 29 : 30; // Adar II or Nisan in common year
        case 7: return leap ? 30 : 29; // Nisan or Iyar
        case 8: return leap ? 29 : 30; // Iyar or Sivan
        case 9: return leap ? 30 : 29; // Sivan or Tammuz
        case 10: return leap ? 29 : 30; // Tammuz or Av
        case 11: return leap ? 30 : 29; // Av or Elul
        case 12: return 29; // Elul
        default: return 29;
    }
}

static int64_t absoluteFromGregorian(int year, int month, int day) {
    int64_t y = year - 1;
    int64_t dayOfYear = day;
    static const int monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    for (int i = 0; i < month; ++i) {
        dayOfYear += monthDays[i];
        if (i == 1 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
            dayOfYear += 1;
        }
    }
    return y * 365 + y / 4 - y / 100 + y / 400 + dayOfYear;
}

static int64_t hebrewToAbsolute(int year, int monthIndex, int day) {
    int64_t absDay = hebrewNewYear(year);
    for (int i = 0; i < monthIndex; ++i) {
        absDay += hebrewMonthLength(year, i);
    }
    absDay += day - 1;
    return absDay;
}

static void hebrewFromAbsolute(int64_t absDay, int& year, int& monthIndex, int& day) {
    year = static_cast<int>((absDay - HEBREW_EPOCH) / 365);
    if (year < 1) {
        year = 1;
    }
    while (absDay < hebrewNewYear(year)) {
        year--;
    }
    while (absDay >= hebrewNewYear(year + 1)) {
        year++;
    }
    int64_t dayOfYear = absDay - hebrewNewYear(year);
    monthIndex = 0;
    while (monthIndex < hebrewMonthCount(year) && dayOfYear >= hebrewMonthLength(year, monthIndex)) {
        dayOfYear -= hebrewMonthLength(year, monthIndex);
        monthIndex++;
    }
    day = static_cast<int>(dayOfYear + 1);
}

static int getHebrewDayOfYear(int year, int monthIndex, int day) {
    int dayOfYear = day;
    for (int i = 0; i < monthIndex; ++i) {
        dayOfYear += hebrewMonthLength(year, i);
    }
    return dayOfYear;
}

} // namespace

namespace cdroid {

HebrewCalendar::HebrewCalendar() : Calendar() {
}

HebrewCalendar::HebrewCalendar(int year, int month, int date) : Calendar() {
    set(year, month, date);
}

HebrewCalendar::HebrewCalendar(int year, int month, int date, int hourOfDay, int minute, int second)
        : Calendar() {
    set(year, month, date);
    set(HOUR_OF_DAY, hourOfDay);
    set(MINUTE, minute);
    set(SECOND, second);
}

void HebrewCalendar::computeTime() {
    int year = internalGet(YEAR);
    int month = internalGet(MONTH);
    int day = internalGet(DAY_OF_MONTH);
    if (month < 0 || month >= hebrewMonthCount(year) || day < 1) {
        Calendar::computeTime();
        return;
    }

    int64_t absoluteDay = hebrewToAbsolute(year, month, day);
    int64_t epochDay = absoluteFromGregorian(1970, 1, 1);
    int64_t millis = (absoluteDay - epochDay) * ONE_DAY;

    int hourOfDay = isSet(HOUR_OF_DAY) ? internalGet(HOUR_OF_DAY) : 0;
    if (!isSet(HOUR_OF_DAY) && isSet(HOUR)) {
        int hour = internalGet(HOUR);
        int ampm = internalGet(AM_PM);
        hourOfDay = (ampm == PM ? 12 : 0) + hour;
    }
    int minute = isSet(MINUTE) ? internalGet(MINUTE) : 0;
    int second = isSet(SECOND) ? internalGet(SECOND) : 0;
    int millisPart = isSet(MILLISECOND) ? internalGet(MILLISECOND) : 0;

    mTime = millis + hourOfDay * ONE_HOUR + minute * ONE_MINUTE + second * ONE_SECOND + millisPart - static_cast<int64_t>(getTimeZone()) * ONE_SECOND;
    isTimeSet = true;
}

void HebrewCalendar::computeFields() {
    int64_t utcMillis = getTime();
    int64_t localMillis = utcMillis + static_cast<int64_t>(getTimeZone()) * ONE_SECOND;
    int64_t localSeconds = floorDiv(localMillis, ONE_SECOND);
    int millisecond = static_cast<int>(localMillis - localSeconds * ONE_SECOND);
    if (millisecond < 0) {
        millisecond += ONE_SECOND;
        localSeconds -= 1;
    }

    time_t localTime = static_cast<time_t>(localSeconds);
    struct tm tn;
    gmtime_r(&localTime, &tn);

    int64_t dayCount = floorDiv(localMillis, ONE_DAY);
    int64_t epochDay = absoluteFromGregorian(1970, 1, 1);
    int64_t absoluteDay = dayCount + epochDay;
    int year;
    int monthIndex;
    int day;
    hebrewFromAbsolute(absoluteDay, year, monthIndex, day);

    internalSet(YEAR, year);
    internalSet(MONTH, monthIndex);
    internalSet(DATE, day);
    internalSet(DAY_OF_MONTH, day);
    internalSet(DAY_OF_YEAR, getHebrewDayOfYear(year, monthIndex, day));
    internalSet(DAY_OF_WEEK, tn.tm_wday + 1);
    internalSet(AM_PM, tn.tm_hour / 12);
    internalSet(HOUR_OF_DAY, tn.tm_hour);
    internalSet(HOUR, tn.tm_hour % 12);
    internalSet(MINUTE, tn.tm_min);
    internalSet(SECOND, tn.tm_sec);
    internalSet(MILLISECOND, millisecond);
    internalSet(ZONE_OFFSET, getTimeZone());
    internalSet(DST_OFFSET, 0);
    internalSet(ERA, 1);
    computeWeekFields();
    setFieldsComputed(ALL_FIELDS);
}

int HebrewCalendar::handleGetMonthLength(int extendedYear, int month) const {
    return hebrewMonthLength(extendedYear, month);
}

int HebrewCalendar::handleGetYearLength(int extendedYear) const {
    return hebrewYearLength(extendedYear);
}

} // namespace cdroid
