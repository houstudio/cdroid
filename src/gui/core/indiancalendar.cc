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

#include <indiancalendar.h>
#include <algorithm>

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
    int dayOfYear = day;
    for (int i = 0; i < month; ++i) {
        dayOfYear += getDaysInGregorianMonth(year, i);
    }
    return dayOfYear;
}

static void gregorianFromDayOfYear(int year, int dayOfYear, int& month, int& day) {
    int remaining = dayOfYear;
    for (int i = 0; i < 12; ++i) {
        int length = getDaysInGregorianMonth(year, i);
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

static bool isIndianLeapYear(int sakaYear) {
    return isGregorianLeapYear(sakaYear + 78);
}

static int getIndianMonthLength(int sakaYear, int month) {
    if (month == 0) {
        return isIndianLeapYear(sakaYear) ? 31 : 30;
    }
    return (month >= 1 && month <= 5) ? 31 : 30;
}

static int getIndianYearLength(int sakaYear) {
    int length = 0;
    for (int i = 0; i < 12; ++i) {
        length += getIndianMonthLength(sakaYear, i);
    }
    return length;
}

static void sakaToGregorian(int sakaYear, int sakaMonth, int sakaDay, int& year, int& month, int& day) {
    year = sakaYear + 78;
    int dayOfYear = 81 + sakaDay - 1;
    for (int idx = 0; idx < sakaMonth; ++idx) {
        dayOfYear += getIndianMonthLength(sakaYear, idx);
    }
    int yearDays = isGregorianLeapYear(year) ? 366 : 365;
    if (dayOfYear > yearDays) {
        dayOfYear -= yearDays;
        year += 1;
    }
    gregorianFromDayOfYear(year, dayOfYear, month, day);
}

static void gregorianToSaka(int gYear, int gMonth, int gDay, int& sakaYear, int& sakaMonth, int& sakaDay) {
    int dayOfYear = getGregorianDayOfYear(gYear, gMonth, gDay);
    int startDay = 81;
    if (dayOfYear < startDay) {
        int prevYear = gYear - 1;
        int prevYearDays = isGregorianLeapYear(prevYear) ? 366 : 365;
        sakaYear = prevYear - 78;
        int offset = dayOfYear + prevYearDays - startDay;
        sakaMonth = 0;
        while (offset >= getIndianMonthLength(sakaYear, sakaMonth)) {
            offset -= getIndianMonthLength(sakaYear, sakaMonth);
            sakaMonth++;
        }
        sakaDay = offset + 1;
    } else {
        sakaYear = gYear - 78;
        int offset = dayOfYear - startDay;
        sakaMonth = 0;
        while (offset >= getIndianMonthLength(sakaYear, sakaMonth)) {
            offset -= getIndianMonthLength(sakaYear, sakaMonth);
            sakaMonth++;
        }
        sakaDay = offset + 1;
    }
}

static int getIndianDayOfYear(int sakaYear, int month, int day) {
    int dayOfYear = day;
    for (int i = 0; i < month; ++i) {
        dayOfYear += getIndianMonthLength(sakaYear, i);
    }
    return dayOfYear;
}

} // namespace

namespace cdroid {

IndianCalendar::IndianCalendar() : GregorianCalendar() {
}

IndianCalendar::IndianCalendar(int year, int month, int date)
        : GregorianCalendar(year, month, date) {
}

IndianCalendar::IndianCalendar(int year, int month, int date, int hourOfDay, int minute, int second)
        : GregorianCalendar(year, month, date, hourOfDay, minute, second) {
}

void IndianCalendar::computeTime() {
    int year = internalGet(YEAR);
    int month = internalGet(MONTH);
    int day = internalGet(DAY_OF_MONTH);
    if (month < JANUARY || month > DECEMBER || day < 1) {
        GregorianCalendar::computeTime();
        return;
    }

    int gYear;
    int gMonth;
    int gDay;
    sakaToGregorian(year, month, day, gYear, gMonth, gDay);
    internalSet(YEAR, gYear);
    internalSet(MONTH, gMonth);
    internalSet(DAY_OF_MONTH, gDay);
    GregorianCalendar::computeTime();
    internalSet(YEAR, year);
    internalSet(MONTH, month);
    internalSet(DAY_OF_MONTH, day);
}

void IndianCalendar::computeFields() {
    Calendar::computeFields();
    int gYear = internalGet(YEAR);
    int gMonth = internalGet(MONTH);
    int gDay = internalGet(DAY_OF_MONTH);
    int year;
    int month;
    int day;
    gregorianToSaka(gYear, gMonth, gDay, year, month, day);

    internalSet(YEAR, year);
    internalSet(MONTH, month);
    internalSet(DATE, day);
    internalSet(DAY_OF_MONTH, day);
    internalSet(DAY_OF_YEAR, getIndianDayOfYear(year, month, day));
    internalSet(ERA, 1);
    computeWeekFields();
    setFieldsComputed(ALL_FIELDS);
}

int IndianCalendar::handleGetMonthLength(int extendedYear, int month) const {
    return getIndianMonthLength(extendedYear, month);
}

int IndianCalendar::handleGetYearLength(int extendedYear) const {
    return getIndianYearLength(extendedYear);
}

} // namespace cdroid
