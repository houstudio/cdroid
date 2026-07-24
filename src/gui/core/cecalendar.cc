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
#include <core/cecalendar.h>
#include <core/calendarutils.h>

namespace {
static const int64_t ONE_SECOND = 1000;
static const int64_t ONE_DAY    = 24 * 60 * 60 * ONE_SECOND;
static const int EPOCH_JULIAN_DAY = 2440588;

// CE date (0-based month, 13 months/year) -> Julian day. ICU CECalendar.ceToJD.
static int ceToJD(int64_t year, int month, int day, int jdEpochOffset) {
    if (month >= 0) {
        year += month / 13;
        month %= 13;
    } else {
        ++month;
        year += month / 13 - 1;
        month = month % 13 + 12;
    }
    return static_cast<int>(jdEpochOffset + 365 * year
            + cdroid::CalendarUtils::floorDivide(year, static_cast<int64_t>(4))
            + 30 * month + day - 1);
}

// Julian day -> CE date (year, 0-based month, day). ICU CECalendar.jdToCE.
static void jdToCE(int julianDay, int jdEpochOffset, int& year, int& month, int& day) {
    int r4;
    int c4 = cdroid::CalendarUtils::floorDivide(julianDay - jdEpochOffset, 1461, r4);
    year = 4 * c4 + (r4 / 365 - r4 / 1460);
    int doy = (r4 == 1460) ? 365 : (r4 % 365);
    month = doy / 30;
    day = (doy % 30) + 1;
}

static int julianDayToDayOfWeek(int jd) {
    int dow = (jd + cdroid::Calendar::MONDAY) % 7;
    if (dow < cdroid::Calendar::SUNDAY) dow += 7;
    return dow;
}
} // namespace

namespace cdroid {

CECalendar::CECalendar() : GregorianCalendar() {
}

CECalendar::CECalendar(int year, int month, int date)
        : GregorianCalendar(year, month, date) {
}

CECalendar::CECalendar(int year, int month, int date, int hourOfDay, int minute, int second)
        : GregorianCalendar(year, month, date, hourOfDay, minute, second) {
}

void CECalendar::computeTime() {
    int year = isSet(YEAR) ? internalGet(YEAR) : 1;
    int month = internalGet(MONTH); // 0-based
    int day = isSet(DAY_OF_MONTH) ? internalGet(DAY_OF_MONTH) : 1;
    int jd = ceToJD(year, month, day, getJDEpochOffset());

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

void CECalendar::computeFields() {
    int64_t zoneMillis = static_cast<int64_t>(getTimeZone()) * ONE_SECOND;
    int64_t localMillis = getTime() + zoneMillis;
    int64_t days = cdroid::CalendarUtils::floorDivide(localMillis, ONE_DAY);
    int julianDay = static_cast<int>(days) + EPOCH_JULIAN_DAY;

    int year, month, day;
    jdToCE(julianDay, getJDEpochOffset(), year, month, day);

    internalSet(YEAR, year);
    internalSet(MONTH, month); // 0-based
    internalSet(DATE, day);
    internalSet(DAY_OF_MONTH, day);
    internalSet(DAY_OF_YEAR, month * 30 + day);
    internalSet(DAY_OF_WEEK, julianDayToDayOfWeek(julianDay));

    int64_t millisInDay = localMillis - days * ONE_DAY;
    internalSet(MILLISECOND, static_cast<int>(millisInDay % 1000));
    millisInDay /= 1000;
    internalSet(SECOND, static_cast<int>(millisInDay % 60));
    millisInDay /= 60;
    internalSet(MINUTE, static_cast<int>(millisInDay % 60));
    millisInDay /= 60;
    internalSet(HOUR_OF_DAY, static_cast<int>(millisInDay));
    internalSet(AM_PM, static_cast<int>(millisInDay / 12));
    internalSet(HOUR, static_cast<int>(millisInDay % 12));
    internalSet(ZONE_OFFSET, static_cast<int>(zoneMillis));
    internalSet(DST_OFFSET, 0);
    internalSet(ERA, 1);

    computeWeekFields();
    setFieldsComputed(ALL_FIELDS);
}

// 12 months of 30 days + intercalary month (5 days, 6 in leap years).
int CECalendar::handleGetMonthLength(int extendedYear, int month) const {
    if ((month + 1) % 13 != 0) return 30;
    return ((extendedYear % 4) / 3) + 5;
}

int CECalendar::handleGetYearLength(int extendedYear) const {
    return 365 + ((extendedYear % 4) / 3);
}

} // namespace cdroid
