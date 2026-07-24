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
#include <gregoriancalendar.h>

namespace cdroid {

static const int LIMITS[][4] = {
    { 0, 0, 1, 1 }, // ERA
    { 1, 1, 5828963, 5838270 }, // YEAR
    { 0, 0, 11, 11 }, // MONTH
    { 1, 1, 52, 53 }, // WEEK_OF_YEAR
    { 0, 0, 0, 0 }, // WEEK_OF_MONTH
    { 1, 1, 28, 31 }, // DAY_OF_MONTH
    { 1, 1, 365, 366 }, // DAY_OF_YEAR
    { 0, 0, 0, 0 }, // DAY_OF_WEEK
    { -1, -1, 5, 5 }, // DAY_OF_WEEK_IN_MONTH
    { 0, 0, 0, 0 }, // AM_PM
    { 0, 0, 0, 0 }, // HOUR
    { 0, 0, 0, 0 }, // HOUR_OF_DAY
    { 0, 0, 0, 0 }, // MINUTE
    { 0, 0, 0, 0 }, // SECOND
    { 0, 0, 0, 0 }, // MILLISECOND
    { 0, 0, 0, 0 }, // ZONE_OFFSET
    { 0, 0, 0, 0 }, // DST_OFFSET
    { 1, 1, 1, 1 }, // WEEK_YEAR
};

GregorianCalendar::GregorianCalendar() : Calendar() {
}

GregorianCalendar::GregorianCalendar(int year, int month, int date) : Calendar() {
    set(year, month, date);
}

GregorianCalendar::GregorianCalendar(int year, int month, int date, int hourOfDay, int minute, int second)
        : Calendar() {
    set(year, month, date, hourOfDay, minute, second, 0);
}

int GregorianCalendar::handleGetLimit(int field, int limitType) const {
    if (field < 0 || field >= static_cast<int>(sizeof(LIMITS) / sizeof(LIMITS[0]))) {
        return 0;
    }
    if (limitType < LIMIT_MINIMUM || limitType > LIMIT_MAXIMUM) {
        return 0;
    }
    return LIMITS[field][limitType];
}

bool GregorianCalendar::isLeapYear(int year) const {
    return Calendar::isLeapYear(year);
}

void GregorianCalendar::computeTime() {
    Calendar::computeTime();
}

void GregorianCalendar::computeFields() {
    Calendar::computeFields();
}

} // namespace cdroid
