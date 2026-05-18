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
#include <taiwancalendar.h>

namespace {
static const int ONE_SECOND = 1000;
static const int ONE_MINUTE = 60 * ONE_SECOND;
static const int ONE_HOUR = 60 * ONE_MINUTE;
static const int ONE_DAY = 24 * ONE_HOUR;

static bool isGregorianLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int getDaysInGregorianMonth(int year, int month) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == cdroid::Calendar::FEBRUARY) {
        return days[month] + (isGregorianLeapYear(year) ? 1 : 0);
    }
    return days[month];
}

static int getGregorianDayOfYear(int year, int month, int day) {
    static const int monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int dayOfYear = day;
    for (int i = 0; i < month; ++i) {
        dayOfYear += monthDays[i];
        if (i == cdroid::Calendar::FEBRUARY && isGregorianLeapYear(year)) {
            dayOfYear += 1;
        }
    }
    return dayOfYear;
}

static void gregorianFromDayOfYear(int year, int dayOfYear, int& month, int& day) {
    static const int monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int remaining = dayOfYear;
    for (int i = 0; i < 12; ++i) {
        int length = monthDays[i];
        if (i == cdroid::Calendar::FEBRUARY && isGregorianLeapYear(year)) {
            length += 1;
        }
        if (remaining <= length) {
            month = i;
            day = remaining;
            return;
        }
        remaining -= length;
    }
    month = cdroid::Calendar::DECEMBER;
    day = 31;
}

static int getTaiwanYearStartDayOfYear(int /*year*/) {
    return 81; // March 21 on leap years, March 22 on common years; both are day 81
}

static int getIndianMonthLength(int sakaYear, int month) {
    if (month == 0) {
        return isGregorianLeapYear(sakaYear + 78) ? 31 : 30;
    }
    return (month >= 1 && month <= 5) ? 31 : 30;
}

static void sakaToGregorian(int sakaYear, int sakaMonth, int sakaDay, int& year, int& month, int& day) {
    year = sakaYear + 78;
    int dayOfYear = getTaiwanYearStartDayOfYear(year) + sakaDay - 1;
    for (int i = 0; i < sakaMonth; ++i) {
        dayOfYear += getIndianMonthLength(sakaYear, i);
    }
    int daysInYear = isGregorianLeapYear(year) ? 366 : 365;
    if (dayOfYear > daysInYear) {
        dayOfYear -= daysInYear;
        year += 1;
    }
    gregorianFromDayOfYear(year, dayOfYear, month, day);
}

static void gregorianToSaka(int gYear, int gMonth, int gDay, int& sakaYear, int& sakaMonth, int& sakaDay) {
    int dayOfYear = getGregorianDayOfYear(gYear, gMonth, gDay);
    int startDay = getTaiwanYearStartDayOfYear(gYear);
    if (dayOfYear < startDay) {
        int prevYear = gYear - 1;
        int prevYearDays = isGregorianLeapYear(prevYear) ? 366 : 365;
        int dayOfYearPrev = dayOfYear + prevYearDays;
        sakaYear = prevYear - 78;
        int offset = dayOfYearPrev - getTaiwanYearStartDayOfYear(prevYear);
        int monthIndex = 0;
        while (offset >= getIndianMonthLength(sakaYear, monthIndex)) {
            offset -= getIndianMonthLength(sakaYear, monthIndex);
            monthIndex++;
        }
        sakaMonth = monthIndex;
        sakaDay = offset + 1;
    } else {
        sakaYear = gYear - 78;
        int offset = dayOfYear - startDay;
        int monthIndex = 0;
        while (offset >= getIndianMonthLength(sakaYear, monthIndex)) {
            offset -= getIndianMonthLength(sakaYear, monthIndex);
            monthIndex++;
        }
        sakaMonth = monthIndex;
        sakaDay = offset + 1;
    }
}

} // namespace

namespace cdroid {

TaiwanCalendar::TaiwanCalendar() : GregorianCalendar() {
}

TaiwanCalendar::TaiwanCalendar(int year, int month, int date)
        : GregorianCalendar(year, month, date) {
}

TaiwanCalendar::TaiwanCalendar(int year, int month, int date, int hourOfDay, int minute, int second)
        : GregorianCalendar(year, month, date, hourOfDay, minute, second) {
}

void TaiwanCalendar::computeTime() {
    int year = internalGet(YEAR);
    int month = internalGet(MONTH);
    int day = internalGet(DAY_OF_MONTH);
    if (month < JANUARY || month > DECEMBER || day < 1) {
        GregorianCalendar::computeTime();
        return;
    }

    internalSet(YEAR, year + 1911);
    GregorianCalendar::computeTime();
    internalSet(YEAR, year);
}

void TaiwanCalendar::computeFields() {
    Calendar::computeFields();
    int gYear = internalGet(YEAR);
    int weekOfYear = internalGet(WEEK_OF_YEAR);
    int weekYear = internalGet(WEEK_YEAR);
    int weekOfMonth = internalGet(WEEK_OF_MONTH);
    int dayOfWeekInMonth = internalGet(DAY_OF_WEEK_IN_MONTH);

    internalSet(YEAR, gYear - 1911);
    internalSet(ERA, 1);
    internalSet(WEEK_OF_YEAR, weekOfYear);
    internalSet(WEEK_YEAR, weekYear);
    internalSet(WEEK_OF_MONTH, weekOfMonth);
    internalSet(DAY_OF_WEEK_IN_MONTH, dayOfWeekInMonth);
}

} // namespace cdroid
